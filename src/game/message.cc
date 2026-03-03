#include "game/message.h"

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "game/gconfig.h"
#include "game/raii.h"
#include "game/roll.h"
#include "platform_compat.h"
#include "plib/db/db.h"
#include "plib/gnw/debug.h"
#include "plib/gnw/memory.h"

namespace fallout {

static constexpr int BADWORD_LENGTH_MAX = 80;

static bool message_parse_number(int* out_num, const char* str);
static int message_load_field(DB_FILE* file, char* str);

// 0x505B10
static char** bad_word = nullptr;

// 0x505B14
static int bad_total = 0;

// 0x505B18
static int* bad_len = nullptr;

// Temporary message list item text used during filtering badwords.
//
// 0x6305D0
static char bad_copy[MESSAGE_LIST_ITEM_FIELD_MAX_SIZE];

// 0x4764E0
int init_message()
{
    DbFileGuard stream(db_fopen("data\\badwords.txt", "rt"));
    if (!stream) {
        return -1;
    }

    char word[BADWORD_LENGTH_MAX];

    bad_total = 0;
    while (stream.get()->fgets(word, BADWORD_LENGTH_MAX - 1)) {
        bad_total++;
    }

    // Use local RAII guards; release to globals on success.
    MemBuffer<char*> wordGuard(static_cast<char**>(mem_malloc(sizeof(char*) * bad_total)));
    if (!wordGuard) {
        return -1;
    }

    MemBuffer<int> lenGuard(static_cast<int*>(mem_malloc(sizeof(int) * bad_total)));
    if (!lenGuard) {
        return -1;
    }

    stream.get()->fseek(0, SEEK_SET);

    int index = 0;
    for (; index < bad_total; index++) {
        if (!stream.get()->fgets(word, BADWORD_LENGTH_MAX - 1)) {
            break;
        }

        int len = strlen(word);
        if (word[len - 1] == '\n') {
            len--;
            word[len] = '\0';
        }

        wordGuard[index] = mem_strdup(word);
        if (wordGuard[index] == nullptr) {
            break;
        }

        compat_strupr(wordGuard[index]);

        lenGuard[index] = len;
    }

    if (index != bad_total) {
        for (; index > 0; index--) {
            mem_free(wordGuard[index - 1]);
        }
        return -1;
    }

    // Success — transfer ownership to module globals.
    bad_word = wordGuard.release();
    bad_len = lenGuard.release();

    return 0;
}

// 0x476660
void exit_message()
{
    for (int index = 0; index < bad_total; index++) {
        mem_free(bad_word[index]);
    }

    if (bad_total != 0) {
        mem_free(bad_word);
        mem_free(bad_len);
    }

    bad_total = 0;
}

// 0x4766BC
bool MessageList::init()
{
    entries_num_ = 0;
    entries_ = nullptr;
    return true;
}

// 0x4766D4
bool MessageList::exit()
{
    for (int i = 0; i < entries_num_; i++) {
        MessageListItem* entry = &(entries_[i]);

        if (entry->audio != nullptr) {
            mem_free(entry->audio);
        }

        if (entry->text != nullptr) {
            mem_free(entry->text);
        }
    }

    entries_num_ = 0;

    if (entries_ != nullptr) {
        mem_free(entries_);
        entries_ = nullptr;
    }

    return true;
}

// 0x476814
bool MessageList::load(const char* path)
{
    char* language;
    char localized_path[COMPAT_MAX_PATH];
    char num[MESSAGE_LIST_ITEM_FIELD_MAX_SIZE];
    char audio[MESSAGE_LIST_ITEM_FIELD_MAX_SIZE];
    char text[MESSAGE_LIST_ITEM_FIELD_MAX_SIZE];
    int rc;
    bool success = false;
    MessageListItem entry;

    if (path == nullptr) {
        return false;
    }

    if (!game_config.getString(GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_LANGUAGE_KEY, &language)) {
        return false;
    }

    snprintf(localized_path, sizeof(localized_path), "%s\\%s\\%s", "text", language, path);

    DbFileGuard file_ptr(db_fopen(localized_path, "rt"));
    if (!file_ptr) {
        return false;
    }

    entry.num = 0;
    entry.audio = audio;
    entry.text = text;

    while (1) {
        rc = message_load_field(file_ptr.get(), num);
        if (rc != 0) {
            break;
        }

        if (message_load_field(file_ptr.get(), audio) != 0) {
            debug_printf("\nError loading audio field.\n", localized_path);
            rc = -1;
            break;
        }

        if (message_load_field(file_ptr.get(), text) != 0) {
            debug_printf("\nError loading text field.\n", localized_path);
            rc = -1;
            break;
        }

        if (!message_parse_number(&(entry.num), num)) {
            debug_printf("\nError parsing number.\n", localized_path);
            rc = -1;
            break;
        }

        if (!add(&entry)) {
            debug_printf("\nError adding message.\n", localized_path);
            rc = -1;
            break;
        }
    }

    if (rc == 1) {
        success = true;
    }

    if (!success) {
        debug_printf("Error loading message file %s at offset %x.", localized_path, file_ptr.get()->ftell());
    }

    return success;
}

// 0x476998
bool MessageList::search(MessageListItem* entry)
{
    if (entry == nullptr) {
        return false;
    }

    if (entries_num_ == 0) {
        return false;
    }

    int index;
    if (!find(entry->num, &index)) {
        return false;
    }

    MessageListItem* ptr = &(entries_[index]);
    entry->audio = ptr->audio;
    entry->text = ptr->text;

    return true;
}

// Builds language-aware path in "text" subfolder.
//
// 0x476A20
bool message_make_path(char* dest, size_t size, const char* path)
{
    char* language;

    if (dest == nullptr) {
        return false;
    }

    if (path == nullptr) {
        return false;
    }

    if (!game_config.getString(GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_LANGUAGE_KEY, &language)) {
        return false;
    }

    snprintf(dest, size, "%s\\%s\\%s", "text", language, path);

    return true;
}

// 0x476A78
bool MessageList::find(int num, int* out_index)
{
    if (entries_num_ == 0) {
        *out_index = 0;
        return false;
    }

    int r = entries_num_ - 1;
    int l = 0;
    int cmp;

    do {
        int mid = (l + r) / 2;
        cmp = num - entries_[mid].num;
        if (cmp == 0) {
            *out_index = mid;
            return true;
        }

        if (cmp > 0) {
            l = l + 1;
        } else {
            r = r - 1;
        }
    } while (r >= l);

    int mid = (l + r) / 2;
    if (cmp < 0) {
        *out_index = mid;
    } else {
        *out_index = mid + 1;
    }

    return false;
}

// 0x476AD0
bool MessageList::add(MessageListItem* new_entry)
{
    int index;
    MessageListItem* existing_entry;

    if (find(new_entry->num, &index)) {
        existing_entry = &(entries_[index]);

        if (existing_entry->audio != nullptr) {
            mem_free(existing_entry->audio);
        }

        if (existing_entry->text != nullptr) {
            mem_free(existing_entry->text);
        }
    } else {
        if (entries_ != nullptr) {
            MessageListItem* entries = static_cast<MessageListItem*>(mem_realloc(entries_, sizeof(MessageListItem) * (entries_num_ + 1)));
            if (entries == nullptr) {
                return false;
            }

            entries_ = entries;

            if (index != entries_num_) {
                // Move all items below insertion point
                memmove(&(entries_[index + 1]), &(entries_[index]), sizeof(MessageListItem) * (entries_num_ - index));
            }
        } else {
            entries_ = static_cast<MessageListItem*>(mem_malloc(sizeof(MessageListItem)));
            if (entries_ == nullptr) {
                return false;
            }
            entries_num_ = 0;
            index = 0;
        }

        existing_entry = &(entries_[index]);
        existing_entry->audio = 0;
        existing_entry->text = 0;
        entries_num_++;
    }

    existing_entry->audio = mem_strdup(new_entry->audio);
    if (existing_entry->audio == nullptr) {
        return false;
    }

    existing_entry->text = mem_strdup(new_entry->text);
    if (existing_entry->text == nullptr) {
        return false;
    }

    existing_entry->num = new_entry->num;

    return true;
}

// 0x476D80
static bool message_parse_number(int* out_num, const char* str)
{
    const char* ch;
    bool success;

    ch = str;
    if (*ch == '\0') {
        return false;
    }

    success = true;
    if (*ch == '+' || *ch == '-') {
        ch++;
    }

    while (*ch != '\0') {
        if (!isdigit(*ch)) {
            success = false;
            break;
        }
        ch++;
    }

    *out_num = atoi(str);
    return success;
}

// Read next message file field, the `str` should be at least
// `MESSAGE_LIST_ITEM_FIELD_MAX_SIZE` bytes long.
//
// Returns:
// 0 - ok
// 1 - eof
// 2 - mismatched delimeters
// 3 - unterminated field
// 4 - limit exceeded (> `MESSAGE_LIST_ITEM_FIELD_MAX_SIZE`)
//
// 0x476DD4
static int message_load_field(DB_FILE* file, char* str)
{
    int ch;
    int len;

    len = 0;

    while (1) {
        ch = file->fgetc();
        if (ch == -1) {
            return 1;
        }

        if (ch == '}') {
            debug_printf("\nError reading message file - mismatched delimiters.\n");
            return 2;
        }

        if (ch == '{') {
            break;
        }
    }

    while (1) {
        ch = file->fgetc();

        if (ch == -1) {
            debug_printf("\nError reading message file - EOF reached.\n");
            return 3;
        }

        if (ch == '}') {
            *(str + len) = '\0';
            return 0;
        }

        if (ch != '\n') {
            *(str + len) = ch;
            len++;

            if (len >= MESSAGE_LIST_ITEM_FIELD_MAX_SIZE) {
                debug_printf("\nError reading message file - text exceeds limit.\n");
                return 4;
            }
        }
    }

    return 0;
}

// 0x476E6C
char* MessageList::getMessage(MessageListItem* entry, int num)
{
    // 0x505B1C
    static char message_error_str[] = "Error";

    entry->num = num;

    if (!search(entry)) {
        entry->text = message_error_str;
        debug_printf("\n ** String not found @ getMessage(), MESSAGE.C **\n");
    }

    return entry->text;
}

// 0x476E98
bool MessageList::filter()
{
    if (entries_num_ == 0) {
        return true;
    }

    if (bad_total == 0) {
        return true;
    }

    int languageFilter = 0;
    game_config.getValue(GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_LANGUAGE_FILTER_KEY, &languageFilter);
    if (languageFilter != 1) {
        return true;
    }

    int replacementsCount = strlen("!@#$%&*@#*!&$%#&%#*%!$&%@*$@&");
    int replacementsIndex = roll_random(1, replacementsCount) - 1;
    static const char* replacements = "!@#$%&*@#*!&$%#&%#*%!$&%@*$@&";

    for (int index = 0; index < entries_num_; index++) {
        MessageListItem* item = &(entries_[index]);
        strcpy(bad_copy, item->text);
        compat_strupr(bad_copy);

        for (int badwordIndex = 0; badwordIndex < bad_total; badwordIndex++) {
            // I don't quite understand the loop below. It has no stop
            // condition besides no matching substring. It also overwrites
            // already masked words on every iteration.
            for (char* p = bad_copy;; p++) {
                const char* substr = strstr(p, bad_word[badwordIndex]);
                if (substr == nullptr) {
                    break;
                }

                if (substr == bad_copy || (!isalpha(substr[-1]) && !isalpha(substr[bad_len[badwordIndex]]))) {
                    char* ptr = item->text + (substr - bad_copy);

                    for (int j = 0; j < bad_len[badwordIndex]; j++) {
                        *ptr++ = replacements[replacementsIndex++];
                        if (replacementsIndex == replacementsCount) {
                            replacementsIndex = 0;
                        }
                    }
                }
            }
        }
    }

    return true;
}

} // namespace fallout

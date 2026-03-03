#include "plib/db/db.h"

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#if defined(_WIN32)
#include <windows.h>
#else
#include <dirent.h>
#endif

#include <fpattern/fpattern.h>

#include "platform_compat.h"
#include "plib/assoc/assoc.h"
#include "plib/db/lzss.h"

namespace fallout {

static constexpr int DB_DATABASE_LIST_CAPACITY = 10;
static constexpr int DB_HASH_TABLE_SIZE = 4095;

#if defined(_WIN32)
static constexpr char PATH_SEP = '\\';
#else
static constexpr char PATH_SEP = '/';
#endif

struct DB_FIND_DATA {
#if defined(_WIN32)
    HANDLE hFind;
    WIN32_FIND_DATAA ffd;
#else
    DIR* dir;
    struct dirent* entry;
    char path[COMPAT_MAX_PATH];
#endif
};

static int db_read_long(FILE* stream, int* value_ptr);
static int db_write_long(FILE* stream, int value);
static int db_assoc_load_dir_entry(FILE* stream, void* buffer, size_t size, int flags);
static int db_assoc_save_dir_entry(FILE* stream, void* buffer, size_t size, int flags);
static int db_assoc_load_db_dir_entry(DB_FILE* stream, void* buffer, size_t size, int flags);
static int db_assoc_save_db_dir_entry(DB_FILE* stream, void* buffer, size_t size, int flags);
static int db_create_database(DB_DATABASE** database_ptr);
static int db_destroy_database(DB_DATABASE** database_ptr);
static int db_hash_string_to_key(const char* path, int sep, unsigned int* key_ptr);
static DB_FILE* db_add_fp_rec(FILE* stream, unsigned char* a2, int a3, int flags);
static int db_find_empty_position(int* position_ptr);
static int db_find_dir_entry(char* path, dir_entry* de);
static int db_findfirst(const char* path, DB_FIND_DATA* find_data);
static int db_findnext(DB_FIND_DATA* find_data);
static int db_findclose(DB_FIND_DATA* find_data);
static void* internal_malloc(size_t size);
static char* internal_strdup(const char* string);
static void internal_free(void* ptr);
static void* db_default_malloc(size_t size);
static char* db_default_strdup(const char* string);
static void db_default_free(void* ptr);
static int fread_short(FILE* stream, unsigned short* s);

static inline bool fileFindIsDirectory(DB_FIND_DATA* find_data);
static inline char* fileFindGetName(DB_FIND_DATA* find_data);

// 0x4FE058
static char empty_patches_path[] = "";

// 0x539D34
static DB_DATABASE* current_database = nullptr;

// 0x539D38
static bool db_used_malloc = false;

// 0x539D3C
static db_malloc_func* db_malloc = db_default_malloc;

// 0x539D40
static db_strdup_func* db_strdup = db_default_strdup;

// 0x539D44
static db_free_func* db_free = db_default_free;

// 0x539D48
static bool hash_is_on = false;

// NOTE: Original type is `unsigned long`.
//
// 0x539D4C
static size_t read_count = 0;

// NOTE: Original type is `unsigned long`.
//
// 0x539D50
static size_t read_threshold = 16384;

// 0x539D54
static db_read_callback* read_callback = nullptr;

// 0x6713C8
static DB_DATABASE* database_list[DB_DATABASE_LIST_CAPACITY];

// 0x4AEE90
DB_DATABASE* db_init(const char* datafile, const char* datafile_path, const char* patches_path, int show_cursor)
{
    DB_DATABASE* database;

    if (db_create_database(&database) != 0) {
        return INVALID_DATABASE_HANDLE;
    }

    if (database->init_database(datafile, datafile_path) != 0) {
        database->close();
        return INVALID_DATABASE_HANDLE;
    }

    if (database->init_patches(patches_path) != 0) {
        database->close();
        return INVALID_DATABASE_HANDLE;
    }

    if (current_database == nullptr) {
        current_database = database;
    }

    if (hash_is_on) {
        if (database->init_hash_table() != 0) {
            database->hash_table = nullptr;
        }
    }

    return database;
}

// 0x4AEF10
int DB_DATABASE::select()
{
    int index;

    for (index = 0; index < DB_DATABASE_LIST_CAPACITY; index++) {
        if (database_list[index] == this) {
            current_database = this;
            return 0;
        }
    }

    return -1;
}

// 0x4AEF54
DB_DATABASE* db_current()
{
    if (current_database != nullptr) {
        return current_database;
    }

    return INVALID_DATABASE_HANDLE;
}

// 0x4AEF6C
int db_total()
{
    int index;
    int count;

    count = 0;
    for (index = 0; index < DB_DATABASE_LIST_CAPACITY; index++) {
        if (database_list[index] != nullptr) {
            count++;
        }
    }

    return count;
}

// 0x4AEF88
int DB_DATABASE::close()
{
    int index;

    for (index = 0; index < DB_DATABASE_LIST_CAPACITY; index++) {
        if (database_list[index] == this) {
            if (database_list[index] == current_database) {
                current_database = nullptr;
            }

            database_list[index]->exit_database();
            database_list[index]->exit_patches();
            database_list[index]->exit_hash_table();
            db_destroy_database(&(database_list[index]));

            return 0;
        }
    }

    return -1;
}

// 0x4AF048
void db_exit()
{
    int index;

    for (index = 0; index < DB_DATABASE_LIST_CAPACITY; index++) {
        if (database_list[index] != nullptr) {
            database_list[index]->close();
        }
    }
}

// 0x4AF068
int db_dir_entry(const char* name, dir_entry* de)
{
    char path[COMPAT_MAX_PATH];
    bool v2;
    bool v3;
    int value;
    FILE* stream;

    if (current_database == nullptr) {
        return -1;
    }

    if (name == nullptr) {
        return -1;
    }

    if (de == nullptr) {
        return -1;
    }

    v2 = true;
    if (name[0] == '@') {
        strcpy(path, name + 1);
        v2 = false;
    }

    if (current_database->patches_path != nullptr) {
        stream = nullptr;
        v3 = false;

        if (v2) {
            snprintf(path, sizeof(path), "%s%s", current_database->patches_path, name);
        }

        compat_windows_path_to_native(path);

        if (current_database->get_hash_value(path, PATH_SEP, &value) != 0 || value == 1) {
            v3 = true;
        }

        if (v3) {
            stream = compat_fopen(path, "rb");
        }

        if (stream != nullptr) {
            de->flags = 4;
            de->offset = 0;
            de->length = getFileSize(stream);
            de->field_C = 0;
            fclose(stream);
            return 0;
        }
    }

    if (current_database->datafile == nullptr) {
        return -1;
    }

    if (v2) {
        snprintf(path, sizeof(path), "%s%s", current_database->datafile_path, name);
    }

    compat_strupr(path);

    if (db_find_dir_entry(path, de) != 0) {
        return -1;
    }

    if (de->flags == 0) {
        de->flags = 16;
    }

    de->flags |= 8;

    return 0;
}

// 0x4AF4F8
int db_read_to_buf(const char* filename, unsigned char* buf)
{
    bool v1;
    char path[COMPAT_MAX_PATH];
    bool v3;
    FILE* stream;
    int hash_value;
    int size;
    size_t bytes_read;
    int remaining_size;
    int chunk_size;
    dir_entry de;
    unsigned char* end;
    unsigned short v4;

    if (current_database == nullptr) {
        return -1;
    }

    if (filename == nullptr) {
        return -1;
    }

    if (buf == nullptr) {
        return -1;
    }

    v1 = true;
    if (filename[0] == '@') {
        strcpy(path, filename + 1);
        v1 = false;
    }

    if (current_database->patches_path != nullptr) {
        stream = nullptr;
        v3 = false;

        if (v1) {
            snprintf(path, sizeof(path), "%s%s", current_database->patches_path, filename);
        }

        compat_windows_path_to_native(path);

        if (current_database->get_hash_value(path, PATH_SEP, &hash_value) != 0 || hash_value == 1) {
            v3 = true;
        }

        if (v3) {
            stream = compat_fopen(path, "rb");
        }

        if (stream != nullptr) {
            size = getFileSize(stream);
            if (read_callback != nullptr) {
                remaining_size = size;
                chunk_size = read_threshold - read_count;

                while (remaining_size >= chunk_size) {
                    bytes_read = fread(buf, 1, chunk_size, stream);
                    buf += bytes_read;
                    remaining_size -= bytes_read;

                    read_count = 0;
                    read_callback();

                    chunk_size = read_threshold;
                }

                if (remaining_size != 0) {
                    fread(buf, 1, remaining_size, stream);
                    read_count += remaining_size;
                }
            } else {
                fread(buf, 1, size, stream);
            }

            fclose(stream);

            return 0;
        }
    }

    if (current_database->datafile == nullptr) {
        return -1;
    }

    if (v1) {
        snprintf(path, sizeof(path), "%s%s", current_database->datafile_path, filename);
    }

    compat_strupr(path);

    if (db_find_dir_entry(path, &de) == -1) {
        return -1;
    }

    if (current_database->stream == nullptr) {
        return -1;
    }

    if (fseek(current_database->stream, de.offset, SEEK_SET) != 0) {
        return -1;
    }

    if (de.flags == 0) {
        de.flags = 16;
    }

    switch (de.flags & 0xF0) {
    case 16:
        lzss_decode_to_buf(current_database->stream, buf, de.field_C);
        break;
    case 32:
        if (read_callback != nullptr) {
            remaining_size = de.length;
            chunk_size = read_threshold - read_count;

            while (remaining_size >= chunk_size) {
                bytes_read = fread(buf, 1, chunk_size, current_database->stream);
                buf += bytes_read;
                remaining_size -= bytes_read;

                read_count = 0;
                read_callback();

                chunk_size = read_threshold;
            }

            if (remaining_size != 0) {
                fread(buf, 1, remaining_size, current_database->stream);
                read_count += remaining_size;
            }
        } else {
            fread(buf, 1, de.length, current_database->stream);
        }
        break;
    case 64:
        end = buf + de.length;
        if (read_callback != nullptr) {
            while (buf < end) {
                if (fread_short(current_database->stream, &v4) == 0) {
                    if ((v4 & 0x8000) != 0) {
                        v4 &= ~0x8000;
                        bytes_read = fread(buf, 1, v4, current_database->stream);

                        buf += bytes_read;
                        read_count += bytes_read;

                        while (read_count >= read_threshold) {
                            read_count -= read_threshold;
                            read_callback();
                        }
                    } else {
                        read_count += lzss_decode_to_buf(current_database->stream, buf, v4);
                        while (read_count >= read_threshold) {
                            read_count -= read_threshold;
                            read_callback();
                        }
                    }
                }
            }
        } else {
            while (buf < end) {
                if (fread_short(current_database->stream, &v4) == 0) {
                    if ((v4 & 0x8000) != 0) {
                        v4 &= ~0x8000;
                        fread(buf, 1, v4, current_database->stream);
                        buf += v4;
                    } else {
                        buf += lzss_decode_to_buf(current_database->stream, buf, v4);
                    }
                }
            }
        }
    }

    return 0;
}

// 0x4AF9C4
DB_FILE* db_fopen(const char* filename, const char* mode)
{
    bool v1;
    char path[COMPAT_MAX_PATH];
    FILE* stream;
    bool v2;
    int hash_value;
    int mode_value;
    bool mode_is_text;
    int flags;
    int k;
    dir_entry de;
    unsigned char* buf;

    if (current_database == nullptr) {
        return nullptr;
    }

    if (filename == nullptr) {
        return nullptr;
    }

    if (mode == nullptr) {
        return nullptr;
    }

    if (current_database->files_length >= DB_DATABASE_FILE_LIST_CAPACITY) {
        return nullptr;
    }

    mode_value = -1;
    mode_is_text = true;
    for (k = 0; mode[k] != '\0'; k++) {
        switch (mode[k]) {
        case 'b':
            mode_is_text = false;
            break;
        case '+':
        case 'a':
        case 'w':
            mode_value = 0;
            break;
        case 'r':
            mode_value = 1;
            break;
        }
    }

    if (mode_value == -1) {
        return nullptr;
    }

    stream = nullptr;
    flags = 1;
    if (mode_is_text) {
        flags = 2;
    }

    v1 = true;
    if (filename[0] == '@') {
        strcpy(path, filename + 1);
        v1 = false;
    }

    if (current_database->patches_path != nullptr) {
        v2 = false;

        if (v1) {
            snprintf(path, sizeof(path), "%s%s", current_database->patches_path, filename);
        }

        compat_windows_path_to_native(path);

        if (mode_value == 0) {
            current_database->add_hash_entry(path, PATH_SEP);
            v2 = true;
        } else {
            if (current_database->get_hash_value(path, PATH_SEP, &hash_value) != 0 || hash_value == 1) {
                v2 = true;
            }
        }

        if (v2) {
            stream = compat_fopen(path, mode);
        }

        if (stream != nullptr) {
            return db_add_fp_rec(stream, nullptr, 0, flags | 0x4);
        }
    }

    if (mode_value == 0) {
        return nullptr;
    }

    if (current_database->datafile == nullptr) {
        return nullptr;
    }

    if (v1) {
        snprintf(path, sizeof(path), "%s%s", current_database->datafile_path, filename);
    }

    compat_strupr(path);

    if (db_find_dir_entry(path, &de) == -1) {
        return nullptr;
    }

    if (current_database->stream == nullptr) {
        return nullptr;
    }

    if (fseek(current_database->stream, de.offset, SEEK_SET) != 0) {
        return nullptr;
    }

    if (de.flags == 0) {
        de.flags = 16;
    }

    switch (de.flags & 0xF0) {
    case 16:
        buf = static_cast<unsigned char*>(internal_malloc(de.length));
        if (buf != nullptr) {
            lzss_decode_to_buf(current_database->stream, buf, de.field_C);
            return db_add_fp_rec(nullptr, buf, de.length, flags | 0x10 | 0x8);
        }
        break;
    case 32:
        return db_add_fp_rec(current_database->stream, nullptr, de.length, flags | 0x20 | 0x8);
    case 64:
        buf = static_cast<unsigned char*>(internal_malloc(0x4000));
        if (buf != nullptr) {
            return db_add_fp_rec(current_database->stream, buf, de.length, flags | 0x40 | 0x8);
        }
        break;
    }

    return nullptr;
}

// 0x4B2664
int DB_FILE::fclose()
{
    return deleteRecord();
}

// 0x4AFD50
size_t DB_FILE::fread(void* ptr, size_t size, size_t count)
{
    int remaining_size;
    int chunk_size;
    size_t bytes_read;
    unsigned char* buf;
    size_t elements_read;
    size_t v1;

    buf = reinterpret_cast<unsigned char*>(ptr);
    elements_read = 0;

    if ((flags & 0x4) != 0) {
        if (read_callback != nullptr) {
            remaining_size = size * count;
            chunk_size = read_threshold - read_count;

            while (remaining_size >= chunk_size) {
                bytes_read = ::fread(buf, 1, chunk_size, uncompressed_file_stream);
                buf += bytes_read;
                remaining_size -= bytes_read;
                elements_read += bytes_read;

                read_count = 0;
                read_callback();

                chunk_size = read_threshold;
            }

            if (remaining_size != 0) {
                elements_read += ::fread(buf, 1, remaining_size, uncompressed_file_stream);
                read_count += remaining_size;
            }

            elements_read /= size;
        } else {
            elements_read = ::fread(buf, size, count, uncompressed_file_stream);
        }
    } else {
        if (ptr != nullptr) {
            switch (flags & 0xF0) {
            case 16:
                if (field_10 != 0) {
                    elements_read = field_10 / size;
                    if (elements_read > count) {
                        elements_read = count;
                    }

                    if (elements_read != 0) {
                        remaining_size = elements_read * size;
                        if (read_callback != nullptr) {
                            chunk_size = read_threshold - read_count;
                            while (remaining_size >= chunk_size) {
                                remaining_size -= chunk_size;
                                memcpy(buf, field_20, chunk_size);

                                buf += chunk_size;
                                field_20 += chunk_size;
                                field_10 -= chunk_size;

                                read_count = 0;
                                read_callback();

                                chunk_size = read_threshold;
                            }

                            if (remaining_size != 0) {
                                memcpy(buf, field_20, remaining_size);
                                field_20 += remaining_size;
                                field_10 -= remaining_size;
                                read_count += remaining_size;
                            }
                        } else {
                            memcpy(ptr, field_20, remaining_size);
                            field_20 += remaining_size;
                            field_10 -= remaining_size;
                        }
                    }
                }
                break;
            case 32:
                if (field_10 != 0) {
                    elements_read = field_10 / size;
                    if (elements_read > count) {
                        elements_read = count;
                    }

                    if (elements_read != 0) {
                        if (::fseek(database->stream, field_18, SEEK_SET) == 0) {
                            if (read_callback != nullptr) {
                                remaining_size = elements_read * size;
                                chunk_size = read_threshold - read_count;

                                // CE: Reuse `elements_read` to represent
                                // number of bytes read.
                                elements_read = 0;

                                while (remaining_size >= chunk_size) {
                                    bytes_read = ::fread(buf, 1, chunk_size, database->stream);
                                    buf += bytes_read;
                                    remaining_size -= bytes_read;
                                    elements_read += bytes_read;

                                    read_count = 0;
                                    read_callback();

                                    chunk_size = read_threshold;
                                }

                                if (remaining_size != 0) {
                                    elements_read += ::fread(buf, 1, remaining_size, database->stream);
                                    read_count += remaining_size;
                                }

                                field_18 = ::ftell(database->stream);
                                field_10 -= elements_read * size;

                                elements_read /= size;
                            } else {
                                elements_read = ::fread(buf, size, elements_read, database->stream);
                                field_18 = ::ftell(database->stream);
                                field_10 -= elements_read * size;
                            }
                        }
                    }
                }
                break;
            case 64:
                if (field_10 != 0) {
                    elements_read = field_10 / size;
                    if (elements_read > count) {
                        elements_read = count;
                    }

                    if (elements_read != 0) {
                        remaining_size = elements_read * size;
                        if (read_callback != nullptr) {
                            chunk_size = read_threshold - read_count;
                            while (remaining_size > chunk_size) {
                                preloadBuffer();

                                v1 = field_1C - (field_20 - 0x4000);
                                if (v1 > chunk_size) {
                                    v1 = chunk_size;
                                }

                                // FIXME: Copying same data twice.
                                memcpy(buf, field_20, v1);
                                memcpy(buf, field_20, v1);

                                field_20 += v1;
                                field_10 -= v1;

                                buf += v1;
                                remaining_size -= v1;

                                read_count += v1;
                                if (read_count >= read_threshold) {
                                    read_count = 0;
                                    read_callback();
                                }

                                chunk_size = read_threshold - read_count;
                            }

                            while (remaining_size != 0) {
                                preloadBuffer();

                                v1 = field_1C - (field_20 - 0x4000);
                                if (v1 > remaining_size) {
                                    v1 = remaining_size;
                                }

                                memcpy(buf, field_20, v1);

                                buf += v1;
                                remaining_size -= v1;

                                field_20 += v1;
                                field_10 -= v1;

                                read_count += v1;
                            }
                        } else {
                            while (remaining_size != 0) {
                                preloadBuffer();

                                v1 = field_1C - (field_20 - 0x4000);
                                if (v1 > remaining_size) {
                                    v1 = remaining_size;
                                }

                                memcpy(buf, field_20, v1);

                                buf += v1;
                                remaining_size -= v1;

                                field_20 += v1;
                                field_10 -= v1;
                            }
                        }
                    }
                }
                break;
            }
        }
    }

    return elements_read;
}

// 0x4B02A0
int DB_FILE::fgetc()
{
    int ch = -1;
    int next_ch;

    if ((flags & 0x4) != 0) {
        ch = ::fgetc(uncompressed_file_stream);
    } else {
        switch (flags & 0xF0) {
        case 16:
            if (field_10 != 0) {
                ch = *field_20;
                field_20++;
                field_10--;

                if (field_10 != 0 && (flags & 0x2) != 0 && ch == '\r') {
                    next_ch = *field_20;
                    if (next_ch == '\n') {
                        field_20++;
                        field_10--;
                        ch = '\n';
                    }
                }
            }
            break;
        case 32:
            if (field_10 != 0) {
                if (::fseek(database->stream, field_18, SEEK_SET) == 0) {
                    ch = ::fgetc(database->stream);
                    field_10 -= 1;

                    if (field_10 != 0 && (flags & 0x2) != 0 && ch == '\r') {
                        next_ch = ::fgetc(database->stream);
                        if (next_ch == '\n') {
                            field_10--;
                            ch = '\n';
                        } else {
                            ::ungetc(next_ch, database->stream);
                        }
                    }
                    field_18 = ::ftell(database->stream);
                }
            }
            break;
        case 64:
            preloadBuffer();

            if (field_10 != 0) {
                ch = *field_20;
                field_20++;
                field_10--;

                if (field_10 != 0 && (flags & 0x2) != 0 && ch == '\r') {
                    next_ch = *field_20;
                    if (next_ch == '\n') {
                        field_20++;
                        field_10--;
                        ch = '\n';
                    }
                }
            }
            break;
        }
    }

    if (read_callback != nullptr) {
        read_count++;
        if (read_count >= read_threshold) {
            read_callback();
            read_count = 0;
        }
    }

    return ch;
}

// 0x4B03F0
int DB_FILE::ungetc(int ch)
{
    if ((flags & 0x4) != 0) {
        return ::ungetc(ch, uncompressed_file_stream);
    } else {
        // NOTE: Original implementation looks broken, it does not return
        // `ch` into stream, but steps back in read stream.
        switch (flags & 0xF0) {
        case 16:
            if (field_20 != field_1C) {
                field_20--;
                field_10++;
            }
            break;
        case 32:
            if (field_18 != field_14) {
                if (::fseek(database->stream, field_18, SEEK_SET) == 0) {
                    if (::fseek(database->stream, -1, SEEK_CUR) == 0) {
                        field_18 = ::ftell(database->stream);
                        field_10++;
                    }
                }
            }
            break;
        case 64:
            if (field_20 != field_1C) {
                field_20--;
                field_10++;
            }
            break;
        }
    }

    return ch;
}

// 0x4B04A4
char* DB_FILE::fgets(char* string, size_t size)
{
    char* res = nullptr;
    size_t index;
    int ch;

    if ((flags & 0x4) != 0) {
        res = ::fgets(string, size, uncompressed_file_stream);
    } else {
        if (string != nullptr) {
            for (index = 0; index < size - 1; index++) {
                ch = fgetc();
                if (ch == -1) {
                    break;
                }

                string[index] = ch;

                if (ch == '\n') {
                    index++;
                    break;
                }
            }

            string[index] = '\0';

            if (index != 0) {
                res = string;
            }
        }
    }

    return res;
}

// 0x4B051C
int DB_FILE::fseek(long offset, int origin)
{
    int rc = -1;
    long current_offset;
    unsigned char* v1;
    int chunks;

    if ((flags & 0x4) != 0) {
        rc = ::fseek(uncompressed_file_stream, offset, origin);
    } else {
        current_offset = ftell();

        switch (origin) {
        case SEEK_SET:
            break;
        case SEEK_CUR:
            offset += current_offset;
            break;
        case SEEK_END:
            offset += field_C;
            break;
        default:
            offset = -1;
            break;
        }

        if (offset < 0 || offset > field_C) {
            return -1;
        }

        switch (flags & 0xF0) {
        case 16:
            field_20 = field_1C + offset;
            field_10 = field_C - offset;
            rc = 0;
            break;
        case 32:
            if (::fseek(database->stream, field_14 + offset, SEEK_SET) == 0) {
                field_18 = ::ftell(database->stream);
                field_10 = field_C - offset;
                rc = 0;
            }
            break;
        case 64:
            v1 = field_20 + offset - current_offset;
            if (v1 >= field_1C && v1 < field_1C + 0x4000) {
                field_20 = v1;
                field_10 = current_offset - offset;
                rc = 0;
            } else {
                if (offset < current_offset) {
                    rewind();
                    chunks = offset / 0x4000;
                } else {
                    field_10 -= field_1C - (field_20 - 0x4000);
                    field_20 = field_1C + 0x4000;
                    preloadBuffer();
                    chunks = (offset - ftell()) / 0x4000;
                }

                while (chunks > 0) {
                    field_10 -= 0x4000;
                    field_20 = field_1C + 0x4000;
                    preloadBuffer();
                    chunks--;
                }

                if (offset % 0x4000 != 0) {
                    field_10 -= offset % 0x4000;
                    field_20 += offset % 0x4000;
                }

                field_10 = field_C - offset;
            }
        }
    }

    return rc;
}

// 0x4B06A8
long DB_FILE::ftell()
{
    if ((flags & 0x4) != 0) {
        return ::ftell(uncompressed_file_stream);
    } else {
        switch (flags & 0xF0) {
        case 16:
            return field_C - field_10;
        case 32:
        case 64:
            return field_C - field_10;
        }
    }

    return -1;
}

// 0x4B06F4
void DB_FILE::rewind()
{
    if ((flags & 0x4) != 0) {
        ::rewind(uncompressed_file_stream);
    } else {
        switch (flags & 0xF0) {
        case 16:
            field_10 = field_C;
            field_20 = field_1C;
            break;
        case 32:
            field_18 = field_14;
            field_10 = field_C;
            break;
        case 64:
            field_10 = field_C;
            field_20 = field_1C + 16384;
            field_18 = field_14;
            preloadBuffer();
            break;
        }
    }
}

// 0x4B0764
size_t DB_FILE::fwrite(const void* buf, size_t size, size_t count)
{
    if ((flags & 0x4) != 0) {
        return ::fwrite(buf, size, count, uncompressed_file_stream);
    }

    return count - 1;
}

// 0x4B077C
int DB_FILE::fputc(int ch)
{
    if ((flags & 0x4) != 0) {
        return ::fputc(ch, uncompressed_file_stream);
    }

    return -1;
}

// 0x4B0794
int DB_FILE::fputs(const char* string)
{
    if ((flags & 0x4) != 0) {
        return ::fputs(string, uncompressed_file_stream);
    }

    return -1;
}

// 0x4B07AC
int DB_FILE::freadByte(unsigned char* c)
{
    int value = fgetc();
    if (value == -1) {
        return -1;
    }

    *c = value & 0xFF;

    return 0;
}

// 0x4B07C0
int DB_FILE::freadShort(unsigned short* s)
{
    unsigned char high;
    unsigned char low;

    // NOTE: Uninline.
    if (freadByte(&high) == -1) {
        return -1;
    }

    // NOTE: Uninline.
    if (freadByte(&low) == -1) {
        return -1;
    }

    *s = (high << 8) | low;

    return 0;
}

// 0x4B0820
int DB_FILE::freadInt(int* i)
{
    unsigned short high;
    unsigned short low;

    if (freadShort(&high) == -1) {
        return -1;
    }

    if (freadShort(&low) == -1) {
        return -1;
    }

    *i = (high << 16) | low;

    return 0;
}

// 0x4B0820
int DB_FILE::freadLong(unsigned long* l)
{
    int i;

    if (freadInt(&i) == -1) {
        return -1;
    }

    *l = static_cast<unsigned long>(i);

    return 0;
}

// 0x4B0820
int DB_FILE::freadFloat(float* q)
{
    unsigned long l;

    if (freadLong(&l) == -1) {
        return -1;
    }

    *q = *reinterpret_cast<float*>(&l);

    return 0;
}

// 0x4B0870
int DB_FILE::fwriteByte(unsigned char c)
{
    // NOTE: Uninline.
    if (fputc(c) == -1) {
        return -1;
    }

    return 0;
};

// 0x4B08A0
int DB_FILE::fwriteShort(unsigned short s)
{
    // NOTE: Uninline.
    if (fwriteByte(s >> 8) == -1) {
        return -1;
    }

    // NOTE: Uninline.
    if (fwriteByte(s & 0xFF) == -1) {
        return -1;
    }

    return 0;
}

// 0x4B08EC
int DB_FILE::fwriteInt(int i)
{
    if (fwriteShort(i >> 16) == -1) {
        return -1;
    }

    if (fwriteShort(i & 0xFFFF) == -1) {
        return -1;
    }

    return 0;
}

// 0x4C6244
int DB_FILE::fwriteLong(unsigned long l)
{
    // NOTE: Uninline.
    return fwriteInt(l);
}

// 0x4B099C
int DB_FILE::fwriteFloat(float q)
{
    // NOTE: Uninline.
    return fwriteLong(*reinterpret_cast<unsigned long*>(&q));
}

// 0x4B09D4
int DB_FILE::freadByteCount(unsigned char* c, int count)
{
    int index;
    unsigned char value;

    for (index = 0; index < count; index++) {
        // NOTE: Uninline.
        if (freadByte(&value) == -1) {
            return -1;
        }

        c[index] = value;
    }

    return 0;
}

// 0x4B0A14
int DB_FILE::freadShortCount(unsigned short* s, int count)
{
    int index;
    unsigned short value;

    for (index = 0; index < count; index++) {
        // NOTE: Uninline.
        if (freadShort(&value) == -1) {
            return -1;
        }

        s[index] = value;
    }

    return 0;
}

// 0x4B0AB0
int DB_FILE::freadIntCount(int* i, int count)
{
    int index;
    int value;

    for (index = 0; index < count; index++) {
        // NOTE: Uninline.
        if (freadInt(&value) == -1) {
            return -1;
        }

        i[index] = value;
    }

    return 0;
}

// 0x4B0AB0
int DB_FILE::freadLongCount(unsigned long* l, int count)
{
    int index;
    unsigned long value;

    for (index = 0; index < count; index++) {
        // NOTE: Uninline.
        if (freadLong(&value) == -1) {
            return -1;
        }

        l[index] = value;
    }

    return 0;
}

// 0x4B0AB0
int DB_FILE::freadFloatCount(float* q, int count)
{
    int index;
    float value;

    for (index = 0; index < count; index++) {
        // NOTE: Uninline.
        if (freadFloat(&value) == -1) {
            return -1;
        }

        q[index] = value;
    }

    return 0;
}

// 0x4B0B80
int DB_FILE::fwriteByteCount(unsigned char* c, int count)
{
    int index;

    for (index = 0; index < count; index++) {
        // NOTE: Uninline.
        if (fwriteByte(c[index]) == -1) {
            return -1;
        }
    }

    return 0;
}

// 0x4B0BC8
int DB_FILE::fwriteShortCount(unsigned short* s, int count)
{
    int index;

    for (index = 0; index < count; index++) {
        // NOTE: Uninline.
        if (fwriteShort(s[index]) == -1) {
            return -1;
        }
    }

    return 0;
}

// 0x4B0C3C
int DB_FILE::fwriteIntCount(int* i, int count)
{
    int index;

    for (index = 0; index < count; index++) {
        // NOTE: Uninline.
        if (fwriteInt(i[index]) == -1) {
            return -1;
        }
    }

    return 0;
}

// 0x4B0C9C
int DB_FILE::fwriteLongCount(unsigned long* l, int count)
{
    int index;

    for (index = 0; index < count; index++) {
        // NOTE: Uninline.
        if (fwriteLong(l[index]) == -1) {
            return -1;
        }
    }

    return 0;
}

// 0x4B0D54
int DB_FILE::fwriteFloatCount(float* q, int count)
{
    int index;

    for (index = 0; index < count; index++) {
        // NOTE: Uninline.
        if (fwriteFloat(q[index]) == -1) {
            return -1;
        }
    }

    return 0;
}

// 0x4B0D94
static int db_read_long(FILE* stream, int* value_ptr)
{
    int c;
    int value;

    c = fgetc(stream);
    if (c == -1) return -1;
    value = c;

    c = fgetc(stream);
    if (c == -1) return -1;
    value <<= 8;
    value |= c;

    c = fgetc(stream);
    if (c == -1) return -1;
    value <<= 8;
    value |= c;

    c = fgetc(stream);
    if (c == -1) return -1;
    value <<= 8;
    value |= c;

    *value_ptr = value;

    return 0;
}

// 0x4B0DF0
static int db_write_long(FILE* stream, int value)
{
    if (fputc(value >> 24, stream) == -1) return -1;
    if (fputc((value >> 16) & 0xFF, stream) == -1) return -1;
    if (fputc((value >> 8) & 0xFF, stream) == -1) return -1;
    if (fputc(value & 0xFF, stream) == -1) return -1;
    return 0;
}

// 0x4C5ED0
int DB_FILE::fprintf(const char* format, ...)
{
    int rc;
    va_list args;

    va_start(args, format);
    if ((flags & 0x4) != 0) {
        rc = vfprintf(uncompressed_file_stream, format, args);
    } else {
        rc = -1;
    }
    va_end(args);

    return rc;
}

// 0x4B0E98
int DB_FILE::feof()
{
    if ((flags & 0x4) != 0) {
        return ::feof(uncompressed_file_stream);
    } else {
        switch (flags & 0xF0) {
        case 16:
            return field_10 == 0;
        case 32:
        case 64:
            return field_10 == 0;
        }
    }

    return -1;
}

// 0x4B0EF0
int db_get_file_list(const char* filespec, char*** filelist, char*** desclist, int desclen)
{
    bool v1;
    char path[COMPAT_MAX_PATH];
    char* sep;
    char* filename;
    char* temp;
    assoc_array ary;
    int pos;
    int index;
    int count = 0;

    if (current_database == nullptr) {
        return 0;
    }

    if (filespec == nullptr) {
        return 0;
    }

    if (filelist == nullptr) {
        return 0;
    }

    char filespec_copy_buffer[COMPAT_MAX_PATH];
    char* filespec_copy = filespec_copy_buffer;
    strcpy(filespec_copy, filespec);

    temp = nullptr;

    v1 = true;
    if (filespec_copy[0] == '@') {
        filespec_copy++;
        v1 = false;
    }

    *filelist = nullptr;

    sep = strrchr(filespec_copy, '\\');
    filename = sep != nullptr ? sep + 1 : filespec_copy;

    if (strlen(filename) == 5 && filename[0] == '*' && filename[1] == '.') {
        if (desclist != nullptr) {
            temp = static_cast<char*>(internal_malloc(desclen));
            if (temp == nullptr) {
                return 0;
            }
        }

        if (ary.init(10, desclen) == -1) {
            if (temp != nullptr) {
                internal_free(temp);
            }
            return 0;
        }

        if (!v1) {
            strcpy(path, filespec_copy);
        }

        if (current_database->datafile != nullptr) {
            pos = 0;

            if (v1) {
                snprintf(path, sizeof(path), "%s%s", current_database->datafile_path, filespec_copy);
            }

            compat_strupr(path);

            sep = strrchr(path, '\\');
            if (sep != nullptr) {
                char* v3;

                *sep = '\0';

                v3 = path;
                if (path[0] == '.') {
                    v3 = path + 1;
                    if (path[1] == '\\') {
                        v3 = path + 2;
                    }
                }

                if (strlen(v3) != 0) {
                    pos = current_database->root.search(v3);
                } else {
                    pos = 0;
                }

                *sep = '\\';

                filename = sep + 1;
            } else {
                filename = path;
            }

            if (pos != -1) {
                char* name;
                size_t name_len;
                for (index = 0; index < current_database->entries[pos].getSize(); index++) {
                    name = current_database->entries[pos].getEntry(index).name;
                    name_len = strlen(name);
                    if (name_len > 4) {
                        if (name[name_len - 3] == filename[2] && name[name_len - 2] == filename[3] && name[name_len - 1] == filename[4]) {
                            if (temp != nullptr) {
                                DB_FILE* stream = db_fopen(name, "rb");
                                if (stream != nullptr) {
                                    if (stream->fgets(temp, desclen) != nullptr) {
                                        temp[strlen(temp) - 1] = '\0';
                                    }
                                    stream->fclose();
                                }
                            }
                            ary.insert(name, temp);
                        }
                    }
                }
            }
        }

        if (current_database->patches_path != nullptr) {
            DB_FIND_DATA find_data;

            if (v1) {
                snprintf(path, sizeof(path), "%s%s", current_database->patches_path, filespec_copy);
            }

            compat_windows_path_to_native(path);

            if (db_findfirst(path, &find_data) == 0) {
                do {
                    if (temp != nullptr) {
                        FILE* stream = compat_fopen(fileFindGetName(&find_data), "rb");
                        if (stream != nullptr) {
                            if (fgets(temp, desclen, stream) != nullptr) {
                                temp[strlen(temp - 1)] = '\0';
                            }
                            fclose(stream);
                        }
                    }
                    ary.insert(fileFindGetName(&find_data), temp);
                } while (db_findnext(&find_data) != -1);

                db_findclose(&find_data);
            }
        }

        count = ary.getSize();
        if (ary.getSize() > 0) {
            // Allocate one continous chunk of memory which is split into two
            // parts. The first part contains pointers (packed end-to-end and
            // thus allows indexed access) to the second part (which are actual
            // storage for strings 13 bytes each).
            //
            // NOTE: The size of storage is 33 bytes in Mac OS binary.
            *filelist = static_cast<char**>(internal_malloc((sizeof(char*) + 13) * ary.getSize()));
            if (*filelist != nullptr) {
                for (index = 0; index < ary.getSize(); index++) {
                    (*filelist)[index] = reinterpret_cast<char*>(*filelist) + sizeof(char*) * ary.getSize() + 13 * index;
                    strcpy((*filelist)[index], ary.getEntry(index).name);
                }
            }

            // TODO: Incomplete.
        }

        if (temp != nullptr) {
            internal_free(temp);
        }
    }

    return count;
}

// 0x4B1518
void db_free_file_list(char*** file_list, char*** desclist)
{
    if (file_list != nullptr) {
        if (*file_list != nullptr) {
            internal_free(*file_list);
            *file_list = nullptr;
        }
    }

    if (desclist != nullptr) {
        if (*desclist != nullptr) {
            internal_free(*desclist);
            *desclist = nullptr;
        }
    }
}

// NOTE: Originally not static.
//
// 0x4B1554
static int db_assoc_load_dir_entry(FILE* stream, void* buffer, size_t size, int flags)
{
    dir_entry* de;
    if (size != sizeof(*de)) return -1;

    de = static_cast<dir_entry*>(buffer);
    if (db_read_long(stream, &(de->flags)) != 0) return -1;
    if (db_read_long(stream, &(de->offset)) != 0) return -1;
    if (db_read_long(stream, &(de->length)) != 0) return -1;
    if (db_read_long(stream, &(de->field_C)) != 0) return -1;

    return 0;
}

// NOTE: Originally not static.
//
// 0x4B159C
static int db_assoc_save_dir_entry(FILE* stream, void* buffer, size_t size, int flags)
{
    dir_entry* de;

    if (size != sizeof(*de)) return -1;

    de = static_cast<dir_entry*>(buffer);
    if (db_write_long(stream, de->flags) != 0) return -1;
    if (db_write_long(stream, de->offset) != 0) return -1;
    if (db_write_long(stream, de->length) != 0) return -1;
    if (db_write_long(stream, de->field_C) != 0) return -1;

    return 0;
}

// NOTE: Originally not static.
//
// 0x4B15E4
static int db_assoc_load_db_dir_entry(DB_FILE* stream, void* buffer, size_t size, int flags)
{
    dir_entry* de;
    if (size != sizeof(*de)) return -1;

    de = static_cast<dir_entry*>(buffer);
    if (stream->freadInt32(&(de->flags)) != 0) return -1;
    if (stream->freadInt32(&(de->offset)) != 0) return -1;
    if (stream->freadInt32(&(de->length)) != 0) return -1;
    if (stream->freadInt32(&(de->field_C)) != 0) return -1;

    return 0;
}

// NOTE: Originally not static.
//
// 0x4B162C
static int db_assoc_save_db_dir_entry(DB_FILE* stream, void* buffer, size_t size, int flags)
{
    dir_entry* de;

    if (size != sizeof(*de)) return -1;

    de = static_cast<dir_entry*>(buffer);
    if (stream->fwriteInt32(de->flags) != 0) return -1;
    if (stream->fwriteInt32(de->offset) != 0) return -1;
    if (stream->fwriteInt32(de->length) != 0) return -1;
    if (stream->fwriteInt32(de->field_C) != 0) return -1;

    return 0;
}

// 0x4B1A98
long DB_FILE::filelength()
{
    if ((flags & 0x4) != 0) {
        return getFileSize(uncompressed_file_stream);
    } else {
        return field_C;
    }
}

// 0x4B1AC0
void db_register_mem(db_malloc_func* malloc_func, db_strdup_func* strdup_func, db_free_func* free_func)
{
    if (!db_used_malloc) {
        if (malloc_func != nullptr && strdup_func != nullptr && free_func != nullptr) {
            db_malloc = malloc_func;
            db_strdup = strdup_func;
            db_free = free_func;
        } else {
            db_malloc = db_default_malloc;
            db_strdup = db_default_strdup;
            db_free = db_default_free;
        }
    }
}

// 0x4B1B14
void db_register_callback(db_read_callback* callback, size_t threshold)
{
    if (callback != nullptr && threshold != 0) {
        read_callback = callback;
        read_threshold = threshold;
    } else {
        read_callback = nullptr;
        read_threshold = 0;
    }
}

// 0x4B1B2C
static int db_create_database(DB_DATABASE** database_ptr)
{
    int index;

    for (index = 0; index < DB_DATABASE_LIST_CAPACITY; index++) {
        if (database_list[index] == nullptr) {
            database_list[index] = static_cast<DB_DATABASE*>(internal_malloc(sizeof(DB_DATABASE)));
            if (database_list[index] == nullptr) {
                return -1;
            }

            memset(database_list[index], 0, sizeof(DB_DATABASE));
            *database_ptr = database_list[index];

            return 0;
        }
    }

    return -1;
}

// 0x4B1B98
static int db_destroy_database(DB_DATABASE** database_ptr)
{
    if (database_ptr == nullptr) {
        return -1;
    }

    if (*database_ptr == nullptr) {
        return -1;
    }

    db_free(*database_ptr);
    *database_ptr = nullptr;

    return 0;
}

// 0x4B1BC4
int DB_DATABASE::init_database(const char* datafile_arg, const char* datafile_path_arg)
{
    assoc_func_list funcs;
    int index;
    const char* v1;
    size_t v2;

    if (datafile_arg == nullptr) {
        return 0;
    }

    datafile = internal_strdup(datafile_arg);
    if (datafile == nullptr) {
        return -1;
    }

    stream = compat_fopen(datafile, "rb");
    if (stream == nullptr) {
        internal_free(datafile);
        datafile = nullptr;
        return -1;
    }

    if (root.init(0, sizeof(*entries)) != 0) {
        fclose(stream);
        internal_free(datafile);
        datafile = nullptr;
        return -1;
    }

    if (root.load(stream, 0) != 0) {
        fclose(stream);
        internal_free(datafile);
        datafile = nullptr;
        return -1;
    }

    entries = static_cast<assoc_array*>(internal_malloc(sizeof(*entries) * root.getSize()));
    if (entries == nullptr) {
        root.destroy();
        fclose(stream);
        internal_free(datafile);
        datafile = nullptr;
        return -1;
    }

    funcs.loadFunc = db_assoc_load_dir_entry;
    funcs.saveFunc = db_assoc_save_dir_entry;
    funcs.loadFuncDB = nullptr;
    funcs.saveFuncDB = nullptr;

    for (index = 0; index < root.getSize(); index++) {
        if (entries[index].init(0, sizeof(dir_entry), &funcs) != 0) {
            break;
        }

        if (entries[index].load(stream, 0) != 0) {
            break;
        }
    }

    if (index < root.getSize()) {
        while (--index >= 0) {
            entries[index].destroy();
        }

        internal_free(entries);
        root.destroy();
        fclose(stream);
        internal_free(datafile);
        datafile = nullptr;
        return -1;
    }

    if (datafile_path_arg != nullptr && strlen(datafile_path_arg) != 0) {
        v1 = datafile_path_arg;
        if (datafile_path_arg[0] == PATH_SEP) {
            v1 = datafile_path_arg + 1;
        }
    } else {
        v1 = ".\\";
    }

    v2 = strlen(v1);
    datafile_path = static_cast<char*>(internal_malloc(v2 + 2));
    if (datafile_path == nullptr) {
        internal_free(entries);
        root.destroy();
        fclose(stream);
        internal_free(datafile);
        datafile = nullptr;
        return -1;
    }

    strcpy(datafile_path, v1);

    if (datafile_path[v2 - 1] != '\\') {
        datafile_path[v2] = '\\';
        datafile_path[v2 + 1] = '\0';
    }

    return 0;
}

// 0x4B1DE0
void DB_DATABASE::exit_database()
{
    int index;

    if (stream != nullptr) {
        fclose(stream);
        stream = nullptr;
    }

    if (datafile != nullptr) {
        internal_free(datafile);
        datafile = nullptr;
    }

    if (entries != nullptr) {
        for (index = 0; index < root.getSize(); index++) {
            entries[index].destroy();
        }
        internal_free(entries);
        entries = nullptr;
    }

    root.destroy();

    if (datafile_path != nullptr) {
        internal_free(datafile_path);
        datafile_path = nullptr;
    }
}

// 0x4B1E70
int DB_DATABASE::init_patches(const char* path)
{
    size_t path_len;

    if (path == nullptr) {
        patches_path = nullptr;
        return 0;
    }

    path_len = strlen(path);
    if (path_len == 0) {
        patches_path = empty_patches_path;
        return 0;
    }

    patches_path = static_cast<char*>(internal_malloc(path_len + 2));
    if (patches_path == nullptr) {
        return -1;
    }

    should_free_patches_path = true;
    strcpy(patches_path, path);

    if (patches_path[path_len - 1] != '\\') {
        patches_path[path_len] = PATH_SEP;
        patches_path[path_len + 1] = '\0';
    }

    return 0;
}

// 0x4B1F10
void DB_DATABASE::exit_patches()
{
    if (patches_path != nullptr) {
        if (should_free_patches_path == true) {
            internal_free(patches_path);
        }
    }

    patches_path = empty_patches_path;
    should_free_patches_path = false;
}

// 0x4B1F3C
int DB_DATABASE::init_hash_table()
{
    if (!hash_is_on) {
        return -1;
    }

    hash_table = static_cast<unsigned char*>(internal_malloc(DB_HASH_TABLE_SIZE));
    if (hash_table == nullptr) {
        return -1;
    }

    return reset_hash_table();
}

// 0x4B1F90
void db_enable_hash_table()
{
    hash_is_on = true;
}

// 0x4B1F9C
int DB_DATABASE::reset_hash_table()
{
    if (!hash_is_on) {
        return -1;
    }

    if (patches_path == nullptr) {
        return -1;
    }

    if (hash_table == nullptr) {
        hash_table = static_cast<unsigned char*>(internal_malloc(DB_HASH_TABLE_SIZE));
        if (hash_table == nullptr) {
            return -1;
        }
    }

    memset(hash_table, 0, DB_HASH_TABLE_SIZE);

    return fill_hash_table(patches_path);
}

// NOTE: Originally not static, but that would require exposing `DB_DATABASE`
// which is most likely considered implementation detail.
//
// 0x4B2028
int DB_DATABASE::fill_hash_table(const char* path)
{
    char pattern[COMPAT_MAX_PATH];
    DB_FIND_DATA find_data;
    bool is_directory;
    char* filename;

    if (!hash_is_on) {
        return -1;
    }

    if (hash_table == nullptr) {
        return -1;
    }

#if defined(_WIN32)
    snprintf(pattern, sizeof(pattern), "%s%s", path, "*.*");
#else
    snprintf(pattern, sizeof(pattern), "%s%s", path, "*");
#endif
    compat_windows_path_to_native(pattern);

    if (db_findfirst(pattern, &find_data) != -1) {
        do {
            is_directory = fileFindIsDirectory(&find_data);
            filename = fileFindGetName(&find_data);

            if (is_directory) {
                if (strcmp(filename, ".") != 0 && strcmp(filename, "..") != 0) {
                    snprintf(pattern, sizeof(pattern), "%s%s%c", path, filename, PATH_SEP);
                    fill_hash_table(pattern);
                }
            } else {
                add_hash_entry(filename, PATH_SEP);
            }
        } while (db_findnext(&find_data) != -1);

        db_findclose(&find_data);
    }

    return 0;
}

// 0x4B2154
int db_reset_hash_tables()
{
    int index;

    if (!hash_is_on) {
        return -1;
    }

    for (index = 0; index < DB_DATABASE_LIST_CAPACITY; index++) {
        if (database_list[index] != nullptr) {
            database_list[index]->reset_hash_table();
        }
    }

    return 0;
}

// 0x4B218C
int db_add_hash_entry(const char* path, int sep)
{
    if (!hash_is_on) {
        return -1;
    }

    if (current_database == nullptr) {
        return -1;
    }

    if (current_database->hash_table == nullptr) {
        return -1;
    }

    if (path == nullptr) {
        return -1;
    }

    return current_database->add_hash_entry(path, sep);
}

// 0x4B21E0
int DB_DATABASE::add_hash_entry(const char* path, int sep)
{
    unsigned int key;

    if (!hash_is_on) {
        return -1;
    }

    if (hash_table == nullptr) {
        return -1;
    }

    if (db_hash_string_to_key(path, sep, &key) != 0) {
        return -1;
    }

    return set_hash_value(key, 1);
}

// 0x4B2258
int DB_DATABASE::set_hash_value(unsigned int key, unsigned char enabled)
{
    if (!hash_is_on) {
        return -1;
    }

    if (hash_table == nullptr) {
        return -1;
    }

    if (key >= 0x7FFF) {
        return -1;
    }

    if (enabled == true) {
        hash_table[key / 8] |= 1 << (key % 8);
    } else {
        hash_table[key / 8] = 0;
    }

    return 0;
}

// 0x4B2304
int DB_DATABASE::get_hash_value(const char* path, int sep, int* value_ptr)
{
    unsigned int key;

    if (!hash_is_on) {
        return -1;
    }

    if (hash_table == nullptr) {
        return -1;
    }

    if (path == nullptr) {
        return -1;
    }

    if (db_hash_string_to_key(path, sep, &key) != 0) {
        return -1;
    }

    if (key >= 0x7FFF) {
        return -1;
    }

    *value_ptr = (hash_table[key / 8] >> (key % 8)) & 1;

    return 0;
}

// 0x4B2394
static int db_hash_string_to_key(const char* path, int sep, unsigned int* key_ptr)
{
    char* copy;
    char* pch;
    char* filename;
    size_t index;
    unsigned int key;

    key = 1;

    if (path == nullptr) {
        return -1;
    }

    copy = reinterpret_cast<char*>(internal_strdup(path));
    compat_strupr(copy);

    pch = strrchr(copy, sep);
    if (pch != nullptr) {
        filename = pch + 1;
    } else {
        filename = copy;
    }

    for (index = 0; index < strlen(filename); index++) {
        key *= *filename++;
        key &= 0x7FFFFFFF;
    }

    *key_ptr = key & 0x7FFF;

    internal_free(copy);

    return 0;
}

// 0x4B2420
void DB_DATABASE::exit_hash_table()
{
    if (hash_table != nullptr) {
        internal_free(hash_table);
    }
    hash_table = nullptr;
}

// 0x4B2444
static DB_FILE* db_add_fp_rec(FILE* stream, unsigned char* a2, int a3, int flags)
{
    DB_FILE* ptr;
    int pos;

    ptr = nullptr;
    if (current_database->files_length < DB_DATABASE_FILE_LIST_CAPACITY) {
        if (db_find_empty_position(&pos) == 0) {
            memset(&(current_database->files[pos]), 0, sizeof(*current_database->files));
            current_database->files[pos].database = current_database;

            if ((flags & 0x4) != 0) {
                current_database->files[pos].uncompressed_file_stream = stream;
                ptr = &(current_database->files[pos]);
            } else {
                current_database->files[pos].field_C = a3;
                current_database->files[pos].field_10 = a3;

                switch (flags & 0xF0) {
                case 16:
                    current_database->files[pos].field_1C = a2;
                    current_database->files[pos].field_20 = a2;
                    ptr = &(current_database->files[pos]);
                    break;
                case 32:
                    current_database->files[pos].field_14 = ftell(stream);
                    current_database->files[pos].field_18 = ftell(stream);
                    ptr = &(current_database->files[pos]);
                    break;
                case 64:
                    current_database->files[pos].field_14 = ftell(stream);
                    current_database->files[pos].field_18 = ftell(stream);
                    current_database->files[pos].field_1C = a2;
                    current_database->files[pos].field_20 = a2 + 0x4000;
                    ptr = &(current_database->files[pos]);
                    break;
                }
            }
        }
    }

    if (ptr != nullptr) {
        current_database->files[pos].flags = flags;
        current_database->files[pos].field_8 = 1;
        current_database->files_length++;
    }

    return ptr;
}

// 0x4B2664
int DB_FILE::deleteRecord()
{
    if ((flags & 0x4) != 0) {
        ::fclose(uncompressed_file_stream);
    } else {
        switch (flags & 0xF0) {
        case 16:
            if (field_1C != nullptr) {
                internal_free(field_1C);
            }
            break;
        case 32:
            break;
        case 64:
            if (field_1C != nullptr) {
                internal_free(field_1C);
            }
            break;
        }
    }

    database->files_length -= 1;
    memset(this, 0, sizeof(*this));

    return 0;
}

// 0x4B26D0
static int db_find_empty_position(int* position_ptr)
{
    int index;

    if (position_ptr == nullptr) {
        return -1;
    }

    if (current_database->files_length >= DB_DATABASE_FILE_LIST_CAPACITY) {
        return -1;
    }

    for (index = 0; index < DB_DATABASE_FILE_LIST_CAPACITY; index++) {
        if (current_database->files[index].field_8 == 0) {
            *position_ptr = index;
            return 0;
        }
    }

    return -1;
}

// 0x4B2714
static int db_find_dir_entry(char* path, dir_entry* de)
{
    char* normalized_path;
    int pos;
    int dir_index;
    int entry_index;

    normalized_path = path;

    if (current_database->datafile == nullptr) {
        return -1;
    }

    if (path == nullptr) {
        return -1;
    }

    if (de == nullptr) {
        return -1;
    }

    if (path[0] == '.') {
        normalized_path = path + 1;
        if (path[1] == '\\') {
            normalized_path = path + 2;
        }
    }

    pos = strlen(normalized_path) - 1;
    while (pos >= 0) {
        if (normalized_path[pos] == '\\') {
            break;
        }
        pos--;
    }

    if (pos >= 0) {
        normalized_path[pos] = '\0';
        dir_index = current_database->root.search(normalized_path);
    } else {
        dir_index = 0;
    }

    if (dir_index == -1) {
        if (pos >= 0) {
            normalized_path[pos] = '\\';
        }
        return -1;
    }

    entry_index = current_database->entries[dir_index].search(normalized_path + pos + 1);
    if (entry_index == -1) {
        if (pos >= 0) {
            normalized_path[pos] = '\\';
        }
        return -1;
    }

    if (pos >= 0) {
        normalized_path[pos] = '\\';
    }

    *de = *(static_cast<dir_entry*>(current_database->entries[dir_index].getEntry(entry_index).data));

    return 0;
}

// 0x4B2810
static int db_findfirst(const char* path, DB_FIND_DATA* findData)
{
#if defined(_WIN32)
    findData->hFind = FindFirstFileA(path, &(findData->ffd));
    if (findData->hFind == INVALID_HANDLE_VALUE) {
        return -1;
    }
#else
    strcpy(findData->path, path);

    char drive[COMPAT_MAX_DRIVE];
    char dir[COMPAT_MAX_DIR];
    compat_splitpath(path, drive, dir, nullptr, nullptr);

    char basePath[COMPAT_MAX_PATH];
    compat_makepath(basePath, drive, dir, nullptr, nullptr);

    compat_resolve_path(basePath);
    findData->dir = opendir(basePath);
    if (findData->dir == nullptr) {
        return -1;
    }

    findData->entry = readdir(findData->dir);
    while (findData->entry != nullptr) {
        char entryPath[COMPAT_MAX_PATH];
        compat_makepath(entryPath, drive, dir, fileFindGetName(findData), nullptr);
        if (fpattern_match(findData->path, entryPath)) {
            break;
        }
        findData->entry = readdir(findData->dir);
    }

    if (findData->entry == nullptr) {
        closedir(findData->dir);
        findData->dir = nullptr;
        return -1;
    }
#endif

    return 0;
}

// 0x4B2838
static int db_findnext(DB_FIND_DATA* findData)
{
#if defined(_WIN32)
    if (!FindNextFileA(findData->hFind, &(findData->ffd))) {
        return -1;
    }
#else
    char drive[COMPAT_MAX_DRIVE];
    char dir[COMPAT_MAX_DIR];
    compat_splitpath(findData->path, drive, dir, nullptr, nullptr);

    findData->entry = readdir(findData->dir);
    while (findData->entry != nullptr) {
        char entryPath[COMPAT_MAX_PATH];
        compat_makepath(entryPath, drive, dir, fileFindGetName(findData), nullptr);
        if (fpattern_match(findData->path, entryPath)) {
            break;
        }
        findData->entry = readdir(findData->dir);
    }

    if (findData->entry == nullptr) {
        closedir(findData->dir);
        findData->dir = nullptr;
        return -1;
    }
#endif

    return 0;
}

// 0x4B2854
static int db_findclose(DB_FIND_DATA* findData)
{
#if defined(_WIN32)
    if (!FindClose(findData->hFind)) {
        return -1;
    }
#else
    if (findData->dir != nullptr) {
        if (closedir(findData->dir) != 0) {
            return -1;
        }
    }
#endif

    return 0;
}

// 0x4B2868
static void* internal_malloc(size_t size)
{
    db_used_malloc = true;
    return db_malloc(size);
}

// 0x4B287C
static char* internal_strdup(const char* string)
{
    db_used_malloc = true;
    return db_strdup(string);
}

// 0x4B2890
static void internal_free(void* ptr)
{
    db_free(ptr);
}

// 0x4B2898
static void* db_default_malloc(size_t size)
{
    return malloc(size);
}

// 0x4B28A0
static char* db_default_strdup(const char* string)
{
    return compat_strdup(string);
}

// 0x4B28A8
static void db_default_free(void* ptr)
{
    free(ptr);
}

// 0x4B28B0
void DB_FILE::preloadBuffer()
{
    unsigned short v1;

    if ((flags & 0x8) != 0 && (flags & 0xF0) == 64) {
        if (field_10 != 0) {
            if (field_20 >= field_1C + 0x4000) {
                if (::fseek(database->stream, field_18, SEEK_SET) == 0) {
                    if (fread_short(database->stream, &v1) == 0) {
                        if ((v1 & 0x8000) != 0) {
                            v1 &= ~0x8000;
                            ::fread(field_1C, 1, v1, database->stream);
                        } else {
                            lzss_decode_to_buf(database->stream, field_1C, v1);
                        }

                        field_20 = field_1C;
                        field_18 = ::ftell(database->stream);
                    }
                }
            }
        }
    }
}

// 0x4B2970
static int fread_short(FILE* stream, unsigned short* s)
{
    int high;
    int low;

    high = fgetc(stream);
    if (high == -1) {
        return -1;
    }

    low = fgetc(stream);
    if (low == -1) {
        return -1;
    }

    *s = (low & 0xFF) | (high << 8);

    return 0;
}

static inline bool fileFindIsDirectory(DB_FIND_DATA* findData)
{
#if defined(_WIN32)
    return (findData->ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
#elif defined(__WATCOMC__)
    return (findData->entry->d_attr & _A_SUBDIR) != 0;
#else
    return findData->entry->d_type == DT_DIR;
#endif
}

static inline char* fileFindGetName(DB_FIND_DATA* findData)
{
#if defined(_WIN32)
    return findData->ffd.cFileName;
#else
    return findData->entry->d_name;
#endif
}

int DB_FILE::freadUInt8(unsigned char* valuePtr)
{
    int value = fgetc();
    if (value == -1) {
        return -1;
    }

    *valuePtr = static_cast<unsigned char>(value);

    return 0;
}

int DB_FILE::freadInt8(char* valuePtr)
{
    unsigned char value;
    if (freadUInt8(&value) == -1) {
        return -1;
    }

    *valuePtr = static_cast<char>(value);

    return 0;
}

int DB_FILE::freadUInt16(unsigned short* valuePtr)
{
    return freadShort(valuePtr);
}

int DB_FILE::freadInt16(short* valuePtr)
{
    unsigned short value;
    if (freadUInt16(&value) == -1) {
        return -1;
    }

    *valuePtr = static_cast<short>(value);

    return 0;
}

int DB_FILE::freadUInt32(unsigned int* valuePtr)
{
    int value;
    if (freadInt(&value) == -1) {
        return -1;
    }

    *valuePtr = static_cast<unsigned int>(value);

    return 0;
}

int DB_FILE::freadInt32(int* valuePtr)
{
    unsigned int value;
    if (freadUInt32(&value) == -1) {
        return -1;
    }

    *valuePtr = static_cast<int>(value);

    return 0;
}

int DB_FILE::freadUInt8List(unsigned char* arr, int count)
{
    for (int index = 0; index < count; index++) {
        if (freadUInt8(&(arr[index])) == -1) {
            return -1;
        }
    }

    return 0;
}

int DB_FILE::freadInt8List(char* arr, int count)
{
    for (int index = 0; index < count; index++) {
        if (freadInt8(&(arr[index])) == -1) {
            return -1;
        }
    }

    return 0;
}

int DB_FILE::freadInt16List(short* arr, int count)
{
    for (int index = 0; index < count; index++) {
        if (freadInt16(&(arr[index])) == -1) {
            return -1;
        }
    }

    return 0;
}

int DB_FILE::freadInt32List(int* arr, int count)
{
    for (int index = 0; index < count; index++) {
        if (freadInt32(&(arr[index])) == -1) {
            return -1;
        }
    }

    return 0;
}

int DB_FILE::freadBool(bool* valuePtr)
{
    int value;
    if (freadInt32(&value) == -1) {
        return -1;
    }

    *valuePtr = (value != 0);

    return 0;
}

int DB_FILE::fwriteUInt8(unsigned char value)
{
    return fputc(static_cast<int>(value));
}

int DB_FILE::fwriteInt8(char value)
{
    return fwriteUInt8(static_cast<unsigned char>(value));
}

int DB_FILE::fwriteUInt16(unsigned short value)
{
    return fwriteShort(value);
}

int DB_FILE::fwriteInt16(short value)
{
    return fwriteUInt16(static_cast<unsigned short>(value));
}

int DB_FILE::fwriteUInt32(unsigned int value)
{
    return fwriteInt(static_cast<int>(value));
}

int DB_FILE::fwriteInt32(int value)
{
    return fwriteUInt32(static_cast<unsigned int>(value));
}

int DB_FILE::fwriteUInt8List(unsigned char* arr, int count)
{
    for (int index = 0; index < count; index++) {
        if (fwriteUInt8(arr[index]) == -1) {
            return -1;
        }
    }

    return 0;
}

int DB_FILE::fwriteInt8List(char* arr, int count)
{
    for (int index = 0; index < count; index++) {
        if (fwriteInt8(arr[index]) == -1) {
            return -1;
        }
    }

    return 0;
}

int DB_FILE::fwriteInt16List(short* arr, int count)
{
    for (int index = 0; index < count; index++) {
        if (fwriteInt16(arr[index]) == -1) {
            return -1;
        }
    }

    return 0;
}

int DB_FILE::fwriteInt32List(int* arr, int count)
{
    for (int index = 0; index < count; index++) {
        if (fwriteInt32(arr[index]) == -1) {
            return -1;
        }
    }

    return 0;
}

int DB_FILE::fwriteBool(bool value)
{
    return fwriteInt32(value ? 1 : 0);
}

} // namespace fallout

#include "game/message_helpers.h"

#include <cstdio>
#include <cstdlib>

#include "game/display.h"
#include "game/scripts.h"
#include "plib/gnw/debug.h"

namespace fallout {

MessageListItem nullMessageListItem = {
    .num = -1,
    .audio = nullptr,
    .text = nullptr,
};

CharBufferPtr make_char_buffer(size_t size)
{
    return std::make_unique<char[]>(size);
}

MessageListItem find_message_or_debug_print(
    MessageList& messageList,
    int messageListItemNum)
{
    return find_message_or_else(messageList, messageListItemNum, []() {
        debug_printf("\nError: Can't find msg num!");
    });
}

MessageListItem find_message_or_panic(
    MessageList& messageList,
    int messageListItemNum)
{
    return find_message_or_else(messageList, messageListItemNum, []() {
        debug_printf("\nError: Can't find msg num!");
        exit(1);
    });
}

void display_message(MessageList& messageList, int messageId)
{
    MessageListItem item = find_message_or_debug_print(messageList, messageId);
    if (is_valid_message(item)) {
        display_print(item.text);
    }
}

void display_formatted_message(MessageList& messageList, int messageId, const char* arg)
{
    MessageListItem item = find_message_or_debug_print(messageList, messageId);
    if (is_valid_message(item)) {
        char formattedText[260];
        snprintf(formattedText, sizeof(formattedText), item.text, arg);
        display_print(formattedText);
    }
}

ScriptOverrideResult try_script_override(
    Object* target,
    Object* source,
    int scriptProc,
    bool* errOut)
{
    int sid = target->sid;
    if (sid == -1) {
        return { false, -1 };
    }

    scr_set_objs(sid, source, target);
    exec_script_proc(sid, scriptProc);

    Script* script;
    if (scr_ptr(sid, &script) == -1) {
        if (errOut) *errOut = true;
        return { false, -1 };
    }

    return { script->scriptOverrides != 0, sid };
}

} // namespace fallout

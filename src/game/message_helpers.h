#pragma once


#include <memory>

#include "game/message.h"
#include "game/object_types.h"

namespace fallout {

// Sentinel returned when a message lookup fails.
extern MessageListItem nullMessageListItem;

using CharBufferPtr = std::unique_ptr<char[]>;

CharBufferPtr make_char_buffer(size_t size);

inline bool is_valid_message(const MessageListItem& item)
{
    return item.text != nullptr && item.num != -1;
}

// Searches for a message; on failure calls the callback and returns nullMessageListItem.
template <typename Callback>
MessageListItem find_message_or_else(
    MessageList& messageList,
    int messageListItemNum,
    Callback callback)
{
    MessageListItem messageListItem = {
        .num = messageListItemNum,
        .audio = nullptr,
        .text = nullptr,
    };
    if (!messageList.search(&messageListItem)) {
        callback();
        return nullMessageListItem;
    }
    return messageListItem;
}

// Searches for a message; on failure prints a debug error.
MessageListItem find_message_or_debug_print(MessageList& messageList, int messageListItemNum);

// Searches for a message; on failure prints a debug error and terminates.
MessageListItem find_message_or_panic(MessageList& messageList, int messageListItemNum);

// Convenience: find message + validate + display_print. Does nothing if the message is not found.
void display_message(MessageList& messageList, int messageId);

// Convenience: find message + validate + snprintf with one arg + display_print.
void display_formatted_message(MessageList& messageList, int messageId, const char* arg);

// --------------------------------------------------------------------------
// Script-override helper
// --------------------------------------------------------------------------

struct ScriptOverrideResult {
    bool overridden;
    int sid;
};

// Attempts to run a script proc on the target object. Returns whether the
// script overrode the default behavior, plus the resolved sid.
// If the target has no script, returns {false, -1}.
// On internal error (scr_ptr failure), returns {false, -1} with errOut set if non-null.
ScriptOverrideResult try_script_override(
    Object* target,
    Object* source,
    int scriptProc,
    bool* errOut = nullptr);

} // namespace fallout

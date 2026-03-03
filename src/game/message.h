#pragma once

#include <cstddef>

namespace fallout {

// TODO: Probably should be private.
inline constexpr int MESSAGE_LIST_ITEM_FIELD_MAX_SIZE = 1024;

struct MessageListItem {
    int num;
    char* audio;
    char* text;
};

// A sorted collection of numbered message entries loaded from .msg files.
class MessageList {
public:
    MessageList() = default;
    ~MessageList() = default;

    [[nodiscard]] bool init();
    bool exit();
    [[nodiscard]] bool load(const char* path);
    [[nodiscard]] bool search(MessageListItem* entry);
    [[nodiscard]] char* getMessage(MessageListItem* entry, int num);
    [[nodiscard]] bool filter();

    bool isEmpty() const { return entries_num_ == 0; }

private:
    int entries_num_ = 0;
    MessageListItem* entries_ = nullptr;

    bool find(int num, int* out_index);
    bool add(MessageListItem* new_entry);
};

int init_message();
void exit_message();
bool message_make_path(char* dest, size_t size, const char* path);

} // namespace fallout

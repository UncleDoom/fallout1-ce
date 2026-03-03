#pragma once

#include <cstddef>

namespace fallout {

struct HeapHandle {
    unsigned int state;
    unsigned char* data;
};

// ---------------------------------------------------------------------------
// Heap — block allocator with compaction and handle-based access
// ---------------------------------------------------------------------------
class Heap {
public:
    Heap() = default;
    ~Heap() = default;

    Heap(const Heap&) = delete;
    Heap& operator=(const Heap&) = delete;

    [[nodiscard]] bool init(int initialSize);
    bool exit();
    [[nodiscard]] bool allocate(int* handleIndexPtr, int size, int flags);
    [[nodiscard]] bool deallocate(int* handleIndexPtr);
    [[nodiscard]] bool lock(int handleIndex, unsigned char** bufferPtr);
    [[nodiscard]] bool unlock(int handleIndex);
    [[nodiscard]] bool stats(char* dest, size_t size);
    [[nodiscard]] bool validate();

private:
    bool initHandles();
    bool exitHandles();
    bool acquireHandle(int* handleIndexPtr);
    bool releaseHandle(int handleIndex);
    bool clearHandles(HeapHandle* handles, unsigned int count);
    bool findFreeBlock(int size, void** blockPtr, int flags);
    bool buildFreeList();
    bool sortFreeList();
    bool buildMoveableList(int* moveableExtentsLengthPtr, int* maxBlocksLengthPtr);

    int size_ = 0;
    int freeBlocks_ = 0;
    int moveableBlocks_ = 0;
    int lockedBlocks_ = 0;
    int systemBlocks_ = 0;
    int handlesLength_ = 0;
    int freeSize_ = 0;
    int moveableSize_ = 0;
    int lockedSize_ = 0;
    int systemSize_ = 0;
    HeapHandle* handles_ = nullptr;
    unsigned char* data_ = nullptr;
};

// Legacy free-function wrappers (delegate to Heap member functions)
bool heap_init(Heap* heap, int a2);
bool heap_exit(Heap* heap);
bool heap_allocate(Heap* heap, int* handleIndexPtr, int size, int a3);
bool heap_deallocate(Heap* heap, int* handleIndexPtr);
bool heap_lock(Heap* heap, int handleIndex, unsigned char** bufferPtr);
bool heap_unlock(Heap* heap, int handleIndex);
bool heap_stats(Heap* heap, char* dest, size_t size);
bool heap_validate(Heap* heap);

} // namespace fallout

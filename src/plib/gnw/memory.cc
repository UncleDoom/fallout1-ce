#include "plib/gnw/memory.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "plib/gnw/debug.h"
#include "plib/gnw/gnw.h"

namespace fallout {

// Guard constants written before/after every allocation to detect corruption.
static constexpr unsigned int kHeaderGuard = 0xFEEDFACE;
static constexpr unsigned int kFooterGuard = 0xBEEFCAFE;

// A header prepended to every memory block.
struct MemoryBlockHeader {
    size_t size;          // Total size including header + footer.
    unsigned int guard;   // Must equal kHeaderGuard.
};

// A footer appended to every memory block.
struct MemoryBlockFooter {
    unsigned int guard;   // Must equal kFooterGuard.
};

// ---------------------------------------------------------------------------
// MemoryManager implementation
// ---------------------------------------------------------------------------

MemoryManager::MemoryManager() noexcept = default;

MemoryManager& MemoryManager::instance() noexcept
{
    static MemoryManager inst;
    return inst;
}

// 0x4AEBE0
char* MemoryManager::strdup(const char* string)
{
    if (string == nullptr) {
        return nullptr;
    }

    auto* copy = static_cast<char*>(malloc_(std::strlen(string) + 1));
    if (copy != nullptr) {
        std::strcpy(copy, string);
    }
    return copy;
}

// 0x4AEC30
void* MemoryManager::malloc(size_t size)
{
    return malloc_(size);
}

// 0x4AECB0
void* MemoryManager::realloc(void* ptr, size_t size)
{
    return realloc_(ptr, size);
}

// 0x4AED84
void MemoryManager::free(void* ptr)
{
    free_(ptr);
}

// 0x4AEDBC
void MemoryManager::printStats() const
{
    if (malloc_ == defaultMalloc) {
        debug_printf("Current memory allocated: %6d blocks, %9u bytes total\n",
                     numBlocks_, static_cast<unsigned>(memAllocated_));
        debug_printf("Max memory allocated:     %6d blocks, %9u bytes total\n",
                     maxBlocks_, static_cast<unsigned>(maxAllocated_));
    }
}

// 0x4AEE08
void MemoryManager::registerAllocator(MallocFunc* mallocFunc, ReallocFunc* reallocFunc, FreeFunc* freeFunc)
{
    if (!GNW_win_init_flag) {
        malloc_ = mallocFunc;
        realloc_ = reallocFunc;
        free_ = freeFunc;
    }
}

// -- Default allocator with guard instrumentation ---------------------------

// 0x4AEC38
void* MemoryManager::defaultMalloc(size_t size)
{
    auto& mgr = instance();

    if (size == 0) {
        return nullptr;
    }

    size += sizeof(MemoryBlockHeader) + sizeof(MemoryBlockFooter);
    size += sizeof(int) - size % sizeof(int);

    auto* block = static_cast<unsigned char*>(std::malloc(size));
    if (block == nullptr) {
        return nullptr;
    }

    void* ptr = prepBlock(block, size);

    mgr.numBlocks_++;
    if (mgr.numBlocks_ > mgr.maxBlocks_) {
        mgr.maxBlocks_ = mgr.numBlocks_;
    }

    mgr.memAllocated_ += size;
    if (mgr.memAllocated_ > mgr.maxAllocated_) {
        mgr.maxAllocated_ = mgr.memAllocated_;
    }

    return ptr;
}

// 0x4AECB8
void* MemoryManager::defaultRealloc(void* ptr, size_t size)
{
    auto& mgr = instance();

    if (ptr != nullptr) {
        auto* block = static_cast<unsigned char*>(ptr) - sizeof(MemoryBlockHeader);
        auto* header = reinterpret_cast<MemoryBlockHeader*>(block);
        size_t oldSize = header->size;

        mgr.memAllocated_ -= oldSize;
        checkBlock(block);

        if (size != 0) {
            size += sizeof(MemoryBlockHeader) + sizeof(MemoryBlockFooter);
            size += sizeof(int) - size % sizeof(int);
        }

        auto* newBlock = static_cast<unsigned char*>(std::realloc(block, size));
        if (newBlock != nullptr) {
            mgr.memAllocated_ += size;
            if (mgr.memAllocated_ > mgr.maxAllocated_) {
                mgr.maxAllocated_ = mgr.memAllocated_;
            }
            ptr = prepBlock(newBlock, size);
        } else {
            if (size != 0) {
                mgr.memAllocated_ += oldSize;
                debug_printf("%s,%u: ", __FILE__, __LINE__);
                debug_printf("Realloc failure.\n");
            } else {
                mgr.numBlocks_--;
            }
            ptr = nullptr;
        }
    } else {
        ptr = mgr.malloc_(size);
    }

    return ptr;
}

// 0x4AED8C
void MemoryManager::defaultFree(void* ptr)
{
    if (ptr == nullptr) {
        return;
    }

    auto& mgr = instance();
    auto* block = static_cast<unsigned char*>(ptr) - sizeof(MemoryBlockHeader);
    auto* header = reinterpret_cast<MemoryBlockHeader*>(block);

    checkBlock(block);

    mgr.memAllocated_ -= header->size;
    mgr.numBlocks_--;

    std::free(block);
}

// 0x4AEE24
void* MemoryManager::prepBlock(void* block, size_t size)
{
    auto* header = static_cast<MemoryBlockHeader*>(block);
    header->guard = kHeaderGuard;
    header->size = size;

    auto* footer = reinterpret_cast<MemoryBlockFooter*>(
        static_cast<unsigned char*>(block) + size - sizeof(MemoryBlockFooter));
    footer->guard = kFooterGuard;

    return static_cast<unsigned char*>(block) + sizeof(MemoryBlockHeader);
}

// 0x4AEE44
void MemoryManager::checkBlock(void* block)
{
    auto* header = static_cast<MemoryBlockHeader*>(block);
    if (header->guard != kHeaderGuard) {
        debug_printf("Memory header stomped.\n");
    }

    auto* footer = reinterpret_cast<MemoryBlockFooter*>(
        static_cast<unsigned char*>(block) + header->size - sizeof(MemoryBlockFooter));
    if (footer->guard != kFooterGuard) {
        debug_printf("Memory footer stomped.\n");
    }
}

// ---------------------------------------------------------------------------
// Legacy free-function API — thin wrappers for backward compatibility.
// ---------------------------------------------------------------------------

char* mem_strdup(const char* string)
{
    return MemoryManager::instance().strdup(string);
}

void* mem_malloc(size_t size)
{
    return MemoryManager::instance().malloc(size);
}

void* mem_realloc(void* ptr, size_t size)
{
    return MemoryManager::instance().realloc(ptr, size);
}

void mem_free(void* ptr)
{
    MemoryManager::instance().free(ptr);
}

void mem_check()
{
    MemoryManager::instance().printStats();
}

void mem_register_func(MallocFunc* mallocFunc, ReallocFunc* reallocFunc, FreeFunc* freeFunc)
{
    MemoryManager::instance().registerAllocator(mallocFunc, reallocFunc, freeFunc);
}

} // namespace fallout

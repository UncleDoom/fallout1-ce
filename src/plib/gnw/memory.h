#pragma once

#include <cstddef>

namespace fallout {

// Allocator function signatures.
using MallocFunc = void*(size_t size);
using ReallocFunc = void*(void* ptr, size_t newSize);
using FreeFunc = void(void* ptr);

/// Encapsulates the custom memory allocator with debug guard tracking.
///
/// Wraps malloc/realloc/free with configurable function pointers and optional
/// header/footer guard values to detect memory corruption. Tracks allocation
/// statistics (current/max blocks, current/max bytes).
///
/// Usage: Access the process-wide instance via MemoryManager::instance().
/// Legacy free functions (mem_malloc, mem_free, etc.) delegate to it.
class MemoryManager {
public:
    /// Returns the process-wide singleton.
    static MemoryManager& instance() noexcept;

    // Non-copyable, non-movable (singleton).
    MemoryManager(const MemoryManager&) = delete;
    MemoryManager& operator=(const MemoryManager&) = delete;
    MemoryManager(MemoryManager&&) = delete;
    MemoryManager& operator=(MemoryManager&&) = delete;

    /// Duplicate a C string. Returns nullptr if \p string is nullptr.
    [[nodiscard]] char* strdup(const char* string);

    /// Allocate \p size bytes. Returns nullptr on failure or if size == 0.
    [[nodiscard]] void* malloc(size_t size);

    /// Reallocate a previously-allocated block to \p size bytes.
    /// If \p ptr is nullptr behaves like malloc. If \p size is 0 behaves like free.
    [[nodiscard]] void* realloc(void* ptr, size_t size);

    /// Free a previously-allocated block. Null-safe.
    void free(void* ptr);

    /// Print current and peak allocation statistics via debug_printf.
    void printStats() const;

    /// Replace the underlying allocator functions.
    /// Must be called before the windowing system is initialised.
    void registerAllocator(MallocFunc* mallocFunc, ReallocFunc* reallocFunc, FreeFunc* freeFunc);

    // -- Accessors -----------------------------------------------------------
    [[nodiscard]] int currentBlocks() const noexcept { return numBlocks_; }
    [[nodiscard]] int peakBlocks() const noexcept { return maxBlocks_; }
    [[nodiscard]] size_t currentBytes() const noexcept { return memAllocated_; }
    [[nodiscard]] size_t peakBytes() const noexcept { return maxAllocated_; }

private:
    MemoryManager() noexcept;

    // Default allocator implementations (with guard pages).
    static void* defaultMalloc(size_t size);
    static void* defaultRealloc(void* ptr, size_t size);
    static void defaultFree(void* ptr);

    static void* prepBlock(void* block, size_t size);
    static void checkBlock(void* block);

    MallocFunc* malloc_ = defaultMalloc;
    ReallocFunc* realloc_ = defaultRealloc;
    FreeFunc* free_ = defaultFree;

    int numBlocks_ = 0;
    int maxBlocks_ = 0;
    size_t memAllocated_ = 0;
    size_t maxAllocated_ = 0;
};

// -- Legacy free-function API (delegates to MemoryManager::instance()) -------
char* mem_strdup(const char* string);
void* mem_malloc(size_t size);
void* mem_realloc(void* ptr, size_t size);
void mem_free(void* ptr);
void mem_check();
void mem_register_func(MallocFunc* mallocFunc, ReallocFunc* reallocFunc, FreeFunc* freeFunc);

} // namespace fallout

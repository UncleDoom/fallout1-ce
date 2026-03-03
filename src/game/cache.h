#pragma once

#include <cstddef>

#include "game/heap.h"

#include "game/enum_utils.h"

namespace fallout {

/// Sentinel returned when a cache lookup fails.
// NOTE: Cannot be constexpr (reinterpret_cast not allowed in constant expressions).
#define INVALID_CACHE_ENTRY (reinterpret_cast<CacheEntry*>(-1))

/// Initial capacity (number of entry pointers) when a cache is created.
inline constexpr int kCacheEntriesInitialCapacity = 100;

/// Number of entry pointers added when the array needs to grow.
inline constexpr int kCacheEntriesGrowCapacity = 50;

enum class CacheEntryFlags : unsigned {
    // Specifies that cache entry has no references as should be evicted during
    // the next sweep operation.
    MarkedForEviction = 0x01,
};

DEFINE_ENUM_FLAG_OPERATORS(CacheEntryFlags)

inline constexpr int CACHE_ENTRY_MARKED_FOR_EVICTION = static_cast<int>(CacheEntryFlags::MarkedForEviction);

enum class CacheListRequestType : int {
    AllItems = 0,
    LockedItems = 1,
    UnlockedItems = 2,
};

inline constexpr int CACHE_LIST_REQUEST_TYPE_ALL_ITEMS = static_cast<int>(CacheListRequestType::AllItems);
inline constexpr int CACHE_LIST_REQUEST_TYPE_LOCKED_ITEMS = static_cast<int>(CacheListRequestType::LockedItems);
inline constexpr int CACHE_LIST_REQUEST_TYPE_UNLOCKED_ITEMS = static_cast<int>(CacheListRequestType::UnlockedItems);

using CacheSizeProc = int(int key, int* sizePtr);
using CacheReadProc = int(int key, int* sizePtr, unsigned char* buffer);
using CacheFreeProc = void(void* ptr);

struct CacheEntry {
    int key;
    int size;
    unsigned char* data;
    unsigned int referenceCount;

    // Total number of hits that this cache entry received during it's
    // lifetime.
    unsigned int hits;

    unsigned int flags;

    // The most recent hit in terms of cache hit counter. Used to track most
    // recently used entries in eviction strategy.
    unsigned int mru;

    int heapHandleIndex;
};

/// An LRU/MRU cache backed by a custom heap allocator.
///
/// Maintains a sorted array of CacheEntry pointers (sorted by key) and
/// provides lock/unlock semantics with reference counting, automatic
/// eviction based on hit counts and MRU timestamps, and corruption-
/// guarded heap storage.
class Cache {
public:
    Cache() noexcept = default;
    ~Cache();

    // Non-copyable (owns heap + entries array).
    Cache(const Cache&) = delete;
    Cache& operator=(const Cache&) = delete;

    /// Initialise the cache with the given callbacks and maximum byte budget.
    [[nodiscard]] bool init(CacheSizeProc* sizeProc, CacheReadProc* readProc, CacheFreeProc* freeProc, int maxSize);

    /// Shut down the cache, releasing all entries and the heap.
    bool exit();

    /// Returns 1 if an entry for \p key exists, 0 otherwise.
    [[nodiscard]] int query(int key);

    /// Lock an entry, loading it into the cache if necessary.
    [[nodiscard]] bool lock(int key, void** data, CacheEntry** cacheEntryPtr);

    /// Release a reference obtained via lock().
    [[nodiscard]] bool unlock(CacheEntry* cacheEntry);

    /// Mark the entry for \p key for eviction and purge immediately.
    [[nodiscard]] int discard(int key);

    /// Mark all unreferenced entries for eviction and purge.
    [[nodiscard]] bool flush();

    /// Write the current size into \p sizePtr.  Returns 1 on success.
    [[nodiscard]] int getSize(int* sizePtr);

    /// Write human-readable stats into \p dest.
    [[nodiscard]] bool stats(char* dest, size_t size);

    /// Build a list of keys matching the given request type.
    [[nodiscard]] int createList(unsigned int requestType, int** tagsPtr, int* tagsLengthPtr);

    /// Free a list previously obtained from createList().
    static int destroyList(int** tagsPtr);

private:
    bool add(int key, int* indexPtr);
    bool insert(CacheEntry* cacheEntry, int index);
    int find(int key, int* indexPtr);
    static int createItem(CacheEntry** cacheEntryPtr);
    static bool initItem(CacheEntry* cacheEntry);
    bool destroyItem(CacheEntry* cacheEntry);
    bool unlockAll();
    bool resetCounter();
    bool makeRoom(int size);
    bool purge();
    bool resizeArray(int newCapacity);

    static int compareMakeRoom(const void* a1, const void* a2);
    static int compareResetCounter(const void* a1, const void* a2);

    // Current size of entries in cache (sum of entry sizes, bytes).
    int size_ = 0;

    // Maximum size of entries in cache.
    int maxSize_ = 0;

    // The length of `entries_` array (number of live entries).
    int entriesLength_ = 0;

    // The capacity of `entries_` array.
    int entriesCapacity_ = 0;

    // Total number of hits during cache lifetime.
    unsigned int hits_ = 0;

    // List of cache entries.
    CacheEntry** entries_ = nullptr;

    CacheSizeProc* sizeProc_ = nullptr;
    CacheReadProc* readProc_ = nullptr;
    CacheFreeProc* freeProc_ = nullptr;
    Heap heap_{};
};

// -- Legacy free-function API (delegates to Cache methods) -------------------
bool cache_init(Cache* cache, CacheSizeProc* sizeProc, CacheReadProc* readProc, CacheFreeProc* freeProc, int maxSize);
bool cache_exit(Cache* cache);
int cache_query(Cache* cache, int key);
bool cache_lock(Cache* cache, int key, void** data, CacheEntry** cacheEntryPtr);
bool cache_unlock(Cache* cache, CacheEntry* cacheEntry);
int cache_discard(Cache* cache, int key);
bool cache_flush(Cache* cache);
int cache_size(Cache* cache, int* sizePtr);
bool cache_stats(Cache* cache, char* dest, size_t size);
int cache_create_list(Cache* cache, unsigned int a2, int** tagsPtr, int* tagsLengthPtr);
int cache_destroy_list(int** tagsPtr);

} // namespace fallout

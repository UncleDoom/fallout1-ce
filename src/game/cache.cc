#include "game/cache.h"

#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "int/sound.h"
#include "plib/gnw/debug.h"
#include "plib/gnw/memory.h"

namespace fallout {

// 0x4FEC7C
static int lock_sound_ticker = 0;

// ---------------------------------------------------------------------------
// Cache implementation
// ---------------------------------------------------------------------------

Cache::~Cache()
{
    // If the cache was initialised (entries_ non-null), clean up.
    if (entries_ != nullptr) {
        exit();
    }
}

// 0x41E9C0
bool Cache::init(CacheSizeProc* sizeProc, CacheReadProc* readProc, CacheFreeProc* freeProc, int maxSize)
{
    if (!heap_.init(maxSize)) {
        return false;
    }

    size_ = 0;
    maxSize_ = maxSize;
    entriesLength_ = 0;
    entriesCapacity_ = kCacheEntriesInitialCapacity;
    hits_ = 0;
    entries_ = static_cast<CacheEntry**>(mem_malloc(sizeof(*entries_) * entriesCapacity_));
    sizeProc_ = sizeProc;
    readProc_ = readProc;
    freeProc_ = freeProc;

    if (entries_ == nullptr) {
        return false;
    }

    std::memset(entries_, 0, sizeof(*entries_) * entriesCapacity_);

    return true;
}

// 0x41EA50
bool Cache::exit()
{
    unlockAll();
    flush();
    heap_.exit();

    size_ = 0;
    maxSize_ = 0;
    entriesLength_ = 0;
    entriesCapacity_ = 0;
    hits_ = 0;

    if (entries_ != nullptr) {
        mem_free(entries_);
        entries_ = nullptr;
    }

    sizeProc_ = nullptr;
    readProc_ = nullptr;
    freeProc_ = nullptr;

    return true;
}

// 0x41EAC0
int Cache::query(int key)
{
    int index;
    if (find(key, &index) != 2) {
        return 0;
    }
    return 1;
}

// 0x41EAE8
bool Cache::lock(int key, void** data, CacheEntry** cacheEntryPtr)
{
    if (data == nullptr || cacheEntryPtr == nullptr) {
        return false;
    }

    *cacheEntryPtr = nullptr;

    int index;
    int rc = find(key, &index);
    if (rc == 2) {
        // Use existing cache entry.
        CacheEntry* cacheEntry = entries_[index];
        cacheEntry->hits++;
    } else if (rc == 3) {
        // New cache entry is required.
        if (entriesLength_ >= INT_MAX) {
            return false;
        }

        if (!add(key, &index)) {
            return false;
        }

        lock_sound_ticker %= 4;
        if (lock_sound_ticker == 0) {
            soundUpdate();
        }
    } else {
        return false;
    }

    CacheEntry* cacheEntry = entries_[index];
    if (cacheEntry->referenceCount == 0) {
        if (!heap_.lock(cacheEntry->heapHandleIndex, &(cacheEntry->data))) {
            return false;
        }
    }

    cacheEntry->referenceCount++;

    hits_++;
    cacheEntry->mru = hits_;

    if (hits_ == UINT_MAX) {
        resetCounter();
    }

    *data = cacheEntry->data;
    *cacheEntryPtr = cacheEntry;

    return true;
}

// 0x41EDB8
bool Cache::unlock(CacheEntry* cacheEntry)
{
    if (cacheEntry == nullptr) {
        return false;
    }

    if (cacheEntry->referenceCount == 0) {
        return false;
    }

    cacheEntry->referenceCount--;

    if (cacheEntry->referenceCount == 0) {
        heap_.unlock(cacheEntry->heapHandleIndex);
    }

    return true;
}

// 0x41EDEC
int Cache::discard(int key)
{
    int index;
    if (find(key, &index) != 2) {
        return 0;
    }

    CacheEntry* cacheEntry = entries_[index];
    if (cacheEntry->referenceCount != 0) {
        return 0;
    }

    cacheEntry->flags |= CACHE_ENTRY_MARKED_FOR_EVICTION;
    purge();

    return 1;
}

// 0x41EE2C
bool Cache::flush()
{
    // Loop thru cache entries and mark those with no references for eviction.
    for (int index = 0; index < entriesLength_; index++) {
        CacheEntry* cacheEntry = entries_[index];
        if (cacheEntry->referenceCount == 0) {
            cacheEntry->flags |= CACHE_ENTRY_MARKED_FOR_EVICTION;
        }
    }

    // Sweep cache entries marked earlier.
    purge();

    // Shrink cache entries array if it's too big.
    int optimalCapacity = entriesLength_ + kCacheEntriesGrowCapacity;
    if (optimalCapacity < entriesCapacity_) {
        resizeArray(optimalCapacity);
    }

    return true;
}

// 0x41EE84
int Cache::getSize(int* sizePtr)
{
    if (sizePtr == nullptr) {
        return 0;
    }

    *sizePtr = size_;
    return 1;
}

// 0x41EE9C
bool Cache::stats(char* dest, size_t size)
{
    if (dest == nullptr) {
        return false;
    }

    std::snprintf(dest, size, "Cache stats are disabled.%s", "\n");
    return true;
}

// 0x41EEC0
int Cache::createList(unsigned int a2, int** tagsPtr, int* tagsLengthPtr)
{
    int cacheItemIndex;
    int tagIndex;

    if (tagsPtr == nullptr) {
        return 0;
    }

    if (tagsLengthPtr == nullptr) {
        return 0;
    }

    *tagsLengthPtr = 0;

    switch (a2) {
    case CACHE_LIST_REQUEST_TYPE_ALL_ITEMS:
        *tagsPtr = static_cast<int*>(mem_malloc(sizeof(*tagsPtr) * entriesLength_));
        if (*tagsPtr == nullptr) {
            return 0;
        }

        for (cacheItemIndex = 0; cacheItemIndex < entriesLength_; cacheItemIndex++) {
            (*tagsPtr)[cacheItemIndex] = entries_[cacheItemIndex]->key;
        }

        *tagsLengthPtr = entriesLength_;
        break;

    case CACHE_LIST_REQUEST_TYPE_LOCKED_ITEMS:
        for (cacheItemIndex = 0; cacheItemIndex < entriesLength_; cacheItemIndex++) {
            if (entries_[cacheItemIndex]->referenceCount != 0) {
                (*tagsLengthPtr)++;
            }
        }

        *tagsPtr = static_cast<int*>(mem_malloc(sizeof(*tagsPtr) * (*tagsLengthPtr)));
        if (*tagsPtr == nullptr) {
            return 0;
        }

        tagIndex = 0;
        for (cacheItemIndex = 0; cacheItemIndex < entriesLength_; cacheItemIndex++) {
            if (entries_[cacheItemIndex]->referenceCount != 0) {
                if (tagIndex < *tagsLengthPtr) {
                    (*tagsPtr)[tagIndex++] = entries_[cacheItemIndex]->key;
                }
            }
        }
        break;

    case CACHE_LIST_REQUEST_TYPE_UNLOCKED_ITEMS:
        for (cacheItemIndex = 0; cacheItemIndex < entriesLength_; cacheItemIndex++) {
            if (entries_[cacheItemIndex]->referenceCount == 0) {
                (*tagsLengthPtr)++;
            }
        }

        *tagsPtr = static_cast<int*>(mem_malloc(sizeof(*tagsPtr) * (*tagsLengthPtr)));
        if (*tagsPtr == nullptr) {
            return 0;
        }

        tagIndex = 0;
        for (cacheItemIndex = 0; cacheItemIndex < entriesLength_; cacheItemIndex++) {
            if (entries_[cacheItemIndex]->referenceCount == 0) {
                if (tagIndex < *tagsLengthPtr) {
                    (*tagsPtr)[tagIndex++] = entries_[cacheItemIndex]->key;
                }
            }
        }
        break;
    }

    return 1;
}

// 0x41F084
int Cache::destroyList(int** tagsPtr)
{
    if (tagsPtr == nullptr) {
        return 0;
    }

    if (*tagsPtr == nullptr) {
        return 0;
    }

    mem_free(*tagsPtr);
    *tagsPtr = nullptr;

    return 1;
}

// -- Private methods --------------------------------------------------------

// Fetches entry for the specified key into the cache.
// 0x41F0AC
bool Cache::add(int key, int* indexPtr)
{
    CacheEntry* cacheEntry;

    // NOTE: Uninline.
    if (createItem(&cacheEntry) != 1) {
        return false;
    }

    do {
        int size;
        if (sizeProc_(key, &size) != 0) {
            break;
        }

        if (!makeRoom(size)) {
            break;
        }

        bool allocated = false;
        int cacheEntrySize = size;
        for (int attempt = 0; attempt < 10; attempt++) {
            if (heap_.allocate(&(cacheEntry->heapHandleIndex), size, 1)) {
                allocated = true;
                break;
            }

            cacheEntrySize = static_cast<int>(static_cast<double>(cacheEntrySize) + static_cast<double>(size) * 0.25);
            if (cacheEntrySize > maxSize_) {
                break;
            }

            if (!makeRoom(cacheEntrySize)) {
                break;
            }
        }

        if (!allocated) {
            flush();

            allocated = true;
            if (!heap_.allocate(&(cacheEntry->heapHandleIndex), size, 1)) {
                if (!heap_.allocate(&(cacheEntry->heapHandleIndex), size, 0)) {
                    allocated = false;
                }
            }
        }

        if (!allocated) {
            break;
        }

        do {
            if (!heap_.lock(cacheEntry->heapHandleIndex, &(cacheEntry->data))) {
                break;
            }

            if (readProc_(key, &size, cacheEntry->data) != 0) {
                break;
            }

            heap_.unlock(cacheEntry->heapHandleIndex);

            cacheEntry->size = size;
            cacheEntry->key = key;

            bool isNewKey = true;
            if (*indexPtr < entriesLength_) {
                if (key < entries_[*indexPtr]->key) {
                    if (*indexPtr == 0 || key > entries_[*indexPtr - 1]->key) {
                        isNewKey = false;
                    }
                }
            }

            if (isNewKey) {
                if (find(key, indexPtr) != 3) {
                    break;
                }
            }

            if (!insert(cacheEntry, *indexPtr)) {
                break;
            }

            return true;
        } while (0);

        heap_.unlock(cacheEntry->heapHandleIndex);
    } while (0);

    // NOTE: Uninline.
    destroyItem(cacheEntry);

    return false;
}

// 0x41F2E8
bool Cache::insert(CacheEntry* cacheEntry, int index)
{
    // Ensure cache have enough space for new entry.
    if (entriesLength_ == entriesCapacity_ - 1) {
        if (!resizeArray(entriesCapacity_ + kCacheEntriesGrowCapacity)) {
            return false;
        }
    }

    // Move entries below insertion point.
    std::memmove(&(entries_[index + 1]), &(entries_[index]), sizeof(*entries_) * (entriesLength_ - index));

    entries_[index] = cacheEntry;
    entriesLength_++;
    size_ += cacheEntry->size;

    return true;
}

// Finds index for given key.
// Returns 2 if entry already exists in cache, or 3 if entry does not exist.
// In this case indexPtr represents insertion point.
// 0x41F354
int Cache::find(int key, int* indexPtr)
{
    int length = entriesLength_;
    if (length == 0) {
        *indexPtr = 0;
        return 3;
    }

    int r = length - 1;
    int l = 0;
    int mid;
    int cmp;

    do {
        mid = (l + r) / 2;

        cmp = key - entries_[mid]->key;
        if (cmp == 0) {
            *indexPtr = mid;
            return 2;
        }

        if (cmp > 0) {
            l = l + 1;
        } else {
            r = r - 1;
        }
    } while (r >= l);

    if (cmp < 0) {
        *indexPtr = mid;
    } else {
        *indexPtr = mid + 1;
    }

    return 3;
}

// 0x41F3C0
int Cache::createItem(CacheEntry** cacheEntryPtr)
{
    *cacheEntryPtr = static_cast<CacheEntry*>(mem_malloc(sizeof(**cacheEntryPtr)));

    // FIXME: Wrong check, should be *cacheEntryPtr != nullptr.
    if (cacheEntryPtr != nullptr) {
        // NOTE: Uninline.
        return initItem(*cacheEntryPtr);
    }

    return 0;
}

// 0x41F408
bool Cache::initItem(CacheEntry* cacheEntry)
{
    cacheEntry->key = 0;
    cacheEntry->size = 0;
    cacheEntry->data = nullptr;
    cacheEntry->referenceCount = 0;
    cacheEntry->hits = 0;
    cacheEntry->flags = 0;
    cacheEntry->mru = 0;
    return true;
}

// 0x41F440
bool Cache::destroyItem(CacheEntry* cacheEntry)
{
    if (cacheEntry->data != nullptr) {
        heap_.deallocate(&(cacheEntry->heapHandleIndex));
    }

    mem_free(cacheEntry);
    return true;
}

// 0x41F464
bool Cache::unlockAll()
{
    for (int index = 0; index < entriesLength_; index++) {
        CacheEntry* cacheEntry = entries_[index];

        // NOTE: Original code is slightly different. For unknown reason it uses
        // inner loop to decrement `referenceCount` one by one. Probably using
        // some inlined function.
        if (cacheEntry->referenceCount != 0) {
            heap_.unlock(cacheEntry->heapHandleIndex);
            cacheEntry->referenceCount = 0;
        }
    }

    return true;
}

// 0x41F4D4
bool Cache::resetCounter()
{
    CacheEntry** entries = static_cast<CacheEntry**>(mem_malloc(sizeof(CacheEntry*) * entriesLength_));
    if (entries == nullptr) {
        return false;
    }

    std::memcpy(entries, entries_, sizeof(*entries) * entriesLength_);

    std::qsort(entries, entriesLength_, sizeof(*entries), compareResetCounter);

    for (int index = 0; index < entriesLength_; index++) {
        CacheEntry* cacheEntry = entries[index];
        cacheEntry->mru = index;
    }

    hits_ = entriesLength_;

    // FIXME: Obviously leak `entries`.

    return true;
}

// Prepare cache for storing new entry with the specified size.
// 0x41F54C
bool Cache::makeRoom(int size)
{
    if (size > maxSize_) {
        // The entry of given size is too big for caching, no matter what.
        return false;
    }

    if (maxSize_ - size_ >= size) {
        // There is space available for entry of given size, there is no need to
        // evict anything.
        return true;
    }

    CacheEntry** entries = static_cast<CacheEntry**>(mem_malloc(sizeof(CacheEntry*) * entriesLength_));
    if (entries != nullptr) {
        std::memcpy(entries, entries_, sizeof(CacheEntry*) * entriesLength_);
        std::qsort(entries, entriesLength_, sizeof(CacheEntry*), compareMakeRoom);

        // The sweeping threshold is 20% of cache size plus size for the new
        // entry. Once the threshold is reached the marking process stops.
        int threshold = size + static_cast<int>(static_cast<double>(size_) * 0.2);

        int accum = 0;
        int index;
        for (index = 0; index < entriesLength_; index++) {
            CacheEntry* entry = entries[index];
            if (entry->referenceCount == 0) {
                if (entry->size >= threshold) {
                    entry->flags |= CACHE_ENTRY_MARKED_FOR_EVICTION;

                    // We've just found one huge entry, there is no point to
                    // mark individual smaller entries in the code path below,
                    // reset the accumulator to skip it entirely.
                    accum = 0;
                    break;
                } else {
                    accum += entry->size;

                    if (accum >= threshold) {
                        break;
                    }
                }
            }
        }

        if (accum != 0) {
            // The loop below assumes index to be positioned on the entry, where
            // accumulator stopped. If we've reached the end, reposition
            // it to the last entry.
            if (index == entriesLength_) {
                index -= 1;
            }

            // Loop backwards from the point we've stopped and mark all
            // unreferenced entries for sweeping.
            for (; index >= 0; index--) {
                CacheEntry* entry = entries[index];
                if (entry->referenceCount == 0) {
                    entry->flags |= CACHE_ENTRY_MARKED_FOR_EVICTION;
                }
            }
        }

        mem_free(entries);
    }

    purge();

    if (maxSize_ - size_ >= size) {
        return true;
    }

    return false;
}

// 0x41F69C
bool Cache::purge()
{
    for (int index = 0; index < entriesLength_; index++) {
        CacheEntry* cacheEntry = entries_[index];
        if ((cacheEntry->flags & CACHE_ENTRY_MARKED_FOR_EVICTION) != 0) {
            if (cacheEntry->referenceCount != 0) {
                // Entry was marked for eviction but still has references,
                // unmark it.
                cacheEntry->flags &= ~CACHE_ENTRY_MARKED_FOR_EVICTION;
            } else {
                int cacheEntrySize = cacheEntry->size;

                // NOTE: Uninline.
                destroyItem(cacheEntry);

                // Move entries up.
                std::memmove(&(entries_[index]), &(entries_[index + 1]), sizeof(*entries_) * ((entriesLength_ - index) - 1));

                entriesLength_--;
                size_ -= cacheEntrySize;

                // The entry was removed, compensate index.
                index--;
            }
        }
    }

    return true;
}

// 0x41F740
bool Cache::resizeArray(int newCapacity)
{
    if (newCapacity < entriesLength_) {
        return false;
    }

    auto** entries = static_cast<CacheEntry**>(mem_realloc(entries_, sizeof(*entries_) * newCapacity));
    if (entries == nullptr) {
        return false;
    }

    entries_ = entries;
    entriesCapacity_ = newCapacity;

    return true;
}

// 0x41F774
int Cache::compareMakeRoom(const void* a1, const void* a2)
{
    CacheEntry* v1 = *static_cast<CacheEntry* const*>(a1);
    CacheEntry* v2 = *static_cast<CacheEntry* const*>(a2);

    if (v1->referenceCount != 0 && v2->referenceCount == 0) {
        return 1;
    }

    if (v2->referenceCount != 0 && v1->referenceCount == 0) {
        return -1;
    }

    if (v1->hits < v2->hits) {
        return -1;
    } else if (v1->hits > v2->hits) {
        return 1;
    }

    if (v1->mru < v2->mru) {
        return -1;
    } else if (v1->mru > v2->mru) {
        return 1;
    }

    return 0;
}

// 0x41F7E8
int Cache::compareResetCounter(const void* a1, const void* a2)
{
    CacheEntry* v1 = *static_cast<CacheEntry* const*>(a1);
    CacheEntry* v2 = *static_cast<CacheEntry* const*>(a2);

    if (v1->mru < v2->mru) {
        return 1;
    } else if (v1->mru > v2->mru) {
        return -1;
    } else {
        return 0;
    }
}

// ---------------------------------------------------------------------------
// Legacy free-function API — thin wrappers for backward compatibility.
// ---------------------------------------------------------------------------

bool cache_init(Cache* cache, CacheSizeProc* sizeProc, CacheReadProc* readProc, CacheFreeProc* freeProc, int maxSize)
{
    if (cache == nullptr) return false;
    return cache->init(sizeProc, readProc, freeProc, maxSize);
}

bool cache_exit(Cache* cache)
{
    if (cache == nullptr) return false;
    return cache->exit();
}

int cache_query(Cache* cache, int key)
{
    if (cache == nullptr) return 0;
    return cache->query(key);
}

bool cache_lock(Cache* cache, int key, void** data, CacheEntry** cacheEntryPtr)
{
    if (cache == nullptr) return false;
    return cache->lock(key, data, cacheEntryPtr);
}

bool cache_unlock(Cache* cache, CacheEntry* cacheEntry)
{
    if (cache == nullptr) return false;
    return cache->unlock(cacheEntry);
}

int cache_discard(Cache* cache, int key)
{
    if (cache == nullptr) return 0;
    return cache->discard(key);
}

bool cache_flush(Cache* cache)
{
    if (cache == nullptr) return false;
    return cache->flush();
}

int cache_size(Cache* cache, int* sizePtr)
{
    if (cache == nullptr) return 0;
    return cache->getSize(sizePtr);
}

bool cache_stats(Cache* cache, char* dest, size_t size)
{
    if (cache == nullptr) return false;
    return cache->stats(dest, size);
}

int cache_create_list(Cache* cache, unsigned int a2, int** tagsPtr, int* tagsLengthPtr)
{
    if (cache == nullptr) return 0;
    return cache->createList(a2, tagsPtr, tagsLengthPtr);
}

int cache_destroy_list(int** tagsPtr)
{
    return Cache::destroyList(tagsPtr);
}

} // namespace fallout

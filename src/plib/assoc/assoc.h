#pragma once


#include <cstdio>

namespace fallout {

class DB_FILE;

using assoc_malloc_func = void*(size_t size);
using assoc_realloc_func = void*(void* ptr, size_t newSize);
using assoc_free_func = void(void* ptr);
using assoc_load_func = int(FILE* stream, void* buffer, size_t size, int flags);
using assoc_save_func = int(FILE* stream, void* buffer, size_t size, int flags);
using assoc_load_func_db = int(DB_FILE* stream, void* buffer, size_t size, int flags);
using assoc_save_func_db = int(DB_FILE* stream, void* buffer, size_t size, int flags);

struct assoc_func_list {
    assoc_load_func* loadFunc;
    assoc_save_func* saveFunc;
    assoc_load_func_db* loadFuncDB;
    assoc_save_func_db* saveFuncDB;
    assoc_load_func* newLoadFunc;
};

// A tuple containing individual key-value pair of an assoc array.
struct assoc_pair {
    char* name;
    void* data;
};

// A sorted collection of key/value pairs.
//
// The keys in assoc array are always strings. Internally pairs are kept sorted
// by the key (case-insensitive). Both keys and values are copied when a new
// entry is added. For this reason the size of the value's type is provided
// during initialization.
class assoc_array {
public:
    // --- Member methods ---

    // Initializes the array with capacity [n] for values of [datasize] bytes.
    [[nodiscard]] int init(int n, size_t datasize, assoc_func_list* funcs = nullptr);

    // Releases all entries and frees internal storage.
    int destroy();

    // Returns the index of the entry for [name], or -1 if not found.
    [[nodiscard]] int search(const char* name) const;

    // Adds a key-value pair. Returns 0 on success, -1 on error or duplicate.
    [[nodiscard]] int insert(const char* name, const void* data);

    // Removes entry by key. Returns 0 on success, -1 if not found.
    [[nodiscard]] int remove(const char* name);

    // Grows or shrinks capacity to [n]. Returns 0 on success.
    [[nodiscard]] int resize(int n);

    // Deep-copies this array into [dst]. Returns 0 on success.
    [[nodiscard]] int copyTo(assoc_array* dst) const;

    // Reads entries from a FILE stream. Returns 0 on success.
    [[nodiscard]] int load(FILE* fp, int flags);

    // Writes entries to a FILE stream. Returns 0 on success.
    [[nodiscard]] int save(FILE* fp, int flags);

    // --- Accessors ---

    [[nodiscard]] int getSize() const { return size; }
    [[nodiscard]] assoc_pair& getEntry(int index) { return list[index]; }
    [[nodiscard]] const assoc_pair& getEntry(int index) const { return list[index]; }

    // --- Static utilities ---

    static void registerMemory(assoc_malloc_func* mallocFunc,
        assoc_realloc_func* reallocFunc,
        assoc_free_func* freeFunc);

    // --- Data members (public for legacy compatibility) ---

    int init_flag;

    // The number of key/value pairs in the array.
    int size;

    // The capacity of key/value pairs in [entries] array.
    int max;

    // The size of the values in bytes.
    size_t datasize;

    // IO callbacks.
    assoc_func_list load_save_funcs;

    // The array of key-value pairs.
    assoc_pair* list;

private:
    // Binary search helper. Returns 0 if found, -1 otherwise.
    // [position] is set to the found index or the insertion point.
    int find(const char* name, int* position) const;
};

// --- Legacy free-function wrappers ---
int assoc_init(assoc_array* a, int n, size_t datasize, assoc_func_list* assoc_funcs);
int assoc_resize(assoc_array* a, int n);
int assoc_free(assoc_array* a);
int assoc_search(assoc_array* a, const char* name);
int assoc_insert(assoc_array* a, const char* name, const void* data);
int assoc_delete(assoc_array* a, const char* name);
int assoc_copy(assoc_array* dst, assoc_array* src);
int assoc_load(FILE* fp, assoc_array* a, int flags);
int assoc_save(FILE* fp, assoc_array* a, int flags);
void assoc_register_mem(assoc_malloc_func* malloc_func, assoc_realloc_func* realloc_func, assoc_free_func* free_func);

} // namespace fallout

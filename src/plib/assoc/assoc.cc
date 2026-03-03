#include "plib/assoc/assoc.h"

#include <cassert>
#include <cstdlib>
#include <cstring>

#include "platform_compat.h"

namespace fallout {

// NOTE: I guess this marker is used as a type discriminator for implementing
// nested dictionaries. That's why every dictionary-related function starts
// with a check for this value.
// Marker value exceeds INT_MAX — keep matching original int comparison.
#define DICTIONARY_MARKER 0xFEBAFEBA

static void* default_malloc(size_t t);
static void* default_realloc(void* p, size_t t);
static void default_free(void* p);
static int assoc_read_long(FILE* fp, long* theLong);
static int assoc_read_assoc_array(FILE* fp, assoc_array* a);
static int assoc_write_long(FILE* fp, long theLong);
static int assoc_write_assoc_array(FILE* fp, assoc_array* a);

// 0x51E408
static assoc_malloc_func* internal_malloc = default_malloc;

// 0x51E40C
static assoc_realloc_func* internal_realloc = default_realloc;

// 0x51E410
static assoc_free_func* internal_free = default_free;

// 0x4D9B90
static void* default_malloc(size_t t)
{
    return malloc(t);
}

// 0x4D9B98
static void* default_realloc(void* p, size_t t)
{
    return realloc(p, t);
}

// 0x4D9BA0
static void default_free(void* p)
{
    free(p);
}

// ---------------------------------------------------------------------------
// assoc_array member methods
// ---------------------------------------------------------------------------

// 0x4D9BA8
int assoc_array::init(int n, size_t ds, assoc_func_list* funcs)
{
    max = n;
    datasize = ds;
    size = 0;

    if (funcs != nullptr) {
        memcpy(&load_save_funcs, funcs, sizeof(*funcs));
    } else {
        load_save_funcs.loadFunc = nullptr;
        load_save_funcs.saveFunc = nullptr;
        load_save_funcs.loadFuncDB = nullptr;
        load_save_funcs.saveFuncDB = nullptr;
    }

    int rc = 0;

    if (n != 0) {
        list = static_cast<assoc_pair*>(internal_malloc(sizeof(*list) * n));
        if (list == nullptr) {
            rc = -1;
        }
    } else {
        list = nullptr;
    }

    if (rc != -1) {
        init_flag = DICTIONARY_MARKER;
    }

    return rc;
}

// 0x4D9C0C
int assoc_array::resize(int n)
{
    if (init_flag != DICTIONARY_MARKER) {
        return -1;
    }

    if (n < size) {
        return -1;
    }

    assoc_pair* entries = static_cast<assoc_pair*>(internal_realloc(list, sizeof(*list) * n));
    if (entries == nullptr) {
        return -1;
    }

    max = n;
    list = entries;

    return 0;
}

// 0x4D9C48
int assoc_array::destroy()
{
    if (init_flag != DICTIONARY_MARKER) {
        return -1;
    }

    for (int index = 0; index < size; index++) {
        assoc_pair* entry = &(list[index]);
        if (entry->name != nullptr) {
            internal_free(entry->name);
        }

        if (entry->data != nullptr) {
            internal_free(entry->data);
        }
    }

    if (list != nullptr) {
        internal_free(list);
    }

    memset(this, 0, sizeof(*this));

    return 0;
}

// Binary search helper.
// Returns 0 if key is found, -1 otherwise. [position] is set to the
// found index or the insertion point for the given key.
//
// 0x4D9CC4
int assoc_array::find(const char* name, int* position) const
{
    if (init_flag != DICTIONARY_MARKER) {
        return -1;
    }

    if (size == 0) {
        *position = 0;
        return -1;
    }

    int r = size - 1;
    int l = 0;
    int mid = 0;
    int cmp = 0;
    while (r >= l) {
        mid = (l + r) / 2;

        cmp = compat_stricmp(name, list[mid].name);
        if (cmp == 0) {
            break;
        }

        if (cmp > 0) {
            l = l + 1;
        } else {
            r = r - 1;
        }
    }

    if (cmp == 0) {
        *position = mid;
        return 0;
    }

    if (cmp < 0) {
        *position = mid;
    } else {
        *position = mid + 1;
    }

    return -1;
}

// 0x4D9D5C
int assoc_array::search(const char* name) const
{
    if (init_flag != DICTIONARY_MARKER) {
        return -1;
    }

    int index;
    if (find(name, &index) != 0) {
        return -1;
    }

    return index;
}

// 0x4D9D88
int assoc_array::insert(const char* name, const void* data)
{
    if (init_flag != DICTIONARY_MARKER) {
        return -1;
    }

    int newElementIndex;
    if (find(name, &newElementIndex) == 0) {
        // Element for this key already exists.
        return -1;
    }

    if (size == max) {
        // Array reached its capacity and needs to be enlarged.
        if (resize(2 * (max + 1)) == -1) {
            return -1;
        }
    }

    // Make a copy of the key.
    char* keyCopy = static_cast<char*>(internal_malloc(strlen(name) + 1));
    if (keyCopy == nullptr) {
        return -1;
    }

    strcpy(keyCopy, name);

    // Make a copy of the value.
    void* valueCopy = nullptr;
    if (data != nullptr && datasize != 0) {
        valueCopy = internal_malloc(datasize);
        if (valueCopy == nullptr) {
            internal_free(keyCopy);
            return -1;
        }
    }

    if (valueCopy != nullptr && datasize != 0) {
        memcpy(valueCopy, data, datasize);
    }

    // Starting at the end of entries array loop backwards and move entries down
    // one by one until we reach insertion point.
    for (int index = size; index > newElementIndex; index--) {
        assoc_pair* src = &(list[index - 1]);
        assoc_pair* dest = &(list[index]);
        memcpy(dest, src, sizeof(*list));
    }

    assoc_pair* entry = &(list[newElementIndex]);
    entry->name = keyCopy;
    entry->data = valueCopy;

    size++;

    return 0;
}

// 0x4D9EE8
int assoc_array::remove(const char* name)
{
    if (init_flag != DICTIONARY_MARKER) {
        return -1;
    }

    int indexToRemove;
    if (find(name, &indexToRemove) == -1) {
        return -1;
    }

    assoc_pair* entry = &(list[indexToRemove]);

    // Free key and value (which are copies).
    internal_free(entry->name);
    if (entry->data != nullptr) {
        internal_free(entry->data);
    }

    size--;

    // Starting from the index of the entry we've just removed, loop thru the
    // remaining of the array and move entries up one by one.
    for (int index = indexToRemove; index < size; index++) {
        assoc_pair* src = &(list[index + 1]);
        assoc_pair* dest = &(list[index]);
        memcpy(dest, src, sizeof(*list));
    }

    return 0;
}

// NOTE: Unused.
//
// 0x4D9F84
int assoc_array::copyTo(assoc_array* dst) const
{
    if (init_flag != DICTIONARY_MARKER) {
        return -1;
    }

    if (dst->init(max, datasize, const_cast<assoc_func_list*>(&load_save_funcs)) != 0) {
        // FIXME: Should return -1, as we were unable to initialize dictionary.
        return 0;
    }

    for (int index = 0; index < size; index++) {
        assoc_pair* entry = &(list[index]);
        if (dst->insert(entry->name, entry->data) == -1) {
            return -1;
        }
    }

    return 0;
}

// NOTE: Unused.
//
// 0x4DA090
static int assoc_read_long(FILE* fp, long* theLong)
{
    int c;
    int temp;

    c = fgetc(fp);
    if (c == -1) {
        return -1;
    }

    temp = (c & 0xFF);

    c = fgetc(fp);
    if (c == -1) {
        return -1;
    }

    temp = (temp << 8) | (c & 0xFF);

    c = fgetc(fp);
    if (c == -1) {
        return -1;
    }

    temp = (temp << 8) | (c & 0xFF);

    c = fgetc(fp);
    if (c == -1) {
        return -1;
    }

    temp = (temp << 8) | (c & 0xFF);

    *theLong = temp;

    return 0;
}

// NOTE: Unused.
//
// 0x4DA0F4
static int assoc_read_assoc_array(FILE* fp, assoc_array* a)
{
    long temp;

    if (assoc_read_long(fp, &temp) != 0) return -1;
    a->size = temp;

    if (assoc_read_long(fp, &temp) != 0) return -1;
    a->max = temp;

    if (assoc_read_long(fp, &temp) != 0) return -1;
    a->datasize = temp;

    // NOTE: original code reads `a->list` pointer which is meaningless.
    if (assoc_read_long(fp, &temp) != 0) return -1;

    return 0;
}

// NOTE: Unused.
//
// 0x4DA158
int assoc_array::load(FILE* fp, int flags)
{
    if (init_flag != DICTIONARY_MARKER) {
        return -1;
    }

    for (int index = 0; index < size; index++) {
        assoc_pair* entry = &(list[index]);
        if (entry->name != nullptr) {
            internal_free(entry->name);
        }

        if (entry->data != nullptr) {
            internal_free(entry->data);
        }
    }

    if (list != nullptr) {
        internal_free(list);
    }

    if (assoc_read_assoc_array(fp, this) != 0) {
        return -1;
    }

    list = nullptr;

    if (max <= 0) {
        return 0;
    }

    list = static_cast<assoc_pair*>(internal_malloc(sizeof(*list) * max));
    if (list == nullptr) {
        return -1;
    }

    for (int index = 0; index < size; index++) {
        assoc_pair* entry = &(list[index]);
        entry->name = nullptr;
        entry->data = nullptr;
    }

    if (size <= 0) {
        return 0;
    }

    for (int index = 0; index < size; index++) {
        assoc_pair* entry = &(list[index]);
        int keyLength = fgetc(fp);
        if (keyLength == -1) {
            return -1;
        }

        entry->name = static_cast<char*>(internal_malloc(keyLength + 1));
        if (entry->name == nullptr) {
            return -1;
        }

        if (fgets(entry->name, keyLength + 1, fp) == nullptr) {
            return -1;
        }

        if (datasize != 0) {
            entry->data = internal_malloc(datasize);
            if (entry->data == nullptr) {
                return -1;
            }

            if (load_save_funcs.loadFunc != nullptr) {
                if (load_save_funcs.loadFunc(fp, entry->data, datasize, flags) != 0) {
                    return -1;
                }
            } else {
                if (fread(entry->data, datasize, 1, fp) != 1) {
                    return -1;
                }
            }
        }
    }

    return 0;
}

// NOTE: Unused.
//
// 0x4DA360
static int assoc_write_assoc_array(FILE* fp, assoc_array* a)
{
    if (assoc_write_long(fp, a->size) != 0) return -1;
    if (assoc_write_long(fp, a->max) != 0) return -1;
    if (assoc_write_long(fp, a->datasize) != 0) return -1;
    // NOTE: Original code writes `a->list` pointer which is meaningless.
    if (assoc_write_long(fp, 0) != 0) return -1;

    return 0;
}

// NOTE: Unused.
//
// 0x4DA3A4
int assoc_array::save(FILE* fp, int flags)
{
    if (init_flag != DICTIONARY_MARKER) {
        return -1;
    }

    if (assoc_write_assoc_array(fp, this) != 0) {
        return -1;
    }

    for (int index = 0; index < size; index++) {
        assoc_pair* entry = &(list[index]);
        int keyLength = strlen(entry->name);
        if (fputc(keyLength, fp) == -1) {
            return -1;
        }

        if (fputs(entry->name, fp) == -1) {
            return -1;
        }

        if (load_save_funcs.saveFunc != nullptr) {
            if (datasize != 0) {
                if (load_save_funcs.saveFunc(fp, entry->data, datasize, flags) != 0) {
                    return -1;
                }
            }
        } else {
            if (datasize != 0) {
                if (fwrite(entry->data, datasize, 1, fp) != 1) {
                    return -1;
                }
            }
        }
    }

    return 0;
}

// 0x4DA498
// static
void assoc_array::registerMemory(assoc_malloc_func* malloc_func,
    assoc_realloc_func* realloc_func,
    assoc_free_func* free_func)
{
    if (malloc_func != nullptr && realloc_func != nullptr && free_func != nullptr) {
        internal_malloc = malloc_func;
        internal_realloc = realloc_func;
        internal_free = free_func;
    } else {
        internal_malloc = default_malloc;
        internal_realloc = default_realloc;
        internal_free = default_free;
    }
}

// ---------------------------------------------------------------------------
// Legacy free-function wrappers
// ---------------------------------------------------------------------------

int assoc_init(assoc_array* a, int n, size_t datasize, assoc_func_list* assoc_funcs)
{
    return a->init(n, datasize, assoc_funcs);
}

int assoc_resize(assoc_array* a, int n)
{
    return a->resize(n);
}

int assoc_free(assoc_array* a)
{
    return a->destroy();
}

int assoc_search(assoc_array* a, const char* name)
{
    return a->search(name);
}

int assoc_insert(assoc_array* a, const char* name, const void* data)
{
    return a->insert(name, data);
}

int assoc_delete(assoc_array* a, const char* name)
{
    return a->remove(name);
}

int assoc_copy(assoc_array* dst, assoc_array* src)
{
    return src->copyTo(dst);
}

int assoc_load(FILE* fp, assoc_array* a, int flags)
{
    return a->load(fp, flags);
}

int assoc_save(FILE* fp, assoc_array* a, int flags)
{
    return a->save(fp, flags);
}

void assoc_register_mem(assoc_malloc_func* malloc_func, assoc_realloc_func* realloc_func, assoc_free_func* free_func)
{
    assoc_array::registerMemory(malloc_func, realloc_func, free_func);
}

// NOTE: Unused.
//
// 0x4DA2EC
static int assoc_write_long(FILE* fp, long theLong)
{
    if (fputc((theLong >> 24) & 0xFF, fp) == -1) return -1;
    if (fputc((theLong >> 16) & 0xFF, fp) == -1) return -1;
    if (fputc((theLong >> 8) & 0xFF, fp) == -1) return -1;
    if (fputc(theLong & 0xFF, fp) == -1) return -1;

    return 0;
}

} // namespace fallout

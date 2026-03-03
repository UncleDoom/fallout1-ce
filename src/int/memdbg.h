#pragma once


#include <cstddef>

namespace fallout {

using MemDbgMallocFunc = void*(size_t size);
using MemDbgReallocFunc = void*(void* ptr, size_t size);
using MemDbgFreeFunc = void(void* ptr);
using MemDbgDebugFunc = void(const char* string);

void memoryRegisterDebug(MemDbgDebugFunc* func);
void memoryRegisterAlloc(MemDbgMallocFunc* mallocProc, MemDbgReallocFunc* reallocProc, MemDbgFreeFunc* freeProc);
void* mymalloc(size_t size, const char* file, int line);
void* myrealloc(void* ptr, size_t size, const char* file, int line);
void myfree(void* ptr, const char* file, int line);
void* mycalloc(int count, int size, const char* file, int line);
char* mystrdup(const char* string, const char* file, int line);

} // namespace fallout

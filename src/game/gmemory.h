#pragma once


#include <cstddef>

namespace fallout {

int gmemory_init();
void* gmalloc(size_t size);
void* grealloc(void* ptr, size_t newSize);
void gfree(void* ptr);

} // namespace fallout

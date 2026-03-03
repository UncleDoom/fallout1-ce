#pragma once


#include <cstddef>

namespace fallout {

// The size of buffer for version string.
inline constexpr int VERSION_MAX = 32;

inline constexpr int VERSION_MAJOR = 1;
inline constexpr int VERSION_MINOR = 1;
#define VERSION_RELEASE 'R'

#define VERSION_BUILD_TIME "Nov 11 1997 14:59:39"

char* getverstr(char* dest, size_t size);

} // namespace fallout

#pragma once


namespace fallout {

inline constexpr int WORD_WRAP_MAX_COUNT = 64;

int word_wrap(const char* string, int width, short* breakpoints, short* breakpointsLengthPtr);

} // namespace fallout

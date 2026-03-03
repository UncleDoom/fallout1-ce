#pragma once


#include <cstdio>

namespace fallout {

int lzss_decode_to_buf(FILE* in, unsigned char* dest, unsigned int length);
void lzss_decode_to_file(FILE* in, FILE* out, unsigned int length);

} // namespace fallout

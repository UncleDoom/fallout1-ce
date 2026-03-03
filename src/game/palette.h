#pragma once


namespace fallout {

extern unsigned char white_palette[256 * 3];
extern unsigned char black_palette[256 * 3];

void palette_init();
void palette_reset();
void palette_exit();
void palette_fade_to(unsigned char* palette);
void palette_set_to(unsigned char* palette);
void palette_set_entries(unsigned char* palette, int start, int end);

} // namespace fallout

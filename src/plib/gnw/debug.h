#pragma once


namespace fallout {

using DebugFunc = int(char* string);

void GNW_debug_init();
void debug_register_mono();
void debug_register_log(const char* fileName, const char* mode);
void debug_register_screen();
void debug_register_env();
void debug_register_func(DebugFunc* func);
int debug_printf(const char* format, ...);
int debug_puts(char* string);
void debug_clear();

} // namespace fallout

#pragma once


namespace fallout {

int moviefx_init();
void moviefx_reset();
void moviefx_exit();
int moviefx_start(const char* fileName);
void moviefx_stop();

} // namespace fallout

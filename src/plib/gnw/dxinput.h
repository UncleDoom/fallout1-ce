#pragma once


#include <SDL.h>

namespace fallout {

struct MouseData {
    int x;
    int y;
    unsigned char buttons[2];
    int wheelX;
    int wheelY;
};

struct KeyboardData {
    int key;
    unsigned char down;
};

bool dxinput_init();
void dxinput_exit();
bool dxinput_acquire_mouse();
bool dxinput_unacquire_mouse();
bool dxinput_get_mouse_state(MouseData* mouseData);
bool dxinput_acquire_keyboard();
bool dxinput_unacquire_keyboard();
bool dxinput_flush_keyboard_buffer();
bool dxinput_read_keyboard_buffer(KeyboardData* keyboardData);

void handleMouseEvent(SDL_Event* event);

} // namespace fallout

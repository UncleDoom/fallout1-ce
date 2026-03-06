#pragma once

#include <SDL.h>

namespace fallout {

// ---------------------------------------------------------------------------
// Gamepad context — each modal screen sets its context so that the same
// button can produce different actions depending on the current game state.
// ---------------------------------------------------------------------------
enum GamepadContext {
    GAMEPAD_CTX_GAMEPLAY = 0,
    GAMEPAD_CTX_COMBAT,
    GAMEPAD_CTX_WORLDMAP,
    GAMEPAD_CTX_DIALOGUE,
    GAMEPAD_CTX_INVENTORY,
    GAMEPAD_CTX_SKILLDEX,
    GAMEPAD_CTX_MENU, // options, save/load, character, pipboy, etc.
    GAMEPAD_CTX_RADIAL,
    GAMEPAD_CTX_BARTER,
    GAMEPAD_CTX_COUNT,
};

// ---------------------------------------------------------------------------
// Abstract game actions produced by the input manager. Higher-level code
// never sees raw SDL_CONTROLLER_BUTTON_* constants.
// ---------------------------------------------------------------------------
enum GamepadAction {
    GAMEPAD_ACTION_NONE = 0,
    GAMEPAD_ACTION_CONFIRM,       // A button
    GAMEPAD_ACTION_CANCEL,        // B button
    GAMEPAD_ACTION_EXAMINE,       // X button (context-dependent)
    GAMEPAD_ACTION_CONTEXT_MENU,  // Y button (context-dependent)
    GAMEPAD_ACTION_SKILLDEX,      // X in gameplay
    GAMEPAD_ACTION_INVENTORY,     // Y in gameplay
    GAMEPAD_ACTION_OPTIONS,       // Start
    GAMEPAD_ACTION_PIPBOY,        // Back/Select
    GAMEPAD_ACTION_AUTOMAP,       // (mapped via combo or context)
    GAMEPAD_ACTION_CHARACTER,     // (mapped via combo or context)
    GAMEPAD_ACTION_END_TURN,      // Space equivalent in combat
    GAMEPAD_ACTION_TOGGLE_MODE,   // B in combat (move↔crosshair)
    GAMEPAD_ACTION_CYCLE_TARGET_NEXT, // RB
    GAMEPAD_ACTION_CYCLE_TARGET_PREV, // LB
    GAMEPAD_ACTION_QUICK_USE,     // L3 / -20
    GAMEPAD_ACTION_SWAP_HANDS,    // R3
    GAMEPAD_ACTION_CENTER_CAMERA, // R3 in gameplay
    GAMEPAD_ACTION_COUNT,
};

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------
bool gamepad_init();
void gamepad_exit();

// ---------------------------------------------------------------------------
// SDL event handling — called from GNW95_process_message()
// ---------------------------------------------------------------------------
void gamepad_handle_device_added(int joystick_index);
void gamepad_handle_device_removed(Sint32 instance_id);
void gamepad_handle_button(Uint8 button, bool pressed);
void gamepad_handle_axis(Uint8 axis, Sint16 value);

// ---------------------------------------------------------------------------
// Context management (small stack for nested modals)
// ---------------------------------------------------------------------------
void gamepad_push_context(GamepadContext ctx);
void gamepad_pop_context();
GamepadContext gamepad_get_context();

// ---------------------------------------------------------------------------
// State queries
// ---------------------------------------------------------------------------
bool gamepad_is_connected();
void gamepad_get_left_stick(float* out_x, float* out_y);
void gamepad_get_right_stick(float* out_x, float* out_y);
bool gamepad_is_button_held(Uint8 button);

// Returns true when the gamepad A button is held in a context that should
// simulate a left mouse click (gameplay, combat, worldmap, menus, etc.).
// Called from mouse_info() to merge gamepad input into the mouse state.
bool gamepad_wants_mouse_click();

// ---------------------------------------------------------------------------
// Stick → action processing — called once per frame from process_bk()
// ---------------------------------------------------------------------------
void gamepad_process_sticks();

// ---------------------------------------------------------------------------
// Configuration
// ---------------------------------------------------------------------------
void gamepad_set_deadzone(float inner, float outer);
void gamepad_get_deadzone(float* inner, float* outer);

// Free-floating cursor: move mouse by stick-proportional delta.
// Called from gamepad_process_sticks() for UI contexts and from
// gameplay/combat tick when cursor mode is NOT Move.
void gamepad_stick_move_cursor();

// Cursor speed multiplier (loaded from config, default 1.0).
float gamepad_get_cursor_speed();

} // namespace fallout

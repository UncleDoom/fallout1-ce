#pragma once

namespace fallout {

// ---------------------------------------------------------------------------
// Game-level gamepad logic — context-specific tick functions that implement
// analog stick movement, hex-snap cursor, target cycling, etc.
//
// Each function is called from the appropriate modal loop or from
// game_handle_input() when the gamepad is connected.
// ---------------------------------------------------------------------------

// Custom synthetic key codes injected by the low-level gamepad module.
// The game layer intercepts these in the event code switch to perform
// context-specific actions that can't be expressed as simple key injections.
inline constexpr int GAMEPAD_KEY_CYCLE_PREV  = 10100;  // 10000 + abs(-100)
inline constexpr int GAMEPAD_KEY_CYCLE_NEXT  = 10101;  // 10000 + abs(-101)
inline constexpr int GAMEPAD_KEY_TOGGLE_MODE = 10102;  // 10000 + abs(-102)
inline constexpr int GAMEPAD_KEY_SWAP_HANDS  = 10104;
inline constexpr int GAMEPAD_KEY_CENTER_CAM  = 10105;
inline constexpr int GAMEPAD_KEY_RADIAL_MENU = 10106;

// ---------------------------------------------------------------------------
// Per-context tick functions (call once per frame in the appropriate loop)
// ---------------------------------------------------------------------------

// Gameplay exploration: left stick → hex movement, handles stick-based party
// movement via mouse warp.
void gamepad_actions_gameplay_tick();

// Combat: left stick → hex-snap cursor movement. Moves a virtual combat
// cursor tile-by-tile in the stick direction.
void gamepad_actions_combat_tick();

// Combat: cycle through hostile targets. direction: +1 = next, -1 = prev.
// Warps mouse cursor to the target's screen position.
void gamepad_combat_cycle_target(int direction);

// Combat: toggle between Move and Crosshair mouse modes.
void gamepad_combat_toggle_mode();

// World map: left stick → party movement (sets target position).
void gamepad_actions_worldmap_tick();

// Inventory: handle X (examine) and Y (context menu) buttons.
// Returns the selected slot event code or -1 if no action.
void gamepad_actions_inventory_tick(int keyCode);

// Skilldex: D-Pad cycling through skill buttons.
// Manages the internal selected skill index.
void gamepad_actions_skilldex_tick(int keyCode);

// Get the currently selected skilldex skill event code (501-508).
// Returns -1 if none selected.
int gamepad_skilldex_get_selected_event_code();

// Reset state (call when entering a new context).
void gamepad_actions_reset();

} // namespace fallout

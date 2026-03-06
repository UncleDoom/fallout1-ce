#include "game/gamepad_actions.h"

#include <cmath>

#include "game/combat.h"
#include "game/combat_defs.h"
#include "game/critter.h"
#include "game/gmouse.h"
#include "game/map.h"
#include "game/object.h"
#include "game/object_types.h"
#include "game/tile.h"
#include "plib/gnw/gamepad.h"
#include "plib/gnw/input.h"
#include "plib/gnw/mouse.h"

namespace fallout {

// ---------------------------------------------------------------------------
// Internal state
// ---------------------------------------------------------------------------

// Combat hex-snap cursor: the tile the gamepad cursor is currently on.
static int g_combat_cursor_tile = -1;

// Combat target cycling: index in the built target list.
static int g_target_index = 0;

// Skilldex: currently highlighted skill (0-7).
static int g_skilldex_index = 0;

// Stick-to-hex timing
static unsigned int g_gameplay_stick_last_move = 0;
static unsigned int g_combat_stick_last_move = 0;
static unsigned int g_worldmap_stick_last_move = 0;

static constexpr unsigned int HEX_SNAP_DELAY_MS = 120;
static constexpr unsigned int WORLDMAP_MOVE_DELAY_MS = 50;

static constexpr int SKILLDEX_SKILL_COUNT = 8;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

// Convert analog stick angle to hex rotation direction (0-5).
// Fallout 1 uses an isometric hex grid projected onto screen space.
// The vertical axis is compressed ~2:1, so screen-space angles of
// hex directions do NOT divide into equal 60° sectors.
// Approximate screen angles:  NE≈+37°, E=0°, SE≈-37°, SW≈-143°, W=±180°, NW≈+143°
// Sector boundaries are at the midpoints between these.
static int stick_to_hex_direction(float sx, float sy)
{
    // Negate sy: SDL Y-axis is inverted (positive = down).
    float angle = atan2f(-sy, sx);
    float degrees = angle * (180.0f / 3.14159265f);

    // Isometric-tuned sector boundaries:
    //  E  = -18..+18    (stick right)
    //  NE = +18..+90     (stick upper-right)
    //  NW = +90..+162    (stick up / upper-left)
    //  W  = +162..-162   (stick left)
    //  SW = -162..-90    (stick down / lower-left)
    //  SE = -90..-18     (stick lower-right)

    if (degrees >= -18.0f && degrees < 18.0f)   return ROTATION_E;
    if (degrees >= 18.0f  && degrees < 90.0f)   return ROTATION_NE;
    if (degrees >= 90.0f  && degrees < 162.0f)  return ROTATION_NW;
    if (degrees >= 162.0f || degrees < -162.0f)  return ROTATION_W;
    if (degrees >= -162.0f && degrees < -90.0f)  return ROTATION_SW;
    return ROTATION_SE; // -90 to -18
}

static float stick_magnitude(float x, float y)
{
    return sqrtf(x * x + y * y);
}

// ---------------------------------------------------------------------------
// Hex-snap helper: warp cursor to a hex tile at given direction/distance
// from a reference tile.
// ---------------------------------------------------------------------------
static void hex_snap_cursor(int ref_tile, int direction, int distance)
{
    int target_tile = tile_num_in_direction(ref_tile, direction, distance);
    if (target_tile == -1) return;

    int sx, sy;
    if (tile_coord(target_tile, &sx, &sy, map_elevation) == 0) {
        // Offset to hex center (hex is ~32x16)
        mouse_set_position(sx + 16, sy + 8);
    }
}

// ---------------------------------------------------------------------------
// Gameplay exploration tick
// ---------------------------------------------------------------------------

void gamepad_actions_gameplay_tick()
{
    if (!gamepad_is_connected()) return;
    if (gamepad_get_context() != GAMEPAD_CTX_GAMEPLAY) return;

    // When cursor mode is NOT Move (0), use free-floating cursor.
    int mode = gmouse_3d_get_mode();
    if (mode != GAME_MOUSE_MODE_MOVE) {
        gamepad_stick_move_cursor();
        return;
    }

    // Move mode: hex-snap cursor to a tile in the stick direction.
    float lx, ly;
    gamepad_get_left_stick(&lx, &ly);

    float mag = stick_magnitude(lx, ly);
    if (mag < 0.3f) return;

    unsigned int now = SDL_GetTicks();
    if (now - g_gameplay_stick_last_move < HEX_SNAP_DELAY_MS) return;

    int direction = stick_to_hex_direction(lx, ly);
    // Scale distance by deflection magnitude: 1 tile at 30-60%, 2 at 60-80%, 3+ at 80%+
    int distance = 1;
    if (mag > 0.8f) distance = 3;
    else if (mag > 0.6f) distance = 2;

    int dude_tile = obj_dude->tile;
    hex_snap_cursor(dude_tile, direction, distance);

    g_gameplay_stick_last_move = now;
}

// ---------------------------------------------------------------------------
// Combat tick — hybrid hex-snap / free cursor
// ---------------------------------------------------------------------------

void gamepad_actions_combat_tick()
{
    if (!gamepad_is_connected()) return;
    if (gamepad_get_context() != GAMEPAD_CTX_COMBAT) return;

    // When cursor mode is Arrow or Crosshair (not Move), free cursor.
    int mode = gmouse_3d_get_mode();
    if (mode != GAME_MOUSE_MODE_MOVE) {
        gamepad_stick_move_cursor();
        return;
    }

    // Initialize combat cursor to dude's tile
    if (g_combat_cursor_tile == -1) {
        g_combat_cursor_tile = obj_dude->tile;
    }

    float lx, ly;
    gamepad_get_left_stick(&lx, &ly);

    float mag = stick_magnitude(lx, ly);
    if (mag < 0.4f) return;

    unsigned int now = SDL_GetTicks();
    if (now - g_combat_stick_last_move < HEX_SNAP_DELAY_MS) return;

    int direction = stick_to_hex_direction(lx, ly);
    int new_tile = tile_num_in_direction(g_combat_cursor_tile, direction, 1);
    if (new_tile == -1 || new_tile == g_combat_cursor_tile) return;

    g_combat_cursor_tile = new_tile;

    // Warp mouse to the new hex
    int sx, sy;
    if (tile_coord(new_tile, &sx, &sy, map_elevation) == 0) {
        mouse_set_position(sx + 16, sy + 8);
    }

    g_combat_stick_last_move = now;
}

// ---------------------------------------------------------------------------
// Combat target cycling
// ---------------------------------------------------------------------------

void gamepad_combat_cycle_target(int direction)
{
    if (!isInCombat()) return;

    // Build list of valid targets using obj_create_list (public API).
    Object** critters = nullptr;
    int count = obj_create_list(-1, map_elevation, OBJ_TYPE_CRITTER, &critters);
    if (count <= 0) return;

    // Filter to hostile, living critters (not the player).
    // We pick a simple approach: collect valid targets in a small stack array.
    static constexpr int MAX_TARGETS = 64;
    Object* targets[MAX_TARGETS];
    int target_count = 0;

    for (int i = 0; i < count && target_count < MAX_TARGETS; i++) {
        Object* critter = critters[i];
        if (critter == obj_dude) continue;
        if (critter_is_dead(critter)) continue;
        // Include all non-dead, non-player critters visible on this elevation
        if (critter->elevation != map_elevation) continue;

        targets[target_count++] = critter;
    }

    obj_delete_list(critters);

    if (target_count == 0) return;

    // Wrap the index
    g_target_index += direction;
    if (g_target_index >= target_count) g_target_index = 0;
    if (g_target_index < 0) g_target_index = target_count - 1;

    Object* target = targets[g_target_index];

    // Warp mouse to target's screen position
    int sx, sy;
    if (tile_coord(target->tile, &sx, &sy, map_elevation) == 0) {
        mouse_set_position(sx + 16, sy + 8);
        g_combat_cursor_tile = target->tile;
    }
}

// ---------------------------------------------------------------------------
// Combat mode toggle (Move ↔ Crosshair)
// ---------------------------------------------------------------------------

void gamepad_combat_toggle_mode()
{
    // Toggle between Move and Crosshair modes (same as right-click cycle).
    gmouse_3d_toggle_mode();
}

// ---------------------------------------------------------------------------
// World map tick
// ---------------------------------------------------------------------------

void gamepad_actions_worldmap_tick()
{
    if (!gamepad_is_connected()) return;
    if (gamepad_get_context() != GAMEPAD_CTX_WORLDMAP) return;

    float lx, ly;
    gamepad_get_left_stick(&lx, &ly);

    float mag = stick_magnitude(lx, ly);
    if (mag < 0.3f) return;

    unsigned int now = SDL_GetTicks();
    if (now - g_worldmap_stick_last_move < WORLDMAP_MOVE_DELAY_MS) return;

    // The world map uses mouse clicks to set destination. We simulate this
    // by injecting arrow key events for viewport scrolling from the left stick.
    // Actual party movement target-setting is done through the existing
    // click-on-map mechanism.
    if (ly < -0.5f) GNW_add_input_buffer(KEY_ARROW_UP);
    if (ly > 0.5f) GNW_add_input_buffer(KEY_ARROW_DOWN);
    if (lx < -0.5f) GNW_add_input_buffer(KEY_ARROW_LEFT);
    if (lx > 0.5f) GNW_add_input_buffer(KEY_ARROW_RIGHT);

    g_worldmap_stick_last_move = now;
}

// ---------------------------------------------------------------------------
// Inventory tick
// ---------------------------------------------------------------------------

void gamepad_actions_inventory_tick(int keyCode)
{
    // X button and Y button are handled here.
    // The actual item slot selection is done by focus manager or arrow keys,
    // which the low-level gamepad module already injects.
    // This function is a placeholder for future context-menu and examine logic.
    (void)keyCode;
}

// ---------------------------------------------------------------------------
// Skilldex tick
// ---------------------------------------------------------------------------

void gamepad_actions_skilldex_tick(int keyCode)
{
    if (keyCode == KEY_ARROW_UP) {
        g_skilldex_index--;
        if (g_skilldex_index < 0) g_skilldex_index = SKILLDEX_SKILL_COUNT - 1;
    } else if (keyCode == KEY_ARROW_DOWN) {
        g_skilldex_index++;
        if (g_skilldex_index >= SKILLDEX_SKILL_COUNT) g_skilldex_index = 0;
    } else if (keyCode == KEY_RETURN) {
        // Confirm: inject the skill event code (501 + index)
        GNW_add_input_buffer(501 + g_skilldex_index);
    }
}

int gamepad_skilldex_get_selected_event_code()
{
    if (g_skilldex_index < 0 || g_skilldex_index >= SKILLDEX_SKILL_COUNT) {
        return -1;
    }
    return 501 + g_skilldex_index;
}

// ---------------------------------------------------------------------------
// Reset state
// ---------------------------------------------------------------------------

void gamepad_actions_reset()
{
    g_combat_cursor_tile = -1;
    g_target_index = 0;
    g_skilldex_index = 0;
    g_gameplay_stick_last_move = 0;
    g_combat_stick_last_move = 0;
    g_worldmap_stick_last_move = 0;
}

} // namespace fallout

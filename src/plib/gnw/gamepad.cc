#include "plib/gnw/gamepad.h"

#include <cmath>
#include <cstring>

#include "game/gconfig.h"
#include "plib/gnw/focus.h"
#include "plib/gnw/input.h"
#include "plib/gnw/kb.h"
#include "plib/gnw/mouse.h"
#include "game/map.h"

namespace fallout {

// ---------------------------------------------------------------------------
// Internal state
// ---------------------------------------------------------------------------

static SDL_GameController* g_controller = nullptr;
static SDL_JoystickID g_controller_instance_id = -1;

// Context stack (supports up to 8 levels of nesting)
static constexpr int CONTEXT_STACK_SIZE = 8;
static GamepadContext g_context_stack[CONTEXT_STACK_SIZE];
static int g_context_stack_top = 0;
static GamepadContext g_current_context = GAMEPAD_CTX_GAMEPLAY;

// Deadzone configuration
static float g_deadzone_inner = 0.15f;
static float g_deadzone_outer = 0.95f;

// Filtered stick values (range -1..1 after deadzone)
static float g_left_stick_x = 0.0f;
static float g_left_stick_y = 0.0f;
static float g_right_stick_x = 0.0f;
static float g_right_stick_y = 0.0f;

// Button held state
static bool g_button_state[SDL_CONTROLLER_BUTTON_MAX];

// Stick → arrow key repeat timing
static unsigned int g_left_stick_last_inject = 0;
static unsigned int g_right_stick_last_inject = 0;
static constexpr unsigned int STICK_REPEAT_DELAY_MS = 100;

// ---------------------------------------------------------------------------
// Button-to-keycode mapping tables (per context)
//
// A value of 0 means "no mapping" (the button does nothing or is handled
// by higher-level code like gamepad_actions).
//
// Negative values are "custom action" markers:
//   -100  → GAMEPAD_ACTION_CYCLE_TARGET_PREV  (handled by game layer)
//   -101  → GAMEPAD_ACTION_CYCLE_TARGET_NEXT
//   -102  → GAMEPAD_ACTION_TOGGLE_MODE
//   -103  → GAMEPAD_ACTION_QUICK_USE  (-20 injected)
//   -104  → GAMEPAD_ACTION_SWAP_HANDS
//   -105  → GAMEPAD_ACTION_CENTER_CAMERA
// ---------------------------------------------------------------------------

// Custom action markers (not key codes; intercepted before inject)
static constexpr int CUSTOM_CYCLE_PREV   = -100;
static constexpr int CUSTOM_CYCLE_NEXT   = -101;
static constexpr int CUSTOM_TOGGLE_MODE  = -102;
static constexpr int CUSTOM_QUICK_USE    = -103;
static constexpr int CUSTOM_SWAP_HANDS   = -104;
static constexpr int CUSTOM_CENTER_CAM   = -105;
static constexpr int CUSTOM_RADIAL_MENU  = -106;

// SDL_CONTROLLER_BUTTON_MAX is typically 21
static int g_button_map[GAMEPAD_CTX_COUNT][SDL_CONTROLLER_BUTTON_MAX];

// CE: Configurable cursor speed for analog stick virtual-mouse movement.
// Base: 8 pixels per frame at full deflection.  Multiplier loaded from config.
static constexpr float CURSOR_BASE_SPEED = 8.0f;
static float g_cursor_speed_multiplier = 1.0f;

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

static float apply_deadzone(Sint16 raw)
{
    float v = static_cast<float>(raw) / 32767.0f;
    float magnitude = fabsf(v);
    if (magnitude < g_deadzone_inner) {
        return 0.0f;
    }
    if (magnitude > g_deadzone_outer) {
        return v > 0 ? 1.0f : -1.0f;
    }
    float sign = v > 0 ? 1.0f : -1.0f;
    return sign * (magnitude - g_deadzone_inner) / (g_deadzone_outer - g_deadzone_inner);
}

static void init_button_maps()
{
    memset(g_button_map, 0, sizeof(g_button_map));

    // -----------------------------------------------------------------------
    // CTX_GAMEPLAY — Default exploration mode
    // -----------------------------------------------------------------------
    auto& gp = g_button_map[GAMEPAD_CTX_GAMEPLAY];
    gp[SDL_CONTROLLER_BUTTON_A]             = 0;                   // handled via mouse simulation in mouse_info()
    gp[SDL_CONTROLLER_BUTTON_B]             = KEY_ESCAPE;          // cancel / options
    gp[SDL_CONTROLLER_BUTTON_X]             = CUSTOM_RADIAL_MENU; // radial skill menu
    gp[SDL_CONTROLLER_BUTTON_Y]             = KEY_LOWERCASE_I;     // inventory
    gp[SDL_CONTROLLER_BUTTON_START]         = KEY_ESCAPE;          // options
    gp[SDL_CONTROLLER_BUTTON_BACK]          = KEY_LOWERCASE_P;     // pip-boy
    gp[SDL_CONTROLLER_BUTTON_LEFTSHOULDER]  = KEY_LOWERCASE_C;  // character screen
    gp[SDL_CONTROLLER_BUTTON_RIGHTSHOULDER] = CUSTOM_TOGGLE_MODE; // cursor mode switch (move/examine/etc)
    gp[SDL_CONTROLLER_BUTTON_DPAD_UP]       = KEY_ARROW_UP;
    gp[SDL_CONTROLLER_BUTTON_DPAD_DOWN]     = KEY_ARROW_DOWN;
    gp[SDL_CONTROLLER_BUTTON_DPAD_LEFT]     = KEY_ARROW_LEFT;
    gp[SDL_CONTROLLER_BUTTON_DPAD_RIGHT]    = KEY_ARROW_RIGHT;
    gp[SDL_CONTROLLER_BUTTON_LEFTSTICK]     = KEY_LOWERCASE_A;     // attack
    gp[SDL_CONTROLLER_BUTTON_RIGHTSTICK]     = KEY_HOME;            // center camera

    // -----------------------------------------------------------------------
    // CTX_COMBAT — Combat turn
    // -----------------------------------------------------------------------
    auto& cb = g_button_map[GAMEPAD_CTX_COMBAT];
    cb[SDL_CONTROLLER_BUTTON_A]             = 0;                   // handled via mouse simulation in mouse_info()
    cb[SDL_CONTROLLER_BUTTON_B]             = CUSTOM_TOGGLE_MODE;  // move↔crosshair
    cb[SDL_CONTROLLER_BUTTON_X]             = CUSTOM_RADIAL_MENU; // radial skill menu (also in combat)
    cb[SDL_CONTROLLER_BUTTON_Y]             = KEY_LOWERCASE_I;     // inventory
    cb[SDL_CONTROLLER_BUTTON_START]         = KEY_SPACE;           // end turn
    cb[SDL_CONTROLLER_BUTTON_BACK]          = KEY_RETURN;          // end combat
    cb[SDL_CONTROLLER_BUTTON_LEFTSHOULDER]  = CUSTOM_CYCLE_PREV;
    cb[SDL_CONTROLLER_BUTTON_RIGHTSHOULDER] = CUSTOM_CYCLE_NEXT;
    cb[SDL_CONTROLLER_BUTTON_DPAD_UP]       = KEY_ARROW_UP;
    cb[SDL_CONTROLLER_BUTTON_DPAD_DOWN]     = KEY_ARROW_DOWN;
    cb[SDL_CONTROLLER_BUTTON_DPAD_LEFT]     = KEY_ARROW_LEFT;
    cb[SDL_CONTROLLER_BUTTON_DPAD_RIGHT]    = KEY_ARROW_RIGHT;
    cb[SDL_CONTROLLER_BUTTON_LEFTSTICK]     = CUSTOM_QUICK_USE;    // use active item
    cb[SDL_CONTROLLER_BUTTON_RIGHTSTICK]     = KEY_HOME;

    // -----------------------------------------------------------------------
    // CTX_WORLDMAP
    // -----------------------------------------------------------------------
    auto& wm = g_button_map[GAMEPAD_CTX_WORLDMAP];
    wm[SDL_CONTROLLER_BUTTON_A]             = 0;                   // handled via mouse simulation in mouse_info()
    wm[SDL_CONTROLLER_BUTTON_B]             = KEY_ESCAPE;
    wm[SDL_CONTROLLER_BUTTON_X]             = KEY_LOWERCASE_P;     // pip-boy in world map
    wm[SDL_CONTROLLER_BUTTON_Y]             = 0;
    wm[SDL_CONTROLLER_BUTTON_START]         = KEY_ESCAPE;
    wm[SDL_CONTROLLER_BUTTON_DPAD_UP]       = KEY_ARROW_UP;
    wm[SDL_CONTROLLER_BUTTON_DPAD_DOWN]     = KEY_ARROW_DOWN;
    wm[SDL_CONTROLLER_BUTTON_DPAD_LEFT]     = KEY_ARROW_LEFT;
    wm[SDL_CONTROLLER_BUTTON_DPAD_RIGHT]    = KEY_ARROW_RIGHT;
    wm[SDL_CONTROLLER_BUTTON_LEFTSHOULDER]  = CUSTOM_CYCLE_PREV;  // cycle towns
    wm[SDL_CONTROLLER_BUTTON_RIGHTSHOULDER] = CUSTOM_CYCLE_NEXT;

    // -----------------------------------------------------------------------
    // CTX_DIALOGUE
    // -----------------------------------------------------------------------
    auto& dl = g_button_map[GAMEPAD_CTX_DIALOGUE];
    dl[SDL_CONTROLLER_BUTTON_A]             = KEY_RETURN;          // confirm response
    dl[SDL_CONTROLLER_BUTTON_B]             = KEY_ESCAPE;
    dl[SDL_CONTROLLER_BUTTON_DPAD_UP]       = KEY_ARROW_UP;
    dl[SDL_CONTROLLER_BUTTON_DPAD_DOWN]     = KEY_ARROW_DOWN;
    dl[SDL_CONTROLLER_BUTTON_START]         = KEY_ESCAPE;

    // -----------------------------------------------------------------------
    // CTX_INVENTORY
    // -----------------------------------------------------------------------
    auto& inv = g_button_map[GAMEPAD_CTX_INVENTORY];
    inv[SDL_CONTROLLER_BUTTON_A]             = 0;                  // handled via mouse simulation in mouse_info()
    inv[SDL_CONTROLLER_BUTTON_B]             = KEY_ESCAPE;         // close
    inv[SDL_CONTROLLER_BUTTON_X]             = 0;                  // examine (handled by gamepad_actions)
    inv[SDL_CONTROLLER_BUTTON_Y]             = 0;                  // context menu (handled by gamepad_actions)
    inv[SDL_CONTROLLER_BUTTON_DPAD_UP]       = KEY_ARROW_UP;
    inv[SDL_CONTROLLER_BUTTON_DPAD_DOWN]     = KEY_ARROW_DOWN;
    inv[SDL_CONTROLLER_BUTTON_DPAD_LEFT]     = KEY_ARROW_LEFT;
    inv[SDL_CONTROLLER_BUTTON_DPAD_RIGHT]    = KEY_ARROW_RIGHT;
    inv[SDL_CONTROLLER_BUTTON_START]         = KEY_ESCAPE;

    // -----------------------------------------------------------------------
    // CTX_SKILLDEX
    // -----------------------------------------------------------------------
    auto& sk = g_button_map[GAMEPAD_CTX_SKILLDEX];
    sk[SDL_CONTROLLER_BUTTON_A]             = 0;                   // handled via mouse simulation in mouse_info()
    sk[SDL_CONTROLLER_BUTTON_B]             = KEY_ESCAPE;          // cancel
    sk[SDL_CONTROLLER_BUTTON_DPAD_UP]       = KEY_ARROW_UP;
    sk[SDL_CONTROLLER_BUTTON_DPAD_DOWN]     = KEY_ARROW_DOWN;
    sk[SDL_CONTROLLER_BUTTON_START]         = KEY_ESCAPE;

    // -----------------------------------------------------------------------
    // CTX_MENU — generic menus (options, save/load, character, pipboy)
    // -----------------------------------------------------------------------
    auto& mn = g_button_map[GAMEPAD_CTX_MENU];
    mn[SDL_CONTROLLER_BUTTON_A]             = 0;                   // handled via mouse simulation in mouse_info()
    mn[SDL_CONTROLLER_BUTTON_B]             = KEY_ESCAPE;
    mn[SDL_CONTROLLER_BUTTON_DPAD_UP]       = KEY_ARROW_UP;
    mn[SDL_CONTROLLER_BUTTON_DPAD_DOWN]     = KEY_ARROW_DOWN;
    mn[SDL_CONTROLLER_BUTTON_DPAD_LEFT]     = KEY_ARROW_LEFT;
    mn[SDL_CONTROLLER_BUTTON_DPAD_RIGHT]    = KEY_ARROW_RIGHT;
    mn[SDL_CONTROLLER_BUTTON_START]         = KEY_ESCAPE;

    // -----------------------------------------------------------------------
    // CTX_RADIAL — radial menu overrides everything
    // -----------------------------------------------------------------------
    auto& rd = g_button_map[GAMEPAD_CTX_RADIAL];
    rd[SDL_CONTROLLER_BUTTON_B]             = KEY_ESCAPE;          // cancel

    // -----------------------------------------------------------------------
    // CTX_BARTER — barter screen
    // -----------------------------------------------------------------------
    auto& bt = g_button_map[GAMEPAD_CTX_BARTER];
    bt[SDL_CONTROLLER_BUTTON_A]             = 0;                   // mouse simulation
    bt[SDL_CONTROLLER_BUTTON_B]             = KEY_ESCAPE;
    bt[SDL_CONTROLLER_BUTTON_X]             = KEY_LOWERCASE_T;     // talk (exit barter)
    bt[SDL_CONTROLLER_BUTTON_Y]             = KEY_LOWERCASE_M;     // make trade
    bt[SDL_CONTROLLER_BUTTON_START]         = KEY_LOWERCASE_M;
    bt[SDL_CONTROLLER_BUTTON_BACK]          = KEY_LOWERCASE_T;
    bt[SDL_CONTROLLER_BUTTON_DPAD_UP]       = KEY_ARROW_UP;
    bt[SDL_CONTROLLER_BUTTON_DPAD_DOWN]     = KEY_ARROW_DOWN;
    bt[SDL_CONTROLLER_BUTTON_DPAD_LEFT]     = KEY_ARROW_LEFT;
    bt[SDL_CONTROLLER_BUTTON_DPAD_RIGHT]    = KEY_ARROW_RIGHT;
}

// ---------------------------------------------------------------------------
// Public API — Lifecycle
// ---------------------------------------------------------------------------

bool gamepad_init()
{
    if (SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER) != 0) {
        return false;
    }

    memset(g_button_state, 0, sizeof(g_button_state));
    g_current_context = GAMEPAD_CTX_GAMEPLAY;
    g_context_stack_top = 0;

    init_button_maps();

    // CE: Load cursor speed from config (default 1.0, range 0.5–3.0).
    double speed = 1.0;
    if (game_config.getDouble(GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_GAMEPAD_CURSOR_SPEED_KEY, &speed)) {
        if (speed < 0.5) speed = 0.5;
        if (speed > 3.0) speed = 3.0;
        g_cursor_speed_multiplier = static_cast<float>(speed);
    }

    // Open the first available controller, if any.
    for (int i = 0; i < SDL_NumJoysticks(); i++) {
        if (SDL_IsGameController(i)) {
            gamepad_handle_device_added(i);
            break;
        }
    }

    return true;
}

void gamepad_exit()
{
    if (g_controller != nullptr) {
        SDL_GameControllerClose(g_controller);
        g_controller = nullptr;
        g_controller_instance_id = -1;
    }

    SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER);
}

// ---------------------------------------------------------------------------
// Public API — SDL event handlers
// ---------------------------------------------------------------------------

void gamepad_handle_device_added(int joystick_index)
{
    // Only support one controller at a time.
    if (g_controller != nullptr) {
        return;
    }

    if (!SDL_IsGameController(joystick_index)) {
        return;
    }

    g_controller = SDL_GameControllerOpen(joystick_index);
    if (g_controller != nullptr) {
        SDL_Joystick* joy = SDL_GameControllerGetJoystick(g_controller);
        g_controller_instance_id = SDL_JoystickInstanceID(joy);
    }
}

void gamepad_handle_device_removed(Sint32 instance_id)
{
    if (g_controller == nullptr || g_controller_instance_id != instance_id) {
        return;
    }

    SDL_GameControllerClose(g_controller);
    g_controller = nullptr;
    g_controller_instance_id = -1;

    // Reset state
    memset(g_button_state, 0, sizeof(g_button_state));
    g_left_stick_x = 0;
    g_left_stick_y = 0;
    g_right_stick_x = 0;
    g_right_stick_y = 0;
}

void gamepad_handle_button(Uint8 button, bool pressed)
{
    if (button >= SDL_CONTROLLER_BUTTON_MAX) {
        return;
    }

    g_button_state[button] = pressed;

    // A button: most contexts use mouse simulation (via mouse_info / gamepad_wants_mouse_click).
    // Only Radial still needs KEY_RETURN injection because its loop doesn't use GNW buttons.
    if (button == SDL_CONTROLLER_BUTTON_A) {
        if (pressed && g_current_context == GAMEPAD_CTX_RADIAL) {
            GNW_add_input_buffer(KEY_RETURN);
        }
        // All other contexts: handled by mouse_info() merging A-held into mouse state.
        return;
    }

    // Only act on button-down events for everything else.
    if (!pressed) {
        return;
    }

    // D-Pad: in UI contexts with registered focus elements, move focus and warp mouse.
    if (button >= SDL_CONTROLLER_BUTTON_DPAD_UP && button <= SDL_CONTROLLER_BUTTON_DPAD_RIGHT) {
        bool is_ui_ctx = (g_current_context == GAMEPAD_CTX_MENU
            || g_current_context == GAMEPAD_CTX_INVENTORY
            || g_current_context == GAMEPAD_CTX_SKILLDEX
            || g_current_context == GAMEPAD_CTX_DIALOGUE
            || g_current_context == GAMEPAD_CTX_BARTER);

        if (is_ui_ctx && focus_get_count() > 0) {
            FocusDirection dir;
            switch (button) {
            case SDL_CONTROLLER_BUTTON_DPAD_UP:    dir = FOCUS_DIR_UP; break;
            case SDL_CONTROLLER_BUTTON_DPAD_DOWN:  dir = FOCUS_DIR_DOWN; break;
            case SDL_CONTROLLER_BUTTON_DPAD_LEFT:  dir = FOCUS_DIR_LEFT; break;
            default:                               dir = FOCUS_DIR_RIGHT; break;
            }
            if (focus_move(dir)) {
                focus_warp_mouse_to_current();
            }
            // Don't also inject arrow key — focus system handled it.
            return;
        }
    }

    int keyCode = g_button_map[g_current_context][button];

    if (keyCode == 0) {
        return; // No mapping
    }

    // Handle custom action markers
    if (keyCode == CUSTOM_QUICK_USE) {
        GNW_add_input_buffer(-20);
        return;
    }

    if (keyCode == CUSTOM_RADIAL_MENU) {
        GNW_add_input_buffer(10000 + (-keyCode)); // 10106
        return;
    }

    if (keyCode == CUSTOM_TOGGLE_MODE || keyCode == CUSTOM_CYCLE_PREV
        || keyCode == CUSTOM_CYCLE_NEXT || keyCode == CUSTOM_SWAP_HANDS
        || keyCode == CUSTOM_CENTER_CAM) {
        GNW_add_input_buffer(10000 + (-keyCode));
        return;
    }

    // Standard key injection
    GNW_add_input_buffer(keyCode);
}

void gamepad_handle_axis(Uint8 axis, Sint16 value)
{
    switch (axis) {
    case SDL_CONTROLLER_AXIS_LEFTX:
        g_left_stick_x = apply_deadzone(value);
        break;
    case SDL_CONTROLLER_AXIS_LEFTY:
        g_left_stick_y = apply_deadzone(value);
        break;
    case SDL_CONTROLLER_AXIS_RIGHTX:
        g_right_stick_x = apply_deadzone(value);
        break;
    case SDL_CONTROLLER_AXIS_RIGHTY:
        g_right_stick_y = apply_deadzone(value);
        break;
    default:
        // Triggers etc. — not used for now
        break;
    }
}

// ---------------------------------------------------------------------------
// Public API — Context management
// ---------------------------------------------------------------------------

void gamepad_push_context(GamepadContext ctx)
{
    if (g_context_stack_top < CONTEXT_STACK_SIZE) {
        g_context_stack[g_context_stack_top++] = g_current_context;
    }
    // If stack is full, overwrite current context but don't corrupt the stack.
    g_current_context = ctx;
}

void gamepad_pop_context()
{
    if (g_context_stack_top > 0) {
        g_current_context = g_context_stack[--g_context_stack_top];
    } else {
        g_current_context = GAMEPAD_CTX_GAMEPLAY;
    }
}

GamepadContext gamepad_get_context()
{
    return g_current_context;
}

// ---------------------------------------------------------------------------
// Public API — State queries
// ---------------------------------------------------------------------------

bool gamepad_is_connected()
{
    return g_controller != nullptr;
}

void gamepad_get_left_stick(float* out_x, float* out_y)
{
    *out_x = g_left_stick_x;
    *out_y = g_left_stick_y;
}

void gamepad_get_right_stick(float* out_x, float* out_y)
{
    *out_x = g_right_stick_x;
    *out_y = g_right_stick_y;
}

bool gamepad_is_button_held(Uint8 button)
{
    if (button >= SDL_CONTROLLER_BUTTON_MAX) {
        return false;
    }
    return g_button_state[button];
}

bool gamepad_wants_mouse_click()
{
    if (g_controller == nullptr) {
        return false;
    }
    if (!g_button_state[SDL_CONTROLLER_BUTTON_A]) {
        return false;
    }
    // In Radial, A injects KEY_RETURN instead of mouse click.
    return g_current_context != GAMEPAD_CTX_RADIAL;
}

// ---------------------------------------------------------------------------
// Public API — Free-floating cursor movement
// ---------------------------------------------------------------------------

void gamepad_stick_move_cursor()
{
    if (!gamepad_is_connected()) return;

    float mag = sqrtf(g_left_stick_x * g_left_stick_x + g_left_stick_y * g_left_stick_y);
    if (mag < 0.15f) return;

    // Ensure cursor is visible (fixes mouse_info/mouse_simulate_input early-return).
    if (mouse_hidden()) {
        mouse_show();
    }

    float speed = CURSOR_BASE_SPEED * g_cursor_speed_multiplier;
    float dx = g_left_stick_x * speed;
    float dy = g_left_stick_y * speed;

    int mx, my;
    mouse_get_position(&mx, &my);
    mouse_set_position(mx + static_cast<int>(dx), my + static_cast<int>(dy));
}

float gamepad_get_cursor_speed()
{
    return g_cursor_speed_multiplier;
}

// ---------------------------------------------------------------------------
// Public API — Stick processing (called from process_bk)
// ---------------------------------------------------------------------------

void gamepad_process_sticks()
{
    if (!gamepad_is_connected()) {
        return;
    }

    unsigned int now = SDL_GetTicks();

    // -----------------------------------------------------------------------
    // Right stick → map scrolling (in gameplay / combat contexts)
    // -----------------------------------------------------------------------
    if (g_current_context == GAMEPAD_CTX_GAMEPLAY || g_current_context == GAMEPAD_CTX_COMBAT) {
        if (now - g_right_stick_last_inject >= STICK_REPEAT_DELAY_MS) {
            int dx = 0;
            int dy = 0;
            if (g_right_stick_x > 0.5f) dx = 1;
            else if (g_right_stick_x < -0.5f) dx = -1;
            if (g_right_stick_y > 0.5f) dy = 1;
            else if (g_right_stick_y < -0.5f) dy = -1;

            if (dx != 0 || dy != 0) {
                map_scroll(dx, dy);
                g_right_stick_last_inject = now;
            }
        }
    }

    // -----------------------------------------------------------------------
    // Left stick — free-floating cursor for UI contexts.
    // Gameplay/Combat left stick is handled by tick functions which
    // call gamepad_stick_move_cursor() for non-Move cursor modes.
    // -----------------------------------------------------------------------
    bool in_ui_context = (g_current_context == GAMEPAD_CTX_INVENTORY
        || g_current_context == GAMEPAD_CTX_SKILLDEX
        || g_current_context == GAMEPAD_CTX_MENU
        || g_current_context == GAMEPAD_CTX_DIALOGUE
        || g_current_context == GAMEPAD_CTX_BARTER);

    if (in_ui_context) {
        gamepad_stick_move_cursor();
    }

    // World map left stick → movement is handled by gamepad_actions_worldmap_tick().
    // Gameplay left stick → hex/cursor is handled by gamepad_actions_gameplay_tick().
    // Combat left stick → hex/cursor is handled by gamepad_actions_combat_tick().
}

// ---------------------------------------------------------------------------
// Public API — Configuration
// ---------------------------------------------------------------------------

void gamepad_set_deadzone(float inner, float outer)
{
    g_deadzone_inner = inner;
    g_deadzone_outer = outer;
}

void gamepad_get_deadzone(float* inner, float* outer)
{
    *inner = g_deadzone_inner;
    *outer = g_deadzone_outer;
}

} // namespace fallout

#pragma once


#include "game/object_types.h"

namespace fallout {

enum class GameMouseMode : int {
    Move = 0,
    Arrow = 1,
    Crosshair = 2,
    UseCrosshair = 3,
    UseFirstAid = 4,
    UseDoctor = 5,
    UseLockpick = 6,
    UseSteal = 7,
    UseTraps = 8,
    UseScience = 9,
    UseRepair = 10,
    Count = 11,
    FirstSkill = UseFirstAid,
};

inline constexpr int GAME_MOUSE_MODE_MOVE = static_cast<int>(GameMouseMode::Move);
inline constexpr int GAME_MOUSE_MODE_ARROW = static_cast<int>(GameMouseMode::Arrow);
inline constexpr int GAME_MOUSE_MODE_CROSSHAIR = static_cast<int>(GameMouseMode::Crosshair);
inline constexpr int GAME_MOUSE_MODE_USE_CROSSHAIR = static_cast<int>(GameMouseMode::UseCrosshair);
inline constexpr int GAME_MOUSE_MODE_USE_FIRST_AID = static_cast<int>(GameMouseMode::UseFirstAid);
inline constexpr int GAME_MOUSE_MODE_USE_DOCTOR = static_cast<int>(GameMouseMode::UseDoctor);
inline constexpr int GAME_MOUSE_MODE_USE_LOCKPICK = static_cast<int>(GameMouseMode::UseLockpick);
inline constexpr int GAME_MOUSE_MODE_USE_STEAL = static_cast<int>(GameMouseMode::UseSteal);
inline constexpr int GAME_MOUSE_MODE_USE_TRAPS = static_cast<int>(GameMouseMode::UseTraps);
inline constexpr int GAME_MOUSE_MODE_USE_SCIENCE = static_cast<int>(GameMouseMode::UseScience);
inline constexpr int GAME_MOUSE_MODE_USE_REPAIR = static_cast<int>(GameMouseMode::UseRepair);
inline constexpr int GAME_MOUSE_MODE_COUNT = static_cast<int>(GameMouseMode::Count);
inline constexpr int FIRST_GAME_MOUSE_MODE_SKILL = static_cast<int>(GameMouseMode::FirstSkill);
inline constexpr int GAME_MOUSE_MODE_SKILL_COUNT = GAME_MOUSE_MODE_COUNT - FIRST_GAME_MOUSE_MODE_SKILL;

enum class GameMouseActionMenuItem : int {
    Cancel = 0,
    Drop = 1,
    Inventory = 2,
    Look = 3,
    Rotate = 4,
    Talk = 5,
    Use = 6,
    Unload = 7,
    UseSkill = 8,
    Count,
};

inline constexpr int GAME_MOUSE_ACTION_MENU_ITEM_CANCEL = static_cast<int>(GameMouseActionMenuItem::Cancel);
inline constexpr int GAME_MOUSE_ACTION_MENU_ITEM_DROP = static_cast<int>(GameMouseActionMenuItem::Drop);
inline constexpr int GAME_MOUSE_ACTION_MENU_ITEM_INVENTORY = static_cast<int>(GameMouseActionMenuItem::Inventory);
inline constexpr int GAME_MOUSE_ACTION_MENU_ITEM_LOOK = static_cast<int>(GameMouseActionMenuItem::Look);
inline constexpr int GAME_MOUSE_ACTION_MENU_ITEM_ROTATE = static_cast<int>(GameMouseActionMenuItem::Rotate);
inline constexpr int GAME_MOUSE_ACTION_MENU_ITEM_TALK = static_cast<int>(GameMouseActionMenuItem::Talk);
inline constexpr int GAME_MOUSE_ACTION_MENU_ITEM_USE = static_cast<int>(GameMouseActionMenuItem::Use);
inline constexpr int GAME_MOUSE_ACTION_MENU_ITEM_UNLOAD = static_cast<int>(GameMouseActionMenuItem::Unload);
inline constexpr int GAME_MOUSE_ACTION_MENU_ITEM_USE_SKILL = static_cast<int>(GameMouseActionMenuItem::UseSkill);
inline constexpr int GAME_MOUSE_ACTION_MENU_ITEM_COUNT = static_cast<int>(GameMouseActionMenuItem::Count);

enum class MouseCursorType : int {
    None = 0,
    Arrow = 1,
    SmallArrowUp = 2,
    SmallArrowDown = 3,
    ScrollNW = 4,
    ScrollN = 5,
    ScrollNE = 6,
    ScrollE = 7,
    ScrollSE = 8,
    ScrollS = 9,
    ScrollSW = 10,
    ScrollW = 11,
    ScrollNWInvalid = 12,
    ScrollNInvalid = 13,
    ScrollNEInvalid = 14,
    ScrollEInvalid = 15,
    ScrollSEInvalid = 16,
    ScrollSInvalid = 17,
    ScrollSWInvalid = 18,
    ScrollWInvalid = 19,
    Crosshair = 20,
    Plus = 21,
    Destroy = 22,
    UseCrosshair = 23,
    Watch = 24,
    WaitPlanet = 25,
    WaitWatch = 26,
    TypeCount = 27,
    FirstAnimatedCursor = WaitPlanet,
};

inline constexpr int MOUSE_CURSOR_NONE = static_cast<int>(MouseCursorType::None);
inline constexpr int MOUSE_CURSOR_ARROW = static_cast<int>(MouseCursorType::Arrow);
inline constexpr int MOUSE_CURSOR_SMALL_ARROW_UP = static_cast<int>(MouseCursorType::SmallArrowUp);
inline constexpr int MOUSE_CURSOR_SMALL_ARROW_DOWN = static_cast<int>(MouseCursorType::SmallArrowDown);
inline constexpr int MOUSE_CURSOR_SCROLL_NW = static_cast<int>(MouseCursorType::ScrollNW);
inline constexpr int MOUSE_CURSOR_SCROLL_N = static_cast<int>(MouseCursorType::ScrollN);
inline constexpr int MOUSE_CURSOR_SCROLL_NE = static_cast<int>(MouseCursorType::ScrollNE);
inline constexpr int MOUSE_CURSOR_SCROLL_E = static_cast<int>(MouseCursorType::ScrollE);
inline constexpr int MOUSE_CURSOR_SCROLL_SE = static_cast<int>(MouseCursorType::ScrollSE);
inline constexpr int MOUSE_CURSOR_SCROLL_S = static_cast<int>(MouseCursorType::ScrollS);
inline constexpr int MOUSE_CURSOR_SCROLL_SW = static_cast<int>(MouseCursorType::ScrollSW);
inline constexpr int MOUSE_CURSOR_SCROLL_W = static_cast<int>(MouseCursorType::ScrollW);
inline constexpr int MOUSE_CURSOR_SCROLL_NW_INVALID = static_cast<int>(MouseCursorType::ScrollNWInvalid);
inline constexpr int MOUSE_CURSOR_SCROLL_N_INVALID = static_cast<int>(MouseCursorType::ScrollNInvalid);
inline constexpr int MOUSE_CURSOR_SCROLL_NE_INVALID = static_cast<int>(MouseCursorType::ScrollNEInvalid);
inline constexpr int MOUSE_CURSOR_SCROLL_E_INVALID = static_cast<int>(MouseCursorType::ScrollEInvalid);
inline constexpr int MOUSE_CURSOR_SCROLL_SE_INVALID = static_cast<int>(MouseCursorType::ScrollSEInvalid);
inline constexpr int MOUSE_CURSOR_SCROLL_S_INVALID = static_cast<int>(MouseCursorType::ScrollSInvalid);
inline constexpr int MOUSE_CURSOR_SCROLL_SW_INVALID = static_cast<int>(MouseCursorType::ScrollSWInvalid);
inline constexpr int MOUSE_CURSOR_SCROLL_W_INVALID = static_cast<int>(MouseCursorType::ScrollWInvalid);
inline constexpr int MOUSE_CURSOR_CROSSHAIR = static_cast<int>(MouseCursorType::Crosshair);
inline constexpr int MOUSE_CURSOR_PLUS = static_cast<int>(MouseCursorType::Plus);
inline constexpr int MOUSE_CURSOR_DESTROY = static_cast<int>(MouseCursorType::Destroy);
inline constexpr int MOUSE_CURSOR_USE_CROSSHAIR = static_cast<int>(MouseCursorType::UseCrosshair);
inline constexpr int MOUSE_CURSOR_WATCH = static_cast<int>(MouseCursorType::Watch);
inline constexpr int MOUSE_CURSOR_WAIT_PLANET = static_cast<int>(MouseCursorType::WaitPlanet);
inline constexpr int MOUSE_CURSOR_WAIT_WATCH = static_cast<int>(MouseCursorType::WaitWatch);
inline constexpr int MOUSE_CURSOR_TYPE_COUNT = static_cast<int>(MouseCursorType::TypeCount);
inline constexpr int FIRST_GAME_MOUSE_ANIMATED_CURSOR = static_cast<int>(MouseCursorType::FirstAnimatedCursor);

extern bool gmouse_clicked_on_edge;

extern Object* obj_mouse;
extern Object* obj_mouse_flat;

int gmouse_init();
int gmouse_reset();
void gmouse_exit();
void gmouse_enable();
void gmouse_disable(int a1);
int gmouse_is_enabled();
void gmouse_enable_scrolling();
void gmouse_disable_scrolling();
int gmouse_scrolling_is_enabled();
void gmouse_set_click_to_scroll(int a1);
int gmouse_get_click_to_scroll();
int gmouse_is_scrolling();
void gmouse_bk_process();
void gmouse_handle_event(int mouseX, int mouseY, int mouseState);
int gmouse_set_cursor(int cursor);
int gmouse_get_cursor();
void gmouse_set_mapper_mode(int mode);
void gmouse_3d_enable_modes();
void gmouse_3d_disable_modes();
int gmouse_3d_modes_are_enabled();
void gmouse_3d_set_mode(int a1);
int gmouse_3d_get_mode();
void gmouse_3d_toggle_mode();
void gmouse_3d_refresh();
int gmouse_3d_set_fid(int fid);
int gmouse_3d_get_fid();
void gmouse_3d_reset_fid();
void gmouse_3d_on();
void gmouse_3d_off();
bool gmouse_3d_is_on();
Object* object_under_mouse(int objectType, bool a2, int elevation);
int gmouse_3d_build_pick_frame(int x, int y, int menuItem, int width, int height);
int gmouse_3d_pick_frame_hot(int* a1, int* a2);
int gmouse_3d_build_menu_frame(int x, int y, const int* menuItems, int menuItemsCount, int width, int height);
int gmouse_3d_menu_frame_hot(int* x, int* y);
int gmouse_3d_highlight_menu_frame(int menuItemIndex);
int gmouse_3d_build_to_hit_frame(const char* string, int color);
int gmouse_3d_build_hex_frame(const char* string, int color);
void gmouse_3d_synch_item_highlight();
void gmouse_remove_item_outline(Object* object);

void gameMouseRefreshImmediately();

} // namespace fallout

#pragma once


#include "game/object_types.h"
#include "plib/db/db.h"

namespace fallout {

inline constexpr int INTERFACE_BAR_WIDTH = 640;
inline constexpr int INTERFACE_BAR_HEIGHT = 100;

enum class Hand : int {
    // Item1 (Punch)
    Left = 0,
    // Item2 (Kick)
    Right = 1,
    Count = 2,
};

inline constexpr int HAND_LEFT = static_cast<int>(Hand::Left);
inline constexpr int HAND_RIGHT = static_cast<int>(Hand::Right);
inline constexpr int HAND_COUNT = static_cast<int>(Hand::Count);

enum class InterfaceItemAction : int {
    Default = -1,
    Use = 0,
    Primary = 1,
    PrimaryAiming = 2,
    Secondary = 3,
    SecondaryAiming = 4,
    Reload = 5,
    Count = 6,
};

inline constexpr int INTERFACE_ITEM_ACTION_DEFAULT = static_cast<int>(InterfaceItemAction::Default);
inline constexpr int INTERFACE_ITEM_ACTION_USE = static_cast<int>(InterfaceItemAction::Use);
inline constexpr int INTERFACE_ITEM_ACTION_PRIMARY = static_cast<int>(InterfaceItemAction::Primary);
inline constexpr int INTERFACE_ITEM_ACTION_PRIMARY_AIMING = static_cast<int>(InterfaceItemAction::PrimaryAiming);
inline constexpr int INTERFACE_ITEM_ACTION_SECONDARY = static_cast<int>(InterfaceItemAction::Secondary);
inline constexpr int INTERFACE_ITEM_ACTION_SECONDARY_AIMING = static_cast<int>(InterfaceItemAction::SecondaryAiming);
inline constexpr int INTERFACE_ITEM_ACTION_RELOAD = static_cast<int>(InterfaceItemAction::Reload);
inline constexpr int INTERFACE_ITEM_ACTION_COUNT = static_cast<int>(InterfaceItemAction::Count);

extern int interfaceWindow;
extern int bar_window;

int intface_init();
void intface_reset();
void intface_exit();
int intface_load(DB_FILE* stream);
int intface_save(DB_FILE* stream);
void intface_hide();
void intface_show();
int intface_is_hidden();
void intface_enable();
void intface_disable();
bool intface_is_enabled();
void intface_redraw();
void intface_update_hit_points(bool animate);
void intface_update_ac(bool animate);
void intface_update_move_points(int actionPoints, int bonusMove);
int intface_get_attack(int* hitMode, bool* aiming);
int intface_update_items(bool animated);
int intface_toggle_items(bool animated);
int intface_toggle_item_state();
void intface_use_item();
int intface_is_item_right_hand();
int intface_get_current_item(Object** itemPtr);
int intface_update_ammo_lights();
void intface_end_window_open(bool animated);
void intface_end_window_close(bool animated);
void intface_end_buttons_enable();
void intface_end_buttons_disable();
int refresh_box_bar_win();
bool enable_box_bar_win();
bool disable_box_bar_win();

} // namespace fallout

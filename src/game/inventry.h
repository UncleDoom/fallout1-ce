#pragma once


#include "game/art.h"
#include "game/object_types.h"

namespace fallout {

// TODO: Convert to enum.
inline constexpr int OFF_59E7BC_COUNT = 12;

enum class InventoryWindowCursor : int {
    Hand = 0,
    Arrow = 1,
    Pick = 2,
    Menu = 3,
    Blank = 4,
    Count = 5,
};

inline constexpr int INVENTORY_WINDOW_CURSOR_HAND = static_cast<int>(InventoryWindowCursor::Hand);
inline constexpr int INVENTORY_WINDOW_CURSOR_ARROW = static_cast<int>(InventoryWindowCursor::Arrow);
inline constexpr int INVENTORY_WINDOW_CURSOR_PICK = static_cast<int>(InventoryWindowCursor::Pick);
inline constexpr int INVENTORY_WINDOW_CURSOR_MENU = static_cast<int>(InventoryWindowCursor::Menu);
inline constexpr int INVENTORY_WINDOW_CURSOR_BLANK = static_cast<int>(InventoryWindowCursor::Blank);
inline constexpr int INVENTORY_WINDOW_CURSOR_COUNT = static_cast<int>(InventoryWindowCursor::Count);

enum class InventoryWindowType : int {
    // Normal inventory window with quick character sheet.
    Normal = 0,

    // Narrow inventory window with just an item scroller that's shown when
    // a "Use item on" is selected from context menu.
    UseItemOn = 1,

    // Looting/stealing interface.
    Loot = 2,

    // Barter interface.
    Trade = 3,

    // Supplementary "Move items" window. Used to set quantity of items when
    // moving items between inventories.
    MoveItems = 4,

    // Supplementary "Set timer" window. Internally it's implemented as "Move
    // items" window but with timer overlay and slightly different adjustment
    // mechanics.
    SetTimer = 5,

    Count = 6,
};

inline constexpr int INVENTORY_WINDOW_TYPE_NORMAL = static_cast<int>(InventoryWindowType::Normal);
inline constexpr int INVENTORY_WINDOW_TYPE_USE_ITEM_ON = static_cast<int>(InventoryWindowType::UseItemOn);
inline constexpr int INVENTORY_WINDOW_TYPE_LOOT = static_cast<int>(InventoryWindowType::Loot);
inline constexpr int INVENTORY_WINDOW_TYPE_TRADE = static_cast<int>(InventoryWindowType::Trade);
inline constexpr int INVENTORY_WINDOW_TYPE_MOVE_ITEMS = static_cast<int>(InventoryWindowType::MoveItems);
inline constexpr int INVENTORY_WINDOW_TYPE_SET_TIMER = static_cast<int>(InventoryWindowType::SetTimer);
inline constexpr int INVENTORY_WINDOW_TYPE_COUNT = static_cast<int>(InventoryWindowType::Count);

extern CacheEntry* ikey[OFF_59E7BC_COUNT];

void inven_set_dude(Object* obj, int pid);
void inven_reset_dude();
void handle_inventory();
bool setup_inventory(int inventoryWindowType);
void exit_inventory(bool a1);
void display_inventory(int a1, int a2, int inventoryWindowType);
void display_body(int fid, int inventoryWindowType);
int inven_init();
void inven_exit();
void inven_set_mouse(int cursor);
void inven_hover_on(int btn, int keyCode);
void inven_hover_off(int btn, int keyCode);
void inven_pickup(int keyCode, int a2);
void switch_hand(Object* a1, Object** a2, Object** a3, int a4);
void adjust_ac(Object* critter, Object* oldArmor, Object* newArmor);
void adjust_fid();
void use_inventory_on(Object* a1);
Object* inven_right_hand(Object* obj);
Object* inven_left_hand(Object* obj);
Object* inven_worn(Object* obj);
int inven_pid_is_carried(Object* obj, int pid);
Object* inven_pid_is_carried_ptr(Object* obj, int pid);
int inven_pid_quantity_carried(Object* obj, int pid);
void display_stats();
Object* inven_find_type(Object* obj, int a2, int* inout_a3);
Object* inven_find_id(Object* obj, int a2);
Object* inven_index_ptr(Object* obj, int a2);
int inven_wield(Object* critter, Object* item, int a3);
int inven_unwield(Object* critter, int a2);
int inven_from_button(int input, Object** a2, Object*** a3, Object** a4);
void inven_display_msg(char* string);
void inven_obj_examine_func(Object* critter, Object* item);
void inven_action_cursor(int eventCode, int inventoryWindowType);
int loot_container(Object* a1, Object* a2);
int inven_steal_container(Object* a1, Object* a2);
int move_inventory(Object* a1, int a2, Object* a3, bool a4);
void barter_inventory(int win, Object* a2, Object* a3, Object* a4, int a5);
void container_enter(int a1, int a2);
void container_exit(int keyCode, int inventoryWindowType);
int drop_into_container(Object* a1, Object* a2, int a3, Object** a4, int quantity);
int drop_ammo_into_weapon(Object* weapon, Object* ammo, Object** a3, int quantity, int keyCode);
void draw_amount(int value, int inventoryWindowType);
int inven_set_timer(Object* a1);

} // namespace fallout

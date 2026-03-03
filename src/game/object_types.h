#pragma once

#include "game/enum_utils.h"

namespace fallout {

class DB_FILE;
struct Rect;

// Rotation directions on the hex grid.
enum class Rotation : int {
    NE = 0, // 0
    E = 1, // 1
    SE = 2, // 2
    SW = 3, // 3
    W = 4, // 4
    NW = 5, // 5
    Count = 6,
};

// Legacy constants for backward compatibility during migration.
// Prefer Rotation::NE etc. in new code.
inline constexpr int ROTATION_NE = static_cast<int>(Rotation::NE);
inline constexpr int ROTATION_E = static_cast<int>(Rotation::E);
inline constexpr int ROTATION_SE = static_cast<int>(Rotation::SE);
inline constexpr int ROTATION_SW = static_cast<int>(Rotation::SW);
inline constexpr int ROTATION_W = static_cast<int>(Rotation::W);
inline constexpr int ROTATION_NW = static_cast<int>(Rotation::NW);
inline constexpr int ROTATION_COUNT = static_cast<int>(Rotation::Count);

enum class ObjectType : int {
    Item = 0,
    Critter = 1,
    Scenery = 2,
    Wall = 3,
    Tile = 4,
    Misc = 5,
    Interface = 6,
    Inventory = 7,
    Head = 8,
    Background = 9,
    Skilldex = 10,
    Count = 11,
};

// Legacy constants for backward compatibility during migration.
inline constexpr int OBJ_TYPE_ITEM = static_cast<int>(ObjectType::Item);
inline constexpr int OBJ_TYPE_CRITTER = static_cast<int>(ObjectType::Critter);
inline constexpr int OBJ_TYPE_SCENERY = static_cast<int>(ObjectType::Scenery);
inline constexpr int OBJ_TYPE_WALL = static_cast<int>(ObjectType::Wall);
inline constexpr int OBJ_TYPE_TILE = static_cast<int>(ObjectType::Tile);
inline constexpr int OBJ_TYPE_MISC = static_cast<int>(ObjectType::Misc);
inline constexpr int OBJ_TYPE_INTERFACE = static_cast<int>(ObjectType::Interface);
inline constexpr int OBJ_TYPE_INVENTORY = static_cast<int>(ObjectType::Inventory);
inline constexpr int OBJ_TYPE_HEAD = static_cast<int>(ObjectType::Head);
inline constexpr int OBJ_TYPE_BACKGROUND = static_cast<int>(ObjectType::Background);
inline constexpr int OBJ_TYPE_SKILLDEX = static_cast<int>(ObjectType::Skilldex);
inline constexpr int OBJ_TYPE_COUNT = static_cast<int>(ObjectType::Count);

constexpr int FID_TYPE(int value) { return ((value) & 0xF000000) >> 24; }
constexpr int PID_TYPE(int value) { return (value) >> 24; }
constexpr int SID_TYPE(int value) { return (value) >> 24; }

enum class OutlineType : unsigned int {
    None = 0,
    Hostile = 1,
    Unknown2 = 2,
    Unknown4 = 4,
    Friendly = 8,
    Item = 16,
};
DEFINE_ENUM_FLAG_OPERATORS(OutlineType)

// Legacy constants for backward compatibility.
inline constexpr unsigned int OUTLINE_TYPE_HOSTILE = 1;
inline constexpr unsigned int OUTLINE_TYPE_2 = 2;
inline constexpr unsigned int OUTLINE_TYPE_4 = 4;
inline constexpr unsigned int OUTLINE_TYPE_FRIENDLY = 8;
inline constexpr unsigned int OUTLINE_TYPE_ITEM = 16;

enum class ObjectFlags : unsigned int {
    None = 0x00,
    Hidden = 0x01,

    // Specifies that the object should not be saved to the savegame file.
    NoSave = 0x04,
    Flat = 0x08,
    NoBlock = 0x10,
    Lighting = 0x20,

    // Specifies that the object should not be removed (freed) from the game
    // world for whatever reason.
    NoRemove = 0x400,
    MultiHex = 0x800,
    NoHighlight = 0x1000,
    Used = 0x2000,
    TransRed = 0x4000,
    TransNone = 0x8000,
    TransWall = 0x10000,
    TransGlass = 0x20000,
    TransSteam = 0x40000,
    TransEnergy = 0x80000,
    InLeftHand = 0x1000000,
    InRightHand = 0x2000000,
    Worn = 0x4000000,
    WallTransEnd = 0x10000000,
    LightThru = 0x20000000,
    Seen = 0x40000000,
    ShootThru = 0x80000000,

    InAnyHand = InLeftHand | InRightHand,
    Equipped = InAnyHand | Worn,
    TransMask = TransEnergy | TransSteam | TransGlass | TransWall | TransNone | TransRed,
    OpenDoor = ShootThru | LightThru | NoBlock,
};
DEFINE_ENUM_FLAG_OPERATORS(ObjectFlags)

// Legacy constants for backward compatibility.
inline constexpr unsigned int OBJECT_HIDDEN = 0x01;
inline constexpr unsigned int OBJECT_NO_SAVE = 0x04;
inline constexpr unsigned int OBJECT_FLAT = 0x08;
inline constexpr unsigned int OBJECT_NO_BLOCK = 0x10;
inline constexpr unsigned int OBJECT_LIGHTING = 0x20;
inline constexpr unsigned int OBJECT_NO_REMOVE = 0x400;
inline constexpr unsigned int OBJECT_MULTIHEX = 0x800;
inline constexpr unsigned int OBJECT_NO_HIGHLIGHT = 0x1000;
inline constexpr unsigned int OBJECT_USED = 0x2000;
inline constexpr unsigned int OBJECT_TRANS_RED = 0x4000;
inline constexpr unsigned int OBJECT_TRANS_NONE = 0x8000;
inline constexpr unsigned int OBJECT_TRANS_WALL = 0x10000;
inline constexpr unsigned int OBJECT_TRANS_GLASS = 0x20000;
inline constexpr unsigned int OBJECT_TRANS_STEAM = 0x40000;
inline constexpr unsigned int OBJECT_TRANS_ENERGY = 0x80000;
inline constexpr unsigned int OBJECT_IN_LEFT_HAND = 0x1000000;
inline constexpr unsigned int OBJECT_IN_RIGHT_HAND = 0x2000000;
inline constexpr unsigned int OBJECT_WORN = 0x4000000;
inline constexpr unsigned int OBJECT_WALL_TRANS_END = 0x10000000;
inline constexpr unsigned int OBJECT_LIGHT_THRU = 0x20000000;
inline constexpr unsigned int OBJECT_SEEN = 0x40000000;
inline constexpr unsigned int OBJECT_SHOOT_THRU = 0x80000000;
inline constexpr unsigned int OBJECT_IN_ANY_HAND = OBJECT_IN_LEFT_HAND | OBJECT_IN_RIGHT_HAND;
inline constexpr unsigned int OBJECT_EQUIPPED = OBJECT_IN_ANY_HAND | OBJECT_WORN;
inline constexpr unsigned int OBJECT_FLAG_0xFC000 = OBJECT_TRANS_ENERGY | OBJECT_TRANS_STEAM | OBJECT_TRANS_GLASS | OBJECT_TRANS_WALL | OBJECT_TRANS_NONE | OBJECT_TRANS_RED;
inline constexpr unsigned int OBJECT_OPEN_DOOR = OBJECT_SHOOT_THRU | OBJECT_LIGHT_THRU | OBJECT_NO_BLOCK;


enum class CritterFlags : unsigned int {
    None = 0x00,
    Barter = 0x02,
    NoSteal = 0x20,
    NoDrop = 0x40,
    NoLimbs = 0x80,
    NoAge = 0x100,
    NoHeal = 0x200,
    Invulnerable = 0x400,
    Flat = 0x800,
    SpecialDeath = 0x1000,
    LongLimbs = 0x2000,
    NoKnockback = 0x4000,
};
DEFINE_ENUM_FLAG_OPERATORS(CritterFlags)

// Legacy constants for backward compatibility.
inline constexpr unsigned int CRITTER_BARTER = 0x02;
inline constexpr unsigned int CRITTER_NO_STEAL = 0x20;
inline constexpr unsigned int CRITTER_NO_DROP = 0x40;
inline constexpr unsigned int CRITTER_NO_LIMBS = 0x80;
inline constexpr unsigned int CRITTER_NO_AGE = 0x100;
inline constexpr unsigned int CRITTER_NO_HEAL = 0x200;
inline constexpr unsigned int CRITTER_INVULNERABLE = 0x400;
inline constexpr unsigned int CRITTER_FLAT = 0x800;
inline constexpr unsigned int CRITTER_SPECIAL_DEATH = 0x1000;
inline constexpr unsigned int CRITTER_LONG_LIMBS = 0x2000;
inline constexpr unsigned int CRITTER_NO_KNOCKBACK = 0x4000;


constexpr int OUTLINE_TYPE_MASK = 0xFFFFFF;
constexpr int OUTLINE_PALETTED = 0x40000000;
constexpr unsigned int OUTLINE_DISABLED = 0x80000000;

// These two values are the same but stored in different fields.
constexpr int CONTAINER_FLAG_JAMMED = 0x04000000;
constexpr int DOOR_FLAG_JAMMGED = 0x04000000;

constexpr int CONTAINER_FLAG_LOCKED = 0x02000000;
constexpr int DOOR_FLAG_LOCKED = 0x02000000;

enum class CritterManeuver : unsigned int {
    None = 0x00,
    Engaging = 0x01,
    Disengaging = 0x02,
    Fleeing = 0x04,
};
DEFINE_ENUM_FLAG_OPERATORS(CritterManeuver)

// Legacy constants for backward compatibility.
inline constexpr unsigned int CRITTER_MANEUVER_NONE = 0;
inline constexpr unsigned int CRITTER_MANEUVER_ENGAGING = 0x01;
inline constexpr unsigned int CRITTER_MANEUVER_DISENGAGING = 0x02;
inline constexpr unsigned int CRITTER_MANUEVER_FLEEING = 0x04;


enum class DamageFlags : unsigned int {
    None = 0x00,
    KnockedOut = 0x01,
    KnockedDown = 0x02,
    CripLegLeft = 0x04,
    CripLegRight = 0x08,
    CripArmLeft = 0x10,
    CripArmRight = 0x20,
    Blind = 0x40,
    Dead = 0x80,
    Hit = 0x100,
    Critical = 0x200,
    OnFire = 0x400,
    Bypass = 0x800,
    Explode = 0x1000,
    Destroy = 0x2000,
    Drop = 0x4000,
    LoseTurn = 0x8000,
    HitSelf = 0x10000,
    LoseAmmo = 0x20000,
    Dud = 0x40000,
    HurtSelf = 0x80000,
    RandomHit = 0x100000,
    CripRandom = 0x200000,
    Backwash = 0x400000,
    PerformReverse = 0x800000,
    CripLegAny = CripLegLeft | CripLegRight,
    CripArmAny = CripArmLeft | CripArmRight,
    Crip = CripLegAny | CripArmAny | Blind,
};
DEFINE_ENUM_FLAG_OPERATORS(DamageFlags)

// Legacy constants for backward compatibility (Dam -> DamageFlags).
using Dam = DamageFlags;
inline constexpr unsigned int DAM_KNOCKED_OUT = 0x01;
inline constexpr unsigned int DAM_KNOCKED_DOWN = 0x02;
inline constexpr unsigned int DAM_CRIP_LEG_LEFT = 0x04;
inline constexpr unsigned int DAM_CRIP_LEG_RIGHT = 0x08;
inline constexpr unsigned int DAM_CRIP_ARM_LEFT = 0x10;
inline constexpr unsigned int DAM_CRIP_ARM_RIGHT = 0x20;
inline constexpr unsigned int DAM_BLIND = 0x40;
inline constexpr unsigned int DAM_DEAD = 0x80;
inline constexpr unsigned int DAM_HIT = 0x100;
inline constexpr unsigned int DAM_CRITICAL = 0x200;
inline constexpr unsigned int DAM_ON_FIRE = 0x400;
inline constexpr unsigned int DAM_BYPASS = 0x800;
inline constexpr unsigned int DAM_EXPLODE = 0x1000;
inline constexpr unsigned int DAM_DESTROY = 0x2000;
inline constexpr unsigned int DAM_DROP = 0x4000;
inline constexpr unsigned int DAM_LOSE_TURN = 0x8000;
inline constexpr unsigned int DAM_HIT_SELF = 0x10000;
inline constexpr unsigned int DAM_LOSE_AMMO = 0x20000;
inline constexpr unsigned int DAM_DUD = 0x40000;
inline constexpr unsigned int DAM_HURT_SELF = 0x80000;
inline constexpr unsigned int DAM_RANDOM_HIT = 0x100000;
inline constexpr unsigned int DAM_CRIP_RANDOM = 0x200000;
inline constexpr unsigned int DAM_BACKWASH = 0x400000;
inline constexpr unsigned int DAM_PERFORM_REVERSE = 0x800000;
inline constexpr unsigned int DAM_CRIP_LEG_ANY = DAM_CRIP_LEG_LEFT | DAM_CRIP_LEG_RIGHT;
inline constexpr unsigned int DAM_CRIP_ARM_ANY = DAM_CRIP_ARM_LEFT | DAM_CRIP_ARM_RIGHT;
inline constexpr unsigned int DAM_CRIP = DAM_CRIP_LEG_ANY | DAM_CRIP_ARM_ANY | DAM_BLIND;


constexpr int OBJ_LOCKED = 0x02000000;
constexpr int OBJ_JAMMED = 0x04000000;

struct Object;

struct InventoryItem {
    Object* item;
    int quantity;
};

// Represents inventory of the object.
class Inventory {
public:
    int length;
    int capacity;
    InventoryItem* items;

    void compact(int inventoryItemIndex);
    int inven_free();
    void display_target(int first_item_index, int selected_index, int inventoryWindowType);
};

struct WeaponObjectData {
    int ammoQuantity; // obj_pudg.pudweapon.cur_ammo_quantity
    int ammoTypePid; // obj_pudg.pudweapon.cur_ammo_type_pid
};

struct AmmoItemData {
    int quantity; // obj_pudg.pudammo.cur_ammo_quantity
};

struct MiscItemData {
    int charges; // obj_pudg.pudmisc_item.curr_charges
};

struct KeyItemData {
    int keyCode; // obj_pudg.pudkey_item.cur_key_code
};

union ItemObjectData {
    WeaponObjectData weapon;
    AmmoItemData ammo;
    MiscItemData misc;
    KeyItemData key;
};

class CritterCombatData {
public:
    int maneuver; // obj_pud.combat_data.maneuver
    int ap; // obj_pud.combat_data.curr_mp
    int results; // obj_pud.combat_data.results
    int damageLastTurn; // obj_pud.combat_data.damage_last_turn
    int aiPacket; // obj_pud.combat_data.ai_packet
    int team; // obj_pud.combat_data.team_num
    union {
        Object* whoHitMe; // obj_pud.combat_data.who_hit_me
        int whoHitMeCid;
    };

    int readCombatData(DB_FILE* stream);
    int writeCombatData(DB_FILE* stream);
};

struct CritterObjectData {
    int field_0; // obj_pud.reaction_to_pc
    CritterCombatData combat; // obj_pud.combat_data
    int hp; // obj_pud.curr_hp
    int radiation; // obj_pud.curr_rad
    int poison; // obj_pud.curr_poison
};

struct DoorSceneryData {
    int openFlags; // obj_pudg.pudportal.cur_open_flags
};

struct StairsSceneryData {
    int destinationMap; // obj_pudg.pudstairs.destMap
    int destinationBuiltTile; // obj_pudg.pudstairs.destBuiltTile
};

struct ElevatorSceneryData {
    int type;
    int level;
};

struct LadderSceneryData {
    int destinationBuiltTile;
};

union SceneryObjectData {
    DoorSceneryData door;
    StairsSceneryData stairs;
    ElevatorSceneryData elevator;
    LadderSceneryData ladder;
};

struct MiscObjectData {
    int map;
    int tile;
    int elevation;
    int rotation;
};

struct ObjectData {
    Inventory inventory;
    union {
        CritterObjectData critter;
        struct {
            int flags;
            union {
                ItemObjectData item;
                SceneryObjectData scenery;
                MiscObjectData misc;
            };
        };
    };
};

struct Object {
    int id; // obj_id
    int tile; // obj_tile_num
    int x; // obj_x
    int y; // obj_y
    int sx; // obj_sx
    int sy; // obj_sy
    int frame; // obj_cur_frm
    int rotation; // obj_cur_rot
    int fid; // obj_fid
    int flags; // obj_flags
    int elevation; // obj_elev
    union {
        int field_2C_array[14];
        ObjectData data;
    };
    int pid; // obj_pid
    int cid; // obj_cid
    int lightDistance; // obj_light_distance
    int lightIntensity; // obj_light_intensity
    int outline; // obj_outline
    int sid; // obj_sid
    Object* owner;
    int field_80;
};

class ObjectListNode {
public:
    Object* obj;
    ObjectListNode* next;

    void insert();
    int remove(ObjectListNode* prev);
    int connect_to_tile(int tile, int elevation, Rect* rect);
};

constexpr int BUILT_TILE_TILE_MASK = 0x3FFFFFF;
constexpr unsigned int BUILT_TILE_ELEVATION_MASK = 0xE0000000;
constexpr int BUILT_TILE_ELEVATION_SHIFT = 29;
constexpr int BUILT_TILE_ROTATION_MASK = 0x1C000000;
constexpr int BUILT_TILE_ROTATION_SHIFT = 26;

constexpr int builtTileGetTile(int builtTile)
{
    return builtTile & BUILT_TILE_TILE_MASK;
}

static inline int builtTileGetElevation(int builtTile)
{
    return (builtTile & BUILT_TILE_ELEVATION_MASK) >> BUILT_TILE_ELEVATION_SHIFT;
}

static inline int builtTileGetRotation(int builtTile)
{
    return (builtTile & BUILT_TILE_ROTATION_MASK) >> BUILT_TILE_ROTATION_SHIFT;
}

static inline int builtTileCreate(int tile, int elevation)
{
    return tile | ((elevation << BUILT_TILE_ELEVATION_SHIFT) & BUILT_TILE_ELEVATION_MASK);
}

} // namespace fallout

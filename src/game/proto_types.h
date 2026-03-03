#pragma once


#include "game/enum_utils.h"

namespace fallout {

class DB_FILE;

// Number of prototypes in prototype extent.
constexpr int PROTO_LIST_EXTENT_SIZE = 16;

// Max number of prototypes of one type to be stored in prototype cache lists.
// Once this value is reached the top most proto extent is removed from the
// cache list.
//
// See:
// - [sub_4A2108]
// - [sub_4A2040]
constexpr int PROTO_LIST_MAX_ENTRIES = 512;

constexpr int WEAPON_TWO_HAND = 0x00000200;

enum class Gender : int {
    Male = 0,
    Female = 1,
    Count = 2,
};

inline constexpr int GENDER_MALE = static_cast<int>(Gender::Male);
inline constexpr int GENDER_FEMALE = static_cast<int>(Gender::Female);
inline constexpr int GENDER_COUNT = static_cast<int>(Gender::Count);

enum class ItemType : int {
    Armor = 0,
    Container = 1,
    Drug = 2,
    Weapon = 3,
    Ammo = 4,
    Misc = 5,
    Key = 6,
    Count = 7,
};

inline constexpr int ITEM_TYPE_ARMOR = static_cast<int>(ItemType::Armor);
inline constexpr int ITEM_TYPE_CONTAINER = static_cast<int>(ItemType::Container);
inline constexpr int ITEM_TYPE_DRUG = static_cast<int>(ItemType::Drug);
inline constexpr int ITEM_TYPE_WEAPON = static_cast<int>(ItemType::Weapon);
inline constexpr int ITEM_TYPE_AMMO = static_cast<int>(ItemType::Ammo);
inline constexpr int ITEM_TYPE_MISC = static_cast<int>(ItemType::Misc);
inline constexpr int ITEM_TYPE_KEY = static_cast<int>(ItemType::Key);
inline constexpr int ITEM_TYPE_COUNT = static_cast<int>(ItemType::Count);

enum class SceneryType : int {
    Door = 0,
    Stairs = 1,
    Elevator = 2,
    LadderUp = 3,
    LadderDown = 4,
    Generic = 5,
    Count = 6,
};

inline constexpr int SCENERY_TYPE_DOOR = static_cast<int>(SceneryType::Door);
inline constexpr int SCENERY_TYPE_STAIRS = static_cast<int>(SceneryType::Stairs);
inline constexpr int SCENERY_TYPE_ELEVATOR = static_cast<int>(SceneryType::Elevator);
inline constexpr int SCENERY_TYPE_LADDER_UP = static_cast<int>(SceneryType::LadderUp);
inline constexpr int SCENERY_TYPE_LADDER_DOWN = static_cast<int>(SceneryType::LadderDown);
inline constexpr int SCENERY_TYPE_GENERIC = static_cast<int>(SceneryType::Generic);
inline constexpr int SCENERY_TYPE_COUNT = static_cast<int>(SceneryType::Count);

enum class MaterialType : int {
    Glass = 0,
    Metal = 1,
    Plastic = 2,
    Wood = 3,
    Dirt = 4,
    Stone = 5,
    Cement = 6,
    Leather = 7,
    Count = 8,
};

inline constexpr int MATERIAL_TYPE_GLASS = static_cast<int>(MaterialType::Glass);
inline constexpr int MATERIAL_TYPE_METAL = static_cast<int>(MaterialType::Metal);
inline constexpr int MATERIAL_TYPE_PLASTIC = static_cast<int>(MaterialType::Plastic);
inline constexpr int MATERIAL_TYPE_WOOD = static_cast<int>(MaterialType::Wood);
inline constexpr int MATERIAL_TYPE_DIRT = static_cast<int>(MaterialType::Dirt);
inline constexpr int MATERIAL_TYPE_STONE = static_cast<int>(MaterialType::Stone);
inline constexpr int MATERIAL_TYPE_CEMENT = static_cast<int>(MaterialType::Cement);
inline constexpr int MATERIAL_TYPE_LEATHER = static_cast<int>(MaterialType::Leather);
inline constexpr int MATERIAL_TYPE_COUNT = static_cast<int>(MaterialType::Count);

enum class DamageType : int {
    Normal = 0,
    Laser = 1,
    Fire = 2,
    Plasma = 3,
    Electrical = 4,
    Emp = 5,
    Explosion = 6,
    Count = 7,
};

inline constexpr int DAMAGE_TYPE_NORMAL = static_cast<int>(DamageType::Normal);
inline constexpr int DAMAGE_TYPE_LASER = static_cast<int>(DamageType::Laser);
inline constexpr int DAMAGE_TYPE_FIRE = static_cast<int>(DamageType::Fire);
inline constexpr int DAMAGE_TYPE_PLASMA = static_cast<int>(DamageType::Plasma);
inline constexpr int DAMAGE_TYPE_ELECTRICAL = static_cast<int>(DamageType::Electrical);
inline constexpr int DAMAGE_TYPE_EMP = static_cast<int>(DamageType::Emp);
inline constexpr int DAMAGE_TYPE_EXPLOSION = static_cast<int>(DamageType::Explosion);
inline constexpr int DAMAGE_TYPE_COUNT = static_cast<int>(DamageType::Count);

enum class CaliberType : int {
    None = 0,
    Rocket = 1,
    FlamethrowerFuel = 2,
    CEnergyCell = 3,
    DEnergyCell = 4,
    Cal223 = 5,
    Mm5 = 6,
    Cal40 = 7,
    Mm10 = 8,
    Cal44 = 9,
    Mm14 = 10,
    Gauge12 = 11,
    Mm9 = 12,
    Bb = 13,
    Count = 14,
};

inline constexpr int CALIBER_TYPE_NONE = static_cast<int>(CaliberType::None);
inline constexpr int CALIBER_TYPE_ROCKET = static_cast<int>(CaliberType::Rocket);
inline constexpr int CALIBER_TYPE_FLAMETHROWER_FUEL = static_cast<int>(CaliberType::FlamethrowerFuel);
inline constexpr int CALIBER_TYPE_C_ENERGY_CELL = static_cast<int>(CaliberType::CEnergyCell);
inline constexpr int CALIBER_TYPE_D_ENERGY_CELL = static_cast<int>(CaliberType::DEnergyCell);
inline constexpr int CALIBER_TYPE_223 = static_cast<int>(CaliberType::Cal223);
inline constexpr int CALIBER_TYPE_5_MM = static_cast<int>(CaliberType::Mm5);
inline constexpr int CALIBER_TYPE_40_CAL = static_cast<int>(CaliberType::Cal40);
inline constexpr int CALIBER_TYPE_10_MM = static_cast<int>(CaliberType::Mm10);
inline constexpr int CALIBER_TYPE_44_CAL = static_cast<int>(CaliberType::Cal44);
inline constexpr int CALIBER_TYPE_14_MM = static_cast<int>(CaliberType::Mm14);
inline constexpr int CALIBER_TYPE_12_GAUGE = static_cast<int>(CaliberType::Gauge12);
inline constexpr int CALIBER_TYPE_9_MM = static_cast<int>(CaliberType::Mm9);
inline constexpr int CALIBER_TYPE_BB = static_cast<int>(CaliberType::Bb);
inline constexpr int CALIBER_TYPE_COUNT = static_cast<int>(CaliberType::Count);

enum class RaceType : int {
    Caucasian = 0,
    African = 1,
    Count = 2,
};

inline constexpr int RACE_TYPE_CAUCASIAN = static_cast<int>(RaceType::Caucasian);
inline constexpr int RACE_TYPE_AFRICAN = static_cast<int>(RaceType::African);
inline constexpr int RACE_TYPE_COUNT = static_cast<int>(RaceType::Count);

enum class BodyType : int {
    Biped = 0,
    Quadruped = 1,
    Robotic = 2,
    Count = 3,
};

inline constexpr int BODY_TYPE_BIPED = static_cast<int>(BodyType::Biped);
inline constexpr int BODY_TYPE_QUADRUPED = static_cast<int>(BodyType::Quadruped);
inline constexpr int BODY_TYPE_ROBOTIC = static_cast<int>(BodyType::Robotic);
inline constexpr int BODY_TYPE_COUNT = static_cast<int>(BodyType::Count);

enum class KillType : int {
    Man = 0,
    Woman = 1,
    Child = 2,
    SuperMutant = 3,
    Ghoul = 4,
    Brahmin = 5,
    Radscorpion = 6,
    Rat = 7,
    Floater = 8,
    Centaur = 9,
    Robot = 10,
    Dog = 11,
    Mantis = 12,
    DeathClaw = 13,
    Plant = 14,
    Count = 15,
};

inline constexpr int KILL_TYPE_MAN = static_cast<int>(KillType::Man);
inline constexpr int KILL_TYPE_WOMAN = static_cast<int>(KillType::Woman);
inline constexpr int KILL_TYPE_CHILD = static_cast<int>(KillType::Child);
inline constexpr int KILL_TYPE_SUPER_MUTANT = static_cast<int>(KillType::SuperMutant);
inline constexpr int KILL_TYPE_GHOUL = static_cast<int>(KillType::Ghoul);
inline constexpr int KILL_TYPE_BRAHMIN = static_cast<int>(KillType::Brahmin);
inline constexpr int KILL_TYPE_RADSCORPION = static_cast<int>(KillType::Radscorpion);
inline constexpr int KILL_TYPE_RAT = static_cast<int>(KillType::Rat);
inline constexpr int KILL_TYPE_FLOATER = static_cast<int>(KillType::Floater);
inline constexpr int KILL_TYPE_CENTAUR = static_cast<int>(KillType::Centaur);
inline constexpr int KILL_TYPE_ROBOT = static_cast<int>(KillType::Robot);
inline constexpr int KILL_TYPE_DOG = static_cast<int>(KillType::Dog);
inline constexpr int KILL_TYPE_MANTIS = static_cast<int>(KillType::Mantis);
inline constexpr int KILL_TYPE_DEATH_CLAW = static_cast<int>(KillType::DeathClaw);
inline constexpr int KILL_TYPE_PLANT = static_cast<int>(KillType::Plant);
inline constexpr int KILL_TYPE_COUNT = static_cast<int>(KillType::Count);

enum class ProtoId : int {
    PowerArmor = 3,
    SmallEnergyCell = 38,
    MicroFusionCell = 39,
    Stimpack = 40,
    Money = 41,
    FirstAidKit = 47,
    Radaway = 48,
    DynamiteI = 51,
    GeigerCounterI = 52,
    Mentats = 53,
    StealthBoyI = 54,
    MotionSensor = 59,
    BigBookOfScience = 73,
    DeansElectronics = 76,
    Flare = 79,
    FirstAidBook = 80,
    PlasticExplosivesI = 85,
    ScoutHandbook = 86,
    BuffOut = 87,
    DoctorsBag = 91,
    GunsAndBullets = 102,
    NukaCola = 106,
    Psycho = 110,
    Beer = 124,
    Booze = 125,
    SuperStimpack = 144,
    MolotovCocktail = 159,
    LitFlare = 205,
    DynamiteII = 206, // armed
    GeigerCounterII = 207,
    PlasticExplosivesII = 209, // armed
    StealthBoyII = 210,
    HardenedPowerArmor = 232,
};

inline constexpr int PROTO_ID_POWER_ARMOR = static_cast<int>(ProtoId::PowerArmor);
inline constexpr int PROTO_ID_SMALL_ENERGY_CELL = static_cast<int>(ProtoId::SmallEnergyCell);
inline constexpr int PROTO_ID_MICRO_FUSION_CELL = static_cast<int>(ProtoId::MicroFusionCell);
inline constexpr int PROTO_ID_STIMPACK = static_cast<int>(ProtoId::Stimpack);
inline constexpr int PROTO_ID_MONEY = static_cast<int>(ProtoId::Money);
inline constexpr int PROTO_ID_FIRST_AID_KIT = static_cast<int>(ProtoId::FirstAidKit);
inline constexpr int PROTO_ID_RADAWAY = static_cast<int>(ProtoId::Radaway);
inline constexpr int PROTO_ID_DYNAMITE_I = static_cast<int>(ProtoId::DynamiteI);
inline constexpr int PROTO_ID_GEIGER_COUNTER_I = static_cast<int>(ProtoId::GeigerCounterI);
inline constexpr int PROTO_ID_MENTATS = static_cast<int>(ProtoId::Mentats);
inline constexpr int PROTO_ID_STEALTH_BOY_I = static_cast<int>(ProtoId::StealthBoyI);
inline constexpr int PROTO_ID_MOTION_SENSOR = static_cast<int>(ProtoId::MotionSensor);
inline constexpr int PROTO_ID_BIG_BOOK_OF_SCIENCE = static_cast<int>(ProtoId::BigBookOfScience);
inline constexpr int PROTO_ID_DEANS_ELECTRONICS = static_cast<int>(ProtoId::DeansElectronics);
inline constexpr int PROTO_ID_FLARE = static_cast<int>(ProtoId::Flare);
inline constexpr int PROTO_ID_FIRST_AID_BOOK = static_cast<int>(ProtoId::FirstAidBook);
inline constexpr int PROTO_ID_PLASTIC_EXPLOSIVES_I = static_cast<int>(ProtoId::PlasticExplosivesI);
inline constexpr int PROTO_ID_SCOUT_HANDBOOK = static_cast<int>(ProtoId::ScoutHandbook);
inline constexpr int PROTO_ID_BUFF_OUT = static_cast<int>(ProtoId::BuffOut);
inline constexpr int PROTO_ID_DOCTORS_BAG = static_cast<int>(ProtoId::DoctorsBag);
inline constexpr int PROTO_ID_GUNS_AND_BULLETS = static_cast<int>(ProtoId::GunsAndBullets);
inline constexpr int PROTO_ID_NUKA_COLA = static_cast<int>(ProtoId::NukaCola);
inline constexpr int PROTO_ID_PSYCHO = static_cast<int>(ProtoId::Psycho);
inline constexpr int PROTO_ID_BEER = static_cast<int>(ProtoId::Beer);
inline constexpr int PROTO_ID_BOOZE = static_cast<int>(ProtoId::Booze);
inline constexpr int PROTO_ID_SUPER_STIMPACK = static_cast<int>(ProtoId::SuperStimpack);
inline constexpr int PROTO_ID_MOLOTOV_COCKTAIL = static_cast<int>(ProtoId::MolotovCocktail);
inline constexpr int PROTO_ID_LIT_FLARE = static_cast<int>(ProtoId::LitFlare);
inline constexpr int PROTO_ID_DYNAMITE_II = static_cast<int>(ProtoId::DynamiteII);
inline constexpr int PROTO_ID_GEIGER_COUNTER_II = static_cast<int>(ProtoId::GeigerCounterII);
inline constexpr int PROTO_ID_PLASTIC_EXPLOSIVES_II = static_cast<int>(ProtoId::PlasticExplosivesII);
inline constexpr int PROTO_ID_STEALTH_BOY_II = static_cast<int>(ProtoId::StealthBoyII);
inline constexpr int PROTO_ID_HARDENED_POWER_ARMOR = static_cast<int>(ProtoId::HardenedPowerArmor);

constexpr int PROTO_ID_0x1000098 = 0x1000098;
constexpr int PROTO_ID_0x10001E0 = 0x10001E0;
constexpr int PROTO_ID_0x2000031 = 0x2000031;
constexpr int PROTO_ID_0x2000158 = 0x2000158;
constexpr int PROTO_ID_CAR = 0x20003F1;
constexpr int PROTO_ID_0x200050D = 0x200050D;
constexpr int PROTO_ID_0x2000099 = 0x2000099;
constexpr int PROTO_ID_0x20001A5 = 0x20001A5;
constexpr int PROTO_ID_0x20001D6 = 0x20001D6;
constexpr int PROTO_ID_0x20001EB = 0x20001EB;
constexpr int FID_0x20001F5 = 0x20001F5;
// first exit grid
constexpr int PROTO_ID_0x5000010 = 0x5000010;
// last exit grid
constexpr int PROTO_ID_0x5000017 = 0x5000017;

enum class ItemProtoFlags : unsigned {
    Flag_0x08 = 0x08,
    Flag_0x10 = 0x10,
    Flag_0x1000 = 0x1000,
    Flag_0x8000 = 0x8000,
    Flag_0x20000000 = 0x20000000,
    Flag_0x80000000 = 0x80000000,
};
DEFINE_ENUM_FLAG_OPERATORS(ItemProtoFlags)

inline constexpr unsigned ItemProtoFlags_0x08 = static_cast<unsigned>(ItemProtoFlags::Flag_0x08);
inline constexpr unsigned ItemProtoFlags_0x10 = static_cast<unsigned>(ItemProtoFlags::Flag_0x10);
inline constexpr unsigned ItemProtoFlags_0x1000 = static_cast<unsigned>(ItemProtoFlags::Flag_0x1000);
inline constexpr unsigned ItemProtoFlags_0x8000 = static_cast<unsigned>(ItemProtoFlags::Flag_0x8000);
inline constexpr unsigned ItemProtoFlags_0x20000000 = static_cast<unsigned>(ItemProtoFlags::Flag_0x20000000);
inline constexpr unsigned ItemProtoFlags_0x80000000 = static_cast<unsigned>(ItemProtoFlags::Flag_0x80000000);


enum class ItemProtoExtendedFlags : unsigned {
    BigGun = 0x0100,
    IsTwoHanded = 0x0200,
    Flag_0x0800 = 0x0800,
    Flag_0x1000 = 0x1000,
    Flag_0x2000 = 0x2000,
    Flag_0x8000 = 0x8000,

    // This flag is used on weapons to indicate that's a natural (integral)
    // part of its owner, for example Claw, or Robot's Rocket Launcher. Items
    // with this flag on do count toward total weight and cannot be dropped.
    NaturalWeapon = 0x08000000,
};
DEFINE_ENUM_FLAG_OPERATORS(ItemProtoExtendedFlags)

inline constexpr unsigned ItemProtoExtendedFlags_BigGun = static_cast<unsigned>(ItemProtoExtendedFlags::BigGun);
inline constexpr unsigned ItemProtoExtendedFlags_IsTwoHanded = static_cast<unsigned>(ItemProtoExtendedFlags::IsTwoHanded);
inline constexpr unsigned ItemProtoExtendedFlags_0x0800 = static_cast<unsigned>(ItemProtoExtendedFlags::Flag_0x0800);
inline constexpr unsigned ItemProtoExtendedFlags_0x1000 = static_cast<unsigned>(ItemProtoExtendedFlags::Flag_0x1000);
inline constexpr unsigned ItemProtoExtendedFlags_0x2000 = static_cast<unsigned>(ItemProtoExtendedFlags::Flag_0x2000);
inline constexpr unsigned ItemProtoExtendedFlags_0x8000 = static_cast<unsigned>(ItemProtoExtendedFlags::Flag_0x8000);
inline constexpr unsigned ItemProtoExtendedFlags_NaturalWeapon = static_cast<unsigned>(ItemProtoExtendedFlags::NaturalWeapon);


struct ProtoItemArmorData {
    int armorClass; // d.ac
    int damageResistance[7]; // d.dam_resist
    int damageThreshold[7]; // d.dam_thresh
    int perk; // d.perk
    int maleFid; // d.male_fid
    int femaleFid; // d.female_fid
};

struct ProtoItemContainerData {
    int maxSize; // d.max_size
    int openFlags; // d.open_flags
};

struct ProtoItemDrugData {
    int stat[3]; // d.stat
    int amount[3]; // d.amount
    int duration1; // d.duration1
    int amount1[3]; // d.amount1
    int duration2; // d.duration2
    int amount2[3]; // d.amount2
    int addictionChance; // d.addiction_chance
    int withdrawalEffect; // d.withdrawal_effect
    int withdrawalOnset; // d.withdrawal_onset
};

struct ProtoItemWeaponData {
    int animationCode; // d.animation_code
    int minDamage; // d.min_damage
    int maxDamage; // d.max_damage
    int damageType; // d.dt
    int maxRange1; // d.max_range1
    int maxRange2; // d.max_range2
    int projectilePid; // d.proj_pid
    int minStrength; // d.min_st
    int actionPointCost1; // d.mp_cost1
    int actionPointCost2; // d.mp_cost2
    int criticalFailureType; // d.crit_fail_table
    int perk; // d.perk
    int rounds; // d.rounds
    int caliber; // d.caliber
    int ammoTypePid; // d.ammo_type_pid
    int ammoCapacity; // d.max_ammo
    unsigned char soundCode; // d.sound_id
};

struct ProtoItemAmmoData {
    int caliber; // d.caliber
    int quantity; // d.quantity
    int armorClassModifier; // d.ac_adjust
    int damageResistanceModifier; // d.dr_adjust
    int damageMultiplier; // d.dam_mult
    int damageDivisor; // d.dam_div
};

struct ProtoItemMiscData {
    int powerTypePid; // d.power_type_pid
    int powerType; // d.power_type
    int charges; // d.charges
};

struct ProtoItemKeyData {
    int keyCode; // d.key_code
};

struct ItemProtoData {
    union {
        struct {
            int field_0;
            int field_4;
            int field_8; // max charges
            int field_C;
            int field_10;
            int field_14;
            int field_18;
        } unknown;
        ProtoItemArmorData armor;
        ProtoItemContainerData container;
        ProtoItemDrugData drug;
        ProtoItemWeaponData weapon;
        ProtoItemAmmoData ammo;
        ProtoItemMiscData misc;
        ProtoItemKeyData key;
    };
};

struct ItemProto {
    int pid; // pid
    int messageId; // message_num
    int fid; // fid
    int lightDistance; // light_distance
    int lightIntensity; // light_intensity
    int flags; // flags
    int extendedFlags; // flags_ext
    int sid; // sid
    int type; // type
    ItemProtoData data; // d
    int material; // material
    int size; // size
    int weight; // weight
    int cost; // cost
    int inventoryFid; // inv_fid
    unsigned char field_80;
};

class CritterProtoData {
public:
    int flags; // d.flags
    int baseStats[35]; // d.stat_base
    int bonusStats[35]; // d.stat_bonus
    int skills[18]; // d.stat_points
    int bodyType; // d.body
    int experience;
    int killType;

    void copyFrom(CritterProtoData* src);
    int loadData(const char* path);
    int readData(DB_FILE* stream);
    int saveData(const char* path);
    int writeData(DB_FILE* stream);
    void setSkillDefaults();
    void setStatDefaults();
};

struct CritterProto {
    int pid; // pid
    int messageId; // message_num
    int fid; // fid
    int lightDistance; // light_distance
    int lightIntensity; // light_intensity
    int flags; // flags
    int extendedFlags; // flags_ext
    int sid; // sid
    CritterProtoData data; // d
    int headFid; // head_fid
    int aiPacket; // ai_packet
    int team; // team_num
};

struct SceneryProtoDoorData {
    int openFlags; // d.open_flags
    int keyCode; // d.key_code
};

struct SceneryProtoStairsData {
    int field_0; // d.lower_tile
    int field_4; // d.upper_tile
};

struct SceneryProtoElevatorData {
    int type;
    int level;
};

struct SceneryProtoLadderData {
    int field_0;
};

struct SceneryProtoGenericData {
    int field_0;
};

struct SceneryProtoData {
    union {
        SceneryProtoDoorData door;
        SceneryProtoStairsData stairs;
        SceneryProtoElevatorData elevator;
        SceneryProtoLadderData ladder;
        SceneryProtoGenericData generic;
    };
};

struct SceneryProto {
    int pid; // id
    int messageId; // message_num
    int fid; // fid
    int lightDistance; // light_distance
    int lightIntensity; // light_intensity
    int flags; // flags
    int extendedFlags; // flags_ext
    int sid; // sid
    int type; // type
    SceneryProtoData data;
    int material;
    int field_30; //
    unsigned char field_34;
};

struct WallProto {
    int pid; // id
    int messageId; // message_num
    int fid; // fid
    int lightDistance; // light_distance
    int lightIntensity; // light_intensity
    int flags; // flags
    int extendedFlags; // flags_ext
    int sid; // sid
    int material; // material
};

struct TileProto {
    int pid; // id
    int messageId; // message_num
    int fid; // fid
    int flags; // flags
    int extendedFlags; // flags_ext
    int sid; // sid
    int material; // material
};

struct MiscProto {
    int pid; // id
    int messageId; // message_num
    int fid; // fid
    int lightDistance; // light_distance
    int lightIntensity; // light_intensity
    int flags; // flags
    int extendedFlags; // flags_ext
};

union Proto {
    struct {
        int pid; // pid
        int messageId; // message_num
        int fid; // fid

        // TODO: Move to NonTile props?
        int lightDistance;
        int lightIntensity;
        int flags;
        int extendedFlags;
        int sid;
    };
    ItemProto item;
    CritterProto critter;
    SceneryProto scenery;
    WallProto wall;
    TileProto tile;
    MiscProto misc;
};

struct ProtoListExtent {
    Proto* proto[PROTO_LIST_EXTENT_SIZE];
    // Number of protos in the extent
    int length;
    struct ProtoListExtent* next;
};

struct ProtoList {
    ProtoListExtent* head;
    ProtoListExtent* tail;
    // Number of extents in the list.
    int length;
    // Number of lines in proto/{type}/{type}.lst.
    int max_entries_num;
};

} // namespace fallout

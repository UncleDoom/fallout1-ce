#pragma once


#include "game/enum_utils.h"
#include "plib/db/db.h"

namespace fallout {

enum class MapFlags : unsigned {
    Saved = 0x01,
    DeadBodiesAge = 0x02,
    PipboyActive = 0x04,
    CanRestElevation0 = 0x08,
    CanRestElevation1 = 0x10,
    CanRestElevation2 = 0x20,
};

DEFINE_ENUM_FLAG_OPERATORS(MapFlags)

inline constexpr int MAP_SAVED = static_cast<int>(MapFlags::Saved);
inline constexpr int MAP_DEAD_BODIES_AGE = static_cast<int>(MapFlags::DeadBodiesAge);
inline constexpr int MAP_PIPBOY_ACTIVE = static_cast<int>(MapFlags::PipboyActive);
inline constexpr int MAP_CAN_REST_ELEVATION_0 = static_cast<int>(MapFlags::CanRestElevation0);
inline constexpr int MAP_CAN_REST_ELEVATION_1 = static_cast<int>(MapFlags::CanRestElevation1);
inline constexpr int MAP_CAN_REST_ELEVATION_2 = static_cast<int>(MapFlags::CanRestElevation2);


enum class City : int {
    Vault13 = 0,
    Vault15 = 1,
    ShadySands = 2,
    Junktown = 3,
    Raiders = 4,
    Necropolis = 5,
    TheHub = 6,
    Brotherhood = 7,
    MilitaryBase = 8,
    TheGlow = 9,
    Boneyard = 10,
    Cathedral = 11,
    Count = 12,
    Special12 = 12,
    Special13 = 13,
    Special14 = 14,
};

inline constexpr int TOWN_VAULT_13 = static_cast<int>(City::Vault13);
inline constexpr int TOWN_VAULT_15 = static_cast<int>(City::Vault15);
inline constexpr int TOWN_SHADY_SANDS = static_cast<int>(City::ShadySands);
inline constexpr int TOWN_JUNKTOWN = static_cast<int>(City::Junktown);
inline constexpr int TOWN_RAIDERS = static_cast<int>(City::Raiders);
inline constexpr int TOWN_NECROPOLIS = static_cast<int>(City::Necropolis);
inline constexpr int TOWN_THE_HUB = static_cast<int>(City::TheHub);
inline constexpr int TOWN_BROTHERHOOD = static_cast<int>(City::Brotherhood);
inline constexpr int TOWN_MILITARY_BASE = static_cast<int>(City::MilitaryBase);
inline constexpr int TOWN_THE_GLOW = static_cast<int>(City::TheGlow);
inline constexpr int TOWN_BONEYARD = static_cast<int>(City::Boneyard);
inline constexpr int TOWN_CATHEDRAL = static_cast<int>(City::Cathedral);
inline constexpr int TOWN_COUNT = static_cast<int>(City::Count);
inline constexpr int TOWN_SPECIAL_12 = static_cast<int>(City::Special12);
inline constexpr int TOWN_SPECIAL_13 = static_cast<int>(City::Special13);
inline constexpr int TOWN_SPECIAL_14 = static_cast<int>(City::Special14);

enum class Map : int {
    Desert1 = 0,
    Desert2 = 1,
    Desert3 = 2,
    Hallded = 3,
    Hotel = 4,
    Watrshd = 5,
    Vault13 = 6,
    Vaultent = 7,
    Vaultbur = 8,
    Vaultnec = 9,
    Junkent = 10,
    Junkcsno = 11,
    Junkkill = 12,
    Brohdent = 13,
    Brohd12 = 14,
    Brohd34 = 15,
    Caves = 16,
    Childrn1 = 17,
    Childrn2 = 18,
    City1 = 19,
    Coast1 = 20,
    Coast2 = 21,
    Colatruk = 22,
    Fsauser = 23,
    Raiders = 24,
    Shadye = 25,
    Shadyw = 26,
    Glowent = 27,
    Laadytum = 28,
    Lafollwr = 29,
    Mbent = 30,
    Mbstrg12 = 31,
    Mbvats12 = 32,
    Mstrlr12 = 33,
    Mstrlr34 = 34,
    V13ent = 35,
    Hubent = 36,
    Dethclaw = 37,
    Hubdwntn = 38,
    Hubheigt = 39,
    Huboldtn = 40,
    Hubwater = 41,
    Glow1 = 42,
    Glow2 = 43,
    Lablades = 44,
    Laripper = 45,
    Lagunrun = 46,
    Childead = 47,
    Mbdead = 48,
    Mountn1 = 49,
    Mountn2 = 50,
    Foot = 51,
    Tardis = 52,
    Talkcow = 53,
    Usedcar = 54,
    Brodead = 55,
    Descrvn1 = 56,
    Descrvn2 = 57,
    Mntcrvn1 = 58,
    Mntcrvn2 = 59,
    Vipers = 60,
    Descrvn3 = 61,
    Mntcrvn3 = 62,
    Descrvn4 = 63,
    Mntcrvn4 = 64,
    Hubmis1 = 65,
    Count = 66,
};

inline constexpr int MAP_DESERT1 = static_cast<int>(Map::Desert1);
inline constexpr int MAP_DESERT2 = static_cast<int>(Map::Desert2);
inline constexpr int MAP_DESERT3 = static_cast<int>(Map::Desert3);
inline constexpr int MAP_HALLDED = static_cast<int>(Map::Hallded);
inline constexpr int MAP_HOTEL = static_cast<int>(Map::Hotel);
inline constexpr int MAP_WATRSHD = static_cast<int>(Map::Watrshd);
inline constexpr int MAP_VAULT13 = static_cast<int>(Map::Vault13);
inline constexpr int MAP_VAULTENT = static_cast<int>(Map::Vaultent);
inline constexpr int MAP_VAULTBUR = static_cast<int>(Map::Vaultbur);
inline constexpr int MAP_VAULTNEC = static_cast<int>(Map::Vaultnec);
inline constexpr int MAP_JUNKENT = static_cast<int>(Map::Junkent);
inline constexpr int MAP_JUNKCSNO = static_cast<int>(Map::Junkcsno);
inline constexpr int MAP_JUNKKILL = static_cast<int>(Map::Junkkill);
inline constexpr int MAP_BROHDENT = static_cast<int>(Map::Brohdent);
inline constexpr int MAP_BROHD12 = static_cast<int>(Map::Brohd12);
inline constexpr int MAP_BROHD34 = static_cast<int>(Map::Brohd34);
inline constexpr int MAP_CAVES = static_cast<int>(Map::Caves);
inline constexpr int MAP_CHILDRN1 = static_cast<int>(Map::Childrn1);
inline constexpr int MAP_CHILDRN2 = static_cast<int>(Map::Childrn2);
inline constexpr int MAP_CITY1 = static_cast<int>(Map::City1);
inline constexpr int MAP_COAST1 = static_cast<int>(Map::Coast1);
inline constexpr int MAP_COAST2 = static_cast<int>(Map::Coast2);
inline constexpr int MAP_COLATRUK = static_cast<int>(Map::Colatruk);
inline constexpr int MAP_FSAUSER = static_cast<int>(Map::Fsauser);
inline constexpr int MAP_RAIDERS = static_cast<int>(Map::Raiders);
inline constexpr int MAP_SHADYE = static_cast<int>(Map::Shadye);
inline constexpr int MAP_SHADYW = static_cast<int>(Map::Shadyw);
inline constexpr int MAP_GLOWENT = static_cast<int>(Map::Glowent);
inline constexpr int MAP_LAADYTUM = static_cast<int>(Map::Laadytum);
inline constexpr int MAP_LAFOLLWR = static_cast<int>(Map::Lafollwr);
inline constexpr int MAP_MBENT = static_cast<int>(Map::Mbent);
inline constexpr int MAP_MBSTRG12 = static_cast<int>(Map::Mbstrg12);
inline constexpr int MAP_MBVATS12 = static_cast<int>(Map::Mbvats12);
inline constexpr int MAP_MSTRLR12 = static_cast<int>(Map::Mstrlr12);
inline constexpr int MAP_MSTRLR34 = static_cast<int>(Map::Mstrlr34);
inline constexpr int MAP_V13ENT = static_cast<int>(Map::V13ent);
inline constexpr int MAP_HUBENT = static_cast<int>(Map::Hubent);
inline constexpr int MAP_DETHCLAW = static_cast<int>(Map::Dethclaw);
inline constexpr int MAP_HUBDWNTN = static_cast<int>(Map::Hubdwntn);
inline constexpr int MAP_HUBHEIGT = static_cast<int>(Map::Hubheigt);
inline constexpr int MAP_HUBOLDTN = static_cast<int>(Map::Huboldtn);
inline constexpr int MAP_HUBWATER = static_cast<int>(Map::Hubwater);
inline constexpr int MAP_GLOW1 = static_cast<int>(Map::Glow1);
inline constexpr int MAP_GLOW2 = static_cast<int>(Map::Glow2);
inline constexpr int MAP_LABLADES = static_cast<int>(Map::Lablades);
inline constexpr int MAP_LARIPPER = static_cast<int>(Map::Laripper);
inline constexpr int MAP_LAGUNRUN = static_cast<int>(Map::Lagunrun);
inline constexpr int MAP_CHILDEAD = static_cast<int>(Map::Childead);
inline constexpr int MAP_MBDEAD = static_cast<int>(Map::Mbdead);
inline constexpr int MAP_MOUNTN1 = static_cast<int>(Map::Mountn1);
inline constexpr int MAP_MOUNTN2 = static_cast<int>(Map::Mountn2);
inline constexpr int MAP_FOOT = static_cast<int>(Map::Foot);
inline constexpr int MAP_TARDIS = static_cast<int>(Map::Tardis);
inline constexpr int MAP_TALKCOW = static_cast<int>(Map::Talkcow);
inline constexpr int MAP_USEDCAR = static_cast<int>(Map::Usedcar);
inline constexpr int MAP_BRODEAD = static_cast<int>(Map::Brodead);
inline constexpr int MAP_DESCRVN1 = static_cast<int>(Map::Descrvn1);
inline constexpr int MAP_DESCRVN2 = static_cast<int>(Map::Descrvn2);
inline constexpr int MAP_MNTCRVN1 = static_cast<int>(Map::Mntcrvn1);
inline constexpr int MAP_MNTCRVN2 = static_cast<int>(Map::Mntcrvn2);
inline constexpr int MAP_VIPERS = static_cast<int>(Map::Vipers);
inline constexpr int MAP_DESCRVN3 = static_cast<int>(Map::Descrvn3);
inline constexpr int MAP_MNTCRVN3 = static_cast<int>(Map::Mntcrvn3);
inline constexpr int MAP_DESCRVN4 = static_cast<int>(Map::Descrvn4);
inline constexpr int MAP_MNTCRVN4 = static_cast<int>(Map::Mntcrvn4);
inline constexpr int MAP_HUBMIS1 = static_cast<int>(Map::Hubmis1);
inline constexpr int MAP_COUNT = static_cast<int>(Map::Count);

enum class TerrainType : int {
    Desert = 0,
    Mountain = 1,
    City = 2,
    Coast = 3,
};

inline constexpr int TERRAIN_TYPE_DESERT = static_cast<int>(TerrainType::Desert);
inline constexpr int TERRAIN_TYPE_MOUNTAIN = static_cast<int>(TerrainType::Mountain);
inline constexpr int TERRAIN_TYPE_CITY = static_cast<int>(TerrainType::City);
inline constexpr int TERRAIN_TYPE_COAST = static_cast<int>(TerrainType::Coast);

enum class WorldmapFrm : int {
    LittleRedButtonNormal = 0,
    LittleRedButtonPressed = 1,
    Box = 2,
    Labels = 3,
    LocationMarker = 4,
    DestinationMarkerBright = 5,
    DestinationMarkerDark = 6,
    RandomEncounterBright = 7,
    RandomEncounterDark = 8,
    Worldmap = 9,
    Months = 10,
    Numbers = 11,
    HotspotNormal = 12,
    HotspotPressed = 13,
    Count = 14,
};

inline constexpr int WORLDMAP_FRM_LITTLE_RED_BUTTON_NORMAL = static_cast<int>(WorldmapFrm::LittleRedButtonNormal);
inline constexpr int WORLDMAP_FRM_LITTLE_RED_BUTTON_PRESSED = static_cast<int>(WorldmapFrm::LittleRedButtonPressed);
inline constexpr int WORLDMAP_FRM_BOX = static_cast<int>(WorldmapFrm::Box);
inline constexpr int WORLDMAP_FRM_LABELS = static_cast<int>(WorldmapFrm::Labels);
inline constexpr int WORLDMAP_FRM_LOCATION_MARKER = static_cast<int>(WorldmapFrm::LocationMarker);
inline constexpr int WORLDMAP_FRM_DESTINATION_MARKER_BRIGHT = static_cast<int>(WorldmapFrm::DestinationMarkerBright);
inline constexpr int WORLDMAP_FRM_DESTINATION_MARKER_DARK = static_cast<int>(WorldmapFrm::DestinationMarkerDark);
inline constexpr int WORLDMAP_FRM_RANDOM_ENCOUNTER_BRIGHT = static_cast<int>(WorldmapFrm::RandomEncounterBright);
inline constexpr int WORLDMAP_FRM_RANDOM_ENCOUNTER_DARK = static_cast<int>(WorldmapFrm::RandomEncounterDark);
inline constexpr int WORLDMAP_FRM_WORLDMAP = static_cast<int>(WorldmapFrm::Worldmap);
inline constexpr int WORLDMAP_FRM_MONTHS = static_cast<int>(WorldmapFrm::Months);
inline constexpr int WORLDMAP_FRM_NUMBERS = static_cast<int>(WorldmapFrm::Numbers);
inline constexpr int WORLDMAP_FRM_HOTSPOT_NORMAL = static_cast<int>(WorldmapFrm::HotspotNormal);
inline constexpr int WORLDMAP_FRM_HOTSPOT_PRESSED = static_cast<int>(WorldmapFrm::HotspotPressed);
inline constexpr int WORLDMAP_FRM_COUNT = static_cast<int>(WorldmapFrm::Count);

enum class TownmapFrm : int {
    Box = 0,
    Labels = 1,
    HotspotPressed = 2,
    HotspotNormal = 3,
    LittleRedButtonNormal = 4,
    LittleRedButtonPressed = 5,
    Months = 6,
    Numbers = 7,
    Count = 8,
};

inline constexpr int TOWNMAP_FRM_BOX = static_cast<int>(TownmapFrm::Box);
inline constexpr int TOWNMAP_FRM_LABELS = static_cast<int>(TownmapFrm::Labels);
inline constexpr int TOWNMAP_FRM_HOTSPOT_PRESSED = static_cast<int>(TownmapFrm::HotspotPressed);
inline constexpr int TOWNMAP_FRM_HOTSPOT_NORMAL = static_cast<int>(TownmapFrm::HotspotNormal);
inline constexpr int TOWNMAP_FRM_LITTLE_RED_BUTTON_NORMAL = static_cast<int>(TownmapFrm::LittleRedButtonNormal);
inline constexpr int TOWNMAP_FRM_LITTLE_RED_BUTTON_PRESSED = static_cast<int>(TownmapFrm::LittleRedButtonPressed);
inline constexpr int TOWNMAP_FRM_MONTHS = static_cast<int>(TownmapFrm::Months);
inline constexpr int TOWNMAP_FRM_NUMBERS = static_cast<int>(TownmapFrm::Numbers);
inline constexpr int TOWNMAP_FRM_COUNT = static_cast<int>(TownmapFrm::Count);

struct WorldMapContext {
    short state;
    short town;
    short section;
};

extern int world_win;
extern int our_section;
extern int our_town;

int init_world_map();
int save_world_map(DB_FILE* stream);
int load_world_map(DB_FILE* stream);
int world_map(WorldMapContext ctx);
WorldMapContext town_map(WorldMapContext ctx);
void KillWorldWin();
int worldmap_script_jump(int city, int a2);
int xlate_mapidx_to_town(int map_idx);
int PlayCityMapMusic();

} // namespace fallout

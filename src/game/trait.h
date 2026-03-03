#pragma once


#include "plib/db/db.h"

namespace fallout {

// The maximum number of traits a player is allowed to select.
inline constexpr int PC_TRAIT_MAX = 2;

// Available traits.
enum class Trait : int {
    FastMetabolism = 0,
    Bruiser = 1,
    SmallFrame = 2,
    OneHander = 3,
    Finesse = 4,
    Kamikaze = 5,
    HeavyHanded = 6,
    FastShot = 7,
    BloodyMess = 8,
    Jinxed = 9,
    GoodNatured = 10,
    ChemReliant = 11,
    ChemResistant = 12,
    NightPerson = 13,
    Skilled = 14,
    Gifted = 15,
    Count = 16,
};

inline constexpr int TRAIT_FAST_METABOLISM = static_cast<int>(Trait::FastMetabolism);
inline constexpr int TRAIT_BRUISER = static_cast<int>(Trait::Bruiser);
inline constexpr int TRAIT_SMALL_FRAME = static_cast<int>(Trait::SmallFrame);
inline constexpr int TRAIT_ONE_HANDER = static_cast<int>(Trait::OneHander);
inline constexpr int TRAIT_FINESSE = static_cast<int>(Trait::Finesse);
inline constexpr int TRAIT_KAMIKAZE = static_cast<int>(Trait::Kamikaze);
inline constexpr int TRAIT_HEAVY_HANDED = static_cast<int>(Trait::HeavyHanded);
inline constexpr int TRAIT_FAST_SHOT = static_cast<int>(Trait::FastShot);
inline constexpr int TRAIT_BLOODY_MESS = static_cast<int>(Trait::BloodyMess);
inline constexpr int TRAIT_JINXED = static_cast<int>(Trait::Jinxed);
inline constexpr int TRAIT_GOOD_NATURED = static_cast<int>(Trait::GoodNatured);
inline constexpr int TRAIT_CHEM_RELIANT = static_cast<int>(Trait::ChemReliant);
inline constexpr int TRAIT_CHEM_RESISTANT = static_cast<int>(Trait::ChemResistant);
inline constexpr int TRAIT_NIGHT_PERSON = static_cast<int>(Trait::NightPerson);
inline constexpr int TRAIT_SKILLED = static_cast<int>(Trait::Skilled);
inline constexpr int TRAIT_GIFTED = static_cast<int>(Trait::Gifted);
inline constexpr int TRAIT_COUNT = static_cast<int>(Trait::Count);

int trait_init();
void trait_reset();
void trait_exit();
int trait_load(DB_FILE* stream);
int trait_save(DB_FILE* stream);
void trait_set(int trait1, int trait2);
void trait_get(int* trait1, int* trait2);
char* trait_name(int trait);
char* trait_description(int trait);
int trait_pic(int trait);
int trait_level(int trait);
int trait_adjust_stat(int stat);
int trait_adjust_skill(int skill);

} // namespace fallout

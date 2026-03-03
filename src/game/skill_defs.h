#pragma once

#include <type_traits>

namespace fallout {

// max number of tagged skills
constexpr int NUM_TAGGED_SKILLS = 4;

constexpr int DEFAULT_TAGGED_SKILLS = 3;

// Available skills.
enum class Skill : int {
    SmallGuns,
    BigGuns,
    EnergyWeapons,
    Unarmed,
    MeleeWeapons,
    Throwing,
    FirstAid,
    Doctor,
    Sneak,
    Lockpick,
    Steal,
    Traps,
    Science,
    Repair,
    Speech,
    Barter,
    Gambling,
    Outdoorsman,
    Count,
};

// Legacy constants for backward compatibility — used as array indices.
inline constexpr int SKILL_SMALL_GUNS = static_cast<int>(Skill::SmallGuns);
inline constexpr int SKILL_BIG_GUNS = static_cast<int>(Skill::BigGuns);
inline constexpr int SKILL_ENERGY_WEAPONS = static_cast<int>(Skill::EnergyWeapons);
inline constexpr int SKILL_UNARMED = static_cast<int>(Skill::Unarmed);
inline constexpr int SKILL_MELEE_WEAPONS = static_cast<int>(Skill::MeleeWeapons);
inline constexpr int SKILL_THROWING = static_cast<int>(Skill::Throwing);
inline constexpr int SKILL_FIRST_AID = static_cast<int>(Skill::FirstAid);
inline constexpr int SKILL_DOCTOR = static_cast<int>(Skill::Doctor);
inline constexpr int SKILL_SNEAK = static_cast<int>(Skill::Sneak);
inline constexpr int SKILL_LOCKPICK = static_cast<int>(Skill::Lockpick);
inline constexpr int SKILL_STEAL = static_cast<int>(Skill::Steal);
inline constexpr int SKILL_TRAPS = static_cast<int>(Skill::Traps);
inline constexpr int SKILL_SCIENCE = static_cast<int>(Skill::Science);
inline constexpr int SKILL_REPAIR = static_cast<int>(Skill::Repair);
inline constexpr int SKILL_SPEECH = static_cast<int>(Skill::Speech);
inline constexpr int SKILL_BARTER = static_cast<int>(Skill::Barter);
inline constexpr int SKILL_GAMBLING = static_cast<int>(Skill::Gambling);
inline constexpr int SKILL_OUTDOORSMAN = static_cast<int>(Skill::Outdoorsman);
inline constexpr int SKILL_COUNT = static_cast<int>(Skill::Count);

} // namespace fallout

#pragma once

#include <type_traits>

namespace fallout {

// The minimum value of SPECIAL stat.
constexpr int PRIMARY_STAT_MIN = 1;

// The maximum value of SPECIAL stat.
constexpr int PRIMARY_STAT_MAX = 10;

// The number of values of SPECIAL stat.
constexpr int PRIMARY_STAT_RANGE = PRIMARY_STAT_MAX - PRIMARY_STAT_MIN + 1;

// The maximum number of PC level.
constexpr int PC_LEVEL_MAX = 21;

// Available stats.
enum class Stat : int {
    Strength,
    Perception,
    Endurance,
    Charisma,
    Intelligence,
    Agility,
    Luck,
    MaximumHitPoints,
    MaximumActionPoints,
    ArmorClass,
    UnarmedDamage,
    MeleeDamage,
    CarryWeight,
    Sequence,
    HealingRate,
    CriticalChance,
    BetterCriticals,
    DamageThreshold,
    DamageThresholdLaser,
    DamageThresholdFire,
    DamageThresholdPlasma,
    DamageThresholdElectrical,
    DamageThresholdEmp,
    DamageThresholdExplosion,
    DamageResistance,
    DamageResistanceLaser,
    DamageResistanceFire,
    DamageResistancePlasma,
    DamageResistanceElectrical,
    DamageResistanceEmp,
    DamageResistanceExplosion,
    RadiationResistance,
    PoisonResistance,
    Age,
    Gender,
    CurrentHitPoints,
    CurrentPoisonLevel,
    CurrentRadiationLevel,
    Count,
};

// Legacy constants for backward compatibility — used as array indices.
inline constexpr int STAT_STRENGTH = static_cast<int>(Stat::Strength);
inline constexpr int STAT_PERCEPTION = static_cast<int>(Stat::Perception);
inline constexpr int STAT_ENDURANCE = static_cast<int>(Stat::Endurance);
inline constexpr int STAT_CHARISMA = static_cast<int>(Stat::Charisma);
inline constexpr int STAT_INTELLIGENCE = static_cast<int>(Stat::Intelligence);
inline constexpr int STAT_AGILITY = static_cast<int>(Stat::Agility);
inline constexpr int STAT_LUCK = static_cast<int>(Stat::Luck);
inline constexpr int STAT_MAXIMUM_HIT_POINTS = static_cast<int>(Stat::MaximumHitPoints);
inline constexpr int STAT_MAXIMUM_ACTION_POINTS = static_cast<int>(Stat::MaximumActionPoints);
inline constexpr int STAT_ARMOR_CLASS = static_cast<int>(Stat::ArmorClass);
inline constexpr int STAT_UNARMED_DAMAGE = static_cast<int>(Stat::UnarmedDamage);
inline constexpr int STAT_MELEE_DAMAGE = static_cast<int>(Stat::MeleeDamage);
inline constexpr int STAT_CARRY_WEIGHT = static_cast<int>(Stat::CarryWeight);
inline constexpr int STAT_SEQUENCE = static_cast<int>(Stat::Sequence);
inline constexpr int STAT_HEALING_RATE = static_cast<int>(Stat::HealingRate);
inline constexpr int STAT_CRITICAL_CHANCE = static_cast<int>(Stat::CriticalChance);
inline constexpr int STAT_BETTER_CRITICALS = static_cast<int>(Stat::BetterCriticals);
inline constexpr int STAT_DAMAGE_THRESHOLD = static_cast<int>(Stat::DamageThreshold);
inline constexpr int STAT_DAMAGE_THRESHOLD_LASER = static_cast<int>(Stat::DamageThresholdLaser);
inline constexpr int STAT_DAMAGE_THRESHOLD_FIRE = static_cast<int>(Stat::DamageThresholdFire);
inline constexpr int STAT_DAMAGE_THRESHOLD_PLASMA = static_cast<int>(Stat::DamageThresholdPlasma);
inline constexpr int STAT_DAMAGE_THRESHOLD_ELECTRICAL = static_cast<int>(Stat::DamageThresholdElectrical);
inline constexpr int STAT_DAMAGE_THRESHOLD_EMP = static_cast<int>(Stat::DamageThresholdEmp);
inline constexpr int STAT_DAMAGE_THRESHOLD_EXPLOSION = static_cast<int>(Stat::DamageThresholdExplosion);
inline constexpr int STAT_DAMAGE_RESISTANCE = static_cast<int>(Stat::DamageResistance);
inline constexpr int STAT_DAMAGE_RESISTANCE_LASER = static_cast<int>(Stat::DamageResistanceLaser);
inline constexpr int STAT_DAMAGE_RESISTANCE_FIRE = static_cast<int>(Stat::DamageResistanceFire);
inline constexpr int STAT_DAMAGE_RESISTANCE_PLASMA = static_cast<int>(Stat::DamageResistancePlasma);
inline constexpr int STAT_DAMAGE_RESISTANCE_ELECTRICAL = static_cast<int>(Stat::DamageResistanceElectrical);
inline constexpr int STAT_DAMAGE_RESISTANCE_EMP = static_cast<int>(Stat::DamageResistanceEmp);
inline constexpr int STAT_DAMAGE_RESISTANCE_EXPLOSION = static_cast<int>(Stat::DamageResistanceExplosion);
inline constexpr int STAT_RADIATION_RESISTANCE = static_cast<int>(Stat::RadiationResistance);
inline constexpr int STAT_POISON_RESISTANCE = static_cast<int>(Stat::PoisonResistance);
inline constexpr int STAT_AGE = static_cast<int>(Stat::Age);
inline constexpr int STAT_GENDER = static_cast<int>(Stat::Gender);
inline constexpr int STAT_CURRENT_HIT_POINTS = static_cast<int>(Stat::CurrentHitPoints);
inline constexpr int STAT_CURRENT_POISON_LEVEL = static_cast<int>(Stat::CurrentPoisonLevel);
inline constexpr int STAT_CURRENT_RADIATION_LEVEL = static_cast<int>(Stat::CurrentRadiationLevel);
inline constexpr int STAT_COUNT = static_cast<int>(Stat::Count);
inline constexpr int PRIMARY_STAT_COUNT = 7;
inline constexpr int SPECIAL_STAT_COUNT = 33;
inline constexpr int SAVEABLE_STAT_COUNT = 35;

constexpr int STAT_INVALID = -1;

// Special stats that are only relevant to player character.
enum class PcStat : int {
    UnspentSkillPoints,
    Level,
    Experience,
    Reputation,
    Karma,
    Count,
};

// Legacy constants.
inline constexpr int PC_STAT_UNSPENT_SKILL_POINTS = static_cast<int>(PcStat::UnspentSkillPoints);
inline constexpr int PC_STAT_LEVEL = static_cast<int>(PcStat::Level);
inline constexpr int PC_STAT_EXPERIENCE = static_cast<int>(PcStat::Experience);
inline constexpr int PC_STAT_REPUTATION = static_cast<int>(PcStat::Reputation);
inline constexpr int PC_STAT_KARMA = static_cast<int>(PcStat::Karma);
inline constexpr int PC_STAT_COUNT = static_cast<int>(PcStat::Count);

} // namespace fallout

#pragma once

#include <type_traits>

namespace fallout {

enum class Perk : int {
    Awareness,
    BonusHthAttacks,
    BonusHthDamage,
    BonusMove,
    BonusRangedDamage,
    BonusRateOfFire,
    EarlierSequence,
    FasterHealing,
    MoreCriticals,
    NightVision,
    Presence,
    RadResistance,
    Toughness,
    StrongBack,
    Sharpshooter,
    SilentRunning,
    Survivalist,
    MasterTrader,
    Educated,
    Healer,
    FortuneFinder,
    BetterCriticals,
    Empathy,
    Slayer,
    Sniper,
    SilentDeath,
    ActionBoy,
    MentalBlock,
    Lifegiver,
    Dodger,
    Snakeater,
    MrFixit,
    Medic,
    MasterThief,
    Speaker,
    HeaveHo,
    FriendlyFoe,
    Pickpocket,
    Ghost,
    CultOfPersonality,
    Scrounger,
    Explorer,
    FlowerChild,
    Pathfinder,
    AnimalFriend,
    Scout,
    MysteriousStranger,
    Ranger,
    QuickPockets,
    SmoothTalker,
    SwiftLearner,
    Tag,
    Mutate,
    NukaColaAddiction,
    BuffoutAddiction,
    MentatsAddiction,
    PsychoAddiction,
    RadawayAddiction,
    WeaponLongRange,
    WeaponAccurate,
    WeaponPenetrate,
    WeaponKnockback,
    PoweredArmor,
    CombatArmor,
    Count,
};

// Legacy constants for backward compatibility — used as array indices.
inline constexpr int PERK_AWARENESS = static_cast<int>(Perk::Awareness);
inline constexpr int PERK_BONUS_HTH_ATTACKS = static_cast<int>(Perk::BonusHthAttacks);
inline constexpr int PERK_BONUS_HTH_DAMAGE = static_cast<int>(Perk::BonusHthDamage);
inline constexpr int PERK_BONUS_MOVE = static_cast<int>(Perk::BonusMove);
inline constexpr int PERK_BONUS_RANGED_DAMAGE = static_cast<int>(Perk::BonusRangedDamage);
inline constexpr int PERK_BONUS_RATE_OF_FIRE = static_cast<int>(Perk::BonusRateOfFire);
inline constexpr int PERK_EARLIER_SEQUENCE = static_cast<int>(Perk::EarlierSequence);
inline constexpr int PERK_FASTER_HEALING = static_cast<int>(Perk::FasterHealing);
inline constexpr int PERK_MORE_CRITICALS = static_cast<int>(Perk::MoreCriticals);
inline constexpr int PERK_NIGHT_VISION = static_cast<int>(Perk::NightVision);
inline constexpr int PERK_PRESENCE = static_cast<int>(Perk::Presence);
inline constexpr int PERK_RAD_RESISTANCE = static_cast<int>(Perk::RadResistance);
inline constexpr int PERK_TOUGHNESS = static_cast<int>(Perk::Toughness);
inline constexpr int PERK_STRONG_BACK = static_cast<int>(Perk::StrongBack);
inline constexpr int PERK_SHARPSHOOTER = static_cast<int>(Perk::Sharpshooter);
inline constexpr int PERK_SILENT_RUNNING = static_cast<int>(Perk::SilentRunning);
inline constexpr int PERK_SURVIVALIST = static_cast<int>(Perk::Survivalist);
inline constexpr int PERK_MASTER_TRADER = static_cast<int>(Perk::MasterTrader);
inline constexpr int PERK_EDUCATED = static_cast<int>(Perk::Educated);
inline constexpr int PERK_HEALER = static_cast<int>(Perk::Healer);
inline constexpr int PERK_FORTUNE_FINDER = static_cast<int>(Perk::FortuneFinder);
inline constexpr int PERK_BETTER_CRITICALS = static_cast<int>(Perk::BetterCriticals);
inline constexpr int PERK_EMPATHY = static_cast<int>(Perk::Empathy);
inline constexpr int PERK_SLAYER = static_cast<int>(Perk::Slayer);
inline constexpr int PERK_SNIPER = static_cast<int>(Perk::Sniper);
inline constexpr int PERK_SILENT_DEATH = static_cast<int>(Perk::SilentDeath);
inline constexpr int PERK_ACTION_BOY = static_cast<int>(Perk::ActionBoy);
inline constexpr int PERK_MENTAL_BLOCK = static_cast<int>(Perk::MentalBlock);
inline constexpr int PERK_LIFEGIVER = static_cast<int>(Perk::Lifegiver);
inline constexpr int PERK_DODGER = static_cast<int>(Perk::Dodger);
inline constexpr int PERK_SNAKEATER = static_cast<int>(Perk::Snakeater);
inline constexpr int PERK_MR_FIXIT = static_cast<int>(Perk::MrFixit);
inline constexpr int PERK_MEDIC = static_cast<int>(Perk::Medic);
inline constexpr int PERK_MASTER_THIEF = static_cast<int>(Perk::MasterThief);
inline constexpr int PERK_SPEAKER = static_cast<int>(Perk::Speaker);
inline constexpr int PERK_HEAVE_HO = static_cast<int>(Perk::HeaveHo);
inline constexpr int PERK_FRIENDLY_FOE = static_cast<int>(Perk::FriendlyFoe);
inline constexpr int PERK_PICKPOCKET = static_cast<int>(Perk::Pickpocket);
inline constexpr int PERK_GHOST = static_cast<int>(Perk::Ghost);
inline constexpr int PERK_CULT_OF_PERSONALITY = static_cast<int>(Perk::CultOfPersonality);
inline constexpr int PERK_SCROUNGER = static_cast<int>(Perk::Scrounger);
inline constexpr int PERK_EXPLORER = static_cast<int>(Perk::Explorer);
inline constexpr int PERK_FLOWER_CHILD = static_cast<int>(Perk::FlowerChild);
inline constexpr int PERK_PATHFINDER = static_cast<int>(Perk::Pathfinder);
inline constexpr int PERK_ANIMAL_FRIEND = static_cast<int>(Perk::AnimalFriend);
inline constexpr int PERK_SCOUT = static_cast<int>(Perk::Scout);
inline constexpr int PERK_MYSTERIOUS_STRANGER = static_cast<int>(Perk::MysteriousStranger);
inline constexpr int PERK_RANGER = static_cast<int>(Perk::Ranger);
inline constexpr int PERK_QUICK_POCKETS = static_cast<int>(Perk::QuickPockets);
inline constexpr int PERK_SMOOTH_TALKER = static_cast<int>(Perk::SmoothTalker);
inline constexpr int PERK_SWIFT_LEARNER = static_cast<int>(Perk::SwiftLearner);
inline constexpr int PERK_TAG = static_cast<int>(Perk::Tag);
inline constexpr int PERK_MUTATE = static_cast<int>(Perk::Mutate);
inline constexpr int PERK_NUKA_COLA_ADDICTION = static_cast<int>(Perk::NukaColaAddiction);
inline constexpr int PERK_BUFFOUT_ADDICTION = static_cast<int>(Perk::BuffoutAddiction);
inline constexpr int PERK_MENTATS_ADDICTION = static_cast<int>(Perk::MentatsAddiction);
inline constexpr int PERK_PSYCHO_ADDICTION = static_cast<int>(Perk::PsychoAddiction);
inline constexpr int PERK_RADAWAY_ADDICTION = static_cast<int>(Perk::RadawayAddiction);
inline constexpr int PERK_WEAPON_LONG_RANGE = static_cast<int>(Perk::WeaponLongRange);
inline constexpr int PERK_WEAPON_ACCURATE = static_cast<int>(Perk::WeaponAccurate);
inline constexpr int PERK_WEAPON_PENETRATE = static_cast<int>(Perk::WeaponPenetrate);
inline constexpr int PERK_WEAPON_KNOCKBACK = static_cast<int>(Perk::WeaponKnockback);
inline constexpr int PERK_POWERED_ARMOR = static_cast<int>(Perk::PoweredArmor);
inline constexpr int PERK_COMBAT_ARMOR = static_cast<int>(Perk::CombatArmor);
inline constexpr int PERK_COUNT = static_cast<int>(Perk::Count);

} // namespace fallout

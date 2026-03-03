#pragma once

#include "game/enum_utils.h"
#include "game/object_types.h"

namespace fallout {

constexpr int EXPLOSION_TARGET_COUNT = 6;

constexpr int CRTICIAL_EFFECT_COUNT = 6;

constexpr int WEAPON_CRITICAL_FAILURE_TYPE_COUNT = 7;
constexpr int WEAPON_CRITICAL_FAILURE_EFFECT_COUNT = 5;

enum class CombatState : unsigned int {
    None = 0x00,
    InCombat = 0x01,
    SecondTurn = 0x02,
    PlayerTurn = 0x08,
};
DEFINE_ENUM_FLAG_OPERATORS(CombatState)

// Legacy constants.
inline constexpr unsigned int COMBAT_STATE_0x01 = 0x01;
inline constexpr unsigned int COMBAT_STATE_0x02 = 0x02;
inline constexpr unsigned int COMBAT_STATE_0x08 = 0x08;


enum class HitMode : int {
    LeftWeaponPrimary = 0,
    LeftWeaponSecondary = 1,
    RightWeaponPrimary = 2,
    RightWeaponSecondary = 3,
    Punch = 4,
    Kick = 5,
    LeftWeaponReload = 6,
    RightWeaponReload = 7,
    StrongPunch = 8,       // Punch Level 2
    HammerPunch = 9,       // Punch Level 3
    Haymaker = 10,         // Punch Level 4 aka 'Lightning Punch'
    Jab = 11,              // Punch Level 5 aka 'Chop Punch'
    PalmStrike = 12,       // Punch Level 6 aka 'Dragon Punch'
    PiercingStrike = 13,   // Punch Level 7 aka 'Force Punch'
    StrongKick = 14,       // Kick Level 2
    SnapKick = 15,         // Kick Level 3
    PowerKick = 16,        // Kick Level 4 aka 'Roundhouse Kick'
    HipKick = 17,          // Kick Level 5
    HookKick = 18,         // Kick Level 6 aka 'Jump Kick'
    PiercingKick = 19,     // Kick Level 7 aka 'Death Blossom Kick'
    Count = 20,
};

// Legacy constants for backward compatibility.
inline constexpr int HIT_MODE_LEFT_WEAPON_PRIMARY = 0;
inline constexpr int HIT_MODE_LEFT_WEAPON_SECONDARY = 1;
inline constexpr int HIT_MODE_RIGHT_WEAPON_PRIMARY = 2;
inline constexpr int HIT_MODE_RIGHT_WEAPON_SECONDARY = 3;
inline constexpr int HIT_MODE_PUNCH = 4;
inline constexpr int HIT_MODE_KICK = 5;
inline constexpr int HIT_MODE_LEFT_WEAPON_RELOAD = 6;
inline constexpr int HIT_MODE_RIGHT_WEAPON_RELOAD = 7;
inline constexpr int HIT_MODE_STRONG_PUNCH = 8;
inline constexpr int HIT_MODE_HAMMER_PUNCH = 9;
inline constexpr int HIT_MODE_HAYMAKER = 10;
inline constexpr int HIT_MODE_JAB = 11;
inline constexpr int HIT_MODE_PALM_STRIKE = 12;
inline constexpr int HIT_MODE_PIERCING_STRIKE = 13;
inline constexpr int HIT_MODE_STRONG_KICK = 14;
inline constexpr int HIT_MODE_SNAP_KICK = 15;
inline constexpr int HIT_MODE_POWER_KICK = 16;
inline constexpr int HIT_MODE_HIP_KICK = 17;
inline constexpr int HIT_MODE_HOOK_KICK = 18;
inline constexpr int HIT_MODE_PIERCING_KICK = 19;
inline constexpr int HIT_MODE_COUNT = 20;
inline constexpr int FIRST_ADVANCED_PUNCH_HIT_MODE = HIT_MODE_STRONG_PUNCH;
inline constexpr int LAST_ADVANCED_PUNCH_HIT_MODE = HIT_MODE_PIERCING_STRIKE;
inline constexpr int FIRST_ADVANCED_KICK_HIT_MODE = HIT_MODE_STRONG_KICK;
inline constexpr int LAST_ADVANCED_KICK_HIT_MODE = HIT_MODE_PIERCING_KICK;
inline constexpr int FIRST_ADVANCED_UNARMED_HIT_MODE = FIRST_ADVANCED_PUNCH_HIT_MODE;
inline constexpr int LAST_ADVANCED_UNARMED_HIT_MODE = LAST_ADVANCED_KICK_HIT_MODE;

enum class HitLocation : int {
    Head = 0,
    LeftArm = 1,
    RightArm = 2,
    Torso = 3,
    RightLeg = 4,
    LeftLeg = 5,
    Eyes = 6,
    Groin = 7,
    Uncalled = 8,
    Count = 9,
};
inline constexpr int HIT_LOCATION_SPECIFIC_COUNT = static_cast<int>(HitLocation::Count) - 1;

// Legacy constants.
inline constexpr int HIT_LOCATION_HEAD = 0;
inline constexpr int HIT_LOCATION_LEFT_ARM = 1;
inline constexpr int HIT_LOCATION_RIGHT_ARM = 2;
inline constexpr int HIT_LOCATION_TORSO = 3;
inline constexpr int HIT_LOCATION_RIGHT_LEG = 4;
inline constexpr int HIT_LOCATION_LEFT_LEG = 5;
inline constexpr int HIT_LOCATION_EYES = 6;
inline constexpr int HIT_LOCATION_GROIN = 7;
inline constexpr int HIT_LOCATION_UNCALLED = 8;
inline constexpr int HIT_LOCATION_COUNT = 9;

// Combat sequence parameters — describes modifiers for a combat encounter.
// (Previously named STRUCT_664980 — decompiled address)
class CombatSequenceParams {
public:
    Object* attacker;
    Object* defender;
    int actionPointsBonus;
    int accuracyBonus;
    int damageBonus;
    int minDamage;
    int maxDamage;
    int hasOverrideFlags; // if nonzero, attackerOverrideFlags and defenderOverrideFlags are used
    int attackerOverrideFlags;
    int defenderOverrideFlags;

    int scripts_request_combat();
    static int scripts_request_combat_no_params();

    void combat();
    static void combat_no_params();
};

class Attack {
public:
    Object* attacker;
    int hitMode;
    Object* weapon;
    int attackHitLocation;
    int attackerDamage;
    int attackerFlags;
    int ammoQuantity;
    int criticalMessageId;
    Object* defender;
    int tile;
    int defenderHitLocation;
    int defenderDamage;
    int defenderFlags;
    int defenderKnockback;
    Object* oops;
    int extrasLength;
    Object* extras[EXPLOSION_TARGET_COUNT];
    int extrasHitLocation[EXPLOSION_TARGET_COUNT];
    int extrasDamage[EXPLOSION_TARGET_COUNT];
    int extrasFlags[EXPLOSION_TARGET_COUNT];
    int extrasKnockback[EXPLOSION_TARGET_COUNT];

    // Public methods (formerly free functions in combat.h / combat.cc)
    void init(Object* attacker, Object* defender, int hitMode, int hitLocation);
    void computeExplosionOnExtras(int a2, bool isGrenade, int a4);
    void deathChecks();
    void applyDamage(bool animated);
    void display();
    int computeAttack();

    // Public methods (formerly free functions in actions.h / actions.cc)
    int showDamageTarget();
    int showDamageExtras();
    void showDamage(int a2, int a3);
    int actionAttack();

    // Public method (formerly free function in combatai.h / combatai.cc)
    int aiMsg(Object* critter, int type, int delay);

private:
    // Private methods (formerly static functions in combat.cc)
    bool checkRangedMiss();
    int shootAlongPath(int endTile, int rounds, int anim);
    int computeSpray(int accuracy, int* roundsHitMainTargetPtr, int* roundsSpentPtr, int anim);
    int critSuccess();
    int critFailure();
    void computeDamage(int rounds, int damageMult);
};

// Provides metadata about critical hit effect.
struct CriticalHitDescription {
    int damageMultiplier;

    // Damage flags that will be applied to defender.
    int flags;

    // Stat to check to upgrade this critical hit to massive critical hit or
    // -1 if there is no massive critical hit.
    int massiveCriticalStat;

    // Bonus/penalty to massive critical stat.
    int massiveCriticalStatModifier;

    // Additional damage flags if this critical hit become massive critical.
    int massiveCriticalFlags;

    int messageId;
    int massiveCriticalMessageId;
};

enum class CombatBadShot : int {
    Ok = 0,
    NoAmmo = 1,
    OutOfRange = 2,
    NotEnoughAP = 3,
    AlreadyDead = 4,
    AimBlocked = 5,
    ArmCrippled = 6,
    BothArmsCrippled = 7,
};

// Legacy constants.
inline constexpr int COMBAT_BAD_SHOT_OK = 0;
inline constexpr int COMBAT_BAD_SHOT_NO_AMMO = 1;
inline constexpr int COMBAT_BAD_SHOT_OUT_OF_RANGE = 2;
inline constexpr int COMBAT_BAD_SHOT_NOT_ENOUGH_AP = 3;
inline constexpr int COMBAT_BAD_SHOT_ALREADY_DEAD = 4;
inline constexpr int COMBAT_BAD_SHOT_AIM_BLOCKED = 5;
inline constexpr int COMBAT_BAD_SHOT_ARM_CRIPPLED = 6;
inline constexpr int COMBAT_BAD_SHOT_BOTH_ARMS_CRIPPLED = 7;

} // namespace fallout

#pragma once


#include "game/enum_utils.h"
#include "game/object_types.h"

namespace fallout {

enum class AnimationRequestOptions : unsigned {
    Unreserved = 0x01,
    Reserved = 0x02,
    NoStand = 0x04,
    Flag0x100 = 0x100,
    Insignificant = 0x200,
};

DEFINE_ENUM_FLAG_OPERATORS(AnimationRequestOptions)

inline constexpr int ANIMATION_REQUEST_UNRESERVED = static_cast<int>(AnimationRequestOptions::Unreserved);
inline constexpr int ANIMATION_REQUEST_RESERVED = static_cast<int>(AnimationRequestOptions::Reserved);
inline constexpr int ANIMATION_REQUEST_NO_STAND = static_cast<int>(AnimationRequestOptions::NoStand);
inline constexpr int ANIMATION_REQUEST_0x100 = static_cast<int>(AnimationRequestOptions::Flag0x100);
inline constexpr int ANIMATION_REQUEST_INSIGNIFICANT = static_cast<int>(AnimationRequestOptions::Insignificant);


// Basic animations: 0-19
// Knockdown and death: 20-35
// Change positions: 36-37
// Weapon: 38-47
// Single-frame death animations (the last frame of knockdown and death animations): 48-63
enum class AnimationType : int {
    Stand = 0,
    Walk = 1,
    JumpBegin = 2,
    JumpEnd = 3,
    ClimbLadder = 4,
    Falling = 5,
    UpStairsRight = 6,
    UpStairsLeft = 7,
    DownStairsRight = 8,
    DownStairsLeft = 9,
    MagicHandsGround = 10,
    MagicHandsMiddle = 11,
    MagicHandsUp = 12,
    DodgeAnim = 13,
    HitFromFront = 14,
    HitFromBack = 15,
    ThrowPunch = 16,
    KickLeg = 17,
    ThrowAnim = 18,
    Running = 19,
    FallBack = 20,
    FallFront = 21,
    BadLanding = 22,
    BigHole = 23,
    CharredBody = 24,
    ChunksOfFlesh = 25,
    DancingAutofire = 26,
    Electrify = 27,
    SlicedInHalf = 28,
    BurnedToNothing = 29,
    ElectrifiedToNothing = 30,
    ExplodedToNothing = 31,
    MeltedToNothing = 32,
    FireDance = 33,
    FallBackBlood = 34,
    FallFrontBlood = 35,
    ProneToStanding = 36,
    BackToStanding = 37,
    TakeOut = 38,
    PutAway = 39,
    ParryAnim = 40,
    ThrustAnim = 41,
    SwingAnim = 42,
    Point = 43,
    Unpoint = 44,
    FireSingle = 45,
    FireBurst = 46,
    FireContinuous = 47,
    FallBackSf = 48,
    FallFrontSf = 49,
    BadLandingSf = 50,
    BigHoleSf = 51,
    CharredBodySf = 52,
    ChunksOfFleshSf = 53,
    DancingAutofireSf = 54,
    ElectrifySf = 55,
    SlicedInHalfSf = 56,
    BurnedToNothingSf = 57,
    ElectrifiedToNothingSf = 58,
    ExplodedToNothingSf = 59,
    MeltedToNothingSf = 60,
    FireDanceSf = 61,
    FallBackBloodSf = 62,
    FallFrontBloodSf = 63,
    CalledShotPic = 64,
    Count = 65,
    FirstKnockdownAndDeath = FallBack,
    LastKnockdownAndDeath = FallFrontBlood,
    FirstSfDeath = FallBackSf,
    LastSfDeath = FallFrontBloodSf,
};

inline constexpr int ANIM_STAND = static_cast<int>(AnimationType::Stand);
inline constexpr int ANIM_WALK = static_cast<int>(AnimationType::Walk);
inline constexpr int ANIM_JUMP_BEGIN = static_cast<int>(AnimationType::JumpBegin);
inline constexpr int ANIM_JUMP_END = static_cast<int>(AnimationType::JumpEnd);
inline constexpr int ANIM_CLIMB_LADDER = static_cast<int>(AnimationType::ClimbLadder);
inline constexpr int ANIM_FALLING = static_cast<int>(AnimationType::Falling);
inline constexpr int ANIM_UP_STAIRS_RIGHT = static_cast<int>(AnimationType::UpStairsRight);
inline constexpr int ANIM_UP_STAIRS_LEFT = static_cast<int>(AnimationType::UpStairsLeft);
inline constexpr int ANIM_DOWN_STAIRS_RIGHT = static_cast<int>(AnimationType::DownStairsRight);
inline constexpr int ANIM_DOWN_STAIRS_LEFT = static_cast<int>(AnimationType::DownStairsLeft);
inline constexpr int ANIM_MAGIC_HANDS_GROUND = static_cast<int>(AnimationType::MagicHandsGround);
inline constexpr int ANIM_MAGIC_HANDS_MIDDLE = static_cast<int>(AnimationType::MagicHandsMiddle);
inline constexpr int ANIM_MAGIC_HANDS_UP = static_cast<int>(AnimationType::MagicHandsUp);
inline constexpr int ANIM_DODGE_ANIM = static_cast<int>(AnimationType::DodgeAnim);
inline constexpr int ANIM_HIT_FROM_FRONT = static_cast<int>(AnimationType::HitFromFront);
inline constexpr int ANIM_HIT_FROM_BACK = static_cast<int>(AnimationType::HitFromBack);
inline constexpr int ANIM_THROW_PUNCH = static_cast<int>(AnimationType::ThrowPunch);
inline constexpr int ANIM_KICK_LEG = static_cast<int>(AnimationType::KickLeg);
inline constexpr int ANIM_THROW_ANIM = static_cast<int>(AnimationType::ThrowAnim);
inline constexpr int ANIM_RUNNING = static_cast<int>(AnimationType::Running);
inline constexpr int ANIM_FALL_BACK = static_cast<int>(AnimationType::FallBack);
inline constexpr int ANIM_FALL_FRONT = static_cast<int>(AnimationType::FallFront);
inline constexpr int ANIM_BAD_LANDING = static_cast<int>(AnimationType::BadLanding);
inline constexpr int ANIM_BIG_HOLE = static_cast<int>(AnimationType::BigHole);
inline constexpr int ANIM_CHARRED_BODY = static_cast<int>(AnimationType::CharredBody);
inline constexpr int ANIM_CHUNKS_OF_FLESH = static_cast<int>(AnimationType::ChunksOfFlesh);
inline constexpr int ANIM_DANCING_AUTOFIRE = static_cast<int>(AnimationType::DancingAutofire);
inline constexpr int ANIM_ELECTRIFY = static_cast<int>(AnimationType::Electrify);
inline constexpr int ANIM_SLICED_IN_HALF = static_cast<int>(AnimationType::SlicedInHalf);
inline constexpr int ANIM_BURNED_TO_NOTHING = static_cast<int>(AnimationType::BurnedToNothing);
inline constexpr int ANIM_ELECTRIFIED_TO_NOTHING = static_cast<int>(AnimationType::ElectrifiedToNothing);
inline constexpr int ANIM_EXPLODED_TO_NOTHING = static_cast<int>(AnimationType::ExplodedToNothing);
inline constexpr int ANIM_MELTED_TO_NOTHING = static_cast<int>(AnimationType::MeltedToNothing);
inline constexpr int ANIM_FIRE_DANCE = static_cast<int>(AnimationType::FireDance);
inline constexpr int ANIM_FALL_BACK_BLOOD = static_cast<int>(AnimationType::FallBackBlood);
inline constexpr int ANIM_FALL_FRONT_BLOOD = static_cast<int>(AnimationType::FallFrontBlood);
inline constexpr int ANIM_PRONE_TO_STANDING = static_cast<int>(AnimationType::ProneToStanding);
inline constexpr int ANIM_BACK_TO_STANDING = static_cast<int>(AnimationType::BackToStanding);
inline constexpr int ANIM_TAKE_OUT = static_cast<int>(AnimationType::TakeOut);
inline constexpr int ANIM_PUT_AWAY = static_cast<int>(AnimationType::PutAway);
inline constexpr int ANIM_PARRY_ANIM = static_cast<int>(AnimationType::ParryAnim);
inline constexpr int ANIM_THRUST_ANIM = static_cast<int>(AnimationType::ThrustAnim);
inline constexpr int ANIM_SWING_ANIM = static_cast<int>(AnimationType::SwingAnim);
inline constexpr int ANIM_POINT = static_cast<int>(AnimationType::Point);
inline constexpr int ANIM_UNPOINT = static_cast<int>(AnimationType::Unpoint);
inline constexpr int ANIM_FIRE_SINGLE = static_cast<int>(AnimationType::FireSingle);
inline constexpr int ANIM_FIRE_BURST = static_cast<int>(AnimationType::FireBurst);
inline constexpr int ANIM_FIRE_CONTINUOUS = static_cast<int>(AnimationType::FireContinuous);
inline constexpr int ANIM_FALL_BACK_SF = static_cast<int>(AnimationType::FallBackSf);
inline constexpr int ANIM_FALL_FRONT_SF = static_cast<int>(AnimationType::FallFrontSf);
inline constexpr int ANIM_BAD_LANDING_SF = static_cast<int>(AnimationType::BadLandingSf);
inline constexpr int ANIM_BIG_HOLE_SF = static_cast<int>(AnimationType::BigHoleSf);
inline constexpr int ANIM_CHARRED_BODY_SF = static_cast<int>(AnimationType::CharredBodySf);
inline constexpr int ANIM_CHUNKS_OF_FLESH_SF = static_cast<int>(AnimationType::ChunksOfFleshSf);
inline constexpr int ANIM_DANCING_AUTOFIRE_SF = static_cast<int>(AnimationType::DancingAutofireSf);
inline constexpr int ANIM_ELECTRIFY_SF = static_cast<int>(AnimationType::ElectrifySf);
inline constexpr int ANIM_SLICED_IN_HALF_SF = static_cast<int>(AnimationType::SlicedInHalfSf);
inline constexpr int ANIM_BURNED_TO_NOTHING_SF = static_cast<int>(AnimationType::BurnedToNothingSf);
inline constexpr int ANIM_ELECTRIFIED_TO_NOTHING_SF = static_cast<int>(AnimationType::ElectrifiedToNothingSf);
inline constexpr int ANIM_EXPLODED_TO_NOTHING_SF = static_cast<int>(AnimationType::ExplodedToNothingSf);
inline constexpr int ANIM_MELTED_TO_NOTHING_SF = static_cast<int>(AnimationType::MeltedToNothingSf);
inline constexpr int ANIM_FIRE_DANCE_SF = static_cast<int>(AnimationType::FireDanceSf);
inline constexpr int ANIM_FALL_BACK_BLOOD_SF = static_cast<int>(AnimationType::FallBackBloodSf);
inline constexpr int ANIM_FALL_FRONT_BLOOD_SF = static_cast<int>(AnimationType::FallFrontBloodSf);
inline constexpr int ANIM_CALLED_SHOT_PIC = static_cast<int>(AnimationType::CalledShotPic);
inline constexpr int ANIM_COUNT = static_cast<int>(AnimationType::Count);
inline constexpr int FIRST_KNOCKDOWN_AND_DEATH_ANIM = static_cast<int>(AnimationType::FirstKnockdownAndDeath);
inline constexpr int LAST_KNOCKDOWN_AND_DEATH_ANIM = static_cast<int>(AnimationType::LastKnockdownAndDeath);
inline constexpr int FIRST_SF_DEATH_ANIM = static_cast<int>(AnimationType::FirstSfDeath);
inline constexpr int LAST_SF_DEATH_ANIM = static_cast<int>(AnimationType::LastSfDeath);

#define FID_ANIM_TYPE(value) ((value) & 0xFF0000) >> 16

// Signature of animation callback accepting 2 parameters.
using AnimationCallback = int(void*, void*);

// Signature of animation callback accepting 3 parameters.
using AnimationCallback3 = int(void*, void*, void*);

using PathBuilderCallback = Object*(Object* object, int tile, int elevation);

struct StraightPathNode {
    int tile;
    int elevation;
    int x;
    int y;
};

void anim_init();
void anim_reset();
void anim_exit();
int register_begin(int a1);
int register_priority(int a1);
int register_clear(Object* a1);
int register_end();
int check_registry(Object* obj);
int anim_busy(Object* a1);
int register_object_move_to_object(Object* owner, Object* destination, int actionPoints, int delay);
int register_object_run_to_object(Object* owner, Object* destination, int actionPoints, int delay);
int register_object_move_to_tile(Object* owner, int tile, int elevation, int actionPoints, int delay);
int register_object_run_to_tile(Object* owner, int tile, int elevation, int actionPoints, int delay);
int register_object_move_straight_to_tile(Object* object, int tile, int elevation, int anim, int delay);
int register_object_animate_and_move_straight(Object* owner, int tile, int elev, int anim, int delay);
int register_object_move_on_stairs(Object* owner, Object* stairs, int delay);
int register_object_check_falling(Object* owner, int delay);
int register_object_animate(Object* owner, int anim, int delay);
int register_object_animate_reverse(Object* owner, int anim, int delay);
int register_object_animate_and_hide(Object* owner, int anim, int delay);
int register_object_turn_towards(Object* owner, int tile);
int register_object_inc_rotation(Object* owner);
int register_object_dec_rotation(Object* owner);
int register_object_erase(Object* object);
int register_object_must_erase(Object* object);
int register_object_call(void* a1, void* a2, AnimationCallback* proc, int delay);
int register_object_call3(void* a1, void* a2, void* a3, AnimationCallback3* proc, int delay);
int register_object_must_call(void* a1, void* a2, AnimationCallback* proc, int delay);
int register_object_fset(Object* object, int flag, int delay);
int register_object_funset(Object* object, int flag, int delay);
int register_object_flatten(Object* object, int delay);
int register_object_change_fid(Object* owner, int fid, int delay);
int register_object_take_out(Object* owner, int weaponAnimationCode, int delay);
int register_object_light(Object* owner, int lightDistance, int delay);
int register_object_outline(Object* object, bool outline, int delay);
int register_object_play_sfx(Object* owner, const char* soundEffectName, int delay);
int register_object_animate_forever(Object* owner, int anim, int delay);
int register_ping(int a1, int a2);
int make_path(Object* object, int from, int to, unsigned char* a4, int a5);
int make_path_func(Object* object, int from, int to, unsigned char* rotations, int a5, PathBuilderCallback* callback);
int idist(int a1, int a2, int a3, int a4);
int EST(int tile1, int tile2);
int make_straight_path(Object* a1, int from, int to, StraightPathNode* pathNodes, Object** a5, int a6);
int make_straight_path_func(Object* a1, int from, int to, StraightPathNode* pathNodes, Object** a5, int a6, PathBuilderCallback* callback);
int anim_move_on_stairs(Object* obj, int tile, int elevation, int anim, int animationSequenceIndex);
int check_for_falling(Object* obj, int anim, int a3);
void object_animate();
int check_move(int* a1);
int dude_move(int a1);
int dude_run(int a1);
void dude_fidget();
void dude_stand(Object* obj, int rotation, int fid);
void dude_standup(Object* a1);
int anim_hide(Object* object, int animationSequenceIndex);
int anim_change_fid(Object* obj, int animationSequenceIndex, int fid);
void anim_stop();
unsigned int compute_tpf(Object* object, int fid);

} // namespace fallout

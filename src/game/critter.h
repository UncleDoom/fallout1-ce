#pragma once


#include "game/object_types.h"
#include "game/proto_types.h"
#include "plib/db/db.h"

namespace fallout {

// Maximum length of dude's name length.
inline constexpr int DUDE_NAME_MAX_LENGTH = 32;

// The number of effects caused by radiation.
//
// A radiation effect is an identifier and does not have it's own name. It's
// stat is specified in `rad_stat`, and it's amount is specified
// in `rad_bonus` for every `RadiationLevel`.
inline constexpr int RADIATION_EFFECT_COUNT = 8;

// Radiation levels.
//
// The names of levels are taken from Fallout 3, comments from Fallout 2.
enum class RadiationLevel : int {
    // Very nauseous.
    None = 0,

    // Slightly fatigued.
    Minor = 1,

    // Vomiting does not stop.
    Advanced = 2,

    // Hair is falling out.
    Critical = 3,

    // Skin is falling off.
    Deadly = 4,

    // Intense agony.
    Fatal = 5,

    // The number of radiation levels.
    Count = 6,
};

inline constexpr int RADIATION_LEVEL_NONE = static_cast<int>(RadiationLevel::None);
inline constexpr int RADIATION_LEVEL_MINOR = static_cast<int>(RadiationLevel::Minor);
inline constexpr int RADIATION_LEVEL_ADVANCED = static_cast<int>(RadiationLevel::Advanced);
inline constexpr int RADIATION_LEVEL_CRITICAL = static_cast<int>(RadiationLevel::Critical);
inline constexpr int RADIATION_LEVEL_DEADLY = static_cast<int>(RadiationLevel::Deadly);
inline constexpr int RADIATION_LEVEL_FATAL = static_cast<int>(RadiationLevel::Fatal);
inline constexpr int RADIATION_LEVEL_COUNT = static_cast<int>(RadiationLevel::Count);

enum class PcFlags : int {
    Sneaking = 0,
    LevelUpAvailable = 3,
    Addicted = 4,
};

inline constexpr int PC_FLAG_SNEAKING = static_cast<int>(PcFlags::Sneaking);
inline constexpr int PC_FLAG_LEVEL_UP_AVAILABLE = static_cast<int>(PcFlags::LevelUpAvailable);
inline constexpr int PC_FLAG_ADDICTED = static_cast<int>(PcFlags::Addicted);


extern int rad_stat[RADIATION_EFFECT_COUNT];
extern int rad_bonus[RADIATION_LEVEL_COUNT][RADIATION_EFFECT_COUNT];

int critter_init();
void critter_reset();
void critter_exit();
int critter_load(DB_FILE* stream);
int critter_save(DB_FILE* stream);
char* critter_name(Object* critter);

int critter_pc_set_name(const char* name);
void critter_pc_reset_name();
int critter_get_hits(Object* critter);
int critter_adjust_hits(Object* critter, int amount);
int critter_get_poison(Object* critter);
int critter_adjust_poison(Object* obj, int amount);
int critter_check_poison(Object* obj, void* data);
int critter_get_rads(Object* critter);
int critter_adjust_rads(Object* obj, int amount);
int critter_check_rads(Object* critter);
int critter_process_rads(Object* obj, void* data);
int critter_load_rads(DB_FILE* stream, void** data);
int critter_save_rads(DB_FILE* stream, void* data);
int critter_kill_count_inc(int critter_type);
int critter_kill_count(int critter_type);
int critter_kill_count_load(DB_FILE* stream);
int critter_kill_count_save(DB_FILE* stream);
int critter_kill_count_type(Object* critter);
char* critter_kill_name(int critter_type);
char* critter_kill_info(int critter_type);
int critter_heal_hours(Object* critter, int hours);
void critter_kill(Object* critter, int anim, bool refresh_window);
int critter_kill_exps(Object* critter);
bool critter_is_active(Object* critter);
bool critter_is_dead(Object* critter);
bool critter_is_crippled(Object* critter);
bool critter_is_prone(Object* critter);
int critter_body_type(Object* critter);
int pc_load_data(const char* path);
int pc_save_data(const char* path);
void pc_flag_off(int pc_flag);
void pc_flag_on(int pc_flag);
void pc_flag_toggle(int pc_flag);
bool is_pc_flag(int pc_flag);
int critter_sneak_check(Object* obj, void* data);
int critter_sneak_clear(Object* obj, void* data);
bool is_pc_sneak_working();
int critter_wake_up(Object* obj, void* data);
int critter_wake_clear(Object* obj, void* data);
int critter_set_who_hit_me(Object* critter, Object* who_hit_me);
bool critter_can_obj_dude_rest();
int critter_compute_ap_from_distance(Object* critter, int distance);

} // namespace fallout

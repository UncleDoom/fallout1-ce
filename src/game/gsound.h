#pragma once


#include "game/object_types.h"
#include "int/sound.h"

namespace fallout {

enum class WeaponSoundEffect : int {
    Ready = 0,
    Attack = 1,
    OutOfAmmo = 2,
    AmmoFlying = 3,
    Hit = 4,
    Count = 5,
};

inline constexpr int WEAPON_SOUND_EFFECT_READY = static_cast<int>(WeaponSoundEffect::Ready);
inline constexpr int WEAPON_SOUND_EFFECT_ATTACK = static_cast<int>(WeaponSoundEffect::Attack);
inline constexpr int WEAPON_SOUND_EFFECT_OUT_OF_AMMO = static_cast<int>(WeaponSoundEffect::OutOfAmmo);
inline constexpr int WEAPON_SOUND_EFFECT_AMMO_FLYING = static_cast<int>(WeaponSoundEffect::AmmoFlying);
inline constexpr int WEAPON_SOUND_EFFECT_HIT = static_cast<int>(WeaponSoundEffect::Hit);
inline constexpr int WEAPON_SOUND_EFFECT_COUNT = static_cast<int>(WeaponSoundEffect::Count);

enum class SoundEffectActionType : int {
    Active = 0,
    Passive = 1,
};

inline constexpr int SOUND_EFFECT_ACTION_TYPE_ACTIVE = static_cast<int>(SoundEffectActionType::Active);
inline constexpr int SOUND_EFFECT_ACTION_TYPE_PASSIVE = static_cast<int>(SoundEffectActionType::Passive);

enum class ScenerySoundEffect : int {
    Open = 0,
    Closed = 1,
    Locked = 2,
    Unlocked = 3,
    Used = 4,
    Count = 5,
};

inline constexpr int SCENERY_SOUND_EFFECT_OPEN = static_cast<int>(ScenerySoundEffect::Open);
inline constexpr int SCENERY_SOUND_EFFECT_CLOSED = static_cast<int>(ScenerySoundEffect::Closed);
inline constexpr int SCENERY_SOUND_EFFECT_LOCKED = static_cast<int>(ScenerySoundEffect::Locked);
inline constexpr int SCENERY_SOUND_EFFECT_UNLOCKED = static_cast<int>(ScenerySoundEffect::Unlocked);
inline constexpr int SCENERY_SOUND_EFFECT_USED = static_cast<int>(ScenerySoundEffect::Used);
inline constexpr int SCENERY_SOUND_EFFECT_COUNT = static_cast<int>(ScenerySoundEffect::Count);

enum class CharacterSoundEffect : int {
    Unused = 0,
    Knockdown = 1,
    PassOut = 2,
    Die = 3,
    Contact = 4,
};

inline constexpr int CHARACTER_SOUND_EFFECT_UNUSED = static_cast<int>(CharacterSoundEffect::Unused);
inline constexpr int CHARACTER_SOUND_EFFECT_KNOCKDOWN = static_cast<int>(CharacterSoundEffect::Knockdown);
inline constexpr int CHARACTER_SOUND_EFFECT_PASS_OUT = static_cast<int>(CharacterSoundEffect::PassOut);
inline constexpr int CHARACTER_SOUND_EFFECT_DIE = static_cast<int>(CharacterSoundEffect::Die);
inline constexpr int CHARACTER_SOUND_EFFECT_CONTACT = static_cast<int>(CharacterSoundEffect::Contact);

using SoundEndCallback = void();

int gsound_init();
void gsound_reset();
int gsound_exit();
void gsound_sfx_enable();
void gsound_sfx_disable();
int gsound_sfx_is_enabled();
int gsound_set_master_volume(int value);
int gsound_get_master_volume();
int gsound_set_sfx_volume(int value);
int gsound_get_sfx_volume();
void gsound_background_disable();
void gsound_background_enable();
int gsound_background_is_enabled();
void gsound_background_volume_set(int value);
int gsound_background_volume_get();
int gsound_background_volume_get_set(int a1);
void gsound_background_fade_set(int value);
int gsound_background_fade_get();
int gsound_background_fade_get_set(int value);
void gsound_background_callback_set(SoundEndCallback* callback);
SoundEndCallback* gsound_background_callback_get();
SoundEndCallback* gsound_background_callback_get_set(SoundEndCallback* callback);
int gsound_background_length_get();
int gsound_background_play(const char* fileName, int a2, int a3, int a4);
int gsound_background_play_level_music(const char* a1, int a2);
int gsound_background_play_preloaded();
void gsound_background_stop();
void gsound_background_restart_last(int value);
void gsound_background_pause();
void gsound_background_unpause();
void gsound_speech_disable();
void gsound_speech_enable();
int gsound_speech_is_enabled();
void gsound_speech_volume_set(int value);
int gsound_speech_volume_get();
int gsound_speech_volume_get_set(int volume);
void gsound_speech_callback_set(SoundEndCallback* callback);
SoundEndCallback* gsound_speech_callback_get();
SoundEndCallback* gsound_speech_callback_get_set(SoundEndCallback* callback);
int gsound_speech_length_get();
int gsound_speech_play(const char* fname, int a2, int a3, int a4);
int gsound_speech_play_preloaded();
void gsound_speech_stop();
void gsound_speech_pause();
void gsound_speech_unpause();
int gsound_play_sfx_file_volume(const char* a1, int a2);
Sound* gsound_load_sound(const char* name, Object* a2);
Sound* gsound_load_sound_volume(const char* a1, Object* a2, int a3);
void gsound_delete_sfx(Sound* a1);
int gsnd_anim_sound(Sound* sound, void* a2);
int gsound_play_sound(Sound* a1);
int gsound_compute_relative_volume(Object* obj);
char* gsnd_build_character_sfx_name(Object* a1, int anim, int extra);
char* gsnd_build_ambient_sfx_name(const char* a1);
char* gsnd_build_interface_sfx_name(const char* a1);
char* gsnd_build_weapon_sfx_name(int effectType, Object* weapon, int hitMode, Object* target);
char* gsnd_build_scenery_sfx_name(int actionType, int action, const char* name);
char* gsnd_build_open_sfx_name(Object* a1, int a2);
void gsound_red_butt_press(int btn, int keyCode);
void gsound_red_butt_release(int btn, int keyCode);
void gsound_toggle_butt_press(int btn, int keyCode);
void gsound_toggle_butt_release(int btn, int keyCode);
void gsound_med_butt_press(int btn, int keyCode);
void gsound_med_butt_release(int btn, int keyCode);
void gsound_lrg_butt_press(int btn, int keyCode);
void gsound_lrg_butt_release(int btn, int keyCode);
int gsound_play_sfx_file(const char* name);

} // namespace fallout

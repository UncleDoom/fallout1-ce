#pragma once


#include "game/cache.h"
#include "game/heap.h"
#include "game/object_types.h"
#include "game/proto_types.h"

namespace fallout {

enum class Head : int {
    HEAD_INVALID,
    HEAD_MARCUS,
    HEAD_MYRON,
    HEAD_ELDER,
    HEAD_LYNETTE,
    HEAD_HAROLD,
    HEAD_TANDI,
    HEAD_COM_OFFICER,
    HEAD_SULIK,
    HEAD_PRESIDENT,
    HEAD_HAKUNIN,
    HEAD_BOSS,
    HEAD_DYING_HAKUNIN,
    HEAD_COUNT,
};

enum class HeadAnimation : int {
    VeryGoodReaction = 0,
    FidgetGood = 1,
    GoodToNeutral = 2,
    NeutralToGood = 3,
    FidgetNeutral = 4,
    NeutralToBad = 5,
    BadToNeutral = 6,
    FidgetBad = 7,
    VeryBadReaction = 8,
    GoodPhonemes = 9,
    NeutralPhonemes = 10,
    BadPhonemes = 11,
};

// Legacy constants
inline constexpr int HEAD_ANIMATION_VERY_GOOD_REACTION = static_cast<int>(HeadAnimation::VeryGoodReaction);
inline constexpr int FIDGET_GOOD = static_cast<int>(HeadAnimation::FidgetGood);
inline constexpr int HEAD_ANIMATION_GOOD_TO_NEUTRAL = static_cast<int>(HeadAnimation::GoodToNeutral);
inline constexpr int HEAD_ANIMATION_NEUTRAL_TO_GOOD = static_cast<int>(HeadAnimation::NeutralToGood);
inline constexpr int FIDGET_NEUTRAL = static_cast<int>(HeadAnimation::FidgetNeutral);
inline constexpr int HEAD_ANIMATION_NEUTRAL_TO_BAD = static_cast<int>(HeadAnimation::NeutralToBad);
inline constexpr int HEAD_ANIMATION_BAD_TO_NEUTRAL = static_cast<int>(HeadAnimation::BadToNeutral);
inline constexpr int FIDGET_BAD = static_cast<int>(HeadAnimation::FidgetBad);
inline constexpr int HEAD_ANIMATION_VERY_BAD_REACTION = static_cast<int>(HeadAnimation::VeryBadReaction);
inline constexpr int HEAD_ANIMATION_GOOD_PHONEMES = static_cast<int>(HeadAnimation::GoodPhonemes);
inline constexpr int HEAD_ANIMATION_NEUTRAL_PHONEMES = static_cast<int>(HeadAnimation::NeutralPhonemes);
inline constexpr int HEAD_ANIMATION_BAD_PHONEMES = static_cast<int>(HeadAnimation::BadPhonemes);

enum class Background : int {
    BACKGROUND_0,
    BACKGROUND_1,
    BACKGROUND_2,
    BACKGROUND_HUB,
    BACKGROUND_NECROPOLIS,
    BACKGROUND_BROTHERHOOD,
    BACKGROUND_MILITARY_BASE,
    BACKGROUND_JUNK_TOWN,
    BACKGROUND_CATHEDRAL,
    BACKGROUND_SHADY_SANDS,
    BACKGROUND_VAULT,
    BACKGROUND_MASTER,
    BACKGROUND_FOLLOWER,
    BACKGROUND_RAIDERS,
    BACKGROUND_CAVE,
    BACKGROUND_ENCLAVE,
    BACKGROUND_WASTELAND,
    BACKGROUND_BOSS,
    BACKGROUND_PRESIDENT,
    BACKGROUND_TENT,
    BACKGROUND_ADOBE,
    BACKGROUND_COUNT,
};

struct ArtFrame;

class Art {
public:
    int field_0;
    short framesPerSecond;
    short actionFrame;
    short frameCount;
    short xOffsets[6];
    short yOffsets[6];
    int dataOffsets[6];
    int padding[6];
    int dataSize;

    int fps();
    int actionFrameIndex();
    int maxFrame();
    int frameWidth(int frame, int direction);
    int frameLength(int frame, int direction);
    int frameWidthLength(int frame, int direction, int* out_width, int* out_height);
    int frameHot(int frame, int direction, int* a4, int* a5);
    int frameOffset(int rotation, int* out_offset_x, int* out_offset_y);
    unsigned char* frameData(int frame, int direction);
    ArtFrame* framePtr(int frame, int direction);
};

struct ArtFrame {
    short width;
    short height;
    int size;
    short x;
    short y;
};

struct HeadDescription {
    int goodFidgetCount;
    int neutralFidgetCount;
    int badFidgetCount;
};

enum class WeaponAnimation : int {
    None = 0,
    Knife = 1, // d
    Club = 2, // e
    Hammer = 3, // f
    Spear = 4, // g
    Pistol = 5, // h
    Smg = 6, // i
    Shotgun = 7, // j
    LaserRifle = 8, // k
    Minigun = 9, // l
    Launcher = 10, // m
    Count = 11,
};

// Legacy constants
inline constexpr int WEAPON_ANIMATION_NONE = static_cast<int>(WeaponAnimation::None);
inline constexpr int WEAPON_ANIMATION_KNIFE = static_cast<int>(WeaponAnimation::Knife);
inline constexpr int WEAPON_ANIMATION_CLUB = static_cast<int>(WeaponAnimation::Club);
inline constexpr int WEAPON_ANIMATION_HAMMER = static_cast<int>(WeaponAnimation::Hammer);
inline constexpr int WEAPON_ANIMATION_SPEAR = static_cast<int>(WeaponAnimation::Spear);
inline constexpr int WEAPON_ANIMATION_PISTOL = static_cast<int>(WeaponAnimation::Pistol);
inline constexpr int WEAPON_ANIMATION_SMG = static_cast<int>(WeaponAnimation::Smg);
inline constexpr int WEAPON_ANIMATION_SHOTGUN = static_cast<int>(WeaponAnimation::Shotgun);
inline constexpr int WEAPON_ANIMATION_LASER_RIFLE = static_cast<int>(WeaponAnimation::LaserRifle);
inline constexpr int WEAPON_ANIMATION_MINIGUN = static_cast<int>(WeaponAnimation::Minigun);
inline constexpr int WEAPON_ANIMATION_LAUNCHER = static_cast<int>(WeaponAnimation::Launcher);
inline constexpr int WEAPON_ANIMATION_COUNT = static_cast<int>(WeaponAnimation::Count);

extern int art_vault_guy_num;
extern int art_vault_person_nums[GENDER_COUNT];
extern int art_mapper_blank_tile;

extern Cache art_cache;
extern HeadDescription* head_info;

int art_init();
void art_reset();
void art_exit();
char* art_dir(int objectType);
int art_get_disable(int objectType);
void art_toggle_disable(int objectType);
int art_total(int objectType);
int art_head_fidgets(int headFid);
void scale_art(int fid, unsigned char* dest, int width, int height, int pitch);
Art* art_ptr_lock(int fid, CacheEntry** cache_entry);
unsigned char* art_ptr_lock_data(int fid, int frame, int direction, CacheEntry** out_cache_entry);
unsigned char* art_lock(int fid, CacheEntry** out_cache_entry, int* widthPtr, int* heightPtr);
int art_ptr_unlock(CacheEntry* cache_entry);
int art_discard(int fid);
int art_flush();
int art_get_base_name(int objectType, int a2, char* a3);
int art_get_code(int a1, int a2, char* a3, char* a4);
char* art_get_name(int a1);
int art_read_lst(const char* path, char** artListPtr, int* artListSizePtr);

bool art_exists(int fid);
bool art_fid_valid(int fid);
int art_alias_num(int a1);
int art_alias_fid(int fid);
int art_data_size(int a1, int* out_size);
int art_data_load(int a1, int* a2, unsigned char* data);
void art_data_free(void* ptr);
int art_id(int objectType, int frmId, int animType, int a4, int rotation);
Art* load_frame(const char* path);
int load_frame_into(const char* path, unsigned char* data);
int save_frame(const char* path, unsigned char* data);

} // namespace fallout

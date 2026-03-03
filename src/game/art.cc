#include "game/art.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "game/anim.h"
#include "game/game.h"
#include "game/gconfig.h"
#include "game/object.h"
#include "game/proto.h"
#include "game/raii.h"
#include "platform_compat.h"
#include "plib/gnw/debug.h"
#include "plib/gnw/grbuf.h"
#include "plib/gnw/memory.h"

namespace fallout {

struct ArtListDescription {
    int flags;
    char dir[16];
    char* fileNames; // dynamic array of null terminated strings 13 bytes long each
    int fileNamesLength; // number of entries in list
};

static int art_readSubFrameData(unsigned char* data, DB_FILE* stream, int count);
static int art_readFrameData(Art* art, DB_FILE* stream);
static int art_writeSubFrameData(unsigned char* data, DB_FILE* stream, int count);
static int art_writeFrameData(Art* art, DB_FILE* stream);
static int artGetDataSize(Art* art);
static int paddingForSize(int size);

// 0x4FEAB4
static ArtListDescription art[OBJ_TYPE_COUNT] = {
    { 0, "items", nullptr, 0 },
    { 0, "critters", nullptr, 0 },
    { 0, "scenery", nullptr, 0 },
    { 0, "walls", nullptr, 0 },
    { 0, "tiles", nullptr, 0 },
    { 0, "misc", nullptr, 0 },
    { 0, "intrface", nullptr, 0 },
    { 0, "inven", nullptr, 0 },
    { 0, "heads", nullptr, 0 },
    { 0, "backgrnd", nullptr, 0 },
    { 0, "skilldex", nullptr, 0 },
};

// 0x4FEBE8
static const char* head1 = "gggnnnbbbgnb";

// 0x4FEBEC
static const char* head2 = "vfngfbnfvppp";

// Current native look base fid.
//
// 0x4FEBF0
int art_vault_guy_num = 0;

// 4FEBF4
int art_vault_person_nums[GENDER_COUNT];

// Index of "grid001.frm" in tiles.lst.
//
// 0x4FEBFC
int art_mapper_blank_tile = 1;

// 0x56B700
Cache art_cache;

// 0x56B754
static char art_name[COMPAT_MAX_PATH];

// 0x56B858
HeadDescription* head_info;

// 0x56B85C
static int* anon_alias;

// 0x418170
int art_init()
{
    char path[COMPAT_MAX_PATH];
    char string[200];
    DB_DATABASE* old_db_handle;
    bool critter_db_selected = false;

    int cacheSize;
    if (!game_config.getValue(GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_ART_CACHE_SIZE_KEY, &cacheSize)) {
        cacheSize = 8;
    }

    if (!cache_init(&art_cache, art_data_size, art_data_load, art_data_free, cacheSize << 20)) {
        debug_printf("cache_init failed in art_init\n");
        return -1;
    }

    for (int objectType = 0; objectType < OBJ_TYPE_COUNT; objectType++) {
        art[objectType].flags = 0;
        snprintf(path, sizeof(path), "%s%s%s\\%s.lst",
            cd_path_base,
            "art\\",
            art[objectType].dir,
            art[objectType].dir);

        if (objectType == OBJ_TYPE_CRITTER) {
            old_db_handle = db_current();
            critter_db_selected = true;
            critter_db_handle->select();
        }

        if (art_read_lst(path, &(art[objectType].fileNames), &(art[objectType].fileNamesLength)) != 0) {
            debug_printf("art_read_lst failed in art_init\n");
            if (critter_db_selected) {
                old_db_handle->select();
            }
            cache_exit(&art_cache);
            return -1;
        }

        if (objectType == OBJ_TYPE_CRITTER) {
            critter_db_selected = false;
            old_db_handle->select();
        }
    }

    MemBuffer<int> anonAliasGuard(static_cast<int*>(mem_malloc(sizeof(int) * art[OBJ_TYPE_CRITTER].fileNamesLength)));
    if (!anonAliasGuard) {
        art[OBJ_TYPE_CRITTER].fileNamesLength = 0;
        debug_printf("Out of memory for anon_alias in art_init\n");
        cache_exit(&art_cache);
        return -1;
    }

    snprintf(path, sizeof(path), "%s%s%s\\%s.lst",
        cd_path_base,
        "art\\",
        art[OBJ_TYPE_CRITTER].dir,
        art[OBJ_TYPE_CRITTER].dir);

    old_db_handle = db_current();
    critter_db_handle->select();

    {
        DbFileGuard stream(db_fopen(path, "rt"));
        if (!stream) {
            debug_printf("Unable to open %s in art_init\n", path);
            old_db_handle->select();
            cache_exit(&art_cache);
            return -1;
        }

        char* critterFileNames = art[OBJ_TYPE_CRITTER].fileNames;
        for (int critterIndex = 0; critterIndex < art[OBJ_TYPE_CRITTER].fileNamesLength; critterIndex++) {
            if (compat_stricmp(critterFileNames, "hmjmps") == 0) {
                art_vault_person_nums[GENDER_MALE] = critterIndex;
            } else if (compat_stricmp(critterFileNames, "hfjmps") == 0) {
                art_vault_person_nums[GENDER_FEMALE] = critterIndex;
            }

            critterFileNames += 13;
        }

        for (int critterIndex = 0; critterIndex < art[OBJ_TYPE_CRITTER].fileNamesLength; critterIndex++) {
            if (!stream.get()->fgets(string, sizeof(string))) {
                break;
            }

            char* sep1 = strchr(string, ',');
            if (sep1 != nullptr) {
                anonAliasGuard[critterIndex] = atoi(sep1 + 1);
            } else {
                anonAliasGuard[critterIndex] = art_vault_guy_num;
            }
        }
    }

    old_db_handle->select();

    char* tileFileNames = art[OBJ_TYPE_TILE].fileNames;
    for (int tileIndex = 0; tileIndex < art[OBJ_TYPE_TILE].fileNamesLength; tileIndex++) {
        if (compat_stricmp(tileFileNames, "grid001.frm") == 0) {
            art_mapper_blank_tile = tileIndex;
        }
        tileFileNames += 13;
    }

    MemBuffer<HeadDescription> headInfoGuard(static_cast<HeadDescription*>(mem_malloc(sizeof(HeadDescription) * art[OBJ_TYPE_HEAD].fileNamesLength)));
    if (!headInfoGuard) {
        art[OBJ_TYPE_HEAD].fileNamesLength = 0;
        debug_printf("Out of memory for head_info in art_init\n");
        cache_exit(&art_cache);
        return -1;
    }

    snprintf(path, sizeof(path), "%s%s%s\\%s.lst",
        cd_path_base,
        "art\\",
        art[OBJ_TYPE_HEAD].dir,
        art[OBJ_TYPE_HEAD].dir);

    {
        DbFileGuard stream(db_fopen(path, "rt"));
        if (!stream) {
            debug_printf("Unable to open %s in art_init\n", path);
            cache_exit(&art_cache);
            return -1;
        }

        for (int headIndex = 0; headIndex < art[OBJ_TYPE_HEAD].fileNamesLength; headIndex++) {
            char* sep1;
            char* sep2;
            char* sep3;
            char* sep4;

            if (!stream.get()->fgets(string, sizeof(string))) {
                break;
            }

            sep1 = strchr(string, ',');
            if (sep1 != nullptr) {
                *sep1 = '\0';
            } else {
                sep1 = string;
            }

            sep2 = strchr(sep1, ',');
            if (sep2 != nullptr) {
                *sep2 = '\0';
            } else {
                sep2 = sep1;
            }

            headInfoGuard[headIndex].goodFidgetCount = atoi(sep1 + 1);

            sep3 = strchr(sep2, ',');
            if (sep3 != nullptr) {
                *sep3 = '\0';
            } else {
                sep3 = sep2;
            }

            headInfoGuard[headIndex].neutralFidgetCount = atoi(sep2 + 1);

            sep4 = strpbrk(sep3 + 1, " ,;\t\n");
            if (sep4 != nullptr) {
                *sep4 = '\0';
            }

            headInfoGuard[headIndex].badFidgetCount = atoi(sep3 + 1);
        }
    }

    // Success — transfer ownership to module globals.
    anon_alias = anonAliasGuard.release();
    head_info = headInfoGuard.release();

    return 0;
}

// 0x418684
void art_reset()
{
}

// 0x418688
void art_exit()
{
    cache_exit(&art_cache);

    mem_free(anon_alias);

    for (int index = 0; index < OBJ_TYPE_COUNT; index++) {
        mem_free(art[index].fileNames);
        art[index].fileNames = nullptr;
    }

    mem_free(head_info);
}

// 0x4186C4
char* art_dir(int objectType)
{
    return objectType >= OBJ_TYPE_ITEM && objectType < OBJ_TYPE_COUNT ? art[objectType].dir : nullptr;
}

// 0x4186E8
int art_get_disable(int objectType)
{
    return objectType >= OBJ_TYPE_ITEM && objectType < OBJ_TYPE_COUNT ? art[objectType].flags & 1 : 0;
}

// 0x41870C
void art_toggle_disable(int objectType)
{
    if (objectType >= 0 && objectType < OBJ_TYPE_COUNT) {
        art[objectType].flags ^= 1;
    }
}

// 0x418728
int art_total(int objectType)
{
    return objectType >= 0 && objectType < OBJ_TYPE_COUNT ? art[objectType].fileNamesLength : 0;
}

// 0x418748
int art_head_fidgets(int headFid)
{
    if (FID_TYPE(headFid) != OBJ_TYPE_HEAD) {
        return 0;
    }

    int head = headFid & 0xFFF;

    if (head > art[OBJ_TYPE_HEAD].fileNamesLength) {
        return 0;
    }

    HeadDescription* headDescription = &(head_info[head]);

    int fidget = (headFid & 0xFF0000) >> 16;
    switch (fidget) {
    case FIDGET_GOOD:
        return headDescription->goodFidgetCount;
    case FIDGET_NEUTRAL:
        return headDescription->neutralFidgetCount;
    case FIDGET_BAD:
        return headDescription->badFidgetCount;
    }
    return 0;
}

// 0x4187C8
void scale_art(int fid, unsigned char* dest, int width, int height, int pitch)
{
    // NOTE: Original code is different. For unknown reason it directly calls
    // many art functions, for example instead of [art_ptr_lock] it calls lower level
    // [cache_lock], instead of [art_frame_width] is calls [frame_ptr], then get
    // width from frame's struct field. I don't know if this was intentional or
    // not. I've replaced these calls with higher level functions where
    // appropriate.

    CacheEntry* handle;
    Art* frm = art_ptr_lock(fid, &handle);
    if (frm == nullptr) {
        return;
    }

    unsigned char* frameData = frm->frameData(0, 0);
    int frameWidth = frm->frameWidth(0, 0);
    int frameHeight = frm->frameLength(0, 0);

    int remainingWidth = width - frameWidth;
    int remainingHeight = height - frameHeight;
    if (remainingWidth < 0 || remainingHeight < 0) {
        if (height * frameWidth >= width * frameHeight) {
            trans_cscale(frameData,
                frameWidth,
                frameHeight,
                frameWidth,
                dest + pitch * ((height - width * frameHeight / frameWidth) / 2),
                width,
                width * frameHeight / frameWidth,
                pitch);
        } else {
            trans_cscale(frameData,
                frameWidth,
                frameHeight,
                frameWidth,
                dest + (width - height * frameWidth / frameHeight) / 2,
                height * frameWidth / frameHeight,
                height,
                pitch);
        }
    } else {
        trans_buf_to_buf(frameData,
            frameWidth,
            frameHeight,
            frameWidth,
            dest + pitch * (remainingHeight / 2) + remainingWidth / 2,
            pitch);
    }

    art_ptr_unlock(handle);
}

// 0x41892C
Art* art_ptr_lock(int fid, CacheEntry** handlePtr)
{
    if (handlePtr == nullptr) {
        return nullptr;
    }

    Art* art = nullptr;
    cache_lock(&art_cache, fid, reinterpret_cast<void**>(&art), handlePtr);
    return art;
}

// 0x418954
unsigned char* art_ptr_lock_data(int fid, int frame, int direction, CacheEntry** handlePtr)
{
    Art* art;
    ArtFrame* frm;

    art = nullptr;
    if (handlePtr) {
        cache_lock(&art_cache, fid, reinterpret_cast<void**>(&art), handlePtr);
    }

    if (art != nullptr) {
        frm = art->framePtr(frame, direction);
        if (frm != nullptr) {

            return reinterpret_cast<unsigned char*>(frm) + sizeof(*frm);
        }
    }

    return nullptr;
}

// 0x418998
unsigned char* art_lock(int fid, CacheEntry** handlePtr, int* widthPtr, int* heightPtr)
{
    *handlePtr = nullptr;

    Art* art = nullptr;
    cache_lock(&art_cache, fid, reinterpret_cast<void**>(&art), handlePtr);

    if (art == nullptr) {
        return nullptr;
    }

    // NOTE: Uninline.
    *widthPtr = art->frameWidth(0, 0);
    if (*widthPtr == -1) {
        return nullptr;
    }

    // NOTE: Uninline.
    *heightPtr = art->frameLength(0, 0);
    if (*heightPtr == -1) {
        return nullptr;
    }

    // NOTE: Uninline.
    return art->frameData(0, 0);
}

// 0x418A2C
int art_ptr_unlock(CacheEntry* handle)
{
    return cache_unlock(&art_cache, handle);
}

// 0x418A48
int art_flush()
{
    return cache_flush(&art_cache);
}

// 0x418A60
int art_discard(int fid)
{
    if (cache_discard(&art_cache, fid) == 0) {
        return -1;
    }

    return 0;
}

// 0x418A7C
int art_get_base_name(int objectType, int id, char* dest)
{
    ArtListDescription* ptr;

    if (objectType < OBJ_TYPE_ITEM && objectType >= OBJ_TYPE_COUNT) {
        return -1;
    }

    ptr = &(art[objectType]);

    if (id >= ptr->fileNamesLength) {
        return -1;
    }

    strcpy(dest, ptr->fileNames + id * 13);

    return 0;
}

// 0x418AE8
int art_get_code(int animation, int weaponType, char* a3, char* a4)
{
    if (weaponType < 0 || weaponType >= WEAPON_ANIMATION_COUNT) {
        return -1;
    }

    if (animation >= ANIM_TAKE_OUT && animation <= ANIM_FIRE_CONTINUOUS) {
        *a4 = 'c' + (animation - ANIM_TAKE_OUT);
        if (weaponType == WEAPON_ANIMATION_NONE) {
            return -1;
        }

        *a3 = 'd' + (weaponType - 1);
        return 0;
    } else if (animation == ANIM_PRONE_TO_STANDING) {
        *a4 = 'h';
        *a3 = 'c';
        return 0;
    } else if (animation == ANIM_BACK_TO_STANDING) {
        *a4 = 'j';
        *a3 = 'c';
        return 0;
    } else if (animation == ANIM_CALLED_SHOT_PIC) {
        *a4 = 'a';
        *a3 = 'n';
        return 0;
    } else if (animation >= FIRST_SF_DEATH_ANIM) {
        *a4 = 'a' + (animation - FIRST_SF_DEATH_ANIM);
        *a3 = 'r';
        return 0;
    } else if (animation >= FIRST_KNOCKDOWN_AND_DEATH_ANIM) {
        *a4 = 'a' + (animation - FIRST_KNOCKDOWN_AND_DEATH_ANIM);
        *a3 = 'b';
        return 0;
    } else if (animation == ANIM_THROW_ANIM) {
        if (weaponType == WEAPON_ANIMATION_KNIFE) {
            // knife
            *a3 = 'd';
            *a4 = 'm';
        } else if (weaponType == WEAPON_ANIMATION_SPEAR) {
            // spear
            *a3 = 'g';
            *a4 = 'm';
        } else {
            // other -> probably rock or grenade
            *a3 = 'a';
            *a4 = 's';
        }
        return 0;
    } else if (animation == ANIM_DODGE_ANIM) {
        if (weaponType <= 0) {
            *a3 = 'a';
            *a4 = 'n';
        } else {
            *a3 = 'd' + (weaponType - 1);
            *a4 = 'e';
        }
        return 0;
    }

    *a4 = 'a' + animation;
    if (animation <= ANIM_WALK && weaponType > 0) {
        *a3 = 'd' + (weaponType - 1);
        return 0;
    }
    *a3 = 'a';

    return 0;
}

// 0x418BFC
char* art_get_name(int fid)
{
    int alias_fid;
    int index;
    int anim;
    int weapon_anim;
    int type;
    int v1;
    char code1;
    char code2;

    v1 = (fid & 0x70000000) >> 28;

    alias_fid = art_alias_fid(fid);
    if (alias_fid != -1) {
        fid = alias_fid;
    }

    *art_name = '\0';

    index = fid & 0xFFF;
    anim = FID_ANIM_TYPE(fid);
    weapon_anim = (fid & 0xF000) >> 12;
    type = FID_TYPE(fid);

    if (index >= art[type].fileNamesLength) {
        return nullptr;
    }

    if (type < OBJ_TYPE_ITEM || type >= OBJ_TYPE_COUNT) {
        return nullptr;
    }

    switch (type) {
    case OBJ_TYPE_CRITTER:
        if (art_get_code(anim, weapon_anim, &code1, &code2) == -1) {
            return nullptr;
        }

        if (v1) {
            snprintf(art_name, sizeof(art_name),
                "%s%s%s\\%s%c%c.fr%c",
                cd_path_base,
                "art\\",
                art[OBJ_TYPE_CRITTER].dir,
                art[OBJ_TYPE_CRITTER].fileNames + index * 13,
                code1,
                code2,
                v1 + 47);
        } else {
            snprintf(art_name, sizeof(art_name),
                "%s%s%s\\%s%c%c.frm",
                cd_path_base,
                "art\\",
                art[OBJ_TYPE_CRITTER].dir,
                art[OBJ_TYPE_CRITTER].fileNames + index * 13,
                code1,
                code2);
        }
        break;
    case OBJ_TYPE_HEAD:
        if (head2[anim] == 'f') {
            snprintf(art_name, sizeof(art_name),
                "%s%s%s\\%s%c%c%d.frm",
                cd_path_base,
                "art\\",
                art[OBJ_TYPE_HEAD].dir,
                art[OBJ_TYPE_HEAD].fileNames + index * 13,
                head1[anim],
                head2[anim],
                weapon_anim);
        } else {
            snprintf(art_name, sizeof(art_name),
                "%s%s%s\\%s%c%c.frm",
                cd_path_base,
                "art\\",
                art[OBJ_TYPE_HEAD].dir,
                art[OBJ_TYPE_HEAD].fileNames + index * 13,
                head1[anim],
                head2[anim]);
        }
        break;
    default:
        snprintf(art_name, sizeof(art_name),
            "%s%s%s\\%s",
            cd_path_base,
            "art\\",
            art[type].dir,
            art[type].fileNames + index * 13);
        break;
    }

    return art_name;
}

// 0x418E38
int art_read_lst(const char* path, char** artListPtr, int* artListSizePtr)
{
    DbFileGuard stream(db_fopen(path, "rt"));
    if (!stream) {
        return -1;
    }

    int count = 0;
    char string[200];
    while (stream.get()->fgets(string, sizeof(string))) {
        count++;
    }

    stream.get()->fseek(0, SEEK_SET);

    *artListSizePtr = count;

    char* artList = static_cast<char*>(mem_malloc(13 * count));
    *artListPtr = artList;
    if (artList == nullptr) {
        return -1;
    }

    while (stream.get()->fgets(string, sizeof(string))) {
        char* brk = strpbrk(string, " ,;\r\t\n");
        if (brk != nullptr) {
            *brk = '\0';
        }

        strncpy(artList, string, 12);
        artList[12] = '\0';

        artList += 13;
    }

    return 0;
}

// 0x418F34
int Art::fps()
{
    return framesPerSecond == 0 ? 10 : framesPerSecond;
}

// 0x418F4C
int Art::actionFrameIndex()
{
    return actionFrame;
}

// 0x418F60
int Art::maxFrame()
{
    return frameCount;
}

// 0x418F74
int Art::frameWidth(int frame, int direction)
{
    ArtFrame* frm = framePtr(frame, direction);
    if (frm == nullptr) {
        return -1;
    }

    return frm->width;
}

// 0x418F8C
int Art::frameLength(int frame, int direction)
{
    ArtFrame* frm = framePtr(frame, direction);
    if (frm == nullptr) {
        return -1;
    }

    return frm->height;
}

// 0x4197D4
int Art::frameWidthLength(int frame, int direction, int* widthPtr, int* heightPtr)
{
    ArtFrame* frm = framePtr(frame, direction);
    if (frm == nullptr) {
        if (widthPtr != nullptr) {
            *widthPtr = 0;
        }

        if (heightPtr != nullptr) {
            *heightPtr = 0;
        }

        return -1;
    }

    if (widthPtr != nullptr) {
        *widthPtr = frm->width;
    }

    if (heightPtr != nullptr) {
        *heightPtr = frm->height;
    }

    return 0;
}

// 0x418FA8
int Art::frameHot(int frame, int direction, int* xPtr, int* yPtr)
{
    ArtFrame* frm = framePtr(frame, direction);
    if (frm == nullptr) {
        return -1;
    }

    *xPtr = frm->x;
    *yPtr = frm->y;

    return 0;
}

// 0x418FD4
int Art::frameOffset(int rotation, int* xPtr, int* yPtr)
{
    *xPtr = xOffsets[rotation];
    *yPtr = yOffsets[rotation];

    return 0;
}

// 0x418FF8
unsigned char* Art::frameData(int frame, int direction)
{
    ArtFrame* frm = framePtr(frame, direction);
    if (frm == nullptr) {
        return nullptr;
    }

    return reinterpret_cast<unsigned char*>(frm) + sizeof(*frm);
}

// 0x419008
ArtFrame* Art::framePtr(int frame, int rotation)
{
    if (rotation < 0 || rotation >= 6) {
        return nullptr;
    }

    if (frame < 0 || frame >= frameCount) {
        return nullptr;
    }

    ArtFrame* frm = (ArtFrame*)(reinterpret_cast<unsigned char*>(this) + sizeof(*this) + dataOffsets[rotation] + padding[rotation]);
    for (int index = 0; index < frame; index++) {
        frm = (ArtFrame*)(reinterpret_cast<unsigned char*>(frm) + sizeof(*frm) + frm->size + paddingForSize(frm->size));
    }
    return frm;
}

// 0x419050
bool art_exists(int fid)
{
    bool result = false;
    DB_DATABASE* oldDb = INVALID_DATABASE_HANDLE;

    if (FID_TYPE(fid) == OBJ_TYPE_CRITTER) {
        oldDb = db_current();
        critter_db_handle->select();
    }

    char* filePath = art_get_name(fid);
    if (filePath != nullptr) {
        dir_entry de;
        if (db_dir_entry(filePath, &de) != -1) {
            result = true;
        }
    }

    if (oldDb != INVALID_DATABASE_HANDLE) {
        oldDb->select();
    }

    return result;
}

// NOTE: Exactly the same implementation as `art_exists`.
//
// 0x4190B8
bool art_fid_valid(int fid)
{
    bool result = false;
    DB_DATABASE* oldDb = INVALID_DATABASE_HANDLE;

    if (FID_TYPE(fid) == OBJ_TYPE_CRITTER) {
        oldDb = db_current();
        critter_db_handle->select();
    }

    char* filePath = art_get_name(fid);
    if (filePath != nullptr) {
        dir_entry de;
        if (db_dir_entry(filePath, &de) != -1) {
            result = true;
        }
    }

    if (oldDb != INVALID_DATABASE_HANDLE) {
        oldDb->select();
    }

    return result;
}

// 0x419120
int art_alias_num(int index)
{
    return anon_alias[index];
}

// 0x419134
int art_alias_fid(int fid)
{
    int type = FID_TYPE(fid);
    int anim = FID_ANIM_TYPE(fid);
    if (type == OBJ_TYPE_CRITTER) {
        if (anim == ANIM_ELECTRIFY
            || anim == ANIM_BURNED_TO_NOTHING
            || anim == ANIM_ELECTRIFIED_TO_NOTHING
            || anim == ANIM_ELECTRIFY_SF
            || anim == ANIM_BURNED_TO_NOTHING_SF
            || anim == ANIM_ELECTRIFIED_TO_NOTHING_SF
            || anim == ANIM_FIRE_DANCE
            || anim == ANIM_CALLED_SHOT_PIC) {
            // NOTE: Original code is slightly different. It uses many mutually
            // mirrored bitwise operators. Probably result of some macros for
            // getting/setting individual bits on fid.
            return (fid & 0x70000000) | ((anim << 16) & 0xFF0000) | 0x1000000 | (fid & 0xF000) | (anon_alias[fid & 0xFFF] & 0xFFF);
        }
    }

    return -1;
}

// 0x4191D8
int art_data_size(int fid, int* sizePtr)
{
    DB_DATABASE* oldDb = INVALID_DATABASE_HANDLE;
    int result = -1;

    if (FID_TYPE(fid) == OBJ_TYPE_CRITTER) {
        oldDb = db_current();
        critter_db_handle->select();
    }

    char* artFilePath = art_get_name(fid);
    if (artFilePath != nullptr) {
        DbFileGuard stream(db_fopen(artFilePath, "rb"));
        if (stream) {
            Art art;
            if (art_readFrameData(&art, stream.get()) == 0) {
                *sizePtr = artGetDataSize(&art);
                result = 0;
            }
        }
    }

    if (oldDb != INVALID_DATABASE_HANDLE) {
        oldDb->select();
    }

    return result;
}

// 0x41924C
int art_data_load(int fid, int* sizePtr, unsigned char* data)
{
    DB_DATABASE* oldDb = INVALID_DATABASE_HANDLE;
    int result = -1;

    if (FID_TYPE(fid) == OBJ_TYPE_CRITTER) {
        oldDb = db_current();
        critter_db_handle->select();
    }

    char* artFileName = art_get_name(fid);
    if (artFileName != nullptr) {
        if (load_frame_into(artFileName, data) == 0) {
            *sizePtr = artGetDataSize(reinterpret_cast<Art*>(data));
            result = 0;
        }
    }

    if (oldDb != INVALID_DATABASE_HANDLE) {
        oldDb->select();
    }

    return result;
}

// 0x4192C0
void art_data_free(void* ptr)
{
    mem_free(ptr);
}

// 0x4192C8
int art_id(int objectType, int frmId, int animType, int a3, int rotation)
{
    int v7, v8, v9, v10;

    v10 = rotation;

    if (objectType != OBJ_TYPE_CRITTER) {
        goto zero;
    }

    if (animType == ANIM_FIRE_DANCE || animType < ANIM_FALL_BACK || animType > ANIM_FALL_FRONT_BLOOD) {
        goto zero;
    }

    v7 = ((a3 << 12) & 0xF000) | ((animType << 16) & 0xFF0000) | 0x1000000;
    v8 = ((rotation << 28) & 0x70000000) | v7;
    v9 = frmId & 0xFFF;

    if (art_exists(v9 | v8) != 0) {
        goto out;
    }

    if (objectType == rotation) {
        goto zero;
    }

    v10 = objectType;
    if (art_exists(v9 | v7 | 0x10000000) != 0) {
        goto out;
    }

zero:

    v10 = 0;

out:

    return ((v10 << 28) & 0x70000000) | (objectType << 24) | ((animType << 16) & 0xFF0000) | ((a3 << 12) & 0xF000) | (frmId & 0xFFF);
}

// 0x4193A0
static int art_readSubFrameData(unsigned char* data, DB_FILE* stream, int count, int* paddingPtr)
{
    unsigned char* ptr = data;
    int padding = 0;
    for (int index = 0; index < count; index++) {
        ArtFrame* frame = reinterpret_cast<ArtFrame*>(ptr);

        if (stream->freadInt16(&(frame->width)) == -1) return -1;
        if (stream->freadInt16(&(frame->height)) == -1) return -1;
        if (stream->freadInt32(&(frame->size)) == -1) return -1;
        if (stream->freadInt16(&(frame->x)) == -1) return -1;
        if (stream->freadInt16(&(frame->y)) == -1) return -1;
        if (stream->fread(ptr + sizeof(ArtFrame), frame->size, 1) != 1) return -1;

        ptr += sizeof(ArtFrame) + frame->size;
        ptr += paddingForSize(frame->size);
        padding += paddingForSize(frame->size);
    }

    *paddingPtr = padding;

    return 0;
}

// 0x41945C
static int art_readFrameData(Art* art, DB_FILE* stream)
{
    if (stream->freadInt32(&(art->field_0)) == -1) return -1;
    if (stream->freadInt16(&(art->framesPerSecond)) == -1) return -1;
    if (stream->freadInt16(&(art->actionFrame)) == -1) return -1;
    if (stream->freadInt16(&(art->frameCount)) == -1) return -1;
    if (stream->freadInt16List(art->xOffsets, ROTATION_COUNT) == -1) return -1;
    if (stream->freadInt16List(art->yOffsets, ROTATION_COUNT) == -1) return -1;
    if (stream->freadInt32List(art->dataOffsets, ROTATION_COUNT) == -1) return -1;
    if (stream->freadInt32(&(art->dataSize)) == -1) return -1;

    return 0;
}

// 0x419500
Art* load_frame(const char* path)
{
    Art header;

    {
        DbFileGuard stream(db_fopen(path, "rb"));
        if (!stream) {
            return nullptr;
        }

        if (art_readFrameData(&header, stream.get()) != 0) {
            return nullptr;
        }
    }

    MemBuffer<unsigned char> data(static_cast<unsigned char*>(mem_malloc(artGetDataSize(&header))));
    if (!data) {
        return nullptr;
    }

    if (load_frame_into(path, data.get()) != 0) {
        return nullptr;
    }

    return reinterpret_cast<Art*>(data.release());
}

// 0x419600
int load_frame_into(const char* path, unsigned char* data)
{
    DbFileGuard stream(db_fopen(path, "rb"));
    if (!stream) {
        return -2;
    }

    Art* art = reinterpret_cast<Art*>(data);
    if (art_readFrameData(art, stream.get()) != 0) {
        return -3;
    }

    int currentPadding = paddingForSize(sizeof(Art));
    int previousPadding = 0;

    for (int index = 0; index < ROTATION_COUNT; index++) {
        art->padding[index] = currentPadding;

        if (index == 0 || art->dataOffsets[index - 1] != art->dataOffsets[index]) {
            art->padding[index] += previousPadding;
            currentPadding += previousPadding;
            if (art_readSubFrameData(data + sizeof(Art) + art->dataOffsets[index] + art->padding[index], stream.get(), art->frameCount, &previousPadding) != 0) {
                return -5;
            }
        }
    }

    return 0;
}

// 0x4196B0
static int art_writeSubFrameData(unsigned char* data, DB_FILE* stream, int count)
{
    unsigned char* ptr = data;
    for (int index = 0; index < count; index++) {
        ArtFrame* frame = reinterpret_cast<ArtFrame*>(ptr);

        if (stream->fwriteInt16(frame->width) == -1) return -1;
        if (stream->fwriteInt16(frame->height) == -1) return -1;
        if (stream->fwriteInt32(frame->size) == -1) return -1;
        if (stream->fwriteInt16(frame->x) == -1) return -1;
        if (stream->fwriteInt16(frame->y) == -1) return -1;
        if (stream->fwrite(ptr + sizeof(ArtFrame), frame->size, 1) != 1) return -1;

        ptr += sizeof(ArtFrame) + frame->size;
        ptr += paddingForSize(frame->size);
    }

    return 0;
}

// 0x419778
static int art_writeFrameData(Art* art, DB_FILE* stream)
{
    if (stream->fwriteInt32(art->field_0) == -1) return -1;
    if (stream->fwriteInt16(art->framesPerSecond) == -1) return -1;
    if (stream->fwriteInt16(art->actionFrame) == -1) return -1;
    if (stream->fwriteInt16(art->frameCount) == -1) return -1;
    if (stream->fwriteInt16List(art->xOffsets, ROTATION_COUNT) == -1) return -1;
    if (stream->fwriteInt16List(art->yOffsets, ROTATION_COUNT) == -1) return -1;
    if (stream->fwriteInt32List(art->dataOffsets, ROTATION_COUNT) == -1) return -1;
    if (stream->fwriteInt32(art->dataSize) == -1) return -1;

    return 0;
}

// 0x419828
int save_frame(const char* path, unsigned char* data)
{
    if (data == nullptr) {
        return -1;
    }

    DbFileGuard stream(db_fopen(path, "wb"));
    if (!stream) {
        return -1;
    }

    Art* art = reinterpret_cast<Art*>(data);
    if (art_writeFrameData(art, stream.get()) == -1) {
        return -1;
    }

    for (int index = 0; index < ROTATION_COUNT; index++) {
        if (index == 0 || art->dataOffsets[index - 1] != art->dataOffsets[index]) {
            if (art_writeSubFrameData(data + sizeof(Art) + art->dataOffsets[index], stream.get(), art->frameCount) != 0) {
                return -1;
            }
        }
    }

    return 0;
}

static int artGetDataSize(Art* art)
{
    int dataSize = sizeof(*art) + art->dataSize;

    for (int index = 0; index < ROTATION_COUNT; index++) {
        if (index == 0 || art->dataOffsets[index - 1] != art->dataOffsets[index]) {
            // Assume worst case - every frame is unaligned and need
            // max padding.
            dataSize += (sizeof(int) - 1) * art->frameCount;
        }
    }

    return dataSize;
}

static int paddingForSize(int size)
{
    return (sizeof(int) - size % sizeof(int)) % sizeof(int);
}

} // namespace fallout

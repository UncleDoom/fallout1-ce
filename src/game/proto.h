#pragma once


#include "game/message.h"
#include "game/object_types.h"
#include "game/perk_defs.h"
#include "game/proto_types.h"
#include "game/skill_defs.h"
#include "game/stat_defs.h"
#include "plib/db/db.h"

namespace fallout {

enum class ItemDataMember : int {
    Pid = 0,
    Name = 1,
    Description = 2,
    Fid = 3,
    LightDistance = 4,
    LightIntensity = 5,
    Flags = 6,
    ExtendedFlags = 7,
    Sid = 8,
    Type = 9,
    Material = 11,
    Size = 12,
    Weight = 13,
    Cost = 14,
    InventoryFid = 15,
};

inline constexpr int ITEM_DATA_MEMBER_PID = static_cast<int>(ItemDataMember::Pid);
inline constexpr int ITEM_DATA_MEMBER_NAME = static_cast<int>(ItemDataMember::Name);
inline constexpr int ITEM_DATA_MEMBER_DESCRIPTION = static_cast<int>(ItemDataMember::Description);
inline constexpr int ITEM_DATA_MEMBER_FID = static_cast<int>(ItemDataMember::Fid);
inline constexpr int ITEM_DATA_MEMBER_LIGHT_DISTANCE = static_cast<int>(ItemDataMember::LightDistance);
inline constexpr int ITEM_DATA_MEMBER_LIGHT_INTENSITY = static_cast<int>(ItemDataMember::LightIntensity);
inline constexpr int ITEM_DATA_MEMBER_FLAGS = static_cast<int>(ItemDataMember::Flags);
inline constexpr int ITEM_DATA_MEMBER_EXTENDED_FLAGS = static_cast<int>(ItemDataMember::ExtendedFlags);
inline constexpr int ITEM_DATA_MEMBER_SID = static_cast<int>(ItemDataMember::Sid);
inline constexpr int ITEM_DATA_MEMBER_TYPE = static_cast<int>(ItemDataMember::Type);
inline constexpr int ITEM_DATA_MEMBER_MATERIAL = static_cast<int>(ItemDataMember::Material);
inline constexpr int ITEM_DATA_MEMBER_SIZE = static_cast<int>(ItemDataMember::Size);
inline constexpr int ITEM_DATA_MEMBER_WEIGHT = static_cast<int>(ItemDataMember::Weight);
inline constexpr int ITEM_DATA_MEMBER_COST = static_cast<int>(ItemDataMember::Cost);
inline constexpr int ITEM_DATA_MEMBER_INVENTORY_FID = static_cast<int>(ItemDataMember::InventoryFid);

enum class CritterDataMember : int {
    Pid = 0,
    Name = 1,
    Description = 2,
    Fid = 3,
    LightDistance = 4,
    LightIntensity = 5,
    Flags = 6,
    ExtendedFlags = 7,
    Sid = 8,
    Data = 9,
    HeadFid = 10,
};

inline constexpr int CRITTER_DATA_MEMBER_PID = static_cast<int>(CritterDataMember::Pid);
inline constexpr int CRITTER_DATA_MEMBER_NAME = static_cast<int>(CritterDataMember::Name);
inline constexpr int CRITTER_DATA_MEMBER_DESCRIPTION = static_cast<int>(CritterDataMember::Description);
inline constexpr int CRITTER_DATA_MEMBER_FID = static_cast<int>(CritterDataMember::Fid);
inline constexpr int CRITTER_DATA_MEMBER_LIGHT_DISTANCE = static_cast<int>(CritterDataMember::LightDistance);
inline constexpr int CRITTER_DATA_MEMBER_LIGHT_INTENSITY = static_cast<int>(CritterDataMember::LightIntensity);
inline constexpr int CRITTER_DATA_MEMBER_FLAGS = static_cast<int>(CritterDataMember::Flags);
inline constexpr int CRITTER_DATA_MEMBER_EXTENDED_FLAGS = static_cast<int>(CritterDataMember::ExtendedFlags);
inline constexpr int CRITTER_DATA_MEMBER_SID = static_cast<int>(CritterDataMember::Sid);
inline constexpr int CRITTER_DATA_MEMBER_DATA = static_cast<int>(CritterDataMember::Data);
inline constexpr int CRITTER_DATA_MEMBER_HEAD_FID = static_cast<int>(CritterDataMember::HeadFid);

enum class SceneryDataMember : int {
    Pid = 0,
    Name = 1,
    Description = 2,
    Fid = 3,
    LightDistance = 4,
    LightIntensity = 5,
    Flags = 6,
    ExtendedFlags = 7,
    Sid = 8,
    Type = 9,
    Data = 10,
    Material = 11,
};

inline constexpr int SCENERY_DATA_MEMBER_PID = static_cast<int>(SceneryDataMember::Pid);
inline constexpr int SCENERY_DATA_MEMBER_NAME = static_cast<int>(SceneryDataMember::Name);
inline constexpr int SCENERY_DATA_MEMBER_DESCRIPTION = static_cast<int>(SceneryDataMember::Description);
inline constexpr int SCENERY_DATA_MEMBER_FID = static_cast<int>(SceneryDataMember::Fid);
inline constexpr int SCENERY_DATA_MEMBER_LIGHT_DISTANCE = static_cast<int>(SceneryDataMember::LightDistance);
inline constexpr int SCENERY_DATA_MEMBER_LIGHT_INTENSITY = static_cast<int>(SceneryDataMember::LightIntensity);
inline constexpr int SCENERY_DATA_MEMBER_FLAGS = static_cast<int>(SceneryDataMember::Flags);
inline constexpr int SCENERY_DATA_MEMBER_EXTENDED_FLAGS = static_cast<int>(SceneryDataMember::ExtendedFlags);
inline constexpr int SCENERY_DATA_MEMBER_SID = static_cast<int>(SceneryDataMember::Sid);
inline constexpr int SCENERY_DATA_MEMBER_TYPE = static_cast<int>(SceneryDataMember::Type);
inline constexpr int SCENERY_DATA_MEMBER_DATA = static_cast<int>(SceneryDataMember::Data);
inline constexpr int SCENERY_DATA_MEMBER_MATERIAL = static_cast<int>(SceneryDataMember::Material);

enum class WallDataMember : int {
    Pid = 0,
    Name = 1,
    Description = 2,
    Fid = 3,
    LightDistance = 4,
    LightIntensity = 5,
    Flags = 6,
    ExtendedFlags = 7,
    Sid = 8,
    Material = 9,
};

inline constexpr int WALL_DATA_MEMBER_PID = static_cast<int>(WallDataMember::Pid);
inline constexpr int WALL_DATA_MEMBER_NAME = static_cast<int>(WallDataMember::Name);
inline constexpr int WALL_DATA_MEMBER_DESCRIPTION = static_cast<int>(WallDataMember::Description);
inline constexpr int WALL_DATA_MEMBER_FID = static_cast<int>(WallDataMember::Fid);
inline constexpr int WALL_DATA_MEMBER_LIGHT_DISTANCE = static_cast<int>(WallDataMember::LightDistance);
inline constexpr int WALL_DATA_MEMBER_LIGHT_INTENSITY = static_cast<int>(WallDataMember::LightIntensity);
inline constexpr int WALL_DATA_MEMBER_FLAGS = static_cast<int>(WallDataMember::Flags);
inline constexpr int WALL_DATA_MEMBER_EXTENDED_FLAGS = static_cast<int>(WallDataMember::ExtendedFlags);
inline constexpr int WALL_DATA_MEMBER_SID = static_cast<int>(WallDataMember::Sid);
inline constexpr int WALL_DATA_MEMBER_MATERIAL = static_cast<int>(WallDataMember::Material);

enum class MiscDataMember : int {
    Pid = 0,
    Name = 1,
    Description = 2,
    Fid = 3,
    LightDistance = 4,
    LightIntensity = 5,
    Flags = 6,
    ExtendedFlags = 7,
};

inline constexpr int MISC_DATA_MEMBER_PID = static_cast<int>(MiscDataMember::Pid);
inline constexpr int MISC_DATA_MEMBER_NAME = static_cast<int>(MiscDataMember::Name);
inline constexpr int MISC_DATA_MEMBER_DESCRIPTION = static_cast<int>(MiscDataMember::Description);
inline constexpr int MISC_DATA_MEMBER_FID = static_cast<int>(MiscDataMember::Fid);
inline constexpr int MISC_DATA_MEMBER_LIGHT_DISTANCE = static_cast<int>(MiscDataMember::LightDistance);
inline constexpr int MISC_DATA_MEMBER_LIGHT_INTENSITY = static_cast<int>(MiscDataMember::LightIntensity);
inline constexpr int MISC_DATA_MEMBER_FLAGS = static_cast<int>(MiscDataMember::Flags);
inline constexpr int MISC_DATA_MEMBER_EXTENDED_FLAGS = static_cast<int>(MiscDataMember::ExtendedFlags);

enum class ProtoDataMemberType : int {
    Int = 1,
    String = 2,
};

inline constexpr int PROTO_DATA_MEMBER_TYPE_INT = static_cast<int>(ProtoDataMemberType::Int);
inline constexpr int PROTO_DATA_MEMBER_TYPE_STRING = static_cast<int>(ProtoDataMemberType::String);

union ProtoDataMemberValue {
    int integerValue;
    char* stringValue;
};

enum class PrototypeMessage : int {
    Name = 0,
    Description = 1,
};

inline constexpr int PROTOTYPE_MESSAGE_NAME = static_cast<int>(PrototypeMessage::Name);
inline constexpr int PROTOTYPE_MESSAGE_DESCRIPTION = static_cast<int>(PrototypeMessage::Description);

extern char cd_path_base[];
extern char proto_path_base[];

extern char* mp_perk_code_strs[1 + PERK_COUNT];
extern char* mp_critter_stats_list[2 + STAT_COUNT];
extern MessageList proto_msg_files[6];
extern char* race_type_strs[RACE_TYPE_COUNT];
extern char* scenery_pro_type[SCENERY_TYPE_COUNT];
extern MessageList proto_main_msg_file;
extern char* item_pro_material[MATERIAL_TYPE_COUNT];
extern char* proto_none_str;
extern char* body_type_strs[BODY_TYPE_COUNT];
extern char* item_pro_type[ITEM_TYPE_COUNT];
extern char* damage_code_strs[DAMAGE_TYPE_COUNT];
extern char* cal_type_strs[CALIBER_TYPE_COUNT];
extern char** perk_code_strs;
extern char** critter_stats_list;

void proto_make_path(char* path, int pid);
int proto_list_str(int pid, char* proto_path);
size_t proto_size(int type);
bool proto_action_can_use(int pid);
bool proto_action_can_use_on(int pid);
bool proto_action_can_look_at(int pid);
bool proto_action_can_talk_to(int pid);
int proto_action_can_pickup(int pid);
char* proto_name(int pid);
char* proto_description(int pid);
int proto_critter_init(Proto* a1, int a2);
void clear_pupdate_data(Object* obj);
int proto_read_protoUpdateData(Object* obj, DB_FILE* stream);
int proto_write_protoUpdateData(Object* obj, DB_FILE* stream);
int proto_update_gen(Object* obj);
int proto_update_init(Object* obj);
int proto_dude_update_gender();
int proto_dude_init(const char* path);
int proto_data_member(int pid, int member, ProtoDataMemberValue* value);
int proto_init();
void proto_reset();
void proto_exit();
int proto_header_load();
int proto_save_pid(int pid);
int proto_load_pid(int pid, Proto** out_proto);
int proto_find_free_subnode(int type, Proto** out_ptr);
void proto_remove_all();
int proto_ptr(int pid, Proto** out_proto);
int proto_undo_new_id(int type);
int proto_max_id(int a1);
int ResetPlayer();

} // namespace fallout

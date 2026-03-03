#pragma once


#include "game/combat_defs.h"
#include "game/enum_utils.h"
#include "game/message.h"
#include "game/object_types.h"
#include "int/intrpret.h"
#include "plib/db/db.h"

namespace fallout {

#define SCRIPT_FLAG_0x01 0x01
#define SCRIPT_FLAG_0x02 0x02
#define SCRIPT_FLAG_0x04 0x04
#define SCRIPT_FLAG_0x08 0x08
#define SCRIPT_FLAG_0x10 0x10

#define GAME_TIME_TICKS_PER_HOUR (60 * 60 * 10)
#define GAME_TIME_TICKS_PER_DAY (24 * 60 * 60 * 10)
#define GAME_TIME_TICKS_PER_YEAR (365 * 24 * 60 * 60 * 10)

inline constexpr int SCRIPT_DIALOG_MESSAGE_LIST_CAPACITY = 1000;

// Bitmask of pending script-initiated requests.
enum class ScriptRequests : unsigned {
    Combat = 0x01,
    TownMap = 0x02,
    WorldMap = 0x04,
    Elevator = 0x08,
    Explosion = 0x10,
    Dialog = 0x20,
    NoInitialCombatState = 0x40,
    Endgame = 0x80,
    Looting = 0x100,
    Stealing = 0x200,
    Locked = 0x400,
};
DEFINE_ENUM_FLAG_OPERATORS(ScriptRequests)

// Legacy constants
inline constexpr int SCRIPT_REQUEST_COMBAT = static_cast<int>(ScriptRequests::Combat);
inline constexpr int SCRIPT_REQUEST_TOWN_MAP = static_cast<int>(ScriptRequests::TownMap);
inline constexpr int SCRIPT_REQUEST_WORLD_MAP = static_cast<int>(ScriptRequests::WorldMap);
inline constexpr int SCRIPT_REQUEST_ELEVATOR = static_cast<int>(ScriptRequests::Elevator);
inline constexpr int SCRIPT_REQUEST_EXPLOSION = static_cast<int>(ScriptRequests::Explosion);
inline constexpr int SCRIPT_REQUEST_DIALOG = static_cast<int>(ScriptRequests::Dialog);
inline constexpr int SCRIPT_REQUEST_NO_INITIAL_COMBAT_STATE = static_cast<int>(ScriptRequests::NoInitialCombatState);
inline constexpr int SCRIPT_REQUEST_ENDGAME = static_cast<int>(ScriptRequests::Endgame);
inline constexpr int SCRIPT_REQUEST_LOOTING = static_cast<int>(ScriptRequests::Looting);
inline constexpr int SCRIPT_REQUEST_STEALING = static_cast<int>(ScriptRequests::Stealing);
inline constexpr int SCRIPT_REQUEST_LOCKED = static_cast<int>(ScriptRequests::Locked);


enum class ScriptType : int {
    System = 0, // s_system
    Spatial = 1, // s_spatial
    Timed = 2, // s_time
    Item = 3, // s_item
    Critter = 4, // s_critter
    Count = 5,
};

// Legacy constants
inline constexpr int SCRIPT_TYPE_SYSTEM = static_cast<int>(ScriptType::System);
inline constexpr int SCRIPT_TYPE_SPATIAL = static_cast<int>(ScriptType::Spatial);
inline constexpr int SCRIPT_TYPE_TIMED = static_cast<int>(ScriptType::Timed);
inline constexpr int SCRIPT_TYPE_ITEM = static_cast<int>(ScriptType::Item);
inline constexpr int SCRIPT_TYPE_CRITTER = static_cast<int>(ScriptType::Critter);
inline constexpr int SCRIPT_TYPE_COUNT = static_cast<int>(ScriptType::Count);

enum class ScriptProc : int {
    NoProc = 0,
    Start = 1,
    Spatial = 2,
    Description = 3,
    Pickup = 4,
    Drop = 5,
    Use = 6,
    UseObjOn = 7,
    UseSkillOn = 8,
    Proc9 = 9, // use_ad_on_proc
    Proc10 = 10, // use_disad_on_proc
    Talk = 11,
    Critter = 12,
    Combat = 13,
    Damage = 14,
    MapEnter = 15,
    MapExit = 16,
    Create = 17,
    Destroy = 18,
    Proc19 = 19, // barter_init_proc
    Proc20 = 20, // barter_proc
    LookAt = 21,
    Timed = 22,
    MapUpdate = 23,
    Count = 24,
};

// Legacy constants
inline constexpr int SCRIPT_PROC_NO_PROC = static_cast<int>(ScriptProc::NoProc);
inline constexpr int SCRIPT_PROC_START = static_cast<int>(ScriptProc::Start);
inline constexpr int SCRIPT_PROC_SPATIAL = static_cast<int>(ScriptProc::Spatial);
inline constexpr int SCRIPT_PROC_DESCRIPTION = static_cast<int>(ScriptProc::Description);
inline constexpr int SCRIPT_PROC_PICKUP = static_cast<int>(ScriptProc::Pickup);
inline constexpr int SCRIPT_PROC_DROP = static_cast<int>(ScriptProc::Drop);
inline constexpr int SCRIPT_PROC_USE = static_cast<int>(ScriptProc::Use);
inline constexpr int SCRIPT_PROC_USE_OBJ_ON = static_cast<int>(ScriptProc::UseObjOn);
inline constexpr int SCRIPT_PROC_USE_SKILL_ON = static_cast<int>(ScriptProc::UseSkillOn);
inline constexpr int SCRIPT_PROC_9 = static_cast<int>(ScriptProc::Proc9);
inline constexpr int SCRIPT_PROC_10 = static_cast<int>(ScriptProc::Proc10);
inline constexpr int SCRIPT_PROC_TALK = static_cast<int>(ScriptProc::Talk);
inline constexpr int SCRIPT_PROC_CRITTER = static_cast<int>(ScriptProc::Critter);
inline constexpr int SCRIPT_PROC_COMBAT = static_cast<int>(ScriptProc::Combat);
inline constexpr int SCRIPT_PROC_DAMAGE = static_cast<int>(ScriptProc::Damage);
inline constexpr int SCRIPT_PROC_MAP_ENTER = static_cast<int>(ScriptProc::MapEnter);
inline constexpr int SCRIPT_PROC_MAP_EXIT = static_cast<int>(ScriptProc::MapExit);
inline constexpr int SCRIPT_PROC_CREATE = static_cast<int>(ScriptProc::Create);
inline constexpr int SCRIPT_PROC_DESTROY = static_cast<int>(ScriptProc::Destroy);
inline constexpr int SCRIPT_PROC_19 = static_cast<int>(ScriptProc::Proc19);
inline constexpr int SCRIPT_PROC_20 = static_cast<int>(ScriptProc::Proc20);
inline constexpr int SCRIPT_PROC_LOOK_AT = static_cast<int>(ScriptProc::LookAt);
inline constexpr int SCRIPT_PROC_TIMED = static_cast<int>(ScriptProc::Timed);
inline constexpr int SCRIPT_PROC_MAP_UPDATE = static_cast<int>(ScriptProc::MapUpdate);
inline constexpr int SCRIPT_PROC_COUNT = static_cast<int>(ScriptProc::Count);

class Script {
public:
    int scr_id;
    int scr_next;

    union {
        struct {
            // scr_udata.sp.built_tile
            int built_tile;
            // scr_udata.sp.radius
            int radius;
        } sp;
        struct {
            // scr_udata.tm.time
            int time;
        } tm;
    };

    int scr_flags;
    int scr_script_idx;
    Program* program;
    int scr_oid;
    int scr_local_var_offset;
    int scr_num_local_vars;

    // return value?
    int field_28;

    // Currently executed action.
    //
    // See [op_script_action].
    int action;
    int fixedParam;
    Object* owner;

    // source_obj
    Object* source;

    // target_obj
    Object* target;
    int actionBeingUsed;
    int scriptOverrides;
    int field_48;
    int howMuch;
    int run_info_flags;
    int procs[SCRIPT_PROC_COUNT];
    int field_C4;
    int field_C8;
    int field_CC;
    int field_D0;
    int field_D4;
    int field_D8;
    int field_DC;

    int clearCombatRequests();
    int removeLocalVars();
    int buildLookupTable();
    int writeSubNode(DB_FILE* stream);
    int readSubNode(DB_FILE* stream);
};

extern int num_script_indexes;

extern MessageList script_dialog_msgs[SCRIPT_DIALOG_MESSAGE_LIST_CAPACITY];
extern MessageList script_message_file;

int game_time();
void game_time_date(int* monthPtr, int* dayPtr, int* yearPtr);
int game_time_hour();
char* game_time_hour_str();
void inc_game_time(int inc);
void inc_game_time_in_seconds(int inc);
void set_game_time(int time);
void set_game_time_in_seconds(int time);
int gtime_q_add();
int gtime_q_process(Object* obj, void* data);
int scr_map_q_process(Object* obj, void* data);
int new_obj_id();
int scr_find_sid_from_program(Program* program);
Object* scr_find_obj_from_program(Program* program);
int scr_set_objs(int sid, Object* source, Object* target);
void scr_set_ext_param(int a1, int a2);
int scr_set_action_num(int sid, int a2);
Program* loadProgram(const char* name);
void scrSetQueueTestVals(Object* a1, int a2);
int scrQueueRemoveFixed(Object* obj, void* data);
int script_q_add(int sid, int delay, int param);
int script_q_save(DB_FILE* stream, void* data);
int script_q_load(DB_FILE* stream, void** dataPtr);
int script_q_process(Object* obj, void* data);
int scripts_clear_state();

int scripts_check_state();
int scripts_check_state_in_combat();
void scripts_request_townmap();
void scripts_request_worldmap();
int scripts_request_elevator(int elevator);
int scripts_request_explosion(int tile, int elevation, int minDamage, int maxDamage);
void scripts_request_dialog(Object* a1);
void scripts_request_endgame_slideshow();
int scripts_request_loot_container(Object* a1, Object* a2);
int scripts_request_steal_container(Object* a1, Object* a2);
void script_make_path(char* path);
int exec_script_proc(int sid, int proc);
int scr_find_str_run_info(int a1, int* a2, int sid);
int scr_list_str(int index, char* name, size_t size);
int scr_set_dude_script();
int scr_clear_dude_script();
int scr_init();
int scr_reset();
int scr_game_init();
int scr_game_reset();
int scr_exit();
int scr_message_free();
int scr_game_exit();
int scr_enable();
int scr_disable();
void scr_enable_critters();
void scr_disable_critters();
int scr_game_save(DB_FILE* stream);
int scr_game_load(DB_FILE* stream);
int scr_game_load2(DB_FILE* stream);
int scr_save(DB_FILE* stream);
int scr_load(DB_FILE* stream);
int scr_ptr(int sid, Script** script);
int scr_new(int* sidPtr, int scriptType);

int scr_remove(int index);
int scr_remove_all();
int scr_remove_all_force();
Script* scr_find_first_at(int elevation);
Script* scr_find_next_at();
bool scr_spatials_enabled();
void scr_spatials_enable();
void scr_spatials_disable();
bool scr_chk_spatials_in(Object* obj, int tile, int elevation);
bool tile_in_tile_bound(int tile1, int radius, int tile2);
int scr_load_all_scripts();
void scr_exec_map_enter_scripts();
void scr_exec_map_update_scripts();
void scr_exec_map_exit_scripts();
int scr_get_dialog_msg_file(int a1, MessageList** out_message_list);
char* scr_get_msg_str(int messageListId, int messageId);
char* scr_get_msg_str_speech(int messageListId, int messageId, int a3);
int scr_get_local_var(int sid, int variable, ProgramValue& value);
int scr_set_local_var(int sid, int variable, ProgramValue& value);
bool scr_end_combat();
int scr_explode_scenery(Object* a1, int tile, int radius, int elevation);

} // namespace fallout

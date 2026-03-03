#include "int/support/intextra.h"

#include <climits>
#include <cstdio>
#include <cstring>

#include "game/actions.h"
#include "game/anim.h"
#include "game/combat.h"
#include "game/combatai.h"
#include "game/critter.h"
#include "game/display.h"
#include "game/endgame.h"
#include "game/game.h"
#include "game/gconfig.h"
#include "game/gdialog.h"
#include "game/gmovie.h"
#include "game/gsound.h"
#include "game/intface.h"
#include "game/item.h"
#include "game/light.h"
#include "game/loadsave.h"
#include "game/map.h"
#include "game/object.h"
#include "game/palette.h"
#include "game/perk.h"
#include "game/protinst.h"
#include "game/proto.h"
#include "game/queue.h"
#include "game/reaction.h"
#include "game/roll.h"
#include "game/scripts.h"
#include "game/skill.h"
#include "game/stat.h"
#include "game/textobj.h"
#include "game/tile.h"
#include "game/trait.h"
#include "game/worldmap.h"
#include "int/dialog.h"
#include "plib/color/color.h"
#include "plib/gnw/debug.h"
#include "plib/gnw/input.h"
#include "plib/gnw/rect.h"
#include "plib/gnw/svga.h"
#include "plib/gnw/vcr.h"

namespace fallout {

enum Metarule {
    METARULE_SIGNAL_END_GAME = 13,
    METARULE_FIRST_RUN = 14,
    METARULE_ELEVATOR = 15,
    METARULE_PARTY_COUNT = 16,
    METARULE_IS_LOADGAME = 22,
};

enum CritterTrait {
    CRITTER_TRAIT_PERK = 0,
    CRITTER_TRAIT_OBJECT = 1,
    CRITTER_TRAIT_TRAIT = 2,
};

enum CritterTraitObject {
    CRITTER_TRAIT_OBJECT_AI_PACKET = 5,
    CRITTER_TRAIT_OBJECT_TEAM = 6,
    CRITTER_TRAIT_OBJECT_ROTATION = 10,
    CRITTER_TRAIT_OBJECT_IS_INVISIBLE = 666,
    CRITTER_TRAIT_OBJECT_GET_INVENTORY_WEIGHT = 669,
};

// See `op_critter_state`.
enum CritterState {
    CRITTER_STATE_NORMAL = 0x00,
    CRITTER_STATE_DEAD = 0x01,
    CRITTER_STATE_PRONE = 0x02,
};

enum {
    INVEN_TYPE_WORN = 0,
    INVEN_TYPE_RIGHT_HAND = 1,
    INVEN_TYPE_LEFT_HAND = 2,
    INVEN_TYPE_INV_COUNT = -2,
};

enum FloatingMessageType {
    FLOATING_MESSAGE_TYPE_WARNING = -2,
    FLOATING_MESSAGE_TYPE_COLOR_SEQUENCE = -1,
    FLOATING_MESSAGE_TYPE_NORMAL = 0,
    FLOATING_MESSAGE_TYPE_BLACK,
    FLOATING_MESSAGE_TYPE_RED,
    FLOATING_MESSAGE_TYPE_GREEN,
    FLOATING_MESSAGE_TYPE_BLUE,
    FLOATING_MESSAGE_TYPE_PURPLE,
    FLOATING_MESSAGE_TYPE_NEAR_WHITE,
    FLOATING_MESSAGE_TYPE_LIGHT_RED,
    FLOATING_MESSAGE_TYPE_YELLOW,
    FLOATING_MESSAGE_TYPE_WHITE,
    FLOATING_MESSAGE_TYPE_GREY,
    FLOATING_MESSAGE_TYPE_DARK_GREY,
    FLOATING_MESSAGE_TYPE_LIGHT_GREY,
    FLOATING_MESSAGE_TYPE_COUNT,
};

enum OpRegAnimFunc {
    OP_REG_ANIM_FUNC_BEGIN = 1,
    OP_REG_ANIM_FUNC_CLEAR = 2,
    OP_REG_ANIM_FUNC_END = 3,
};

// TODO: Remove.
// 0x4F4144
char _aCritter[] = "<Critter>";

// NOTE: This value is a little bit odd. It's used to handle 2 operations:
// [op_start_gdialog] and [op_dialogue_reaction]. It's not used outside those
// functions.
//
// When used inside [op_start_gdialog] this value stores [Fidget] constant
// (1 - Good, 4 - Neutral, 7 - Bad).
//
// When used inside [op_dialogue_reaction] this value contains specified
// reaction (-1 - Good, 0 - Neutral, 1 - Bad).
//
// 0x5970D0
static int dialogue_mood;

// 0x44B5A8
void dbg_error(Program* program, const char* name, int error)
{
    // 0ч5054C0
    static const char* dbg_error_strs[SCRIPT_ERROR_COUNT] = {
        "unimped",
        "obj is nullptr",
        "can't match program to sid",
        "follows",
    };

    char string[260];

    snprintf(string, sizeof(string), "Script Error: %s: op_%s: %s", program->name, name, dbg_error_strs[error]);

    debug_printf(string);
}

// 0x44B5E4
static void int_debug(const char* format, ...)
{
    char string[260];

    va_list argptr;
    va_start(argptr, format);
    vsnprintf(string, sizeof(string), format, argptr);
    va_end(argptr);

    debug_printf(string);
}

// 0x44B624
static int scripts_tile_is_visible(int tile)
{
    if (abs(tile_center_tile - tile) % 200 < 5) {
        return 1;
    }

    if (abs(tile_center_tile - tile) / 200 < 5) {
        return 1;
    }

    return 0;
}

// 0x44B674
static int correctFidForRemovedItem(Object* critter, Object* item, int flags)
{
    if (critter == obj_dude) {
        intface_update_items(true);
    }

    int fid = critter->fid;
    int anim = (fid & 0xF000) >> 12;
    int newFid = -1;

    if ((flags & OBJECT_IN_ANY_HAND) != 0) {
        if (critter == obj_dude) {
            if (intface_is_item_right_hand()) {
                if ((flags & OBJECT_IN_RIGHT_HAND) != 0) {
                    anim = 0;
                }
            } else {
                if ((flags & OBJECT_IN_LEFT_HAND) != 0) {
                    anim = 0;
                }
            }
        } else {
            if ((flags & OBJECT_IN_RIGHT_HAND) != 0) {
                anim = 0;
            }
        }

        if (anim == 0) {
            newFid = art_id(FID_TYPE(fid), fid & 0xFFF, FID_ANIM_TYPE(fid), 0, (fid & 0x70000000) >> 28);
        }
    } else {
        if (critter == obj_dude) {
            newFid = art_id(FID_TYPE(fid), art_vault_guy_num, FID_ANIM_TYPE(fid), anim, (fid & 0x70000000) >> 28);
            adjust_ac(obj_dude, item, nullptr);
        }
    }

    if (newFid != -1) {
        Rect rect;
        obj_change_fid(critter, newFid, &rect);
        tile_refresh_rect(&rect, map_elevation);
    }

    return 0;
}

// 0x44B78C
static void op_give_exp_points(Program* program)
{
    int xp = program->stackPopInteger();

    if (stat_pc_add_experience(xp) != 0) {
        int_debug("\nScript Error: %s: op_give_exp_points: stat_pc_set failed");
    }
}

// 0x44B7E0
static void op_scr_return(Program* program)
{
    int data = program->stackPopInteger();

    int sid = scr_find_sid_from_program(program);

    Script* script;
    if (scr_ptr(sid, &script) != -1) {
        script->field_28 = data;
    }
}

// 0x44B838
static void op_play_sfx(Program* program)
{
    char* name = program->stackPopString();

    gsound_play_sfx_file(name);
}

// 0x44B888
static void op_set_map_start(Program* program)
{
    int rotation = program->stackPopInteger();
    int elevation = program->stackPopInteger();
    int y = program->stackPopInteger();
    int x = program->stackPopInteger();

    if (map_set_elevation(elevation) != 0) {
        int_debug("\nScript Error: %s: op_set_map_start: map_set_elevation failed", program->name);
        return;
    }

    int tile = 200 * y + x;
    if (tile_set_center(tile, TILE_SET_CENTER_REFRESH_WINDOW | TILE_SET_CENTER_FLAG_IGNORE_SCROLL_RESTRICTIONS) != 0) {
        int_debug("\nScript Error: %s: op_set_map_start: tile_set_center failed", program->name);
        return;
    }

    map_set_entrance_hex(tile, elevation, rotation);
}

// 0x44B94C
static void op_override_map_start(Program* program)
{
    program->flags |= PROGRAM_FLAG_0x20;

    int rotation = program->stackPopInteger();
    int elevation = program->stackPopInteger();
    int y = program->stackPopInteger();
    int x = program->stackPopInteger();

    char text[60];
    snprintf(text, sizeof(text), "OVERRIDE_MAP_START: x: %d, y: %d", x, y);
    debug_printf(text);

    int tile = 200 * y + x;
    int previousTile = tile_center_tile;
    if (tile != -1) {
        if (obj_set_rotation(obj_dude, rotation, nullptr) != 0) {
            int_debug("\nError: %s: obj_set_rotation failed in override_map_start!", program->name);
        }

        if (obj_move_to_tile(obj_dude, tile, elevation, nullptr) != 0) {
            int_debug("\nError: %s: obj_move_to_tile failed in override_map_start!", program->name);

            if (obj_move_to_tile(obj_dude, previousTile, elevation, nullptr) != 0) {
                int_debug("\nError: %s: obj_move_to_tile RECOVERY Also failed!");
                exit(1);
            }
        }

        tile_set_center(tile, TILE_SET_CENTER_REFRESH_WINDOW);
        tile_refresh_display();
    }

    program->flags &= ~PROGRAM_FLAG_0x20;
}

// 0x44BAA4
static void op_has_skill(Program* program)
{
    int skill = program->stackPopInteger();
    Object* object = static_cast<Object*>(program->stackPopPointer());

    int result = 0;
    if (object != nullptr) {
        if (PID_TYPE(object->pid) == OBJ_TYPE_CRITTER) {
            result = skill_level(object, skill);
        }
    } else {
        dbg_error(program, "has_skill", SCRIPT_ERROR_OBJECT_IS_NULL);
    }

    program->stackPushInteger(result);
}

// 0x44BB50
static void op_using_skill(Program* program)
{
    int skill = program->stackPopInteger();
    Object* object = static_cast<Object*>(program->stackPopPointer());

    // NOTE: In the original source code this value is left uninitialized, that
    // explains why garbage is returned when using something else than dude and
    // SKILL_SNEAK as arguments.
    int result = 0;

    if (skill == SKILL_SNEAK && object == obj_dude) {
        result = is_pc_flag(PC_FLAG_SNEAKING);
    }

    program->stackPushInteger(result);
}

// 0x44BBE4
static void op_roll_vs_skill(Program* program)
{
    int modifier = program->stackPopInteger();
    int skill = program->stackPopInteger();
    Object* object = static_cast<Object*>(program->stackPopPointer());

    int roll = ROLL_CRITICAL_FAILURE;
    if (object != nullptr) {
        if (PID_TYPE(object->pid) == OBJ_TYPE_CRITTER) {
            int sid = scr_find_sid_from_program(program);

            Script* script;
            if (scr_ptr(sid, &script) != -1) {
                roll = skill_result(object, skill, modifier, &(script->howMuch));
            }
        }
    } else {
        dbg_error(program, "roll_vs_skill", SCRIPT_ERROR_OBJECT_IS_NULL);
    }

    program->stackPushInteger(roll);
}

// 0x44BCAC
static void op_skill_contest(Program* program)
{
    int data[3];

    for (int arg = 0; arg < 3; arg++) {
        data[arg] = program->stackPopInteger();
    }

    dbg_error(program, "skill_contest", SCRIPT_ERROR_NOT_IMPLEMENTED);
    program->stackPushInteger(0);
}

// 0x44BD48
static void op_do_check(Program* program)
{
    int mod = program->stackPopInteger();
    int stat = program->stackPopInteger();
    Object* object = static_cast<Object*>(program->stackPopPointer());

    int roll = 0;
    if (object != nullptr) {
        int sid = scr_find_sid_from_program(program);

        Script* script;
        if (scr_ptr(sid, &script) != -1) {
            switch (stat) {
            case STAT_STRENGTH:
            case STAT_PERCEPTION:
            case STAT_ENDURANCE:
            case STAT_CHARISMA:
            case STAT_INTELLIGENCE:
            case STAT_AGILITY:
            case STAT_LUCK:
                roll = stat_result(object, stat, mod, &(script->howMuch));
                break;
            default:
                int_debug("\nScript Error: %s: op_do_check: Stat out of range", program->name);
                break;
            }
        }
    } else {
        dbg_error(program, "do_check", SCRIPT_ERROR_OBJECT_IS_NULL);
    }

    program->stackPushInteger(roll);
}

// 0x44BE3C
static void op_is_success(Program* program)
{
    int data = program->stackPopInteger();

    int result = -1;

    switch (data) {
    case ROLL_CRITICAL_FAILURE:
    case ROLL_FAILURE:
        result = 0;
        break;
    case ROLL_SUCCESS:
    case ROLL_CRITICAL_SUCCESS:
        result = 1;
        break;
    }

    program->stackPushInteger(result);
}

// 0x44BEB4
static void op_is_critical(Program* program)
{
    int data = program->stackPopInteger();

    int result = -1;

    switch (data) {
    case ROLL_CRITICAL_FAILURE:
    case ROLL_CRITICAL_SUCCESS:
        result = 1;
        break;
    case ROLL_FAILURE:
    case ROLL_SUCCESS:
        result = 0;
        break;
    }

    program->stackPushInteger(result);
}

// 0x44BF1C
static void op_how_much(Program* program)
{
    int data = program->stackPopInteger();

    int result = 0;

    int sid = scr_find_sid_from_program(program);

    Script* script;
    if (scr_ptr(sid, &script) != -1) {
        result = script->howMuch;
    } else {
        dbg_error(program, "how_much", SCRIPT_ERROR_CANT_MATCH_PROGRAM_TO_SID);
    }

    program->stackPushInteger(result);
}

// 0x44BFA0
static void op_reaction_roll(Program* program)
{
    int data[3];

    for (int arg = 0; arg < 3; arg++) {
        data[arg] = program->stackPopInteger();
    }

    program->stackPushInteger(reaction_roll(data[2], data[1], data[0]));
}

// 0x44C024
static void op_reaction_influence(Program* program)
{
    int data[3];

    for (int arg = 0; arg < 3; arg++) {
        data[arg] = program->stackPopInteger();
    }

    program->stackPushInteger(reaction_influence(data[2], data[1], data[0]));
}

// 0x44C0A8
static void op_random(Program* program)
{
    int data[2];

    for (int arg = 0; arg < 2; arg++) {
        data[arg] = program->stackPopInteger();
    }

    int result;
    if (vcr_status() == VCR_STATE_TURNED_OFF) {
        result = roll_random(data[1], data[0]);
    } else {
        result = (data[0] - data[1]) / 2;
    }

    program->stackPushInteger(result);
}

// 0x44C13C
static void op_roll_dice(Program* program)
{
    int data[2];

    for (int arg = 0; arg < 2; arg++) {
        data[arg] = program->stackPopInteger();
    }

    dbg_error(program, "roll_dice", SCRIPT_ERROR_NOT_IMPLEMENTED);

    program->stackPushInteger(0);
}

// 0x44C1BC
static void op_move_to(Program* program)
{
    int elevation = program->stackPopInteger();
    int tile = program->stackPopInteger();
    Object* object = static_cast<Object*>(program->stackPopPointer());

    int newTile;

    if (object != nullptr) {
        if (object == obj_dude) {
            bool tileLimitingEnabled = tile_get_scroll_limiting();
            bool tileBlockingEnabled = tile_get_scroll_blocking();

            if (tileLimitingEnabled) {
                tile_disable_scroll_limiting();
            }

            if (tileBlockingEnabled) {
                tile_disable_scroll_blocking();
            }

            Rect rect;
            newTile = obj_move_to_tile(object, tile, elevation, &rect);
            if (newTile != -1) {
                tile_set_center(object->tile, TILE_SET_CENTER_REFRESH_WINDOW);
            }

            if (tileLimitingEnabled) {
                tile_enable_scroll_limiting();
            }

            if (tileBlockingEnabled) {
                tile_enable_scroll_blocking();
            }
        } else {
            Rect before;
            obj_bound(object, &before);

            if (object->elevation != elevation && PID_TYPE(object->pid) == OBJ_TYPE_CRITTER) {
                combat_delete_critter(object);
            }

            Rect after;
            newTile = obj_move_to_tile(object, tile, elevation, &after);
            if (newTile != -1) {
                before.minBound(after);
                tile_refresh_rect(&before, map_elevation);
            }
        }
    } else {
        dbg_error(program, "move_to", SCRIPT_ERROR_OBJECT_IS_NULL);
        newTile = -1;
    }

    program->stackPushInteger(newTile);
}

// 0x44C31C
static void op_create_object_sid(Program* program)
{
    int data[4];

    for (int arg = 0; arg < 4; arg++) {
        data[arg] = program->stackPopInteger();
    }

    int pid = data[3];
    int tile = data[2];
    int elevation = data[1];
    int sid = data[0];

    Object* object = nullptr;

    if (isLoadingGame() != 0) {
        debug_printf("\nError: attempt to Create critter in load/save-game: %s!", program->name);
        goto out;
    }

    Proto* proto;
    if (proto_ptr(pid, &proto) != -1) {
        if (obj_new(&object, proto->fid, pid) != -1) {
            if (tile == -1) {
                tile = 0;
            }

            Rect rect;
            if (obj_move_to_tile(object, tile, elevation, &rect) != -1) {
                tile_refresh_rect(&rect, object->elevation);
            }
        }
    }

    if (sid != -1) {
        int scriptType = 0;
        switch (PID_TYPE(object->pid)) {
        case OBJ_TYPE_CRITTER:
            scriptType = SCRIPT_TYPE_CRITTER;
            break;
        case OBJ_TYPE_ITEM:
        case OBJ_TYPE_SCENERY:
            scriptType = SCRIPT_TYPE_ITEM;
            break;
        }

        if (object->sid != -1) {
            scr_remove(object->sid);
            object->sid = -1;
        }

        if (scr_new(&(object->sid), scriptType) == -1) {
            goto out;
        }

        Script* script;
        if (scr_ptr(object->sid, &script) == -1) {
            goto out;
        }

        script->scr_script_idx = sid - 1;

        object->id = new_obj_id();
        script->scr_oid = object->id;
        script->owner = object;
        scr_find_str_run_info(sid - 1, &(script->run_info_flags), object->sid);
    };

out:

    program->stackPushPointer(object);
}

// 0x44C4FC
static void op_destroy_object(Program* program)
{
    program->flags |= PROGRAM_FLAG_0x20;

    Object* object = static_cast<Object*>(program->stackPopPointer());

    if (object == nullptr) {
        dbg_error(program, "destroy_object", SCRIPT_ERROR_OBJECT_IS_NULL);
        program->flags &= ~PROGRAM_FLAG_0x20;
        return;
    }

    if (PID_TYPE(object->pid) == OBJ_TYPE_CRITTER) {
        if (isLoadingGame()) {
            debug_printf("\nError: attempt to destroy critter in load/save-game: %s!", program->name);
            program->flags &= ~PROGRAM_FLAG_0x20;
            return;
        }
    }

    bool isSelf = object == scr_find_obj_from_program(program);

    if (PID_TYPE(object->pid) == OBJ_TYPE_CRITTER) {
        combat_delete_critter(object);
    }

    Object* owner = obj_top_environment(object);
    if (owner != nullptr) {
        int quantity = item_count(owner, object);
        item_remove_mult(owner, object, quantity);

        if (owner == obj_dude) {
            intface_update_items(true);
        }

        obj_connect(object, 1, 0, nullptr);

        if (isSelf) {
            object->sid = -1;
            object->flags |= (OBJECT_HIDDEN | OBJECT_NO_SAVE);
        } else {
            register_clear(object);
            obj_erase_object(object, nullptr);
        }
    } else {
        register_clear(object);

        Rect rect;
        obj_erase_object(object, &rect);
        tile_refresh_rect(&rect, map_elevation);
    }

    program->flags &= ~PROGRAM_FLAG_0x20;

    if (isSelf) {
        program->flags |= PROGRAM_FLAG_0x0100;
    }
}

// 0x44C668
static void op_display_msg(Program* program)
{
    char* string = program->stackPopString();
    display_print(string);

    bool showScriptMessages = false;
    game_config.getBool(GAME_CONFIG_DEBUG_KEY, GAME_CONFIG_SHOW_SCRIPT_MESSAGES_KEY, &showScriptMessages);

    if (showScriptMessages) {
        debug_printf("\n");
        debug_printf(string);
    }
}

// 0x44C6F8
static void op_script_overrides(Program* program)
{
    int sid = scr_find_sid_from_program(program);

    Script* script;
    if (scr_ptr(sid, &script) != -1) {
        script->scriptOverrides = 1;
    } else {
        dbg_error(program, "script_overrides", SCRIPT_ERROR_CANT_MATCH_PROGRAM_TO_SID);
    }
}

// 0x44C738
static void op_obj_is_carrying_obj_pid(Program* program)
{
    int pid = program->stackPopInteger();
    Object* obj = static_cast<Object*>(program->stackPopPointer());

    int result = 0;
    if (obj != nullptr) {
        result = inven_pid_quantity_carried(obj, pid);
    } else {
        dbg_error(program, "obj_is_carrying_obj_pid", SCRIPT_ERROR_OBJECT_IS_NULL);
    }

    program->stackPushInteger(result);
}

// 0x44C7D0
static void op_tile_contains_obj_pid(Program* program)
{
    int pid = program->stackPopInteger();
    int elevation = program->stackPopInteger();
    int tile = program->stackPopInteger();

    int result = 0;

    Object* object = obj_find_first_at(elevation);
    while (object) {
        if (object->tile == tile && object->pid == pid) {
            result = 1;
            break;
        }
        object = obj_find_next_at();
    }

    program->stackPushInteger(result);
}

// 0x44C87C
static void op_self_obj(Program* program)
{
    Object* object = scr_find_obj_from_program(program);
    program->stackPushPointer(object);
}

// 0x44C8A0
static void op_source_obj(Program* program)
{
    Object* object = nullptr;

    int sid = scr_find_sid_from_program(program);

    Script* script;
    if (scr_ptr(sid, &script) != -1) {
        object = script->source;
    } else {
        dbg_error(program, "source_obj", SCRIPT_ERROR_CANT_MATCH_PROGRAM_TO_SID);
    }

    program->stackPushPointer(object);
}

// 0x44C8F4
static void op_target_obj(Program* program)
{
    Object* object = nullptr;

    int sid = scr_find_sid_from_program(program);

    Script* script;
    if (scr_ptr(sid, &script) != -1) {
        object = script->target;
    } else {
        dbg_error(program, "target_obj", SCRIPT_ERROR_CANT_MATCH_PROGRAM_TO_SID);
    }

    program->stackPushPointer(object);
}

// 0x44C948
static void op_dude_obj(Program* program)
{
    program->stackPushPointer(obj_dude);
}

// NOTE: The implementation is the same as in [op_target_obj].
//
// 0x44C968
static void op_obj_being_used_with(Program* program)
{
    Object* object = nullptr;

    int sid = scr_find_sid_from_program(program);

    Script* script;
    if (scr_ptr(sid, &script) != -1) {
        object = script->target;
    } else {
        dbg_error(program, "obj_being_used_with", SCRIPT_ERROR_CANT_MATCH_PROGRAM_TO_SID);
    }

    program->stackPushPointer(object);
}

// 0x44C9BC
static void op_local_var(Program* program)
{
    int data = program->stackPopInteger();

    ProgramValue value;
    value.opcode = VALUE_TYPE_INT;
    value.integerValue = -1;

    int sid = scr_find_sid_from_program(program);
    scr_get_local_var(sid, data, value);

    program->stackPushValue(value);
}

// 0x44CA28
static void op_set_local_var(Program* program)
{
    ProgramValue value = program->stackPopValue();
    int variable = program->stackPopInteger();

    int sid = scr_find_sid_from_program(program);
    scr_set_local_var(sid, variable, value);
}

// 0x44CA9C
static void op_map_var(Program* program)
{
    int data = program->stackPopInteger();

    ProgramValue value;
    if (map_get_global_var(data, value) == -1) {
        value.opcode = VALUE_TYPE_INT;
        value.integerValue = -1;
    }

    program->stackPushValue(value);
}

// 0x44CAF0
static void op_set_map_var(Program* program)
{
    ProgramValue value = program->stackPopValue();
    int variable = program->stackPopInteger();

    map_set_global_var(variable, value);
}

// 0x44CB5C
static void op_global_var(Program* program)
{
    int data = program->stackPopInteger();

    int value = -1;
    if (num_game_global_vars != 0) {
        value = game_get_global_var(data);
    } else {
        int_debug("\nScript Error: %s: op_global_var: no global vars found!", program->name);
    }

    program->stackPushInteger(value);
}

// 0x44CBD8
static void op_set_global_var(Program* program)
{
    int value = program->stackPopInteger();
    int variable = program->stackPopInteger();

    if (num_game_global_vars != 0) {
        game_set_global_var(variable, value);
    } else {
        int_debug("\nScript Error: %s: op_set_global_var: no global vars found!", program->name);
    }
}

// 0x44CC5C
static void op_script_action(Program* program)
{
    int action = 0;

    int sid = scr_find_sid_from_program(program);

    Script* script;
    if (scr_ptr(sid, &script) != -1) {
        action = script->action;
    } else {
        dbg_error(program, "script_action", SCRIPT_ERROR_CANT_MATCH_PROGRAM_TO_SID);
    }

    program->stackPushInteger(action);
}

// 0x44CCB0
static void op_obj_type(Program* program)
{
    Object* object = static_cast<Object*>(program->stackPopPointer());

    int objectType = -1;
    if (object != nullptr) {
        objectType = FID_TYPE(object->fid);
    }

    program->stackPushInteger(objectType);
}

// 0x44CD14
static void op_obj_item_subtype(Program* program)
{
    Object* obj = static_cast<Object*>(program->stackPopPointer());

    int itemType = -1;
    if (obj != nullptr) {
        if (PID_TYPE(obj->pid) == OBJ_TYPE_ITEM) {
            Proto* proto;
            if (proto_ptr(obj->pid, &proto) != -1) {
                itemType = item_get_type(obj);
            }
        }
    }

    program->stackPushInteger(itemType);
}

// 0x44CD9C
static void op_get_critter_stat(Program* program)
{
    int stat = program->stackPopInteger();
    Object* object = static_cast<Object*>(program->stackPopPointer());

    int value = -1;
    if (object != nullptr) {
        value = stat_level(object, stat);
    } else {
        dbg_error(program, "get_critter_stat", SCRIPT_ERROR_OBJECT_IS_NULL);
    }

    program->stackPushInteger(value);
}

// NOTE: Despite it's name it does not actually "set" stat, but "adjust". So
// it's last argument is amount of adjustment, not it's final value.
//
// 0x44CE3C
static void op_set_critter_stat(Program* program)
{
    int value = program->stackPopInteger();
    int stat = program->stackPopInteger();
    Object* object = static_cast<Object*>(program->stackPopPointer());

    int result = 0;
    if (object != nullptr) {
        if (object == obj_dude) {
            int currentValue = stat_get_base(object, stat);
            stat_set_base(object, stat, currentValue + value);
        } else {
            dbg_error(program, "set_critter_stat", SCRIPT_ERROR_FOLLOWS);
            debug_printf(" Can't modify anyone except obj_dude!");
            result = -1;
        }
    } else {
        dbg_error(program, "set_critter_stat", SCRIPT_ERROR_OBJECT_IS_NULL);
        result = -1;
    }

    program->stackPushInteger(result);
}

// 0x44CF1C
static void op_animate_stand_obj(Program* program)
{
    Object* object = static_cast<Object*>(program->stackPopPointer());
    if (object == nullptr) {
        int sid = scr_find_sid_from_program(program);

        Script* script;
        if (scr_ptr(sid, &script) == -1) {
            dbg_error(program, "animate_stand_obj", SCRIPT_ERROR_CANT_MATCH_PROGRAM_TO_SID);
            return;
        }

        object = scr_find_obj_from_program(program);
    }

    if (!isInCombat()) {
        register_begin(ANIMATION_REQUEST_UNRESERVED);
        register_object_animate(object, ANIM_STAND, 0);
        register_end();
    }
}

// 0x44CFB4
static void op_animate_stand_reverse_obj(Program* program)
{
    Object* object = static_cast<Object*>(program->stackPopPointer());
    if (object == nullptr) {
        int sid = scr_find_sid_from_program(program);

        Script* script;
        if (scr_ptr(sid, &script) == -1) {
            dbg_error(program, "animate_stand_reverse_obj", SCRIPT_ERROR_CANT_MATCH_PROGRAM_TO_SID);
            return;
        }

        object = scr_find_obj_from_program(program);
    }

    if (!isInCombat()) {
        register_begin(ANIMATION_REQUEST_UNRESERVED);
        register_object_animate_reverse(object, ANIM_STAND, 0);
        register_end();
    }
}

// 0x44D04C
static void op_animate_move_obj_to_tile(Program* program)
{
    int flags = program->stackPopInteger();
    ProgramValue tileValue = program->stackPopValue();
    Object* object = static_cast<Object*>(program->stackPopPointer());

    if (object == nullptr) {
        dbg_error(program, "animate_move_obj_to_tile", SCRIPT_ERROR_OBJECT_IS_NULL);
        return;
    }

    // CE: There is a bug in `sinthia` script. It's supposed that Sinthia moves
    // to `dest_tile`, but this function is passed `self_obj` as tile.
    int tile;
    if (tileValue.opcode == VALUE_TYPE_INT) {
        tile = tileValue.integerValue;
    } else {
        dbg_error(program, "animate_move_obj_to_tile", SCRIPT_ERROR_FOLLOWS);
        debug_printf("Invalid tile type.");
        tile = -1;
    }

    if (tile <= -1) {
        return;
    }

    int sid = scr_find_sid_from_program(program);

    Script* script;
    if (scr_ptr(sid, &script) == -1) {
        dbg_error(program, "animate_move_obj_to_tile", SCRIPT_ERROR_CANT_MATCH_PROGRAM_TO_SID);
        return;
    }

    if (!critter_is_active(object)) {
        return;
    }

    if (isInCombat()) {
        return;
    }

    if ((flags & 0x10) != 0) {
        register_clear(object);
        flags &= ~0x10;
    }

    register_begin(ANIMATION_REQUEST_UNRESERVED);

    if (flags == 0) {
        register_object_move_to_tile(object, tile, object->elevation, -1, 0);
    } else {
        register_object_run_to_tile(object, tile, object->elevation, -1, 0);
    }

    register_end();
}

// 0x44D17
static void op_animate_jump(Program* program)
{
    int_debug("\nScript Error: %s: op_animate_jump: INVALID ACTION!");
}

// 0x44D18C
static void op_make_daytime(Program* program)
{
}

// 0x44D190
static void op_tile_distance(Program* program)
{
    int tile2 = program->stackPopInteger();
    int tile1 = program->stackPopInteger();

    int distance;

    if (tile1 != -1 && tile2 != -1) {
        distance = tile_dist(tile1, tile2);
    } else {
        distance = 9999;
    }

    program->stackPushInteger(distance);
}

// 0x44D224
static void op_tile_distance_objs(Program* program)
{
    Object* object2 = static_cast<Object*>(program->stackPopPointer());
    Object* object1 = static_cast<Object*>(program->stackPopPointer());

    int distance = 9999;
    if (object1 != nullptr && object2 != nullptr) {
        if (reinterpret_cast<uintptr_t>(object2) >= HEX_GRID_SIZE && reinterpret_cast<uintptr_t>(object1) >= HEX_GRID_SIZE) {
            if (object1->elevation == object2->elevation) {
                if (object1->tile != -1 && object2->tile != -1) {
                    distance = tile_dist(object1->tile, object2->tile);
                }
            }
        } else {
            dbg_error(program, "tile_distance_objs", SCRIPT_ERROR_FOLLOWS);
            debug_printf(" Passed a tile # instead of an object!!!BADBADBAD!");
        }
    }

    program->stackPushInteger(distance);
}

// 0x44D304
static void op_tile_num(Program* program)
{
    Object* obj = static_cast<Object*>(program->stackPopPointer());

    int tile = -1;
    if (obj != nullptr) {
        tile = obj->tile;
    } else {
        dbg_error(program, "tile_num", SCRIPT_ERROR_OBJECT_IS_NULL);
    }

    program->stackPushInteger(tile);
}

// 0x44D374
static void op_tile_num_in_direction(Program* program)
{
    int distance = program->stackPopInteger();
    int rotation = program->stackPopInteger();
    int origin = program->stackPopInteger();

    int tile = -1;

    if (origin != -1) {
        if (rotation < ROTATION_COUNT) {
            if (distance != 0) {
                tile = tile_num_in_direction(origin, rotation, distance);
                if (tile < -1) {
                    debug_printf("\nError: %s: op_tile_num_in_direction got #: %d", program->name, tile);
                    tile = -1;
                }
            }
        } else {
            dbg_error(program, "tile_num_in_direction", SCRIPT_ERROR_FOLLOWS);
            debug_printf(" rotation out of Range!");
        }
    } else {
        dbg_error(program, "tile_num_in_direction", SCRIPT_ERROR_FOLLOWS);
        debug_printf(" tileNum is -1!");
    }

    program->stackPushInteger(tile);
}

// 0x44D448
static void op_pickup_obj(Program* program)
{
    Object* object = static_cast<Object*>(program->stackPopPointer());

    if (object == nullptr) {
        return;
    }

    int sid = scr_find_sid_from_program(program);

    Script* script;
    if (scr_ptr(sid, &script) == 1) {
        dbg_error(program, "pickup_obj", SCRIPT_ERROR_CANT_MATCH_PROGRAM_TO_SID);
        return;
    }

    if (script->target == nullptr) {
        dbg_error(program, "pickup_obj", SCRIPT_ERROR_OBJECT_IS_NULL);
        return;
    }

    action_get_an_object(script->target, object);
}

// 0x44D4D8
static void op_drop_obj(Program* program)
{
    Object* object = static_cast<Object*>(program->stackPopPointer());

    if (object == nullptr) {
        return;
    }

    int sid = scr_find_sid_from_program(program);

    Script* script;
    if (scr_ptr(sid, &script) == -1) {
        // FIXME: Should be SCRIPT_ERROR_CANT_MATCH_PROGRAM_TO_SID.
        dbg_error(program, "drop_obj", SCRIPT_ERROR_OBJECT_IS_NULL);
        return;
    }

    if (script->target == nullptr) {
        // FIXME: Should be SCRIPT_ERROR_OBJECT_IS_NULL.
        dbg_error(program, "drop_obj", SCRIPT_ERROR_CANT_MATCH_PROGRAM_TO_SID);
        return;
    }

    obj_drop(script->target, object);
}

// 0x44D568
static void op_add_obj_to_inven(Program* program)
{
    Object* item = static_cast<Object*>(program->stackPopPointer());
    Object* owner = static_cast<Object*>(program->stackPopPointer());

    if (owner == nullptr || item == nullptr) {
        return;
    }

    if (item->owner == nullptr) {
        if (item_add_force(owner, item, 1) == 0) {
            Rect rect;
            obj_disconnect(item, &rect);
            tile_refresh_rect(&rect, item->elevation);
        }
    } else {
        dbg_error(program, "add_obj_to_inven", SCRIPT_ERROR_FOLLOWS);
        debug_printf(" Item was already attached to something else!");
    }
}

// 0x44D624
static void op_rm_obj_from_inven(Program* program)
{
    Object* item = static_cast<Object*>(program->stackPopPointer());
    Object* owner = static_cast<Object*>(program->stackPopPointer());

    if (owner == nullptr || item == nullptr) {
        return;
    }

    bool updateFlags = false;
    int flags = 0;

    if ((item->flags & OBJECT_EQUIPPED) != 0) {
        if ((item->flags & OBJECT_IN_LEFT_HAND) != 0) {
            flags |= OBJECT_IN_LEFT_HAND;
        }

        if ((item->flags & OBJECT_IN_RIGHT_HAND) != 0) {
            flags |= OBJECT_IN_RIGHT_HAND;
        }

        if ((item->flags & OBJECT_WORN) != 0) {
            flags |= OBJECT_WORN;
        }

        updateFlags = true;
    }

    if (item_remove_mult(owner, item, 1) == 0) {
        Rect rect;
        obj_connect(item, 1, 0, &rect);
        tile_refresh_rect(&rect, item->elevation);

        if (updateFlags) {
            correctFidForRemovedItem(owner, item, flags);
        }
    }
}

// 0x44D718
static void op_wield_obj_critter(Program* program)
{
    Object* item = static_cast<Object*>(program->stackPopPointer());
    Object* critter = static_cast<Object*>(program->stackPopPointer());

    if (critter == nullptr) {
        dbg_error(program, "wield_obj_critter", SCRIPT_ERROR_OBJECT_IS_NULL);
        return;
    }

    if (item == nullptr) {
        dbg_error(program, "wield_obj_critter", SCRIPT_ERROR_OBJECT_IS_NULL);
        return;
    }

    if (PID_TYPE(critter->pid) != OBJ_TYPE_CRITTER) {
        dbg_error(program, "wield_obj_critter", SCRIPT_ERROR_FOLLOWS);
        debug_printf(" Only works for critters!  ERROR ERROR ERROR!");
        return;
    }

    int hand = HAND_RIGHT;

    bool shouldAdjustArmorClass = false;
    Object* oldArmor = nullptr;
    Object* newArmor = nullptr;
    if (critter == obj_dude) {
        if (intface_is_item_right_hand() == HAND_LEFT) {
            hand = HAND_LEFT;
        }

        if (item_get_type(item) == ITEM_TYPE_ARMOR) {
            oldArmor = inven_worn(obj_dude);
            shouldAdjustArmorClass = true;
            newArmor = item;
        }
    }

    inven_wield(critter, item, hand);

    if (critter == obj_dude) {
        if (shouldAdjustArmorClass) {
            adjust_ac(critter, oldArmor, newArmor);
        }
    }
}

// 0x44D870
static void op_use_obj(Program* program)
{
    Object* object = static_cast<Object*>(program->stackPopPointer());

    if (object == nullptr) {
        dbg_error(program, "use_obj", SCRIPT_ERROR_OBJECT_IS_NULL);
        return;
    }

    int sid = scr_find_sid_from_program(program);

    Script* script;
    if (scr_ptr(sid, &script) == -1) {
        // FIXME: Should be SCRIPT_ERROR_CANT_MATCH_PROGRAM_TO_SID.
        dbg_error(program, "use_obj", SCRIPT_ERROR_OBJECT_IS_NULL);
        return;
    }

    if (script->target == nullptr) {
        dbg_error(program, "use_obj", SCRIPT_ERROR_OBJECT_IS_NULL);
        return;
    }

    Object* self = scr_find_obj_from_program(program);
    if (PID_TYPE(self->pid) == OBJ_TYPE_CRITTER) {
        action_use_an_object(script->target, object);
    } else {
        obj_use(self, object);
    }
}

// 0x44D944
static void op_obj_can_see_obj(Program* program)
{
    Object* object2 = static_cast<Object*>(program->stackPopPointer());
    Object* object1 = static_cast<Object*>(program->stackPopPointer());

    int result = 0;

    if (object1 != nullptr && object2 != nullptr) {
        if (object2->tile != -1) {
            // NOTE: Looks like dead code, I guess these checks were incorporated
            // into higher level functions, but this code left intact.
            if (object2 == obj_dude) {
                is_pc_flag(0);
            }

            stat_level(object1, STAT_PERCEPTION);

            if (is_within_perception(object1, object2)) {
                Object* a5;
                make_straight_path(object1, object1->tile, object2->tile, nullptr, &a5, 16);
                if (a5 == object2) {
                    result = 1;
                }
            }
        }
    } else {
        dbg_error(program, "obj_can_see_obj", SCRIPT_ERROR_OBJECT_IS_NULL);
    }

    program->stackPushInteger(result);
}

// 0x44DA5
static int dbg_print_com_data(Object* attacker, Object* defender)
{
    debug_printf("\nScripts [Combat]: %s(Team %d) requests attack on %s(Team %d)",
        object_name(attacker),
        attacker->data.critter.combat.team,
        object_name(defender),
        defender->data.critter.combat.team);
    return 0;
}

// 0x44DA94
static void op_attack(Program* program)
{
    int data[8];

    for (int arg = 0; arg < 7; arg++) {
        data[arg] = program->stackPopInteger();
    }

    Object* target = static_cast<Object*>(program->stackPopPointer());
    if (target == nullptr) {
        dbg_error(program, "attack", SCRIPT_ERROR_OBJECT_IS_NULL);
        return;
    }

    program->flags |= PROGRAM_FLAG_0x20;

    Object* self = scr_find_obj_from_program(program);
    if (self == nullptr) {
        program->flags &= ~PROGRAM_FLAG_0x20;
        return;
    }

    if (!critter_is_active(self)) {
        dbg_print_com_data(self, target);
        debug_printf("\n   But is already Inactive (Dead/Stunned)");
        program->flags &= ~PROGRAM_FLAG_0x20;
        return;
    }

    if (!critter_is_active(target)) {
        dbg_print_com_data(self, target);
        debug_printf("\n   But target is already dead");
        program->flags &= ~PROGRAM_FLAG_0x20;
        return;
    }

    if ((target->data.critter.combat.maneuver & CRITTER_MANUEVER_FLEEING) != 0) {
        dbg_print_com_data(self, target);
        debug_printf("\n   But target is AFRAID");
        program->flags &= ~PROGRAM_FLAG_0x20;
        return;
    }

    if (dialog_active()) {
        // TODO: Might be an error, program flag is not removed.
        return;
    }

    if (isInCombat()) {
        CritterCombatData* combatData = &(self->data.critter.combat);
        if ((combatData->maneuver & CRITTER_MANEUVER_ENGAGING) == 0) {
            combatData->maneuver |= CRITTER_MANEUVER_ENGAGING;
            combatData->whoHitMe = target;
        }
    } else {
        CombatSequenceParams attack;
        attack.attacker = self;
        attack.defender = target;
        attack.actionPointsBonus = 0;
        attack.accuracyBonus = data[4];
        attack.damageBonus = 0;
        attack.minDamage = data[3];
        attack.maxDamage = data[2];

        // TODO: Something is probably broken here, why it wants
        // flags to be the same? Maybe because both of them
        // are applied to defender because of the bug in 0x422F3C?
        if (data[1] == data[0]) {
            attack.hasOverrideFlags = 1;
            attack.defenderOverrideFlags = data[0];
            attack.attackerOverrideFlags = data[1];
        } else {
            attack.hasOverrideFlags = 0;
        }

        dbg_print_com_data(self, target);
        attack.scripts_request_combat();
    }

    program->flags &= ~PROGRAM_FLAG_0x20;
}

// 0x44DC84
static void op_start_gdialog(Program* program)
{
    int backgroundId = program->stackPopInteger();
    int headId = program->stackPopInteger();
    int reactionLevel = program->stackPopInteger();
    Object* obj = static_cast<Object*>(program->stackPopPointer());
    program->stackPopInteger();

    if (isInCombat()) {
        return;
    }

    if (obj == nullptr) {
        dbg_error(program, "start_gdialog", SCRIPT_ERROR_OBJECT_IS_NULL);
        return;
    }

    dialogue_head = -1;
    if (PID_TYPE(obj->pid) == OBJ_TYPE_CRITTER) {
        Proto* proto;
        if (proto_ptr(obj->pid, &proto) == -1) {
            return;
        }
    }

    if (headId != -1) {
        dialogue_head = art_id(OBJ_TYPE_HEAD, headId, 0, 0, 0);
    }

    gdialog_set_background(backgroundId);
    dialogue_mood = reactionLevel;

    if (dialogue_head != -1) {
        int npcReactionValue = reaction_get(dialog_target);
        NpcReaction npcReactionType = reaction_to_level(npcReactionValue);
        switch (npcReactionType) {
        case NpcReaction::NPC_REACTION_BAD:
            dialogue_mood = FIDGET_BAD;
            break;
        case NpcReaction::NPC_REACTION_NEUTRAL:
            dialogue_mood = FIDGET_NEUTRAL;
            break;
        case NpcReaction::NPC_REACTION_GOOD:
            dialogue_mood = FIDGET_GOOD;
            break;
        }
    }

    dialogue_scr_id = scr_find_sid_from_program(program);
    dialog_target = scr_find_obj_from_program(program);
    scr_dialogue_init(dialogue_head, dialogue_mood);
}

// 0x44DE08
static void op_end_dialogue(Program* program)
{
    if (scr_dialogue_exit() != -1) {
        dialog_target = nullptr;
        dialogue_scr_id = -1;
    }
}

// 0x44DE2C
static void op_dialogue_reaction(Program* program)
{
    int value = program->stackPopInteger();

    dialogue_mood = value;
    talk_to_critter_reacts(value);
}

// 0x44DE74
static void objs_area_turn_on_off(int a1, int a2, int a3, int a4, int enabled)
{
    // 0x44B560
    static Rect rect;

    int temp;
    Object* object;
    Rect object_bounds;

    if (a1 > a2) {
        temp = a1;
        a1 = a2;
        a2 = temp;
    }

    if (a3 > a4) {
        temp = a3;
        a3 = a4;
        a4 = a3;
    }

    while (a1 <= a2) {
        object = obj_find_first_at(a1);
        while (object != nullptr) {
            if ((object->flags & OBJECT_HIDDEN) == enabled) {
                if (object->tile >= a3 && object->tile <= a4 && (object->tile - a3) / 200 <= a4 / 200 - a3 / 200) {
                    obj_bound(object, &object_bounds);
                    if (enabled) {
                        object->flags &= ~OBJECT_HIDDEN;
                    } else {
                        object->flags |= OBJECT_HIDDEN;
                    }
                    rect.minBound(object_bounds);
                }
            }
            object = obj_find_next_at();
        }
        tile_refresh_rect(&rect, a1);
    }
}

// 0x44DF7C
static void op_turn_off_objs_in_area(Program* program)
{
    int data[4];

    for (int arg = 0; arg < 4; arg++) {
        data[arg] = program->stackPopInteger();
    }

    objs_area_turn_on_off(data[3], data[2], data[1], data[0], 0);
}

// 0x44DFF0
static void op_turn_on_objs_in_area(Program* program)
{
    int data[4];

    for (int arg = 0; arg < 4; arg++) {
        data[arg] = program->stackPopInteger();
    }

    objs_area_turn_on_off(data[3], data[2], data[1], data[0], 1);
}

// NOTE: Function name is a bit misleading. Last parameter is a boolean value
// where 1 or true makes object invisible, and value 0 (false) makes it visible
// again. So a better name for this function is opSetObjectInvisible.
//
//
// 0x44E064
static void op_set_obj_visibility(Program* program)
{
    int invisible = program->stackPopInteger();
    Object* obj = static_cast<Object*>(program->stackPopPointer());

    if (obj == nullptr) {
        dbg_error(program, "set_obj_visibility", SCRIPT_ERROR_OBJECT_IS_NULL);
        return;
    }

    if (isLoadingGame()) {
        debug_printf("Error: attempt to set_obj_visibility in load/save-game: %s!", program->name);
        return;
    }

    if (invisible != 0) {
        if ((obj->flags & OBJECT_HIDDEN) == 0) {
            Rect rect;
            obj_bound(obj, &rect);

            obj->flags |= OBJECT_HIDDEN;
            if (PID_TYPE(obj->pid) == OBJ_TYPE_CRITTER) {
                obj->flags |= OBJECT_NO_BLOCK;
            }

            tile_refresh_rect(&rect, obj->elevation);
        }
    } else {
        if ((obj->flags & OBJECT_HIDDEN) != 0) {
            if (PID_TYPE(obj->pid) == OBJ_TYPE_CRITTER) {
                obj->flags &= ~OBJECT_NO_BLOCK;
            }

            obj->flags &= ~OBJECT_HIDDEN;

            Rect rect;
            obj_bound(obj, &rect);
            tile_refresh_rect(&rect, obj->elevation);
        }
    }
}

// 0x44E178
static void op_load_map(Program* program)
{
    int param = program->stackPopInteger();
    ProgramValue mapIndexOrName = program->stackPopValue();

    char* mapName = nullptr;

    if ((mapIndexOrName.opcode & VALUE_TYPE_MASK) != VALUE_TYPE_INT) {
        if ((mapIndexOrName.opcode & VALUE_TYPE_MASK) == VALUE_TYPE_STRING) {
            mapName = program->getString(mapIndexOrName.opcode, mapIndexOrName.integerValue);
        } else {
            interpretError("script error: %s: invalid arg 1 to load_map", program->name);
        }
    }

    int mapIndex = -1;

    if (mapName != nullptr) {
        game_global_vars[GVAR_LOAD_MAP_INDEX] = param;
        mapIndex = map_match_map_name(mapName);
    } else {
        if (mapIndexOrName.integerValue >= 0) {
            game_global_vars[GVAR_LOAD_MAP_INDEX] = param;
            mapIndex = mapIndexOrName.integerValue;
        }
    }

    if (mapIndex != -1) {
        MapTransition transition;
        transition.map = mapIndex;
        transition.elevation = -1;
        transition.tile = -1;
        transition.rotation = -1;
        map_leave_map(&transition);
    }
}

// 0x44E274
static void op_barter_offer(Program* program)
{
    int data[3];

    for (int arg = 0; arg < 3; arg++) {
        data[arg] = program->stackPopInteger();
    }
}

// 0x44E2D4
static void op_barter_asking(Program* program)
{
    int data[3];

    for (int arg = 0; arg < 3; arg++) {
        data[arg] = program->stackPopInteger();
    }
}

// 0x44E334
static void op_anim_busy(Program* program)
{
    Object* object = static_cast<Object*>(program->stackPopPointer());

    int rc = 0;
    if (object != nullptr) {
        rc = anim_busy(object);
    } else {
        dbg_error(program, "anim_busy", SCRIPT_ERROR_OBJECT_IS_NULL);
    }

    program->stackPushInteger(rc);
}

// 0x44E3A8
static void op_critter_heal(Program* program)
{
    int amount = program->stackPopInteger();
    Object* critter = static_cast<Object*>(program->stackPopPointer());

    int rc = critter_adjust_hits(critter, amount);

    if (critter == obj_dude) {
        intface_update_hit_points(true);
    }

    program->stackPushInteger(rc);
}

// 0x44E440
static void op_set_light_level(Program* program)
{
    // Maps light level to light intensity.
    //
    // Middle value is mapped one-to-one which corresponds to 50% light level
    // (cavern lighting). Light levels above (51-100%) and below (0-49) is
    // calculated as percentage from two adjacent light values.
    //
    // 0x44B570
    static const int dword_453F90[3] = {
        0x4000,
        0xA000,
        0x10000,
    };

    int data = program->stackPopInteger();

    int lightLevel = data;

    if (data == 50) {
        light_set_ambient(dword_453F90[1], true);
        return;
    }

    int lightIntensity;
    if (data > 50) {
        lightIntensity = dword_453F90[1] + data * (dword_453F90[2] - dword_453F90[1]) / 100;
    } else {
        lightIntensity = dword_453F90[0] + data * (dword_453F90[1] - dword_453F90[0]) / 100;
    }

    light_set_ambient(lightIntensity, true);
}

// 0x44E4E8
static void op_game_time(Program* program)
{
    int time = game_time();
    program->stackPushInteger(time);
}

// 0x44E50C
static void op_game_time_in_seconds(Program* program)
{
    int time = game_time();
    program->stackPushInteger(time / 10);
}

// 0x44E538
static void op_elevation(Program* program)
{
    Object* object = static_cast<Object*>(program->stackPopPointer());

    int elevation = 0;
    if (object != nullptr) {
        elevation = object->elevation;
    } else {
        dbg_error(program, "elevation", SCRIPT_ERROR_OBJECT_IS_NULL);
    }

    program->stackPushInteger(elevation);
}

// 0x44E5A4
static void op_kill_critter(Program* program)
{
    int deathFrame = program->stackPopInteger();
    Object* object = static_cast<Object*>(program->stackPopPointer());

    if (object == nullptr) {
        dbg_error(program, "kill_critter", SCRIPT_ERROR_OBJECT_IS_NULL);
        return;
    }

    if (isLoadingGame()) {
        debug_printf("\nError: attempt to destroy critter in load/save-game: %s!", program->name);
    }

    program->flags |= PROGRAM_FLAG_0x20;

    Object* self = scr_find_obj_from_program(program);
    bool isSelf = self == object;

    register_clear(object);
    combat_delete_critter(object);
    critter_kill(object, deathFrame, 1);

    program->flags &= ~PROGRAM_FLAG_0x20;

    if (isSelf) {
        program->flags |= PROGRAM_FLAG_0x0100;
    }
}

// [forceBack] is to force fall back animation, otherwise it's fall front if it's present
//
// 0x0x44E690
int correctDeath(Object* critter, int anim, bool forceBack)
{
    if (anim >= ANIM_BIG_HOLE_SF && anim <= ANIM_FALL_FRONT_BLOOD_SF) {
        int violenceLevel = VIOLENCE_LEVEL_MAXIMUM_BLOOD;
        game_config.getValue(GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_VIOLENCE_LEVEL_KEY, &violenceLevel);

        bool useStandardDeath = false;
        if (violenceLevel < VIOLENCE_LEVEL_MAXIMUM_BLOOD) {
            useStandardDeath = true;
        } else {
            int fid = art_id(OBJ_TYPE_CRITTER, critter->fid & 0xFFF, anim, (critter->fid & 0xF000) >> 12, critter->rotation + 1);
            if (!art_exists(fid)) {
                useStandardDeath = true;
            }
        }

        if (useStandardDeath) {
            if (forceBack) {
                anim = ANIM_FALL_BACK;
            } else {
                int fid = art_id(OBJ_TYPE_CRITTER, critter->fid & 0xFFF, ANIM_FALL_FRONT, (critter->fid & 0xF000) >> 12, critter->rotation + 1);
                if (art_exists(fid)) {
                    anim = ANIM_FALL_FRONT;
                } else {
                    anim = ANIM_FALL_BACK;
                }
            }
        }
    }

    return anim;
}

// 0x44E750
static void op_kill_critter_type(Program* program)
{
    // 0x518ED0
    static int ftList[11] = {
        ANIM_FALL_BACK_BLOOD_SF,
        ANIM_BIG_HOLE_SF,
        ANIM_CHARRED_BODY_SF,
        ANIM_CHUNKS_OF_FLESH_SF,
        ANIM_FALL_FRONT_BLOOD_SF,
        ANIM_FALL_BACK_BLOOD_SF,
        ANIM_DANCING_AUTOFIRE_SF,
        ANIM_SLICED_IN_HALF_SF,
        ANIM_EXPLODED_TO_NOTHING_SF,
        ANIM_FALL_BACK_BLOOD_SF,
        ANIM_FALL_FRONT_BLOOD_SF,
    };

    int deathFrame = program->stackPopInteger();
    int pid = program->stackPopInteger();

    if (isLoadingGame()) {
        debug_printf("\nError: attempt to destroy critter in load/save-game: %s!", program->name);
        return;
    }

    program->flags |= PROGRAM_FLAG_0x20;

    Object* previousObj = nullptr;
    int count = 0;
    int v3 = 0;

    Object* obj = obj_find_first();
    while (obj != nullptr) {
        if (FID_ANIM_TYPE(obj->fid) >= ANIM_FALL_BACK_SF) {
            obj = obj_find_next();
            continue;
        }

        if ((obj->flags & OBJECT_HIDDEN) == 0 && obj->pid == pid && !critter_is_dead(obj)) {
            if (obj == previousObj || count > 200) {
                dbg_error(program, "kill_critter_type", SCRIPT_ERROR_FOLLOWS);
                debug_printf(" Infinite loop destroying critters!");
                program->flags &= ~PROGRAM_FLAG_0x20;
                return;
            }

            register_clear(obj);

            if (deathFrame != 0) {
                combat_delete_critter(obj);
                if (deathFrame == 1) {
                    int anim = correctDeath(obj, ftList[v3], 1);
                    critter_kill(obj, anim, 1);
                    v3 += 1;
                    if (v3 >= 11) {
                        v3 = 0;
                    }
                } else {
                    critter_kill(obj, ANIM_FALL_BACK_SF, 1);
                }
            } else {
                register_clear(obj);

                Rect rect;
                obj_erase_object(obj, &rect);
                tile_refresh_rect(&rect, map_elevation);
            }

            previousObj = obj;
            count += 1;

            obj_find_first();

            map_data.lastVisitTime = game_time();
        }

        obj = obj_find_next();
    }

    program->flags &= ~PROGRAM_FLAG_0x20;
}

// 0x44E918
static void op_critter_damage(Program* program)
{
    program->flags |= PROGRAM_FLAG_0x20;

    int damageTypeWithFlags = program->stackPopInteger();
    int amount = program->stackPopInteger();
    Object* object = static_cast<Object*>(program->stackPopPointer());

    if (object == nullptr) {
        dbg_error(program, "critter_damage", SCRIPT_ERROR_OBJECT_IS_NULL);
        return;
    }

    if (PID_TYPE(object->pid) != OBJ_TYPE_CRITTER) {
        dbg_error(program, "critter_damage", SCRIPT_ERROR_FOLLOWS);
        debug_printf(" Can't call on non-critters!");
        return;
    }

    Object* self = scr_find_obj_from_program(program);
    if (object->data.critter.combat.whoHitMeCid == -1) {
        object->data.critter.combat.whoHitMe = nullptr;
    }

    bool animate = (damageTypeWithFlags & 0x200) == 0;
    bool bypassArmor = (damageTypeWithFlags & 0x100) != 0;
    int damageType = damageTypeWithFlags & ~(0x100 | 0x200);
    action_dmg(object->tile, object->elevation, amount, amount, damageType, animate, bypassArmor);

    program->flags &= ~PROGRAM_FLAG_0x20;

    if (self == object) {
        program->flags |= PROGRAM_FLAG_0x0100;
    }
}

// 0x44EA38
static void op_add_timer_event(Program* program)
{
    int param = program->stackPopInteger();
    int delay = program->stackPopInteger();
    Object* object = static_cast<Object*>(program->stackPopPointer());

    if (object == nullptr) {
        int_debug("\nScript Error: %s: op_add_timer_event: pobj is nullptr!", program->name);
        return;
    }

    script_q_add(object->sid, delay, param);
}

// 0x44EAC0
static void op_rm_timer_event(Program* program)
{
    Object* object = static_cast<Object*>(program->stackPopPointer());

    if (object == nullptr) {
        // FIXME: Should be op_rm_timer_event.
        int_debug("\nScript Error: %s: op_add_timer_event: pobj is nullptr!");
        return;
    }

    queue_remove(object);
}

// Converts seconds into game ticks.
//
// 0x44EB1C
static void op_game_ticks(Program* program)
{
    int ticks = program->stackPopInteger();

    if (ticks < 0) {
        ticks = 0;
    }

    program->stackPushInteger(ticks * 10);
}

// NOTE: The name of this function is misleading. It has (almost) nothing to do
// with player's "Traits" as a feature. Instead it's used to query many
// information of the critters using passed parameters. It's like "metarule" but
// for critters.
//
// 0x44EB78
static void op_has_trait(Program* program)
{
    int param = program->stackPopInteger();
    Object* object = static_cast<Object*>(program->stackPopPointer());
    int type = program->stackPopInteger();

    int result = 0;

    if (object != nullptr) {
        switch (type) {
        case CRITTER_TRAIT_PERK:
            if (param < PERK_COUNT) {
                result = perk_level(param);
            } else {
                int_debug("\nScript Error: %s: op_has_trait: Perk out of range", program->name);
            }
            break;
        case CRITTER_TRAIT_OBJECT:
            switch (param) {
            case CRITTER_TRAIT_OBJECT_AI_PACKET:
                if (PID_TYPE(object->pid) == OBJ_TYPE_CRITTER) {
                    result = object->data.critter.combat.aiPacket;
                }
                break;
            case CRITTER_TRAIT_OBJECT_TEAM:
                if (PID_TYPE(object->pid) == OBJ_TYPE_CRITTER) {
                    result = object->data.critter.combat.team;
                }
                break;
            case CRITTER_TRAIT_OBJECT_ROTATION:
                result = object->rotation;
                break;
            case CRITTER_TRAIT_OBJECT_IS_INVISIBLE:
                result = (object->flags & OBJECT_HIDDEN) == 0;
                break;
            case CRITTER_TRAIT_OBJECT_GET_INVENTORY_WEIGHT:
                result = item_total_weight(object);
                break;
            }
            break;
        case CRITTER_TRAIT_TRAIT:
            if (param < TRAIT_COUNT) {
                result = trait_level(param);
            } else {
                int_debug("\nScript Error: %s: op_has_trait: Trait out of range", program->name);
            }
            break;
        default:
            int_debug("\nScript Error: %s: op_has_trait: Trait out of range", program->name);
            break;
        }
    } else {
        dbg_error(program, "has_trait", SCRIPT_ERROR_OBJECT_IS_NULL);
    }

    program->stackPushInteger(result);
}

// 0x44EC90
static void op_obj_can_hear_obj(Program* program)
{
    Object* object2 = static_cast<Object*>(program->stackPopPointer());
    Object* object1 = static_cast<Object*>(program->stackPopPointer());

    bool canHear = false;

    // FIXME: This is clearly an error. If any of the object is nullptr
    // dereferencing will crash the game.
    if (object2 == nullptr || object1 == nullptr) {
        if (object2->elevation == object1->elevation) {
            if (object2->tile != -1 && object1->tile != -1) {
                if (is_within_perception(object1, object2)) {
                    canHear = true;
                }
            }
        }
    }

    program->stackPushInteger(canHear);
}

// 0x44ED40
static void op_game_time_hour(Program* program)
{
    int value = game_time_hour();
    program->stackPushInteger(value);
}

// 0x44ED64
static void op_fixed_param(Program* program)
{
    int fixedParam = 0;

    int sid = scr_find_sid_from_program(program);

    Script* script;
    if (scr_ptr(sid, &script) != -1) {
        fixedParam = script->fixedParam;
    } else {
        dbg_error(program, "fixed_param", SCRIPT_ERROR_CANT_MATCH_PROGRAM_TO_SID);
    }

    program->stackPushInteger(fixedParam);
}

// 0x44EDB8
static void op_tile_is_visible(Program* program)
{
    int data = program->stackPopInteger();

    int isVisible = 0;
    if (scripts_tile_is_visible(data)) {
        isVisible = 1;
    }

    program->stackPushInteger(isVisible);
}

// 0x44EE18
static void op_dialogue_system_enter(Program* program)
{
    int sid = scr_find_sid_from_program(program);

    Script* script;
    if (scr_ptr(sid, &script) == -1) {
        return;
    }

    Object* self = scr_find_obj_from_program(program);
    if (PID_TYPE(self->pid) == OBJ_TYPE_CRITTER) {
        if (!critter_is_active(self)) {
            return;
        }
    }

    if (isInCombat()) {
        return;
    }

    if (game_state_request(GAME_STATE_4) == -1) {
        return;
    }

    dialog_target = scr_find_obj_from_program(program);
}

// 0x44EE78
static void op_action_being_used(Program* program)
{
    int action = -1;

    int sid = scr_find_sid_from_program(program);

    Script* script;
    if (scr_ptr(sid, &script) != -1) {
        action = script->actionBeingUsed;
    } else {
        dbg_error(program, "action_being_used", SCRIPT_ERROR_CANT_MATCH_PROGRAM_TO_SID);
    }

    program->stackPushInteger(action);
}

// 0x44EECC
static void op_critter_state(Program* program)
{
    Object* critter = static_cast<Object*>(program->stackPopPointer());

    int state = CRITTER_STATE_DEAD;
    if (critter != nullptr && PID_TYPE(critter->pid) == OBJ_TYPE_CRITTER) {
        if (critter_is_active(critter)) {
            state = CRITTER_STATE_NORMAL;

            int anim = FID_ANIM_TYPE(critter->fid);
            if (anim >= ANIM_FALL_BACK_SF && anim <= ANIM_FALL_FRONT_SF) {
                state = CRITTER_STATE_PRONE;
            }

            state |= (critter->data.critter.combat.results & DAM_CRIP);
        }
    } else {
        dbg_error(program, "critter_state", SCRIPT_ERROR_OBJECT_IS_NULL);
    }

    program->stackPushInteger(state);
}

// 0x44EF6C
static void op_game_time_advance(Program* program)
{
    int data = program->stackPopInteger();

    int days = data / GAME_TIME_TICKS_PER_DAY;
    int remainder = data % GAME_TIME_TICKS_PER_DAY;

    for (int day = 0; day < days; day++) {
        inc_game_time(GAME_TIME_TICKS_PER_DAY);
        queue_process();
    }

    inc_game_time(remainder);
    queue_process();
}

// 0x44EFE8
static void op_radiation_inc(Program* program)
{
    int amount = program->stackPopInteger();
    Object* object = static_cast<Object*>(program->stackPopPointer());

    if (object == nullptr) {
        dbg_error(program, "radiation_inc", SCRIPT_ERROR_OBJECT_IS_NULL);
        return;
    }

    critter_adjust_rads(object, amount);
}

// 0x44F06C
static void op_radiation_dec(Program* program)
{
    int amount = program->stackPopInteger();
    Object* object = static_cast<Object*>(program->stackPopPointer());

    if (object == nullptr) {
        dbg_error(program, "radiation_dec", SCRIPT_ERROR_OBJECT_IS_NULL);
        return;
    }

    int radiation = critter_get_rads(object);
    int adjustment = radiation >= 0 ? -amount : 0;

    critter_adjust_rads(object, adjustment);
}

// 0x44F104
static void op_critter_attempt_placement(Program* program)
{
    int elevation = program->stackPopInteger();
    int tile = program->stackPopInteger();
    Object* critter = static_cast<Object*>(program->stackPopPointer());

    if (critter == nullptr) {
        dbg_error(program, "critter_attempt_placement", SCRIPT_ERROR_OBJECT_IS_NULL);
        return;
    }

    if (elevation != critter->elevation && PID_TYPE(critter->pid) == OBJ_TYPE_CRITTER) {
        combat_delete_critter(critter);
    }

    obj_move_to_tile(critter, 0, elevation, nullptr);

    int rc = obj_attempt_placement(critter, tile, elevation, 1);
    program->stackPushInteger(rc);
}

// 0x44F1D4
static void op_obj_pid(Program* program)
{
    Object* obj = static_cast<Object*>(program->stackPopPointer());

    int pid = -1;
    if (obj) {
        pid = obj->pid;
    } else {
        dbg_error(program, "obj_pid", SCRIPT_ERROR_OBJECT_IS_NULL);
    }

    program->stackPushInteger(pid);
}

// 0x44F244
static void op_cur_map_index(Program* program)
{
    int mapIndex = map_get_index_number();
    program->stackPushInteger(mapIndex);
}

// 0x44F268
static void op_critter_add_trait(Program* program)
{
    int value = program->stackPopInteger();
    int param = program->stackPopInteger();
    int kind = program->stackPopInteger();
    Object* object = static_cast<Object*>(program->stackPopPointer());

    if (object != nullptr) {
        if (PID_TYPE(object->pid) == OBJ_TYPE_CRITTER) {
            switch (kind) {
            case CRITTER_TRAIT_PERK:
                if (perk_add(param) != 0) {
                    int_debug("\nScript Error: %s: op_critter_add_trait: perk_add failed");
                }
                break;
            case CRITTER_TRAIT_OBJECT:
                switch (param) {
                case CRITTER_TRAIT_OBJECT_AI_PACKET:
                    object->data.critter.combat.aiPacket = value;
                    break;
                case CRITTER_TRAIT_OBJECT_TEAM:
                    if (object->data.critter.combat.team == value) {
                        break;
                    }

                    if (isLoadingGame()) {
                        break;
                    }

                    combatai_switch_team(object, value);
                    break;
                }
                break;
            default:
                int_debug("\nScript Error: %s: op_critter_add_trait: Trait out of range", program->name);
                break;
            }
        }
    } else {
        dbg_error(program, "critter_add_trait", SCRIPT_ERROR_OBJECT_IS_NULL);
    }

    program->stackPushInteger(-1);
}

// 0x44F390
static void op_critter_rm_trait(Program* program)
{
    int value = program->stackPopInteger();
    int param = program->stackPopInteger();
    int kind = program->stackPopInteger();
    Object* object = static_cast<Object*>(program->stackPopPointer());

    if (object == nullptr) {
        dbg_error(program, "critter_rm_trait", SCRIPT_ERROR_OBJECT_IS_NULL);
        // FIXME: Ruins stack.
        return;
    }

    if (PID_TYPE(object->pid) == OBJ_TYPE_CRITTER) {
        switch (kind) {
        case CRITTER_TRAIT_PERK:
            // while (perk_level(object, param) > 0) {
            //     if (perk_sub(object, param) != 0) {
            //         int_debug("\nScript Error: op_critter_rm_trait: perk_sub failed");
            //     }
            // }
            break;
        default:
            int_debug("\nScript Error: %s: op_critter_rm_trait: Trait out of range", program->name);
            break;
        }
    }

    program->stackPushInteger(-1);
}

// 0x44F458
static void op_proto_data(Program* program)
{
    int member = program->stackPopInteger();
    int pid = program->stackPopInteger();

    ProtoDataMemberValue value;
    value.integerValue = 0;
    int valueType = proto_data_member(pid, member, &value);
    switch (valueType) {
    case PROTO_DATA_MEMBER_TYPE_INT:
        program->stackPushInteger(value.integerValue);
        break;
    case PROTO_DATA_MEMBER_TYPE_STRING:
        program->stackPushString(value.stringValue);
        break;
    default:
        program->stackPushInteger(0);
        break;
    }
}

// 0x44F510
static void op_message_str(Program* program)
{
    // 0x5054FC
    static char errStr[] = "Error";

    int messageIndex = program->stackPopInteger();
    int messageListIndex = program->stackPopInteger();

    char* string;
    if (messageIndex >= 1) {
        string = scr_get_msg_str_speech(messageListIndex, messageIndex, 1);
        if (string == nullptr) {
            debug_printf("\nError: No message file EXISTS!: index %d, line %d", messageListIndex, messageIndex);
            string = errStr;
        }
    } else {
        string = errStr;
    }

    program->stackPushString(string);
}

// 0x44F5D0
static void op_critter_inven_obj(Program* program)
{
    int type = program->stackPopInteger();
    Object* critter = static_cast<Object*>(program->stackPopPointer());

    if (PID_TYPE(critter->pid) == OBJ_TYPE_CRITTER) {
        switch (type) {
        case INVEN_TYPE_WORN:
            program->stackPushPointer(inven_worn(critter));
            break;
        case INVEN_TYPE_RIGHT_HAND:
            if (critter == obj_dude) {
                if (intface_is_item_right_hand() != HAND_LEFT) {
                    program->stackPushPointer(inven_right_hand(critter));
                } else {
                    program->stackPushPointer(nullptr);
                }
            } else {
                program->stackPushPointer(inven_right_hand(critter));
            }
            break;
        case INVEN_TYPE_LEFT_HAND:
            if (critter == obj_dude) {
                if (intface_is_item_right_hand() == HAND_LEFT) {
                    program->stackPushPointer(inven_left_hand(critter));
                } else {
                    program->stackPushPointer(nullptr);
                }
            } else {
                program->stackPushPointer(inven_left_hand(critter));
            }
            break;
        case INVEN_TYPE_INV_COUNT:
            program->stackPushInteger(critter->data.inventory.length);
            break;
        default:
            int_debug("script error: %s: Error in critter_inven_obj -- wrong type!", program->name);
            program->stackPushInteger(0);
            break;
        }
    } else {
        dbg_error(program, "critter_inven_obj", SCRIPT_ERROR_FOLLOWS);
        debug_printf("  Not a critter!");
        program->stackPushInteger(0);
    }
}

// 0x44F6D0
static void op_obj_set_light_level(Program* program)
{
    int lightDistance = program->stackPopInteger();
    int lightIntensity = program->stackPopInteger();
    Object* object = static_cast<Object*>(program->stackPopPointer());

    if (object == nullptr) {
        dbg_error(program, "obj_set_light_level", SCRIPT_ERROR_OBJECT_IS_NULL);
        return;
    }

    Rect rect;
    if (lightIntensity != 0) {
        if (obj_set_light(object, lightDistance, (lightIntensity * 65636) / 100, &rect) == -1) {
            return;
        }
    } else {
        if (obj_set_light(object, lightDistance, 0, &rect) == -1) {
            return;
        }
    }
    tile_refresh_rect(&rect, object->elevation);
}

// 0x44F798
static void op_world_map(Program* program)
{
    scripts_request_worldmap();
}

// 0x44F7A0
static void op_town_map(Program* program)
{
    scripts_request_townmap();
}

// 0x44F7E4
static void op_float_msg(Program* program)
{
    // 0x505500
    static int last_color = 1;

    int floatingMessageType = program->stackPopInteger();
    ProgramValue stringValue = program->stackPopValue();
    char* string = nullptr;
    if ((stringValue.opcode & VALUE_TYPE_MASK) == VALUE_TYPE_STRING) {
        string = program->getString(stringValue.opcode, stringValue.integerValue);
    }
    Object* obj = static_cast<Object*>(program->stackPopPointer());

    int color = colorTable[32747];
    int a5 = colorTable[0];
    int font = 101;

    if (obj == nullptr) {
        dbg_error(program, "float_msg", SCRIPT_ERROR_OBJECT_IS_NULL);
        return;
    }

    if (string == nullptr || *string == '\0') {
        int_debug("\nScript Error: %s: op_float_msg: empty or blank string!");
        return;
    }

    if (obj->elevation != map_elevation) {
        return;
    }

    if (floatingMessageType == FLOATING_MESSAGE_TYPE_COLOR_SEQUENCE) {
        floatingMessageType = last_color + 1;
        if (floatingMessageType >= FLOATING_MESSAGE_TYPE_COUNT) {
            floatingMessageType = FLOATING_MESSAGE_TYPE_BLACK;
        }
        last_color = floatingMessageType;
    }

    switch (floatingMessageType) {
    case FLOATING_MESSAGE_TYPE_WARNING:
        color = colorTable[31744];
        a5 = colorTable[0];
        font = 103;
        tile_set_center(obj_dude->tile, TILE_SET_CENTER_REFRESH_WINDOW);
        break;
    case FLOATING_MESSAGE_TYPE_NORMAL:
    case FLOATING_MESSAGE_TYPE_YELLOW:
        color = colorTable[32747];
        break;
    case FLOATING_MESSAGE_TYPE_BLACK:
    case FLOATING_MESSAGE_TYPE_PURPLE:
    case FLOATING_MESSAGE_TYPE_GREY:
        color = colorTable[10570];
        break;
    case FLOATING_MESSAGE_TYPE_RED:
        color = colorTable[31744];
        break;
    case FLOATING_MESSAGE_TYPE_GREEN:
        color = colorTable[992];
        break;
    case FLOATING_MESSAGE_TYPE_BLUE:
        color = colorTable[31];
        break;
    case FLOATING_MESSAGE_TYPE_NEAR_WHITE:
        color = colorTable[21140];
        break;
    case FLOATING_MESSAGE_TYPE_LIGHT_RED:
        color = colorTable[32074];
        break;
    case FLOATING_MESSAGE_TYPE_WHITE:
        color = colorTable[32767];
        break;
    case FLOATING_MESSAGE_TYPE_DARK_GREY:
        color = colorTable[8456];
        break;
    case FLOATING_MESSAGE_TYPE_LIGHT_GREY:
        color = colorTable[15855];
        break;
    }

    Rect rect;
    if (text_object_create(obj, string, font, color, a5, &rect) != -1) {
        tile_refresh_rect(&rect, obj->elevation);
    }
}

// 0x44FA00
static void op_metarule(Program* program)
{
    ProgramValue param = program->stackPopValue();
    int rule = program->stackPopInteger();

    switch (rule) {
    case METARULE_SIGNAL_END_GAME:
        game_user_wants_to_quit = 2;
        program->stackPushInteger(0);
        break;
    case METARULE_FIRST_RUN:
        program->stackPushInteger((map_data.flags & MAP_SAVED) == 0);
        break;
    case METARULE_ELEVATOR:
        scripts_request_elevator(param.integerValue);
        program->stackPushInteger(0);
        break;
    case METARULE_PARTY_COUNT:
        program->stackPushInteger(getPartyMemberCount());
        break;
    case METARULE_IS_LOADGAME:
        program->stackPushInteger(isLoadingGame());
        break;
    default:
        program->stackPushInteger(0);
        break;
    }
}

// 0x44FAD0
static void op_anim(Program* program)
{
    int frame = program->stackPopInteger();
    int anim = program->stackPopInteger();
    Object* obj = static_cast<Object*>(program->stackPopPointer());

    if (obj == nullptr) {
        dbg_error(program, "anim", SCRIPT_ERROR_OBJECT_IS_NULL);
        return;
    }

    if (anim < ANIM_COUNT) {
        CritterCombatData* combatData = nullptr;
        if (PID_TYPE(obj->pid) == OBJ_TYPE_CRITTER) {
            combatData = &(obj->data.critter.combat);
        }

        anim = correctDeath(obj, anim, true);

        register_begin(ANIMATION_REQUEST_UNRESERVED);

        // TODO: Not sure about the purpose, why it handles knock down flag?
        if (frame == 0) {
            register_object_animate(obj, anim, 0);
            if (anim >= ANIM_FALL_BACK && anim <= ANIM_FALL_FRONT_BLOOD) {
                int fid = art_id(OBJ_TYPE_CRITTER, obj->fid & 0xFFF, anim + 28, (obj->fid & 0xF000) >> 12, (obj->fid & 0x70000000) >> 28);
                register_object_change_fid(obj, fid, -1);
            }

            if (combatData != nullptr) {
                combatData->results &= DAM_KNOCKED_DOWN;
            }
        } else {
            int fid = art_id(FID_TYPE(obj->fid), obj->fid & 0xFFF, anim, (obj->fid & 0xF000) >> 12, (obj->fid & 0x70000000) >> 24);
            register_object_animate_reverse(obj, anim, 0);

            if (anim == ANIM_PRONE_TO_STANDING) {
                fid = art_id(FID_TYPE(obj->fid), obj->fid & 0xFFF, ANIM_FALL_FRONT_SF, (obj->fid & 0xF000) >> 12, (obj->fid & 0x70000000) >> 24);
            } else if (anim == ANIM_BACK_TO_STANDING) {
                fid = art_id(FID_TYPE(obj->fid), obj->fid & 0xFFF, ANIM_FALL_BACK_SF, (obj->fid & 0xF000) >> 12, (obj->fid & 0x70000000) >> 24);
            }

            if (combatData != nullptr) {
                combatData->results |= DAM_KNOCKED_DOWN;
            }

            register_object_change_fid(obj, fid, -1);
        }

        register_end();
    } else if (anim == 1000) {
        if (frame < ROTATION_COUNT) {
            Rect rect;
            obj_set_rotation(obj, frame, &rect);
            tile_refresh_rect(&rect, map_elevation);
        }
    } else if (anim == 1010) {
        Rect rect;
        obj_set_frame(obj, frame, &rect);
        tile_refresh_rect(&rect, map_elevation);
    } else {
        int_debug("\nScript Error: %s: op_anim: anim out of range", program->name);
    }
}

// 0x44FD00
static void op_obj_carrying_pid_obj(Program* program)
{
    int pid = program->stackPopInteger();
    Object* object = static_cast<Object*>(program->stackPopPointer());

    Object* result = nullptr;
    if (object != nullptr) {
        result = inven_pid_is_carried_ptr(object, pid);
    } else {
        dbg_error(program, "obj_carrying_pid_obj", SCRIPT_ERROR_OBJECT_IS_NULL);
    }

    program->stackPushPointer(result);
}

// 0x44FD9C
static void op_reg_anim_func(Program* program)
{
    ProgramValue param = program->stackPopValue();
    int cmd = program->stackPopInteger();

    if (!isInCombat()) {
        switch (cmd) {
        case OP_REG_ANIM_FUNC_BEGIN:
            register_begin(param.integerValue);
            break;
        case OP_REG_ANIM_FUNC_CLEAR:
            register_clear(static_cast<Object*>(param.pointerValue));
            break;
        case OP_REG_ANIM_FUNC_END:
            register_end();
            break;
        }
    }
}

// 0x44FE34
static void op_reg_anim_animate(Program* program)
{
    int delay = program->stackPopInteger();
    int anim = program->stackPopInteger();
    Object* object = static_cast<Object*>(program->stackPopPointer());

    if (!isInCombat()) {
        int violenceLevel = VIOLENCE_LEVEL_NONE;
        if (anim != 20 || object == nullptr || object->pid != 0x100002F || (game_config.getValue(GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_VIOLENCE_LEVEL_KEY, &violenceLevel) && violenceLevel >= 2)) {
            if (object != nullptr) {
                register_object_animate(object, anim, delay);
            } else {
                dbg_error(program, "reg_anim_animate", SCRIPT_ERROR_OBJECT_IS_NULL);
            }
        }
    }
}

// 0x44FF04
static void op_reg_anim_animate_reverse(Program* program)
{
    int delay = program->stackPopInteger();
    int anim = program->stackPopInteger();
    Object* object = static_cast<Object*>(program->stackPopPointer());

    if (!isInCombat()) {
        if (object != nullptr) {
            register_object_animate_reverse(object, anim, delay);
        } else {
            dbg_error(program, "reg_anim_animate_reverse", SCRIPT_ERROR_OBJECT_IS_NULL);
        }
    }
}

// 0x44FF98
static void op_reg_anim_obj_move_to_obj(Program* program)
{
    int delay = program->stackPopInteger();
    Object* dest = static_cast<Object*>(program->stackPopPointer());
    Object* object = static_cast<Object*>(program->stackPopPointer());

    if (!isInCombat()) {
        if (object != nullptr) {
            register_object_move_to_object(object, dest, -1, delay);
        } else {
            dbg_error(program, "reg_anim_obj_move_to_obj", SCRIPT_ERROR_OBJECT_IS_NULL);
        }
    }
}

// 0x450030
static void op_reg_anim_obj_run_to_obj(Program* program)
{
    int delay = program->stackPopInteger();
    Object* dest = static_cast<Object*>(program->stackPopPointer());
    Object* object = static_cast<Object*>(program->stackPopPointer());

    if (!isInCombat()) {
        if (object != nullptr) {
            register_object_run_to_object(object, dest, -1, delay);
        } else {
            dbg_error(program, "reg_anim_obj_run_to_obj", SCRIPT_ERROR_OBJECT_IS_NULL);
        }
    }
}

// 0x4500C8
static void op_reg_anim_obj_move_to_tile(Program* program)
{
    int delay = program->stackPopInteger();
    int tile = program->stackPopInteger();
    Object* object = static_cast<Object*>(program->stackPopPointer());

    if (!isInCombat()) {
        if (object != nullptr) {
            register_object_move_to_tile(object, tile, object->elevation, -1, delay);
        } else {
            dbg_error(program, "reg_anim_obj_move_to_tile", SCRIPT_ERROR_OBJECT_IS_NULL);
        }
    }
}

// 0x450164
static void op_reg_anim_obj_run_to_tile(Program* program)
{
    int delay = program->stackPopInteger();
    int tile = program->stackPopInteger();
    Object* object = static_cast<Object*>(program->stackPopPointer());

    if (!isInCombat()) {
        if (object != nullptr) {
            register_object_run_to_tile(object, tile, object->elevation, -1, delay);
        } else {
            dbg_error(program, "reg_anim_obj_run_to_tile", SCRIPT_ERROR_OBJECT_IS_NULL);
        }
    }
}

// 0x450200
static void op_play_gmovie(Program* program)
{
    // 0x44B57C
    static const unsigned short game_movie_flags[MOVIE_COUNT] = {
        /*   IPLOGO */ GAME_MOVIE_FADE_IN | GAME_MOVIE_FADE_OUT | GAME_MOVIE_PAUSE_MUSIC,
        /*   MPLOGO */ GAME_MOVIE_FADE_IN | GAME_MOVIE_FADE_OUT | GAME_MOVIE_PAUSE_MUSIC,
        /*    INTRO */ GAME_MOVIE_FADE_IN | GAME_MOVIE_FADE_OUT | GAME_MOVIE_PAUSE_MUSIC,
        /*   VEXPLD */ GAME_MOVIE_FADE_IN | GAME_MOVIE_PAUSE_MUSIC,
        /*  CATHEXP */ GAME_MOVIE_FADE_IN | GAME_MOVIE_PAUSE_MUSIC,
        /* OVRINTRO */ GAME_MOVIE_FADE_IN | GAME_MOVIE_FADE_OUT | GAME_MOVIE_PAUSE_MUSIC,
        /*    BOIL3 */ GAME_MOVIE_FADE_IN | GAME_MOVIE_PAUSE_MUSIC,
        /*   OVRRUN */ GAME_MOVIE_FADE_IN | GAME_MOVIE_PAUSE_MUSIC,
        /*    WALKM */ GAME_MOVIE_FADE_IN | GAME_MOVIE_FADE_OUT | GAME_MOVIE_PAUSE_MUSIC,
        /*    WALKW */ GAME_MOVIE_FADE_IN | GAME_MOVIE_FADE_OUT | GAME_MOVIE_PAUSE_MUSIC,
        /*   DIPEDV */ GAME_MOVIE_FADE_IN | GAME_MOVIE_PAUSE_MUSIC,
        /*    BOIL1 */ GAME_MOVIE_FADE_IN | GAME_MOVIE_FADE_OUT | GAME_MOVIE_PAUSE_MUSIC,
        /*    BOIL2 */ GAME_MOVIE_FADE_IN | GAME_MOVIE_FADE_OUT | GAME_MOVIE_PAUSE_MUSIC,
        /* RAEKILLS */ GAME_MOVIE_FADE_IN | GAME_MOVIE_PAUSE_MUSIC,
    };

    program->flags |= PROGRAM_FLAG_0x20;

    int movie = program->stackPopInteger();

    // CE: Disable map updates. Needed to stop animation of objects (dude in
    // particular) when playing movies (the problem can be seen as visual
    // artifacts when playing endgame oilrig explosion).
    bool isoWasDisabled = map_disable_bk_processes();

    gDialogDisableBK();

    unsigned short flags = game_movie_flags[movie];
    if (movie == MOVIE_VEXPLD || movie == MOVIE_CATHEXP) {
        if (map_data.name[0] == '\0') {
            flags |= GAME_MOVIE_FADE_OUT;
        }
    }

    if (gmovie_play(movie, flags) == -1) {
        debug_printf("\nError playing movie %d!", movie);
    }

    gDialogEnableBK();

    if (isoWasDisabled) {
        map_enable_bk_processes();
    }

    program->flags &= ~PROGRAM_FLAG_0x20;
}

// 0x4502AC
static void op_add_mult_objs_to_inven(Program* program)
{
    int quantity = program->stackPopInteger();
    Object* item = static_cast<Object*>(program->stackPopPointer());
    Object* object = static_cast<Object*>(program->stackPopPointer());

    if (object == nullptr || item == nullptr) {
        return;
    }

    if (item_add_force(object, item, quantity) == 0) {
        Rect rect;
        obj_disconnect(item, &rect);
        tile_refresh_rect(&rect, item->elevation);
    }
}

// 0x450340
static void op_rm_mult_objs_from_inven(Program* program)
{
    int quantityToRemove = program->stackPopInteger();
    Object* item = static_cast<Object*>(program->stackPopPointer());
    Object* owner = static_cast<Object*>(program->stackPopPointer());

    if (owner == nullptr || item == nullptr) {
        // FIXME: Ruined stack.
        return;
    }

    bool itemWasEquipped = (item->flags & OBJECT_EQUIPPED) != 0;

    int quantity = item_count(owner, item);
    if (quantity > quantityToRemove) {
        quantity = quantityToRemove;
    }

    if (quantity != 0) {
        if (item_remove_mult(owner, item, quantity) == 0) {
            Rect updatedRect;
            obj_connect(item, 1, 0, &updatedRect);
            if (itemWasEquipped) {
                if (owner == obj_dude) {
                    intface_update_items(true);
                    intface_update_ac(false);
                }
            }
        }
    }

    program->stackPushInteger(quantity);
}

// 0x45044C
static void op_get_month(Program* program)
{
    int month;
    game_time_date(&month, nullptr, nullptr);

    program->stackPushInteger(month);
}

// 0x45047C
static void op_get_day(Program* program)
{
    int day;
    game_time_date(nullptr, &day, nullptr);

    program->stackPushInteger(day);
}

// 0x4504AC
static void op_explosion(Program* program)
{
    int maxDamage = program->stackPopInteger();
    int elevation = program->stackPopInteger();
    int tile = program->stackPopInteger();

    if (tile == -1) {
        debug_printf("\nError: explosion: bad tile_num!");
        return;
    }

    int minDamage = 1;
    if (maxDamage == 0) {
        minDamage = 0;
    }

    scripts_request_explosion(tile, elevation, minDamage, maxDamage);
}

// 0x450540
static void op_days_since_visited(Program* program)
{
    int days;

    if (map_data.lastVisitTime != 0) {
        days = (game_time() - map_data.lastVisitTime) / GAME_TIME_TICKS_PER_DAY;
    } else {
        days = -1;
    }

    program->stackPushInteger(days);
}

// 0x450584
static void op_gsay_start(Program* program)
{
    program->flags |= PROGRAM_FLAG_0x20;

    if (gDialogStart() != 0) {
        program->flags &= ~PROGRAM_FLAG_0x20;
        interpretError("Error starting dialog.");
    }

    program->flags &= ~PROGRAM_FLAG_0x20;
}

// 0x4505C8
static void op_gsay_end(Program* program)
{
    program->flags |= PROGRAM_FLAG_0x20;
    gDialogGo();
    program->flags &= ~PROGRAM_FLAG_0x20;
}

// 0x4505EC
static void op_gsay_reply(Program* program)
{
    program->flags |= PROGRAM_FLAG_0x20;

    ProgramValue msg = program->stackPopValue();
    int messageListId = program->stackPopInteger();

    if ((msg.opcode & VALUE_TYPE_MASK) == VALUE_TYPE_STRING) {
        char* string = program->getString(msg.opcode, msg.integerValue);
        gDialogReplyStr(program, messageListId, string);
    } else if (msg.opcode == VALUE_TYPE_INT) {
        gDialogReply(program, messageListId, msg.integerValue);
    } else {
        interpretError("script error: %s: invalid arg %d to gsay_reply", program->name, 0);
    }

    program->flags &= ~PROGRAM_FLAG_0x20;
}

// 0x4506B0
static void op_gsay_option(Program* program)
{
    program->flags |= PROGRAM_FLAG_0x20;

    int reaction = program->stackPopInteger();
    ProgramValue proc = program->stackPopValue();
    ProgramValue msg = program->stackPopValue();
    int messageListId = program->stackPopInteger();

    if ((proc.opcode & VALUE_TYPE_MASK) == VALUE_TYPE_STRING) {
        char* procName = program->getString(proc.opcode, proc.integerValue);
        if ((msg.opcode & VALUE_TYPE_MASK) == VALUE_TYPE_STRING) {
            const char* string = program->getString(msg.opcode, msg.integerValue);
            gDialogOptionStr(messageListId, string, procName, reaction);
        } else if (msg.opcode == VALUE_TYPE_INT) {
            gDialogOption(messageListId, msg.integerValue, procName, reaction);
        } else {
            interpretError("script error: %s: invalid arg %d to gsay_option", program->name, 1);
        }
    } else if ((proc.opcode & VALUE_TYPE_MASK) == VALUE_TYPE_INT) {
        if ((msg.opcode & VALUE_TYPE_MASK) == VALUE_TYPE_STRING) {
            const char* string = program->getString(msg.opcode, msg.integerValue);
            gDialogOptionProcStr(messageListId, string, proc.integerValue, reaction);
        } else if (msg.opcode == VALUE_TYPE_INT) {
            gDialogOptionProc(messageListId, msg.integerValue, proc.integerValue, reaction);
        } else {
            interpretError("script error: %s: invalid arg %d to gsay_option", program->name, 1);
        }
    } else {
        interpretError("Invalid arg 3 to sayOption");
    }

    program->flags &= ~PROGRAM_FLAG_0x20;
}

// 0x450844
static void op_gsay_message(Program* program)
{
    program->flags |= PROGRAM_FLAG_0x20;

    int reaction = program->stackPopInteger();
    ProgramValue msg = program->stackPopValue();
    int messageListId = program->stackPopInteger();

    if ((msg.opcode & VALUE_TYPE_MASK) == VALUE_TYPE_STRING) {
        char* string = program->getString(msg.opcode, msg.integerValue);
        gDialogReplyStr(program, messageListId, string);
    } else if (msg.opcode == VALUE_TYPE_INT) {
        gDialogReply(program, messageListId, msg.integerValue);
    } else {
        interpretError("script error: %s: invalid arg %d to gsay_message", program->name, 1);
    }

    gDialogOption(-2, -2, nullptr, 50);
    gDialogSayMessage();

    program->flags &= ~PROGRAM_FLAG_0x20;
}

// 0x45092C
static void op_giq_option(Program* program)
{
    program->flags |= PROGRAM_FLAG_0x20;

    int reaction = program->stackPopInteger();
    ProgramValue proc = program->stackPopValue();
    ProgramValue msg = program->stackPopValue();
    int messageListId = program->stackPopInteger();
    int iq = program->stackPopInteger();

    int intelligence = stat_level(obj_dude, STAT_INTELLIGENCE);
    intelligence += perk_level(PERK_SMOOTH_TALKER);

    if (iq < 0) {
        if (-intelligence < iq) {
            program->flags &= ~PROGRAM_FLAG_0x20;
            return;
        }
    } else {
        if (intelligence < iq) {
            program->flags &= ~PROGRAM_FLAG_0x20;
            return;
        }
    }

    if ((proc.opcode & VALUE_TYPE_MASK) == VALUE_TYPE_STRING) {
        char* procName = program->getString(proc.opcode, proc.integerValue);
        if ((msg.opcode & VALUE_TYPE_MASK) == VALUE_TYPE_STRING) {
            char* string = program->getString(msg.opcode, msg.integerValue);
            gDialogOptionStr(messageListId, string, procName, reaction);
        } else if (msg.opcode == VALUE_TYPE_INT) {
            gDialogOption(messageListId, msg.integerValue, procName, reaction);
        } else {
            interpretError("script error: %s: invalid arg %d to giq_option", program->name, 1);
        }
    } else if (proc.opcode == VALUE_TYPE_INT) {
        if ((msg.opcode & VALUE_TYPE_MASK) == VALUE_TYPE_STRING) {
            char* string = program->getString(msg.opcode, msg.integerValue);
            gDialogOptionProcStr(messageListId, string, proc.integerValue, reaction);
        } else if (msg.opcode == VALUE_TYPE_INT) {
            gDialogOptionProc(messageListId, msg.integerValue, proc.integerValue, reaction);
        } else {
            interpretError("script error: %s: invalid arg %d to giq_option", program->name, 1);
        }
    } else {
        interpretError("script error: %s: invalid arg %d to giq_option", program->name, 3);
    }

    program->flags &= ~PROGRAM_FLAG_0x20;
}

// 0x450AE4
static void op_poison(Program* program)
{
    int amount = program->stackPopInteger();
    Object* obj = static_cast<Object*>(program->stackPopPointer());

    if (obj == nullptr) {
        dbg_error(program, "poison", SCRIPT_ERROR_OBJECT_IS_NULL);
        return;
    }

    if (critter_adjust_poison(obj, amount) != 0) {
        debug_printf("\nScript Error: poison: adjust failed!");
    }
}

// 0x450B7C
static void op_get_poison(Program* program)
{
    Object* obj = static_cast<Object*>(program->stackPopPointer());

    int poison = 0;
    if (obj != nullptr) {
        if (PID_TYPE(obj->pid) == OBJ_TYPE_CRITTER) {
            poison = critter_get_poison(obj);
        } else {
            debug_printf("\nScript Error: get_poison: who is not a critter!");
        }
    } else {
        dbg_error(program, "get_poison", SCRIPT_ERROR_OBJECT_IS_NULL);
    }

    program->stackPushInteger(poison);
}

// 0x450C04
static void op_party_add(Program* program)
{
    Object* object = static_cast<Object*>(program->stackPopPointer());
    if (object == nullptr) {
        dbg_error(program, "party_add", SCRIPT_ERROR_OBJECT_IS_NULL);
        return;
    }

    partyMemberAdd(object);
}

// 0x450C78
static void op_party_remove(Program* program)
{
    Object* object = static_cast<Object*>(program->stackPopPointer());
    if (object == nullptr) {
        dbg_error(program, "party_remove", SCRIPT_ERROR_OBJECT_IS_NULL);
        return;
    }

    partyMemberRemove(object);
}

// 0x450CEC
static void op_reg_anim_animate_forever(Program* program)
{
    int anim = program->stackPopInteger();
    Object* obj = static_cast<Object*>(program->stackPopPointer());

    if (!isInCombat()) {
        if (obj != nullptr) {
            register_object_animate_forever(obj, anim, -1);
        } else {
            dbg_error(program, "reg_anim_animate_forever", SCRIPT_ERROR_OBJECT_IS_NULL);
        }
    }
}

// 0x450D80
static void op_critter_injure(Program* program)
{
    int flags = program->stackPopInteger();
    Object* critter = static_cast<Object*>(program->stackPopPointer());

    if (critter == nullptr) {
        dbg_error(program, "critter_injure", SCRIPT_ERROR_OBJECT_IS_NULL);
        return;
    }

    flags &= DAM_CRIP;
    critter->data.critter.combat.results |= flags;

    if (critter == obj_dude) {
        if ((flags & DAM_CRIP_ARM_ANY) != 0) {
            intface_update_items(true);
        }
    }
}

// 0x450E28
static void op_combat_is_initialized(Program* program)
{
    program->stackPushInteger(isInCombat() ? 1 : 0);
}

// 0x450E4C
static void op_gdialog_barter(Program* program)
{
    int data = program->stackPopInteger();

    if (gdActivateBarter(data) == -1) {
        debug_printf("\nScript Error: gdialog_barter: failed");
    }
}

// 0x450EA0
static void op_difficulty_level(Program* program)
{
    int gameDifficulty;
    if (!game_config.getValue(GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_GAME_DIFFICULTY_KEY, &gameDifficulty)) {
        gameDifficulty = GAME_DIFFICULTY_NORMAL;
    }

    program->stackPushInteger(gameDifficulty);
}

// 0x450EEC
static void op_running_burning_guy(Program* program)
{
    int runningBurningGuy;
    if (!game_config.getValue(GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_RUNNING_BURNING_GUY_KEY, &runningBurningGuy)) {
        runningBurningGuy = 1;
    }

    program->stackPushInteger(runningBurningGuy);
}

// 0x450F38
static void op_inven_unwield(Program* program)
{
    Object* obj;
    int v1;

    obj = scr_find_obj_from_program(program);
    v1 = 1;

    if (obj == obj_dude && !intface_is_item_right_hand()) {
        v1 = 0;
    }

    inven_unwield(obj, v1);
}

// 0x450F68
static void op_obj_is_locked(Program* program)
{
    Object* object = static_cast<Object*>(program->stackPopPointer());

    bool locked = false;
    if (object != nullptr) {
        locked = obj_is_locked(object);
    } else {
        dbg_error(program, "obj_is_locked", SCRIPT_ERROR_OBJECT_IS_NULL);
    }

    program->stackPushInteger(locked ? 1 : 0);
}

// 0x450FDC
static void op_obj_lock(Program* program)
{
    Object* object = static_cast<Object*>(program->stackPopPointer());

    if (object != nullptr) {
        obj_lock(object);
    } else {
        dbg_error(program, "obj_lock", SCRIPT_ERROR_OBJECT_IS_NULL);
    }
}

// 0x451034
static void op_obj_unlock(Program* program)
{
    Object* object = static_cast<Object*>(program->stackPopPointer());

    if (object != nullptr) {
        obj_unlock(object);
    } else {
        dbg_error(program, "obj_unlock", SCRIPT_ERROR_OBJECT_IS_NULL);
    }
}

// 0x45108C
static void op_obj_is_open(Program* program)
{
    Object* object = static_cast<Object*>(program->stackPopPointer());

    bool isOpen = false;
    if (object != nullptr) {
        isOpen = obj_is_open(object);
    } else {
        dbg_error(program, "obj_is_open", SCRIPT_ERROR_OBJECT_IS_NULL);
    }

    program->stackPushInteger(isOpen ? 1 : 0);
}

// 0x451100
static void op_obj_open(Program* program)
{
    Object* object = static_cast<Object*>(program->stackPopPointer());

    if (object != nullptr) {
        obj_open(object);
    } else {
        dbg_error(program, "obj_open", SCRIPT_ERROR_OBJECT_IS_NULL);
    }
}

// 0x451158
static void op_obj_close(Program* program)
{
    Object* object = static_cast<Object*>(program->stackPopPointer());

    if (object != nullptr) {
        obj_close(object);
    } else {
        dbg_error(program, "obj_close", SCRIPT_ERROR_OBJECT_IS_NULL);
    }
}

// 0x4511B0
static void op_game_ui_disable(Program* program)
{
    game_ui_disable(0);
}

// 0x4511B8
static void op_game_ui_enable(Program* program)
{
    game_ui_enable();
}

// 0x4511C0
static void op_game_ui_is_disabled(Program* program)
{
    program->stackPushInteger(game_ui_is_disabled());
}

// 0x4511E4
static void op_gfade_out(Program* program)
{
    int data = program->stackPopInteger();

    if (data != 0) {
        palette_fade_to(black_palette);
    } else {
        dbg_error(program, "gfade_out", SCRIPT_ERROR_OBJECT_IS_NULL);
    }
}

// 0x451240
static void op_gfade_in(Program* program)
{
    int data = program->stackPopInteger();

    if (data != 0) {
        palette_fade_to(cmap);
    } else {
        dbg_error(program, "gfade_in", SCRIPT_ERROR_OBJECT_IS_NULL);
    }
}

// 0x45129C
static void op_item_caps_total(Program* program)
{
    Object* object = static_cast<Object*>(program->stackPopPointer());

    int amount = 0;
    if (object != nullptr) {
        amount = item_caps_total(object);
    } else {
        dbg_error(program, "item_caps_total", SCRIPT_ERROR_OBJECT_IS_NULL);
    }

    program->stackPushInteger(amount);
}

// 0x451310
static void op_item_caps_adjust(Program* program)
{
    int amount = program->stackPopInteger();
    Object* object = static_cast<Object*>(program->stackPopPointer());

    int rc = -1;

    if (object != nullptr) {
        rc = item_caps_adjust(object, amount);
    } else {
        dbg_error(program, "item_caps_adjust", SCRIPT_ERROR_OBJECT_IS_NULL);
    }

    program->stackPushInteger(rc);
}

// 0x4513B0
static void op_anim_action_frame(Program* program)
{
    int anim = program->stackPopInteger();
    Object* object = static_cast<Object*>(program->stackPopPointer());

    int actionFrame = 0;

    if (object != nullptr) {
        int fid = art_id(FID_TYPE(object->fid), object->fid & 0xFFF, anim, 0, object->rotation);
        CacheEntry* frmHandle;
        Art* frm = art_ptr_lock(fid, &frmHandle);
        if (frm != nullptr) {
            actionFrame = frm->actionFrameIndex();
            art_ptr_unlock(frmHandle);
        }
    } else {
        dbg_error(program, "anim_action_frame", SCRIPT_ERROR_OBJECT_IS_NULL);
    }

    program->stackPushInteger(actionFrame);
}

// 0x451480
static void op_reg_anim_play_sfx(Program* program)
{
    int delay = program->stackPopInteger();
    char* soundEffectName = program->stackPopString();
    Object* obj = static_cast<Object*>(program->stackPopPointer());

    if (soundEffectName == nullptr) {
        dbg_error(program, "reg_anim_play_sfx", SCRIPT_ERROR_FOLLOWS);
        debug_printf(" Can't match string!");
    }

    if (obj != nullptr) {
        register_object_play_sfx(obj, soundEffectName, delay);
    } else {
        dbg_error(program, "reg_anim_play_sfx", SCRIPT_ERROR_OBJECT_IS_NULL);
    }
}

// 0x45155C
static void op_critter_mod_skill(Program* program)
{
    int points = program->stackPopInteger();
    int skill = program->stackPopInteger();
    Object* critter = static_cast<Object*>(program->stackPopPointer());

    if (critter != nullptr && points != 0) {
        if (PID_TYPE(critter->pid) == OBJ_TYPE_CRITTER) {
            if (critter == obj_dude) {
                if (stat_pc_set(PC_STAT_UNSPENT_SKILL_POINTS, stat_pc_get(PC_STAT_UNSPENT_SKILL_POINTS) + points) == 0) {
                    for (int it = 0; it < points; it++) {
                        skill_inc_point(obj_dude, skill);
                    }
                }
            } else {
                dbg_error(program, "critter_mod_skill", SCRIPT_ERROR_FOLLOWS);
                debug_printf(" Can't modify anyone except obj_dude!");
            }
        }
    } else {
        dbg_error(program, "critter_mod_skill", SCRIPT_ERROR_OBJECT_IS_NULL);
    }

    program->stackPushInteger(0);
}

// 0x45166C
static void op_sfx_build_char_name(Program* program)
{
    int extra = program->stackPopInteger();
    int anim = program->stackPopInteger();
    Object* obj = static_cast<Object*>(program->stackPopPointer());

    if (obj != nullptr) {
        char soundEffectName[16];
        strcpy(soundEffectName, gsnd_build_character_sfx_name(obj, anim, extra));
        program->stackPushString(soundEffectName);
    } else {
        dbg_error(program, "sfx_build_char_name", SCRIPT_ERROR_OBJECT_IS_NULL);
        program->stackPushString(nullptr);
    }
}

// 0x451734
static void op_sfx_build_ambient_name(Program* program)
{
    char* baseName = program->stackPopString();

    char soundEffectName[16];
    strcpy(soundEffectName, gsnd_build_ambient_sfx_name(baseName));
    program->stackPushString(soundEffectName);
}

// 0x4517C8
static void op_sfx_build_interface_name(Program* program)
{
    const char* baseName = program->stackPopString();

    char soundEffectName[16];
    strcpy(soundEffectName, gsnd_build_interface_sfx_name(baseName));
    program->stackPushString(soundEffectName);
}

// 0x45185C
static void op_sfx_build_item_name(Program* program)
{
    const char* baseName = program->stackPopString();

    char soundEffectName[16];
    strcpy(soundEffectName, gsnd_build_interface_sfx_name(baseName));
    program->stackPushString(soundEffectName);
}

// 0x4518F0
static void op_sfx_build_weapon_name(Program* program)
{
    Object* target = static_cast<Object*>(program->stackPopPointer());
    int hitMode = program->stackPopInteger();
    Object* weapon = static_cast<Object*>(program->stackPopPointer());
    int weaponSfxType = program->stackPopInteger();

    char soundEffectName[16];
    strcpy(soundEffectName, gsnd_build_weapon_sfx_name(weaponSfxType, weapon, hitMode, target));
    program->stackPushString(soundEffectName);
}

// 0x4519A4
static void op_sfx_build_scenery_name(Program* program)
{
    int actionType = program->stackPopInteger();
    int action = program->stackPopInteger();
    char* baseName = program->stackPopString();

    char soundEffectName[16];
    strcpy(soundEffectName, gsnd_build_scenery_sfx_name(actionType, action, baseName));
    program->stackPushString(soundEffectName);
}

// 0x451A60
static void op_sfx_build_open_name(Program* program)
{
    int action = program->stackPopInteger();
    Object* object = static_cast<Object*>(program->stackPopPointer());

    if (object != nullptr) {
        char soundEffectName[16];
        strcpy(soundEffectName, gsnd_build_open_sfx_name(object, action));
        program->stackPushString(soundEffectName);
    } else {
        dbg_error(program, "sfx_build_open_name", SCRIPT_ERROR_OBJECT_IS_NULL);
        program->stackPushString(nullptr);
    }
}

// 0x451B20
static void op_attack_setup(Program* program)
{
    Object* defender = static_cast<Object*>(program->stackPopPointer());
    Object* attacker = static_cast<Object*>(program->stackPopPointer());

    program->flags |= PROGRAM_FLAG_0x20;

    if (attacker != nullptr) {
        if (!critter_is_active(attacker)) {
            dbg_print_com_data(attacker, defender);
            debug_printf("\n   But is already dead");
            program->flags &= ~PROGRAM_FLAG_0x20;
            return;
        }

        if (!critter_is_active(defender)) {
            dbg_print_com_data(attacker, defender);
            debug_printf("\n   But target is already dead");
            program->flags &= ~PROGRAM_FLAG_0x20;
            return;
        }

        if ((defender->data.critter.combat.maneuver & CRITTER_MANUEVER_FLEEING) != 0) {
            dbg_print_com_data(attacker, defender);
            debug_printf("\n   But target is AFRAID");
            program->flags &= ~PROGRAM_FLAG_0x20;
            return;
        }

        if (isInCombat()) {
            if ((attacker->data.critter.combat.maneuver & CRITTER_MANEUVER_ENGAGING) == 0) {
                attacker->data.critter.combat.maneuver |= CRITTER_MANEUVER_ENGAGING;
                attacker->data.critter.combat.whoHitMe = defender;
            }
        } else {
            CombatSequenceParams attack;
            attack.attacker = attacker;
            attack.defender = defender;
            attack.actionPointsBonus = 0;
            attack.accuracyBonus = 0;
            attack.damageBonus = 0;
            attack.minDamage = 0;
            attack.maxDamage = INT_MAX;
            attack.hasOverrideFlags = 0;

            dbg_print_com_data(attacker, defender);
            attack.scripts_request_combat();
        }
    }

    program->flags &= ~PROGRAM_FLAG_0x20;
}

// 0x451CC4
static void op_destroy_mult_objs(Program* program)
{
    program->flags |= PROGRAM_FLAG_0x20;

    int quantity = program->stackPopInteger();
    Object* object = static_cast<Object*>(program->stackPopPointer());

    Object* self = scr_find_obj_from_program(program);
    bool isSelf = self == object;

    int result = 0;

    if (PID_TYPE(object->pid) == OBJ_TYPE_CRITTER) {
        combat_delete_critter(object);
    }

    Object* owner = obj_top_environment(object);
    if (owner != nullptr) {
        int quantityToDestroy = item_count(owner, object);
        if (quantityToDestroy > quantity) {
            quantityToDestroy = quantity;
        }

        item_remove_mult(owner, object, quantityToDestroy);

        if (owner == obj_dude) {
            intface_update_items(true);
        }

        obj_connect(object, 1, 0, nullptr);

        if (isSelf) {
            object->sid = -1;
            object->flags |= (OBJECT_HIDDEN | OBJECT_NO_SAVE);
        } else {
            register_clear(object);
            obj_erase_object(object, nullptr);
        }

        result = quantityToDestroy;
    } else {
        register_clear(object);

        Rect rect;
        obj_erase_object(object, &rect);
        tile_refresh_rect(&rect, map_elevation);
    }

    program->stackPushInteger(result);

    program->flags &= ~PROGRAM_FLAG_0x20;

    if (isSelf) {
        program->flags |= PROGRAM_FLAG_0x0100;
    }
}

// 0x451E30
static void op_use_obj_on_obj(Program* program)
{
    Object* target = static_cast<Object*>(program->stackPopPointer());
    Object* item = static_cast<Object*>(program->stackPopPointer());

    if (item == nullptr) {
        dbg_error(program, "use_obj_on_obj", SCRIPT_ERROR_OBJECT_IS_NULL);
        return;
    }

    if (target == nullptr) {
        dbg_error(program, "use_obj_on_obj", SCRIPT_ERROR_OBJECT_IS_NULL);
        return;
    }

    Script* script;
    int sid = scr_find_sid_from_program(program);
    if (scr_ptr(sid, &script) == -1) {
        // FIXME: Should be SCRIPT_ERROR_CANT_MATCH_PROGRAM_TO_SID.
        dbg_error(program, "use_obj_on_obj", SCRIPT_ERROR_OBJECT_IS_NULL);
        return;
    }

    Object* self = scr_find_obj_from_program(program);
    if (PID_TYPE(self->pid) == OBJ_TYPE_CRITTER) {
        action_use_an_item_on_object(self, target, item);
    } else {
        obj_use_item_on(self, target, item);
    }
}

// 0x451F2C
static void op_endgame_slideshow(Program* program)
{
    program->flags |= PROGRAM_FLAG_0x20;
    scripts_request_endgame_slideshow();
    program->flags &= ~PROGRAM_FLAG_0x20;
}

// 0x451F4C
static void op_move_obj_inven_to_obj(Program* program)
{
    Object* object2 = static_cast<Object*>(program->stackPopPointer());
    Object* object1 = static_cast<Object*>(program->stackPopPointer());

    if (object1 == nullptr) {
        dbg_error(program, "move_obj_inven_to_obj", SCRIPT_ERROR_OBJECT_IS_NULL);
        return;
    }

    if (object2 == nullptr) {
        dbg_error(program, "move_obj_inven_to_obj", SCRIPT_ERROR_OBJECT_IS_NULL);
        return;
    }

    Object* oldArmor = nullptr;
    Object* item2 = nullptr;
    if (object1 == obj_dude) {
        oldArmor = inven_worn(object1);
    } else {
        item2 = inven_right_hand(object1);
    }

    if (object1 != obj_dude && item2 != nullptr) {
        int flags = 0;
        if ((item2->flags & OBJECT_IN_LEFT_HAND) != 0) {
            flags |= OBJECT_IN_LEFT_HAND;
        }

        if ((item2->flags & OBJECT_IN_RIGHT_HAND) != 0) {
            flags |= OBJECT_IN_RIGHT_HAND;
        }

        correctFidForRemovedItem(object1, item2, flags);
    }

    item_move_all(object1, object2);

    if (object1 == obj_dude) {
        if (oldArmor != nullptr) {
            adjust_ac(obj_dude, oldArmor, nullptr);
        }

        proto_dude_update_gender();

        intface_update_items(true);
    }
}

// 0x45207C
static void op_endgame_movie(Program* program)
{
    program->flags |= PROGRAM_FLAG_0x20;
    endgame_movie();
    program->flags &= ~PROGRAM_FLAG_0x20;
}

// 0x45209C
static void op_obj_art_fid(Program* program)
{
    Object* object = static_cast<Object*>(program->stackPopPointer());

    int fid = 0;
    if (object != nullptr) {
        fid = object->fid;
    } else {
        dbg_error(program, "obj_art_fid", SCRIPT_ERROR_OBJECT_IS_NULL);
    }

    program->stackPushInteger(fid);
}

// 0x452108
static void op_art_anim(Program* program)
{
    int data = program->stackPopInteger();
    program->stackPushInteger((data & 0xFF0000) >> 16);
}

// 0x452160
static void op_party_member_obj(Program* program)
{
    int data = program->stackPopInteger();

    Object* object = partyMemberFindObjFromPid(data);
    program->stackPushPointer(object);
}

// 0x4521B4
static void op_rotation_to_tile(Program* program)
{
    int tile2 = program->stackPopInteger();
    int tile1 = program->stackPopInteger();

    int rotation = tile_dir(tile1, tile2);
    program->stackPushInteger(rotation);
}

// 0x452234
static void op_jam_lock(Program* program)
{
    Object* object = static_cast<Object*>(program->stackPopPointer());

    obj_jam_lock(object);
}

// 0x452274
static void op_gdialog_set_barter_mod(Program* program)
{
    int data = program->stackPopInteger();

    gdialogSetBarterMod(data);
}

// 0x4522B4
static void op_combat_difficulty(Program* program)
{
    int combatDifficulty;
    if (!game_config.getValue(GAME_CONFIG_PREFERENCES_KEY, GAME_CONFIG_COMBAT_DIFFICULTY_KEY, &combatDifficulty)) {
        combatDifficulty = 0;
    }

    program->stackPushInteger(combatDifficulty);
}

// 0x4522FC
static void op_obj_on_screen(Program* program)
{
    Object* object = static_cast<Object*>(program->stackPopPointer());

    int result = 0;

    if (object != nullptr) {
        if (map_elevation == object->elevation) {
            Rect objectRect;
            obj_bound(object, &objectRect);

            // CE: Original code checks if object intersects hardcoded 640x480
            // rectangle (i.e. without accounting for interface bar). Do the
            // same but with screen rectangle.
            if (objectRect.insideBound(scr_size, objectRect) == 0) {
                result = 1;
            }
        }
    } else {
        dbg_error(program, "obj_on_screen", SCRIPT_ERROR_OBJECT_IS_NULL);
    }

    program->stackPushInteger(result);
}

// 0x4523A8
static void op_critter_is_fleeing(Program* program)
{
    Object* obj = static_cast<Object*>(program->stackPopPointer());

    bool fleeing = false;
    if (obj != nullptr) {
        fleeing = (obj->data.critter.combat.maneuver & CRITTER_MANUEVER_FLEEING) != 0;
    } else {
        dbg_error(program, "critter_is_fleeing", SCRIPT_ERROR_OBJECT_IS_NULL);
    }

    program->stackPushInteger(fleeing ? 1 : 0);
}

// 0x452424
static void op_critter_set_flee_state(Program* program)
{
    int fleeing = program->stackPopInteger();
    Object* object = static_cast<Object*>(program->stackPopPointer());

    if (object != nullptr) {
        if (fleeing != 0) {
            object->data.critter.combat.maneuver |= CRITTER_MANUEVER_FLEEING;
        } else {
            object->data.critter.combat.maneuver &= ~CRITTER_MANUEVER_FLEEING;
        }
    } else {
        dbg_error(program, "critter_set_flee_state", SCRIPT_ERROR_OBJECT_IS_NULL);
    }
}

// 0x4524B0
static void op_terminate_combat(Program* program)
{
    if (isInCombat()) {
        game_user_wants_to_quit = 1;
    }
}

// 0x4524C4
static void op_debug_msg(Program* program)
{
    char* string = program->stackPopString();

    if (string != nullptr) {
        bool showScriptMessages = false;
        game_config.getBool(GAME_CONFIG_DEBUG_KEY, GAME_CONFIG_SHOW_SCRIPT_MESSAGES_KEY, &showScriptMessages);
        if (showScriptMessages) {
            debug_printf("\n");
            debug_printf(string);
        }
    }
}

// 0x452554
static void op_critter_stop_attacking(Program* program)
{
    Object* critter = static_cast<Object*>(program->stackPopPointer());
    if (critter == nullptr) {
        dbg_error(program, "critter_stop_attacking", SCRIPT_ERROR_OBJECT_IS_NULL);
        return;
    }

    critter->data.critter.combat.maneuver |= CRITTER_MANEUVER_DISENGAGING;
    critter->data.critter.combat.whoHitMe = nullptr;
}

// 0x4525B8
static void op_tile_contains_pid_obj(Program* program)
{
    int pid = program->stackPopInteger();
    int elevation = program->stackPopInteger();
    int tile = program->stackPopInteger();
    Object* found = nullptr;

    if (tile != -1) {
        Object* object = obj_find_first_at(elevation);
        while (object != nullptr) {
            if (object->tile == tile && object->pid == pid) {
                found = object;
                break;
            }
            object = obj_find_next_at();
        }
    }

    program->stackPushPointer(found);
}

// 0x452668
static void op_obj_name(Program* program)
{
    // 0x505504
    static char* strName = _aCritter;

    Object* obj = static_cast<Object*>(program->stackPopPointer());
    if (obj != nullptr) {
        strName = object_name(obj);
    } else {
        dbg_error(program, "obj_name", SCRIPT_ERROR_OBJECT_IS_NULL);
    }

    program->stackPushString(strName);
}

// 0x4526E8
static void op_get_pc_stat(Program* program)
{
    int data = program->stackPopInteger();
    program->stackPushInteger(stat_pc_get(data));
}

// 0x45273C
void intExtraClose()
{
}

// 0x452740
void initIntExtra()
{
    interpretAddFunc(0x80A1, op_give_exp_points);
    interpretAddFunc(0x80A2, op_scr_return);
    interpretAddFunc(0x80A3, op_play_sfx);
    interpretAddFunc(0x80A4, op_obj_name);
    interpretAddFunc(0x80A5, op_sfx_build_open_name);
    interpretAddFunc(0x80A6, op_get_pc_stat);
    interpretAddFunc(0x80A7, op_tile_contains_pid_obj);
    interpretAddFunc(0x80A8, op_set_map_start);
    interpretAddFunc(0x80A9, op_override_map_start);
    interpretAddFunc(0x80AA, op_has_skill);
    interpretAddFunc(0x80AB, op_using_skill);
    interpretAddFunc(0x80AC, op_roll_vs_skill);
    interpretAddFunc(0x80AD, op_skill_contest);
    interpretAddFunc(0x80AE, op_do_check);
    interpretAddFunc(0x80AF, op_is_success);
    interpretAddFunc(0x80B0, op_is_critical);
    interpretAddFunc(0x80B1, op_how_much);
    interpretAddFunc(0x80B2, op_reaction_roll);
    interpretAddFunc(0x80B3, op_reaction_influence);
    interpretAddFunc(0x80B4, op_random);
    interpretAddFunc(0x80B5, op_roll_dice);
    interpretAddFunc(0x80B6, op_move_to);
    interpretAddFunc(0x80B7, op_create_object_sid);
    interpretAddFunc(0x80B8, op_display_msg);
    interpretAddFunc(0x80B9, op_script_overrides);
    interpretAddFunc(0x80BA, op_obj_is_carrying_obj_pid);
    interpretAddFunc(0x80BB, op_tile_contains_obj_pid);
    interpretAddFunc(0x80BC, op_self_obj);
    interpretAddFunc(0x80BD, op_source_obj);
    interpretAddFunc(0x80BE, op_target_obj);
    interpretAddFunc(0x80BF, op_dude_obj);
    interpretAddFunc(0x80C0, op_obj_being_used_with);
    interpretAddFunc(0x80C1, op_local_var);
    interpretAddFunc(0x80C2, op_set_local_var);
    interpretAddFunc(0x80C3, op_map_var);
    interpretAddFunc(0x80C4, op_set_map_var);
    interpretAddFunc(0x80C5, op_global_var);
    interpretAddFunc(0x80C6, op_set_global_var);
    interpretAddFunc(0x80C7, op_script_action);
    interpretAddFunc(0x80C8, op_obj_type);
    interpretAddFunc(0x80C9, op_obj_item_subtype);
    interpretAddFunc(0x80CA, op_get_critter_stat);
    interpretAddFunc(0x80CB, op_set_critter_stat);
    interpretAddFunc(0x80CC, op_animate_stand_obj);
    interpretAddFunc(0x80CD, op_animate_stand_reverse_obj);
    interpretAddFunc(0x80CE, op_animate_move_obj_to_tile);
    interpretAddFunc(0x80CF, op_animate_jump);
    interpretAddFunc(0x80D0, op_attack);
    interpretAddFunc(0x80D1, op_make_daytime);
    interpretAddFunc(0x80D2, op_tile_distance);
    interpretAddFunc(0x80D3, op_tile_distance_objs);
    interpretAddFunc(0x80D4, op_tile_num);
    interpretAddFunc(0x80D5, op_tile_num_in_direction);
    interpretAddFunc(0x80D6, op_pickup_obj);
    interpretAddFunc(0x80D7, op_drop_obj);
    interpretAddFunc(0x80D8, op_add_obj_to_inven);
    interpretAddFunc(0x80D9, op_rm_obj_from_inven);
    interpretAddFunc(0x80DA, op_wield_obj_critter);
    interpretAddFunc(0x80DB, op_use_obj);
    interpretAddFunc(0x80DC, op_obj_can_see_obj);
    interpretAddFunc(0x80DD, op_attack);
    interpretAddFunc(0x80DE, op_start_gdialog);
    interpretAddFunc(0x80DF, op_end_dialogue);
    interpretAddFunc(0x80E0, op_dialogue_reaction);
    interpretAddFunc(0x80E1, op_turn_off_objs_in_area);
    interpretAddFunc(0x80E2, op_turn_on_objs_in_area);
    interpretAddFunc(0x80E3, op_set_obj_visibility);
    interpretAddFunc(0x80E4, op_load_map);
    interpretAddFunc(0x80E5, op_barter_offer);
    interpretAddFunc(0x80E6, op_barter_asking);
    interpretAddFunc(0x80E7, op_anim_busy);
    interpretAddFunc(0x80E8, op_critter_heal);
    interpretAddFunc(0x80E9, op_set_light_level);
    interpretAddFunc(0x80EA, op_game_time);
    interpretAddFunc(0x80EB, op_game_time_in_seconds);
    interpretAddFunc(0x80EC, op_elevation);
    interpretAddFunc(0x80ED, op_kill_critter);
    interpretAddFunc(0x80EE, op_kill_critter_type);
    interpretAddFunc(0x80EF, op_critter_damage);
    interpretAddFunc(0x80F0, op_add_timer_event);
    interpretAddFunc(0x80F1, op_rm_timer_event);
    interpretAddFunc(0x80F2, op_game_ticks);
    interpretAddFunc(0x80F3, op_has_trait);
    interpretAddFunc(0x80F4, op_destroy_object);
    interpretAddFunc(0x80F5, op_obj_can_hear_obj);
    interpretAddFunc(0x80F6, op_game_time_hour);
    interpretAddFunc(0x80F7, op_fixed_param);
    interpretAddFunc(0x80F8, op_tile_is_visible);
    interpretAddFunc(0x80F9, op_dialogue_system_enter);
    interpretAddFunc(0x80FA, op_action_being_used);
    interpretAddFunc(0x80FB, op_critter_state);
    interpretAddFunc(0x80FC, op_game_time_advance);
    interpretAddFunc(0x80FD, op_radiation_inc);
    interpretAddFunc(0x80FE, op_radiation_dec);
    interpretAddFunc(0x80FF, op_critter_attempt_placement);
    interpretAddFunc(0x8100, op_obj_pid);
    interpretAddFunc(0x8101, op_cur_map_index);
    interpretAddFunc(0x8102, op_critter_add_trait);
    interpretAddFunc(0x8103, op_critter_rm_trait);
    interpretAddFunc(0x8104, op_proto_data);
    interpretAddFunc(0x8105, op_message_str);
    interpretAddFunc(0x8106, op_critter_inven_obj);
    interpretAddFunc(0x8107, op_obj_set_light_level);
    interpretAddFunc(0x8108, op_world_map);
    interpretAddFunc(0x8109, op_town_map);
    interpretAddFunc(0x810A, op_float_msg);
    interpretAddFunc(0x810B, op_metarule);
    interpretAddFunc(0x810C, op_anim);
    interpretAddFunc(0x810D, op_obj_carrying_pid_obj);
    interpretAddFunc(0x810E, op_reg_anim_func);
    interpretAddFunc(0x810F, op_reg_anim_animate);
    interpretAddFunc(0x8110, op_reg_anim_animate_reverse);
    interpretAddFunc(0x8111, op_reg_anim_obj_move_to_obj);
    interpretAddFunc(0x8112, op_reg_anim_obj_run_to_obj);
    interpretAddFunc(0x8113, op_reg_anim_obj_move_to_tile);
    interpretAddFunc(0x8114, op_reg_anim_obj_run_to_tile);
    interpretAddFunc(0x8115, op_play_gmovie);
    interpretAddFunc(0x8116, op_add_mult_objs_to_inven);
    interpretAddFunc(0x8117, op_rm_mult_objs_from_inven);
    interpretAddFunc(0x8118, op_get_month);
    interpretAddFunc(0x8119, op_get_day);
    interpretAddFunc(0x811A, op_explosion);
    interpretAddFunc(0x811B, op_days_since_visited);
    interpretAddFunc(0x811C, op_gsay_start);
    interpretAddFunc(0x811D, op_gsay_end);
    interpretAddFunc(0x811E, op_gsay_reply);
    interpretAddFunc(0x811F, op_gsay_option);
    interpretAddFunc(0x8120, op_gsay_message);
    interpretAddFunc(0x8121, op_giq_option);
    interpretAddFunc(0x8122, op_poison);
    interpretAddFunc(0x8123, op_get_poison);
    interpretAddFunc(0x8124, op_party_add);
    interpretAddFunc(0x8125, op_party_remove);
    interpretAddFunc(0x8126, op_reg_anim_animate_forever);
    interpretAddFunc(0x8127, op_critter_injure);
    interpretAddFunc(0x8128, op_combat_is_initialized);
    interpretAddFunc(0x8129, op_gdialog_barter);
    interpretAddFunc(0x812A, op_difficulty_level);
    interpretAddFunc(0x812B, op_running_burning_guy);
    interpretAddFunc(0x812C, op_inven_unwield);
    interpretAddFunc(0x812D, op_obj_is_locked);
    interpretAddFunc(0x812E, op_obj_lock);
    interpretAddFunc(0x812F, op_obj_unlock);
    interpretAddFunc(0x8131, op_obj_open);
    interpretAddFunc(0x8130, op_obj_is_open);
    interpretAddFunc(0x8132, op_obj_close);
    interpretAddFunc(0x8133, op_game_ui_disable);
    interpretAddFunc(0x8134, op_game_ui_enable);
    interpretAddFunc(0x8135, op_game_ui_is_disabled);
    interpretAddFunc(0x8136, op_gfade_out);
    interpretAddFunc(0x8137, op_gfade_in);
    interpretAddFunc(0x8138, op_item_caps_total);
    interpretAddFunc(0x8139, op_item_caps_adjust);
    interpretAddFunc(0x813A, op_anim_action_frame);
    interpretAddFunc(0x813B, op_reg_anim_play_sfx);
    interpretAddFunc(0x813C, op_critter_mod_skill);
    interpretAddFunc(0x813D, op_sfx_build_char_name);
    interpretAddFunc(0x813E, op_sfx_build_ambient_name);
    interpretAddFunc(0x813F, op_sfx_build_interface_name);
    interpretAddFunc(0x8140, op_sfx_build_item_name);
    interpretAddFunc(0x8141, op_sfx_build_weapon_name);
    interpretAddFunc(0x8142, op_sfx_build_scenery_name);
    interpretAddFunc(0x8143, op_attack_setup);
    interpretAddFunc(0x8144, op_destroy_mult_objs);
    interpretAddFunc(0x8145, op_use_obj_on_obj);
    interpretAddFunc(0x8146, op_endgame_slideshow);
    interpretAddFunc(0x8147, op_move_obj_inven_to_obj);
    interpretAddFunc(0x8148, op_endgame_movie);
    interpretAddFunc(0x8149, op_obj_art_fid);
    interpretAddFunc(0x814A, op_art_anim);
    interpretAddFunc(0x814B, op_party_member_obj);
    interpretAddFunc(0x814C, op_rotation_to_tile);
    interpretAddFunc(0x814D, op_jam_lock);
    interpretAddFunc(0x814E, op_gdialog_set_barter_mod);
    interpretAddFunc(0x814F, op_combat_difficulty);
    interpretAddFunc(0x8150, op_obj_on_screen);
    interpretAddFunc(0x8151, op_critter_is_fleeing);
    interpretAddFunc(0x8152, op_critter_set_flee_state);
    interpretAddFunc(0x8153, op_terminate_combat);
    interpretAddFunc(0x8154, op_debug_msg);
    interpretAddFunc(0x8155, op_critter_stop_attacking);
}

// 0x4531E0
void updateIntExtra()
{
}

// 0x4531E0
void intExtraRemoveProgramReferences(Program* program)
{
}

} // namespace fallout

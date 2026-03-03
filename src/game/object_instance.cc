#include "game/object_instance.h"

#include <cassert>

#include "game/anim.h"
#include "game/map.h"
#include "game/object.h"
#include "game/object_actions.h"
#include "game/proto.h"
#include "game/queue.h"
#include "game/scripts.h"
#include "game/tile.h"
#include "plib/gnw/rect.h"

namespace fallout {

// =========================================================================
// Construction
// =========================================================================

ObjectInstance::ObjectInstance(Object* obj)
    : obj_(obj)
{
}

// =========================================================================
// Script ID management
// =========================================================================

// 0x489F60
int ObjectInstance::sid(int* sidPtr)
{
    *sidPtr = obj_->sid;
    if (*sidPtr == -1) {
        return -1;
    }
    return 0;
}

// 0x489F74
int ObjectInstance::newSid(int* sidPtr)
{
    *sidPtr = -1;

    Proto* proto;
    if (proto_ptr(obj_->pid, &proto) == -1) {
        return -1;
    }

    int scriptId;
    int objectType = PID_TYPE(obj_->pid);
    if (objectType < OBJ_TYPE_TILE) {
        scriptId = proto->sid;
    } else if (objectType == OBJ_TYPE_TILE) {
        scriptId = proto->tile.sid;
    } else if (objectType == OBJ_TYPE_MISC) {
        scriptId = -1;
    } else {
        assert(false && "Should be unreachable");
    }

    if (scriptId == -1) {
        return -1;
    }

    int scriptType = SID_TYPE(scriptId);
    if (scr_new(sidPtr, scriptType) == -1) {
        return -1;
    }

    Script* script;
    if (scr_ptr(*sidPtr, &script) == -1) {
        return -1;
    }

    script->scr_script_idx = scriptId & 0xFFFFFF;

    if (objectType == OBJ_TYPE_CRITTER) {
        obj_->field_80 = script->scr_script_idx;
    }

    if (scriptType == SCRIPT_TYPE_SPATIAL) {
        script->sp.built_tile = builtTileCreate(obj_->tile, obj_->elevation);
        script->sp.radius = 3;
    }

    if (obj_->id == -1) {
        obj_->id = new_obj_id();
    }

    script->scr_oid = obj_->id;
    script->owner = obj_;

    scr_find_str_run_info(scriptId & 0xFFFFFF, &(script->run_info_flags), *sidPtr);

    return 0;
}

// 0x48A080
int ObjectInstance::newSidInst(int scriptType, int a3)
{
    if (a3 == -1) {
        return -1;
    }

    int newSidValue;
    if (scr_new(&newSidValue, scriptType) == -1) {
        return -1;
    }

    Script* script;
    if (scr_ptr(newSidValue, &script) == -1) {
        return -1;
    }

    script->scr_script_idx = a3;
    if (scriptType == SCRIPT_TYPE_SPATIAL) {
        script->sp.built_tile = builtTileCreate(obj_->tile, obj_->elevation);
        script->sp.radius = 3;
    }

    obj_->sid = newSidValue;

    obj_->id = new_obj_id();
    script->scr_oid = obj_->id;

    script->owner = obj_;

    scr_find_str_run_info(a3 & 0xFFFFFF, &(script->run_info_flags), newSidValue);

    if (PID_TYPE(obj_->pid) == OBJ_TYPE_CRITTER) {
        obj_->field_80 = script->scr_script_idx;
    }

    return 0;
}

// =========================================================================
// Destroy
// =========================================================================

// 0x48AD38
int ObjectInstance::destroy()
{
    if (obj_ == nullptr) {
        return -1;
    }

    int elev;
    Object* owner = obj_->owner;
    if (owner != nullptr) {
        ObjectActions(owner).removeFromInven(obj_);
    } else {
        elev = obj_->elevation;
    }

    queue_remove(obj_);

    Rect rect;
    obj_erase_object(obj_, &rect);

    if (owner == nullptr) {
        tile_refresh_rect(&rect, elev);
    }

    return 0;
}

// =========================================================================
// Placement
// =========================================================================

// 0x48C1A8
int ObjectInstance::attemptPlacement(int tile, int elevation, int a4)
{
    if (tile == -1) {
        return -1;
    }

    int newTile = tile;
    if (obj_blocking_at(nullptr, tile, elevation) != nullptr) {
        int v6 = a4;
        if (a4 < 1) {
            v6 = 1;
        }

        while (v6 < 7) {
            for (int rotation = 0; rotation < ROTATION_COUNT; rotation++) {
                newTile = tile_num_in_direction(tile, rotation, v6);
                if (obj_blocking_at(nullptr, newTile, elevation) == nullptr && v6 > 1 && make_path(obj_dude, obj_dude->tile, newTile, nullptr, 0) != 0) {
                    break;
                }
            }
            v6++;
        }

        if (a4 != 1 && v6 > a4 + 2) {
            for (int rotation = 0; rotation < ROTATION_COUNT; rotation++) {
                int candidate = tile_num_in_direction(tile, rotation, 1);
                if (obj_blocking_at(nullptr, candidate, elevation) == nullptr) {
                    newTile = candidate;
                    break;
                }
            }
        }
    }

    obj_->flags &= ~OBJECT_HIDDEN;

    Rect temp;
    if (obj_move_to_tile(obj_, newTile, elevation, &temp) != -1) {
        if (elevation == map_elevation) {
            tile_refresh_rect(&temp, elevation);
        }
    }

    return 0;
}

} // namespace fallout

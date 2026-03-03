#include "game/protinst.h"

#include "game/object.h"
#include "game/object_actions.h"
#include "game/object_instance.h"
#include "game/object_lock_state.h"

namespace fallout {

// =========================================================================
// ObjectInstance wrappers
// =========================================================================

int obj_sid(Object* object, int* sidPtr)
{
    return ObjectInstance(object).sid(sidPtr);
}

int obj_new_sid(Object* object, int* sidPtr)
{
    return ObjectInstance(object).newSid(sidPtr);
}

int obj_new_sid_inst(Object* obj, int a2, int a3)
{
    return ObjectInstance(obj).newSidInst(a2, a3);
}

int obj_destroy(Object* obj)
{
    return ObjectInstance(obj).destroy();
}

int obj_attempt_placement(Object* obj, int tile, int elevation, int a4)
{
    return ObjectInstance(obj).attemptPlacement(tile, elevation, a4);
}

// =========================================================================
// ObjectActions wrappers
// =========================================================================

int obj_look_at(Object* a1, Object* a2)
{
    return ObjectActions(a1).lookAt(a2);
}

int obj_look_at_func(Object* a1, Object* a2, void (*a3)(char* string))
{
    return ObjectActions(a1).lookAtFunc(a2, a3);
}

int obj_examine(Object* a1, Object* a2)
{
    return ObjectActions(a1).examine(a2);
}

int obj_examine_func(Object* critter, Object* target, void (*fn)(char* string))
{
    return ObjectActions(critter).examineFunc(target, fn);
}

int obj_pickup(Object* critter, Object* item)
{
    return ObjectActions(critter).pickup(item);
}

int obj_remove_from_inven(Object* critter, Object* item)
{
    return ObjectActions(critter).removeFromInven(item);
}

int obj_drop(Object* a1, Object* a2)
{
    return ObjectActions(a1).drop(a2);
}

int obj_use_radio(Object* item_obj)
{
    return ObjectActions(obj_dude).useRadio(item_obj);
}

int protinst_use_item(Object* a1, Object* a2)
{
    return ObjectActions(a1).protinstUseItem(a2);
}

int obj_use_item(Object* a1, Object* a2)
{
    return ObjectActions(a1).useItem(a2);
}

int protinst_use_item_on(Object* a1, Object* a2, Object* item)
{
    return ObjectActions(a1).protinstUseItemOn(a2, item);
}

int obj_use_item_on(Object* a1, Object* a2, Object* a3)
{
    return ObjectActions(a1).useItemOn(a2, a3);
}

int check_scenery_ap_cost(Object* obj, Object* a2)
{
    return ObjectActions(obj).checkSceneryApCost(a2);
}

int obj_use(Object* a1, Object* a2)
{
    return ObjectActions(a1).use(a2);
}

int obj_use_door(Object* a1, Object* a2, int a3)
{
    return ObjectActions(a1).useDoor(a2, a3);
}

int obj_use_container(Object* critter, Object* item)
{
    return ObjectActions(critter).useContainer(item);
}

int obj_use_skill_on(Object* a1, Object* a2, int skill)
{
    return ObjectActions(a1).useSkillOn(a2, skill);
}

// =========================================================================
// ObjectLockState wrappers
// =========================================================================

bool obj_is_a_portal(Object* obj)
{
    return ObjectLockState(obj).isPortal();
}

bool obj_is_lockable(Object* obj)
{
    return ObjectLockState(obj).isLockable();
}

bool obj_is_locked(Object* obj)
{
    return ObjectLockState(obj).isLocked();
}

int obj_lock(Object* obj)
{
    return ObjectLockState(obj).lock();
}

int obj_unlock(Object* obj)
{
    return ObjectLockState(obj).unlock();
}

bool obj_is_openable(Object* obj)
{
    return ObjectLockState(obj).isOpenable();
}

int obj_is_open(Object* obj)
{
    return ObjectLockState(obj).isOpen();
}

int obj_toggle_open(Object* obj)
{
    return ObjectLockState(obj).toggleOpen();
}

int obj_open(Object* obj)
{
    return ObjectLockState(obj).open();
}

int obj_close(Object* obj)
{
    return ObjectLockState(obj).close();
}

bool obj_lock_is_jammed(Object* obj)
{
    return ObjectLockState(obj).isJammed();
}

int obj_jam_lock(Object* obj)
{
    return ObjectLockState(obj).jam();
}

int obj_unjam_lock(Object* obj)
{
    return ObjectLockState(obj).unjam();
}

int obj_unjam_all_locks()
{
    return ObjectLockState::unjamAll();
}

} // namespace fallout

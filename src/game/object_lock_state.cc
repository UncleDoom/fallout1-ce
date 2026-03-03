#include "game/object_lock_state.h"

#include "game/anim.h"
#include "game/art.h"
#include "game/gsound.h"
#include "game/map.h"
#include "game/message_helpers.h"
#include "game/object.h"
#include "game/proto.h"
#include "game/proto_types.h"
#include "game/tile.h"
#include "plib/gnw/rect.h"

namespace fallout {

// -------------------------------------------------------------------------
// Construction
// -------------------------------------------------------------------------

ObjectLockState::ObjectLockState(Object* obj)
    : obj_(obj)
{
}

// -------------------------------------------------------------------------
// Private helpers
// -------------------------------------------------------------------------

int* ObjectLockState::lockFlagsPtr()
{
    if (obj_ == nullptr) return nullptr;
    switch (PID_TYPE(obj_->pid)) {
    case OBJ_TYPE_ITEM:
        return &obj_->data.flags;
    case OBJ_TYPE_SCENERY:
        return reinterpret_cast<int*>(&obj_->data.scenery.door.openFlags);
    default:
        return nullptr;
    }
}

const int* ObjectLockState::lockFlagsPtr() const
{
    if (obj_ == nullptr) return nullptr;
    switch (PID_TYPE(obj_->pid)) {
    case OBJ_TYPE_ITEM:
        return &obj_->data.flags;
    case OBJ_TYPE_SCENERY:
        return reinterpret_cast<const int*>(&obj_->data.scenery.door.openFlags);
    default:
        return nullptr;
    }
}

bool ObjectLockState::isContainerOrDoor() const
{
    if (obj_ == nullptr) return false;

    Proto* proto;
    if (proto_ptr(obj_->pid, &proto) == -1) return false;

    switch (PID_TYPE(obj_->pid)) {
    case OBJ_TYPE_ITEM:
        return proto->item.type == ITEM_TYPE_CONTAINER;
    case OBJ_TYPE_SCENERY:
        return proto->scenery.type == SCENERY_TYPE_DOOR;
    default:
        return false;
    }
}

// -------------------------------------------------------------------------
// Rebuild light helper (was static rebuild_all_light)
// -------------------------------------------------------------------------

static int rebuildAllLight()
{
    obj_rebuild_all_light();
    tile_refresh_display();
    return 0;
}

// -------------------------------------------------------------------------
// Portal
// -------------------------------------------------------------------------

bool ObjectLockState::isPortal() const
{
    if (obj_ == nullptr) return false;

    Proto* proto;
    if (proto_ptr(obj_->pid, &proto) == -1) return false;

    return proto->scenery.type == SCENERY_TYPE_DOOR;
}

// -------------------------------------------------------------------------
// Lockable / Openable (shared via isContainerOrDoor)
// -------------------------------------------------------------------------

bool ObjectLockState::isLockable() const
{
    return isContainerOrDoor();
}

bool ObjectLockState::isOpenable() const
{
    return isContainerOrDoor();
}

// -------------------------------------------------------------------------
// Lock / Unlock
// -------------------------------------------------------------------------

bool ObjectLockState::isLocked() const
{
    const int* flags = lockFlagsPtr();
    if (flags == nullptr) return false;
    return (*flags & OBJ_LOCKED) != 0;
}

int ObjectLockState::lock()
{
    int* flags = lockFlagsPtr();
    if (flags == nullptr) return -1;
    *flags |= OBJ_LOCKED;
    return 0;
}

int ObjectLockState::unlock()
{
    int* flags = lockFlagsPtr();
    if (flags == nullptr) return -1;
    *flags &= ~OBJ_LOCKED;
    return 0;
}

// -------------------------------------------------------------------------
// Open / Close
// -------------------------------------------------------------------------

int ObjectLockState::isOpen() const
{
    return obj_->frame != 0;
}

int ObjectLockState::toggleOpen()
{
    if (obj_ == nullptr) return -1;
    if (!isOpenable()) return -1;
    if (isLocked()) return -1;

    unjam();

    register_begin(ANIMATION_REQUEST_RESERVED);

    if (obj_->frame != 0) {
        register_object_must_call(obj_, obj_, reinterpret_cast<AnimationCallback*>(setDoorStateClosed), -1);
        const char* sfx = gsnd_build_open_sfx_name(obj_, SCENERY_SOUND_EFFECT_CLOSED);
        register_object_play_sfx(obj_, sfx, -1);
        register_object_animate_reverse(obj_, ANIM_STAND, 0);
    } else {
        register_object_must_call(obj_, obj_, reinterpret_cast<AnimationCallback*>(setDoorStateOpen), -1);
        const char* sfx = gsnd_build_open_sfx_name(obj_, SCENERY_SOUND_EFFECT_OPEN);
        register_object_play_sfx(obj_, sfx, -1);
        register_object_animate(obj_, ANIM_STAND, 0);
    }

    register_object_must_call(obj_, obj_, reinterpret_cast<AnimationCallback*>(checkDoorState), -1);
    register_end();

    return 0;
}

int ObjectLockState::open()
{
    if (obj_->frame == 0) {
        toggleOpen();
    }
    return 0;
}

int ObjectLockState::close()
{
    if (obj_->frame != 0) {
        toggleOpen();
    }
    return 0;
}

// -------------------------------------------------------------------------
// Jam / Unjam
// -------------------------------------------------------------------------

bool ObjectLockState::isJammed() const
{
    if (!isLockable()) return false;

    const int* flags = lockFlagsPtr();
    if (flags == nullptr) return false;
    return (*flags & OBJ_JAMMED) != 0;
}

int ObjectLockState::jam()
{
    if (!isLockable()) return -1;
    int* flags = lockFlagsPtr();
    if (flags == nullptr) return -1;
    *flags |= OBJ_JAMMED;
    return 0;
}

int ObjectLockState::unjam()
{
    if (!isLockable()) return -1;
    int* flags = lockFlagsPtr();
    if (flags == nullptr) return -1;
    *flags &= ~OBJ_JAMMED;
    return 0;
}

// -------------------------------------------------------------------------
// Unjam All (static)
// -------------------------------------------------------------------------

int ObjectLockState::unjamAll()
{
    Object* obj = obj_find_first();
    while (obj != nullptr) {
        ObjectLockState(obj).unjam();
        obj = obj_find_next();
    }
    return 0;
}

// -------------------------------------------------------------------------
// Animation callbacks (static — C function pointer compatible)
// -------------------------------------------------------------------------

// 0x48B7FC
int ObjectLockState::setDoorStateOpen(Object* a1, Object* /*a2*/)
{
    a1->data.scenery.door.openFlags |= 0x01;
    return 0;
}

// 0x48B80C
int ObjectLockState::setDoorStateClosed(Object* a1, Object* /*a2*/)
{
    a1->data.scenery.door.openFlags &= ~0x01;
    return 0;
}

// 0x48B81C
int ObjectLockState::checkDoorState(Object* a1, Object* /*a2*/)
{
    if ((a1->data.scenery.door.openFlags & 0x01) == 0) {
        a1->flags &= ~OBJECT_OPEN_DOOR;

        rebuildAllLight();

        if (a1->frame == 0) {
            return 0;
        }

        CacheEntry* artHandle;
        Art* art = art_ptr_lock(a1->fid, &artHandle);
        if (art == nullptr) {
            return -1;
        }

        Rect dirty;
        Rect temp;
        obj_bound(a1, &dirty);

        for (int frame = a1->frame - 1; frame >= 0; frame--) {
            int x, y;
            art->frameHot(frame, a1->rotation, &x, &y);
            obj_offset(a1, -x, -y, &temp);
        }

        obj_set_frame(a1, 0, &temp);
        dirty.minBound(temp);
        tile_refresh_rect(&dirty, map_elevation);

        art_ptr_unlock(artHandle);
        return 0;
    } else {
        a1->flags |= OBJECT_OPEN_DOOR;

        rebuildAllLight();

        CacheEntry* artHandle;
        Art* art = art_ptr_lock(a1->fid, &artHandle);
        if (art == nullptr) {
            return -1;
        }

        int frameCount = art->maxFrame();
        if (a1->frame == frameCount - 1) {
            art_ptr_unlock(artHandle);
            return 0;
        }

        Rect dirty;
        Rect temp;
        obj_bound(a1, &dirty);

        for (int frame = a1->frame + 1; frame < frameCount; frame++) {
            int x, y;
            art->frameHot(frame, a1->rotation, &x, &y);
            obj_offset(a1, x, y, &temp);
        }

        obj_set_frame(a1, frameCount - 1, &temp);
        dirty.minBound(temp);
        tile_refresh_rect(&dirty, map_elevation);

        art_ptr_unlock(artHandle);
        return 0;
    }
}

} // namespace fallout

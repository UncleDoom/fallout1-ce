#pragma once


#include "game/object_types.h"

namespace fallout {

// Manages lock, open, jam, and portal state of scene objects (doors) and item
// containers. Wraps a single Object*.
class ObjectLockState {
public:
    explicit ObjectLockState(Object* obj);

    bool isPortal() const;
    bool isLockable() const;
    bool isLocked() const;
    int lock();
    int unlock();
    bool isOpenable() const;
    int isOpen() const;
    int toggleOpen();
    int open();
    int close();
    bool isJammed() const;
    int jam();
    int unjam();

    static int unjamAll();

    // Animation callbacks — must be static (plain C function pointers required
    // by AnimationCallback typedef).
    static int setDoorStateOpen(Object* a1, Object* a2);
    static int setDoorStateClosed(Object* a1, Object* a2);
    static int checkDoorState(Object* a1, Object* a2);

private:
    // Returns a pointer to the flags word that stores lock/jam bits for this
    // object, or nullptr if the object type doesn't support it.
    int* lockFlagsPtr();
    const int* lockFlagsPtr() const;

    // Shared check: fetches proto and returns true if the object is a
    // container (item) or door (scenery).
    bool isContainerOrDoor() const;

    Object* obj_;
};

} // namespace fallout

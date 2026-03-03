#pragma once


#include <cstddef>

#include "game/object_types.h"

namespace fallout {

// Wraps the actor (critter) Object* and provides interaction methods that
// operate on target objects: look, examine, pickup, drop, use, etc.
class ObjectActions {
public:
    explicit ObjectActions(Object* actor);

    // Look / Examine
    int lookAt(Object* target);
    int lookAtFunc(Object* target, void (*fn)(char* string));
    int examine(Object* target);
    int examineFunc(Object* target, void (*fn)(char* string));

    // Inventory
    int pickup(Object* item);
    int removeFromInven(Object* item);
    int drop(Object* item);

    // Item usage
    int useItem(Object* item);
    int useItemOn(Object* target, Object* item);

    // Scenery interaction
    int checkSceneryApCost(Object* target);
    int use(Object* target);
    int useDoor(Object* target, int a3);
    int useContainer(Object* target);
    int useSkillOn(Object* target, int skill);

    // Exposing the internal-use functions that are called externally by
    // inventry.cc. They are public for backward compatibility but are
    // semantically "internal".
    int protinstUseItem(Object* item);
    int protinstUseItemOn(Object* target, Object* item);

    // Radio usage (called externally via obj_use_radio)
    int useRadio(Object* item);

private:
    // Internal item-use helpers
    int useBook(Object* item);
    int useFlare(Object* item);
    int useExplosive(Object* item);
    int defaultUseItem(Object* target, Object* item);

    // Examine sub-helpers
    void examineWithAwareness(Object* target, char* buf, size_t bufSize, void (*fn)(char*));
    void examineHealthStatus(Object* target, char* buf, size_t bufSize);
    void appendCrippledStatus(Object* target, char* buf, size_t bufSize);
    void examineWeaponAmmo(Object* target, char* buf, size_t bufSize, void (*fn)(char*));

    Object* actor_;
};

} // namespace fallout

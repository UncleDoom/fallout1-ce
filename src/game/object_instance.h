#pragma once


#include "game/object_types.h"

namespace fallout {

// Manages script-ID assignment, destruction, and placement for a single
// Object*.
class ObjectInstance {
public:
    explicit ObjectInstance(Object* obj);

    int sid(int* sidPtr);
    int newSid(int* sidPtr);
    int newSidInst(int scriptType, int a3);
    int destroy();
    int attemptPlacement(int tile, int elevation, int a4);

private:
    Object* obj_;
};

} // namespace fallout

#pragma once


#include "game/object_types.h"

namespace fallout {

enum class NpcReaction : int {
    NPC_REACTION_BAD,
    NPC_REACTION_NEUTRAL,
    NPC_REACTION_GOOD,
};

int reaction_set(Object* critter, int value);
int level_to_reaction();
NpcReaction reaction_to_level_internal(int sid, int reaction);
NpcReaction reaction_to_level(int reaction);
int reaction_roll(int a1, int a2, int a3);
int reaction_influence(int a1, int a2, int a3);
int reaction_get(Object* critter);

} // namespace fallout

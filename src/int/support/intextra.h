#pragma once


#include "game/object_types.h"
#include "int/intrpret.h"

namespace fallout {

enum class ScriptError : int {
    NotImplemented = 0,
    ObjectIsNull = 1,
    CantMatchProgramToSid = 2,
    Follows = 3,
    Count = 4,
};

inline constexpr int SCRIPT_ERROR_NOT_IMPLEMENTED = static_cast<int>(ScriptError::NotImplemented);
inline constexpr int SCRIPT_ERROR_OBJECT_IS_NULL = static_cast<int>(ScriptError::ObjectIsNull);
inline constexpr int SCRIPT_ERROR_CANT_MATCH_PROGRAM_TO_SID = static_cast<int>(ScriptError::CantMatchProgramToSid);
inline constexpr int SCRIPT_ERROR_FOLLOWS = static_cast<int>(ScriptError::Follows);
inline constexpr int SCRIPT_ERROR_COUNT = static_cast<int>(ScriptError::Count);

void dbg_error(Program* program, const char* name, int error);
int correctDeath(Object* critter, int anim, bool a3);
void intExtraClose();
void initIntExtra();
void updateIntExtra();
void intExtraRemoveProgramReferences(Program* program);

} // namespace fallout

#pragma once


#include "plib/db/db.h"

namespace fallout {

enum class Roll : int {
    CriticalFailure = 0,
    Failure = 1,
    Success = 2,
    CriticalSuccess = 3,
};

inline constexpr int ROLL_CRITICAL_FAILURE = static_cast<int>(Roll::CriticalFailure);
inline constexpr int ROLL_FAILURE = static_cast<int>(Roll::Failure);
inline constexpr int ROLL_SUCCESS = static_cast<int>(Roll::Success);
inline constexpr int ROLL_CRITICAL_SUCCESS = static_cast<int>(Roll::CriticalSuccess);

void roll_init();
int roll_reset();
int roll_exit();
int roll_save(DB_FILE* stream);
int roll_load(DB_FILE* stream);
int roll_check(int difficulty, int criticalSuccessModifier, int* howMuchPtr);
int roll_check_critical(int delta, int criticalSuccessModifier);
int roll_random(int min, int max);
void roll_set_seed(int seed);

} // namespace fallout

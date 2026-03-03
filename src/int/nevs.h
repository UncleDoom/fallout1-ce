#pragma once


#include "int/intrpret.h"

namespace fallout {

using NevsCallback = void(const char* name);

enum class NevsType : int {
    Event = 0,
    Handler = 1,
};

inline constexpr int NEVS_TYPE_EVENT = static_cast<int>(NevsType::Event);
inline constexpr int NEVS_TYPE_HANDLER = static_cast<int>(NevsType::Handler);

void nevs_close();
void nevs_initonce();
int nevs_addevent(const char* name, Program* program, int proc, int type);
int nevs_addCevent(const char* name, NevsCallback* callback, int type);
int nevs_clearevent(const char* name);
int nevs_signal(const char* name);
void nevs_update();

} // namespace fallout

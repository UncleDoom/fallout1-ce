#pragma once


#include "game/art.h"
#include "game/message.h"
#include "plib/db/db.h"
#include "plib/gnw/rect.h"

namespace fallout {

enum class PipboyOpenIntent : int {
    Unspecified = 0,
    Rest = 1,
};

inline constexpr int PIPBOY_OPEN_INTENT_UNSPECIFIED = static_cast<int>(PipboyOpenIntent::Unspecified);
inline constexpr int PIPBOY_OPEN_INTENT_REST = static_cast<int>(PipboyOpenIntent::Rest);

using PipboyRenderProc = void(int a1);

int pipboy(int intent);
void pip_init();
int save_pipboy(DB_FILE* stream);
int load_pipboy(DB_FILE* stream);

} // namespace fallout

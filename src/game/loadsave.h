#pragma once


#include "game/art.h"
#include "game/message.h"
#include "plib/db/db.h"
#include "plib/gnw/rect.h"

namespace fallout {

enum class LoadSaveMode : int {
    // Special case - loading game from main menu.
    FromMainMenu = 0,

    // Normal (full-screen) save/load screen.
    Normal = 1,

    // Quick load/save.
    Quick = 2,
};

inline constexpr int LOAD_SAVE_MODE_FROM_MAIN_MENU = static_cast<int>(LoadSaveMode::FromMainMenu);
inline constexpr int LOAD_SAVE_MODE_NORMAL = static_cast<int>(LoadSaveMode::Normal);
inline constexpr int LOAD_SAVE_MODE_QUICK = static_cast<int>(LoadSaveMode::Quick);

void InitLoadSave();
void ResetLoadSave();
int SaveGame(int mode);
int LoadGame(int mode);
int isLoadingGame();
void KillOldMaps();
int MapDirErase(const char* path, const char* a2);
int MapDirEraseFile(const char* a1, const char* a2);

} // namespace fallout

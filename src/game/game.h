#pragma once


#include "game/game_vars.h"
#include "game/message.h"
#include "plib/db/db.h"

namespace fallout {

enum class GameState : int {
    State0 = 0,
    State1 = 1,
    State2 = 2,
    State3 = 3,
    State4 = 4,
    State5 = 5,
};

inline constexpr int GAME_STATE_0 = static_cast<int>(GameState::State0);
inline constexpr int GAME_STATE_1 = static_cast<int>(GameState::State1);
inline constexpr int GAME_STATE_2 = static_cast<int>(GameState::State2);
inline constexpr int GAME_STATE_3 = static_cast<int>(GameState::State3);
inline constexpr int GAME_STATE_4 = static_cast<int>(GameState::State4);
inline constexpr int GAME_STATE_5 = static_cast<int>(GameState::State5);

extern int* game_global_vars;
extern int num_game_global_vars;
extern const char* msg_path;
extern int game_user_wants_to_quit;

extern MessageList misc_message_file;
extern DB_DATABASE* master_db_handle;
extern DB_DATABASE* critter_db_handle;

int game_init(const char* windowTitle, bool isMapper, int font, int flags, int argc, char** argv);
void game_reset();
void game_exit();
int game_handle_input(int eventCode, bool isInCombatMode);
void game_ui_disable(int a1);
void game_ui_enable();
bool game_ui_is_disabled();
int game_get_global_var(int var);
int game_set_global_var(int var, int value);
int game_load_info();
int game_load_info_vars(const char* path, const char* section, int* variablesListLengthPtr, int** variablesListPtr);
int game_state();
int game_state_request(int a1);
void game_state_update();
int game_quit_with_confirm();

} // namespace fallout

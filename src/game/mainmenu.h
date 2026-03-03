#pragma once


namespace fallout {

enum class MainMenuOption : int {
    Intro = 0,
    NewGame = 1,
    LoadGame = 2,
    Screensaver = 3,
    Timeout = 4,
    Credits = 5,
    Quotes = 6,
    Exit = 7,
    SelfRun = 8,
    Options = 9,
};

inline constexpr int MAIN_MENU_INTRO = static_cast<int>(MainMenuOption::Intro);
inline constexpr int MAIN_MENU_NEW_GAME = static_cast<int>(MainMenuOption::NewGame);
inline constexpr int MAIN_MENU_LOAD_GAME = static_cast<int>(MainMenuOption::LoadGame);
inline constexpr int MAIN_MENU_SCREENSAVER = static_cast<int>(MainMenuOption::Screensaver);
inline constexpr int MAIN_MENU_TIMEOUT = static_cast<int>(MainMenuOption::Timeout);
inline constexpr int MAIN_MENU_CREDITS = static_cast<int>(MainMenuOption::Credits);
inline constexpr int MAIN_MENU_QUOTES = static_cast<int>(MainMenuOption::Quotes);
inline constexpr int MAIN_MENU_EXIT = static_cast<int>(MainMenuOption::Exit);
inline constexpr int MAIN_MENU_SELFRUN = static_cast<int>(MainMenuOption::SelfRun);
inline constexpr int MAIN_MENU_OPTIONS = static_cast<int>(MainMenuOption::Options);

extern bool in_main_menu;

int main_menu_create();
void main_menu_destroy();
void main_menu_hide(bool animate);
void main_menu_show(bool animate);
int main_menu_is_shown();
int main_menu_is_enabled();
void main_menu_set_timeout(unsigned int timeout);
unsigned int main_menu_get_timeout();
int main_menu_loop();

} // namespace fallout

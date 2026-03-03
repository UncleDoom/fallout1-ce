#pragma once


#include <cstddef>

#include "plib/gnw/gnw_types.h"
#include "plib/gnw/rect.h"
#include "plib/gnw/svga_types.h"

namespace fallout {

enum class WindowManagerErr : int {
    Ok = 0,
    InitializingVideoMode = 1,
    NoMemory = 2,
    InitializingTextFonts = 3,
    WindowSystemAlreadyInitialized = 4,
    WindowSystemNotInitialized = 5,
    CurrentWindowsTooBig = 6,
    InitializingDefaultDatabase = 7,

    // Unknown fatal error.
    //
    // NOTE: When this error code returned from window system initialization, the
    // game simply exits without any debug message. There is no way to figure out
    // its meaning.
    Err8 = 8,
    AlreadyRunning = 9,
    TitleNotSet = 10,
    InitializingInput = 11,
};

inline constexpr int WINDOW_MANAGER_OK = static_cast<int>(WindowManagerErr::Ok);
inline constexpr int WINDOW_MANAGER_ERR_INITIALIZING_VIDEO_MODE = static_cast<int>(WindowManagerErr::InitializingVideoMode);
inline constexpr int WINDOW_MANAGER_ERR_NO_MEMORY = static_cast<int>(WindowManagerErr::NoMemory);
inline constexpr int WINDOW_MANAGER_ERR_INITIALIZING_TEXT_FONTS = static_cast<int>(WindowManagerErr::InitializingTextFonts);
inline constexpr int WINDOW_MANAGER_ERR_WINDOW_SYSTEM_ALREADY_INITIALIZED = static_cast<int>(WindowManagerErr::WindowSystemAlreadyInitialized);
inline constexpr int WINDOW_MANAGER_ERR_WINDOW_SYSTEM_NOT_INITIALIZED = static_cast<int>(WindowManagerErr::WindowSystemNotInitialized);
inline constexpr int WINDOW_MANAGER_ERR_CURRENT_WINDOWS_TOO_BIG = static_cast<int>(WindowManagerErr::CurrentWindowsTooBig);
inline constexpr int WINDOW_MANAGER_ERR_INITIALIZING_DEFAULT_DATABASE = static_cast<int>(WindowManagerErr::InitializingDefaultDatabase);
inline constexpr int WINDOW_MANAGER_ERR_8 = static_cast<int>(WindowManagerErr::Err8);
inline constexpr int WINDOW_MANAGER_ERR_ALREADY_RUNNING = static_cast<int>(WindowManagerErr::AlreadyRunning);
inline constexpr int WINDOW_MANAGER_ERR_TITLE_NOT_SET = static_cast<int>(WindowManagerErr::TitleNotSet);
inline constexpr int WINDOW_MANAGER_ERR_INITIALIZING_INPUT = static_cast<int>(WindowManagerErr::InitializingInput);

extern bool GNW_win_init_flag;
extern int GNW_wcolor[6];

extern void* GNW_texture;

int win_init(VideoOptions* video_options, int flags);
int win_active();
void win_exit(void);
int win_add(int x, int y, int width, int height, int color, int flags);
void win_delete(int win);
void win_buffering(bool a1);
void win_border(int win);
void win_no_texture();
void win_set_bk_color(int color);
void win_print(int win, const char* str, int width, int x, int y, int color);
void win_text(int win, char** fileNameList, int fileNameListLength, int maxWidth, int x, int y, int color);
void win_line(int win, int left, int top, int right, int bottom, int color);
void win_box(int win, int left, int top, int right, int bottom, int color);
void win_shaded_box(int id, int ulx, int uly, int lrx, int lry, int color1, int color2);
void win_fill(int win, int x, int y, int width, int height, int color);
void win_show(int win);
void win_hide(int win);
void win_move(int win_index, int x, int y);
void win_draw(int win);
void win_draw_rect(int win, const Rect* rect);
void win_refresh_all(Rect* rect);
void win_drag(int win);
void win_get_mouse_buf(unsigned char* a1);
Window* GNW_find(int win);
unsigned char* win_get_buf(int win);
int win_get_top_win(int x, int y);
int win_width(int win);
int win_height(int win);
int win_get_rect(int win, Rect* rect);
int win_check_all_buttons();
Button* GNW_find_button(int btn, Window** out_win);
int GNW_check_menu_bars(int a1);
void win_set_minimized_title(const char* title);
void win_set_trans_b2b(int id, WindowBlitProc* trans_b2b);
bool GNWSystemError(const char* str);

} // namespace fallout

#pragma once


#include "game/enum_utils.h"
#include "plib/gnw/rect.h"

namespace fallout {

// The maximum number of buttons in one button group.
inline constexpr int BUTTON_GROUP_BUTTON_LIST_CAPACITY = 64;

enum class WindowFlags : unsigned {
    // Use system window flags which are set during game startup and does not
    // change afterwards.
    UseDefaults = 0x1,
    DontMoveTop = 0x2,
    MoveOnTop = 0x4,
    Hidden = 0x8,
    // Sfall calls this Exclusive.
    Modal = 0x10,
    Transparent = 0x20,
    Flag_0x40 = 0x40,
    // Draggable?
    Flag_0x80 = 0x80,
    Managed = 0x100,
};
DEFINE_ENUM_FLAG_OPERATORS(WindowFlags)

inline constexpr int WINDOW_USE_DEFAULTS = static_cast<int>(WindowFlags::UseDefaults);
inline constexpr int WINDOW_DONT_MOVE_TOP = static_cast<int>(WindowFlags::DontMoveTop);
inline constexpr int WINDOW_MOVE_ON_TOP = static_cast<int>(WindowFlags::MoveOnTop);
inline constexpr int WINDOW_HIDDEN = static_cast<int>(WindowFlags::Hidden);
inline constexpr int WINDOW_MODAL = static_cast<int>(WindowFlags::Modal);
inline constexpr int WINDOW_TRANSPARENT = static_cast<int>(WindowFlags::Transparent);
inline constexpr int WINDOW_FLAG_0x40 = static_cast<int>(WindowFlags::Flag_0x40);
inline constexpr int WINDOW_FLAG_0x80 = static_cast<int>(WindowFlags::Flag_0x80);
inline constexpr int WINDOW_MANAGED = static_cast<int>(WindowFlags::Managed);


enum class ButtonFlags : unsigned {
    Flag_0x01 = 0x01,
    Flag_0x02 = 0x02,
    Flag_0x04 = 0x04,
    Disabled = 0x08,
    Flag_0x10 = 0x10,
    Transparent = 0x20,
    Flag_0x40 = 0x40,
    Graphic = 0x010000,
    Checked = 0x020000,
    Radio = 0x040000,
    RightMouseButtonConfigured = 0x080000,
};
DEFINE_ENUM_FLAG_OPERATORS(ButtonFlags)

inline constexpr int BUTTON_FLAG_0x01 = static_cast<int>(ButtonFlags::Flag_0x01);
inline constexpr int BUTTON_FLAG_0x02 = static_cast<int>(ButtonFlags::Flag_0x02);
inline constexpr int BUTTON_FLAG_0x04 = static_cast<int>(ButtonFlags::Flag_0x04);
inline constexpr int BUTTON_FLAG_DISABLED = static_cast<int>(ButtonFlags::Disabled);
inline constexpr int BUTTON_FLAG_0x10 = static_cast<int>(ButtonFlags::Flag_0x10);
inline constexpr int BUTTON_FLAG_TRANSPARENT = static_cast<int>(ButtonFlags::Transparent);
inline constexpr int BUTTON_FLAG_0x40 = static_cast<int>(ButtonFlags::Flag_0x40);
inline constexpr int BUTTON_FLAG_GRAPHIC = static_cast<int>(ButtonFlags::Graphic);
inline constexpr int BUTTON_FLAG_CHECKED = static_cast<int>(ButtonFlags::Checked);
inline constexpr int BUTTON_FLAG_RADIO = static_cast<int>(ButtonFlags::Radio);
inline constexpr int BUTTON_FLAG_RIGHT_MOUSE_BUTTON_CONFIGURED = static_cast<int>(ButtonFlags::RightMouseButtonConfigured);


struct Button;
struct ButtonGroup;

using WindowBlitProc = void(unsigned char* src, int width, int height, int srcPitch, unsigned char* dest, int destPitch);
using ButtonCallback = void(int btn, int keyCode);
using RadioButtonCallback = void(int btn);

struct MenuPulldown {
    Rect rect;
    int keyCode;
    int itemsLength;
    char** items;
    int foregroundColor;
    int backgroundColor;
};

class MenuBar {
public:
    int win;
    Rect rect;
    int pulldownsLength;
    MenuPulldown pulldowns[15];
    int foregroundColor;
    int backgroundColor;

    int GNW_process_menu(int pulldownIndex);
};

class Window {
public:
    int id;
    int flags;
    Rect rect;
    int width;
    int height;
    int color;
    int tx;
    int ty;
    unsigned char* buffer;
    Button* buttonListHead;
    Button* hoveredButton;
    Button* clickedButton;
    MenuBar* menuBar;
    WindowBlitProc* blitProc;

    void winRefresh(Rect* rect, unsigned char* a3);
    int checkButtons(int* keyCodePtr);
    void buttonRefresh(Rect* rect);
};

class Button {
public:
    int id;
    int flags;
    Rect rect;
    int mouseEnterEventCode;
    int mouseExitEventCode;
    int lefMouseDownEventCode;
    int leftMouseUpEventCode;
    int rightMouseDownEventCode;
    int rightMouseUpEventCode;
    unsigned char* normalImage;
    unsigned char* pressedImage;
    unsigned char* hoverImage;
    unsigned char* disabledNormalImage;
    unsigned char* disabledPressedImage;
    unsigned char* disabledHoverImage;
    unsigned char* currentImage;
    unsigned char* mask;
    ButtonCallback* mouseEnterProc;
    ButtonCallback* mouseExitProc;
    ButtonCallback* leftMouseDownProc;
    ButtonCallback* leftMouseUpProc;
    ButtonCallback* rightMouseDownProc;
    ButtonCallback* rightMouseUpProc;
    ButtonCallback* pressSoundFunc;
    ButtonCallback* releaseSoundFunc;
    ButtonGroup* buttonGroup;
    Button* prev;
    Button* next;

    bool underMouse(Rect* rect);
    int checkGroup();
    void draw(Window* window, unsigned char* data, bool draw, Rect* bound, bool sound);
    void destroy();
};

struct ButtonGroup {
    int maxChecked;
    int currChecked;
    RadioButtonCallback* func;
    int buttonsLength;
    Button* buttons[BUTTON_GROUP_BUTTON_LIST_CAPACITY];
};

} // namespace fallout

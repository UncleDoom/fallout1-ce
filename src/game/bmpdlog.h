#pragma once


#include "game/enum_utils.h"

namespace fallout {

enum class DialogBoxOptions : unsigned {
    Large = 0x01,
    Medium = 0x02,
    NoHorizontalCentering = 0x04,
    NoVerticalCentering = 0x08,
    YesNo = 0x10,
    Flag_0x20 = 0x20,
};
DEFINE_ENUM_FLAG_OPERATORS(DialogBoxOptions)

inline constexpr int DIALOG_BOX_LARGE = static_cast<int>(DialogBoxOptions::Large);
inline constexpr int DIALOG_BOX_MEDIUM = static_cast<int>(DialogBoxOptions::Medium);
inline constexpr int DIALOG_BOX_NO_HORIZONTAL_CENTERING = static_cast<int>(DialogBoxOptions::NoHorizontalCentering);
inline constexpr int DIALOG_BOX_NO_VERTICAL_CENTERING = static_cast<int>(DialogBoxOptions::NoVerticalCentering);
inline constexpr int DIALOG_BOX_YES_NO = static_cast<int>(DialogBoxOptions::YesNo);
inline constexpr int DIALOG_BOX_0x20 = static_cast<int>(DialogBoxOptions::Flag_0x20);


enum class DialogType : int {
    Medium = 0,
    Large = 1,
    Count = 2,
};

inline constexpr int DIALOG_TYPE_MEDIUM = static_cast<int>(DialogType::Medium);
inline constexpr int DIALOG_TYPE_LARGE = static_cast<int>(DialogType::Large);
inline constexpr int DIALOG_TYPE_COUNT = static_cast<int>(DialogType::Count);

enum class FileDialogFrm : int {
    Background = 0,
    LittleRedButtonNormal = 1,
    LittleRedButtonPressed = 2,
    ScrollDownArrowNormal = 3,
    ScrollDownArrowPressed = 4,
    ScrollUpArrowNormal = 5,
    ScrollUpArrowPressed = 6,
    Count = 7,
};

inline constexpr int FILE_DIALOG_FRM_BACKGROUND = static_cast<int>(FileDialogFrm::Background);
inline constexpr int FILE_DIALOG_FRM_LITTLE_RED_BUTTON_NORMAL = static_cast<int>(FileDialogFrm::LittleRedButtonNormal);
inline constexpr int FILE_DIALOG_FRM_LITTLE_RED_BUTTON_PRESSED = static_cast<int>(FileDialogFrm::LittleRedButtonPressed);
inline constexpr int FILE_DIALOG_FRM_SCROLL_DOWN_ARROW_NORMAL = static_cast<int>(FileDialogFrm::ScrollDownArrowNormal);
inline constexpr int FILE_DIALOG_FRM_SCROLL_DOWN_ARROW_PRESSED = static_cast<int>(FileDialogFrm::ScrollDownArrowPressed);
inline constexpr int FILE_DIALOG_FRM_SCROLL_UP_ARROW_NORMAL = static_cast<int>(FileDialogFrm::ScrollUpArrowNormal);
inline constexpr int FILE_DIALOG_FRM_SCROLL_UP_ARROW_PRESSED = static_cast<int>(FileDialogFrm::ScrollUpArrowPressed);
inline constexpr int FILE_DIALOG_FRM_COUNT = static_cast<int>(FileDialogFrm::Count);

enum class FileDialogScrollDirection : int {
    None = 0,
    Up = 1,
    Down = 2,
};

inline constexpr int FILE_DIALOG_SCROLL_DIRECTION_NONE = static_cast<int>(FileDialogScrollDirection::None);
inline constexpr int FILE_DIALOG_SCROLL_DIRECTION_UP = static_cast<int>(FileDialogScrollDirection::Up);
inline constexpr int FILE_DIALOG_SCROLL_DIRECTION_DOWN = static_cast<int>(FileDialogScrollDirection::Down);

extern int dbox[DIALOG_TYPE_COUNT];
extern int ytable[DIALOG_TYPE_COUNT];
extern int xtable[DIALOG_TYPE_COUNT];
extern int doneY[DIALOG_TYPE_COUNT];
extern int doneX[DIALOG_TYPE_COUNT];
extern int dblines[DIALOG_TYPE_COUNT];
extern int flgids[FILE_DIALOG_FRM_COUNT];
extern int flgids2[FILE_DIALOG_FRM_COUNT];

int dialog_out(const char* title, const char** body, int bodyLength, int x, int y, int titleColor, const char* a8, int bodyColor, int flags);
int file_dialog(char* title, char** fileList, char* dest, int fileListLength, int x, int y, int flags);
int save_file_dialog(char* title, char** fileList, char* dest, int fileListLength, int x, int y, int flags);

} // namespace fallout

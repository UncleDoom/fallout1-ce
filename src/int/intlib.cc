#include "int/intlib.h"

#include <cstdio>

#include "int/datafile.h"
#include "int/dialog.h"
#include "int/memdbg.h"
#include "int/mousemgr.h"
#include "int/nevs.h"
#include "int/share1.h"
#include "int/sound.h"
#include "int/support/intextra.h"
#include "int/window.h"
#include "plib/color/color.h"
#include "plib/gnw/debug.h"
#include "plib/gnw/input.h"
#include "plib/gnw/intrface.h"
#include "plib/gnw/svga.h"
#include "plib/gnw/text.h"

namespace fallout {

static constexpr int INT_LIB_SOUNDS_CAPACITY = 32;
static constexpr int INT_LIB_KEY_HANDLERS_CAPACITY = 256;

struct IntLibKeyHandlerEntry {
    Program* program;
    int proc;
};

static void op_fillwin3x3(Program* program);
static void op_format(Program* program);
static void op_print(Program* program);
static void op_selectfilelist(Program* program);
static void op_tokenize(Program* program);
static void op_printrect(Program* program);
static void op_selectwin(Program* program);
static void op_display(Program* program);
static void op_displayraw(Program* program);
static void interpretFadePaletteBK(unsigned char* oldPalette, unsigned char* newPalette, int a3, float duration, int shouldProcessBk);
static void op_fadein(Program* program);
static void op_fadeout(Program* program);
static void op_movieflags(Program* program);
static void op_playmovie(Program* program);
static void op_playmovierect(Program* program);
static void op_stopmovie(Program* program);
static void op_addregionproc(Program* program);
static void op_addregionrightproc(Program* program);
static void op_createwin(Program* program);
static void op_resizewin(Program* program);
static void op_scalewin(Program* program);
static void op_deletewin(Program* program);
static void op_saystart(Program* program);
static void op_deleteregion(Program* program);
static void op_activateregion(Program* program);
static void op_checkregion(Program* program);
static void op_addregion(Program* program);
static void op_saystartpos(Program* program);
static void op_sayreplytitle(Program* program);
static void op_saygotoreply(Program* program);
static void op_sayoption(Program* program);
static void op_sayreply(Program* program);
static int checkDialog(Program* program);
static void op_sayend(Program* program);
static void op_saygetlastpos(Program* program);
static void op_sayquit(Program* program);
static void op_saymessagetimeout(Program* program);
static void op_saymessage(Program* program);
static void op_gotoxy(Program* program);
static void op_addbuttonflag(Program* program);
static void op_addregionflag(Program* program);
static void op_addbutton(Program* program);
static void op_addbuttontext(Program* program);
static void op_addbuttongfx(Program* program);
static void op_addbuttonproc(Program* program);
static void op_addbuttonrightproc(Program* program);
static void op_showwin(Program* program);
static void op_deletebutton(Program* program);
static void op_fillwin(Program* program);
static void op_fillrect(Program* program);
static void op_hidemouse(Program* program);
static void op_showmouse(Program* program);
static void op_mouseshape(Program* program);
static void op_setglobalmousefunc(Program* Program);
static void op_displaygfx(Program* program);
static void op_loadpalettetable(Program* program);
static void op_addNamedEvent(Program* program);
static void op_addNamedHandler(Program* program);
static void op_clearNamed(Program* program);
static void op_signalNamed(Program* program);
static void op_addkey(Program* program);
static void op_deletekey(Program* program);
static void op_refreshmouse(Program* program);
static void op_setfont(Program* program);
static void op_settextflags(Program* program);
static void op_settextcolor(Program* program);
static void op_sayoptioncolor(Program* program);
static void op_sayreplycolor(Program* program);
static void op_sethighlightcolor(Program* program);
static void op_sayreplywindow(Program* program);
static void op_sayreplyflags(Program* program);
static void op_sayoptionflags(Program* program);
static void op_sayoptionwindow(Program* program);
static void op_sayborder(Program* program);
static void op_sayscrollup(Program* program);
static void op_sayscrolldown(Program* program);
static void op_saysetspacing(Program* program);
static void op_sayrestart(Program* program);
static void soundCallbackInterpret(void* userData, int a2);
static int soundDeleteInterpret(int value);
static int soundPauseInterpret(int value);
static int soundRewindInterpret(int value);
static int soundUnpauseInterpret(int value);
static void op_soundplay(Program* program);
static void op_soundpause(Program* program);
static void op_soundresume(Program* program);
static void op_soundstop(Program* program);
static void op_soundrewind(Program* program);
static void op_sounddelete(Program* program);
static void op_setoneoptpause(Program* program);
static bool intLibDoInput(int key);

// 0x505620
static int TimeOut = 0;

// 0x59BB60
static Sound* interpretSounds[INT_LIB_SOUNDS_CAPACITY];

// 0x59BBE0
static unsigned char blackPal[256 * 3];

// 0x59BEE0
static IntLibKeyHandlerEntry inputProc[INT_LIB_KEY_HANDLERS_CAPACITY];

// 0x59C6E0
static bool currentlyFadedIn;

// 0x59C6E4
static int anyKeyOffset;

// 0x59C6E8
static int numCallbacks;

// 0x59C6EC
static Program* anyKeyProg;

// 0x59C6F0
static IntLibProgramDeleteCallback** callbacks;

// 0x59C6F4
static int sayStartingPosition;

// 0x456CC0
static void op_fillwin3x3(Program* program)
{
    char* fileName = program->stackPopString();
    char* mangledFileName = interpretMangleName(fileName);

    int imageWidth;
    int imageHeight;
    unsigned char* imageData = loadDataFile(mangledFileName, &imageWidth, &imageHeight);
    if (imageData == nullptr) {
        interpretError("cannot load 3x3 file '%s'", mangledFileName);
    }

    selectWindowID(program->windowId);

    fillBuf3x3(imageData,
        imageWidth,
        imageHeight,
        windowGetBuffer(),
        windowWidth(),
        windowHeight());

    myfree(imageData, __FILE__, __LINE__); // "..\\int\\INTLIB.C", 94
}

// 0x456D74
static void op_format(Program* program)
{
    int textAlignment = program->stackPopInteger();
    int height = program->stackPopInteger();
    int width = program->stackPopInteger();
    int y = program->stackPopInteger();
    int x = program->stackPopInteger();
    char* string = program->stackPopString();

    if (!windowFormatMessage(string, x, y, width, height, textAlignment)) {
        interpretError("Error formatting message\n");
    }
}

// 0x456EE8
static void op_print(Program* program)
{
    selectWindowID(program->windowId);

    ProgramValue value = program->stackPopValue();
    char string[80];

    // SFALL: Fix broken Print() script function.
    // CE: Original code uses `interpretOutput` to handle printing. However
    // this function looks invalid or broken itself. Check `opSelect` - it sets
    // `outputFunc` to `windowOutput`, but `outputFunc` is never called. I'm not
    // sure if this fix can be moved into `interpretOutput` because it is also
    // used in procedure setup functions.
    //
    // The fix is slightly different, Sfall fixes strings only, ints and floats
    // are still passed to `interpretOutput`.
    switch (value.opcode & VALUE_TYPE_MASK) {
    case VALUE_TYPE_STRING:
        windowOutput(program->getString(value.opcode, value.integerValue));
        break;
    case VALUE_TYPE_FLOAT:
        snprintf(string, sizeof(string), "%.5f", value.floatValue);
        windowOutput(string);
        break;
    case VALUE_TYPE_INT:
        snprintf(string, sizeof(string), "%d", value.integerValue);
        windowOutput(string);
        break;
    }
}

// 0x456F80
static void op_selectfilelist(Program* program)
{
    program->flags |= PROGRAM_FLAG_0x20;

    char* pattern = program->stackPopString();
    char* title = program->stackPopString();

    int fileListLength;
    char** fileList = getFileList(interpretMangleName(pattern), &fileListLength);
    if (fileList != nullptr && fileListLength != 0) {
        int selectedIndex = win_list_select(title,
            fileList,
            fileListLength,
            nullptr,
            320 - text_width(title) / 2,
            200,
            colorTable[0x7FFF] | 0x10000);

        if (selectedIndex != -1) {
            program->stackPushString(fileList[selectedIndex]);
        } else {
            program->stackPushInteger(0);
        }

        freeFileList(fileList);
    } else {
        program->stackPushInteger(0);
    }

    program->flags &= ~PROGRAM_FLAG_0x20;
}

// 0x4570DC
static void op_tokenize(Program* program)
{
    int ch = program->stackPopInteger();

    ProgramValue prevValue = program->stackPopValue();

    char* prev = nullptr;
    if ((prevValue.opcode & VALUE_TYPE_MASK) == VALUE_TYPE_INT) {
        if (prevValue.integerValue != 0) {
            interpretError("Error, invalid arg 2 to tokenize. (only accept 0 for int value)");
        }
    } else if ((prevValue.opcode & VALUE_TYPE_MASK) == VALUE_TYPE_STRING) {
        prev = program->getString(prevValue.opcode, prevValue.integerValue);
    } else {
        interpretError("Error, invalid arg 2 to tokenize. (string)");
    }

    char* string = program->stackPopString();
    char* temp = nullptr;

    if (prev != nullptr) {
        char* start = strstr(string, prev);
        if (start != nullptr) {
            start += strlen(prev);
            while (*start != ch && *start != '\0') {
                start++;
            }
        }

        if (*start == ch) {
            int length = 0;
            char* end = start + 1;
            while (*end != ch && *end != '\0') {
                end++;
                length++;
            }

            temp = static_cast<char*>(mycalloc(1, length + 1, __FILE__, __LINE__)); // "..\\int\\INTLIB.C, 230
            strncpy(temp, start, length);
            program->stackPushString(temp);
        } else {
            program->stackPushInteger(0);
        }
    } else {
        int length = 0;
        char* end = string;
        while (*end != ch && *end != '\0') {
            end++;
            length++;
        }

        if (string != nullptr) {
            temp = static_cast<char*>(mycalloc(1, length + 1, __FILE__, __LINE__)); // "..\\int\\INTLIB.C", 248
            strncpy(temp, string, length);
            program->stackPushString(temp);
        } else {
            program->stackPushInteger(0);
        }
    }

    if (temp != nullptr) {
        myfree(temp, __FILE__, __LINE__); // "..\\int\\INTLIB.C" , 260
    }
}

// 0x457308
static void op_printrect(Program* program)
{
    selectWindowID(program->windowId);

    int v1 = program->stackPopInteger();
    if (v1 > 2) {
        interpretError("Invalid arg 3 given to printrect, expecting int");
    }

    int v2 = program->stackPopInteger();

    ProgramValue value = program->stackPopValue();
    char string[80];
    switch (value.opcode & VALUE_TYPE_MASK) {
    case VALUE_TYPE_STRING:
        snprintf(string, sizeof(string), "%s", program->getString(value.opcode, value.integerValue));
        break;
    case VALUE_TYPE_FLOAT:
        snprintf(string, sizeof(string), "%.5f", value.floatValue);
        break;
    case VALUE_TYPE_INT:
        snprintf(string, sizeof(string), "%d", value.integerValue);
        break;
    }

    if (!windowPrintRect(string, v2, v1)) {
        interpretError("Error in printrect");
    }
}

// 0x457430
static void op_selectwin(Program* program)
{
    const char* windowName = program->stackPopString();
    int win = pushWindow(windowName);
    if (win == -1) {
        interpretError("Error selecing window %s\n", windowName);
    }

    program->windowId = win;

    interpretOutputFunc(windowOutput);
}

// 0x4574B4
static void op_display(Program* program)
{
    char* fileName = program->stackPopString();

    selectWindowID(program->windowId);

    char* mangledFileName = interpretMangleName(fileName);
    displayFile(mangledFileName);
}

// 0x457514
static void op_displayraw(Program* program)
{
    char* fileName = program->stackPopString();

    selectWindowID(program->windowId);

    char* mangledFileName = interpretMangleName(fileName);
    displayFileRaw(mangledFileName);
}

// 0x457574
static void interpretFadePaletteBK(unsigned char* oldPalette, unsigned char* newPalette, int a3, float duration, int shouldProcessBk)
{
    unsigned int time;
    unsigned int previousTime;
    unsigned int delta;
    int step;
    int steps;
    int index;
    unsigned char palette[256 * 3];

    time = get_time();
    previousTime = time;
    steps = static_cast<int>(duration);
    step = 0;
    delta = 0;

    if (duration != 0.0) {
        while (step < steps) {
            if (delta != 0) {
                for (index = 0; index < 768; index++) {
                    palette[index] = oldPalette[index] - (oldPalette[index] - newPalette[index]) * step / steps;
                }

                setSystemPalette(palette);
                renderPresent();

                previousTime = time;
                step += delta;
            }

            if (shouldProcessBk) {
                process_bk();
            }

            time = get_time();
            delta = time - previousTime;
        }
    }

    setSystemPalette(newPalette);
    renderPresent();
}

// 0x457678
void interpretFadePalette(unsigned char* oldPalette, unsigned char* newPalette, int a3, float duration)
{
    interpretFadePaletteBK(oldPalette, newPalette, a3, duration, 1);
}

// 0x457688
int intlibGetFadeIn()
{
    return currentlyFadedIn;
}

// 0x457690
void interpretFadeOut(float duration)
{
    int cursorWasHidden;

    cursorWasHidden = mouse_hidden();
    mouse_hide();

    interpretFadePaletteBK(getSystemPalette(), blackPal, 64, duration, 1);

    if (!cursorWasHidden) {
        mouse_show();
    }
}

// 0x4576C8
void interpretFadeIn(float duration)
{
    interpretFadePaletteBK(blackPal, cmap, 64, duration, 1);
}

// 0x4576EC
void interpretFadeOutNoBK(float duration)
{
    int cursorWasHidden;

    cursorWasHidden = mouse_hidden();
    mouse_hide();

    interpretFadePaletteBK(getSystemPalette(), blackPal, 64, duration, 0);

    if (!cursorWasHidden) {
        mouse_show();
    }
}

// 0x457724
void interpretFadeInNoBK(float duration)
{
    interpretFadePaletteBK(blackPal, cmap, 64, duration, 0);
}

// 0x457748
static void op_fadein(Program* program)
{
    int data = program->stackPopInteger();

    program->flags |= PROGRAM_FLAG_0x20;

    setSystemPalette(blackPal);

    // NOTE: Uninline.
    interpretFadeIn(static_cast<float>(data));

    currentlyFadedIn = true;

    program->flags &= ~PROGRAM_FLAG_0x20;
}

// 0x4577E0
static void op_fadeout(Program* program)
{
    int data = program->stackPopInteger();

    program->flags |= PROGRAM_FLAG_0x20;

    // NOTE: Uninline.
    interpretFadeOut(static_cast<float>(data));

    currentlyFadedIn = false;

    program->flags &= ~PROGRAM_FLAG_0x20;
}

// 0x457884
int checkMovie(Program* program)
{
    if (dialogGetDialogDepth() > 0) {
        return 1;
    }

    return windowMoviePlaying();
}

// 0x457898
static void op_movieflags(Program* program)
{
    int data = program->stackPopInteger();

    if (!windowSetMovieFlags(data)) {
        interpretError("Error setting movie flags\n");
    }
}

// 0x4578C0
static void op_playmovie(Program* program)
{
    // 0x59C6F8
    static char name[100];

    char* movieFileName = program->stackPopString();

    strcpy(name, movieFileName);

    if (strrchr(name, '.') == nullptr) {
        strcat(name, ".mve");
    }

    selectWindowID(program->windowId);

    program->flags |= PROGRAM_IS_WAITING;
    program->checkWaitFunc = checkMovie;

    char* mangledFileName = interpretMangleName(name);
    if (!windowPlayMovie(mangledFileName)) {
        interpretError("Error playing movie");
    }
}

// 0x45799C
static void op_playmovierect(Program* program)
{
    // 0x59C75C
    static char name[100];

    int height = program->stackPopInteger();
    int width = program->stackPopInteger();
    int y = program->stackPopInteger();
    int x = program->stackPopInteger();
    char* movieFileName = program->stackPopString();

    strcpy(name, movieFileName);

    if (strrchr(name, '.') == nullptr) {
        strcat(name, ".mve");
    }

    selectWindowID(program->windowId);

    program->checkWaitFunc = checkMovie;
    program->flags |= PROGRAM_IS_WAITING;

    char* mangledFileName = interpretMangleName(name);
    if (!windowPlayMovieRect(mangledFileName, x, y, width, height)) {
        interpretError("Error playing movie");
    }
}

// 0x457ADC
static void op_stopmovie(Program* program)
{
    windowStopMovie();
    program->flags |= PROGRAM_FLAG_0x40;
}

// 0x457AF0
static void op_deleteregion(Program* program)
{
    ProgramValue value = program->stackPopValue();

    selectWindowID(program->windowId);

    const char* regionName = value.integerValue != -1 ? program->getString(value.opcode, value.integerValue) : nullptr;
    windowDeleteRegion(regionName);
}

// 0x457B6C
static void op_activateregion(Program* program)
{
    int v1 = program->stackPopInteger();
    char* regionName = program->stackPopString();

    windowActivateRegion(regionName, v1);
}

// 0x457BAC
static void op_checkregion(Program* program)
{
    const char* regionName = program->stackPopString();

    bool regionExists = windowCheckRegionExists(regionName);
    program->stackPushInteger(regionExists);
}

// 0x457C0C
static void op_addregion(Program* program)
{
    int args = program->stackPopInteger();

    if (args < 2) {
        interpretError("addregion call without enough points!");
    }

    selectWindowID(program->windowId);

    windowStartRegion(args / 2);

    while (args >= 2) {
        int y = program->stackPopInteger();
        int x = program->stackPopInteger();

        y = (y * windowGetYres() + 479) / 480;
        x = (x * windowGetXres() + 639) / 640;
        args -= 2;

        windowAddRegionPoint(x, y, true);
    }

    if (args == 0) {
        interpretError("Unnamed regions not allowed\n");
        windowEndRegion();
    } else {
        const char* regionName = program->stackPopString();
        windowAddRegionName(regionName);
        windowEndRegion();
    }
}

// 0x457D90
static void op_addregionproc(Program* program)
{
    int v1 = program->stackPopInteger();
    int v2 = program->stackPopInteger();
    int v3 = program->stackPopInteger();
    int v4 = program->stackPopInteger();
    const char* regionName = program->stackPopString();

    selectWindowID(program->windowId);

    if (!windowAddRegionProc(regionName, program, v4, v3, v2, v1)) {
        interpretError("Error setting procedures to region %s\n", regionName);
    }
}

// 0x457EDC
static void op_addregionrightproc(Program* program)
{
    int v1 = program->stackPopInteger();
    int v2 = program->stackPopInteger();
    const char* regionName = program->stackPopString();
    selectWindowID(program->windowId);

    if (!windowAddRegionRightProc(regionName, program, v2, v1)) {
        interpretError("ErrorError setting right button procedures to region %s\n", regionName);
    }
}

// 0x457FB4
static void op_createwin(Program* program)
{
    int height = program->stackPopInteger();
    int width = program->stackPopInteger();
    int y = program->stackPopInteger();
    int x = program->stackPopInteger();
    char* windowName = program->stackPopString();

    x = (x * windowGetXres() + 639) / 640;
    y = (y * windowGetYres() + 479) / 480;
    width = (width * windowGetXres() + 639) / 640;
    height = (height * windowGetYres() + 479) / 480;

    if (createWindow(windowName, x, y, width, height, colorTable[0], 0) == -1) {
        interpretError("Couldn't create window.");
    }
}

// 0x4580B4
static void op_resizewin(Program* program)
{
    int height = program->stackPopInteger();
    int width = program->stackPopInteger();
    int y = program->stackPopInteger();
    int x = program->stackPopInteger();
    char* windowName = program->stackPopString();

    x = (x * windowGetXres() + 639) / 640;
    y = (y * windowGetYres() + 479) / 480;
    width = (width * windowGetXres() + 639) / 640;
    height = (height * windowGetYres() + 479) / 480;

    if (resizeWindow(windowName, x, y, width, height) == -1) {
        interpretError("Couldn't resize window.");
    }
}

// 0x4581A8
static void op_scalewin(Program* program)
{
    int height = program->stackPopInteger();
    int width = program->stackPopInteger();
    int y = program->stackPopInteger();
    int x = program->stackPopInteger();
    char* windowName = program->stackPopString();

    x = (x * windowGetXres() + 639) / 640;
    y = (y * windowGetYres() + 479) / 480;
    width = (width * windowGetXres() + 639) / 640;
    height = (height * windowGetYres() + 479) / 480;

    if (scaleWindow(windowName, x, y, width, height) == -1) {
        interpretError("Couldn't scale window.");
    }
}

// 0x45829C
static void op_deletewin(Program* program)
{
    char* windowName = program->stackPopString();

    if (!deleteWindow(windowName)) {
        interpretError("Error deleting window %s\n", windowName);
    }

    program->windowId = popWindow();
}

// 0x4582E8
static void op_saystart(Program* program)
{
    sayStartingPosition = 0;

    program->flags |= PROGRAM_FLAG_0x20;
    int rc = dialogStart(program);
    program->flags &= ~PROGRAM_FLAG_0x20;

    if (rc != 0) {
        interpretError("Error starting dialog.");
    }
}

// 0x458334
static void op_saystartpos(Program* program)
{
    sayStartingPosition = program->stackPopInteger();

    program->flags |= PROGRAM_FLAG_0x20;
    int rc = dialogStart(program);
    program->flags &= ~PROGRAM_FLAG_0x20;

    if (rc != 0) {
        interpretError("Error starting dialog.");
    }
}

// 0x45838C
static void op_sayreplytitle(Program* program)
{
    ProgramValue value = program->stackPopValue();

    char* string = nullptr;
    if ((value.opcode & VALUE_TYPE_MASK) == VALUE_TYPE_STRING) {
        string = program->getString(value.opcode, value.integerValue);
    }

    if (dialogTitle(string) != 0) {
        interpretError("Error setting title.");
    }
}

// 0x4583E0
static void op_saygotoreply(Program* program)
{
    ProgramValue value = program->stackPopValue();

    char* string = nullptr;
    if ((value.opcode & VALUE_TYPE_MASK) == VALUE_TYPE_STRING) {
        string = program->getString(value.opcode, value.integerValue);
    }

    if (dialogGotoReply(string) != 0) {
        interpretError("Error during goto, couldn't find reply target %s", string);
    }
}

// 0x458438
static void op_sayoption(Program* program)
{
    program->flags |= PROGRAM_FLAG_0x20;

    ProgramValue v3 = program->stackPopValue();
    ProgramValue v2 = program->stackPopValue();

    const char* v1;
    if ((v2.opcode & VALUE_TYPE_MASK) == VALUE_TYPE_STRING) {
        v1 = program->getString(v2.opcode, v2.integerValue);
    } else {
        v1 = nullptr;
    }
    if ((v3.opcode & VALUE_TYPE_MASK) == VALUE_TYPE_STRING) {
        const char* v2 = program->getString(v3.opcode, v3.integerValue);
        if (dialogOption(v1, v2) != 0) {
            program->flags &= ~PROGRAM_FLAG_0x20;
            interpretError("Error setting option.");
        }
    } else if ((v3.opcode & VALUE_TYPE_MASK) == VALUE_TYPE_INT) {
        if (dialogOptionProc(v1, v3.integerValue) != 0) {
            program->flags &= ~PROGRAM_FLAG_0x20;
            interpretError("Error setting option.");
        }
    } else {
        interpretError("Invalid arg 2 to sayOption");
    }

    program->flags &= ~PROGRAM_FLAG_0x20;
}

// 0x458524
static void op_sayreply(Program* program)
{
    program->flags |= PROGRAM_FLAG_0x20;

    ProgramValue v3 = program->stackPopValue();
    ProgramValue v4 = program->stackPopValue();

    const char* v1;
    if ((v4.opcode & VALUE_TYPE_MASK) == VALUE_TYPE_STRING) {
        v1 = program->getString(v4.opcode, v4.integerValue);
    } else {
        v1 = nullptr;
    }

    const char* v2;
    if ((v3.opcode & VALUE_TYPE_MASK) == VALUE_TYPE_STRING) {
        v2 = program->getString(v3.opcode, v3.integerValue);
    } else {
        v2 = nullptr;
    }

    if (dialogReply(v1, v2) != 0) {
        program->flags &= ~PROGRAM_FLAG_0x20;
        interpretError("Error setting option.");
    }

    program->flags &= ~PROGRAM_FLAG_0x20;
}

// 0x4585DC
static int checkDialog(Program* program)
{
    program->flags |= PROGRAM_FLAG_0x40;
    return dialogGetDialogDepth() != -1;
}

// 0x4585F4
static void op_sayend(Program* program)
{
    program->flags |= PROGRAM_FLAG_0x20;
    int rc = dialogGo(sayStartingPosition);
    program->flags &= ~PROGRAM_FLAG_0x20;

    if (rc == -2) {
        program->checkWaitFunc = checkDialog;
        program->flags |= PROGRAM_IS_WAITING;
    }
}

// 0x45863C
static void op_saygetlastpos(Program* program)
{
    int value = dialogGetExitPoint();
    program->stackPushInteger(value);
}

// 0x458660
static void op_sayquit(Program* program)
{
    if (dialogQuit() != 0) {
        interpretError("Error quitting option.");
    }
}

// 0x458678
int getTimeOut()
{
    return TimeOut;
}

// 0x458680
void setTimeOut(int value)
{
    TimeOut = value;
}

// 0x458688
static void op_saymessagetimeout(Program* program)
{
    ProgramValue value = program->stackPopValue();

    // TODO: What the hell is this?
    if ((value.opcode & VALUE_TYPE_MASK) == 0x4000) {
        interpretError("sayMsgTimeout:  invalid var type passed.");
    }

    TimeOut = value.integerValue;
}

// 0x4586C4
static void op_saymessage(Program* program)
{
    program->flags |= PROGRAM_FLAG_0x20;

    ProgramValue v3 = program->stackPopValue();
    ProgramValue v4 = program->stackPopValue();

    const char* v1;
    if ((v4.opcode & VALUE_TYPE_MASK) == VALUE_TYPE_STRING) {
        v1 = program->getString(v4.opcode, v4.integerValue);
    } else {
        v1 = nullptr;
    }

    const char* v2;
    if ((v3.opcode & VALUE_TYPE_MASK) == VALUE_TYPE_STRING) {
        v2 = program->getString(v3.opcode, v3.integerValue);
    } else {
        v2 = nullptr;
    }

    if (dialogMessage(v1, v2, TimeOut) != 0) {
        program->flags &= ~PROGRAM_FLAG_0x20;
        interpretError("Error setting option.");
    }

    program->flags &= ~PROGRAM_FLAG_0x20;
}

// 0x458780
static void op_gotoxy(Program* program)
{
    int y = program->stackPopInteger();
    int x = program->stackPopInteger();

    selectWindowID(program->windowId);

    windowGotoXY(x, y);
}

// 0x4587FC
static void op_addbuttonflag(Program* program)
{
    int flag = program->stackPopInteger();
    const char* buttonName = program->stackPopString();
    if (!windowSetButtonFlag(buttonName, flag)) {
        // NOTE: Original code calls interpretGetString one more time with the
        // same params.
        interpretError("Error setting flag on button %s", buttonName);
    }
}

// 0x45889C
static void op_addregionflag(Program* program)
{
    int flag = program->stackPopInteger();
    const char* regionName = program->stackPopString();
    if (!windowSetRegionFlag(regionName, flag)) {
        // NOTE: Original code calls interpretGetString one more time with the
        // same params.
        interpretError("Error setting flag on region %s", regionName);
    }
}

// 0x45893C
static void op_addbutton(Program* program)
{
    int height = program->stackPopInteger();
    int width = program->stackPopInteger();
    int y = program->stackPopInteger();
    int x = program->stackPopInteger();
    char* buttonName = program->stackPopString();

    selectWindowID(program->windowId);

    height = (height * windowGetYres() + 479) / 480;
    width = (width * windowGetXres() + 639) / 640;
    y = (y * windowGetYres() + 479) / 480;
    x = (x * windowGetXres() + 639) / 640;

    windowAddButton(buttonName, x, y, width, height, 0);
}

// 0x458ACC
static void op_addbuttontext(Program* program)
{
    const char* text = program->stackPopString();
    const char* buttonName = program->stackPopString();

    if (!windowAddButtonText(buttonName, text)) {
        interpretError("Error setting text to button %s\n", buttonName);
    }
}

// 0x458B90
static void op_addbuttongfx(Program* program)
{
    ProgramValue v1 = program->stackPopValue();
    ProgramValue v2 = program->stackPopValue();
    ProgramValue v3 = program->stackPopValue();
    char* buttonName = program->stackPopString();

    if (((v3.opcode & VALUE_TYPE_MASK) == VALUE_TYPE_STRING || ((v3.opcode & VALUE_TYPE_MASK) == VALUE_TYPE_INT && v3.integerValue == 0))
        || ((v2.opcode & VALUE_TYPE_MASK) == VALUE_TYPE_STRING || ((v2.opcode & VALUE_TYPE_MASK) == VALUE_TYPE_INT && v2.integerValue == 0))
        || ((v1.opcode & VALUE_TYPE_MASK) == VALUE_TYPE_STRING || ((v1.opcode & VALUE_TYPE_MASK) == VALUE_TYPE_INT && v1.integerValue == 0))) {
        char* pressedFileName = interpretMangleName(program->getString(v3.opcode, v3.integerValue));
        char* normalFileName = interpretMangleName(program->getString(v2.opcode, v2.integerValue));
        char* hoverFileName = interpretMangleName(program->getString(v1.opcode, v1.integerValue));

        selectWindowID(program->windowId);

        if (!windowAddButtonGfx(buttonName, pressedFileName, normalFileName, hoverFileName)) {
            interpretError("Error setting graphics to button %s\n", buttonName);
        }
    } else {
        interpretError("Invalid filename given to addbuttongfx");
    }
}

// 0x458D28
static void op_addbuttonproc(Program* program)
{
    int v1 = program->stackPopInteger();
    int v2 = program->stackPopInteger();
    int v3 = program->stackPopInteger();
    int v4 = program->stackPopInteger();
    const char* buttonName = program->stackPopString();
    selectWindowID(program->windowId);

    if (!windowAddButtonProc(buttonName, program, v4, v3, v2, v1)) {
        interpretError("Error setting procedures to button %s\n", buttonName);
    }
}

// 0x458E74
static void op_addbuttonrightproc(Program* program)
{
    int v1 = program->stackPopInteger();
    int v2 = program->stackPopInteger();
    const char* regionName = program->stackPopString();
    selectWindowID(program->windowId);

    if (!windowAddRegionRightProc(regionName, program, v2, v1)) {
        interpretError("Error setting right button procedures to button %s\n", regionName);
    }
}

// 0x458F4C
static void op_showwin(Program* program)
{
    selectWindowID(program->windowId);
    windowDraw();
}

// 0x458F5C
static void op_deletebutton(Program* program)
{
    ProgramValue value = program->stackPopValue();

    switch (value.opcode & VALUE_TYPE_MASK) {
    case VALUE_TYPE_STRING:
        break;
    case VALUE_TYPE_INT:
        if (value.integerValue == -1) {
            break;
        }
        // FALLTHROUGH
    default:
        interpretError("Invalid type given to delete button");
    }

    selectWindowID(program->windowId);

    if ((value.opcode & 0xF7FF) == VALUE_TYPE_INT) {
        if (windowDeleteButton(nullptr)) {
            return;
        }
    } else {
        const char* buttonName = program->getString(value.opcode, value.integerValue);
        if (windowDeleteButton(buttonName)) {
            return;
        }
    }

    interpretError("Error deleting button");
}

// 0x458FF8
static void op_fillwin(Program* program)
{
    ProgramValue b = program->stackPopValue();
    ProgramValue g = program->stackPopValue();
    ProgramValue r = program->stackPopValue();

    if ((r.opcode & VALUE_TYPE_MASK) != VALUE_TYPE_FLOAT) {
        if ((r.opcode & VALUE_TYPE_MASK) == VALUE_TYPE_INT) {
            if (r.integerValue == 1) {
                r.floatValue = 1.0;
            } else if (r.integerValue != 0) {
                interpretError("Invalid red value given to fillwin");
            }
        }
    }

    if ((g.opcode & VALUE_TYPE_MASK) != VALUE_TYPE_FLOAT) {
        if ((g.opcode & VALUE_TYPE_MASK) == VALUE_TYPE_INT) {
            if (g.integerValue == 1) {
                g.floatValue = 1.0;
            } else if (g.integerValue != 0) {
                interpretError("Invalid green value given to fillwin");
            }
        }
    }

    if ((b.opcode & VALUE_TYPE_MASK) != VALUE_TYPE_FLOAT) {
        if ((b.opcode & VALUE_TYPE_MASK) == VALUE_TYPE_INT) {
            if (b.integerValue == 1) {
                b.floatValue = 1.0;
            } else if (b.integerValue != 0) {
                interpretError("Invalid blue value given to fillwin");
            }
        }
    }

    selectWindowID(program->windowId);

    windowFill(r.floatValue, g.floatValue, b.floatValue);
}

// 0x459108
static void op_fillrect(Program* program)
{
    ProgramValue b = program->stackPopValue();
    ProgramValue g = program->stackPopValue();
    ProgramValue r = program->stackPopValue();
    int height = program->stackPopInteger();
    int width = program->stackPopInteger();
    int y = program->stackPopInteger();
    int x = program->stackPopInteger();

    if ((r.opcode & VALUE_TYPE_MASK) != VALUE_TYPE_FLOAT) {
        if ((r.opcode & VALUE_TYPE_MASK) == VALUE_TYPE_INT) {
            if (r.integerValue == 1) {
                r.floatValue = 1.0;
            } else if (r.integerValue != 0) {
                interpretError("Invalid red value given to fillrect");
            }
        }
    }

    if ((g.opcode & VALUE_TYPE_MASK) != VALUE_TYPE_FLOAT) {
        if ((g.opcode & VALUE_TYPE_MASK) == VALUE_TYPE_INT) {
            if (g.integerValue == 1) {
                g.floatValue = 1.0;
            } else if (g.integerValue != 0) {
                interpretError("Invalid green value given to fillrect");
            }
        }
    }

    if ((b.opcode & VALUE_TYPE_MASK) != VALUE_TYPE_FLOAT) {
        if ((b.opcode & VALUE_TYPE_MASK) == VALUE_TYPE_INT) {
            if (b.integerValue == 1) {
                b.floatValue = 1.0;
            } else if (b.integerValue != 0) {
                interpretError("Invalid blue value given to fillrect");
            }
        }
    }

    selectWindowID(program->windowId);

    windowFillRect(x, y, width, height, r.floatValue, g.floatValue, b.floatValue);
}

// 0x459300
static void op_hidemouse(Program* program)
{
    mouse_hide();
}

// 0x459308
static void op_showmouse(Program* program)
{
    mouse_show();
}

// 0x459310
static void op_mouseshape(Program* program)
{
    int v1 = program->stackPopInteger();
    int v2 = program->stackPopInteger();
    char* fileName = program->stackPopString();

    if (!mouseSetMouseShape(fileName, v2, v1)) {
        interpretError("Error loading mouse shape.");
    }
}

// 0x4593D8
static void op_setglobalmousefunc(Program* Program)
{
    interpretError("setglobalmousefunc not defined");
}

// 0x4593E8
static void op_displaygfx(Program* program)
{
    int height = program->stackPopInteger();
    int width = program->stackPopInteger();
    int y = program->stackPopInteger();
    int x = program->stackPopInteger();
    char* fileName = program->stackPopString();

    char* mangledFileName = interpretMangleName(fileName);
    windowDisplay(mangledFileName, x, y, width, height);
}

// 0x459464
static void op_loadpalettetable(Program* program)
{
    char* path = program->stackPopString();
    if (!loadColorTable(path)) {
        interpretError(colorError());
    }
}

// 0x4594C0
static void op_addNamedEvent(Program* program)
{
    int proc = program->stackPopInteger();
    const char* name = program->stackPopString();
    nevs_addevent(name, program, proc, NEVS_TYPE_EVENT);
}

// 0x459524
static void op_addNamedHandler(Program* program)
{
    int proc = program->stackPopInteger();
    const char* name = program->stackPopString();
    nevs_addevent(name, program, proc, NEVS_TYPE_HANDLER);
}

// 0x45958C
static void op_clearNamed(Program* program)
{
    char* string = program->stackPopString();
    nevs_clearevent(string);
}

// 0x4595D8
static void op_signalNamed(Program* program)
{
    char* str = program->stackPopString();
    nevs_signal(str);
}

// 0x459624
static void op_addkey(Program* program)
{
    int proc = program->stackPopInteger();
    int key = program->stackPopInteger();

    if (key == -1) {
        anyKeyOffset = proc;
        anyKeyProg = program;
    } else {
        if (key > INT_LIB_KEY_HANDLERS_CAPACITY - 1) {
            interpretError("Key out of range");
        }

        inputProc[key].program = program;
        inputProc[key].proc = proc;
    }
}

// 0x4596C4
static void op_deletekey(Program* program)
{
    int key = program->stackPopInteger();

    if (key == -1) {
        anyKeyOffset = 0;
        anyKeyProg = nullptr;
    } else {
        if (key > INT_LIB_KEY_HANDLERS_CAPACITY - 1) {
            interpretError("Key out of range");
        }

        inputProc[key].program = nullptr;
        inputProc[key].proc = 0;
    }
}

// 0x459738
static void op_refreshmouse(Program* program)
{
    int data = program->stackPopInteger();

    if (!windowRefreshRegions()) {
        program->executeProc(data);
    }
}

// 0x459784
static void op_setfont(Program* program)
{
    int data = program->stackPopInteger();

    if (!windowSetFont(data)) {
        interpretError("Error setting font");
    }
}

// 0x4597D0
static void op_settextflags(Program* program)
{
    int data = program->stackPopInteger();

    if (!windowSetTextFlags(data)) {
        interpretError("Error setting text flags");
    }
}

// 0x45981C
static void op_settextcolor(Program* program)
{
    ProgramValue value[3];

    // NOTE: Original code does not use loops.
    for (int arg = 0; arg < 3; arg++) {
        value[arg] = program->stackPopValue();
    }

    for (int arg = 0; arg < 3; arg++) {
        if ((value[arg].opcode & VALUE_TYPE_MASK) != VALUE_TYPE_FLOAT
            && (value[arg].opcode & VALUE_TYPE_MASK) == VALUE_TYPE_INT
            && value[arg].integerValue != 0) {
            interpretError("Invalid type given to settextcolor");
        }
    }

    float r = value[2].floatValue;
    float g = value[1].floatValue;
    float b = value[0].floatValue;

    if (!windowSetTextColor(r, g, b)) {
        interpretError("Error setting text color");
    }
}

// 0x459920
static void op_sayoptioncolor(Program* program)
{
    ProgramValue value[3];

    // NOTE: Original code does not use loops.
    for (int arg = 0; arg < 3; arg++) {
        value[arg] = program->stackPopValue();
    }

    for (int arg = 0; arg < 3; arg++) {
        if ((value[arg].opcode & VALUE_TYPE_MASK) != VALUE_TYPE_FLOAT
            && (value[arg].opcode & VALUE_TYPE_MASK) == VALUE_TYPE_INT
            && value[arg].integerValue != 0) {
            interpretError("Invalid type given to sayoptioncolor");
        }
    }

    float r = value[2].floatValue;
    float g = value[1].floatValue;
    float b = value[0].floatValue;

    if (dialogSetOptionColor(r, g, b)) {
        interpretError("Error setting option color");
    }
}

// 0x459A24
static void op_sayreplycolor(Program* program)
{
    ProgramValue value[3];

    // NOTE: Original code does not use loops.
    for (int arg = 0; arg < 3; arg++) {
        value[arg] = program->stackPopValue();
    }

    for (int arg = 0; arg < 3; arg++) {
        if ((value[arg].opcode & VALUE_TYPE_MASK) != VALUE_TYPE_FLOAT
            && (value[arg].opcode & VALUE_TYPE_MASK) == VALUE_TYPE_INT
            && value[arg].integerValue != 0) {
            interpretError("Invalid type given to sayreplycolor");
        }
    }

    float r = value[2].floatValue;
    float g = value[1].floatValue;
    float b = value[0].floatValue;

    if (dialogSetReplyColor(r, g, b) != 0) {
        interpretError("Error setting reply color");
    }
}

// 0x459B28
static void op_sethighlightcolor(Program* program)
{
    ProgramValue value[3];

    // NOTE: Original code does not use loops.
    for (int arg = 0; arg < 3; arg++) {
        value[arg] = program->stackPopValue();
    }

    for (int arg = 0; arg < 3; arg++) {
        if ((value[arg].opcode & VALUE_TYPE_MASK) != VALUE_TYPE_FLOAT
            && (value[arg].opcode & VALUE_TYPE_MASK) == VALUE_TYPE_INT
            && value[arg].integerValue != 0) {
            interpretError("Invalid type given to sayreplycolor");
        }
    }

    float r = value[2].floatValue;
    float g = value[1].floatValue;
    float b = value[0].floatValue;

    if (!windowSetHighlightColor(r, g, b)) {
        interpretError("Error setting text highlight color");
    }
}

// 0x459C2C
static void op_sayreplywindow(Program* program)
{
    ProgramValue v2 = program->stackPopValue();
    int height = program->stackPopInteger();
    int width = program->stackPopInteger();
    int y = program->stackPopInteger();
    int x = program->stackPopInteger();

    char* v1;
    if ((v2.opcode & VALUE_TYPE_MASK) == VALUE_TYPE_STRING) {
        v1 = program->getString(v2.opcode, v2.integerValue);
        v1 = interpretMangleName(v1);
        v1 = mystrdup(v1, __FILE__, __LINE__); // "..\\int\\INTLIB.C", 1510
    } else if ((v2.opcode & VALUE_TYPE_MASK) == VALUE_TYPE_INT && v2.integerValue == 0) {
        v1 = nullptr;
    } else {
        interpretError("Invalid arg 5 given to sayreplywindow");
    }

    if (dialogSetReplyWindow(x, y, width, height, v1) != 0) {
        interpretError("Error setting reply window");
    }
}

// 0x459D08
static void op_sayreplyflags(Program* program)
{
    int data = program->stackPopInteger();

    if (!dialogSetReplyFlags(data)) {
        interpretError("Error setting reply flags");
    }
}

// 0x459D54
static void op_sayoptionflags(Program* program)
{
    int data = program->stackPopInteger();

    if (!dialogSetOptionFlags(data)) {
        interpretError("Error setting option flags");
    }
}

// 0x459DA0
static void op_sayoptionwindow(Program* program)
{
    ProgramValue v2 = program->stackPopValue();
    int height = program->stackPopInteger();
    int width = program->stackPopInteger();
    int y = program->stackPopInteger();
    int x = program->stackPopInteger();

    char* v1;
    if ((v2.opcode & VALUE_TYPE_MASK) == VALUE_TYPE_STRING) {
        v1 = program->getString(v2.opcode, v2.integerValue);
        v1 = interpretMangleName(v1);
        v1 = mystrdup(v1, __FILE__, __LINE__); // "..\\int\\INTLIB.C", 1556
    } else if ((v2.opcode & VALUE_TYPE_MASK) == VALUE_TYPE_INT && v2.integerValue == 0) {
        v1 = nullptr;
    } else {
        interpretError("Invalid arg 5 given to sayoptionwindow");
    }

    if (dialogSetOptionWindow(x, y, width, height, v1) != 0) {
        interpretError("Error setting option window");
    }
}

// 0x459E7C
static void op_sayborder(Program* program)
{
    int y = program->stackPopInteger();
    int x = program->stackPopInteger();

    if (dialogSetBorder(x, y) != 0) {
        interpretError("Error setting dialog border");
    }
}

// 0x459F00
static void op_sayscrollup(Program* program)
{
    ProgramValue v6 = program->stackPopValue();
    ProgramValue v7 = program->stackPopValue();
    ProgramValue v8 = program->stackPopValue();
    ProgramValue v9 = program->stackPopValue();
    int y = program->stackPopInteger();
    int x = program->stackPopInteger();

    char* v1 = nullptr;
    char* v2 = nullptr;
    char* v3 = nullptr;
    char* v4 = nullptr;
    int v5 = 0;

    if ((v6.opcode & VALUE_TYPE_MASK) == VALUE_TYPE_INT) {
        if (v6.integerValue != -1 && v6.integerValue != 0) {
            interpretError("Invalid arg 4 given to sayscrollup");
        }

        if (v6.integerValue == -1) {
            v5 = 1;
        }
    } else {
        if ((v6.opcode & VALUE_TYPE_MASK) != VALUE_TYPE_STRING) {
            interpretError("Invalid arg 4 given to sayscrollup");
        }
    }

    if ((v7.opcode & VALUE_TYPE_MASK) != VALUE_TYPE_STRING && (v7.opcode & VALUE_TYPE_MASK) == VALUE_TYPE_INT && v7.integerValue != 0) {
        interpretError("Invalid arg 3 given to sayscrollup");
    }

    if ((v8.opcode & VALUE_TYPE_MASK) != VALUE_TYPE_STRING && (v8.opcode & VALUE_TYPE_MASK) == VALUE_TYPE_INT && v8.integerValue != 0) {
        interpretError("Invalid arg 2 given to sayscrollup");
    }

    if ((v9.opcode & VALUE_TYPE_MASK) != VALUE_TYPE_STRING && (v9.opcode & VALUE_TYPE_MASK) == VALUE_TYPE_INT && v9.integerValue != 0) {
        interpretError("Invalid arg 1 given to sayscrollup");
    }

    if ((v9.opcode & VALUE_TYPE_MASK) == VALUE_TYPE_STRING) {
        v1 = program->getString(v9.opcode, v9.integerValue);
        v1 = interpretMangleName(v1);
        v1 = mystrdup(v1, __FILE__, __LINE__); // "..\\int\\INTLIB.C", 1611
    }

    if ((v8.opcode & VALUE_TYPE_MASK) == VALUE_TYPE_STRING) {
        v2 = program->getString(v8.opcode, v8.integerValue);
        v2 = interpretMangleName(v2);
        v2 = mystrdup(v2, __FILE__, __LINE__); // "..\\int\\INTLIB.C", 1613
    }

    if ((v7.opcode & VALUE_TYPE_MASK) == VALUE_TYPE_STRING) {
        v3 = program->getString(v7.opcode, v7.integerValue);
        v3 = interpretMangleName(v3);
        v3 = mystrdup(v3, __FILE__, __LINE__); // "..\\int\\INTLIB.C", 1615
    }

    if ((v6.opcode & VALUE_TYPE_MASK) == VALUE_TYPE_STRING) {
        v4 = program->getString(v6.opcode, v6.integerValue);
        v4 = interpretMangleName(v4);
        v4 = mystrdup(v4, __FILE__, __LINE__); // "..\\int\\INTLIB.C", 1617
    }

    if (dialogSetScrollUp(x, y, v1, v2, v3, v4, v5) != 0) {
        interpretError("Error setting scroll up");
    }
}

// 0x45A1A0
static void op_sayscrolldown(Program* program)
{
    ProgramValue v6 = program->stackPopValue();
    ProgramValue v7 = program->stackPopValue();
    ProgramValue v8 = program->stackPopValue();
    ProgramValue v9 = program->stackPopValue();
    int y = program->stackPopInteger();
    int x = program->stackPopInteger();

    char* v1 = nullptr;
    char* v2 = nullptr;
    char* v3 = nullptr;
    char* v4 = nullptr;
    int v5 = 0;

    if ((v6.opcode & VALUE_TYPE_MASK) == VALUE_TYPE_INT) {
        if (v6.integerValue != -1 && v6.integerValue != 0) {
            // FIXME: Wrong function name, should be sayscrolldown.
            interpretError("Invalid arg 4 given to sayscrollup");
        }

        if (v6.integerValue == -1) {
            v5 = 1;
        }
    } else {
        if ((v6.opcode & VALUE_TYPE_MASK) != VALUE_TYPE_STRING) {
            // FIXME: Wrong function name, should be sayscrolldown.
            interpretError("Invalid arg 4 given to sayscrollup");
        }
    }

    if ((v7.opcode & VALUE_TYPE_MASK) != VALUE_TYPE_STRING && (v7.opcode & VALUE_TYPE_MASK) == VALUE_TYPE_INT && v7.integerValue != 0) {
        interpretError("Invalid arg 3 given to sayscrolldown");
    }

    if ((v8.opcode & VALUE_TYPE_MASK) != VALUE_TYPE_STRING && (v8.opcode & VALUE_TYPE_MASK) == VALUE_TYPE_INT && v8.integerValue != 0) {
        interpretError("Invalid arg 2 given to sayscrolldown");
    }

    if ((v9.opcode & VALUE_TYPE_MASK) != VALUE_TYPE_STRING && (v9.opcode & VALUE_TYPE_MASK) == VALUE_TYPE_INT && v9.integerValue != 0) {
        interpretError("Invalid arg 1 given to sayscrolldown");
    }

    if ((v9.opcode & VALUE_TYPE_MASK) == VALUE_TYPE_STRING) {
        v1 = program->getString(v9.opcode, v9.integerValue);
        v1 = interpretMangleName(v1);
        v1 = mystrdup(v1, __FILE__, __LINE__); // "..\\int\\INTLIB.C", 1652
    }

    if ((v8.opcode & VALUE_TYPE_MASK) == VALUE_TYPE_STRING) {
        v2 = program->getString(v8.opcode, v8.integerValue);
        v2 = interpretMangleName(v2);
        v2 = mystrdup(v2, __FILE__, __LINE__); // "..\\int\\INTLIB.C", 1654
    }

    if ((v7.opcode & VALUE_TYPE_MASK) == VALUE_TYPE_STRING) {
        v3 = program->getString(v7.opcode, v7.integerValue);
        v3 = interpretMangleName(v3);
        v3 = mystrdup(v3, __FILE__, __LINE__); // "..\\int\\INTLIB.C", 1656
    }

    if ((v6.opcode & VALUE_TYPE_MASK) == VALUE_TYPE_STRING) {
        v4 = program->getString(v6.opcode, v6.integerValue);
        v4 = interpretMangleName(v4);
        v4 = mystrdup(v4, __FILE__, __LINE__); // "..\\int\\INTLIB.C", 1658
    }

    if (dialogSetScrollDown(x, y, v1, v2, v3, v4, v5) != 0) {
        interpretError("Error setting scroll down");
    }
}

// 0x45A440
static void op_saysetspacing(Program* program)
{
    int data = program->stackPopInteger();

    if (dialogSetSpacing(data) != 0) {
        interpretError("Error setting option spacing");
    }
}

// 0x45A48C
static void op_sayrestart(Program* program)
{
    if (dialogRestart() != 0) {
        interpretError("Error restarting option");
    }
}

// 0x45A4A4
static void soundCallbackInterpret(void* userData, int a2)
{
    if (a2 == 1) {
        Sound** sound = reinterpret_cast<Sound**>(userData);
        *sound = nullptr;
    }
}

// 0x45A4B0
static int soundDeleteInterpret(int value)
{
    if (value == -1) {
        return 1;
    }

    if ((value & 0xA0000000) == 0) {
        return 0;
    }

    int index = value & ~0xA0000000;
    Sound* sound = interpretSounds[index];
    if (sound == nullptr) {
        return 0;
    }

    if (sound->isPlaying()) {
        sound->stop();
    }

    sound->destroy();

    interpretSounds[index] = nullptr;

    return 1;
}

// NOTE: Inlined.
//
// 0x45A528
void soundCloseInterpret()
{
    int index;

    for (index = 0; index < INT_LIB_SOUNDS_CAPACITY; index++) {
        if (interpretSounds[index] != nullptr) {
            soundDeleteInterpret(index | 0xA0000000);
        }
    }
}

// 0x45A550
int soundStartInterpret(char* fileName, int mode)
{
    int v3 = 1;
    int v5 = 0;

    if (mode & 0x01) {
        // looping
        v5 |= 0x20;
    } else {
        v3 = 5;
    }

    if (mode & 0x02) {
        v5 |= 0x08;
    } else {
        v5 |= 0x10;
    }

    if (mode & 0x0100) {
        // memory
        v3 &= ~0x03;
        v3 |= 0x01;
    }

    if (mode & 0x0200) {
        // streamed
        v3 &= ~0x03;
        v3 |= 0x02;
    }

    int index;
    for (index = 0; index < INT_LIB_SOUNDS_CAPACITY; index++) {
        if (interpretSounds[index] == nullptr) {
            break;
        }
    }

    if (index == INT_LIB_SOUNDS_CAPACITY) {
        return -1;
    }

    Sound* sound = interpretSounds[index] = soundAllocate(v3, v5);
    if (sound == nullptr) {
        return -1;
    }

    sound->setCallback(soundCallbackInterpret, &(interpretSounds[index]));

    if (mode & 0x01) {
        sound->setLoop(0xFFFF);
    }

    if (mode & 0x1000) {
        // mono
        sound->setChannel(2);
    }

    if (mode & 0x2000) {
        // stereo
        sound->setChannel(3);
    }

    int rc = sound->load(fileName);
    if (rc != SOUND_NO_ERROR) {
        sound->destroy();
        interpretSounds[index] = nullptr;
        return -1;
    }

    rc = sound->play();

    // TODO: Maybe wrong.
    switch (rc) {
    case SOUND_NO_DEVICE:
        debug_printf("soundPlay error: %s\n", "SOUND_NO_DEVICE");
        break;
    case SOUND_NOT_INITIALIZED:
        debug_printf("soundPlay error: %s\n", "SOUND_NOT_INITIALIZED");
        break;
    case SOUND_NO_SOUND:
        debug_printf("soundPlay error: %s\n", "SOUND_NO_SOUND");
        break;
    case SOUND_FUNCTION_NOT_SUPPORTED:
        debug_printf("soundPlay error: %s\n", "SOUND_FUNC_NOT_SUPPORTED");
        break;
    case SOUND_NO_BUFFERS_AVAILABLE:
        debug_printf("soundPlay error: %s\n", "SOUND_NO_BUFFERS_AVAILABLE");
        break;
    case SOUND_FILE_NOT_FOUND:
        debug_printf("soundPlay error: %s\n", "SOUND_FILE_NOT_FOUND");
        break;
    case SOUND_ALREADY_PLAYING:
        debug_printf("soundPlay error: %s\n", "SOUND_ALREADY_PLAYING");
        break;
    case SOUND_NOT_PLAYING:
        debug_printf("soundPlay error: %s\n", "SOUND_NOT_PLAYING");
        break;
    case SOUND_ALREADY_PAUSED:
        debug_printf("soundPlay error: %s\n", "SOUND_ALREADY_PAUSED");
        break;
    case SOUND_NOT_PAUSED:
        debug_printf("soundPlay error: %s\n", "SOUND_NOT_PAUSED");
        break;
    case SOUND_INVALID_HANDLE:
        debug_printf("soundPlay error: %s\n", "SOUND_INVALID_HANDLE");
        break;
    case SOUND_NO_MEMORY_AVAILABLE:
        debug_printf("soundPlay error: %s\n", "SOUND_NO_MEMORY");
        break;
    case SOUND_UNKNOWN_ERROR:
        debug_printf("soundPlay error: %s\n", "SOUND_ERROR");
        break;
    default:
        return index | 0xA0000000;
    }

    sound->destroy();
    interpretSounds[index] = nullptr;
    return -1;
}

// 0x45A99C
static int soundPauseInterpret(int value)
{
    if (value == -1) {
        return 1;
    }

    if ((value & 0xA0000000) == 0) {
        return 0;
    }

    int index = value & ~0xA0000000;
    Sound* sound = interpretSounds[index];
    if (sound == nullptr) {
        return 0;
    }

    int rc;
    if (sound->getType(0x01)) {
        rc = sound->stop();
    } else {
        rc = sound->pause();
    }
    return rc == SOUND_NO_ERROR;
}

// 0x45AA08
static int soundRewindInterpret(int value)
{
    if (value == -1) {
        return 1;
    }

    if ((value & 0xA0000000) == 0) {
        return 0;
    }

    int index = value & ~0xA0000000;
    Sound* sound = interpretSounds[index];
    if (sound == nullptr) {
        return 0;
    }

    if (!sound->isPlaying()) {
        return 1;
    }

    sound->stop();

    return sound->play() == SOUND_NO_ERROR;
}

// 0x45AA6C
static int soundUnpauseInterpret(int value)
{
    if (value == -1) {
        return 1;
    }

    if ((value & 0xA0000000) == 0) {
        return 0;
    }

    int index = value & ~0xA0000000;
    Sound* sound = interpretSounds[index];
    if (sound == nullptr) {
        return 0;
    }

    int rc;
    if (sound->getType(0x01)) {
        rc = sound->play();
    } else {
        rc = sound->unpause();
    }
    return rc == SOUND_NO_ERROR;
}

// 0x45AAD8
static void op_soundplay(Program* program)
{
    int flags = program->stackPopInteger();
    char* fileName = program->stackPopString();

    char* mangledFileName = interpretMangleName(fileName);
    int rc = soundStartInterpret(mangledFileName, flags);

    program->stackPushInteger(rc);
}

// 0x45AB6C
static void op_soundpause(Program* program)
{
    int data = program->stackPopInteger();
    soundPauseInterpret(data);
}

// 0x45ABA8
static void op_soundresume(Program* program)
{
    int data = program->stackPopInteger();
    soundUnpauseInterpret(data);
}

// 0x45ABE4
static void op_soundstop(Program* program)
{
    int data = program->stackPopInteger();
    soundPauseInterpret(data);
}

// 0x45AC20
static void op_soundrewind(Program* program)
{
    int data = program->stackPopInteger();
    soundRewindInterpret(data);
}

// 0x45AC5C
static void op_sounddelete(Program* program)
{
    int data = program->stackPopInteger();
    soundDeleteInterpret(data);
}

// 0x45AC98
static void op_setoneoptpause(Program* program)
{
    int data = program->stackPopInteger();

    if (data) {
        if ((dialogGetMediaFlag() & 8) == 0) {
            return;
        }
    } else {
        if ((dialogGetMediaFlag() & 8) != 0) {
            return;
        }
    }

    dialogToggleMediaFlag(8);
}

// 0x45ACF0
void updateIntLib()
{
    nevs_update();
    updateIntExtra();
}

// 0x45ACFC
void intlibClose()
{
    dialogClose();
    intExtraClose();

    // NOTE: Uninline.
    soundCloseInterpret();

    nevs_close();

    if (callbacks != nullptr) {
        myfree(callbacks, __FILE__, __LINE__); // "..\\int\\INTLIB.C", 1976
        callbacks = nullptr;
        numCallbacks = 0;
    }
}

// 0x45AD60
static bool intLibDoInput(int key)
{
    if (key < 0 || key >= INT_LIB_KEY_HANDLERS_CAPACITY) {
        return false;
    }

    if (anyKeyProg != nullptr) {
        if (anyKeyOffset != 0) {
            anyKeyProg->executeProc(anyKeyOffset);
        }
        return true;
    }

    IntLibKeyHandlerEntry* entry = &(inputProc[key]);
    if (entry->program == nullptr) {
        return false;
    }

    if (entry->proc != 0) {
        entry->program->executeProc(entry->proc);
    }

    return true;
}

// 0x45ADCC
void initIntlib()
{
    windowAddInputFunc(intLibDoInput);

    interpretAddFunc(0x806A, op_fillwin3x3);
    interpretAddFunc(0x808C, op_deletebutton);
    interpretAddFunc(0x8086, op_addbutton);
    interpretAddFunc(0x8088, op_addbuttonflag);
    interpretAddFunc(0x8087, op_addbuttontext);
    interpretAddFunc(0x8089, op_addbuttongfx);
    interpretAddFunc(0x808A, op_addbuttonproc);
    interpretAddFunc(0x808B, op_addbuttonrightproc);
    interpretAddFunc(0x8067, op_showwin);
    interpretAddFunc(0x8068, op_fillwin);
    interpretAddFunc(0x8069, op_fillrect);
    interpretAddFunc(0x8072, op_print);
    interpretAddFunc(0x8073, op_format);
    interpretAddFunc(0x8074, op_printrect);
    interpretAddFunc(0x8075, op_setfont);
    interpretAddFunc(0x8076, op_settextflags);
    interpretAddFunc(0x8077, op_settextcolor);
    interpretAddFunc(0x8078, op_sethighlightcolor);
    interpretAddFunc(0x8064, op_selectwin);
    interpretAddFunc(0x806B, op_display);
    interpretAddFunc(0x806D, op_displayraw);
    interpretAddFunc(0x806C, op_displaygfx);
    interpretAddFunc(0x806F, op_fadein);
    interpretAddFunc(0x8070, op_fadeout);
    interpretAddFunc(0x807A, op_playmovie);
    interpretAddFunc(0x807B, op_movieflags);
    interpretAddFunc(0x807C, op_playmovierect);
    interpretAddFunc(0x8079, op_stopmovie);
    interpretAddFunc(0x807F, op_addregion);
    interpretAddFunc(0x8080, op_addregionflag);
    interpretAddFunc(0x8081, op_addregionproc);
    interpretAddFunc(0x8082, op_addregionrightproc);
    interpretAddFunc(0x8083, op_deleteregion);
    interpretAddFunc(0x8084, op_activateregion);
    interpretAddFunc(0x8085, op_checkregion);
    interpretAddFunc(0x8062, op_createwin);
    interpretAddFunc(0x8063, op_deletewin);
    interpretAddFunc(0x8065, op_resizewin);
    interpretAddFunc(0x8066, op_scalewin);
    interpretAddFunc(0x804E, op_saystart);
    interpretAddFunc(0x804F, op_saystartpos);
    interpretAddFunc(0x8050, op_sayreplytitle);
    interpretAddFunc(0x8051, op_saygotoreply);
    interpretAddFunc(0x8053, op_sayoption);
    interpretAddFunc(0x8052, op_sayreply);
    interpretAddFunc(0x804D, op_sayend);
    interpretAddFunc(0x804C, op_sayquit);
    interpretAddFunc(0x8054, op_saymessage);
    interpretAddFunc(0x8055, op_sayreplywindow);
    interpretAddFunc(0x8056, op_sayoptionwindow);
    interpretAddFunc(0x805F, op_sayreplyflags);
    interpretAddFunc(0x8060, op_sayoptionflags);
    interpretAddFunc(0x8057, op_sayborder);
    interpretAddFunc(0x8058, op_sayscrollup);
    interpretAddFunc(0x8059, op_sayscrolldown);
    interpretAddFunc(0x805A, op_saysetspacing);
    interpretAddFunc(0x805B, op_sayoptioncolor);
    interpretAddFunc(0x805C, op_sayreplycolor);
    interpretAddFunc(0x805D, op_sayrestart);
    interpretAddFunc(0x805E, op_saygetlastpos);
    interpretAddFunc(0x8061, op_saymessagetimeout);
    interpretAddFunc(0x8071, op_gotoxy);
    interpretAddFunc(0x808D, op_hidemouse);
    interpretAddFunc(0x808E, op_showmouse);
    interpretAddFunc(0x8090, op_refreshmouse);
    interpretAddFunc(0x808F, op_mouseshape);
    interpretAddFunc(0x8091, op_setglobalmousefunc);
    interpretAddFunc(0x806E, op_loadpalettetable);
    interpretAddFunc(0x8092, op_addNamedEvent);
    interpretAddFunc(0x8093, op_addNamedHandler);
    interpretAddFunc(0x8094, op_clearNamed);
    interpretAddFunc(0x8095, op_signalNamed);
    interpretAddFunc(0x8096, op_addkey);
    interpretAddFunc(0x8097, op_deletekey);
    interpretAddFunc(0x8098, op_soundplay);
    interpretAddFunc(0x8099, op_soundpause);
    interpretAddFunc(0x809A, op_soundresume);
    interpretAddFunc(0x809B, op_soundstop);
    interpretAddFunc(0x809C, op_soundrewind);
    interpretAddFunc(0x809D, op_sounddelete);
    interpretAddFunc(0x809E, op_setoneoptpause);
    interpretAddFunc(0x809F, op_selectfilelist);
    interpretAddFunc(0x80A0, op_tokenize);

    nevs_initonce();
    initIntExtra();
    initDialog();
}

// 0x45B2C8
void interpretRegisterProgramDeleteCallback(IntLibProgramDeleteCallback* callback)
{
    int index;
    for (index = 0; index < numCallbacks; index++) {
        if (callbacks[index] == nullptr) {
            break;
        }
    }

    if (index == numCallbacks) {
        if (callbacks != nullptr) {
            callbacks = static_cast<IntLibProgramDeleteCallback**>(myrealloc(callbacks, sizeof(*callbacks) * (numCallbacks + 1), __FILE__, __LINE__)); // ..\\int\\INTLIB.C, 2110
        } else {
            callbacks = static_cast<IntLibProgramDeleteCallback**>(mymalloc(sizeof(*callbacks), __FILE__, __LINE__)); // ..\\int\\INTLIB.C, 2112
        }
        numCallbacks++;
    }

    callbacks[index] = callback;
}

// 0x45B39C
void removeProgramReferences(Program* program)
{
    for (int index = 0; index < INT_LIB_KEY_HANDLERS_CAPACITY; index++) {
        if (program == inputProc[index].program) {
            inputProc[index].program = nullptr;
        }
    }

    intExtraRemoveProgramReferences(program);

    for (int index = 0; index < numCallbacks; index++) {
        IntLibProgramDeleteCallback* callback = callbacks[index];
        if (callback != nullptr) {
            callback(program);
        }
    }
}

} // namespace fallout

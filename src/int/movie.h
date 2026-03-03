#pragma once


#include "game/enum_utils.h"
#include "plib/gnw/rect.h"

namespace fallout {

enum class MovieFlags : unsigned {
    Flag0x01 = 0x01,
    Flag0x02 = 0x02,
    Flag0x04 = 0x04,
    Flag0x08 = 0x08,
};

DEFINE_ENUM_FLAG_OPERATORS(MovieFlags)

inline constexpr int MOVIE_FLAG_0x01 = static_cast<int>(MovieFlags::Flag0x01);
inline constexpr int MOVIE_FLAG_0x02 = static_cast<int>(MovieFlags::Flag0x02);
inline constexpr int MOVIE_FLAG_0x04 = static_cast<int>(MovieFlags::Flag0x04);
inline constexpr int MOVIE_FLAG_0x08 = static_cast<int>(MovieFlags::Flag0x08);

enum class MovieExtendedFlags : unsigned {
    Flag0x01 = 0x01,
    Flag0x02 = 0x02,
    Flag0x04 = 0x04,
    Flag0x08 = 0x08,
    Flag0x10 = 0x10,
};

DEFINE_ENUM_FLAG_OPERATORS(MovieExtendedFlags)

inline constexpr int MOVIE_EXTENDED_FLAG_0x01 = static_cast<int>(MovieExtendedFlags::Flag0x01);
inline constexpr int MOVIE_EXTENDED_FLAG_0x02 = static_cast<int>(MovieExtendedFlags::Flag0x02);
inline constexpr int MOVIE_EXTENDED_FLAG_0x04 = static_cast<int>(MovieExtendedFlags::Flag0x04);
inline constexpr int MOVIE_EXTENDED_FLAG_0x08 = static_cast<int>(MovieExtendedFlags::Flag0x08);
inline constexpr int MOVIE_EXTENDED_FLAG_0x10 = static_cast<int>(MovieExtendedFlags::Flag0x10);


using MovieSubtitleFunc = char*(char* movieFilePath);
using MoviePaletteFunc = void(unsigned char* palette, int start, int end);
using MovieUpdateCallbackProc = void(int frame);
using MovieFrameGrabProc = void(unsigned char* data, int width, int height, int pitch);
using MovieCaptureFrameProc = void(unsigned char* data, int width, int height, int pitch, int movieX, int movieY, int movieWidth, int movieHeight);
using MoviePreDrawFunc = void(int win, Rect* rect);
using MovieStartFunc = void(int win);
using MovieEndFunc = void(int win, int x, int y, int width, int height);
using MovieFailedOpenFunc = int(char* path);

void movieSetPreDrawFunc(MoviePreDrawFunc* func);
void movieSetFailedOpenFunc(MovieFailedOpenFunc* func);
void movieSetFunc(MovieStartFunc* startFunc, MovieEndFunc* endFunc);
void movieSetFrameGrabFunc(MovieFrameGrabProc* func);
void movieSetCaptureFrameFunc(MovieCaptureFrameProc* func);
void initMovie();
void movieClose();
void movieStop();
int movieSetFlags(int a1);
void movieSetSubtitleFont(int font);
void movieSetSubtitleColor(float r, float g, float b);
void movieSetPaletteFunc(MoviePaletteFunc* func);
void movieSetCallback(MovieUpdateCallbackProc* func);
int movieRun(int win, char* filePath);
int movieRunRect(int win, char* filePath, int a3, int a4, int a5, int a6);
void movieSetSubtitleFunc(MovieSubtitleFunc* proc);
void movieSetVolume(int volume);
void movieUpdate();
int moviePlaying();

} // namespace fallout

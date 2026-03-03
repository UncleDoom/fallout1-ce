#pragma once


#include "game/enum_utils.h"
#include "plib/db/db.h"

namespace fallout {

enum class GameMovieFlags : unsigned {
    FadeIn = 0x01,
    FadeOut = 0x02,
    StopMusic = 0x04,
    PauseMusic = 0x08,
};
DEFINE_ENUM_FLAG_OPERATORS(GameMovieFlags)

inline constexpr int GAME_MOVIE_FADE_IN = static_cast<int>(GameMovieFlags::FadeIn);
inline constexpr int GAME_MOVIE_FADE_OUT = static_cast<int>(GameMovieFlags::FadeOut);
inline constexpr int GAME_MOVIE_STOP_MUSIC = static_cast<int>(GameMovieFlags::StopMusic);
inline constexpr int GAME_MOVIE_PAUSE_MUSIC = static_cast<int>(GameMovieFlags::PauseMusic);


enum class GameMovie : int {
    IpLogo = 0,
    MpLogo = 1,
    Intro = 2,
    VExpld = 3,
    CathExp = 4,
    OvrIntro = 5,
    Boil3 = 6,
    OvrRun = 7,
    WalkM = 8,
    WalkW = 9,
    DipedV = 10,
    Boil1 = 11,
    Boil2 = 12,
    RaeKills = 13,
    Count = 14,
};

inline constexpr int MOVIE_IPLOGO = static_cast<int>(GameMovie::IpLogo);
inline constexpr int MOVIE_MPLOGO = static_cast<int>(GameMovie::MpLogo);
inline constexpr int MOVIE_INTRO = static_cast<int>(GameMovie::Intro);
inline constexpr int MOVIE_VEXPLD = static_cast<int>(GameMovie::VExpld);
inline constexpr int MOVIE_CATHEXP = static_cast<int>(GameMovie::CathExp);
inline constexpr int MOVIE_OVRINTRO = static_cast<int>(GameMovie::OvrIntro);
inline constexpr int MOVIE_BOIL3 = static_cast<int>(GameMovie::Boil3);
inline constexpr int MOVIE_OVRRUN = static_cast<int>(GameMovie::OvrRun);
inline constexpr int MOVIE_WALKM = static_cast<int>(GameMovie::WalkM);
inline constexpr int MOVIE_WALKW = static_cast<int>(GameMovie::WalkW);
inline constexpr int MOVIE_DIPEDV = static_cast<int>(GameMovie::DipedV);
inline constexpr int MOVIE_BOIL1 = static_cast<int>(GameMovie::Boil1);
inline constexpr int MOVIE_BOIL2 = static_cast<int>(GameMovie::Boil2);
inline constexpr int MOVIE_RAEKILLS = static_cast<int>(GameMovie::RaeKills);
inline constexpr int MOVIE_COUNT = static_cast<int>(GameMovie::Count);

int gmovie_init();
void gmovie_reset();
void gmovie_exit();
int gmovie_load(DB_FILE* stream);
int gmovie_save(DB_FILE* stream);
int gmovie_play(int game_movie, int game_movie_flags);
bool gmovie_has_been_played(int game_movie);

} // namespace fallout

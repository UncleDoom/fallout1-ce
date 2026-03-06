#include "plib/gnw/winmain.h"

#include <cstdlib>
#include <memory>

#include <SDL.h>

#ifndef _WIN32
#include <unistd.h>
#endif

#include "game/main.h"
#include "plib/gnw/gnw.h"
#include "plib/gnw/svga.h"

#if __APPLE__ && TARGET_OS_IOS
#include "platform/ios/paths.h"
#endif

namespace fallout {

// Platform detection as constexpr booleans for use in runtime branches.
// Preprocessor #if is still required for include-level gating and
// platform-specific types, but runtime logic can use these.
inline constexpr bool kIsWindows =
#if _WIN32
    true;
#else
    false;
#endif

inline constexpr bool kIsIOS =
#if __APPLE__ && TARGET_OS_IOS
    true;
#else
    false;
#endif

inline constexpr bool kIsMacOS =
#if __APPLE__ && TARGET_OS_OSX
    true;
#else
    false;
#endif

inline constexpr bool kIsAndroid =
#if __ANDROID__
    true;
#else
    false;
#endif

// RAII wrapper for SDL initialization/teardown.
class SdlContext {
public:
    SdlContext() noexcept = default;
    ~SdlContext() = default;

    SdlContext(const SdlContext&) = delete;
    SdlContext& operator=(const SdlContext&) = delete;

    void hideCursor() noexcept
    {
        SDL_ShowCursor(SDL_DISABLE);
    }
};

#if _WIN32
// RAII wrapper for a Windows named mutex.
class NamedMutex {
public:
    explicit NamedMutex(const char* name) noexcept
        : handle_(CreateMutexA(nullptr, TRUE, name))
    {
        valid_ = (GetLastError() == ERROR_SUCCESS);
    }

    ~NamedMutex()
    {
        if (handle_ != nullptr) {
            CloseHandle(handle_);
        }
    }

    NamedMutex(const NamedMutex&) = delete;
    NamedMutex& operator=(const NamedMutex&) = delete;

    [[nodiscard]] bool isValid() const noexcept { return valid_; }

private:
    HANDLE handle_ = nullptr;
    bool valid_ = false;
};
#endif

static bool override_base_path = true;

// Performs platform-specific initialization (working directory, touch hints).
static void platformInit() noexcept
{
#if __APPLE__ && TARGET_OS_IOS
    SDL_SetHint(SDL_HINT_MOUSE_TOUCH_EVENTS, "0");
    SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");
    chdir(iOSGetDocumentsPath());
#endif

#if __APPLE__ && TARGET_OS_OSX
    if (override_base_path) {
        char* basePath = SDL_GetBasePath();
        chdir(basePath);
        SDL_free(basePath);
    }
#endif

#if __ANDROID__
    SDL_SetHint(SDL_HINT_MOUSE_TOUCH_EVENTS, "0");
    SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");
    chdir(SDL_AndroidGetExternalStoragePath());
#endif
}

// 0x53A290
bool GNW95_isActive = false;

#if _WIN32
// 0x53A294
HANDLE GNW95_mutex = nullptr;
#endif

// 0x6B0760
char GNW95_title[256];

void parse_arguments(int argc, char* argv[]) {
    if (argc <= 1) {
        return;
    }
    for (size_t i = 1; i < argc; i++) {
        if (strcmp("--keep-pwd", argv[i]) == 0) {
            override_base_path = false;
        }
    }

}

int main(int argc, char* argv[])
{
#if _WIN32
    NamedMutex mutex("GNW95MUTEX");
    if (!mutex.isValid()) {
        return 0;
    }
#endif
    parse_arguments(argc, argv);

    platformInit();

    SdlContext sdl;
    sdl.hideCursor();

    GNW95_isActive = true;
    const int rc = gnw_main(argc, argv);

    return rc;
}

} // namespace fallout

int main(int argc, char* argv[])
{
    return fallout::main(argc, argv);
}

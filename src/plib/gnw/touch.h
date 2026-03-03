#pragma once


#include <SDL.h>

namespace fallout {

enum class GestureType : int {
    Unrecognized = 0,
    Tap = 1,
    LongPress = 2,
    Pan = 3,
};

inline constexpr int kUnrecognized = static_cast<int>(GestureType::Unrecognized);
inline constexpr int kTap = static_cast<int>(GestureType::Tap);
inline constexpr int kLongPress = static_cast<int>(GestureType::LongPress);
inline constexpr int kPan = static_cast<int>(GestureType::Pan);

enum class GestureState : int {
    Possible = 0,
    Began = 1,
    Changed = 2,
    Ended = 3,
};

inline constexpr int kPossible = static_cast<int>(GestureState::Possible);
inline constexpr int kBegan = static_cast<int>(GestureState::Began);
inline constexpr int kChanged = static_cast<int>(GestureState::Changed);
inline constexpr int kEnded = static_cast<int>(GestureState::Ended);

struct Gesture {
    GestureType type;
    GestureState state;
    int numberOfTouches;
    int x;
    int y;
};

void touch_handle_start(SDL_TouchFingerEvent* event);
void touch_handle_move(SDL_TouchFingerEvent* event);
void touch_handle_end(SDL_TouchFingerEvent* event);
void touch_process_gesture();
bool touch_get_gesture(Gesture* gesture);

} // namespace fallout

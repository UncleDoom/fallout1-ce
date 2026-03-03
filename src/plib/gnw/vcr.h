#pragma once


#include "plib/db/db.h"

namespace fallout {

inline constexpr int VCR_BUFFER_CAPACITY = 4096;

enum class VcrState : int {
    Recording = 0,
    Playing = 1,
    TurnedOff = 2,
};

inline constexpr int VCR_STATE_RECORDING = static_cast<int>(VcrState::Recording);
inline constexpr int VCR_STATE_PLAYING = static_cast<int>(VcrState::Playing);
inline constexpr int VCR_STATE_TURNED_OFF = static_cast<int>(VcrState::TurnedOff);

inline constexpr unsigned int VCR_STATE_STOP_REQUESTED = 0x80000000;

enum class VcrTerminationFlags : unsigned {
    // Specifies that VCR playback should stop if any key is pressed.
    OnKeyPress = 0x01,

    // Specifies that VCR playback should stop if mouse is moved.
    OnMouseMove = 0x02,

    // Specifies that VCR playback should stop if any mouse button is pressed.
    OnMousePress = 0x04,
};

inline constexpr int VCR_TERMINATE_ON_KEY_PRESS = static_cast<int>(VcrTerminationFlags::OnKeyPress);
inline constexpr int VCR_TERMINATE_ON_MOUSE_MOVE = static_cast<int>(VcrTerminationFlags::OnMouseMove);
inline constexpr int VCR_TERMINATE_ON_MOUSE_PRESS = static_cast<int>(VcrTerminationFlags::OnMousePress);


enum class VcrPlaybackCompletionReason : int {
    None = 0,

    // Indicates that VCR playback completed normally.
    Completed = 1,

    // Indicates that VCR playback terminated according to termination flags.
    Terminated = 2,
};

inline constexpr int VCR_PLAYBACK_COMPLETION_REASON_NONE = static_cast<int>(VcrPlaybackCompletionReason::None);
inline constexpr int VCR_PLAYBACK_COMPLETION_REASON_COMPLETED = static_cast<int>(VcrPlaybackCompletionReason::Completed);
inline constexpr int VCR_PLAYBACK_COMPLETION_REASON_TERMINATED = static_cast<int>(VcrPlaybackCompletionReason::Terminated);

enum class VcrEntryType : int {
    None = 0,
    InitialState = 1,
    KeyboardEvent = 2,
    MouseEvent = 3,
};

inline constexpr int VCR_ENTRY_TYPE_NONE = static_cast<int>(VcrEntryType::None);
inline constexpr int VCR_ENTRY_TYPE_INITIAL_STATE = static_cast<int>(VcrEntryType::InitialState);
inline constexpr int VCR_ENTRY_TYPE_KEYBOARD_EVENT = static_cast<int>(VcrEntryType::KeyboardEvent);
inline constexpr int VCR_ENTRY_TYPE_MOUSE_EVENT = static_cast<int>(VcrEntryType::MouseEvent);

class VcrEntry {
public:
    unsigned int type;
    unsigned int time;
    unsigned int counter;
    union {
        struct {
            int mouseX;
            int mouseY;
            int keyboardLayout;
        } initial;
        struct {
            short key;
        } keyboardEvent;
        struct {
            int dx;
            int dy;
            int buttons;
        } mouseEvent;
    };

    bool save(DB_FILE* stream);
    bool load(DB_FILE* stream);
};

using VcrPlaybackCompletionCallback = void(int reason);

extern VcrEntry* vcr_buffer;
extern int vcr_buffer_index;
extern unsigned int vcr_state;
extern unsigned int vcr_time;
extern unsigned int vcr_counter;
extern unsigned int vcr_terminate_flags;
extern int vcr_terminated_condition;

bool vcr_record(const char* fileName);
bool vcr_play(const char* fileName, unsigned int terminationFlags, VcrPlaybackCompletionCallback* callback);
void vcr_stop();
int vcr_status();
int vcr_update();
bool vcr_dump_buffer();

} // namespace fallout

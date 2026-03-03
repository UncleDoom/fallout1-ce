#include "game/selfrun.h"

#include <cstdlib>
#include <cstring>

#include "game/game.h"
#include "game/gconfig.h"
#include "platform_compat.h"
#include "plib/db/db.h"
#include "plib/gnw/input.h"
#include "plib/gnw/svga.h"
#include "plib/gnw/vcr.h"

namespace fallout {

enum SelfrunState {
    SELFRUN_STATE_TURNED_OFF,
    SELFRUN_STATE_PLAYING,
    SELFRUN_STATE_RECORDING,
};

static void selfrun_playback_callback(int reason);

// 0x507A6C
static int selfrun_state = SELFRUN_STATE_TURNED_OFF;

// 0x496D60
int selfrun_get_list(char*** fileListPtr, int* fileListLengthPtr)
{
    if (fileListPtr == nullptr) {
        return -1;
    }

    if (fileListLengthPtr == nullptr) {
        return -1;
    }

    *fileListLengthPtr = db_get_file_list("selfrun\\*.sdf", fileListPtr, nullptr, 0);

    return 0;
}

// 0x496D90
int selfrun_free_list(char*** fileListPtr)
{
    if (fileListPtr == nullptr) {
        return -1;
    }

    db_free_file_list(fileListPtr, nullptr);

    return 0;
}

// 0x496DA8
int SelfrunData::prepPlayback(const char* fileName)
{
    if (fileName == nullptr) {
        return -1;
    }

    if (vcr_status() != VCR_STATE_TURNED_OFF) {
        return -1;
    }

    if (selfrun_state != SELFRUN_STATE_TURNED_OFF) {
        return -1;
    }

    char path[COMPAT_MAX_PATH];
    snprintf(path, sizeof(path), "%s%s", "selfrun\\", fileName);

    if (loadData(path) != 0) {
        return -1;
    }

    selfrun_state = SELFRUN_STATE_PLAYING;

    return 0;
}

// 0x496E08
void SelfrunData::playbackLoop()
{
    if (selfrun_state == SELFRUN_STATE_PLAYING) {
        char path[COMPAT_MAX_PATH];
        snprintf(path, sizeof(path), "%s%s", "selfrun\\", recordingFileName);

        if (vcr_play(path, VCR_TERMINATE_ON_KEY_PRESS | VCR_TERMINATE_ON_MOUSE_PRESS, selfrun_playback_callback)) {
            bool cursorWasHidden = mouse_hidden();
            if (cursorWasHidden) {
                mouse_show();
            }

            while (selfrun_state == SELFRUN_STATE_PLAYING) {
                sharedFpsLimiter.mark();

                int keyCode = get_input();
                if (keyCode != stopKeyCode) {
                    game_handle_input(keyCode, false);
                }

                renderPresent();
                sharedFpsLimiter.throttle();
            }

            while (mouse_get_buttons() != 0) {
                sharedFpsLimiter.mark();

                get_input();

                renderPresent();
                sharedFpsLimiter.throttle();
            }

            if (cursorWasHidden) {
                mouse_hide();
            }
        }
    }
}

// 0x496EA8
int SelfrunData::prepRecording(const char* recordingName, const char* mapFileName)
{
    if (recordingName == nullptr) {
        return -1;
    }

    if (mapFileName == nullptr) {
        return -1;
    }

    if (vcr_status() != VCR_STATE_TURNED_OFF) {
        return -1;
    }

    if (selfrun_state != SELFRUN_STATE_TURNED_OFF) {
        return -1;
    }

    snprintf(recordingFileName, sizeof(recordingFileName), "%s%s", recordingName, ".vcr");
    strcpy(this->mapFileName, mapFileName);

    stopKeyCode = KEY_CTRL_R;

    char path[COMPAT_MAX_PATH];
    snprintf(path, sizeof(path), "%s%s%s", "selfrun\\", recordingName, ".sdf");

    if (saveData(path) != 0) {
        return -1;
    }

    selfrun_state = SELFRUN_STATE_RECORDING;

    return 0;
}

// 0x496F5C
void SelfrunData::recordingLoop()
{
    if (selfrun_state == SELFRUN_STATE_RECORDING) {
        char path[COMPAT_MAX_PATH];
        snprintf(path, sizeof(path), "%s%s", "selfrun\\", recordingFileName);
        if (vcr_record(path)) {
            if (!mouse_hidden()) {
                mouse_show();
            }

            bool done = false;
            while (!done) {
                sharedFpsLimiter.mark();

                int keyCode = get_input();
                if (keyCode == stopKeyCode) {
                    vcr_stop();
                    game_user_wants_to_quit = 2;
                    done = true;
                } else {
                    game_handle_input(keyCode, false);
                }

                renderPresent();
                sharedFpsLimiter.throttle();
            }
        }
        selfrun_state = SELFRUN_STATE_TURNED_OFF;
    }
}

// 0x496FF4
static void selfrun_playback_callback(int reason)
{
    game_user_wants_to_quit = 2;
    selfrun_state = SELFRUN_STATE_TURNED_OFF;
}

// 0x49700C
int SelfrunData::loadData(const char* path)
{
    if (path == nullptr) {
        return -1;
    }

    DB_FILE* stream = db_fopen(path, "rb");
    if (stream == nullptr) {
        return -1;
    }

    int rc = -1;
    if (stream->freadInt8List(recordingFileName, SELFRUN_RECORDING_FILE_NAME_LENGTH) == 0
        && stream->freadInt8List(mapFileName, SELFRUN_MAP_FILE_NAME_LENGTH) == 0
        && stream->freadInt32(&stopKeyCode) == 0) {
        rc = 0;
    }

    stream->fclose();

    return rc;
}

// 0x497074
int SelfrunData::saveData(const char* path)
{
    if (path == nullptr) {
        return -1;
    }

    char* masterPatches;
    game_config.getString(GAME_CONFIG_SYSTEM_KEY, GAME_CONFIG_MASTER_PATCHES_KEY, &masterPatches);

    char selfrunDirectoryPath[COMPAT_MAX_PATH];
    snprintf(selfrunDirectoryPath, sizeof(selfrunDirectoryPath), "%s\\%s", masterPatches, "selfrun\\");

    compat_mkdir(selfrunDirectoryPath);

    DB_FILE* stream = db_fopen(path, "wb");
    if (stream == nullptr) {
        return -1;
    }

    int rc = -1;
    if (stream->fwriteInt8List(recordingFileName, SELFRUN_RECORDING_FILE_NAME_LENGTH) == 0
        && stream->fwriteInt8List(mapFileName, SELFRUN_MAP_FILE_NAME_LENGTH) == 0
        && stream->fwriteInt32(stopKeyCode) == 0) {
        rc = 0;
    }

    stream->fclose();

    return rc;
}

} // namespace fallout

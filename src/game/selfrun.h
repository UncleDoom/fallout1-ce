#pragma once


namespace fallout {

inline constexpr int SELFRUN_RECORDING_FILE_NAME_LENGTH = 13;
inline constexpr int SELFRUN_MAP_FILE_NAME_LENGTH = 13;

class SelfrunData {
public:
    char recordingFileName[SELFRUN_RECORDING_FILE_NAME_LENGTH];
    char mapFileName[SELFRUN_MAP_FILE_NAME_LENGTH];
    int stopKeyCode;

    int prepPlayback(const char* fileName);
    void playbackLoop();
    int prepRecording(const char* recordingName, const char* mapFileName);
    void recordingLoop();
    int loadData(const char* path);
    int saveData(const char* path);
};

int selfrun_get_list(char*** fileListPtr, int* fileListLengthPtr);
int selfrun_free_list(char*** fileListPtr);

} // namespace fallout

#pragma once


#include <cstddef>

#include "game/enum_utils.h"

namespace fallout {

inline constexpr int VOLUME_MIN = 0;
inline constexpr int VOLUME_MAX = 0x7FFF;

enum class SoundError : int {
    NoError = 0,
    SosDriverNotLoaded = 1,
    SosInvalidPointer = 2,
    SosDetectInitialized = 3,
    SosFailOnFileOpen = 4,
    SosMemoryFail = 5,
    SosInvalidDriverId = 6,
    SosNoDriverFound = 7,
    SosDetectionFailure = 8,
    SosDriverLoaded = 9,
    SosInvalidHandle = 10,
    SosNoHandles = 11,
    SosPaused = 12,
    SosNoPaused = 13,
    SosInvalidData = 14,
    SosDrvFileFail = 15,
    SosInvalidPort = 16,
    SosInvalidIrq = 17,
    SosInvalidDma = 18,
    SosInvalidDmaIrq = 19,
    NoDevice = 20,
    NotInitialized = 21,
    NoSound = 22,
    FunctionNotSupported = 23,
    NoBuffersAvailable = 24,
    FileNotFound = 25,
    AlreadyPlaying = 26,
    NotPlaying = 27,
    AlreadyPaused = 28,
    NotPaused = 29,
    InvalidHandle = 30,
    NoMemoryAvailable = 31,
    UnknownError = 32,
    Count,
};

inline constexpr int SOUND_NO_ERROR = static_cast<int>(SoundError::NoError);
inline constexpr int SOUND_SOS_DRIVER_NOT_LOADED = static_cast<int>(SoundError::SosDriverNotLoaded);
inline constexpr int SOUND_SOS_INVALID_POINTER = static_cast<int>(SoundError::SosInvalidPointer);
inline constexpr int SOUND_SOS_DETECT_INITIALIZED = static_cast<int>(SoundError::SosDetectInitialized);
inline constexpr int SOUND_SOS_FAIL_ON_FILE_OPEN = static_cast<int>(SoundError::SosFailOnFileOpen);
inline constexpr int SOUND_SOS_MEMORY_FAIL = static_cast<int>(SoundError::SosMemoryFail);
inline constexpr int SOUND_SOS_INVALID_DRIVER_ID = static_cast<int>(SoundError::SosInvalidDriverId);
inline constexpr int SOUND_SOS_NO_DRIVER_FOUND = static_cast<int>(SoundError::SosNoDriverFound);
inline constexpr int SOUND_SOS_DETECTION_FAILURE = static_cast<int>(SoundError::SosDetectionFailure);
inline constexpr int SOUND_SOS_DRIVER_LOADED = static_cast<int>(SoundError::SosDriverLoaded);
inline constexpr int SOUND_SOS_INVALID_HANDLE = static_cast<int>(SoundError::SosInvalidHandle);
inline constexpr int SOUND_SOS_NO_HANDLES = static_cast<int>(SoundError::SosNoHandles);
inline constexpr int SOUND_SOS_PAUSED = static_cast<int>(SoundError::SosPaused);
inline constexpr int SOUND_SOS_NO_PAUSED = static_cast<int>(SoundError::SosNoPaused);
inline constexpr int SOUND_SOS_INVALID_DATA = static_cast<int>(SoundError::SosInvalidData);
inline constexpr int SOUND_SOS_DRV_FILE_FAIL = static_cast<int>(SoundError::SosDrvFileFail);
inline constexpr int SOUND_SOS_INVALID_PORT = static_cast<int>(SoundError::SosInvalidPort);
inline constexpr int SOUND_SOS_INVALID_IRQ = static_cast<int>(SoundError::SosInvalidIrq);
inline constexpr int SOUND_SOS_INVALID_DMA = static_cast<int>(SoundError::SosInvalidDma);
inline constexpr int SOUND_SOS_INVALID_DMA_IRQ = static_cast<int>(SoundError::SosInvalidDmaIrq);
inline constexpr int SOUND_NO_DEVICE = static_cast<int>(SoundError::NoDevice);
inline constexpr int SOUND_NOT_INITIALIZED = static_cast<int>(SoundError::NotInitialized);
inline constexpr int SOUND_NO_SOUND = static_cast<int>(SoundError::NoSound);
inline constexpr int SOUND_FUNCTION_NOT_SUPPORTED = static_cast<int>(SoundError::FunctionNotSupported);
inline constexpr int SOUND_NO_BUFFERS_AVAILABLE = static_cast<int>(SoundError::NoBuffersAvailable);
inline constexpr int SOUND_FILE_NOT_FOUND = static_cast<int>(SoundError::FileNotFound);
inline constexpr int SOUND_ALREADY_PLAYING = static_cast<int>(SoundError::AlreadyPlaying);
inline constexpr int SOUND_NOT_PLAYING = static_cast<int>(SoundError::NotPlaying);
inline constexpr int SOUND_ALREADY_PAUSED = static_cast<int>(SoundError::AlreadyPaused);
inline constexpr int SOUND_NOT_PAUSED = static_cast<int>(SoundError::NotPaused);
inline constexpr int SOUND_INVALID_HANDLE = static_cast<int>(SoundError::InvalidHandle);
inline constexpr int SOUND_NO_MEMORY_AVAILABLE = static_cast<int>(SoundError::NoMemoryAvailable);
inline constexpr int SOUND_UNKNOWN_ERROR = static_cast<int>(SoundError::UnknownError);
inline constexpr int SOUND_ERR_COUNT = static_cast<int>(SoundError::Count);

using SoundMallocFunc = void*(size_t size);
using SoundReallocFunc = void*(void* ptr, size_t size);
using SoundFreeFunc = void(void* ptr);
using SoundFileNameMangler = char*(char*);
using SoundOpenProc = int(const char* filePath, int flags);
using SoundCloseProc = int(int fileHandle);
using SoundReadProc = int(int fileHandle, void* buf, unsigned int size);
using SoundWriteProc = int(int fileHandle, const void* buf, unsigned int size);
using SoundSeekProc = long(int fileHandle, long offset, int origin);
using SoundTellProc = long(int fileHandle);
using SoundFileLengthProc = long(int fileHandle);

struct SoundFileIO {
    SoundOpenProc* open;
    SoundCloseProc* close;
    SoundReadProc* read;
    SoundWriteProc* write;
    SoundSeekProc* seek;
    SoundTellProc* tell;
    SoundFileLengthProc* filelength;
    int fd;
};

enum class SoundType : unsigned {
    Memory = 0x01,
    Streaming = 0x02,
    FireAndForget = 0x04,
    Infinite = 0x10,
    Type0x20 = 0x20,
};

DEFINE_ENUM_FLAG_OPERATORS(SoundType)

inline constexpr int SOUND_TYPE_MEMORY = static_cast<int>(SoundType::Memory);
inline constexpr int SOUND_TYPE_STREAMING = static_cast<int>(SoundType::Streaming);
inline constexpr int SOUND_TYPE_FIRE_AND_FORGET = static_cast<int>(SoundType::FireAndForget);
inline constexpr int SOUND_TYPE_INFINITE = static_cast<int>(SoundType::Infinite);
inline constexpr int SOUND_TYPE_0x20 = static_cast<int>(SoundType::Type0x20);

enum class SoundFlags : unsigned {
    Flag0x02 = 0x02,
    Flag0x04 = 0x04,
    Bits16 = 0x08,
    Bits8 = 0x10,
    Looping = 0x20,
    Flag0x80 = 0x80,
    Flag0x100 = 0x100,
    Flag0x200 = 0x200,
};

DEFINE_ENUM_FLAG_OPERATORS(SoundFlags)

inline constexpr int SOUND_FLAG_0x02 = static_cast<int>(SoundFlags::Flag0x02);
inline constexpr int SOUND_FLAG_0x04 = static_cast<int>(SoundFlags::Flag0x04);
inline constexpr int SOUND_16BIT = static_cast<int>(SoundFlags::Bits16);
inline constexpr int SOUND_8BIT = static_cast<int>(SoundFlags::Bits8);
inline constexpr int SOUND_LOOPING = static_cast<int>(SoundFlags::Looping);
inline constexpr int SOUND_FLAG_0x80 = static_cast<int>(SoundFlags::Flag0x80);
inline constexpr int SOUND_FLAG_0x100 = static_cast<int>(SoundFlags::Flag0x100);
inline constexpr int SOUND_FLAG_0x200 = static_cast<int>(SoundFlags::Flag0x200);


using SoundCallback = void(void* userData, int a2);
using SoundDeleteCallback = void(void* userData);

class Sound {
public:
    SoundFileIO io;
    unsigned char* data;
    int soundBuffer;
    int bitsPerSample;
    int channels;
    int rate;
    int soundFlags;
    int statusFlags;
    int type;
    int pausePos;
    int volume;
    int loops;
    int field_54;
    int field_58;
    int minReadBuffer;
    int fileSize;
    int numBytesRead;
    int field_68;
    int readLimit;
    int lastUpdate;
    unsigned int lastPosition;
    int numBuffers;
    int dataSize;
    int field_80;
    void* callbackUserData;
    SoundCallback* callback;
    void* deleteUserData;
    SoundDeleteCallback* deleteCallback;
    Sound* next;
    Sound* prev;

    int load(char* filePath);
    int rewind();
    int setData(unsigned char* buf, int size);
    int play();
    int stop();
    int destroy();
    int advance();
    bool isPlaying();
    bool isDone();
    bool isFading();
    bool isPaused();
    int getFlags(int flags);
    int getType(int type);
    int length();
    int setLoop(int loops);
    int setVolume(int volume);
    int getVolume();
    int setCallback(SoundCallback* callback, void* userData);
    int setChannel(int channels);
    int setReadLimit(int readLimit);
    int pause();
    int unpause();
    int setFileIO(SoundOpenProc* openProc, SoundCloseProc* closeProc, SoundReadProc* readProc, SoundWriteProc* writeProc, SoundSeekProc* seekProc, SoundTellProc* tellProc, SoundFileLengthProc* fileLengthProc);
    void mgrDelete();
    int getPosition();
    int setPosition(int pos);
    int fade(int duration, int targetVolume);

private:
    void refreshBuffers();
    int preloadBuffers();
    int addData(unsigned char* buf, int size);
    int internalFade(int duration, int targetVolume, int a4);
};

void soundRegisterAlloc(SoundMallocFunc* mallocProc, SoundReallocFunc* reallocProc, SoundFreeFunc* freeProc);
const char* soundError(int err);
int soundInit(int a1, int a2, int a3, int a4, int rate);
void soundClose();
Sound* soundAllocate(int a1, int a2);
int numSoundsPlaying();
int soundVolumeHMItoDirectSound(int a1);
int soundSetMasterVolume(int value);
void soundFlushAllSounds();
void soundUpdate();
int soundSetDefaultFileIO(SoundOpenProc* openProc, SoundCloseProc* closeProc, SoundReadProc* readProc, SoundWriteProc* writeProc, SoundSeekProc* seekProc, SoundTellProc* tellProc, SoundFileLengthProc* fileLengthProc);

} // namespace fallout

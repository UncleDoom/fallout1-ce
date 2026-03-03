#include "int/sound.h"

#include <cstdlib>
#include <cstring>

#ifdef _WIN32
#include <io.h>
#else
#include <fcntl.h>
#include <unistd.h>
#endif

#include <algorithm>

#include <SDL.h>

#include "audio_engine.h"
#include "platform_compat.h"
#include "plib/gnw/debug.h"
#include "plib/gnw/memory.h"
#include "plib/gnw/winmain.h"

namespace fallout {

enum SoundStatusFlags {
    SOUND_STATUS_DONE = 0x01,
    SOUND_STATUS_IS_PLAYING = 0x02,
    SOUND_STATUS_IS_FADING = 0x04,
    SOUND_STATUS_IS_PAUSED = 0x08,
};

struct FadeSound {
    Sound* sound;
    int deltaVolume;
    int targetVolume;
    int initialVolume;
    int currentVolume;
    int field_14;
    struct FadeSound* prev;
    struct FadeSound* next;
};

static void* defaultMalloc(size_t size);
static void* defaultRealloc(void* ptr, size_t size);
static void defaultFree(void* ptr);
static long soundFileSize(int fileHandle);
static long soundTellData(int fileHandle);
static int soundWriteData(int fileHandle, const void* buf, unsigned int size);
static int soundReadData(int fileHandle, void* buf, unsigned int size);
static int soundOpenData(const char* filePath, int flags);
static long soundSeekData(int fileHandle, long offset, int origin);
static int soundCloseData(int fileHandle);
static char* defaultMangler(char* fname);
static Uint32 doTimerEvent(Uint32 interval, void* param);
static void removeTimedEvent(SDL_TimerID* timerId);
static void removeFadeSound(FadeSound* fadeSound);
static void fadeSounds();

// 0x507E04
static FadeSound* fadeHead = nullptr;

// 0x507E08
static FadeSound* fadeFreeList = nullptr;

// 0x507E10
static int defaultChannel = 2;

// 0x507E14
static SoundMallocFunc* mallocPtr = defaultMalloc;

// 0x507E18
static SoundReallocFunc* reallocPtr = defaultRealloc;

// 0x507E1C
static SoundFreeFunc* freePtr = defaultFree;

// 0x507E20
static SoundFileIO defaultStream = {
    soundOpenData,
    soundCloseData,
    soundReadData,
    soundWriteData,
    soundSeekData,
    soundTellData,
    soundFileSize,
    -1,
};

// 0x507E40
static SoundFileNameMangler* nameMangler = defaultMangler;

// 0x507E44
static const char* errorMsgs[SOUND_ERR_COUNT] = {
    "sound.c: No error",
    "sound.c: SOS driver not loaded",
    "sound.c: SOS invalid pointer",
    "sound.c: SOS detect initialized",
    "sound.c: SOS fail on file open",
    "sound.c: SOS memory fail",
    "sound.c: SOS invalid driver ID",
    "sound.c: SOS no driver found",
    "sound.c: SOS detection failure",
    "sound.c: SOS driver loaded",
    "sound.c: SOS invalid handle",
    "sound.c: SOS no handles",
    "sound.c: SOS paused",
    "sound.c: SOS not paused",
    "sound.c: SOS invalid data",
    "sound.c: SOS drv file fail",
    "sound.c: SOS invalid port",
    "sound.c: SOS invalid IRQ",
    "sound.c: SOS invalid DMA",
    "sound.c: SOS invalid DMA IRQ",
    "sound.c: no device",
    "sound.c: not initialized",
    "sound.c: no sound",
    "sound.c: function not supported",
    "sound.c: no buffers available",
    "sound.c: file not found",
    "sound.c: already playing",
    "sound.c: not playing",
    "sound.c: already paused",
    "sound.c: not paused",
    "sound.c: invalid handle",
    "sound.c: no memory available",
    "sound.c: unknown error",
};

// 0x6651A0
static int soundErrorno;

// 0x6651A4
static int masterVol;

// 0x6651AC
static int sampleRate;

// Number of sounds currently playing.
//
// 0x6651B0
static int numSounds;

// 0x6651B4
static int deviceInit;

// 0x6651B8
static int dataSize;

// 0x6651BC
static int numBuffers;

// 0x6651C0
static bool driverInit;

// 0x6651C4
static Sound* soundMgrList;

static SDL_TimerID gFadeSoundsTimerId = 0;

// 0x499C80
static void* defaultMalloc(size_t size)
{
    return malloc(size);
}

// 0x499C88
static void* defaultRealloc(void* ptr, size_t size)
{
    return realloc(ptr, size);
}

// 0x499C90
static void defaultFree(void* ptr)
{
    free(ptr);
}

// 0x499C98
void soundRegisterAlloc(SoundMallocFunc* mallocProc, SoundReallocFunc* reallocProc, SoundFreeFunc* freeProc)
{
    mallocPtr = mallocProc;
    reallocPtr = reallocProc;
    freePtr = freeProc;
}

// 0x499CAC
static long soundFileSize(int fileHandle)
{
    long pos;
    long size;

    pos = compat_tell(fileHandle);
    size = lseek(fileHandle, 0, SEEK_END);
    lseek(fileHandle, pos, SEEK_SET);

    return size;
}

// 0x499CE0
static long soundTellData(int fileHandle)
{
    return compat_tell(fileHandle);
}

// 0x499CE8
static int soundWriteData(int fileHandle, const void* buf, unsigned int size)
{
    return write(fileHandle, buf, size);
}

// 0x499CF0
static int soundReadData(int fileHandle, void* buf, unsigned int size)
{
    return read(fileHandle, buf, size);
}

// 0x499CF8
static int soundOpenData(const char* filePath, int flags)
{
    return open(filePath, flags);
}

// 0x499D04
static long soundSeekData(int fileHandle, long offset, int origin)
{
    return lseek(fileHandle, offset, origin);
}

// 0x499D0C
static int soundCloseData(int fileHandle)
{
    return close(fileHandle);
}

// 0x499D1C
static char* defaultMangler(char* fname)
{
    return fname;
}

// 0x499D20
const char* soundError(int err)
{
    if (err == -1) {
        err = soundErrorno;
    }

    if (err < 0 || err > SOUND_UNKNOWN_ERROR) {
        err = SOUND_UNKNOWN_ERROR;
    }

    return errorMsgs[err];
}

// 0x499D40
void Sound::refreshBuffers()
{
    if (soundFlags & 0x80) {
        return;
    }

    unsigned int readPos;
    unsigned int writePos;
    bool hr = audioEngineSoundBufferGetCurrentPosition(soundBuffer, &readPos, &writePos);
    if (!hr) {
        return;
    }

    if (readPos < lastPosition) {
        numBytesRead += readPos + numBuffers * dataSize - lastPosition;
    } else {
        numBytesRead += readPos - lastPosition;
    }

    if (soundFlags & 0x0100) {
        if (type & 0x20) {
            if (soundFlags & 0x0200) {
                soundFlags |= 0x80;
            }
        } else {
            if (fileSize <= numBytesRead) {
                soundFlags |= 0x0280;
            }
        }
    }
    lastPosition = readPos;

    if (fileSize < numBytesRead) {
        int v3;
        do {
            v3 = numBytesRead - fileSize;
            numBytesRead = v3;
        } while (v3 > fileSize);
    }

    int v6 = readPos / dataSize;
    if (lastUpdate == v6) {
        return;
    }

    int v53;
    if (lastUpdate > v6) {
        v53 = v6 + numBuffers - lastUpdate;
    } else {
        v53 = v6 - lastUpdate;
    }

    if (dataSize * v53 >= readLimit) {
        v53 = (readLimit + dataSize - 1) / dataSize;
    }

    if (v53 < minReadBuffer) {
        return;
    }

    void* audioPtr1;
    void* audioPtr2;
    unsigned int audioBytes1;
    unsigned int audioBytes2;
    hr = audioEngineSoundBufferLock(soundBuffer, dataSize * lastUpdate, dataSize * v53, &audioPtr1, &audioBytes1, &audioPtr2, &audioBytes2, 0);
    if (!hr) {
        return;
    }

    if (audioBytes1 + audioBytes2 != static_cast<unsigned int>(dataSize * v53)) {
        debug_printf("locked memory region not big enough, wanted %d (%d * %d), got %d (%d + %d)\n", dataSize * v53, v53, dataSize, audioBytes1 + audioBytes2, audioBytes1, audioBytes2);
        debug_printf("Resetting readBuffers from %d to %d\n", v53, (audioBytes1 + audioBytes2) / dataSize);

        v53 = (audioBytes1 + audioBytes2) / dataSize;
        if (v53 < minReadBuffer) {
            debug_printf("No longer above read buffer size, returning\n");
            return;
        }
    }
    unsigned char* audioPtr = reinterpret_cast<unsigned char*>(audioPtr1);
    int audioBytes = audioBytes1;
    while (--v53 != -1) {
        int bytesRead;
        if (soundFlags & 0x0200) {
            bytesRead = dataSize;
            memset(data, 0, bytesRead);
        } else {
            int bytesToRead = dataSize;
            if (field_58 != -1) {
                int pos = io.tell(io.fd);
                if (bytesToRead + pos > field_58) {
                    bytesToRead = field_58 - pos;
                }
            }

            bytesRead = io.read(io.fd, data, bytesToRead);
            if (bytesRead < dataSize) {
                if (!(soundFlags & 0x20) || (soundFlags & 0x0100)) {
                    memset(data + bytesRead, 0, dataSize - bytesRead);
                    soundFlags |= 0x0200;
                    bytesRead = dataSize;
                } else {
                    while (bytesRead < dataSize) {
                        if (loops == -1) {
                            io.seek(io.fd, field_54, SEEK_SET);
                            if (callback != nullptr) {
                                callback(callbackUserData, 0x0400);
                            }
                        } else {
                            if (loops <= 0) {
                                field_58 = -1;
                                field_54 = 0;
                                loops = 0;
                                soundFlags &= ~0x20;
                                bytesRead += io.read(io.fd, data + bytesRead, dataSize - bytesRead);
                                break;
                            }

                            loops--;
                            io.seek(io.fd, field_54, SEEK_SET);

                            if (callback != nullptr) {
                                callback(callbackUserData, 0x400);
                            }
                        }

                        if (field_58 == -1) {
                            bytesToRead = dataSize - bytesRead;
                        } else {
                            int pos = io.tell(io.fd);
                            if (dataSize + bytesRead + pos <= field_58) {
                                bytesToRead = dataSize - bytesRead;
                            } else {
                                bytesToRead = field_58 - bytesRead - pos;
                            }
                        }

                        int v20 = io.read(io.fd, data + bytesRead, bytesToRead);
                        bytesRead += v20;
                        if (v20 < bytesToRead) {
                            break;
                        }
                    }
                }
            }
        }

        if (bytesRead > audioBytes) {
            if (audioBytes != 0) {
                memcpy(audioPtr, data, audioBytes);
            }

            if (audioPtr2 != nullptr) {
                memcpy(audioPtr2, data + audioBytes, bytesRead - audioBytes);
                audioPtr = reinterpret_cast<unsigned char*>(audioPtr2) + bytesRead - audioBytes;
                audioBytes = audioBytes2 - bytesRead;
            } else {
                debug_printf("Hm, no second write pointer, but buffer not big enough, this shouldn't happen\n");
            }
        } else {
            memcpy(audioPtr, data, bytesRead);
            audioPtr += bytesRead;
            audioBytes -= bytesRead;
        }
    }

    audioEngineSoundBufferUnlock(soundBuffer, audioPtr1, audioBytes1, audioPtr2, audioBytes2);

    lastUpdate = v6;

    return;
}

// 0x49A1E4
int soundInit(int a1, int num_buffers, int a3, int data_size, int sample_rate)
{
    if (!audioEngineInit()) {
        debug_printf("soundInit: Unable to init audio engine\n");

        soundErrorno = SOUND_SOS_DETECTION_FAILURE;
        return soundErrorno;
    }

    sampleRate = sample_rate;
    dataSize = data_size;
    numBuffers = num_buffers;
    driverInit = true;
    deviceInit = 1;

    soundSetMasterVolume(VOLUME_MAX);

    soundErrorno = SOUND_NO_ERROR;
    return 0;
}

// 0x49A5D8
void soundClose()
{
    while (soundMgrList != nullptr) {
        Sound* next = soundMgrList->next;
        soundMgrList->destroy();
        soundMgrList = next;
    }

    if (gFadeSoundsTimerId != -1) {
        removeTimedEvent(&gFadeSoundsTimerId);
    }

    while (fadeFreeList != nullptr) {
        FadeSound* next = fadeFreeList->next;
        freePtr(fadeFreeList);
        fadeFreeList = next;
    }

    audioEngineExit();

    soundErrorno = SOUND_NO_ERROR;
    driverInit = false;
}

// 0x49A688
Sound* soundAllocate(int type, int soundFlags)
{
    if (!driverInit) {
        soundErrorno = SOUND_NOT_INITIALIZED;
        return nullptr;
    }

    Sound* sound = static_cast<Sound*>(mallocPtr(sizeof(*sound)));
    memset(sound, 0, sizeof(*sound));

    memcpy(&(sound->io), &defaultStream, sizeof(defaultStream));

    if (!(soundFlags & SOUND_FLAG_0x02)) {
        soundFlags |= SOUND_FLAG_0x02;
    }

    sound->bitsPerSample = (soundFlags & SOUND_16BIT) != 0 ? 16 : 8;
    sound->channels = 1;
    sound->rate = sampleRate;

    sound->soundFlags = soundFlags;
    sound->type = type;
    sound->dataSize = dataSize;
    sound->numBytesRead = 0;
    sound->soundBuffer = -1;
    sound->statusFlags = 0;
    sound->numBuffers = numBuffers;
    sound->readLimit = sound->dataSize * numBuffers;

    if ((type & SOUND_TYPE_INFINITE) != 0) {
        sound->loops = -1;
        sound->soundFlags |= SOUND_LOOPING;
    }

    sound->field_58 = -1;
    sound->minReadBuffer = 1;
    sound->volume = VOLUME_MAX;
    sound->prev = nullptr;
    sound->field_54 = 0;
    sound->next = soundMgrList;

    if (soundMgrList != nullptr) {
        soundMgrList->prev = sound;
    }

    soundMgrList = sound;

    return sound;
}

// 0x49A88C
int Sound::preloadBuffers()
{
    unsigned char* buf;
    int bytes_read;
    int result;
    int v15;
    unsigned char* v14;
    int size;

    size = io.filelength(io.fd);
    fileSize = size;

    if ((type & SOUND_TYPE_STREAMING) != 0) {
        if ((soundFlags & SOUND_LOOPING) == 0) {
            soundFlags |= SOUND_FLAG_0x100 | SOUND_LOOPING;
        }

        if (numBuffers * dataSize >= size) {
            if (size / dataSize * dataSize != size) {
                size = (size / dataSize + 1) * dataSize;
            }
        } else {
            size = numBuffers * dataSize;
        }
    } else {
        type &= ~(SOUND_TYPE_MEMORY | SOUND_TYPE_STREAMING);
        type |= SOUND_TYPE_MEMORY;
    }

    buf = static_cast<unsigned char*>(mallocPtr(size));
    bytes_read = io.read(io.fd, buf, size);
    if (bytes_read != size) {
        if ((soundFlags & SOUND_LOOPING) == 0 || (soundFlags & SOUND_FLAG_0x100) != 0) {
            memset(buf + bytes_read, 0, size - bytes_read);
        } else {
            v14 = buf + bytes_read;
            v15 = bytes_read;
            while (size - v15 > bytes_read) {
                memcpy(v14, buf, bytes_read);
                v15 += bytes_read;
                v14 += bytes_read;
            }

            if (v15 < size) {
                memcpy(v14, buf, size - v15);
            }
        }
    }

    result = setData(buf, size);
    freePtr(buf);

    if ((type & SOUND_TYPE_MEMORY) != 0) {
        io.close(io.fd);
        io.fd = -1;
    } else {
        if (data == nullptr) {
            data = static_cast<unsigned char*>(mallocPtr(dataSize));
        }
    }

    return result;
}

// 0x49AA1C
int Sound::load(char* filePath)
{
    if (!driverInit) {
        soundErrorno = SOUND_NOT_INITIALIZED;
        return soundErrorno;
    }

    io.fd = io.open(nameMangler(filePath), 0x0200);
    if (io.fd == -1) {
        soundErrorno = SOUND_FILE_NOT_FOUND;
        return soundErrorno;
    }

    return preloadBuffers();
}

// 0x49AA88
int Sound::rewind()
{
    bool hr;

    if (!driverInit) {
        soundErrorno = SOUND_NOT_INITIALIZED;
        return soundErrorno;
    }

    if (soundBuffer == -1) {
        soundErrorno = SOUND_NO_SOUND;
        return soundErrorno;
    }

    if ((type & SOUND_TYPE_STREAMING) != 0) {
        io.seek(io.fd, 0, SEEK_SET);
        lastUpdate = 0;
        lastPosition = 0;
        numBytesRead = 0;
        soundFlags &= 0xFD7F;
        hr = audioEngineSoundBufferSetCurrentPosition(soundBuffer, 0);
        preloadBuffers();
    } else {
        hr = audioEngineSoundBufferSetCurrentPosition(soundBuffer, 0);
    }

    if (!hr) {
        soundErrorno = SOUND_UNKNOWN_ERROR;
        return soundErrorno;
    }

    statusFlags &= ~SOUND_STATUS_DONE;

    soundErrorno = SOUND_NO_ERROR;
    return soundErrorno;
}

// 0x49AB4C
int Sound::addData(unsigned char* buf, int size)
{
    bool hr;
    void* audioPtr1;
    unsigned int audioBytes1;
    void* audioPtr2;
    unsigned int audioBytes2;

    hr = audioEngineSoundBufferLock(soundBuffer, 0, size, &audioPtr1, &audioBytes1, &audioPtr2, &audioBytes2, AUDIO_ENGINE_SOUND_BUFFER_LOCK_FROM_WRITE_POS);
    if (!hr) {
        soundErrorno = SOUND_UNKNOWN_ERROR;
        return soundErrorno;
    }

    memcpy(audioPtr1, buf, audioBytes1);

    if (audioPtr2 != nullptr) {
        memcpy(audioPtr2, buf + audioBytes1, audioBytes2);
    }
    hr = audioEngineSoundBufferUnlock(soundBuffer, audioPtr1, audioBytes1, audioPtr2, audioBytes2);
    if (!hr) {
        soundErrorno = SOUND_UNKNOWN_ERROR;
        return soundErrorno;
    }

    soundErrorno = SOUND_NO_ERROR;
    return soundErrorno;
}

// 0x49AC44
int Sound::setData(unsigned char* buf, int size)
{
    if (!driverInit) {
        soundErrorno = SOUND_NOT_INITIALIZED;
        return soundErrorno;
    }

    if (soundBuffer == -1) {
        soundBuffer = audioEngineCreateSoundBuffer(size, bitsPerSample, channels, rate);
        if (soundBuffer == -1) {
            soundErrorno = SOUND_UNKNOWN_ERROR;
            return soundErrorno;
        }
    }

    return addData(buf, size);
}

// 0x49ACC0
int Sound::play()
{
    bool hr;
    unsigned int readPos;
    unsigned int writePos;

    if (!driverInit) {
        soundErrorno = SOUND_NOT_INITIALIZED;
        return soundErrorno;
    }

    if (soundBuffer == -1) {
        soundErrorno = SOUND_NO_SOUND;
        return soundErrorno;
    }

    if ((statusFlags & SOUND_STATUS_DONE) != 0) {
        rewind();
    }

    setVolume(volume);

    hr = audioEngineSoundBufferPlay(soundBuffer, soundFlags & SOUND_LOOPING ? AUDIO_ENGINE_SOUND_BUFFER_PLAY_LOOPING : 0);

    audioEngineSoundBufferGetCurrentPosition(soundBuffer, &readPos, &writePos);
    lastUpdate = readPos / dataSize;

    if (!hr) {
        soundErrorno = SOUND_UNKNOWN_ERROR;
        return soundErrorno;
    }

    statusFlags |= SOUND_STATUS_IS_PLAYING;

    ++numSounds;

    soundErrorno = SOUND_NO_ERROR;
    return soundErrorno;
}

// 0x49ADAC
int Sound::stop()
{
    bool hr;

    if (!driverInit) {
        soundErrorno = SOUND_NOT_INITIALIZED;
        return soundErrorno;
    }

    if (soundBuffer == -1) {
        soundErrorno = SOUND_NO_SOUND;
        return soundErrorno;
    }

    if ((statusFlags & SOUND_STATUS_IS_PLAYING) == 0) {
        soundErrorno = SOUND_NOT_PLAYING;
        return soundErrorno;
    }

    hr = audioEngineSoundBufferStop(soundBuffer);
    if (!hr) {
        soundErrorno = SOUND_UNKNOWN_ERROR;
        return soundErrorno;
    }

    statusFlags &= ~SOUND_STATUS_IS_PLAYING;
    numSounds--;

    soundErrorno = SOUND_NO_ERROR;
    return soundErrorno;
}

// 0x49AE60
int Sound::destroy()
{
    if (!driverInit) {
        soundErrorno = SOUND_NOT_INITIALIZED;
        return soundErrorno;
    }

    if (io.fd != -1) {
        io.close(io.fd);
        io.fd = -1;
    }

    mgrDelete();

    soundErrorno = SOUND_NO_ERROR;
    return soundErrorno;
}

// 0x49AEC4
int numSoundsPlaying()
{
    return numSounds;
}

// 0x49AECC
int Sound::advance()
{
    bool hr;
    unsigned int status;

    if (!driverInit) {
        soundErrorno = SOUND_NOT_INITIALIZED;
        return soundErrorno;
    }

    if (soundBuffer == -1) {
        soundErrorno = SOUND_NO_SOUND;
        return soundErrorno;
    }

    if ((statusFlags & SOUND_STATUS_IS_PLAYING) == 0) {
        soundErrorno = SOUND_NOT_PLAYING;
        return soundErrorno;
    }

    if ((statusFlags & SOUND_STATUS_IS_PAUSED) != 0) {
        soundErrorno = SOUND_NOT_PLAYING;
        return soundErrorno;
    }

    if ((statusFlags & SOUND_STATUS_DONE) != 0) {
        soundErrorno = SOUND_UNKNOWN_ERROR;
        return soundErrorno;
    }

    hr = audioEngineSoundBufferGetStatus(soundBuffer, &status);
    if (!hr) {
        debug_printf("Error in soundContinue, %x\n", hr);

        soundErrorno = SOUND_UNKNOWN_ERROR;
        return soundErrorno;
    }

    if ((soundFlags & SOUND_FLAG_0x80) == 0 && (status & (AUDIO_ENGINE_SOUND_BUFFER_STATUS_PLAYING | AUDIO_ENGINE_SOUND_BUFFER_STATUS_LOOPING)) != 0) {
        if ((statusFlags & SOUND_STATUS_IS_PAUSED) == 0 && (type & SOUND_TYPE_STREAMING) != 0) {
            refreshBuffers();
        }
    } else if ((statusFlags & SOUND_STATUS_IS_PAUSED) == 0) {
        if (callback != nullptr) {
            callback(callbackUserData, 1);
            callback = nullptr;
        }

        if (type & 0x04) {
            callback = nullptr;
            destroy();
        } else {
            statusFlags |= SOUND_STATUS_DONE;

            if (statusFlags & SOUND_STATUS_IS_PLAYING) {
                --numSounds;
            }

            stop();

            statusFlags &= ~(SOUND_STATUS_DONE | SOUND_STATUS_IS_PLAYING);
        }
    }

    soundErrorno = SOUND_NO_ERROR;
    return soundErrorno;
}

// 0x49B008
bool Sound::isPlaying()
{
    if (!driverInit) {
        soundErrorno = SOUND_NOT_INITIALIZED;
        return false;
    }

    if (soundBuffer == -1) {
        soundErrorno = SOUND_NO_SOUND;
        return false;
    }

    return (statusFlags & SOUND_STATUS_IS_PLAYING) != 0;
}

// 0x49B048
bool Sound::isDone()
{
    if (!driverInit) {
        soundErrorno = SOUND_NOT_INITIALIZED;
        return false;
    }

    if (soundBuffer == -1) {
        soundErrorno = SOUND_NO_SOUND;
        return false;
    }

    return (statusFlags & SOUND_STATUS_DONE) != 0;
}

// 0x49B088
bool Sound::isFading()
{
    if (!driverInit) {
        soundErrorno = SOUND_NOT_INITIALIZED;
        return false;
    }

    if (soundBuffer == -1) {
        soundErrorno = SOUND_NO_SOUND;
        return false;
    }

    return (statusFlags & SOUND_STATUS_IS_FADING) != 0;
}

// 0x49B0C8
bool Sound::isPaused()
{
    if (!driverInit) {
        soundErrorno = SOUND_NOT_INITIALIZED;
        return false;
    }

    if (soundBuffer == -1) {
        soundErrorno = SOUND_NO_SOUND;
        return false;
    }

    return (statusFlags & SOUND_STATUS_IS_PAUSED) != 0;
}

// 0x49B108
int Sound::getFlags(int flags)
{
    if (!driverInit) {
        soundErrorno = SOUND_NOT_INITIALIZED;
        return 0;
    }

    if (soundBuffer == -1) {
        soundErrorno = SOUND_NO_SOUND;
        return 0;
    }

    return soundFlags & flags;
}

// 0x49B148
int Sound::getType(int type_)
{
    if (!driverInit) {
        soundErrorno = SOUND_NOT_INITIALIZED;
        return 0;
    }

    if (soundBuffer == -1) {
        soundErrorno = SOUND_NO_SOUND;
        return 0;
    }

    return type & type_;
}

// 0x49B188
int Sound::length()
{
    if (!driverInit) {
        soundErrorno = SOUND_NOT_INITIALIZED;
        return soundErrorno;
    }

    if (soundBuffer == -1) {
        soundErrorno = SOUND_NO_SOUND;
        return soundErrorno;
    }

    int bytesPerSec = bitsPerSample / 8 * rate;
    int v3 = fileSize;
    int v4 = v3 % bytesPerSec;
    int result = v3 / bytesPerSec;
    if (v4 != 0) {
        result += 1;
    }

    return result;
}

// 0x49B284
int Sound::setLoop(int loops_)
{
    if (!driverInit) {
        soundErrorno = SOUND_NOT_INITIALIZED;
        return soundErrorno;
    }

    if (loops_) {
        soundFlags |= SOUND_LOOPING;
        loops = loops_;
    } else {
        loops = 0;
        field_58 = -1;
        field_54 = 0;
        soundFlags &= ~SOUND_LOOPING;
    }

    soundErrorno = SOUND_NO_ERROR;
    return soundErrorno;
}

// 0x49B2EC
int soundVolumeHMItoDirectSound(int volume)
{
    double normalizedVolume;

    if (volume > VOLUME_MAX) {
        volume = VOLUME_MAX;
    }

    // Normalize volume to SDL (0-128).
    normalizedVolume = static_cast<double>(volume - VOLUME_MIN) / static_cast<double>(VOLUME_MAX - VOLUME_MIN) * 128;

    return static_cast<int>(normalizedVolume);
}

// 0x49B38C
int Sound::setVolume(int volume_)
{
    int normalizedVolume;
    bool hr;

    if (!driverInit) {
        soundErrorno = SOUND_NOT_INITIALIZED;
        return soundErrorno;
    }

    volume = volume_;

    if (soundBuffer == -1) {
        soundErrorno = SOUND_NO_ERROR;
        return soundErrorno;
    }

    normalizedVolume = soundVolumeHMItoDirectSound(masterVol * volume_ / VOLUME_MAX);

    hr = audioEngineSoundBufferSetVolume(soundBuffer, normalizedVolume);
    if (!hr) {
        soundErrorno = SOUND_UNKNOWN_ERROR;
        return soundErrorno;
    }

    soundErrorno = SOUND_NO_ERROR;
    return soundErrorno;
}

// 0x49B400
int Sound::getVolume()
{
    if (!deviceInit) {
        soundErrorno = SOUND_NOT_INITIALIZED;
        return soundErrorno;
    }

    if (soundBuffer == -1) {
        soundErrorno = SOUND_NO_SOUND;
        return soundErrorno;
    }

    return volume;
}

// 0x49B570
int Sound::setCallback(SoundCallback* callback_, void* userData)
{
    if (!driverInit) {
        soundErrorno = SOUND_NOT_INITIALIZED;
        return soundErrorno;
    }

    callback = callback_;
    callbackUserData = userData;

    soundErrorno = SOUND_NO_ERROR;
    return soundErrorno;
}

// 0x49B5AC
int Sound::setChannel(int channels_)
{
    if (!driverInit) {
        soundErrorno = SOUND_NOT_INITIALIZED;
        return soundErrorno;
    }

    if (channels_ == 3) {
        channels = 2;
    }

    soundErrorno = SOUND_NO_ERROR;
    return soundErrorno;
}

// 0x49B630
int Sound::setReadLimit(int readLimit_)
{
    if (!driverInit) {
        soundErrorno = SOUND_NOT_INITIALIZED;
        return soundErrorno;
    }

    readLimit = readLimit_;

    soundErrorno = SOUND_NO_ERROR;
    return soundErrorno;
}

// 0x49B664
int Sound::pause()
{
    bool hr;
    unsigned int readPos;
    unsigned int writePos;

    if (!driverInit) {
        soundErrorno = SOUND_NOT_INITIALIZED;
        return soundErrorno;
    }

    if (soundBuffer == -1) {
        soundErrorno = SOUND_NO_SOUND;
        return soundErrorno;
    }

    if ((statusFlags & SOUND_STATUS_IS_PLAYING) == 0) {
        soundErrorno = SOUND_NOT_PLAYING;
        return soundErrorno;
    }

    if ((statusFlags & SOUND_STATUS_IS_PAUSED) != 0) {
        soundErrorno = SOUND_ALREADY_PAUSED;
        return soundErrorno;
    }

    hr = audioEngineSoundBufferGetCurrentPosition(soundBuffer, &readPos, &writePos);
    if (!hr) {
        soundErrorno = SOUND_UNKNOWN_ERROR;
        return soundErrorno;
    }

    pausePos = readPos;
    statusFlags |= SOUND_STATUS_IS_PAUSED;

    return stop();
}

// 0x49B770
int Sound::unpause()
{
    bool hr;

    if (!driverInit) {
        soundErrorno = SOUND_NOT_INITIALIZED;
        return soundErrorno;
    }

    if (soundBuffer == -1) {
        soundErrorno = SOUND_NO_SOUND;
        return soundErrorno;
    }

    if ((statusFlags & SOUND_STATUS_IS_PLAYING) != 0) {
        soundErrorno = SOUND_NOT_PAUSED;
        return soundErrorno;
    }

    if ((statusFlags & SOUND_STATUS_IS_PAUSED) == 0) {
        soundErrorno = SOUND_NOT_PAUSED;
        return soundErrorno;
    }

    hr = audioEngineSoundBufferSetCurrentPosition(soundBuffer, pausePos);
    if (!hr) {
        soundErrorno = SOUND_UNKNOWN_ERROR;
        return soundErrorno;
    }

    statusFlags &= ~SOUND_STATUS_IS_PAUSED;
    pausePos = 0;

    return play();
}

// 0x49B87C
int Sound::setFileIO(SoundOpenProc* openProc, SoundCloseProc* closeProc, SoundReadProc* readProc, SoundWriteProc* writeProc, SoundSeekProc* seekProc, SoundTellProc* tellProc, SoundFileLengthProc* fileLengthProc)
{
    if (!driverInit) {
        soundErrorno = SOUND_NOT_INITIALIZED;
        return soundErrorno;
    }

    if (openProc != nullptr) {
        io.open = openProc;
    }

    if (closeProc != nullptr) {
        io.close = closeProc;
    }

    if (readProc != nullptr) {
        io.read = readProc;
    }

    if (writeProc != nullptr) {
        io.write = writeProc;
    }

    if (seekProc != nullptr) {
        io.seek = seekProc;
    }

    if (tellProc != nullptr) {
        io.tell = tellProc;
    }

    if (fileLengthProc != nullptr) {
        io.filelength = fileLengthProc;
    }

    soundErrorno = SOUND_NO_ERROR;
    return soundErrorno;
}

// 0x49B8F8
void Sound::mgrDelete()
{
    if ((statusFlags & SOUND_STATUS_IS_FADING) != 0) {
        FadeSound* curr = fadeHead;

        while (curr != nullptr) {
            if (this == curr->sound) {
                break;
            }

            curr = curr->next;
        }

        removeFadeSound(curr);
    }

    if (soundBuffer != -1) {
        // NOTE: Uninline.
        if (!isPlaying()) {
            stop();
        }

        if (callback != nullptr) {
            callback(callbackUserData, 1);
        }

        audioEngineSoundBufferRelease(soundBuffer);
        soundBuffer = -1;
    }

    if (deleteCallback != nullptr) {
        deleteCallback(deleteUserData);
    }

    if (data != nullptr) {
        freePtr(data);
        data = nullptr;
    }

    Sound* nextSound = next;
    if (nextSound != nullptr) {
        nextSound->prev = prev;
    }

    Sound* prevSound = prev;
    if (prevSound != nullptr) {
        prevSound->next = next;
    } else {
        soundMgrList = next;
    }

    freePtr(this);
}

// 0x49BAF8
int soundSetMasterVolume(int volume)
{
    if (volume < VOLUME_MIN || volume > VOLUME_MAX) {
        soundErrorno = SOUND_UNKNOWN_ERROR;
        return soundErrorno;
    }

    masterVol = volume;

    Sound* curr = soundMgrList;
    while (curr != nullptr) {
        curr->setVolume(curr->volume);
        curr = curr->next;
    }

    soundErrorno = SOUND_NO_ERROR;
    return soundErrorno;
}

// 0x49BB48
static Uint32 doTimerEvent(Uint32 interval, void* param)
{
    void (*fn)();

    if (param != nullptr) {
        fn = reinterpret_cast<void (*)()>(param);
        fn();
    }

    return 40;
}

// 0x49BB94
static void removeTimedEvent(SDL_TimerID* timerId)
{
    if (*timerId != 0) {
        SDL_RemoveTimer(*timerId);
        *timerId = 0;
    }
}

// 0x49BBB4
int Sound::getPosition()
{
    if (!driverInit) {
        soundErrorno = SOUND_NOT_INITIALIZED;
        return soundErrorno;
    }

    unsigned int readPos;
    unsigned int writePos;
    audioEngineSoundBufferGetCurrentPosition(soundBuffer, &readPos, &writePos);

    if ((type & SOUND_TYPE_STREAMING) != 0) {
        if (readPos < lastPosition) {
            readPos += numBytesRead + numBuffers * dataSize - lastPosition;
        } else {
            readPos -= lastPosition + numBytesRead;
        }
    }

    return readPos;
}

// 0x49BC48
int Sound::setPosition(int pos)
{
    if (!driverInit) {
        soundErrorno = SOUND_NOT_INITIALIZED;
        return soundErrorno;
    }

    if (soundBuffer == -1) {
        soundErrorno = SOUND_NO_SOUND;
        return soundErrorno;
    }

    if ((type & SOUND_TYPE_STREAMING) != 0) {
        int section = pos / dataSize % numBuffers;

        audioEngineSoundBufferSetCurrentPosition(soundBuffer, section * dataSize + pos % dataSize);

        io.seek(io.fd, section * dataSize, SEEK_SET);
        int bytes_read = io.read(io.fd, data, dataSize);
        if (bytes_read < dataSize) {
            if (type & 0x02) {
                io.seek(io.fd, 0, SEEK_SET);
                io.read(io.fd, data + bytes_read, dataSize - bytes_read);
            } else {
                memset(data + bytes_read, 0, dataSize - bytes_read);
            }
        }

        int nextSection = section + 1;
        numBytesRead = pos;

        if (nextSection < numBuffers) {
            lastUpdate = nextSection;
        } else {
            lastUpdate = 0;
        }

        advance();
    } else {
        audioEngineSoundBufferSetCurrentPosition(soundBuffer, pos);
    }

    soundErrorno = SOUND_NO_ERROR;
    return soundErrorno;
}

// 0x49BDAC
static void removeFadeSound(FadeSound* fadeSound)
{
    FadeSound* prev;
    FadeSound* next;
    FadeSound* tmp;

    if (fadeSound == nullptr) {
        return;
    }

    if (fadeSound->sound == nullptr) {
        return;
    }

    if ((fadeSound->sound->statusFlags & SOUND_STATUS_IS_FADING) == 0) {
        return;
    }

    prev = fadeSound->prev;
    if (prev != nullptr) {
        prev->next = fadeSound->next;
    } else {
        fadeHead = fadeSound->next;
    }

    next = fadeSound->next;
    if (next != nullptr) {
        next->prev = fadeSound->prev;
    }

    fadeSound->sound->statusFlags &= ~SOUND_STATUS_IS_FADING;
    fadeSound->sound = nullptr;

    tmp = fadeFreeList;
    fadeFreeList = fadeSound;
    fadeSound->next = tmp;
}

// 0x49BE2C
static void fadeSounds()
{
    FadeSound* ptr;

    ptr = fadeHead;
    while (ptr != nullptr) {
        if ((ptr->currentVolume > ptr->targetVolume || ptr->currentVolume + ptr->deltaVolume < ptr->targetVolume) && (ptr->currentVolume < ptr->targetVolume || ptr->currentVolume + ptr->deltaVolume > ptr->targetVolume)) {
            ptr->currentVolume += ptr->deltaVolume;
            ptr->sound->setVolume(ptr->currentVolume);
        } else {
            if (ptr->targetVolume == 0) {
                if (ptr->field_14) {
                    ptr->sound->pause();
                    ptr->sound->setVolume(ptr->initialVolume);
                } else {
                    if (ptr->sound->type & 0x04) {
                        ptr->sound->destroy();
                    } else {
                        ptr->sound->stop();

                        ptr->initialVolume = ptr->targetVolume;
                        ptr->currentVolume = ptr->targetVolume;
                        ptr->deltaVolume = 0;

                        ptr->sound->setVolume(ptr->targetVolume);
                    }
                }
            }

            removeFadeSound(ptr);
        }
    }

    if (fadeHead == nullptr) {
        // NOTE: Uninline.
        removeTimedEvent(&gFadeSoundsTimerId);
    }
}

// 0x49BF04
int Sound::internalFade(int duration, int targetVolume, int a4)
{
    FadeSound* ptr;

    if (!deviceInit) {
        soundErrorno = SOUND_NOT_INITIALIZED;
        return soundErrorno;
    }

    ptr = nullptr;
    if ((statusFlags & SOUND_STATUS_IS_FADING) != 0) {
        ptr = fadeHead;
        while (ptr != nullptr) {
            if (ptr->sound == this) {
                break;
            }

            ptr = ptr->next;
        }
    }

    if (ptr == nullptr) {
        if (fadeFreeList != nullptr) {
            ptr = fadeFreeList;
            fadeFreeList = fadeFreeList->next;
        } else {
            ptr = static_cast<FadeSound*>(mallocPtr(sizeof(FadeSound)));
        }

        if (ptr != nullptr) {
            if (fadeHead != nullptr) {
                fadeHead->prev = ptr;
            }

            ptr->sound = this;
            ptr->prev = nullptr;
            ptr->next = fadeHead;
            fadeHead = ptr;
        }
    }

    if (ptr == nullptr) {
        soundErrorno = SOUND_NO_MEMORY_AVAILABLE;
        return soundErrorno;
    }

    ptr->targetVolume = targetVolume;
    ptr->initialVolume = getVolume();
    ptr->currentVolume = ptr->initialVolume;
    ptr->field_14 = a4;
    // TODO: Check.
    ptr->deltaVolume = 8 * (125 * (targetVolume - ptr->initialVolume)) / (40 * duration);

    statusFlags |= SOUND_STATUS_IS_FADING;

    bool shouldPlay;
    if (driverInit) {
        if (soundBuffer != -1) {
            shouldPlay = (statusFlags & SOUND_STATUS_IS_PLAYING) == 0;
        } else {
            soundErrorno = SOUND_NO_SOUND;
            shouldPlay = true;
        }
    } else {
        soundErrorno = SOUND_NOT_INITIALIZED;
        shouldPlay = true;
    }

    if (shouldPlay) {
        play();
    }

    if (gFadeSoundsTimerId != 0) {
        soundErrorno = SOUND_NO_ERROR;
        return soundErrorno;
    }

    gFadeSoundsTimerId = SDL_AddTimer(40, doTimerEvent, reinterpret_cast<void*>(fadeSounds));
    if (gFadeSoundsTimerId == 0) {
        soundErrorno = SOUND_UNKNOWN_ERROR;
        return soundErrorno;
    }

    soundErrorno = SOUND_NO_ERROR;
    return soundErrorno;
}

// 0x49C088
int Sound::fade(int duration, int targetVolume)
{
    return internalFade(duration, targetVolume, 0);
}

// 0x49C0D0
void soundFlushAllSounds()
{
    while (soundMgrList != nullptr) {
        soundMgrList->destroy();
    }
}

// 0x49C15C
void soundUpdate()
{
    Sound* curr = soundMgrList;
    while (curr != nullptr) {
        // Sound can be deallocated in `advance`.
        Sound* next = curr->next;
        curr->advance();
        curr = next;
    }
}

// 0x49C17C
int soundSetDefaultFileIO(SoundOpenProc* openProc, SoundCloseProc* closeProc, SoundReadProc* readProc, SoundWriteProc* writeProc, SoundSeekProc* seekProc, SoundTellProc* tellProc, SoundFileLengthProc* fileLengthProc)
{
    if (openProc != nullptr) {
        defaultStream.open = openProc;
    }

    if (closeProc != nullptr) {
        defaultStream.close = closeProc;
    }

    if (readProc != nullptr) {
        defaultStream.read = readProc;
    }

    if (writeProc != nullptr) {
        defaultStream.write = writeProc;
    }

    if (seekProc != nullptr) {
        defaultStream.seek = seekProc;
    }

    if (tellProc != nullptr) {
        defaultStream.tell = tellProc;
    }

    if (fileLengthProc != nullptr) {
        defaultStream.filelength = fileLengthProc;
    }

    soundErrorno = SOUND_NO_ERROR;
    return soundErrorno;
}

} // namespace fallout

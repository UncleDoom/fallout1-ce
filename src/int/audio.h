#pragma once


namespace fallout {

using AudioQueryCompressedFunc = bool(char* filePath);

int audioOpen(const char* fname, int mode);
int audioCloseFile(int fileHandle);
int audioRead(int fileHandle, void* buffer, unsigned int size);
long audioSeek(int fileHandle, long offset, int origin);
long audioFileSize(int fileHandle);
long audioTell(int fileHandle);
int audioWrite(int handle, const void* buf, unsigned int size);
int initAudio(AudioQueryCompressedFunc* isCompressedProc);
void audioClose();

} // namespace fallout

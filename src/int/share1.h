#pragma once


namespace fallout {

char** getFileList(const char* pattern, int* fileNameListLengthPtr);
void freeFileList(char** fileList);

} // namespace fallout

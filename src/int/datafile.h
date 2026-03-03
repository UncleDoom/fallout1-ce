#pragma once


namespace fallout {

using DatafileLoader = unsigned char*(char* path, unsigned char* palette, int* widthPtr, int* heightPtr);
using DatafileNameMangler = char*(char* path);

void datafileSetFilenameFunc(DatafileNameMangler* mangler);
void setBitmapLoadFunc(DatafileLoader* loader);
void datafileConvertData(unsigned char* data, unsigned char* palette, int width, int height);
void datafileConvertDataVGA(unsigned char* data, unsigned char* palette, int width, int height);
unsigned char* loadRawDataFile(char* path, int* widthPtr, int* heightPtr);
unsigned char* loadDataFile(char* path, int* widthPtr, int* heightPtr);
unsigned char* load256Palette(char* path);
void trimBuffer(unsigned char* data, int* widthPtr, int* heightPtr);
unsigned char* datafileGetPalette();
unsigned char* datafileLoadBlock(char* path, int* sizePtr);

} // namespace fallout

#include "int/pcx.h"

#include <cstdio>

#include "int/memdbg.h"
#include "game/raii.h"
#include "plib/db/db.h"

namespace fallout {

struct PcxHeader {
    unsigned char identifier;
    unsigned char version;
    unsigned char encoding;
    unsigned char bitsPerPixel;
    short minX;
    short minY;
    short maxX;
    short maxY;
    short horizontalResolution;
    short verticalResolution;
    unsigned char palette[48];
    unsigned char reserved1;
    unsigned char planeCount;
    short bytesPerLine;
    short paletteType;
    short horizontalScreenSize;
    short verticalScreenSize;
    unsigned char reserved2[54];
};

static short getWord(DB_FILE* stream);
static void readPcxHeader(PcxHeader* pcxHeader, DB_FILE* stream);
static int pcxDecodeScanline(unsigned char* data, int size, DB_FILE* stream);
static int readPcxVgaPalette(PcxHeader* pcxHeader, unsigned char* palette, DB_FILE* stream);

// 0x506320
static unsigned char runcount = 0;

// 0x506321
static unsigned char runvalue = 0;

// 0x486160
static short getWord(DB_FILE* stream)
{
    short value;
    stream->fread(&value, sizeof(value), 1);
    return value;
}

// 0x486184
static void readPcxHeader(PcxHeader* pcxHeader, DB_FILE* stream)
{
    pcxHeader->identifier = stream->fgetc();
    pcxHeader->version = stream->fgetc();
    pcxHeader->encoding = stream->fgetc();
    pcxHeader->bitsPerPixel = stream->fgetc();
    pcxHeader->minX = getWord(stream);
    pcxHeader->minY = getWord(stream);
    pcxHeader->maxX = getWord(stream);
    pcxHeader->maxY = getWord(stream);
    pcxHeader->horizontalResolution = getWord(stream);
    pcxHeader->verticalResolution = getWord(stream);

    for (int index = 0; index < 48; index++) {
        pcxHeader->palette[index] = stream->fgetc();
    }

    pcxHeader->reserved1 = stream->fgetc();
    pcxHeader->planeCount = stream->fgetc();
    pcxHeader->bytesPerLine = getWord(stream);
    pcxHeader->paletteType = getWord(stream);
    pcxHeader->horizontalScreenSize = getWord(stream);
    pcxHeader->verticalScreenSize = getWord(stream);

    for (int index = 0; index < 54; index++) {
        pcxHeader->reserved2[index] = stream->fgetc();
    }
}

// 0x48631C
static int pcxDecodeScanline(unsigned char* data, int size, DB_FILE* stream)
{
    unsigned char runLength = runcount;
    unsigned char value = runvalue;

    int uncompressedSize = 0;
    int index = 0;
    do {
        uncompressedSize += runLength;
        while (runLength > 0 && index < size) {
            data[index] = value;
            runLength--;
            index++;
        }

        runcount = runLength;
        runvalue = value;

        if (runLength != 0) {
            uncompressedSize -= runLength;
            break;
        }

        value = stream->fgetc();
        if ((value & 0xC0) == 0xC0) {
            runcount = value & 0x3F;
            value = stream->fgetc();
            runLength = runcount;
        } else {
            runLength = 1;
        }
    } while (index < size);

    runcount = runLength;
    runvalue = value;

    return uncompressedSize;
}

// 0x4863CC
static int readPcxVgaPalette(PcxHeader* pcxHeader, unsigned char* palette, DB_FILE* stream)
{
    if (pcxHeader->version != 5) {
        return 0;
    }

    long pos = stream->ftell();
    long size = stream->filelength();
    stream->fseek(size - 769, SEEK_SET);
    if (stream->fgetc() != 12) {
        stream->fseek(pos, SEEK_SET);
        return 0;
    }

    for (int index = 0; index < 768; index++) {
        palette[index] = stream->fgetc();
    }

    stream->fseek(pos, SEEK_SET);

    return 1;
}

// 0x486444
unsigned char* loadPCX(const char* path, int* widthPtr, int* heightPtr, unsigned char* palette)
{
    DbFileGuard stream(db_fopen(path, "rb"));
    if (!stream) {
        return nullptr;
    }

    PcxHeader pcxHeader;
    readPcxHeader(&pcxHeader, stream.get());

    int width = pcxHeader.maxX - pcxHeader.minX + 1;
    int height = pcxHeader.maxY - pcxHeader.minY + 1;

    *widthPtr = width;
    *heightPtr = height;

    int bytesPerLine = pcxHeader.planeCount * pcxHeader.bytesPerLine;
    unsigned char* data = static_cast<unsigned char*>(mymalloc(bytesPerLine * height, __FILE__, __LINE__)); // "..\\int\\PCX.C", 195
    if (data == nullptr) {
        // NOTE: This code is unreachable, internal_malloc_safe never fails.
        return nullptr;
    }

    runcount = 0;
    runvalue = 0;

    unsigned char* ptr = data;
    for (int y = 0; y < height; y++) {
        pcxDecodeScanline(ptr, bytesPerLine, stream.get());
        ptr += width;
    }

    readPcxVgaPalette(&pcxHeader, palette, stream.get());

    return data;
}

} // namespace fallout

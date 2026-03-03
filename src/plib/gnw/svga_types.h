#pragma once


namespace fallout {

// NOTE: These typedefs always appear in this order in every implementation file
// with extended debug info. However `mouse.c` does not have DirectX types
// implying it does not include `svga.h` which does so to expose primary
// DirectDraw objects.

using UpdatePaletteFunc = void();
using ResetModeFunc = void();
using SetModeFunc = int();
using ScreenTransBlitFunc = void(unsigned char* srcBuf, unsigned int srcW, unsigned int srcH, unsigned int subX, unsigned int subY, unsigned int subW, unsigned int subH, unsigned int dstX, unsigned int dstY, unsigned char trans);
using ScreenBlitFunc = void(unsigned char* srcBuf, unsigned int srcW, unsigned int srcH, unsigned int subX, unsigned int subY, unsigned int subW, unsigned int subH, unsigned int dstX, unsigned int dstY);

struct VideoOptions {
    int width;
    int height;
    bool fullscreen;
    int scale;
};

} // namespace fallout

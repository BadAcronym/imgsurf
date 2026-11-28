#ifndef IM_HEADER
#define IM_HEADER

#include "pd_util.h"
#include "pd_int_types.h"

#include <stdio.h>

// #define IM_LOG_READ
// #define IM_LOG_WRITE

#define bool  _Bool
#define true  1
#define false 0

#define v_persistent  static
#define s_global      static
#define f_internal    static

#define IM_CHANNELS_4      0
#define IM_CHANNELS_4_FLIP 1
#define IM_CHANNELS_3      2
#define IM_CHANNELS_3_FLIP 3
#define IM_CHANNELS_MAX    3

#define IM_FILE_QOI  0
#define IM_FILE_BMP  1
#define IM_FILE_WEBP 2
#define IM_FILE_AVIF 3
#define IM_FILE_JXL  4
#define IM_FILE_PNG  5
#define IM_FILE_MAX  5

typedef struct pixel
{
    u08 red;
    u08 green;
    u08 blue;
    u08 alpha;
}
pixel;

u08* loadBMP
(
    BinBuffer *file,
    u32       *width,
    u32       *height,
    u08       channels
);

bool writeBMP
(
    FILE *file,
    u08  *data,
    u32  width,
    u32  height,
    u08  channels
);

u08* loadPNG
(
    BinBuffer *file,
    u32       *width,
    u32       *height,
    u08       channels
);

bool writePNG
(
    FILE *file,
    u08  *data,
    u32  width,
    u32  height,
    u08  channels
);

u08* loadQOI
(
    BinBuffer *file,
    u32       *width,
    u32       *height,
    u08       channels
);

bool writeQOI
(
    FILE *file,
    u08  *data,
    u32  width,
    u32  height,
    u08  channels
);

extern u08* imLoadFile
(
    const char *path,
    u32        *width,
    u32        *height,
    u08        channels,
    u08        bitdepth
);

extern u08* imLoadPtr
(
    BinBuffer *file,
    u08       fileFormat,
    u32       *width,
    u32       *height,
    u08       channels,
    u08       bitdepth
);

extern bool imWriteFile
(
    const char *path,
    void       *data,
    u32        width,
    u32        height,
    u08        channels,
    u08        bitdepth,
    u08        fileFormat
);

extern void imWritePtr
(
    FILE  *file,
    void  *data,
    u08   fileFormat,
    u32   width,
    u32   height,
    u08   channels,
    u08   bitdepth
);

#endif

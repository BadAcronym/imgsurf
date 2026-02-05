#pragma once

#include <stdint.h>
#include <stdio.h>

#define bool  _Bool
#define true  1
#define false 0

#define persistent  static
#define global      static
#define internal    static

#define IMGSURF_CHANNELS_RGBA 0
#define IMGSURF_CHANNELS_BGRA 1
#define IMGSURF_CHANNELS_RGB  2
#define IMGSURF_CHANNELS_BGR  3
#define IMGSURF_CHANNELS_MAX  3

#define IMGSURF_FILE_QOI  0
#define IMGSURF_FILE_BMP  1
#define IMGSURF_FILE_WEBP 2
#define IMGSURF_FILE_AVIF 3
#define IMGSURF_FILE_JXL  4
#define IMGSURF_FILE_PNG  5
#define IMGSURF_FILE_MAX  5

#define IMGSURF_TYPE_FILE      0
#define IMGSURF_TYPE_DIRECTORY 1
#define IMGSURF_TYPE_ERROR     2
#define IMGSURF_TYPE_OTHER     3
#define IMGSURF_TYPE_MAX       3

#define IMGSURF_QOI_INDEX (prev_pixel.red * 3 + prev_pixel.green * 5 + prev_pixel.blue * 7 + prev_pixel.alpha * 11) % 64

#define QOI_OP_RGB    254
#define QOI_OP_RGBA   255

#define QOI_OP_INDEX  0
#define QOI_OP_DIFF   1
#define QOI_OP_LUMA   2
#define QOI_OP_RUN    3

typedef struct pixel
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t alpha;
}
pixel;

extern uint8_t imgsurf_verifyPath
(
    const char *path
);

extern uint8_t* imgsurf_load_file
(
    const char *path,
    uint32_t   *width,
    uint32_t   *height,
    uint8_t    channels,
    uint8_t    bitdepth
);

extern uint8_t* imgsurf_load_ptr
(
    FILE       *file,
    uint8_t    fileFormat,
    uint32_t   *width,
    uint32_t   *height,
    uint8_t    channels,
    uint8_t    bitdepth
);

extern uint8_t imgsurf_write_file
(
    const char *path,
    void       *data,
    uint32_t   width,
    uint32_t   height,
    uint8_t    channels,
    uint8_t    bitdepth,
    uint8_t    fileFormat
);

extern void imgsurf_write_ptr
(
    FILE     *file,
    void     *data,
    uint8_t  fileFormat,
    uint32_t width,
    uint32_t height,
    uint8_t  channels,
    uint8_t  bitdepth
);

#pragma once

#include <stdint.h>

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

typedef struct pixel
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t alpha;
}
pixel;

extern uint8_t* imgsurf_load
(
    const char *path,
    uint32_t   *width,
    uint32_t   *height,
    uint8_t    channels,
    uint8_t    bitdepth
);

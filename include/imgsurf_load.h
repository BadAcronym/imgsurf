#pragma once

#include <stdint.h>
#include <stdio.h>

#define persistent  static
#define global      static
#define internal    static

#define IMGSURF_CHANNELS_RGBA 0
#define IMGSURF_CHANNELS_RGB  1
#define IMGSURF_CHANNELS_MAX  1

#define IMGSURF_FILE_PNG  0
#define IMGSURF_FILE_BMP  1
#define IMGSURF_FILE_WEBP 2
#define IMGSURF_FILE_AVIF 3
#define IMGSURF_FILE_QOI  4
#define IMGSURF_FILE_JXL  5
#define IMGSURF_FILE_MAX  5

//TODO: provide some way to flip channels around in specified format
extern uint8_t* imgsurf_load
(
    const char* path,
    uint32_t    *width,
    uint32_t    *height,
    uint8_t     channels,
    uint8_t     bitdepth
);

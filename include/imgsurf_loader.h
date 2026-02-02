#pragma once

#include <stdint.h>
#include <stdio.h>

#define persistent  static
#define global      static
#define internal    static

#define IMGSURF_CHANNELS_MIN  0
#define IMGSURF_CHANNELS_RGBA 0
#define IMGSURF_CHANNELS_RGB  1
#define IMGSURF_CHANNELS_MAX  1

#define IMGSURF_FILE_MIN  10
#define IMGSURF_FILE_PNG  10
#define IMGSURF_FILE_BMP  11
#define IMGSURF_FILE_MAX  11

//TODO: provide some way to flip channels around in specified format
extern uint8_t* imgsurf_load
(
    const char* path,
    uint8_t     format,
    uint8_t     bitdepth
);

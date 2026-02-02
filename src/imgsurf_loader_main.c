#include "imgsurf_loader.h"

internal void loadPNG
(
){
}

internal void findFormat
(
    const char* path,
    uint8_t     *format
){
    uint32_t period = 0;

    uint32_t i = 0;
    //TESTING: find null terminator, reliable?
    while(path[i] != '\0')
    {
        if(path[i] == '.')
        {
            period = i;
        }
        ++i;
    }

    if(period == 0)
    {
        fprintf(stderr, "Path doesn't look like a file. Please check or add a file extension (.png/.bmp...)\n");
    }

    //NOTE: should prevent accessing beyond string, no supported extension name is < 2 || > 4
    if(period + 3 > i || i > period + 4)
    {
        return;
    }

    if(path[period] == 'p' && path[period + 1] == 'n' && path[period + 2] == 'g')
    {
        *format = IMGSURF_FILE_PNG;
    }
    else if(path[period] == 'b' && path[period + 1] == 'm' && path[period + 2] == 'p')
    {
        *format = IMGSURF_FILE_BMP;
    }
    else if(path[period] == 'w' && path[period + 1] == 'e' &&
            path[period + 2] == 'b' && path[period + 3] == 'p')
    {
        *format = IMGSURF_FILE_WEBP;
    }
    else if(path[period] == 'a' && path[period + 1] == 'v' &&
            path[period + 2] == 'i' && path[period + 3] == 'f')
    {
        *format = IMGSURF_FILE_AVIF;
    }
    else if(path[period] == 'q' && path[period + 1] == 'o' && path[period + 2] == 'i')
    {
        *format = IMGSURF_FILE_QOI;
    }
    else if(path[period] == 'j' && path[period + 1] == 'x' && path[period + 2] == 'l')
    {
        *format = IMGSURF_FILE_JXL;
    }
}

uint8_t* imgsurf_load
(
    const char* path,
    uint8_t     channels,
    uint8_t     bitdepth
){
    uint8_t format = UINT8_MAX;
    findFormat(path, &format);

    if(format == UINT8_MAX)
    {
        fprintf(stderr, "File format is not supported. Try a .png/.bmp/.webp/.avif/.qoi/.jxl file.\n");
    }

    if(channels > IMGSURF_CHANNELS_MAX)
    {
        fprintf(stderr, "Invalid colour channels specified.\n");
    }

    if(bitdepth == 0)
    {
        fprintf(stderr, "Bit depth cannot be null.\n");
    }

    if(format == IMGSURF_FILE_PNG && bitdepth > 2 && bitdepth != 4 && bitdepth != 8 && bitdepth != 16)
    {
        fprintf(stderr, "Only bit depths of 1, 2, 4, 8 or 16 are supported by .png!\n");
    }

    //delegate loading based on parsed format, after parameter checking

    return 0;
}

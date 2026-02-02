#include "imgsurf_loader.h"

internal uint8_t* loadPNG
(
){
    //WIP: decode PNG, allocate size and write back!
    fprintf(stderr, "PNG loader under construction!\n");
    return 0;
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
        return;
    }

    //NOTE: should prevent accessing beyond string, no supported extension name is < 2 || > 4
    if(period + 3 > i || i > period + 4)
    {
        return;
    }

    if(path[period + 1] == 'p' && path[period + 2] == 'n' && path[period + 3] == 'g')
    {
        *format = IMGSURF_FILE_PNG;
    }
    else if(path[period + 1] == 'b' && path[period + 2] == 'm' && path[period + 3] == 'p')
    {
        *format = IMGSURF_FILE_BMP;
    }
    else if(path[period + 1] == 'w' && path[period + 2] == 'e' &&
            path[period + 3] == 'b' && path[period + 4] == 'p')
    {
        *format = IMGSURF_FILE_WEBP;
    }
    else if(path[period + 1] == 'a' && path[period + 2] == 'v' &&
            path[period + 3] == 'i' && path[period + 4] == 'f')
    {
        *format = IMGSURF_FILE_AVIF;
    }
    else if(path[period + 1] == 'q' && path[period + 2] == 'o' && path[period + 3] == 'i')
    {
        *format = IMGSURF_FILE_QOI;
    }
    else if(path[period + 1] == 'j' && path[period + 2] == 'x' && path[period + 3] == 'l')
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
    uint8_t* image = 0;

    uint8_t format = UINT8_MAX;
    findFormat(path, &format);

    if(format == UINT8_MAX)
    {
        fprintf(stderr, "File format is not supported. Try a .png/.bmp/.webp/.avif/.qoi/.jxl file.\n");
        return 0;
    }

    if(channels > IMGSURF_CHANNELS_MAX)
    {
        fprintf(stderr, "Invalid colour channels specified.\n");
        return 0;
    }

    if(bitdepth == 0)
    {
        fprintf(stderr, "Bit depth cannot be null.\n");
        return 0;
    }

    //TODO: more bit depth checking? maybe less strict, if it can load with wrong depth, let it
    switch(format)
    {
        case IMGSURF_FILE_PNG:
        {
            if(bitdepth > 2 && bitdepth != 4 && bitdepth != 8 && bitdepth != 16)
            {
                fprintf(stderr, "Only bit depths of 1, 2, 4, 8 or 16 are supported by .png!\n");
            }
            else
            {
                image = loadPNG();
            }
            break;
        }
        case IMGSURF_FILE_BMP:
        {
            fprintf(stderr, "Format BMP not supported yet.\n");
            break;
        }
        case IMGSURF_FILE_WEBP:
        {
            fprintf(stderr, "Format WEBP not supported yet.\n");
            break;
        }
        case IMGSURF_FILE_AVIF:
        {
            fprintf(stderr, "Format AVIF not supported yet.\n");
            break;
        }
        case IMGSURF_FILE_QOI:
        {
            fprintf(stderr, "Format QOI not supported yet.\n");
            break;
        }
        case IMGSURF_FILE_JXL:
        {
            fprintf(stderr, "Format JXL not supported yet.\n");
            break;
        }
        default:
        {
            fprintf(stderr, "Format not supported, code path should be impossible.\n");
            return 0;
        }
    }

    return image;
}

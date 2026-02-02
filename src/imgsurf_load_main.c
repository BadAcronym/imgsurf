#include "imgsurf_load.h"

#ifdef BUILD_LINUX
    #include "linux_imgsurf_platform.h"
#elifdef BUILD_WINDOWS
    #include "win32_imgsurf_platform.h"
#endif

internal uint8_t* loadPNG
(
    const char* path,
    uint8_t     channels,
    uint32_t    *width,
    uint32_t    *height
){
    //WIP: decode PNG, allocate size
    //write width and height into vars
    fprintf(stderr, "\nWIP: PNG loader under construction!\n");
    return 0;
}

internal void findFormat
(
    const char* path,
    uint8_t     *format
){
    uint32_t period = 0;
    uint32_t i = 0;
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
        fprintf(stderr, "\nPath doesn't look like a file.\n");
        return;
    }

    if(period + 3 > i || i > period + 4)
    {
        fprintf(stderr, "\nERROR: File extension doesn't look valid.\n");
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
    uint32_t    *width,
    uint32_t    *height,
    uint8_t     channels,
    uint8_t     bitdepth
){
    uint8_t* image = 0;

    uint8_t format = UINT8_MAX;
    findFormat(path, &format);

    if(format == UINT8_MAX)
    {
        fprintf(stderr, "\nERROR: File format is not supported. Try a .png/.bmp/.webp/.avif/.qoi/.jxl file.\n");
        return 0;
    }

    if(channels > IMGSURF_CHANNELS_MAX)
    {
        fprintf(stderr, "\nERROR: Invalid colour channels specified.\n");
        return 0;
    }

    if(bitdepth == 0)
    {
        fprintf(stderr, "\nERROR: Bit depth cannot be null.\n");
        return 0;
    }

    uint8_t code = imgsurf_verifyPath(path);
    if(code == IMGSURF_TYPE_ERROR)
    {
        fprintf(stderr, "\nERROR: Path %s is not valid.\n", path);
        return 0;
    }
    if(code == IMGSURF_TYPE_DIRECTORY)
    {
        fprintf(stderr, "\nERROR: Path %s is a directory.\n", path);
        return 0;
    }
    if(code > IMGSURF_TYPE_MAX)
    {
        fprintf(stderr, "\nERROR: Verifying the path %s has failed.\n", path);
        return 0;
    }

    //TODO: more bit depth checking? maybe less strict, if it can load with wrong depth, let it
    switch(format)
    {
        case IMGSURF_FILE_PNG:
        {
            if(bitdepth > 2 && bitdepth != 4 && bitdepth != 8 && bitdepth != 16)
            {
                fprintf(stderr, "\nERROR: Only bit depths of 1, 2, 4, 8 or 16 are supported by .png!\n");
            }
            else
            {
                image = loadPNG(path, channels, width, height);
            }
            break;
        }
        case IMGSURF_FILE_BMP:
        {
            fprintf(stderr, "\nTODO: Format BMP not supported yet.\n");
            break;
        }
        case IMGSURF_FILE_WEBP:
        {
            fprintf(stderr, "\nTODO: Format WEBP not supported yet.\n");
            break;
        }
        case IMGSURF_FILE_AVIF:
        {
            fprintf(stderr, "\nTODO: Format AVIF not supported yet.\n");
            break;
        }
        case IMGSURF_FILE_QOI:
        {
            fprintf(stderr, "\nTODO: Format QOI not supported yet.\n");
            break;
        }
        case IMGSURF_FILE_JXL:
        {
            fprintf(stderr, "\nTODO: Format JXL not supported yet.\n");
            break;
        }
        default:
        {
            fprintf(stderr, "\nTODO: Format not supported, code path should be impossible.\n");
            return 0;
        }
    }

    return image;
}

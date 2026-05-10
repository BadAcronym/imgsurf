#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

// #define IMGSURF_LOG_READ
// #define IMGSURF_LOG_WRITE

#include "imgsurf_main.h"

#ifdef BUILD_LINUX
    #include "linux_imgsurf_platform.h"
#elif defined(BUILD_WINDOWS)
    #include "win32_imgsurf_platform.h"
#endif

bool same_pixel
(
    pixel pixel1,
    pixel pixel2
){
    return (pixel1.red  == pixel2.red  && pixel1.green == pixel2.green &&
            pixel1.blue == pixel2.blue && pixel1.alpha == pixel2.alpha);
}

f_internal void findFormat
(
    const char *path,
    uint8_t    *format
){
    uint32_t dot = 0;

    uint32_t i = 0;
    while(path[i] != '\0')
    {
        if(path[i] == '.')
        {
            dot = i;
        }
        ++i;
    }

    if(dot == 0)
    {
        fprintf(stderr, "\n\033[31;1;7mERROR: Path doesn't look like a file.\033[0m\n");
        return;
    }

    if(dot + 3 > i || i > dot + 4)
    {
        fprintf(stderr, "\n\033[31;1;7mERROR: File extension doesn't look valid."
                "\033[0m\n");
        return;
    }

    if(path[dot + 1] == 'p' && path[dot + 2] == 'n' && path[dot + 3] == 'g')
    {
        *format = IMGSURF_FILE_PNG;
    }
    else if(path[dot + 1] == 'b' && path[dot + 2] == 'm' && path[dot + 3] == 'p')
    {
        *format = IMGSURF_FILE_BMP;
    }
    else if(path[dot + 1] == 'w' && path[dot + 2] == 'e' &&
            path[dot + 3] == 'b' && path[dot + 4] == 'p')
    {
        *format = IMGSURF_FILE_WEBP;
    }
    else if(path[dot + 1] == 'a' && path[dot + 2] == 'v' &&
            path[dot + 3] == 'i' && path[dot + 4] == 'f')
    {
        *format = IMGSURF_FILE_AVIF;
    }
    else if(path[dot + 1] == 'q' && path[dot + 2] == 'o' && path[dot + 3] == 'i')
    {
        *format = IMGSURF_FILE_QOI;
    }
    else if(path[dot + 1] == 'j' && path[dot + 2] == 'x' && path[dot + 3] == 'l')
    {
        *format = IMGSURF_FILE_JXL;
    }
    else
    {
        fprintf(stderr, "\n\033[31;1;7mERROR: invalid file extension in path: %s."
                "\033[0m\n", path);
        return;
    }
}

uint8_t* imgsurf_load_file
(
    const char *path,
    uint32_t   *width,
    uint32_t   *height,
    uint8_t    channels,
    uint8_t    bitdepth
){
    uint8_t *image = 0;
    uint8_t format = UINT8_MAX;

    findFormat(path, &format);
    if(format == UINT8_MAX)
    {
        fprintf(stderr, "\n\033[31;1;7mERROR: File format is not supported. Try "
                "a .qoi/.png/.bmp/.webp/.avif/.jxl file.\033[0m\n");
        return 0;
    }

    if(channels > IMGSURF_CHANNELS_MAX)
    {
        fprintf(stderr, "\n\033[31;1;7mERROR: Invalid colour channels specified."
                "\033[0m\n");
        return 0;
    }

    if(bitdepth == 0)
    {
        fprintf(stderr, "\n\033[31;1;7mERROR: Bit depth cannot be null.\033[0m\n");
        return 0;
    }

    uint8_t code = imgsurf_verifyPath(path);
    if(code == IMGSURF_TYPE_ERROR)
    {
        fprintf(stderr, "\n\033[31;1;7mERROR: Path %s is not valid.\033[0m\n", path);
        return 0;
    }
    if(code == IMGSURF_TYPE_DIRECTORY)
    {
        fprintf(stderr, "\n\033[31;1;7mERROR: Path %s is a directory.\033[0m\n", path);
        return 0;
    }
    if(code > IMGSURF_TYPE_MAX)
    {
        fprintf(stderr, "\n\033[31;1;7mERROR: Verifying the path %s has failed."
                "\033[0m\n", path);
        return 0;
    }

    FILE *file = fopen(path, "rb");
    if(!file)
    {
        fprintf(stderr, "\n\033[31;1;7mERROR: Could not open file %s for reading!"
                "\033[0m\n", path);
        return 0;
    }

    if(channels > IMGSURF_CHANNELS_MAX)
    {
        fprintf(stderr, "\n\033[31;1;7mERROR: unknown channel format!\033[0m\n");
        return 0;
    }

    switch(format)
    {
        case IMGSURF_FILE_QOI:
        {
            if(bitdepth != 8)
            {
                fprintf(stderr, "\n\033[31;1;7mERROR: Only a bit depth of 8 is "
                        "supported by .qoi!\033[0m\n");
                return 0;
            }

            image = loadQOI(file, width, height, channels);

            break;
        }
        case IMGSURF_FILE_PNG:
        {
            if(bitdepth > 2 && bitdepth != 4 && bitdepth != 8 && bitdepth != 16)
            {
                fprintf(stderr, "\n\033[31;1;7mERROR: Only bit depths of 1, 2, 4, 8 or "
                        "16 are supported by .png!\033[0m\n");
                return 0;
            }

            image = loadPNG(file, width, height, channels);

            break;
        }
        case IMGSURF_FILE_BMP:
        {
            image = loadBMP(file, width, height, channels);
            break;
        }
        case IMGSURF_FILE_WEBP:
        {
            fprintf(stderr, "\nTODO: Format WEBP not supported yet.\033[0m\n");
            break;
        }
        case IMGSURF_FILE_AVIF:
        {
            fprintf(stderr, "\nTODO: Format AVIF not supported yet.\033[0m\n");
            break;
        }
        case IMGSURF_FILE_JXL:
        {
            fprintf(stderr, "\nTODO: Format JXL not supported yet.\033[0m\n");
            break;
        }
        default:
        {
            fprintf(stderr, "\nUnknown format; not supported.\033[0m\n");
            return 0;
        }
    }

    fclose(file);

    return image;
}

uint8_t* imgsurf_load_ptr
(
    FILE       *file,
    uint8_t    fileFormat,
    uint32_t   *width,
    uint32_t   *height,
    uint8_t    channels,
    uint8_t    bitdepth
){
    if(fileFormat == IMGSURF_FILE_QOI)
    {
        if(bitdepth != 8)
        {
            fprintf(stderr, "\n\033[31;1;7mERROR: Only a bit depth of 8 is "
                    "supported by .qoi!\033[0m\n");
            return 0;
        }
        return loadQOI(file, width, height, channels);
    }

    fprintf(stderr, "\033[31;1;1mERROR: format not implemented or supported. try QOI "
            "for now.\033[0m\n");
    return 0;
}

uint8_t imgsurf_write_file
(
    const char *path,
    void       *data,
    uint32_t   width,
    uint32_t   height,
    uint8_t    channels,
    uint8_t    bitdepth,
    uint8_t    fileFormat
){
    if(!data)
    {
        fprintf(stderr, "\n\033[31;1;7mERROR: data is null.\033[0m\n");
        return 1;
    }

    uint8_t readFileFormat = UINT8_MAX;
    findFormat(path, &readFileFormat);
    if(readFileFormat == UINT8_MAX)
    {
        fprintf(stderr, "\n\033[31;1;7mERROR: File format is not supported. "
                "Try a .qoi/.png/.bmp/.webp/.avif/.jxl file.\033[0m\n");
        return 0;
    }

    if(channels > IMGSURF_CHANNELS_MAX)
    {
        fprintf(stderr, "\n\033[31;1;7mERROR: Invalid colour channels specified."
                "\033[0m\n");
        return 0;
    }

    if(bitdepth == 0)
    {
        fprintf(stderr, "\n\033[31;1;7mERROR: Bit depth cannot be null.\033[0m\n");
        return 0;
    }

    FILE *file = fopen(path, "wb");
    if(!file)
    {
        fprintf(stderr, "\n\033[31;1;7mERROR: Could not open file %s for writing!"
                "\033[0m\n", path);
        return 0;
    }

    if(channels > IMGSURF_CHANNELS_MAX)
    {
        fprintf(stderr, "\n\033[31;1;7mERROR: unknown channel format!\033[0m\n");
        return 0;
    }

    switch(fileFormat)
    {
        case IMGSURF_FILE_QOI:
        {
            if(bitdepth != 8)
            {
                fprintf(stderr, "\n\033[31;1;7mERROR: Only a bit depth of 8 is "
                        "supported by .qoi!\033[0m\n");
                return 1;
            }

            uint8_t result = writeQOI(file, data, width, height, channels);
            if(result)
            {
                fprintf(stderr, "\n\033[31;1;7mERROR: couldn't write .qoi!"
                        "\033[0m\n");
                fclose(file);
                return result;
            }

            break;
        }
        case IMGSURF_FILE_PNG:
        {
            if(bitdepth > 2 && bitdepth != 4 && bitdepth != 8 && bitdepth != 16)
            {
                fprintf(stderr, "\n\033[31;1;7mERROR: Only bit depths of 1, 2, 4, 8 "
                        "or 16 are supported by .png!\033[0m\n");
                return 1;
            }

            uint8_t result = writePNG(file, data, width, height, channels);
            if(result)
            {
                fprintf(stderr, "\n\033[31;1;7mERROR: couldn't write .png!"
                        "\033[0m\n");
                fclose(file);
                return result;
            }

            break;
        }
        case IMGSURF_FILE_BMP:
        {
            uint8_t result = writeBMP(file, data, width, height, channels);
            if(result)
            {
                fprintf(stderr, "\n\033[31;1;7mERROR: couldn't write .bmp!"
                        "\033[0m\n");
                fclose(file);
                return result;
            }

            break;
        }
        case IMGSURF_FILE_WEBP:
        {
            fprintf(stderr, "\nTODO: Format WEBP not supported yet.\033[0m\n");
            break;
        }
        case IMGSURF_FILE_AVIF:
        {
            fprintf(stderr, "\nTODO: Format AVIF not supported yet.\033[0m\n");
            break;
        }
        case IMGSURF_FILE_JXL:
        {
            fprintf(stderr, "\nTODO: Format JXL not supported yet.\033[0m\n");
            break;
        }
        default:
        {
            fprintf(stderr, "\nUnknown format; not supported.\033[0m\n");
            return 0;
        }
    }

    fclose(file);
    return 0;
}

void imgsurf_write_ptr
(
    FILE     *file,
    void     *data,
    uint8_t  fileFormat,
    uint32_t width,
    uint32_t height,
    uint8_t  channels,
    uint8_t  bitdepth
){
    if(fileFormat == IMGSURF_FILE_QOI)
    {
        writeQOI(file, data, width, height, channels);
        return;
    }

    fprintf(stderr, "\033[31;1;1mERROR: format not implemented or supported. try QOI "
            "for now.\033[0m\n");
}

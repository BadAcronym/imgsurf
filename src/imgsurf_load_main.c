#include <stdio.h>
#include <stdlib.h>

#include "imgsurf_load.h"

#ifdef BUILD_LINUX
    #include "linux_imgsurf_platform.h"
#elifdef BUILD_WINDOWS
    #include "win32_imgsurf_platform.h"
#endif

internal uint8_t* loadPNG
(
    FILE        *file,
    uint8_t     channels,
    uint32_t    *width,
    uint32_t    *height
){
    //Wunused-parameter
    (void)file;
    (void)channels;
    (void)width;
    (void)height;

    //TODO: decode PNG, allocate size
    //write width and height into vars
    fprintf(stderr, "\n\033[33;1;7mWIP: PNG loader under construction!\033[0m\n");
    return 0;
}

#define IMGSURF_QOI_INDEX (red * 3 + green * 5 + blue * 7 + alpha * 11) % 64

#define QOI_OP_RGB   0b11111110
#define QOI_OP_RGBA  0b11111111

#define QOI_OP_INDEX 0b00
#define QOI_OP_DIFF  0b01
#define QOI_OP_LUMA  0b10
#define QOI_OP_RUN   0b11

internal uint8_t* loadQOI
(
    FILE        *file,
    uint8_t     channels,
    uint32_t    *width,
    uint32_t    *height
){
    char magic[5] = "qoif";
    *width  = 0;
    *height = 0;

    int byte = 0;
    for(uint8_t i = 0; i < 4; ++i)
    {
        if((byte = fgetc(file)) != magic[i] || byte == EOF)
        {
            fprintf(stderr, "\n\033[31;1;7mERROR: QOI header at byte %i corrupted.\033[0m\n", i);
            fprintf(stderr, "got: %b\n", byte);
            fprintf(stderr, "expected: %b\n", magic[i]);
            return 0;
        }
    }

    for(uint8_t i = 0; i < 4; ++i)
    {
        if((byte = fgetc(file)) == EOF)
        {
            fprintf(stderr, "\n\033[31;1;7mERROR: QOI header at byte %i corrupted.\033[0m\n", 4 + i);
            return 0;
        }
        *width += (byte << (3 - i) * 8);
    }

    for(uint8_t i = 0; i < 4; ++i)
    {
        if((byte = fgetc(file)) == EOF)
        {
            fprintf(stderr, "\n\033[31;1;7mERROR: QOI header at byte %i corrupted.\033[0m\n", 8 + i);
            return 0;
        }
        *height += (byte << (3 - i) * 8);
    }

    if((byte = fgetc(file)) == EOF)
    {
        fprintf(stderr, "\n\033[31;1;7mERROR: QOI header at byte 12 corrupted.\033[0m\n");
    }
    // uint8_t channelcount = byte;

    if((byte = fgetc(file)) == EOF)
    {
        fprintf(stderr, "\n\033[31;1;7mERROR: QOI header at byte 13 corrupted.\033[0m\n");
        return 0;
    }
    // uint8_t colourspace = byte;

    bool discardAlpha = channels == IMGSURF_CHANNELS_RGB || channels == IMGSURF_CHANNELS_BGR;
    bool flipRnB      = channels == IMGSURF_CHANNELS_BGR || channels == IMGSURF_CHANNELS_BGRA;

    uint64_t pixelcount = *width * *height;

    //TODO: figure out best user interface to free image
    uint8_t* image = malloc(pixelcount * (discardAlpha ? 3 : 4));
    if(!image)
    {
        fprintf(stderr, "\n\033[31;1;7mERROR: Failed to allocate image.");
        fprintf(stderr, "Requested Bytes: %lu\033[0m\n", (pixelcount * (discardAlpha ? 3 : 4)));
        return 0;
    }

    uint8_t  red   = 0;
    uint8_t  green = 0;
    uint8_t  blue  = 0;
    uint8_t  alpha = 255;

    uint32_t prev_pixels[64] = {0};

    //check for end of stream (7x 0x00, 1x 0x01) necessary?
    //would be annoying to check a byte in adv
    //pixelcount prevents us from reading past the image pointer.
    //EOF prevents us from reading past the file pointer.
    //So we'll write two extra black pixels if the image ends early - but then it's cropped weird anyways?
    //I'll leave it at that.

    //TODO: verify all OPs
    for(uint64_t i = 0; (byte = fgetc(file)) != EOF && i < pixelcount; ++i)
    {
        if(byte == QOI_OP_RGB)
        {
            if((byte = fgetc(file)) == EOF)
            {
                return image;
            }
            red = byte;

            if((byte = fgetc(file)) == EOF)
            {
                return image;
            }
            green = byte;

            if((byte = fgetc(file)) == EOF)
            {
                return image;
            }
            blue = byte;

            image[i + 1] = green;

            if(flipRnB)
            {
                image[i]     = blue;
                image[i + 2] = red;
            }
            else
            {
                image[i]     = red;
                image[i + 2] = blue;
            }
            uint32_t index = IMGSURF_QOI_INDEX;

            //FIXME: verify BE/LE
            prev_pixels[index] += red;
            prev_pixels[index] += green << 8;
            prev_pixels[index] += blue  << 16;

            if(!discardAlpha)
            {
                image[i + 3] = 255;
                prev_pixels[index] += alpha << 24;
            }
        }
        if(byte == QOI_OP_RGBA)
        {
            if((byte = fgetc(file)) == EOF)
            {
                return image;
            }
            red = byte;

            if((byte = fgetc(file)) == EOF)
            {
                return image;
            }
            green = byte;

            if((byte = fgetc(file)) == EOF)
            {
                return image;
            }
            blue = byte;

            if((byte = fgetc(file)) == EOF)
            {
                return image;
            }
            alpha = byte;

            image[i + 1] = green;

            if(flipRnB)
            {
                image[i]     = blue;
                image[i + 2] = red;
            }
            else
            {
                image[i]     = red;
                image[i + 2] = blue;
            }
            uint32_t index = IMGSURF_QOI_INDEX;

            //FIXME: verify BE/LE
            prev_pixels[index] += red;
            prev_pixels[index] += green << 8;
            prev_pixels[index] += blue  << 16;

            if(!discardAlpha)
            {
                image[i + 3] = alpha;
                prev_pixels[index] += alpha << 24;
            }
        }
        else if((byte >> 6) == QOI_OP_INDEX)
        {
            uint32_t index = byte % 64;

            //FIXME: verify indexing
            red   = prev_pixels[index];
            green = prev_pixels[index >> 8];
            blue  = prev_pixels[index >> 16];
            alpha = prev_pixels[index >> 24];
        }
        else if((byte >> 6) == QOI_OP_DIFF)
        {
            red   += ((0b00110000 & byte) >> 4) - 2;
            green += ((0b00001100 & byte) >> 2) - 2;
            blue  +=  (0b00000011 & byte) - 2;

            image[i + 1] = green;

            if(flipRnB)
            {
                image[i]     = blue;
                image[i + 2] = red;
            }
            else
            {
                image[i]     = red;
                image[i + 2] = blue;
            }
            uint32_t index = IMGSURF_QOI_INDEX;

            //FIXME: verify BE/LE
            prev_pixels[index] += red;
            prev_pixels[index] += green << 8;
            prev_pixels[index] += blue  << 16;

            if(!discardAlpha)
            {
                image[i + 3] = alpha;
                prev_pixels[index] += alpha << 24;
            }
        }
        else if((byte >> 6) == QOI_OP_LUMA)
        {
            uint8_t diffGreen = byte % 64;

            if((byte = fgetc(file)) == EOF)
            {
                return image;
            }
            uint8_t diffRest = byte;

            green = green - 32 + diffGreen;

            red  = red - 32 + diffGreen - 8 + (diffRest >> 4);

            blue = blue - 32 + diffGreen - 8 + (diffRest % 8);

            uint32_t index = IMGSURF_QOI_INDEX;

            //FIXME: verify BE/LE
            prev_pixels[index] += red;
            prev_pixels[index] += green << 8;
            prev_pixels[index] += blue  << 16;

            if(!discardAlpha)
            {
                image[i + 3] = alpha;
                prev_pixels[index] += alpha << 24;
            }
        }
        else if((byte >> 6) == QOI_OP_RUN)
        {
            uint8_t runlength = byte % 64;

            for(uint8_t j = 0; j < runlength; ++j)
            {
                image[i + 1] = green;

                if(flipRnB)
                {
                    image[i]     = blue;
                    image[i + 2] = red;
                }
                else
                {
                    image[i]     = red;
                    image[i + 2] = blue;
                }

                if(!discardAlpha)
                {
                    image[i + 3] = alpha;
                }
                i += j;
            }
        }
        else
        {
            fprintf(stderr, "\n\033[31;1;7mERROR: Unknown QOI_OP.\n");
            fprintf(stderr, "byte: %b\n", byte);
            fprintf(stderr, "byte >> 6: %b\033[0m\n", byte >> 6);
            return image;
        }
    }

    return image;
}

internal void findFormat
(
    const char *path,
    uint8_t    *format
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
        fprintf(stderr, "\n\033[31;1;7mERROR: Path doesn't look like a file.\033[0m\n");
        return;
    }

    if(period + 3 > i || i > period + 4)
    {
        fprintf(stderr, "\n\033[31;1;7mERROR: File extension doesn't look valid.\033[0m\n");
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

//TODO: write back format to River2D_Image
uint8_t* imgsurf_load
(
    const char *path,
    uint32_t   *width,
    uint32_t   *height,
    uint8_t    channels,
    uint8_t    bitdepth
){
    uint8_t* image = 0;
    uint8_t format = UINT8_MAX;

    findFormat(path, &format);
    if(format == UINT8_MAX)
    {
        fprintf(stderr, "\n\033[31;1;7mERROR: File format is not supported. Try a .qoi/.png/.bmp/.webp/.avif/.jxl file.\033[0m\n");
        return 0;
    }

    if(channels > IMGSURF_CHANNELS_MAX)
    {
        fprintf(stderr, "\n\033[31;1;7mERROR: Invalid colour channels specified.\033[0m\n");
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
        fprintf(stderr, "\n\033[31;1;7mERROR: Verifying the path %s has failed.\033[0m\n", path);
        return 0;
    }

    FILE *file = fopen(path, "rb");
    if(!file)
    {
        fprintf(stderr, "\n\033[31;1;7mERROR: Could not open file %s\033[0m\n", path);
        return 0;
    }

    if(channels > IMGSURF_CHANNELS_MAX)
    {
        fprintf(stderr, "\n\033[33;1;7mERROR: unknown channel format!\033[0m\n");
        return 0;
    }

    //TODO: more bit depth checking? maybe less, if it can load with wrong depth, let it
    switch(format)
    {
        case IMGSURF_FILE_QOI:
        {
            if(bitdepth > 2 && bitdepth != 4 && bitdepth != 8 && bitdepth != 16)
            {
                fprintf(stderr, "\n\033[31;1;7mERROR: Only bit depths of 1, 2, 4, 8 or 16 are supported by .qoi!\033[0m\n");
            }
            else
            {
                image = loadQOI(file, channels, width, height);
            }
            break;
        }
        case IMGSURF_FILE_PNG:
        {
            if(bitdepth > 2 && bitdepth != 4 && bitdepth != 8 && bitdepth != 16)
            {
                fprintf(stderr, "\n\033[31;1;7mERROR: Only bit depths of 1, 2, 4, 8 or 16 are supported by .png!\033[0m\n");
            }
            else
            {
                image = loadPNG(file, channels, width, height);
            }
            break;
        }
        case IMGSURF_FILE_BMP:
        {
            fprintf(stderr, "\nTODO: Format BMP not supported yet.\033[0m\n");
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
            fprintf(stderr, "\nTODO: Format not supported, code path should be impossible.\033[0m\n");
            return 0;
        }
    }

    fclose(file);

    return image;
}

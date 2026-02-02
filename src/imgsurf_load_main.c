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

#define IMGSURF_QOI_INDEX (prev_pixel.red * 3 + prev_pixel.green * 5 + prev_pixel.blue * 7 + prev_pixel.alpha * 11) % 64

#define QOI_OP_RGB    0b11111110
#define QOI_OP_RGBA   0b11111111

#define QOI_OP_INDEX  0b00
#define QOI_OP_DIFF   0b01
#define QOI_OP_LUMA   0b10
#define QOI_OP_RUN    0b11

#define QOI_EOS_part1 0b00000000
#define QOI_EOS_part2 0b00000001

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
    uint8_t  pixelwidth = discardAlpha ? 3 : 4;

    //TODO: figure out best user interface to free image
    uint8_t* image = malloc(pixelcount * pixelwidth);
    if(!image)
    {
        fprintf(stderr, "\n\033[31;1;7mERROR: Failed to allocate image.");
        fprintf(stderr, "Requested Bytes: %lu\033[0m\n", (pixelcount * pixelwidth));
        return 0;
    }

    pixel prev_pixel = {0, 0, 0, 255};
    pixel seen_pixels[64] = {0};

    //WIP: debug
    uint64_t count = 0;

    uint64_t loopWidth  = *width * pixelwidth;
    uint64_t loopHeight = *height;

    //FIXME: loop count & indexing broken
    for(uint64_t y = 0; y < loopHeight; y++)
    {
        for(uint64_t x = 0; x < loopWidth && ((byte = fgetc(file)) != EOF); x += pixelwidth)
        {
            if(byte == QOI_OP_RGB)
            {
                if((byte = fgetc(file)) == EOF)
                {
                    return image;
                }
                prev_pixel.red = byte;

                if((byte = fgetc(file)) == EOF)
                {
                    return image;
                }
                prev_pixel.green = byte;

                if((byte = fgetc(file)) == EOF)
                {
                    return image;
                }
                prev_pixel.blue = byte;

                image[y * loopWidth + x + 1] = prev_pixel.green;

                if(flipRnB)
                {
                    image[y * loopWidth + x]     = prev_pixel.blue;
                    image[y * loopWidth + x + 2] = prev_pixel.red;
                }
                else
                {
                    image[y * loopWidth + x]     = prev_pixel.red;
                    image[y * loopWidth + x + 2] = prev_pixel.blue;
                }

                if(!discardAlpha)
                {
                    image[y * loopWidth + x + 3] = prev_pixel.alpha;
                }

                seen_pixels[IMGSURF_QOI_INDEX] = prev_pixel;
            }
            else if(byte == QOI_OP_RGBA)
            {
                if((byte = fgetc(file)) == EOF)
                {
                    return image;
                }
                prev_pixel.red = byte;

                if((byte = fgetc(file)) == EOF)
                {
                    return image;
                }
                prev_pixel.green = byte;

                if((byte = fgetc(file)) == EOF)
                {
                    return image;
                }
                prev_pixel.blue = byte;

                if((byte = fgetc(file)) == EOF)
                {
                    return image;
                }
                prev_pixel.alpha = byte;

                image[y * loopWidth + x + 1] = prev_pixel.green;

                if(flipRnB)
                {
                    image[y * loopWidth + x]     = prev_pixel.blue;
                    image[y * loopWidth + x + 2] = prev_pixel.red;
                }
                else
                {
                    image[y * loopWidth + x]     = prev_pixel.red;
                    image[y * loopWidth + x + 2] = prev_pixel.blue;
                }

                if(!discardAlpha)
                {
                    image[y * loopWidth + x + 3] = prev_pixel.alpha;
                }

                seen_pixels[IMGSURF_QOI_INDEX] = prev_pixel;
            }
            else if((byte >> 6) == QOI_OP_DIFF)
            {
                prev_pixel.red   += ((0b00110000 & byte) >> 4) - 2;
                prev_pixel.green += ((0b00001100 & byte) >> 2) - 2;
                prev_pixel.blue  +=  (byte % 4) - 2;

                image[y * loopWidth + x + 1] = prev_pixel.green;

                if(flipRnB)
                {
                    image[y * loopWidth + x]     = prev_pixel.blue;
                    image[y * loopWidth + x + 2] = prev_pixel.red;
                }
                else
                {
                    image[y * loopWidth + x]     = prev_pixel.red;
                    image[y * loopWidth + x + 2] = prev_pixel.blue;
                }

                if(!discardAlpha)
                {
                    image[y * loopWidth + x + 3] = prev_pixel.alpha;
                }

                seen_pixels[IMGSURF_QOI_INDEX] = prev_pixel;
            }
            else if((byte >> 6) == QOI_OP_LUMA)
            {
                uint8_t diffGreen = byte % 64;

                if((byte = fgetc(file)) == EOF)
                {
                    return image;
                }
                uint8_t diffRest = byte;

                prev_pixel.green = prev_pixel.green - 32 + diffGreen;
                prev_pixel.red   = prev_pixel.red   - 32 + diffGreen - 8 + (diffRest >> 4);
                prev_pixel.blue  = prev_pixel.blue  - 32 + diffGreen - 8 + (diffRest % 16);

                if(!discardAlpha)
                {
                    image[y * loopWidth + x + 3] = prev_pixel.alpha;
                }

                seen_pixels[IMGSURF_QOI_INDEX] = prev_pixel;
            }
            else if((byte >> 6) == QOI_OP_RUN)
            {
                //FIXME: verify runlength
                uint8_t runlength = (byte % 64);

                for(uint8_t j = 0; j < runlength; ++j)
                {
                    image[y * loopWidth + x + 1] = prev_pixel.green;

                    if(flipRnB)
                    {
                        image[y * loopWidth + x]     = prev_pixel.blue;
                        image[y * loopWidth + x + 2] = prev_pixel.red;
                    }
                    else
                    {
                        image[y * loopWidth + x]     = prev_pixel.red;
                        image[y * loopWidth + x + 2] = prev_pixel.blue;
                    }

                    if(!discardAlpha)
                    {
                        image[y * loopWidth + x + 3] = prev_pixel.alpha;
                    }
                    x += pixelwidth;
                }
            }
            else if((byte >> 6) == QOI_OP_INDEX)
            {
                int nextByte = ungetc(fgetc(file), file);
                if(byte == QOI_EOS_part1 && nextByte == QOI_EOS_part2)
                {
                    return image;
                }

                prev_pixel = seen_pixels[byte & 0b00111111];

                image[y * loopWidth + x + 1] = prev_pixel.green;

                if(flipRnB)
                {
                    image[y * loopWidth + x]     = prev_pixel.blue;
                    image[y * loopWidth + x + 2] = prev_pixel.red;
                }
                else
                {
                    image[y * loopWidth + x]     = prev_pixel.red;
                    image[y * loopWidth + x + 2] = prev_pixel.blue;
                }

                if(!discardAlpha)
                {
                    image[y * loopWidth + x + 3] = prev_pixel.alpha;
                }
            }
            else
            {
                fprintf(stderr, "\n\033[31;1;7mERROR: Unknown QOI_OP.\n");
                fprintf(stderr, "byte: %b\n", byte);
                fprintf(stderr, "byte >> 6: %b\033[0m\n", byte >> 6);
                return image;
            }
            //WIP: debug
            // count = y * *width;
            // fprintf(stderr, "\nINFO: parsed %lu.\n", y * loopWidth + x);
        }
    }

    fprintf(stderr, "\nINFO: parsed %lu of %lu.\n", count, pixelcount);
    fprintf(stderr, "diff: %lu too many\n", count - pixelcount);

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

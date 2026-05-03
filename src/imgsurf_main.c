#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include "imgsurf_main.h"

#ifdef BUILD_LINUX
    #include "linux_imgsurf_platform.h"
#elif defined(BUILD_WINDOWS)
    #include "win32_imgsurf_platform.h"
#endif

f_internal bool same_pixel
(
    pixel pixel1,
    pixel pixel2
){
    return (pixel1.red  == pixel2.red  && pixel1.green == pixel2.green &&
            pixel1.blue == pixel2.blue && pixel1.alpha == pixel2.alpha);
}

f_internal uint8_t* loadPNG
(
    FILE        *file,
    uint8_t     channels,
    uint32_t    *width,
    uint32_t    *height
){
    (void)file;
    (void)channels;
    (void)width;
    (void)height;

    fprintf(stderr, "\n\033[33;1;7mWIP: PNG loader under construction!\033[0m\n");
    return 0;
}

f_internal uint8_t* loadQOI
(
    FILE        *file,
    uint32_t    *width,
    uint32_t    *height,
    uint8_t     channels
){
    char magic[5] = "qoif";
    *width  = 0;
    *height = 0;

    size_t elements  = 0;
    char   magicByte = 0;

    for(uint8_t i = 0; i < 4; ++i)
    {
        if((elements = fread(&magicByte, 1, 1, file)) != 1)
        {
            fprintf(stderr, "\n\033[31;1;7mERROR: could not read header at byte %u.\033[0m\n", i);
            if(feof(file))
            {
                fprintf(stderr, "\n\033[31;1;7munexpected end of file.\033[0m\n");
            }
            else if(ferror(file))
            {
                fprintf(stderr, "\n\033[31;1;7mcould not read file.\033[0m\n");
            }

            return 0;
        }

        if(magicByte != magic[i])
        {
            fprintf(stderr, "\n\033[31;1;7mERROR: QOI header at byte %u corrupted.\033[0m\n", i);
            fprintf(stderr, "got: %u\n", (uint8_t)magicByte);
            fprintf(stderr, "expected: %u\n", (uint8_t)magic[i]);
            return 0;
        }
    }

    int byte = 0;
    for(uint8_t i = 0; i < 4; ++i)
    {
        if((byte = fgetc(file)) == EOF)
        {
            fprintf(stderr, "\n\033[31;1;7mERROR: QOI header at byte %u corrupted.\033[0m\n", 4 + i);
            return 0;
        }
        *width += (uint32_t)(byte << (3 - i) * 8);
    }

    for(uint8_t i = 0; i < 4; ++i)
    {
        if((byte = fgetc(file)) == EOF)
        {
            fprintf(stderr, "\n\033[31;1;7mERROR: QOI header at byte %u corrupted.\033[0m\n", 8 + i);
            return 0;
        }
        *height += (uint32_t)(byte << (3 - i) * 8);
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

    #ifdef IMGSURF_LOGGING
    uint8_t *data = (uint8_t*)file;
    uint8_t *data_start = (uint8_t*)file;
    #endif

    uint8_t* image = malloc(pixelcount * pixelwidth);
    if(!image)
    {
        fprintf(stderr, "\n\033[31;1;7mERROR: Failed to allocate image.");
        fprintf(stderr, "Requested Bytes: %" PRIu64 "\033[0m\n", (pixelcount * pixelwidth));
        return 0;
    }

    pixel prev_pixel      = {0, 0, 0, 255};
    pixel seen_pixels[64] = {0};

    uint64_t loopWidth  = *width * pixelwidth;
    uint64_t loopHeight = *height;

    #ifdef IMGSURF_LOGGING
    uint64_t run_count   = 0;
    uint64_t diff_count  = 0;
    uint64_t index_count = 0;
    uint64_t luma_count  = 0;
    uint64_t rgb_count   = 0;
    uint64_t rgba_count  = 0;
    #endif

    for(uint64_t y = 0; y < loopHeight; y++)
    {
        for(uint64_t x = 0; x < loopWidth && ((byte = fgetc(file)) != EOF); x += pixelwidth)
        {
            if(byte == QOI_OP_RGB)
            {
                if((elements = fread(&byte, 1, 1, file)) != 1)
                {
                    fprintf(stderr, "\n\033[31;1;7mERROR: failed to read red @ QOI_OP_RGB.\033[0m\n");
                }
                prev_pixel.red = (uint8_t)byte;

                if((elements = fread(&byte, 1, 1, file)) != 1)
                {
                    fprintf(stderr, "\n\033[31;1;7mERROR: failed to read green @ QOI_OP_RGB.\033[0m\n");
                }
                prev_pixel.green = (uint8_t)byte;

                if((elements = fread(&byte, 1, 1, file)) != 1)
                {
                    fprintf(stderr, "\n\033[31;1;7mERROR: failed to read blue @ QOI_OP_RGB.\033[0m\n");
                }
                prev_pixel.blue = (uint8_t)byte;

                image[y * loopWidth + x]     = flipRnB ? prev_pixel.blue : prev_pixel.red;
                image[y * loopWidth + x + 1] = prev_pixel.green;
                image[y * loopWidth + x + 2] = flipRnB ? prev_pixel.red  : prev_pixel.blue;

                if(!discardAlpha)
                {
                    image[y * loopWidth + x + 3] = prev_pixel.alpha;
                }

                #ifdef IMGSURF_LOGGING
                ++rgb_count;
                fprintf(stderr, "%lx: CURRENT READ OP: QOI_OP_RGB with new pixel: %u, %u, %u\n", data - data_start, prev_pixel.red, prev_pixel.green, prev_pixel.blue);
                #endif
                seen_pixels[IMGSURF_QOI_INDEX] = prev_pixel;
            }
            else if(byte == QOI_OP_RGBA)
            {
                if((elements = fread(&byte, 1, 1, file)) != 1)
                {
                    fprintf(stderr, "\n\033[31;1;7mERROR: failed to read red @ QOI_OP_RGBA.\033[0m\n");
                }
                prev_pixel.red = (uint8_t)byte;

                if((elements = fread(&byte, 1, 1, file)) != 1)
                {
                    fprintf(stderr, "\n\033[31;1;7mERROR: failed to read green @ QOI_OP_RGBA.\033[0m\n");
                }
                prev_pixel.green = (uint8_t)byte;

                if((elements = fread(&byte, 1, 1, file)) != 1)
                {
                    fprintf(stderr, "\n\033[31;1;7mERROR: failed to read blue @ QOI_OP_RGBA.\033[0m\n");
                }
                prev_pixel.blue = (uint8_t)byte;

                if((elements = fread(&byte, 1, 1, file)) != 1)
                {
                    fprintf(stderr, "\n\033[31;1;7mERROR: failed to read alpha @ QOI_OP_RGBA.\033[0m\n");
                }
                prev_pixel.alpha = (uint8_t)byte;

                image[y * loopWidth + x]     = flipRnB ? prev_pixel.blue : prev_pixel.red;
                image[y * loopWidth + x + 1] = prev_pixel.green;
                image[y * loopWidth + x + 2] = flipRnB ? prev_pixel.red  : prev_pixel.blue;

                if(!discardAlpha)
                {
                    image[y * loopWidth + x + 3] = prev_pixel.alpha;
                }

                #ifdef IMGSURF_LOGGING
                ++rgba_count;
                fprintf(stderr, "%lx: CURRENT READ OP: QOI_OP_RGBA with new pixel: %u, %u, %u, %u\n", data - data_start, prev_pixel.red, prev_pixel.green, prev_pixel.blue, prev_pixel.alpha);
                #endif
                seen_pixels[IMGSURF_QOI_INDEX] = prev_pixel;
            }
            else if((byte >> 6) == QOI_OP_DIFF)
            {
                uint8_t diffRed   = (0x30 & byte) >> 4;
                uint8_t diffGreen = (0xC  & byte) >> 2;
                uint8_t diffBlue  =  0x3  & byte;

                prev_pixel.red   += diffRed   - 2;
                prev_pixel.green += diffGreen - 2;
                prev_pixel.blue  += diffBlue  - 2;

                image[y * loopWidth + x]     = flipRnB ? prev_pixel.blue : prev_pixel.red;
                image[y * loopWidth + x + 1] = prev_pixel.green;
                image[y * loopWidth + x + 2] = flipRnB ? prev_pixel.red  : prev_pixel.blue;

                if(!discardAlpha)
                {
                    image[y * loopWidth + x + 3] = prev_pixel.alpha;
                }

                seen_pixels[IMGSURF_QOI_INDEX] = prev_pixel;
                #ifdef IMGSURF_LOGGING
                ++diff_count;
                fprintf(stderr, "%lx: CURRENT READ OP: QOI_OP_DIFF with diff: %u, %u, %u\n", data - data_start, diffRed, diffGreen, diffBlue);
                #endif
            }
            else if((byte >> 6) == QOI_OP_LUMA)
            {
                uint8_t diffGreen = byte & 0x3F;

                if((elements = fread(&byte, 1, 1, file)) != 1)
                {
                    fprintf(stderr, "\n\033[31;1;7mERROR: failed to read diffs @ QOI_OP_LUMA.\033[0m\n");
                }
                uint8_t diffRed  = (0xF0 & byte) >> 4;
                uint8_t diffBlue = (0xF  & byte);

                prev_pixel.green += diffGreen - 32;
                prev_pixel.red   += diffGreen + diffRed  - 40;
                prev_pixel.blue  += diffGreen + diffBlue - 40;

                image[y * loopWidth + x]     = flipRnB ? prev_pixel.blue : prev_pixel.red;
                image[y * loopWidth + x + 1] = prev_pixel.green;
                image[y * loopWidth + x + 2] = flipRnB ? prev_pixel.red  : prev_pixel.blue;

                if(!discardAlpha)
                {
                    image[y * loopWidth + x + 3] = prev_pixel.alpha;
                }

                seen_pixels[IMGSURF_QOI_INDEX] = prev_pixel;
                #ifdef IMGSURF_LOGGING
                ++luma_count;
                fprintf(stderr, "%lx: CURRENT READ OP: QOI_OP_LUMA with encoded diffs: %u, %u, %u\n", data - data_start, (uint8_t)(diffGreen), (uint8_t)(diffRed), (uint8_t)(diffBlue));
                #endif
            }
            else if((byte >> 6) == QOI_OP_RUN)
            {
                uint8_t runlength = (byte & 0x3F) + 1;

                uint64_t index = y * loopWidth + x;

                for(uint8_t j = 0; j < runlength && index + j * pixelwidth < loopHeight * loopWidth; ++j)
                {
                    image[index + j * pixelwidth]     = flipRnB ? prev_pixel.blue : prev_pixel.red;
                    image[index + j * pixelwidth + 1] = prev_pixel.green;
                    image[index + j * pixelwidth + 2] = flipRnB ? prev_pixel.red  : prev_pixel.blue;

                    if(!discardAlpha)
                    {
                        image[index + j * pixelwidth + 3] = prev_pixel.alpha;
                    }
                }
                x += (runlength - 1) * pixelwidth;
                y += x / loopWidth;
                x %= loopWidth;

                #ifdef IMGSURF_LOGGING
                data += (runlength - 1) * pixelwidth;
                ++run_count;
                fprintf(stderr, "%lx: (%x) CURRENT READ OP: QOI_OP_RUN with pixel: %u, %u, %u and runlength %hhu\n", data - data_start, byte, prev_pixel.red, prev_pixel.green, prev_pixel.blue, runlength);
                #endif
            }
            else if((byte >> 6) == QOI_OP_INDEX)
            {
                if(byte == 0x00)
                {
                    int byteBuffer[8];
                    uint8_t zeroCounter = 0;
                    for(uint8_t i = 0; i < 6 && (byteBuffer[i] = fgetc(file)) == 0x00; ++i)
                    {
                        if(++zeroCounter == 6)
                        {
                            if((byteBuffer[i + 1] = fgetc(file)) == 0x01)
                            {
                                fprintf(stderr, "\n\033[31;1;7mERROR: QOI end-of-stream reached early.\033[0m\n");
                                return image;
                            }
                        }
                    }
                    for(uint8_t i = 0; i < zeroCounter + 1; ++i)
                    {
                        ungetc(byteBuffer[zeroCounter - i], file);
                    }
                }

                prev_pixel = seen_pixels[byte % 0x40];

                image[y * loopWidth + x]     = flipRnB ? prev_pixel.blue : prev_pixel.red;
                image[y * loopWidth + x + 1] = prev_pixel.green;
                image[y * loopWidth + x + 2] = flipRnB ? prev_pixel.red  : prev_pixel.blue;

                if(!discardAlpha)
                {
                    image[y * loopWidth + x + 3] = prev_pixel.alpha;
                }
                #ifdef IMGSURF_LOGGING
                ++index_count;
                fprintf(stderr, "%lx: CURRENT READ OP: QOI_OP_INDEX with index: %u\n", data - data_start, byte % 0x40);
                #endif
            }
            else
            {
                fprintf(stderr, "\n\033[31;1;7mERROR: Unknown QOI_OP.\n");
                return image;
            }
            #ifdef IMGSURF_LOGGING
            data += pixelwidth;
            #endif
        }
    }

    #ifdef IMGSURF_LOGGING
    fprintf(stderr, "\n\nREAD BACK (actual):\n\n");
    fprintf(stderr, "run_count:   %lu\n", run_count);
    fprintf(stderr, "diff_count:  %lu\n", diff_count);
    fprintf(stderr, "index_count: %lu\n", index_count);
    fprintf(stderr, "luma_count:  %lu\n", luma_count);
    fprintf(stderr, "rgb_count:   %lu\n", rgb_count);
    fprintf(stderr, "rgba_count:  %lu\n", rgba_count);
    fprintf(stderr, "\n\n");
    #endif

    int byteBuffer[8];
    uint8_t zeroCounter = 0;
    for(uint8_t i = 0; i < 7 && (byteBuffer[i] = fgetc(file)) == 0x00; ++i)
    {
        if(++zeroCounter == 7)
        {
            if((byteBuffer[i + 1] = fgetc(file)) == 0x01)
            {
                #ifdef IMGSURF_LOGGING
                    fprintf(stderr, "QOI end-of-stream reached.\n");
                #endif
                return image;
            }
        }
    }

    return image;
}

f_internal void findFormat
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
    else
    {
        fprintf(stderr, "\n\033[31;1;7mERROR: invalid file extension in path: %s.\033[0m\n", path);
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
        fprintf(stderr, "\n\033[31;1;7mERROR: Could not open file %s for reading!\033[0m\n", path);
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
                fprintf(stderr, "\n\033[31;1;7mERROR: Only a bit depth of 8 is supported by .qoi!\033[0m\n");
            }
            else
            {
                image = loadQOI(file, width, height, channels);
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
        return loadQOI(file, width, height, channels);
    }

    fprintf(stderr, "\033[31;1;1mERROR: format not implemented or supported. try QOI for now.\033[0m\n");
    return 0;
}

f_internal uint8_t writeQOI
(
    FILE     *file,
    uint8_t  *data,
    uint32_t width,
    uint32_t height,
    uint8_t  channels
){
    uint8_t *data_start  = data;
    uint8_t channelcount = (channels == IMGSURF_CHANNELS_RGBA || channels == IMGSURF_CHANNELS_BGRA) ? 4 : 3;

    if(channelcount != 3 && channelcount != 4)
    {
        fprintf(stderr, "\n\033[31;1;7mERROR: QOI only supports channelcounts of 3 (RGB) or 4 (RGBA). Got: %u\033[0m\n", channelcount);
        return 1;
    }

    // colourspace ignored
    uint8_t colourspace = 0;
    size_t  elements    = 0;

    if((elements = fwrite("qoif", 4, 1, file)) != 1)
    {
        fprintf(stderr, "\n\033[31;1;7mERROR: failed to write magic bytes of header.\033[0m\n");
        return 2;
    }

    for(int8_t i = 3; i > -1; --i)
    {
        uint8_t width_shifted = (width >> (i * 8)) & 0xFF;
        if((elements = fwrite(&width_shifted, 1, 1, file)) != 1)
        {
            fprintf(stderr, "\n\033[31;1;7mERROR: failed to write width @ byte: %u.\033[0m\n", i);
            return 2;
        }
    }

    for(int8_t i = 3; i > -1; --i)
    {
        uint8_t height_shifted = (height >> (i * 8)) & 0xFF;
        if((elements = fwrite(&height_shifted, 1, 1, file)) != 1)
        {
            fprintf(stderr, "\n\033[31;1;7mERROR: failed to write height @ byte: %u.\033[0m\n", i);
            return 2;
        }
    }

    if((elements = fwrite(&channelcount, 1, 1, file)) != 1)
    {
        fprintf(stderr, "\n\033[31;1;7mERROR: failed to write channelcount.\033[0m\n");
        return 2;
    }

    if((elements = fwrite(&colourspace, 1, 1, file)) != 1)
    {
        fprintf(stderr, "\n\033[31;1;7mERROR: failed to write colourspace.\033[0m\n");
        return 2;
    }

    pixel prev_pixel      = {0, 0, 0, 255};
    pixel seen_pixels[64] = {0};

    bool flipRnB  = channels == IMGSURF_CHANNELS_BGR  ||
                    channels == IMGSURF_CHANNELS_BGRA;
    bool useAlpha = channels == IMGSURF_CHANNELS_RGBA ||
                    channels == IMGSURF_CHANNELS_BGRA;

    #ifdef IMGSURF_LOGGING
    uint64_t run_count   = 0;
    uint64_t diff_count  = 0;
    uint64_t index_count = 0;
    uint64_t luma_count  = 0;
    uint64_t rgb_count   = 0;
    uint64_t rgba_count  = 0;
    #endif

    uint8_t *data_end = data + width * height * channelcount;
    uint8_t runlength = 0;

    for(; data < data_end; data += channelcount)
    {
        pixel curr_pixel = {0, 0, 0, 255};

        uint8_t og_r = *data;
        uint8_t og_b = *(data + 2);

        curr_pixel.red   = flipRnB ? og_b : og_r;
        curr_pixel.green = *(data + 1);
        curr_pixel.blue  = flipRnB ? og_r : og_b;
        if(useAlpha)
        {
            curr_pixel.alpha = *(data + 3);
        }

        uint8_t dr = curr_pixel.red   - prev_pixel.red;
        uint8_t dg = curr_pixel.green - prev_pixel.green;
        uint8_t db = curr_pixel.blue  - prev_pixel.blue;

        uint8_t index = (curr_pixel.red  * 3 + curr_pixel.green * 5
                       + curr_pixel.blue * 7 + curr_pixel.alpha * 11) % 64;

        if(same_pixel(curr_pixel, prev_pixel))
        {
            ++runlength;
            if(runlength == 62 || data + channelcount == data_end)
            {
                uint8_t byte = 0xC0 | (runlength - 1);
                if((elements = fwrite(&byte, 1, 1, file)) != 1)
                {
                    fprintf(stderr, "\n\033[31;1;7mERROR: failed to write data @ QOI_OP_RUN.\033[0m\n");
                    return 3;
                }

                #ifdef IMGSURF_LOGGING
                ++run_count;
                fprintf(stderr, "%lx: (%x) CURRENT WRITE OP: QOI_OP_RUN with pixel: %u, %u, %u, %u, runlength: %hhu\n", data - data_start, byte, prev_pixel.red, prev_pixel.green, prev_pixel.blue, prev_pixel.alpha, runlength);
                fprintf(stderr, "data_end is: %lx", data_end - data_start);
                #endif
                runlength = 0;
            }
            continue;
        }

        if(runlength > 0)
        {
            uint8_t byte = 0xC0 | (runlength - 1);
            if((elements = fwrite(&byte, 1, 1, file)) != 1)
            {
                fprintf(stderr, "\n\033[31;1;7mERROR: failed to write data @ QOI_OP_RUN.\033[0m\n");
                return 3;
            }

            #ifdef IMGSURF_LOGGING
            ++run_count;
            fprintf(stderr, "%lx: (%x) CURRENT WRITE OP: QOI_OP_RUN with pixel: %u, %u, %u, %u, runlength: %hhu\n", data - data_start, byte, prev_pixel.red, prev_pixel.green, prev_pixel.blue, prev_pixel.alpha, runlength);
            #endif
            runlength = 0;
        }

        if(same_pixel(curr_pixel, seen_pixels[index]))
        {
            if((elements = fwrite(&index, 1, 1, file)) != 1)
            {
                fprintf(stderr, "\n\033[31;1;7mERROR: failed to write data @ QOI_OP_INDEX.\033[0m\n");
                return 5;
            }

            #ifdef IMGSURF_LOGGING
            ++index_count;
            fprintf(stderr, "%lx: CURRENT WRITE OP: QOI_OP_INDEX with index: %u\n", data - data_start, index);
            #endif
        }
        else if(curr_pixel.alpha == prev_pixel.alpha && (uint8_t)(dr + 2) < 0x4 && (uint8_t)(dg + 2) < 0x4 && (uint8_t)(db + 2) < 0x4)
        {
            uint8_t byte = (uint8_t)(0x40 | ((uint8_t)(dr + 2) << 4) | ((uint8_t)(dg + 2) << 2) | (uint8_t)(db + 2));

            if((elements = fwrite(&byte, 1, 1, file)) != 1)
            {
                fprintf(stderr, "\n\033[31;1;7mERROR: failed to write data @ QOI_OP_DIFF.\033[0m\n");
                return 4;
            }

            #ifdef IMGSURF_LOGGING
            ++diff_count;
            fprintf(stderr, "%lx: CURRENT WRITE OP: QOI_OP_DIFF and diff: %i, %i, %i\n", data - data_start, (uint8_t)(dr + 2), (uint8_t)(dg + 2), (uint8_t)(db + 2));
            #endif
        }
        else if(curr_pixel.alpha == prev_pixel.alpha && (uint8_t)(dg + 32) < 0x40 && (uint8_t)(dr - dg + 8) < 0x10 && (uint8_t)(db - dg + 8) < 0x10)
        {
            uint8_t byte1 = 0x80 | (uint8_t)(dg + 32);
            uint8_t byte2 = (uint8_t)(((uint8_t)(dr - dg + 8) << 4) | (uint8_t)(db - dg + 8));

            if((elements = fwrite(&byte1, 1, 1, file)) != 1)
            {
                fprintf(stderr, "\n\033[31;1;7mERROR: failed to write tag/dg @ QOI_OP_LUMA.\033[0m\n");
                return 6;
            }

            if((elements = fwrite(&byte2, 1, 1, file)) != 1)
            {
                fprintf(stderr, "\n\033[31;1;7mERROR: failed to write dr/db data @ QOI_OP_LUMA.\033[0m\n");
                return 6;
            }

            #ifdef IMGSURF_LOGGING
            ++luma_count;
            fprintf(stderr, "%lx: CURRENT WRITE OP: QOI_OP_LUMA and encoded diffs: %u, %u, %u\n", data - data_start, (uint8_t)(dg + 32), (uint8_t)(dr - dg + 8), (uint8_t)(db - dg + 8));
            #endif
        }
        else if(curr_pixel.alpha == prev_pixel.alpha)
        {
            uint8_t tag  = 0xFE;
            if((elements = fwrite(&tag, 1, 1, file)) != 1)
            {
                fprintf(stderr, "\n\033[31;1;7mERROR: failed to write tag @ QOI_OP_RGB.\033[0m\n");
                return 7;
            }

            if((elements = fwrite(&curr_pixel.red, 1, 1, file)) != 1)
            {
                fprintf(stderr, "\n\033[31;1;7mERROR: failed to write red @ QOI_OP_RGB.\033[0m\n");
                return 8;
            }
            if((elements = fwrite(&curr_pixel.green, 1, 1, file)) != 1)
            {
                fprintf(stderr, "\n\033[31;1;7mERROR: failed to write green @ QOI_OP_RGB.\033[0m\n");
                return 8;
            }
            if((elements = fwrite(&curr_pixel.blue, 1, 1, file)) != 1)
            {
                fprintf(stderr, "\n\033[31;1;7mERROR: failed to write blue @ QOI_OP_RGB.\033[0m\n");
                return 8;
            }

            #ifdef IMGSURF_LOGGING
            ++rgb_count;
            fprintf(stderr, "%lx: CURRENT WRITE OP: QOI_OP_RGB with new pixel: %u, %u, %u\n", data - data_start, curr_pixel.red, curr_pixel.green, curr_pixel.blue);
            #endif
        }
        else
        {
            uint8_t tag  = 0xFF;
            if((elements = fwrite(&tag, 1, 1, file)) != 1)
            {
                fprintf(stderr, "\n\033[31;1;7mERROR: failed to write tag @ QOI_OP_RGBA.\033[0m\n");
                return 10;
            }

            if((elements = fwrite(&curr_pixel.red, 1, 1, file)) != 1)
            {
                fprintf(stderr, "\n\033[31;1;7mERROR: failed to write red @ QOI_OP_RGBA.\033[0m\n");
                return 8;
            }
            if((elements = fwrite(&curr_pixel.green, 1, 1, file)) != 1)
            {
                fprintf(stderr, "\n\033[31;1;7mERROR: failed to write green @ QOI_OP_RGBA.\033[0m\n");
                return 8;
            }
            if((elements = fwrite(&curr_pixel.blue, 1, 1, file)) != 1)
            {
                fprintf(stderr, "\n\033[31;1;7mERROR: failed to write blue @ QOI_OP_RGBA.\033[0m\n");
                return 8;
            }
            if((elements = fwrite(&curr_pixel.alpha, 1, 1, file)) != 1)
            {
                fprintf(stderr, "\n\033[31;1;7mERROR: failed to write alpha @ QOI_OP_RGBA.\033[0m\n");
                return 8;
            }

            #ifdef IMGSURF_LOGGING
            ++rgba_count;
            fprintf(stderr, "CURRENT WRITE OP: QOI_OP_RGBA with new pixel: %u, %u, %u, %u\n", curr_pixel.red, curr_pixel.green, curr_pixel.blue, curr_pixel.alpha);
            #endif
        }

        prev_pixel = curr_pixel;
        seen_pixels[index] = curr_pixel;
    }

    #ifdef IMGSURF_LOGGING
    fprintf(stderr, "\n\n");
    fprintf(stderr, "run_count:   %lu\n", run_count);
    fprintf(stderr, "diff_count:  %lu\n", diff_count);
    fprintf(stderr, "index_count: %lu\n", index_count);
    fprintf(stderr, "luma_count:  %lu\n", luma_count);
    fprintf(stderr, "rgb_count:   %lu\n", rgb_count);
    fprintf(stderr, "rgba_count:  %lu\n", rgba_count);
    #endif

    uint64_t EOS0 = 0x00;
    if((elements = fwrite(&EOS0, 7, 1, file)) != 1)
    {
        fprintf(stderr, "\n\033[31;1;7mERROR: failed to write EOS 0x00 tag.\033[0m\n");
        return 12;
    }

    uint8_t EOS1 = 0x01;
    if((elements = fwrite(&EOS1, 1, 1, file)) != 1)
    {
        fprintf(stderr, "\n\033[31;1;7mERROR: failed to write EOS 0x01 tag.\033[0m\n");
        return 13;
    }

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

    FILE *file = fopen(path, "wb");
    if(!file)
    {
        fprintf(stderr, "\n\033[31;1;7mERROR: Could not open file %s for writing!\033[0m\n", path);
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
                fprintf(stderr, "\n\033[31;1;7mERROR: Only a bit depth of 8 is supported by .qoi!\033[0m\n");
            }
            else
            {
                uint8_t result = writeQOI(file, data, width, height, channels);
                if(result)
                {
                    fprintf(stderr, "\n\033[31;1;7mERROR: couldn't correctly write .qoi!\033[0m\n");
                    fclose(file);
                    return result;
                }
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
                // writePNG();
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

    fprintf(stderr, "\033[31;1;1mERROR: format not implemented or supported. try QOI for now.\033[0m\n");
}

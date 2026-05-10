#include "imgsurf_main.h"

#include <stdlib.h>
#include <inttypes.h>

uint8_t* loadQOI
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
            fprintf(stderr, "\n\033[31;1;7mERROR: could not read header at byte %u."
                    "\033[0m\n", i);
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
            fprintf(stderr, "\n\033[31;1;7mERROR: QOI header at byte %u corrupted."
                    "\033[0m\n", i);
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
            fprintf(stderr, "\n\033[31;1;7mERROR: QOI header at byte %u corrupted."
                    "\033[0m\n", 4 + i);
            return 0;
        }
        *width += (uint32_t)(byte << (3 - i) * 8);
    }

    for(uint8_t i = 0; i < 4; ++i)
    {
        if((byte = fgetc(file)) == EOF)
        {
            fprintf(stderr, "\n\033[31;1;7mERROR: QOI header at byte %u corrupted."
                    "\033[0m\n", 8 + i);
            return 0;
        }
        *height += (uint32_t)(byte << (3 - i) * 8);
    }

    if((byte = fgetc(file)) == EOF)
    {
        fprintf(stderr, "\n\033[31;1;7mERROR: QOI header at byte 12 corrupted."
                "\033[0m\n");
    }
    // uint8_t channelcount = byte;

    if((byte = fgetc(file)) == EOF)
    {
        fprintf(stderr, "\n\033[31;1;7mERROR: QOI header at byte 13 corrupted."
                "\033[0m\n");
        return 0;
    }
    // uint8_t colourspace = byte;

    bool discardAlpha = channels == IMGSURF_CHANNELS_RGB ||
                        channels == IMGSURF_CHANNELS_BGR;

    bool flipRnB      = channels == IMGSURF_CHANNELS_BGR ||
                        channels == IMGSURF_CHANNELS_BGRA;

    uint64_t pixelcount = *width * *height;
    uint8_t  bpp        = discardAlpha ? 3 : 4;

    #ifdef IMGSURF_LOG_READ
    uint8_t *data = (uint8_t*)file;
    uint8_t *data_start = (uint8_t*)file;
    #endif

    uint8_t* image = malloc(pixelcount * bpp);
    if(!image)
    {
        fprintf(stderr, "\n\033[31;1;7mERROR: Failed to allocate image.");
        fprintf(stderr, "Requested Bytes: %" PRIu64 "\033[0m\n",
                (pixelcount * bpp));
        return 0;
    }

    pixel prev     = {0, 0, 0, 255};
    pixel seen[64] = {0};

    uint64_t loopWidth  = *width * bpp;
    uint64_t loopHeight = *height;

    #ifdef IMGSURF_LOG_READ
    uint64_t run_count   = 0;
    uint64_t diff_count  = 0;
    uint64_t index_count = 0;
    uint64_t luma_count  = 0;
    uint64_t rgb_count   = 0;
    uint64_t rgba_count  = 0;
    #endif

    for(uint64_t y = 0; y < loopHeight; y++)
    {
        for(uint64_t x = 0; x < loopWidth && ((byte = fgetc(file)) != EOF); x += bpp)
        {
            if(byte == QOI_OP_RGB)
            {
                if((elements = fread(&byte, 1, 1, file)) != 1)
                {
                    fprintf(stderr, "\n\033[31;1;7mERROR: failed to read red @ "
                            "QOI_OP_RGB.\033[0m\n");
                }
                prev.red = (uint8_t)byte;

                if((elements = fread(&byte, 1, 1, file)) != 1)
                {
                    fprintf(stderr, "\n\033[31;1;7mERROR: failed to read green @ "
                            "QOI_OP_RGB.\033[0m\n");
                }
                prev.green = (uint8_t)byte;

                if((elements = fread(&byte, 1, 1, file)) != 1)
                {
                    fprintf(stderr, "\n\033[31;1;7mERROR: failed to read blue @ "
                            "QOI_OP_RGB.\033[0m\n");
                }
                prev.blue = (uint8_t)byte;

                image[y * loopWidth + x]     = flipRnB ? prev.blue : prev.red;
                image[y * loopWidth + x + 1] = prev.green;
                image[y * loopWidth + x + 2] = flipRnB ? prev.red  : prev.blue;

                if(!discardAlpha)
                {
                    image[y * loopWidth + x + 3] = prev.alpha;
                }

                #ifdef IMGSURF_LOG_READ
                ++rgb_count;
                fprintf(stderr, "%lx: CURRENT READ OP: QOI_OP_RGB with new pixel: "
                        "%u, %u, %u\n",
                        data - data_start, prev.red, prev.green, prev.blue);
                #endif
                seen[IMGSURF_QOI_INDEX] = prev;
            }
            else if(byte == QOI_OP_RGBA)
            {
                if((elements = fread(&byte, 1, 1, file)) != 1)
                {
                    fprintf(stderr, "\n\033[31;1;7mERROR: failed to read red @ "
                            "QOI_OP_RGBA.\033[0m\n");
                }
                prev.red = (uint8_t)byte;

                if((elements = fread(&byte, 1, 1, file)) != 1)
                {
                    fprintf(stderr, "\n\033[31;1;7mERROR: failed to read green @ "
                            "QOI_OP_RGBA.\033[0m\n");
                }
                prev.green = (uint8_t)byte;

                if((elements = fread(&byte, 1, 1, file)) != 1)
                {
                    fprintf(stderr, "\n\033[31;1;7mERROR: failed to read blue @ "
                            "QOI_OP_RGBA.\033[0m\n");
                }
                prev.blue = (uint8_t)byte;

                if((elements = fread(&byte, 1, 1, file)) != 1)
                {
                    fprintf(stderr, "\n\033[31;1;7mERROR: failed to read alpha @ "
                            "QOI_OP_RGBA.\033[0m\n");
                }
                prev.alpha = (uint8_t)byte;

                image[y * loopWidth + x]     = flipRnB ? prev.blue : prev.red;
                image[y * loopWidth + x + 1] = prev.green;
                image[y * loopWidth + x + 2] = flipRnB ? prev.red  : prev.blue;

                if(!discardAlpha)
                {
                    image[y * loopWidth + x + 3] = prev.alpha;
                }

                #ifdef IMGSURF_LOG_READ
                ++rgba_count;
                fprintf(stderr, "%lx: CURRENT READ OP: QOI_OP_RGBA with new pixel: "
                        "%u, %u, %u, %u\n",
                        data - data_start, prev.red, prev.green, prev.blue, prev.alpha);
                #endif
                seen[IMGSURF_QOI_INDEX] = prev;
            }
            else if((byte >> 6) == QOI_OP_DIFF)
            {
                uint8_t diffRed   = (0x30 & byte) >> 4;
                uint8_t diffGreen = (0xC  & byte) >> 2;
                uint8_t diffBlue  =  0x3  & byte;

                prev.red   += diffRed   - 2;
                prev.green += diffGreen - 2;
                prev.blue  += diffBlue  - 2;

                image[y * loopWidth + x]     = flipRnB ? prev.blue : prev.red;
                image[y * loopWidth + x + 1] = prev.green;
                image[y * loopWidth + x + 2] = flipRnB ? prev.red  : prev.blue;

                if(!discardAlpha)
                {
                    image[y * loopWidth + x + 3] = prev.alpha;
                }

                seen[IMGSURF_QOI_INDEX] = prev;
                #ifdef IMGSURF_LOG_READ
                ++diff_count;
                fprintf(stderr, "%lx: CURRENT READ OP: QOI_OP_DIFF with diff: %u, %u, "
                        "%u\n", data - data_start, diffRed, diffGreen, diffBlue);
                #endif
            }
            else if((byte >> 6) == QOI_OP_LUMA)
            {
                uint8_t diffGreen = byte & 0x3F;

                if((elements = fread(&byte, 1, 1, file)) != 1)
                {
                    fprintf(stderr, "\n\033[31;1;7mERROR: failed to read diffs @ "
                            "QOI_OP_LUMA.\033[0m\n");
                }
                uint8_t diffRed  = (0xF0 & byte) >> 4;
                uint8_t diffBlue = (0xF  & byte);

                prev.green += diffGreen - 32;
                prev.red   += diffGreen + diffRed  - 40;
                prev.blue  += diffGreen + diffBlue - 40;

                image[y * loopWidth + x]     = flipRnB ? prev.blue : prev.red;
                image[y * loopWidth + x + 1] = prev.green;
                image[y * loopWidth + x + 2] = flipRnB ? prev.red  : prev.blue;

                if(!discardAlpha)
                {
                    image[y * loopWidth + x + 3] = prev.alpha;
                }

                seen[IMGSURF_QOI_INDEX] = prev;
                #ifdef IMGSURF_LOG_READ
                ++luma_count;
                fprintf(stderr, "%lx: CURRENT READ OP: QOI_OP_LUMA with encoded diffs: "
                        "%u, %u, %u\n", data - data_start, (uint8_t)(diffGreen),
                        (uint8_t)(diffRed), (uint8_t)(diffBlue));
                #endif
            }
            else if((byte >> 6) == QOI_OP_RUN)
            {
                uint8_t  runlength = (byte & 0x3F) + 1;
                uint64_t index     = y * loopWidth + x;

                for(uint8_t j = 0; j < runlength; ++j)
                {
                    image[index + j * bpp]     = flipRnB ? prev.blue : prev.red;
                    image[index + j * bpp + 1] = prev.green;
                    image[index + j * bpp + 2] = flipRnB ? prev.red  : prev.blue;

                    if(!discardAlpha)
                    {
                        image[index + j * bpp + 3] = prev.alpha;
                    }

                    if(index + j * bpp < loopHeight * loopWidth)
                    {
                        break;
                    }
                }
                x += (runlength - 1) * bpp;
                y += x / loopWidth;
                x %= loopWidth;

                #ifdef IMGSURF_LOG_READ
                data += (runlength - 1) * bpp;
                ++run_count;
                fprintf(stderr, "%lx: (%x) CURRENT READ OP: QOI_OP_RUN with pixel: "
                        "%u, %u, %u and runlength %hhu\n", data - data_start, byte,
                        prev.red, prev.green, prev.blue, runlength);
                #endif
            }
            else if((byte >> 6) == QOI_OP_INDEX)
            {
                if(byte == 0x00)
                {
                    int byteBuffer[8];
                    uint8_t zeroCounter = 0;
                    for(uint8_t i = 0; i < 6 && (byteBuffer[i] = fgetc(file)) == 0; ++i)
                    {
                        if(++zeroCounter == 6)
                        {
                            if((byteBuffer[i + 1] = fgetc(file)) == 0x01)
                            {
                                fprintf(stderr, "\n\033[31;1;7mERROR: QOI end-of-stream"
                                        " reached early.\033[0m\n");
                                return image;
                            }
                        }
                    }
                    for(uint8_t i = 0; i < zeroCounter + 1; ++i)
                    {
                        ungetc(byteBuffer[zeroCounter - i], file);
                    }
                }

                prev = seen[byte % 0x40];

                image[y * loopWidth + x]     = flipRnB ? prev.blue : prev.red;
                image[y * loopWidth + x + 1] = prev.green;
                image[y * loopWidth + x + 2] = flipRnB ? prev.red  : prev.blue;

                if(!discardAlpha)
                {
                    image[y * loopWidth + x + 3] = prev.alpha;
                }
                #ifdef IMGSURF_LOG_READ
                ++index_count;
                fprintf(stderr, "%lx: CURRENT READ OP: QOI_OP_INDEX with index: %u\n",
                        data - data_start, byte % 0x40);
                #endif
            }
            else
            {
                fprintf(stderr, "\n\033[31;1;7mERROR: Unknown QOI_OP.\n");
                return image;
            }
            #ifdef IMGSURF_LOG_READ
            data += bpp;
            #endif
        }
    }

    #ifdef IMGSURF_LOG_READ
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
                #ifdef IMGSURF_LOG_READ
                    fprintf(stderr, "QOI end-of-stream reached.\n");
                #endif
                return image;
            }
        }
    }

    return image;
}

uint8_t writeQOI
(
    FILE     *file,
    uint8_t  *data,
    uint32_t width,
    uint32_t height,
    uint8_t  channels
){
    uint8_t *data_start  = data;
    uint8_t channelcount = (channels == IMGSURF_CHANNELS_RGBA ||
                            channels == IMGSURF_CHANNELS_BGRA) ? 4 : 3;

    if(channelcount != 3 && channelcount != 4)
    {
        fprintf(stderr, "\n\033[31;1;7mERROR: QOI only supports channelcounts of 3 "
                "(RGB) or 4 (RGBA). Got: %u\033[0m\n", channelcount);
        return 1;
    }

    // colourspace ignored
    uint8_t colourspace = 0;
    size_t  elements    = 0;

    if((elements = fwrite("qoif", 4, 1, file)) != 1)
    {
        fprintf(stderr, "\n\033[31;1;7mERROR: failed to write magic bytes of header."
                "\033[0m\n");
        return 2;
    }

    for(int8_t i = 3; i > -1; --i)
    {
        uint8_t width_shifted = (width >> (i * 8)) & 0xFF;
        if((elements = fwrite(&width_shifted, 1, 1, file)) != 1)
        {
            fprintf(stderr, "\n\033[31;1;7mERROR: failed to write width @ byte: %u."
                    "\033[0m\n", i);
            return 2;
        }
    }

    for(int8_t i = 3; i > -1; --i)
    {
        uint8_t height_shifted = (height >> (i * 8)) & 0xFF;
        if((elements = fwrite(&height_shifted, 1, 1, file)) != 1)
        {
            fprintf(stderr, "\n\033[31;1;7mERROR: failed to write height @ byte: %u."
                    "\033[0m\n", i);
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

    pixel prev     = {0, 0, 0, 255};
    pixel seen[64] = {0};

    bool flipRnB  = channels == IMGSURF_CHANNELS_BGR  ||
                    channels == IMGSURF_CHANNELS_BGRA;
    bool useAlpha = channels == IMGSURF_CHANNELS_RGBA ||
                    channels == IMGSURF_CHANNELS_BGRA;

    #ifdef IMGSURF_LOG_WRITE
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
        pixel curr = {0, 0, 0, 255};

        uint8_t og_r = *data;
        uint8_t og_b = *(data + 2);

        curr.red   = flipRnB ? og_b : og_r;
        curr.green = *(data + 1);
        curr.blue  = flipRnB ? og_r : og_b;
        if(useAlpha)
        {
            curr.alpha = *(data + 3);
        }

        uint8_t dr = curr.red   - prev.red;
        uint8_t dg = curr.green - prev.green;
        uint8_t db = curr.blue  - prev.blue;

        uint8_t index = (curr.red  * 3 + curr.green * 5
                       + curr.blue * 7 + curr.alpha * 11) % 64;

        if(same_pixel(curr, prev))
        {
            ++runlength;
            if(runlength == 62 || data + channelcount == data_end)
            {
                uint8_t byte = 0xC0 | (runlength - 1);
                if((elements = fwrite(&byte, 1, 1, file)) != 1)
                {
                    fprintf(stderr, "\n\033[31;1;7mERROR: failed to write data @ "
                            "QOI_OP_RUN.\033[0m\n");
                    return 3;
                }

                #ifdef IMGSURF_LOG_WRITE
                ++run_count;
                fprintf(stderr, "%lx: (%x) CURRENT WRITE OP: QOI_OP_RUN with pixel: "
                        "%u, %u, %u, %u, runlength: %hhu\n", data - data_start, byte,
                        prev.red, prev.green, prev.blue, prev.alpha, runlength);
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
                fprintf(stderr, "\n\033[31;1;7mERROR: failed to write data @ "
                        "QOI_OP_RUN.\033[0m\n");
                return 3;
            }

            #ifdef IMGSURF_LOG_WRITE
            ++run_count;
            fprintf(stderr, "%lx: (%x) CURRENT WRITE OP: QOI_OP_RUN with pixel: "
                    "%u, %u, %u, %u, runlength: %hhu\n", data - data_start, byte,
                    prev.red, prev.green, prev.blue, prev.alpha, runlength);
            #endif
            runlength = 0;
        }

        if(same_pixel(curr, seen[index]))
        {
            if((elements = fwrite(&index, 1, 1, file)) != 1)
            {
                fprintf(stderr, "\n\033[31;1;7mERROR: failed to write data @ "
                        "QOI_OP_INDEX.\033[0m\n");
                return 5;
            }

            #ifdef IMGSURF_LOG_WRITE
            ++index_count;
            fprintf(stderr, "%lx: CURRENT WRITE OP: QOI_OP_INDEX with index: "
                    "%u\n", data - data_start, index);
            #endif
        }
        else if(curr.alpha == prev.alpha &&
                (uint8_t)(dr + 2) < 0x4  &&
                (uint8_t)(dg + 2) < 0x4  &&
                (uint8_t)(db + 2) < 0x4
        ){
            uint8_t byte = (uint8_t)(0x40 | ((uint8_t)(dr + 2) << 4) |
                                     ((uint8_t)(dg + 2) << 2)        |
                                     (uint8_t)(db + 2));

            if((elements = fwrite(&byte, 1, 1, file)) != 1)
            {
                fprintf(stderr, "\n\033[31;1;7mERROR: failed to write data @ "
                        "QOI_OP_DIFF.\033[0m\n");
                return 4;
            }

            #ifdef IMGSURF_LOG_WRITE
            ++diff_count;
            fprintf(stderr, "%lx: CURRENT WRITE OP: QOI_OP_DIFF and diff: "
                    "%i, %i, %i\n", data - data_start,
                    (uint8_t)(dr + 2), (uint8_t)(dg + 2), (uint8_t)(db + 2));
            #endif
        }
        else if(curr.alpha == prev.alpha      &&
                (uint8_t)(dg + 32) < 0x40     &&
                (uint8_t)(dr - dg + 8) < 0x10 &&
                (uint8_t)(db - dg + 8) < 0x10
        ){
            uint8_t byte1 = 0x80 | (uint8_t)(dg + 32);
            uint8_t byte2 = (uint8_t)(((uint8_t)(dr - dg + 8) << 4) |
                            (uint8_t)(db - dg + 8));

            if((elements = fwrite(&byte1, 1, 1, file)) != 1)
            {
                fprintf(stderr, "\n\033[31;1;7mERROR: failed to write tag/dg @ "
                        "QOI_OP_LUMA.\033[0m\n");
                return 6;
            }

            if((elements = fwrite(&byte2, 1, 1, file)) != 1)
            {
                fprintf(stderr, "\n\033[31;1;7mERROR: failed to write dr/db data @ "
                        "QOI_OP_LUMA.\033[0m\n");
                return 6;
            }

            #ifdef IMGSURF_LOG_WRITE
            ++luma_count;
            fprintf(stderr, "%lx: CURRENT WRITE OP: QOI_OP_LUMA and encoded diffs: "
                    "%u, %u, %u\n", data - data_start, (uint8_t)(dg + 32),
                    (uint8_t)(dr - dg + 8),
                    (uint8_t)(db - dg + 8));
            #endif
        }
        else if(curr.alpha == prev.alpha)
        {
            uint8_t tag  = 0xFE;
            if((elements = fwrite(&tag, 1, 1, file)) != 1)
            {
                fprintf(stderr, "\n\033[31;1;7mERROR: failed to write tag @ "
                        "QOI_OP_RGB.\033[0m\n");
                return 7;
            }

            if((elements = fwrite(&curr.red, 1, 1, file)) != 1)
            {
                fprintf(stderr, "\n\033[31;1;7mERROR: failed to write red @ "
                        "QOI_OP_RGB.\033[0m\n");
                return 8;
            }
            if((elements = fwrite(&curr.green, 1, 1, file)) != 1)
            {
                fprintf(stderr, "\n\033[31;1;7mERROR: failed to write green @ "
                        "QOI_OP_RGB.\033[0m\n");
                return 8;
            }
            if((elements = fwrite(&curr.blue, 1, 1, file)) != 1)
            {
                fprintf(stderr, "\n\033[31;1;7mERROR: failed to write blue @ "
                        "QOI_OP_RGB.\033[0m\n");
                return 8;
            }

            #ifdef IMGSURF_LOG_WRITE
            ++rgb_count;
            fprintf(stderr, "%lx: CURRENT WRITE OP: QOI_OP_RGB with new pixel: "
                    "%u, %u, %u\n", data - data_start, curr.red, curr.green, curr.blue);
            #endif
        }
        else
        {
            uint8_t tag  = 0xFF;
            if((elements = fwrite(&tag, 1, 1, file)) != 1)
            {
                fprintf(stderr, "\n\033[31;1;7mERROR: failed to write tag @ "
                        "QOI_OP_RGBA.\033[0m\n");
                return 10;
            }

            if((elements = fwrite(&curr.red, 1, 1, file)) != 1)
            {
                fprintf(stderr, "\n\033[31;1;7mERROR: failed to write red @ "
                        "QOI_OP_RGBA.\033[0m\n");
                return 8;
            }
            if((elements = fwrite(&curr.green, 1, 1, file)) != 1)
            {
                fprintf(stderr, "\n\033[31;1;7mERROR: failed to write green @ "
                        "QOI_OP_RGBA.\033[0m\n");
                return 8;
            }
            if((elements = fwrite(&curr.blue, 1, 1, file)) != 1)
            {
                fprintf(stderr, "\n\033[31;1;7mERROR: failed to write blue @ "
                        "QOI_OP_RGBA.\033[0m\n");
                return 8;
            }
            if((elements = fwrite(&curr.alpha, 1, 1, file)) != 1)
            {
                fprintf(stderr, "\n\033[31;1;7mERROR: failed to write alpha @ "
                        "QOI_OP_RGBA.\033[0m\n");
                return 8;
            }

            #ifdef IMGSURF_LOG_WRITE
            ++rgba_count;
            fprintf(stderr, "CURRENT WRITE OP: QOI_OP_RGBA with new pixel: %u, %u, %u, "
                    "%u\n", curr.red, curr.green, curr.blue, curr.alpha);
            #endif
        }

        prev = curr;
        seen[index] = curr;
    }

    #ifdef IMGSURF_LOG_WRITE
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

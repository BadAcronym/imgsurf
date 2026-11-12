#include "imgsurf_main.h"
#include "im_print_macros.h"

#include <stdlib.h>

#define IM_QOI_INDEX \
(prev.red * 3 + prev.green * 5 + \
prev.blue * 7 + prev.alpha * 11) % 64

#define QOI_OP_RGB    254
#define QOI_OP_RGBA   255

#define QOI_OP_INDEX  0
#define QOI_OP_DIFF   1
#define QOI_OP_LUMA   2
#define QOI_OP_RUN    3

f_internal bool samePixel
(
    pixel pixel1,
    pixel pixel2
){
    return (pixel1.red  == pixel2.red  && pixel1.green == pixel2.green &&
            pixel1.blue == pixel2.blue && pixel1.alpha == pixel2.alpha);
}

u08* loadQOI
(
    FILE *file,
    u32  *width,
    u32  *height,
    u08  channels
){
    char magic[5] = "qoif";
    *width  = 0;
    *height = 0;

    size elements  = 0;
    char magicByte = 0;

    for(u08 i = 0; i < 4; ++i)
    {
        if((elements = fread(&magicByte, 1, 1, file)) != 1)
        {
            IM_ERROR("could not read header at byte %u.", i);
            if(feof(file))
            {
                IM_ERROR("unexpected end of file.");
            }
            else if(ferror(file))
            {
                IM_ERROR("could not read file.");
            }

            return 0;
        }

        if(magicByte != magic[i])
        {
            IM_ERROR("QOI header at byte %u corrupted. Got: %u, expected: %u",
                     i, (u08)magicByte, (u08)magic[i]);
            return 0;
        }
    }

    int byte = 0;
    for(u08 i = 0; i < 4; ++i)
    {
        if((byte = fgetc(file)) == EOF)
        {
            IM_ERROR("QOI header at byte %u corrupted.", 4 + i);
            return 0;
        }
        *width += (u32)(byte << (3 - i) * 8);
    }

    for(u08 i = 0; i < 4; ++i)
    {
        if((byte = fgetc(file)) == EOF)
        {
            IM_ERROR("QOI header at byte %u corrupted.", 8 + i);
            return 0;
        }
        *height += (u32)(byte << (3 - i) * 8);
    }

    if((byte = fgetc(file)) == EOF)
    {
        IM_ERROR("QOI header at byte 12 corrupted.");
    }
    bool discardAlpha = byte == 3;

    if((byte = fgetc(file)) == EOF)
    {
        IM_ERROR("QOI header at byte 13 corrupted.");
        return 0;
    }
    // u08 colourspace = byte;

    // this is not really correct. but we'll see
    bool flipRnB = channels == IM_CHANNELS_BGR ||
                   channels == IM_CHANNELS_BGRA;

    u64 pixelcount = *width * *height;
    u08 bpp        = discardAlpha ? 3 : 4;

    #ifdef IM_LOG_READ
    u08 *data = (u08*)file;
    u08 *data_start = (u08*)file;
    #endif

    u08* image = malloc(pixelcount * bpp);
    if(!image)
    {
        IM_ERROR("failed to allocate image. Requested bytes: %lu", pixelcount * bpp);
        return 0;
    }

    pixel prev     = {0, 0, 0, 255};
    pixel seen[64] = {0};

    u64 loopWidth  = *width * bpp;
    u64 loopHeight = *height;

    #ifdef IM_LOG_READ
    u64 run_count   = 0;
    u64 diff_count  = 0;
    u64 index_count = 0;
    u64 luma_count  = 0;
    u64 rgb_count   = 0;
    u64 rgba_count  = 0;
    #endif

    for(u64 y = 0; y < loopHeight; y++)
    {
        for(u64 x = 0; x < loopWidth && ((byte = fgetc(file)) != EOF); x += bpp)
        {
            if(byte == QOI_OP_RGB)
            {
                if((elements = fread(&byte, 1, 1, file)) != 1)
                {
                    IM_ERROR("failed to read red @ QOI_OP_RGB");
                }
                prev.red = (u08)byte;

                if((elements = fread(&byte, 1, 1, file)) != 1)
                {
                    IM_ERROR("failed to read green @ QOI_OP_RGB");
                }
                prev.green = (u08)byte;

                if((elements = fread(&byte, 1, 1, file)) != 1)
                {
                    IM_ERROR("failed to read blue @ QOI_OP_RGB");
                }
                prev.blue = (u08)byte;

                image[y * loopWidth + x]     = flipRnB ? prev.blue : prev.red;
                image[y * loopWidth + x + 1] = prev.green;
                image[y * loopWidth + x + 2] = flipRnB ? prev.red  : prev.blue;

                if(!discardAlpha)
                {
                    image[y * loopWidth + x + 3] = prev.alpha;
                }

                #ifdef IM_LOG_READ
                ++rgb_count;
                IM_DEBUG("%lx: CURRENT READ OP: QOI_OP_RGB with new pixel: %u, %u, %u",
                         data - data_start, prev.red, prev.green, prev.blue);
                #endif
                seen[IM_QOI_INDEX] = prev;
            }
            else if(byte == QOI_OP_RGBA)
            {
                if((elements = fread(&byte, 1, 1, file)) != 1)
                {
                    IM_ERROR("failed to read red @ QOI_OP_RGBA");
                }
                prev.red = (u08)byte;

                if((elements = fread(&byte, 1, 1, file)) != 1)
                {
                    IM_ERROR("failed to read green @ QOI_OP_RGBA");
                }
                prev.green = (u08)byte;

                if((elements = fread(&byte, 1, 1, file)) != 1)
                {
                    IM_ERROR("failed to read blue @ QOI_OP_RGBA");
                }
                prev.blue = (u08)byte;

                if((elements = fread(&byte, 1, 1, file)) != 1)
                {
                    IM_ERROR("failed to read alpha @ QOI_OP_RGBA");
                }
                prev.alpha = (u08)byte;

                image[y * loopWidth + x]     = flipRnB ? prev.blue : prev.red;
                image[y * loopWidth + x + 1] = prev.green;
                image[y * loopWidth + x + 2] = flipRnB ? prev.red  : prev.blue;

                if(!discardAlpha)
                {
                    image[y * loopWidth + x + 3] = prev.alpha;
                }

                #ifdef IM_LOG_READ
                ++rgba_count;
                IM_DEBUG("%lx: CURRENT READ OP: QOI_OP_RGBA with new pixel: "
                         "%u, %u, %u, %u", data - data_start,
                         prev.red, prev.green, prev.blue, prev.alpha);
                #endif
                seen[IM_QOI_INDEX] = prev;
            }
            else if((byte >> 6) == QOI_OP_DIFF)
            {
                u08 diffRed   = (0x30 & byte) >> 4;
                u08 diffGreen = (0xC  & byte) >> 2;
                u08 diffBlue  =  0x3  & byte;

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

                seen[IM_QOI_INDEX] = prev;
                #ifdef IM_LOG_READ
                ++diff_count;
                IM_DEBUG("%lx: CURRENT READ OP: QOI_OP_DIFF with diff: %u, %u, %u",
                         data - data_start, diffRed, diffGreen, diffBlue);
                #endif
            }
            else if((byte >> 6) == QOI_OP_LUMA)
            {
                u08 diffGreen = byte & 0x3F;

                if((elements = fread(&byte, 1, 1, file)) != 1)
                {
                    IM_ERROR("failed to read diffs @ QOI_OP_LUMA.");
                }
                u08 diffRed  = (0xF0 & byte) >> 4;
                u08 diffBlue = (0xF  & byte);

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

                seen[IM_QOI_INDEX] = prev;
                #ifdef IM_LOG_READ
                ++luma_count;
                IM_DEBUG("%lx: CURRENT READ OP: QOI_OP_LUMA with encoded diffs: "
                         "%u, %u, %u", data - data_start, (u08)(diffGreen),
                         (u08)(diffRed), (u08)(diffBlue));
                #endif
            }
            else if((byte >> 6) == QOI_OP_RUN)
            {
                u08  runlength = (byte & 0x3F) + 1;
                u64 index     = y * loopWidth + x;

                u08 j  = 0;
                bool under = index + j * bpp < loopHeight * loopWidth;

                for(; j < runlength && under; ++j)
                {
                    image[index + j * bpp]     = flipRnB ? prev.blue : prev.red;
                    image[index + j * bpp + 1] = prev.green;
                    image[index + j * bpp + 2] = flipRnB ? prev.red  : prev.blue;

                    if(!discardAlpha)
                    {
                        image[index + j * bpp + 3] = prev.alpha;
                    }
                }
                x += (runlength - 1) * bpp;
                y += x / loopWidth;
                x %= loopWidth;

                #ifdef IM_LOG_READ
                data += (runlength - 1) * bpp;
                ++run_count;
                IM_DEBUG("%lx: (%x) CURRENT READ OP: QOI_OP_RUN with pixel: "
                         "%u, %u, %u and runlength %hhu", data - data_start, byte,
                         prev.red, prev.green, prev.blue, runlength);
                #endif
            }
            else if((byte >> 6) == QOI_OP_INDEX)
            {
                if(byte == 0x00)
                {
                    int byteBuffer[8];
                    u08 zeroCounter = 0;
                    for(u08 i = 0; i < 6 && (byteBuffer[i] = fgetc(file)) == 0; ++i)
                    {
                        if(++zeroCounter == 6)
                        {
                            if((byteBuffer[i + 1] = fgetc(file)) == 0x01)
                            {
                                IM_ERROR("QOI end-of-stream reached early.");
                                return image;
                            }
                        }
                    }
                    for(u08 i = 0; i < zeroCounter + 1; ++i)
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
                #ifdef IM_LOG_READ
                ++index_count;
                IM_DEBUG("%lx: CURRENT READ OP: QOI_OP_INDEX with index: %u",
                         data - data_start, byte % 0x40);
                #endif
            }
            else
            {
                IM_ERROR("could not read QOI_OP.");
                return image;
            }
            #ifdef IM_LOG_READ
            data += bpp;
            #endif
        }
    }

    #ifdef IM_LOG_READ
    IM_DEBUG("\n\nREAD BACK (actual):\n");
    IM_DEBUG("run_count:   %lu", run_count);
    IM_DEBUG("diff_count:  %lu", diff_count);
    IM_DEBUG("index_count: %lu", index_count);
    IM_DEBUG("luma_count:  %lu", luma_count);
    IM_DEBUG("rgb_count:   %lu", rgb_count);
    IM_DEBUG("rgba_count:  %lu", rgba_count);
    IM_DEBUG("\n");
    #endif

    int byteBuffer[8];
    u08 zeroCounter = 0;
    for(u08 i = 0; i < 7 && (byteBuffer[i] = fgetc(file)) == 0x00; ++i)
    {
        if(++zeroCounter == 7)
        {
            if((byteBuffer[i + 1] = fgetc(file)) == 0x01)
            {
                #ifdef IM_LOG_READ
                IM_DEBUG("QOI end-of-stream reached.");
                #endif
                return image;
            }
        }
    }

    return image;
}

bool writeQOI
(
    FILE *file,
    u08  *data,
    u32  width,
    u32  height,
    u08  channels
){
    #ifdef IM_LOG_WRITE
    u08 *data_start  = data;
    #endif
    u08 channelcount = (channels == IM_CHANNELS_RGBA ||
                        channels == IM_CHANNELS_BGRA) ? 4 : 3;

    // colourspace ignored
    u08  colourspace = 0;
    size elements    = 0;

    if((elements = fwrite("qoif", 4, 1, file)) != 1)
    {
        IM_ERROR("failed to write magic bytes to header.");
        return false;
    }

    for(i08 i = 3; i > -1; --i)
    {
        u08 width_shifted = (width >> (i * 8)) & 0xFF;
        if((elements = fwrite(&width_shifted, 1, 1, file)) != 1)
        {
            IM_ERROR("failed to write width @ byte: %u.", i);
            return false;
        }
    }

    for(i08 i = 3; i > -1; --i)
    {
        u08 height_shifted = (height >> (i * 8)) & 0xFF;
        if((elements = fwrite(&height_shifted, 1, 1, file)) != 1)
        {
            IM_ERROR("failed to write height @ byte: %u.", i);
            return false;
        }
    }

    if((elements = fwrite(&channelcount, 1, 1, file)) != 1)
    {
        IM_ERROR("failed to write channelcount.");
        return false;
    }

    if((elements = fwrite(&colourspace, 1, 1, file)) != 1)
    {
        IM_ERROR("failed to write colourspace.");
        return false;
    }

    pixel prev     = {0, 0, 0, 255};
    pixel seen[64] = {0};

    bool useAlpha = channelcount == 3;

    bool flipRnB  = channels == IM_CHANNELS_BGR || channels == IM_CHANNELS_BGRA;

    #ifdef IM_LOG_WRITE
    u64 run_count   = 0;
    u64 diff_count  = 0;
    u64 index_count = 0;
    u64 luma_count  = 0;
    u64 rgb_count   = 0;
    u64 rgba_count  = 0;
    #endif

    u08 *data_end = data + width * height * channelcount;
    u08 runlength = 0;

    for(; data < data_end; data += channelcount)
    {
        pixel curr = {0, 0, 0, 255};

        u08 og_r = *data;
        u08 og_b = *(data + 2);

        curr.red   = flipRnB ? og_b : og_r;
        curr.green = *(data + 1);
        curr.blue  = flipRnB ? og_r : og_b;
        if(useAlpha)
        {
            curr.alpha = *(data + 3);
        }

        u08 dr = curr.red   - prev.red;
        u08 dg = curr.green - prev.green;
        u08 db = curr.blue  - prev.blue;

        u08 index = (curr.red  * 3 + curr.green * 5 +
                     curr.blue * 7 + curr.alpha * 11) % 64;

        if(samePixel(curr, prev))
        {
            ++runlength;
            if(runlength == 62 || data + channelcount == data_end)
            {
                u08 byte = 0xC0 | (runlength - 1);
                if((elements = fwrite(&byte, 1, 1, file)) != 1)
                {
                    IM_ERROR("failed to write data @ QOI_OP_RUN.");
                    return false;
                }

                #ifdef IM_LOG_WRITE
                ++run_count;
                IM_DEBUG("%lx: (%x) CURRENT WRITE OP: QOI_OP_RUN with pixel: "
                         "%u, %u, %u, %u, runlength: %hhu", data - data_start, byte,
                         prev.red, prev.green, prev.blue, prev.alpha, runlength);
                IM_DEBUG("data_end is: %lx", data_end - data_start);
                #endif
                runlength = 0;
            }
            continue;
        }

        if(runlength > 0)
        {
            u08 byte = 0xC0 | (runlength - 1);
            if((elements = fwrite(&byte, 1, 1, file)) != 1)
            {
                IM_ERROR("failed to write data @ QOI_OP_RUN.");
                return false;
            }

            #ifdef IM_LOG_WRITE
            ++run_count;
            IM_DEBUG("%lx: (%x) CURRENT WRITE OP: QOI_OP_RUN with pixel: "
                     "%u, %u, %u, %u, runlength: %hhu", data - data_start, byte,
                     prev.red, prev.green, prev.blue, prev.alpha, runlength);
            #endif
            runlength = 0;
        }

        if(samePixel(curr, seen[index]))
        {
            if((elements = fwrite(&index, 1, 1, file)) != 1)
            {
                IM_ERROR("failed to write data @ QOI_OP_INDEX.");
                return false;
            }

            #ifdef IM_LOG_WRITE
            ++index_count;
            IM_DEBUG("%lx: CURRENT WRITE OP: QOI_OP_INDEX with index: "
                     "%u", data - data_start, index);
            #endif
        }
        else if(curr.alpha == prev.alpha &&
                (u08)(dr + 2) < 0x4  &&
                (u08)(dg + 2) < 0x4  &&
                (u08)(db + 2) < 0x4
        ){
            u08 byte = (u08)(0x40 | ((u08)(dr + 2) << 4) |
                        ((u08)(dg + 2) << 2) | (u08)(db + 2));

            if((elements = fwrite(&byte, 1, 1, file)) != 1)
            {
                IM_ERROR("failed to write data @ QOI_OP_DIFF.");
                return false;
            }

            #ifdef IM_LOG_WRITE
            ++diff_count;
            IM_DEBUG("%lx: CURRENT WRITE OP: QOI_OP_DIFF and diff: "
                     "%i, %i, %i", data - data_start,
                     (u08)(dr + 2), (u08)(dg + 2), (u08)(db + 2));
            #endif
        }
        else if(curr.alpha == prev.alpha  &&
                (u08)(dg + 32)     < 0x40 &&
                (u08)(dr - dg + 8) < 0x10 &&
                (u08)(db - dg + 8) < 0x10
        ){
            u08 byte1 = 0x80 | (u08)(dg + 32);
            u08 byte2 = (u08)(((u08)(dr - dg + 8) << 4) |
                              (u08)(db - dg + 8));

            if((elements = fwrite(&byte1, 1, 1, file)) != 1)
            {
                IM_ERROR("failed to write tag/dg @ QOI_OP_LUMA.");
                return false;
            }

            if((elements = fwrite(&byte2, 1, 1, file)) != 1)
            {
                IM_ERROR("failed to write dr/db data @ QOI_OP_LUMA.");
                return false;
            }

            #ifdef IM_LOG_WRITE
            ++luma_count;
            IM_DEBUG("%lx: CURRENT WRITE OP: QOI_OP_LUMA and encoded diffs: "
                     "%u, %u, %u", data - data_start, (u08)(dg + 32),
                     (u08)(dr - dg + 8), (u08)(db - dg + 8));
            #endif
        }
        else if(curr.alpha == prev.alpha)
        {
            u08 tag = 0xFE;
            if((elements = fwrite(&tag, 1, 1, file)) != 1)
            {
                IM_ERROR("failed to write tag @ QOI_OP_RGB.");
                return false;
            }

            if((elements = fwrite(&curr.red, 1, 1, file)) != 1)
            {
                IM_ERROR("failed to write red @ QOI_OP_RGB.");
                return false;
            }
            if((elements = fwrite(&curr.green, 1, 1, file)) != 1)
            {
                IM_ERROR("failed to write green @ QOI_OP_RGB.");
                return false;
            }
            if((elements = fwrite(&curr.blue, 1, 1, file)) != 1)
            {
                IM_ERROR("failed to write blue @ QOI_OP_RGB.");
                return false;
            }

            #ifdef IM_LOG_WRITE
            ++rgb_count;
            IM_DEBUG("%lx: CURRENT WRITE OP: QOI_OP_RGB with new pixel: "
                     "%u, %u, %u", data - data_start,
                     curr.red, curr.green, curr.blue);
            #endif
        }
        else
        {
            u08 tag = 0xFF;
            if((elements = fwrite(&tag, 1, 1, file)) != 1)
            {
                IM_ERROR("failed to write tag @ QOI_OP_RGBA.");
                return false;
            }

            if((elements = fwrite(&curr.red, 1, 1, file)) != 1)
            {
                IM_ERROR("failed to write red @ QOI_OP_RGBA.");
                return false;
            }
            if((elements = fwrite(&curr.green, 1, 1, file)) != 1)
            {
                IM_ERROR("failed to write green @ QOI_OP_RGBA.");
                return false;
            }
            if((elements = fwrite(&curr.blue, 1, 1, file)) != 1)
            {
                IM_ERROR("failed to write blue @ QOI_OP_RGBA.");
                return false;
            }
            if((elements = fwrite(&curr.alpha, 1, 1, file)) != 1)
            {
                IM_ERROR("failed to write alpha @ QOI_OP_RGBA.");
                return false;
            }

            #ifdef IM_LOG_WRITE
            ++rgba_count;
            IM_DEBUG("CURRENT WRITE OP: QOI_OP_RGBA with new pixel: %u, %u, %u, "
                     "%u", curr.red, curr.green, curr.blue, curr.alpha);
            #endif
        }

        prev = curr;
        seen[index] = curr;
    }

    #ifdef IM_LOG_WRITE
    IM_DEBUG("\n");
    IM_DEBUG("run_count:   %lu", run_count);
    IM_DEBUG("diff_count:  %lu", diff_count);
    IM_DEBUG("index_count: %lu", index_count);
    IM_DEBUG("luma_count:  %lu", luma_count);
    IM_DEBUG("rgb_count:   %lu", rgb_count);
    IM_DEBUG("rgba_count:  %lu", rgba_count);
    #endif

    u64 EOS0 = 0x00;
    if((elements = fwrite(&EOS0, 7, 1, file)) != 1)
    {
        IM_ERROR("failed to write EOS 0x00 tag.");
        return false;
    }

    u08 EOS1 = 0x01;
    if((elements = fwrite(&EOS1, 1, 1, file)) != 1)
    {
        IM_ERROR("failed to write EOS 0x01 tag.");
        return false;
    }

    return true;
}

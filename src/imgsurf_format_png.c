#include "imgsurf_main.h"
#include "im_print_macros.h"

#include "datasurf_main.h"

#include "pd_string_view.h"

#define PNG_STREAM_END      0
#define PNG_STREAM_CONTINUE 1

#define IM_PNG_TYPE_GREYSCALE        0
#define IM_PNG_TYPE_TRUECOLOUR       2
#define IM_PNG_TYPE_INDEXED          3
#define IM_PNG_TYPE_GREYSCALE_ALPHA  4
#define IM_PNG_TYPE_TRUECOLOUR_ALPHA 6

s_global StringView IHDR = { .data = "IHDR", .size = 4};
s_global StringView PLTE = { .data = "PLTE", .size = 4};
s_global StringView IDAT = { .data = "IDAT", .size = 4};
s_global StringView IEND = { .data = "IEND", .size = 4};
s_global StringView cHRM = { .data = "cHRM", .size = 4};
s_global StringView bKGD = { .data = "bKGD", .size = 4};
s_global StringView tIME = { .data = "tIME", .size = 4};
s_global StringView tEXt = { .data = "tEXt", .size = 4};

typedef struct RGB8
{
    u08 red;
    u08 green;
    u08 blue;
}
RGB8;

typedef struct RGB16
{
    u16 red;
    u16 green;
    u16 blue;
}
RGB16;

typedef struct IHDRData
{
    u32 width;
    u32 height;
    u08 bitdepth;
    u08 colorType;
    u08 compression;
    u08 filter;
    u08 interlace;
}
IHDRData;

typedef struct IDATdata
{
    u08 *data;
    u64 offset;
}
IDATdata;

typedef struct cHRMData
{
    u32 whitePointX;
    u32 whitePointY;
    u32 redX;
    u32 redY;
    u32 greenX;
    u32 greenY;
    u32 blueX;
    u32 blueY;
}
cHRMData;

f_internal StringView readChunkHeader
(
    FILE *file,
    u32  *length
){
    *length = 0;
    char *result = calloc(5, 1);

    u08 byte     = 0;
    size elements = 0;

    for(u08 i = 0; i < 4; ++i)
    {
        if((elements = fread(&byte, 1, 1, file)) != 1)
        {
            IM_ERROR("could not read chunk length.");
            goto cleanup;
        }

        *length += ((u32)byte << (3 - i) * 8);
    }

    IM_DEBUG("---------------------------------");
    IM_DEBUG("---------------------------------");
    IM_DEBUG("next chunk length: %u", *length);

    if(*length > I32MAX)
    {
        IM_ERROR("chunk length %u exceeds maximum of %u.", *length, I32MAX);
    }

    for(u08 i = 0; i < 4; ++i)
    {
        if((elements = fread(&byte, 1, 1, file)) != 1)
        {
            IM_ERROR("could not read chunk header at byte %u.", i);
            if(feof(file))
            {
                IM_ERROR("unexpected end of file.");
            }
            else if(ferror(file))
            {
                IM_ERROR("could not read file.");
            }

            goto cleanup;
        }

        result[i] = (char)byte;
    }

    IM_DEBUG("identified chunk: (%s)", result);
    return pdCstrSV(result);

cleanup:
    free(result);
    return(StringView){0};
}

f_internal u08 readChunk_IHDR
(
    FILE     *file,
    IHDRData *data
){
    size elements = 0;
    u08  byte     = 0;

    data->width  = 0;
    data->height = 0;

    for(u08 i = 0; i < 4; ++i)
    {
        if((elements = fread(&byte, 1, 1, file)) != 1)
        {
            IM_ERROR("could not read width from IHDR chunk.");
            return PNG_STREAM_END;
        }

        data->width += ((u32)byte << (3 - i) * 8);
    }

    for(u08 i = 0; i < 4; ++i)
    {
        if((elements = fread(&byte, 1, 1, file)) != 1)
        {
            IM_ERROR("could not read height from IHDR chunk.");
            return PNG_STREAM_END;
        }

        data->height += ((u32)byte << (3 - i) * 8);
    }

    if((elements = fread(&data->bitdepth, 1, 1, file)) != 1)
    {
        IM_ERROR("could not read bitdepth from IHDR chunk.");
        return PNG_STREAM_END;
    }

    if((elements = fread(&data->colorType, 1, 1, file)) != 1)
    {
        IM_ERROR("could not read color type from IHDR chunk.");
        return PNG_STREAM_END;
    }

    if((elements = fread(&data->compression, 1, 1, file)) != 1)
    {
        IM_ERROR("could not read compression method from IHDR chunk.");
        return PNG_STREAM_END;
    }

    if((elements = fread(&data->filter, 1, 1, file)) != 1)
    {
        IM_ERROR("could not read filter method from IHDR chunk.");
        return PNG_STREAM_END;
    }

    if((elements = fread(&data->interlace, 1, 1, file)) != 1)
    {
        IM_ERROR("could not read interlace method from IHDR chunk.");
        return PNG_STREAM_END;
    }

    return PNG_STREAM_CONTINUE;
}

f_internal u08 readChunk_PLTE
(
    FILE *file,
    u32  length,
    RGB8 *palette
){
    if(length > 255)
    {
        IM_ERROR("corrupted PLTE chunk, palette size too big: %u entries.", length);
        return PNG_STREAM_END;
    }
    else if(length % 3 != 0)
    {
        IM_ERROR("corrupted PLTE chunk, palette size not divisible by 3: %u.", length);
        return PNG_STREAM_END;
    }

    size elements = 0;
    for(u32 i = 0; i < length; i += 3)
    {
        if((elements = fread(&palette[i].red, 1, 1, file)) != 1)
        {
            IM_ERROR("could not read red palette value %u from PLTE chunk.", i);
            return PNG_STREAM_END;
        }
        if((elements = fread(&palette[i].green, 1, 1, file)) != 1)
        {
            IM_ERROR("could not read green palette value %u from PLTE chunk.", i);
            return PNG_STREAM_END;
        }
        if((elements = fread(&palette[i].blue, 1, 1, file)) != 1)
        {
            IM_ERROR("could not read blue palette value %u from PLTE chunk.", i);
            return PNG_STREAM_END;
        }
    }

    return PNG_STREAM_CONTINUE;
}

f_internal u08 readChunk_IDAT
(
    FILE     *file,
    u32      length,
    IDATdata *idat
){
    u64 elements = 0;

    if((elements = fread(&idat->data[idat->offset], 1, length, file)) != length)
    {
        IM_ERROR("could not read zlib compressed data from IDAT chunk.");
        return PNG_STREAM_END;
    }

    IM_DEBUG("copied %lu bytes into IDAT offset %lu.", elements, idat->offset);

    idat->offset += length;

    return PNG_STREAM_CONTINUE;
}

f_internal u08 readChunk_cHRM
(
    FILE     *file,
    cHRMData *data
){
    size elements = 0;
    u08  byte     = 0;

    data->whitePointX = 0;
    data->whitePointY = 0;
    data->redX        = 0;
    data->redY        = 0;
    data->greenX      = 0;
    data->greenY      = 0;
    data->blueX       = 0;
    data->blueY       = 0;

    for(u08 i = 0; i < 4; ++i)
    {
        if((elements = fread(&byte, 1, 1, file)) != 1)
        {
            IM_ERROR("could not read white point X from cHRM chunk.");
            return PNG_STREAM_END;
        }

        data->whitePointX += ((u32)byte << (3 - i) * 8);
    }
    for(u08 i = 0; i < 4; ++i)
    {
        if((elements = fread(&byte, 1, 1, file)) != 1)
        {
            IM_ERROR("could not read white point Y from cHRM chunk.");
            return PNG_STREAM_END;
        }

        data->whitePointY += ((u32)byte << (3 - i) * 8);
    }

    for(u08 i = 0; i < 4; ++i)
    {
        if((elements = fread(&byte, 1, 1, file)) != 1)
        {
            IM_ERROR("could not read red X from cHRM chunk.");
            return PNG_STREAM_END;
        }

        data->redX += ((u32)byte << (3 - i) * 8);
    }
    for(u08 i = 0; i < 4; ++i)
    {
        if((elements = fread(&byte, 1, 1, file)) != 1)
        {
            IM_ERROR("could not read red Y from cHRM chunk.");
            return PNG_STREAM_END;
        }

        data->redY += ((u32)byte << (3 - i) * 8);
    }

    for(u08 i = 0; i < 4; ++i)
    {
        if((elements = fread(&byte, 1, 1, file)) != 1)
        {
            IM_ERROR("could not read green X from cHRM chunk.");
            return PNG_STREAM_END;
        }

        data->greenX += ((u32)byte << (3 - i) * 8);
    }
    for(u08 i = 0; i < 4; ++i)
    {
        if((elements = fread(&byte, 1, 1, file)) != 1)
        {
            IM_ERROR("could not read green Y from cHRM chunk.");
            return PNG_STREAM_END;
        }

        data->greenY += ((u32)byte << (3 - i) * 8);
    }

    for(u08 i = 0; i < 4; ++i)
    {
        if((elements = fread(&byte, 1, 1, file)) != 1)
        {
            IM_ERROR("could not read blue X from cHRM chunk.");
            return PNG_STREAM_END;
        }

        data->blueX += ((u32)byte << (3 - i) * 8);
    }
    for(u08 i = 0; i < 4; ++i)
    {
        if((elements = fread(&byte, 1, 1, file)) != 1)
        {
            IM_ERROR("could not read blue Y from cHRM chunk.");
            return PNG_STREAM_END;
        }

        data->blueY += ((u32)byte << (3 - i) * 8);
    }

    return PNG_STREAM_CONTINUE;
}

f_internal u08 readChunk_bKGD
(
    FILE       *file,
    const u08  colorType,
    const RGB8 *palette,
    RGB16      *background
){
    size elements = 0;
    u08  byte     = 0;

    background->red   = 0;
    background->green = 0;
    background->blue  = 0;

    if(colorType == IM_PNG_TYPE_INDEXED)
    {
        if((elements = fread(&byte, 1, 1, file)) != 1)
        {
            IM_ERROR("could not read palette index from bKGD chunk.");
            return PNG_STREAM_END;
        }

        background->red   = palette[byte].red;
        background->green = palette[byte].green;
        background->blue  = palette[byte].blue;

        return PNG_STREAM_CONTINUE;
    }
    else if(colorType == IM_PNG_TYPE_GREYSCALE ||
            colorType == IM_PNG_TYPE_GREYSCALE_ALPHA
    ){
        for(u08 i = 0; i < 2; ++i)
        {
            if((elements = fread(&byte, 1, 1, file)) != 1)
            {
                IM_ERROR("could not read grey from bKGD chunk.");
                return PNG_STREAM_END;
            }

            background->red   += (u16)((u16)byte << (1 - i) * 8);
            background->green += (u16)((u16)byte << (1 - i) * 8);
            background->blue  += (u16)((u16)byte << (1 - i) * 8);
        }

        return PNG_STREAM_CONTINUE;
    }

    for(u08 i = 0; i < 2; ++i)
    {
        if((elements = fread(&byte, 1, 1, file)) != 1)
        {
            IM_ERROR("could not read red from bKGD chunk.");
            return PNG_STREAM_END;
        }

        background->red += ((u16)byte << (1 - i) * 8);
    }
    for(u08 i = 0; i < 2; ++i)
    {
        if((elements = fread(&byte, 1, 1, file)) != 1)
        {
            IM_ERROR("could not read green from bKGD chunk.");
            return PNG_STREAM_END;
        }

        background->green += ((u16)byte << (1 - i) * 8);
    }
    for(u08 i = 0; i < 2; ++i)
    {
        if((elements = fread(&byte, 1, 1, file)) != 1)
        {
            IM_ERROR("could not read blue from bKGD chunk.");
            return PNG_STREAM_END;
        }

        background->blue += ((u16)byte << (1 - i) * 8);
    }

    return PNG_STREAM_CONTINUE;
}

f_internal u08 readChunk_tIME
(
    FILE *file
){
    u64 elements = 0;
    u08 byte     = 0;

    u16 year   = 0;
    u08 month  = 0;
    u08 day    = 0;
    u08 hour   = 0;
    u08 minute = 0;
    u08 second = 0;

    for(u08 i = 0; i < 2; ++i)
    {
        if((elements = fread(&byte, 1, 1, file)) != 1)
        {
            IM_ERROR("could not read year from tIME chunk.");
            return PNG_STREAM_END;
        }

        year += ((u16)byte << (1 - i) * 8);
    }

    if((elements = fread(&byte, 1, 1, file)) != 1)
    {
        IM_ERROR("could not read month from tIME chunk.");
        return PNG_STREAM_END;
    }
    month = byte;

    if((elements = fread(&byte, 1, 1, file)) != 1)
    {
        IM_ERROR("could not read day from tIME chunk.");
        return PNG_STREAM_END;
    }
    day = byte;

    if((elements = fread(&byte, 1, 1, file)) != 1)
    {
        IM_ERROR("could not read hour from tIME chunk.");
        return PNG_STREAM_END;
    }
    hour = byte;

    if((elements = fread(&byte, 1, 1, file)) != 1)
    {
        IM_ERROR("could not read minute from tIME chunk.");
        return PNG_STREAM_END;
    }
    minute = byte;

    if((elements = fread(&byte, 1, 1, file)) != 1)
    {
        IM_ERROR("could not read second from tIME chunk.");
        return PNG_STREAM_END;
    }
    second = byte;

    return PNG_STREAM_CONTINUE;
}

f_internal bool readChunk_tEXt
(
    FILE *file,
    u32  length
){
    u64 elements    = 0;
    u32 byteCounter = 0;
    // keyword, 1 - 79 bytes, one of:
    // title
    // author
    // description
    // copyright
    // creation time
    // software
    // disclaimer
    // warning
    // source
    // comment
    // xml:com.adobe.xmp?
    // collection
    // others...?

    char keywordBuf[80] = {0};
    for(u32 i = 0; i < 79; ++i)
    {
        if((elements = fread(&keywordBuf[i], 1, 1, file)) != 1)
        {
            IM_ERROR("could not read keyword from tEXt chunk.");
            return PNG_STREAM_END;
        }
        ++byteCounter;

        if(keywordBuf[i] == '\0')
        {
            break;
        }
    }

    u32 readLength = length - byteCounter;

    char valueBuf[readLength + 1];
    valueBuf[readLength] = '\0';
    for(u32 i = 0; i < readLength; ++i)
    {
        if((elements = fread(&valueBuf[i], 1, 1, file)) != 1)
        {
            IM_ERROR("could not read keyword data from tEXt chunk.");
            return PNG_STREAM_END;
        }

        if(valueBuf[i] == '\0')
        {
            break;
        }
    }

    StringView keyword = pdCstrSV(keywordBuf);
    StringView value   = pdCstrSV(valueBuf);

    IM_DEBUG("identified keyword: '"PRI_SV"'\n", ARG_SV(keyword));
    IM_DEBUG("value: '"PRI_SV"'\n", ARG_SV(value));

    return PNG_STREAM_CONTINUE;
}

f_internal bool readChunkCRC
(
    FILE *file
){
    // actually verify these bytes and not just skip them
    size elements = 0;
    u08  byte     = 0;
    u32  CRC      = 0;

    for(u08 i = 0; i < 4; ++i)
    {
        if((elements = fread(&byte, 1, 1, file)) != 1)
        {
            IM_ERROR("could not read CRC.");
            return PNG_STREAM_END;
        }

        CRC += ((u32)byte << (3 - i) * 8);
    }

    return PNG_STREAM_CONTINUE;
}

u08* loadPNG
(
    FILE *file,
    u32  *width,
    u32  *height,
    u08  channels
){
    u08 header[8] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };
    *width  = 0;
    *height = 0;

    size elements = 0;
    u08  byte     = 0;

    for(u08 i = 0; i < 8; ++i)
    {
        if((elements = fread(&byte, 1, 1, file)) != 1)
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

        if(byte != header[i])
        {
            IM_ERROR("PNG header at byte %u corrupted. Got: %u, expected: %u",
                     i, (u08)byte, (u08)header[i]);
            return 0;
        }
    }

    u32 length = 0;
    StringView chunkHeader = readChunkHeader(file, &length);
    if(chunkHeader.data && !pdSVSame(chunkHeader, IHDR))
    {
        IM_ERROR("could not read IHDR header at beginning of PNG stream.\n"
                 "Read chunk header: '"PRI_SV"'", ARG_SV(chunkHeader));
        free((void*)chunkHeader.data);
        return 0;
    }

    u08 *imgFiltered = 0;

    IHDRData ihdrData = {0};
    if(!readChunk_IHDR(file, &ihdrData))
    {
        IM_ERROR("could not read IHDR header data.");
        free((void*)chunkHeader.data);
        return 0;
    }

    *width  = ihdrData.width;
    *height = ihdrData.height;

    imgFiltered = malloc(*width * *height * 5);

    IDATdata imgIdat     = {0};
    cHRMData chrmData    = {0};
    RGB8     *palette    = 0;
    RGB16    background  = {0};

    imgIdat.data = malloc(*width * *height * 4);

    readChunkCRC(file);

    IM_DEBUG("width:              %u", ihdrData.width);
    IM_DEBUG("height:             %u", ihdrData.height);
    IM_DEBUG("bitdepth:           %u", ihdrData.bitdepth);
    IM_DEBUG("color type:         %u", ihdrData.colorType);
    IM_DEBUG("filter method:      %u", ihdrData.filter);
    IM_DEBUG("interlace method:   %u", ihdrData.interlace);
    IM_DEBUG("compression method: %u", ihdrData.compression);

    if(ihdrData.compression)
    {
        IM_ERROR("compression method not DEFLATE.");
    }

    bool streamData = true;
    while(streamData)
    {
        if(chunkHeader.data)
        {
            free((void*)chunkHeader.data);
            chunkHeader.size = 0;
        }
        chunkHeader = readChunkHeader(file, &length);

        if(!chunkHeader.data)
        {
            IM_ERROR("could not successfully read chunk header.");
            goto error;
        }
        else if(pdSVSame(chunkHeader, PLTE))
        {
            palette    = malloc(length * sizeof(RGB8));
            streamData = readChunk_PLTE(file, length, palette);
        }
        else if(pdSVSame(chunkHeader, IDAT))
        {
            streamData = readChunk_IDAT(file, length, &imgIdat);
        }
        else if(pdSVSame(chunkHeader, IEND))
        {
            streamData = false;
        }
        else if(pdSVSame(chunkHeader, cHRM))
        {
            streamData = readChunk_cHRM(file, &chrmData);
        }
        else if(pdSVSame(chunkHeader, bKGD))
        {
            streamData = readChunk_bKGD(file, ihdrData.colorType, palette, &background);
        }
        else if(pdSVSame(chunkHeader, tIME))
        {
            streamData = readChunk_tIME(file);
        }
        else if(pdSVSame(chunkHeader, tEXt))
        {
            streamData = readChunk_tEXt(file, length);
        }
        else
        {
            IM_ERROR("chunk type '"PRI_SV"' not implemented.", ARG_SV(chunkHeader));
            goto error;
        }

        readChunkCRC(file);
    }

    DeflateInfo dfInfo = dsReadZlibPtr(imgIdat.data, imgFiltered, *width * *height * 5);
    if(!dfInfo.success)
    {
        IM_ERROR("could not decode zlib compressed image data.");
    }

    // that wasn't actually the image data. this data is still filter-compressed,
    // so I need to figure out what to do with this.

    if(chunkHeader.data)
    {
        free((void*)chunkHeader.data);
        chunkHeader.size = 0;
    }
    if(palette)
    {
        free(palette);
    }
    return imgFiltered;

error:
    if(chunkHeader.data)
    {
        free((void*)chunkHeader.data);
        chunkHeader.size = 0;
    }
    if(palette)
    {
        free(palette);
    }
    free(imgFiltered);
    return 0;
}

bool writePNG
(
    FILE *file,
    u08  *data,
    u32  width,
    u32  height,
    u08  channels
){
    (void)file;
    (void)data;
    (void)width;
    (void)height;
    (void)channels;
    IM_WARN("PNG writer under construction.");
    return 0;
}

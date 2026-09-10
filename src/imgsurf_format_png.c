#include "imgsurf_main.h"
#include "pd_print_macros.h"

#include "string_view.h"
#include "datasurf_main.h"

#define PNG_STREAM_END      0
#define PNG_STREAM_CONTINUE 1

#define IM_PNG_TYPE_GREYSCALE        0
#define IM_PNG_TYPE_TRUECOLOUR       2
#define IM_PNG_TYPE_INDEXED          3
#define IM_PNG_TYPE_GREYSCALE_ALPHA  4
#define IM_PNG_TYPE_TRUECOLOUR_ALPHA 6

typedef struct RGB8
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
}
RGB8;

typedef struct RGB16
{
    uint16_t red;
    uint16_t green;
    uint16_t blue;
}
RGB16;

typedef struct IHDRData
{
    uint32_t width;
    uint32_t height;
    uint8_t  bitdepth;
    uint8_t  colorType;
    uint8_t  compression;
    uint8_t  filter;
    uint8_t  interlace;
}
IHDRData;

typedef struct IDATdata
{
    uint8_t  *data;
    uint64_t offset;
}
IDATdata;

typedef struct cHRMData
{
    uint32_t whitePointX;
    uint32_t whitePointY;
    uint32_t redX;
    uint32_t redY;
    uint32_t greenX;
    uint32_t greenY;
    uint32_t blueX;
    uint32_t blueY;
}
cHRMData;

f_internal StringView readChunkHeader
(
    FILE     *file,
    uint32_t *length
){
    *length = 0;
    char *result = calloc(5, 1);

    uint8_t byte     = 0;
    size_t  elements = 0;

    for(uint8_t i = 0; i < 4; ++i)
    {
        if((elements = fread(&byte, 1, 1, file)) != 1)
        {
            PD_ERROR("could not read chunk length.");
            goto cleanup;
        }

        *length += ((uint32_t)byte << (3 - i) * 8);
    }

    PD_DEBUG("---------------------------------");
    PD_DEBUG("---------------------------------");
    PD_DEBUG("next chunk length: %u", *length);

    if(*length > INT32_MAX)
    {
        PD_ERROR("chunk length %u exceeds maximum of %u.", *length, INT32_MAX);
    }

    for(uint8_t i = 0; i < 4; ++i)
    {
        if((elements = fread(&byte, 1, 1, file)) != 1)
        {
            PD_ERROR("could not read chunk header at byte %u.", i);
            if(feof(file))
            {
                PD_ERROR("unexpected end of file.");
            }
            else if(ferror(file))
            {
                PD_ERROR("could not read file.");
            }

            goto cleanup;
        }

        result[i] = (char)byte;
    }

    PD_DEBUG("identified chunk: (%s)", result);
    return cstr_sv(result);

cleanup:
    free(result);
    return(StringView){0};
}

f_internal uint8_t readChunk_IHDR
(
    FILE     *file,
    IHDRData *data
){
    size_t  elements = 0;
    uint8_t byte     = 0;

    data->width  = 0;
    data->height = 0;

    for(uint8_t i = 0; i < 4; ++i)
    {
        if((elements = fread(&byte, 1, 1, file)) != 1)
        {
            PD_ERROR("could not read width from IHDR chunk.");
            return PNG_STREAM_END;
        }

        data->width += ((uint32_t)byte << (3 - i) * 8);
    }

    for(uint8_t i = 0; i < 4; ++i)
    {
        if((elements = fread(&byte, 1, 1, file)) != 1)
        {
            PD_ERROR("could not read height from IHDR chunk.");
            return PNG_STREAM_END;
        }

        data->height += ((uint32_t)byte << (3 - i) * 8);
    }

    if((elements = fread(&data->bitdepth, 1, 1, file)) != 1)
    {
        PD_ERROR("could not read bitdepth from IHDR chunk.");
        return PNG_STREAM_END;
    }

    if((elements = fread(&data->colorType, 1, 1, file)) != 1)
    {
        PD_ERROR("could not read color type from IHDR chunk.");
        return PNG_STREAM_END;
    }

    if((elements = fread(&data->compression, 1, 1, file)) != 1)
    {
        PD_ERROR("could not read compression method from IHDR chunk.");
        return PNG_STREAM_END;
    }

    if((elements = fread(&data->filter, 1, 1, file)) != 1)
    {
        PD_ERROR("could not read filter method from IHDR chunk.");
        return PNG_STREAM_END;
    }

    if((elements = fread(&data->interlace, 1, 1, file)) != 1)
    {
        PD_ERROR("could not read interlace method from IHDR chunk.");
        return PNG_STREAM_END;
    }

    return PNG_STREAM_CONTINUE;
}

f_internal uint8_t readChunk_PLTE
(
    FILE     *file,
    uint32_t length,
    RGB8     *palette
){
    if(length > 255)
    {
        PD_ERROR("corrupted PLTE chunk, palette size too big: %u entries.", length);
        return PNG_STREAM_END;
    }
    else if(length % 3 != 0)
    {
        PD_ERROR("corrupted PLTE chunk, palette size not divisible by 3: %u.", length);
        return PNG_STREAM_END;
    }

    size_t elements = 0;
    for(uint32_t i = 0; i < length; i += 3)
    {
        if((elements = fread(&palette[i].red, 1, 1, file)) != 1)
        {
            PD_ERROR("could not read red palette value %u from PLTE chunk.", i);
            return PNG_STREAM_END;
        }
        if((elements = fread(&palette[i].green, 1, 1, file)) != 1)
        {
            PD_ERROR("could not read green palette value %u from PLTE chunk.", i);
            return PNG_STREAM_END;
        }
        if((elements = fread(&palette[i].blue, 1, 1, file)) != 1)
        {
            PD_ERROR("could not read blue palette value %u from PLTE chunk.", i);
            return PNG_STREAM_END;
        }
    }

    return PNG_STREAM_CONTINUE;
}

f_internal uint8_t readChunk_IDAT
(
    FILE     *file,
    uint32_t length,
    IDATdata *idat
){
    uint64_t elements = 0;

    if((elements = fread(&idat->data[idat->offset], length, 1, file)) != 1)
    {
        PD_ERROR("could not read zlib compressed data from IDAT chunk.");
        return PNG_STREAM_END;
    }

    idat->offset += length;

    return PNG_STREAM_CONTINUE;
}

f_internal uint8_t readChunk_cHRM
(
    FILE     *file,
    cHRMData *data
){
    size_t  elements = 0;
    uint8_t byte     = 0;

    data->whitePointX = 0;
    data->whitePointY = 0;
    data->redX        = 0;
    data->redY        = 0;
    data->greenX      = 0;
    data->greenY      = 0;
    data->blueX       = 0;
    data->blueY       = 0;

    for(uint8_t i = 0; i < 4; ++i)
    {
        if((elements = fread(&byte, 1, 1, file)) != 1)
        {
            PD_ERROR("could not read white point X from cHRM chunk.");
            return PNG_STREAM_END;
        }

        data->whitePointX += ((uint32_t)byte << (3 - i) * 8);
    }
    for(uint8_t i = 0; i < 4; ++i)
    {
        if((elements = fread(&byte, 1, 1, file)) != 1)
        {
            PD_ERROR("could not read white point Y from cHRM chunk.");
            return PNG_STREAM_END;
        }

        data->whitePointY += ((uint32_t)byte << (3 - i) * 8);
    }

    for(uint8_t i = 0; i < 4; ++i)
    {
        if((elements = fread(&byte, 1, 1, file)) != 1)
        {
            PD_ERROR("could not read red X from cHRM chunk.");
            return PNG_STREAM_END;
        }

        data->redX += ((uint32_t)byte << (3 - i) * 8);
    }
    for(uint8_t i = 0; i < 4; ++i)
    {
        if((elements = fread(&byte, 1, 1, file)) != 1)
        {
            PD_ERROR("could not read red Y from cHRM chunk.");
            return PNG_STREAM_END;
        }

        data->redY += ((uint32_t)byte << (3 - i) * 8);
    }

    for(uint8_t i = 0; i < 4; ++i)
    {
        if((elements = fread(&byte, 1, 1, file)) != 1)
        {
            PD_ERROR("could not read green X from cHRM chunk.");
            return PNG_STREAM_END;
        }

        data->greenX += ((uint32_t)byte << (3 - i) * 8);
    }
    for(uint8_t i = 0; i < 4; ++i)
    {
        if((elements = fread(&byte, 1, 1, file)) != 1)
        {
            PD_ERROR("could not read green Y from cHRM chunk.");
            return PNG_STREAM_END;
        }

        data->greenY += ((uint32_t)byte << (3 - i) * 8);
    }

    for(uint8_t i = 0; i < 4; ++i)
    {
        if((elements = fread(&byte, 1, 1, file)) != 1)
        {
            PD_ERROR("could not read blue X from cHRM chunk.");
            return PNG_STREAM_END;
        }

        data->blueX += ((uint32_t)byte << (3 - i) * 8);
    }
    for(uint8_t i = 0; i < 4; ++i)
    {
        if((elements = fread(&byte, 1, 1, file)) != 1)
        {
            PD_ERROR("could not read blue Y from cHRM chunk.");
            return PNG_STREAM_END;
        }

        data->blueY += ((uint32_t)byte << (3 - i) * 8);
    }

    return PNG_STREAM_CONTINUE;
}

f_internal uint8_t readChunk_bKGD
(
    FILE          *file,
    const uint8_t colorType,
    const RGB8    *palette,
    RGB16         *background
){
    size_t  elements = 0;
    uint8_t byte     = 0;

    background->red   = 0;
    background->green = 0;
    background->blue  = 0;

    if(colorType == IM_PNG_TYPE_INDEXED)
    {
        if((elements = fread(&byte, 1, 1, file)) != 1)
        {
            PD_ERROR("could not read palette index from bKGD chunk.");
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
        for(uint8_t i = 0; i < 2; ++i)
        {
            if((elements = fread(&byte, 1, 1, file)) != 1)
            {
                PD_ERROR("could not read grey from bKGD chunk.");
                return PNG_STREAM_END;
            }

            background->red   += (uint16_t)((uint16_t)byte << (1 - i) * 8);
            background->green += (uint16_t)((uint16_t)byte << (1 - i) * 8);
            background->blue  += (uint16_t)((uint16_t)byte << (1 - i) * 8);
        }

        return PNG_STREAM_CONTINUE;
    }

    for(uint8_t i = 0; i < 2; ++i)
    {
        if((elements = fread(&byte, 1, 1, file)) != 1)
        {
            PD_ERROR("could not read red from bKGD chunk.");
            return PNG_STREAM_END;
        }

        background->red += ((uint16_t)byte << (1 - i) * 8);
    }
    for(uint8_t i = 0; i < 2; ++i)
    {
        if((elements = fread(&byte, 1, 1, file)) != 1)
        {
            PD_ERROR("could not read green from bKGD chunk.");
            return PNG_STREAM_END;
        }

        background->green += ((uint16_t)byte << (1 - i) * 8);
    }
    for(uint8_t i = 0; i < 2; ++i)
    {
        if((elements = fread(&byte, 1, 1, file)) != 1)
        {
            PD_ERROR("could not read blue from bKGD chunk.");
            return PNG_STREAM_END;
        }

        background->blue += ((uint16_t)byte << (1 - i) * 8);
    }

    return PNG_STREAM_CONTINUE;
}

f_internal uint8_t readChunk_tIME
(
    FILE *file
){
    uint64_t elements = 0;
    uint8_t  byte     = 0;

    uint16_t year   = 0;
    uint8_t  month  = 0;
    uint8_t  day    = 0;
    uint8_t  hour   = 0;
    uint8_t  minute = 0;
    uint8_t  second = 0;

    for(uint8_t i = 0; i < 2; ++i)
    {
        if((elements = fread(&byte, 1, 1, file)) != 1)
        {
            PD_ERROR("could not read year from tIME chunk.");
            return PNG_STREAM_END;
        }

        year += ((uint16_t)byte << (1 - i) * 8);
    }

    if((elements = fread(&byte, 1, 1, file)) != 1)
    {
        PD_ERROR("could not read month from tIME chunk.");
        return PNG_STREAM_END;
    }
    month = byte;

    if((elements = fread(&byte, 1, 1, file)) != 1)
    {
        PD_ERROR("could not read day from tIME chunk.");
        return PNG_STREAM_END;
    }
    day = byte;

    if((elements = fread(&byte, 1, 1, file)) != 1)
    {
        PD_ERROR("could not read hour from tIME chunk.");
        return PNG_STREAM_END;
    }
    hour = byte;

    if((elements = fread(&byte, 1, 1, file)) != 1)
    {
        PD_ERROR("could not read minute from tIME chunk.");
        return PNG_STREAM_END;
    }
    minute = byte;

    if((elements = fread(&byte, 1, 1, file)) != 1)
    {
        PD_ERROR("could not read second from tIME chunk.");
        return PNG_STREAM_END;
    }
    second = byte;

    return PNG_STREAM_CONTINUE;
}

f_internal bool readChunk_tEXt
(
    FILE     *file,
    uint32_t length
){
    uint64_t elements    = 0;
    uint32_t byteCounter = 0;
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
    for(uint32_t i = 0; i < 79; ++i)
    {
        if((elements = fread(&keywordBuf[i], 1, 1, file)) != 1)
        {
            PD_ERROR("could not read keyword from tEXt chunk.");
            return PNG_STREAM_END;
        }
        ++byteCounter;

        if(keywordBuf[i] == '\0')
        {
            break;
        }
    }

    uint32_t readLength = length - byteCounter;

    char valueBuf[readLength + 1];
    valueBuf[readLength] = '\0';
    for(uint32_t i = 0; i < readLength; ++i)
    {
        if((elements = fread(&valueBuf[i], 1, 1, file)) != 1)
        {
            PD_ERROR("could not read keyword data from tEXt chunk.");
            return PNG_STREAM_END;
        }

        if(valueBuf[i] == '\0')
        {
            break;
        }
    }

    StringView keyword = cstr_sv(keywordBuf);
    StringView value   = cstr_sv(valueBuf);

    PD_DEBUG("identified keyword: '"PRI_SV"'\n", ARG_SV(keyword));
    PD_DEBUG("value: '"PRI_SV"'\n", ARG_SV(value));

    return PNG_STREAM_CONTINUE;
}

f_internal bool readChunkCRC
(
    FILE *file
){
    // actually verify these bytes and not just skip them
    size_t   elements = 0;
    uint8_t  byte     = 0;
    uint32_t CRC      = 0;

    for(uint8_t i = 0; i < 4; ++i)
    {
        if((elements = fread(&byte, 1, 1, file)) != 1)
        {
            PD_ERROR("could not read CRC.");
            return PNG_STREAM_END;
        }

        CRC += ((uint32_t)byte << (3 - i) * 8);
    }

    return PNG_STREAM_CONTINUE;
}

uint8_t* loadPNG
(
    FILE     *file,
    uint32_t *width,
    uint32_t *height,
    uint8_t  channels
){
    uint8_t header[8] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };
    *width  = 0;
    *height = 0;

    size_t  elements = 0;
    uint8_t byte     = 0;

    for(uint8_t i = 0; i < 8; ++i)
    {
        if((elements = fread(&byte, 1, 1, file)) != 1)
        {
            PD_ERROR("could not read header at byte %u.", i);
            if(feof(file))
            {
                PD_ERROR("unexpected end of file.");
            }
            else if(ferror(file))
            {
                PD_ERROR("could not read file.");
            }

            return 0;
        }

        if(byte != header[i])
        {
            PD_ERROR("PNG header at byte %u corrupted. Got: %u, expected: %u",
                     i, (uint8_t)byte, (uint8_t)header[i]);
            return 0;
        }
    }

    StringView IHDR = cstr_sv("IHDR");
    StringView PLTE = cstr_sv("PLTE");
    StringView IDAT = cstr_sv("IDAT");
    StringView IEND = cstr_sv("IEND");
    StringView cHRM = cstr_sv("cHRM");
    StringView bKGD = cstr_sv("bKGD");
    StringView tIME = cstr_sv("tIME");
    StringView tEXt = cstr_sv("tEXt");

    uint32_t   length      = 0;
    StringView chunkHeader = readChunkHeader(file, &length);
    if(chunkHeader.data && !sv_same(chunkHeader, IHDR))
    {
        PD_ERROR("could not read IHDR header at beginning of PNG stream.\n"
                 "Read chunk header: '"PRI_SV"'", ARG_SV(chunkHeader));
        free((void*)chunkHeader.data);
        return 0;
    }

    uint8_t *img = 0;

    IHDRData ihdrData = {0};
    if(!readChunk_IHDR(file, &ihdrData))
    {
        PD_ERROR("could not read IHDR header data.");
        free((void*)chunkHeader.data);
        return 0;
    }

    *width  = ihdrData.width;
    *height = ihdrData.height;

    img = malloc(*width * *height * 4);

    IDATdata imgIdat     = {0};
    cHRMData chrmData    = {0};
    RGB8     *palette    = 0;
    RGB16    background  = {0};

    imgIdat.data = malloc(*width * *height * 4);

    readChunkCRC(file);

    PD_DEBUG("width:              %u", ihdrData.width);
    PD_DEBUG("height:             %u", ihdrData.height);
    PD_DEBUG("bitdepth:           %u", ihdrData.bitdepth);
    PD_DEBUG("color type:         %u", ihdrData.colorType);
    PD_DEBUG("filter method:      %u", ihdrData.filter);
    PD_DEBUG("interlace method:   %u", ihdrData.interlace);
    PD_DEBUG("compression method: %u", ihdrData.compression);

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
            PD_ERROR("could not successfully read chunk header.");
            goto error;
        }
        else if(sv_same(chunkHeader, PLTE))
        {
            palette    = malloc(length * sizeof(RGB8));
            streamData = readChunk_PLTE(file, length, palette);
        }
        else if(sv_same(chunkHeader, IDAT))
        {
            streamData = readChunk_IDAT(file, length, &imgIdat);
        }
        else if(sv_same(chunkHeader, IEND))
        {
            streamData = false;
        }
        else if(sv_same(chunkHeader, cHRM))
        {
            streamData = readChunk_cHRM(file, &chrmData);
        }
        else if(sv_same(chunkHeader, bKGD))
        {
            streamData = readChunk_bKGD(file, ihdrData.colorType, palette, &background);
        }
        else if(sv_same(chunkHeader, tIME))
        {
            streamData = readChunk_tIME(file);
        }
        else if(sv_same(chunkHeader, tEXt))
        {
            streamData = readChunk_tEXt(file, length);
        }
        else
        {
            PD_ERROR("chunk type '"PRI_SV"' not implemented.", ARG_SV(chunkHeader));
            goto error;
        }

        readChunkCRC(file);
    }

    if(!dsReadZlibPtr(imgIdat.data, img))
    {
        PD_ERROR("could not decode zlib compressed image data.");
    }

    if(chunkHeader.data)
    {
        free((void*)chunkHeader.data);
        chunkHeader.size = 0;
    }
    if(palette)
    {
        free(palette);
    }
    return img;

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
    free(img);
    return 0;
}

bool writePNG
(
    FILE     *file,
    uint8_t  *data,
    uint32_t width,
    uint32_t height,
    uint8_t  channels
){
    (void)file;
    (void)data;
    (void)width;
    (void)height;
    (void)channels;
    PD_WARN("PNG writer under construction.");
    return 0;
}

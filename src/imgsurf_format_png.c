#include "imgsurf_main.h"

#include "string_view.h"

#define PNG_STREAM_END      0
#define PNG_STREAM_CONTINUE 1

#define IS_PNG_TYPE_GREYSCALE        0
#define IS_PNG_TYPE_TRUECOLOUR       2
#define IS_PNG_TYPE_INDEXED          3
#define IS_PNG_TYPE_GREYSCALE_ALPHA  4
#define IS_PNG_TYPE_TRUECOLOUR_ALPHA 6

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

    char   byte     = 0;
    size_t elements = 0;

    for(uint8_t i = 0; i < 4; ++i)
    {
        if((elements = fread(&byte, 1, 1, file)) != 1)
        {
            fprintf(stderr, "\n\033[31;1;7mERROR: could not read chunk length."
                    "\033[0m\n");
            free(result);
            return(StringView){0};
        }

        *length += ((uint32_t)byte << (3 - i) * 8);
    }

    #ifdef DEBUG
    fprintf(stderr, "---------------------------------\n");
    fprintf(stderr, "next chunk length: %u\n", *length);
    #endif
    uint32_t max = (uint32_t)((1 << 31)) - 1;
    if(*length > max)
    {
        fprintf(stderr, "\n\033[31;1;7mERROR: chunk length %u exceeds maximum of %u."
                "\033[0m\n", *length, max);
        free(result);
        return (StringView){0};
    }

    for(uint8_t i = 0; i < 4; ++i)
    {
        if((elements = fread(&byte, 1, 1, file)) != 1)
        {
            fprintf(stderr, "\n\033[31;1;7mERROR: could not read chunk header at byte "
                    "%u.\033[0m\n", i);
            if(feof(file))
            {
                fprintf(stderr, "\n\033[31;1;7munexpected end of file.\033[0m\n");
            }
            else if(ferror(file))
            {
                fprintf(stderr, "\n\033[31;1;7mcould not read file.\033[0m\n");
            }

            return(StringView){0};
        }

        result[i] = byte;
    }

    #ifdef DEBUG
    fprintf(stderr, "identified chunk: %s\n", result);
    fprintf(stderr, "---------------------------------\n");
    #endif
    return cstr_sv(result);
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
            fprintf(stderr, "\n\033[31;1;7mERROR: could not read width from IHDR chunk."
                    "\033[0m\n");
            return PNG_STREAM_END;
        }

        data->width += ((uint32_t)byte << (3 - i) * 8);
    }

    for(uint8_t i = 0; i < 4; ++i)
    {
        if((elements = fread(&byte, 1, 1, file)) != 1)
        {
            fprintf(stderr, "\n\033[31;1;7mERROR: could not read height from IHDR "
                    "chunk.\033[0m\n");
            return PNG_STREAM_END;
        }

        data->height += ((uint32_t)byte << (3 - i) * 8);
    }

    if((elements = fread(&data->bitdepth, 1, 1, file)) != 1)
    {
        fprintf(stderr, "\n\033[31;1;7mERROR: could not read bitdepth from IHDR chunk."
                "\033[0m\n");
        return PNG_STREAM_END;
    }

    if((elements = fread(&data->colorType, 1, 1, file)) != 1)
    {
        fprintf(stderr, "\n\033[31;1;7mERROR: could not read color type from IHDR "
                "chunk.\033[0m\n");
        return PNG_STREAM_END;
    }

    if((elements = fread(&data->compression, 1, 1, file)) != 1)
    {
        fprintf(stderr, "\n\033[31;1;7mERROR: could not read compression method from "
                "IHDR chunk.\033[0m\n");
        return PNG_STREAM_END;
    }

    if((elements = fread(&data->filter, 1, 1, file)) != 1)
    {
        fprintf(stderr, "\n\033[31;1;7mERROR: could not read filter method from IHDR "
                "chunk.\033[0m\n");
        return PNG_STREAM_END;
    }

    if((elements = fread(&data->interlace, 1, 1, file)) != 1)
    {
        fprintf(stderr, "\n\033[31;1;7mERROR: could not read interlace method from "
                "IHDR chunk.\033[0m\n");
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
        fprintf(stderr, "\n\033[31;1;7mERROR: corrupted PLTE chunk, palette size too "
                "big: %u entries.\033[0m\n", length);
        return PNG_STREAM_END;
    }
    else if(length % 3 != 0)
    {
        fprintf(stderr, "\n\033[31;1;7mERROR: corrupted PLTE chunk, palette size not "
                "divisible by 3: %u.\033[0m\n", length);
        return PNG_STREAM_END;
    }

    size_t elements = 0;
    for(uint32_t i = 0; i < length; i += 3)
    {
        if((elements = fread(&palette[i].red, 1, 1, file)) != 1)
        {
            fprintf(stderr, "\n\033[31;1;7mERROR: could not read red palette value %u "
                    "from PLTE chunk.\033[0m\n", i);
            return PNG_STREAM_END;
        }
        if((elements = fread(&palette[i].green, 1, 1, file)) != 1)
        {
            fprintf(stderr, "\n\033[31;1;7mERROR: could not read green palette value %u "
                    "from PLTE chunk.\033[0m\n", i);
            return PNG_STREAM_END;
        }
        if((elements = fread(&palette[i].blue, 1, 1, file)) != 1)
        {
            fprintf(stderr, "\n\033[31;1;7mERROR: could not read blue palette value %u "
                    "from PLTE chunk.\033[0m\n", i);
            return PNG_STREAM_END;
        }
    }

    return PNG_STREAM_CONTINUE;
}

f_internal uint8_t readChunk_IDAT
(
    FILE *file
){
    //
    fprintf(stderr, "\n\033[31;1;7mERROR: readChunk_IDAT not implemented.\033[0m\n");
    return PNG_STREAM_END;
    //

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
            fprintf(stderr, "\n\033[31;1;7mERROR: could not read white point X from "
                    "cHRM chunk.\033[0m\n");
            return PNG_STREAM_END;
        }

        data->whitePointX += ((uint32_t)byte << (3 - i) * 8);
    }
    for(uint8_t i = 0; i < 4; ++i)
    {
        if((elements = fread(&byte, 1, 1, file)) != 1)
        {
            fprintf(stderr, "\n\033[31;1;7mERROR: could not read white point Y from "
                    "cHRM chunk.\033[0m\n");
            return PNG_STREAM_END;
        }

        data->whitePointY += ((uint32_t)byte << (3 - i) * 8);
    }

    for(uint8_t i = 0; i < 4; ++i)
    {
        if((elements = fread(&byte, 1, 1, file)) != 1)
        {
            fprintf(stderr, "\n\033[31;1;7mERROR: could not read red X from cHRM "
                    "chunk.\033[0m\n");
            return PNG_STREAM_END;
        }

        data->redX += ((uint32_t)byte << (3 - i) * 8);
    }
    for(uint8_t i = 0; i < 4; ++i)
    {
        if((elements = fread(&byte, 1, 1, file)) != 1)
        {
            fprintf(stderr, "\n\033[31;1;7mERROR: could not read red Y from cHRM "
                    "chunk.\033[0m\n");
            return PNG_STREAM_END;
        }

        data->redY += ((uint32_t)byte << (3 - i) * 8);
    }

    for(uint8_t i = 0; i < 4; ++i)
    {
        if((elements = fread(&byte, 1, 1, file)) != 1)
        {
            fprintf(stderr, "\n\033[31;1;7mERROR: could not read green X from cHRM "
                    "chunk.\033[0m\n");
            return PNG_STREAM_END;
        }

        data->greenX += ((uint32_t)byte << (3 - i) * 8);
    }
    for(uint8_t i = 0; i < 4; ++i)
    {
        if((elements = fread(&byte, 1, 1, file)) != 1)
        {
            fprintf(stderr, "\n\033[31;1;7mERROR: could not read green Y from cHRM "
                    "chunk.\033[0m\n");
            return PNG_STREAM_END;
        }

        data->greenY += ((uint32_t)byte << (3 - i) * 8);
    }

    for(uint8_t i = 0; i < 4; ++i)
    {
        if((elements = fread(&byte, 1, 1, file)) != 1)
        {
            fprintf(stderr, "\n\033[31;1;7mERROR: could not read blue X from cHRM "
                    "chunk.\033[0m\n");
            return PNG_STREAM_END;
        }

        data->blueX += ((uint32_t)byte << (3 - i) * 8);
    }
    for(uint8_t i = 0; i < 4; ++i)
    {
        if((elements = fread(&byte, 1, 1, file)) != 1)
        {
            fprintf(stderr, "\n\033[31;1;7mERROR: could not read blue Y from cHRM "
                    "chunk.\033[0m\n");
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

    if(colorType == IS_PNG_TYPE_INDEXED)
    {
        if((elements = fread(&byte, 1, 1, file)) != 1)
        {
            fprintf(stderr, "\n\033[31;1;7mERROR: could not read palette index from "
                    "bKGD chunk.\033[0m\n");
            return PNG_STREAM_END;
        }

        background->red   = palette[byte].red;
        background->green = palette[byte].green;
        background->blue  = palette[byte].blue;

        return PNG_STREAM_CONTINUE;
    }
    else if(colorType == IS_PNG_TYPE_GREYSCALE ||
            colorType == IS_PNG_TYPE_GREYSCALE_ALPHA
    ){
        for(uint8_t i = 0; i < 2; ++i)
        {
            if((elements = fread(&byte, 1, 1, file)) != 1)
            {
                fprintf(stderr, "\n\033[31;1;7mERROR: could not read grey from bKGD "
                        "chunk.\033[0m\n");
                return PNG_STREAM_END;
            }

            background->red   += ((uint16_t)byte << (3 - i) * 8);
            background->green += ((uint16_t)byte << (3 - i) * 8);
            background->blue  += ((uint16_t)byte << (3 - i) * 8);
        }

        return PNG_STREAM_CONTINUE;
    }

    for(uint8_t i = 0; i < 2; ++i)
    {
        if((elements = fread(&byte, 1, 1, file)) != 1)
        {
            fprintf(stderr, "\n\033[31;1;7mERROR: could not read red from bKGD "
                    "chunk.\033[0m\n");
            return PNG_STREAM_END;
        }

        background->red += ((uint16_t)byte << (3 - i) * 8);
    }
    for(uint8_t i = 0; i < 2; ++i)
    {
        if((elements = fread(&byte, 1, 1, file)) != 1)
        {
            fprintf(stderr, "\n\033[31;1;7mERROR: could not read green from bKGD "
                    "chunk.\033[0m\n");
            return PNG_STREAM_END;
        }

        background->green += ((uint16_t)byte << (3 - i) * 8);
    }
    for(uint8_t i = 0; i < 2; ++i)
    {
        if((elements = fread(&byte, 1, 1, file)) != 1)
        {
            fprintf(stderr, "\n\033[31;1;7mERROR: could not read blue from bKGD "
                    "chunk.\033[0m\n");
            return PNG_STREAM_END;
        }

        background->blue += ((uint16_t)byte << (3 - i) * 8);
    }

    return PNG_STREAM_CONTINUE;
}

f_internal uint8_t readChunk_tIME
(
    FILE *file
){
    //
    fprintf(stderr, "\n\033[31;1;7mERROR: readChunk_tIME not implemented.\033[0m\n");
    return PNG_STREAM_END;
    //

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
            fprintf(stderr, "\n\033[31;1;7mERROR: could not read CRC.\033[0m\n");
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
    char header[8] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };
    *width  = 0;
    *height = 0;

    size_t elements = 0;
    char   byte     = 0;

    for(uint8_t i = 0; i < 8; ++i)
    {
        if((elements = fread(&byte, 1, 1, file)) != 1)
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

        if(byte != header[i])
        {
            fprintf(stderr, "\n\033[31;1;7mERROR: PNG header at byte %u corrupted."
                    "\033[0m\n", i);
            fprintf(stderr, "got: %u\n", (uint8_t)byte);
            fprintf(stderr, "expected: %u\n", (uint8_t)header[i]);
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

    uint32_t   length      = 0;
    StringView chunkHeader = readChunkHeader(file, &length);
    if(chunkHeader.data && !sv_same(chunkHeader, IHDR))
    {
        fprintf(stderr, "\n\033[31;1mERROR: could not read IHDR header at beginning "
                "of PNG stream. Read chunk header: "PRI_SV"\033[0m\n",
                ARG_SV(chunkHeader));
        free((void*)chunkHeader.data);
        return 0;
    }

    IHDRData ihdrData = {0};
    if(!readChunk_IHDR(file, &ihdrData))
    {
        fprintf(stderr, "\n\033[31;1;1mERROR: could not read IHDR header data."
                "\033[0m\n");
        free((void*)chunkHeader.data);
        return 0;
    }

    cHRMData chrmData = {0};
    // other chunk data structs
    RGB8  *palette   = 0;
    RGB16 background = {0};

    readChunkCRC(file);

    #ifdef DEBUG
    fprintf(stderr, "width: %u\n", ihdrData.width);
    fprintf(stderr, "height: %u\n", ihdrData.height);
    fprintf(stderr, "bitdepth: %u\n", ihdrData.bitdepth);
    fprintf(stderr, "color type: %u\n", ihdrData.colorType);
    fprintf(stderr, "compression method: %u\n", ihdrData.compression);
    fprintf(stderr, "filter method: %u\n", ihdrData.filter);
    fprintf(stderr, "interlace method: %u\n", ihdrData.interlace);
    #endif

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
            fprintf(stderr, "\033[31;1;1mERROR: could not successfully read chunk "
                    "header.\033[0m");
            goto cleanup;
        }
        else if(sv_same(chunkHeader, PLTE))
        {
            palette = malloc(length * sizeof(RGB8));
            streamData = readChunk_PLTE(file, length, palette);
        }
        else if(sv_same(chunkHeader, IDAT))
        {
            streamData = readChunk_IDAT(file);
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
        else
        {
            fprintf(stderr, "\033[31;1;1mERROR: chunk type '"PRI_SV"' not implemented."
                    "\033[0m", ARG_SV(chunkHeader));
            goto cleanup;
        }

        readChunkCRC(file);
    }

cleanup:
    if(chunkHeader.data)
    {
        free((void*)chunkHeader.data);
    }
    if(palette)
    {
        free(palette);
    }
    return 0;
}

uint8_t writePNG
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
    fprintf(stderr, "\n\033[33;1;7mWIP: PNG writer under construction!\033[0m\n");
    return 0;
}

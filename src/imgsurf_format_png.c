#include "imgsurf_main.h"

#include "string_view.h"

#define PNG_STREAM_END      0
#define PNG_STREAM_CONTINUE 1

f_internal StringView readChunkHeader
(
    FILE     *file,
    uint32_t *length
){
    *length = 0;
    char *result = calloc(5, 1);

    char   byte     = 0;
    size_t elements = 0;
    if((elements = fread(length, 4, 1, file)) != 1)
    {
        fprintf(stderr, "\n\033[31;1;7mERROR: could not read chunk length.\033[0m\n");
        free(result);
        return(StringView){0};
    }

    fprintf(stderr, "read length: %u\n", *length);
    uint32_t max = (uint32_t)((1 << 31)) - 1;
    if(*length > max)
    {
        fprintf(stderr, "\n\033[31;1;7mERROR: chunk length %u exceedes maximum of %u."
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

        fprintf(stderr, "byte: %u\n", byte);
        result[i] = byte;
    }

    return cstr_sv(result);
}

f_internal uint8_t readChunk_IHDR
(
    FILE     *file,
    uint32_t length
){
    return PNG_STREAM_END;
}

f_internal uint8_t readChunk_PLTE
(
    FILE     *file,
    uint32_t length
){
    return PNG_STREAM_END;
}

f_internal uint8_t readChunk_IDAT
(
    FILE     *file,
    uint32_t length
){
    return PNG_STREAM_END;
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

    if(!readChunk_IHDR(file, length))
    {
        fprintf(stderr, "\n\033[31;1;1mERROR: could not read IHDR header data."
                "\033[0m\n");
        free((void*)chunkHeader.data);
        return 0;
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
            fprintf(stderr, "\033[31;1;1mERROR: could not successfully read chunk "
                    "header.\033[0m");
            return 0;
        }
        else if(sv_same(chunkHeader, PLTE))
        {
            streamData = readChunk_PLTE(file, length);
        }
        else if(sv_same(chunkHeader, IDAT))
        {
            streamData = readChunk_IDAT(file, length);
        }
        else if(sv_same(chunkHeader, IEND))
        {
            streamData = false;
        }
        else
        {
            fprintf(stderr, "\033[31;1;1mERROR: unknown chunk type: "PRI_SV"\033[0m",
                    ARG_SV(chunkHeader));
            return 0;
        }
    }

    fprintf(stderr, "\n\033[33;1;1mWIP: PNG loader under construction!\033[0m\n");
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

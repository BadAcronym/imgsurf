#include "imgsurf_main.h"

#define STRING_VIEW_IMPL
#include "string_view.h"

f_internal StringView readChunkHeader
(
    FILE *file
){
    char result[4] = {0, 0, 0, 0};

    char   byte     = 0;
    size_t elements = 0;
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

    return(StringView)
    {
        .data = result,
        .size = 4
    };
}

f_internal uint8_t readChunk_IHDR
(
    FILE *file
){
    return 0;
}

f_internal uint8_t readChunk_PLTE
(
    FILE *file
){
    return 0;
}

f_internal uint8_t readChunk_IDAT
(
    FILE *file
){
    return 0;
}

uint8_t* loadPNG
(
    FILE        *file,
    uint32_t    *width,
    uint32_t    *height,
    uint8_t     channels
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

    StringView chunkHeader = readChunkHeader(file);
    if(!sv_same(chunkHeader, IHDR))
    {
        fprintf(stderr, "\n\033[31;1;7mERROR: could not read IHDR header at beginning "
                "of PNG stream.\033[0m\n");
        return 0;
    }

    readChunk_IHDR(file);

    bool streamData = true;
    while(streamData)
    {
        chunkHeader = readChunkHeader(file);
        if(!chunkHeader.data)
        {
            fprintf(stderr, "\033[31;1;7mERROR: could not successfully read chunk "
                    "header.\033[0m");
            return 0;
        }
        else if(sv_same(chunkHeader, PLTE))
        {
            readChunk_PLTE(file);
        }
        else if(sv_same(chunkHeader, IDAT))
        {
            readChunk_IDAT(file);
        }
        else if(sv_same(chunkHeader, IEND))
        {
            streamData = false;
        }
    }

    fprintf(stderr, "\n\033[33;1;7mWIP: PNG loader under construction!\033[0m\n");
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

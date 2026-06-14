#include "imgsurf_main.h"

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

    size_t elements  = 0;
    char   headerByte = 0;

    for(uint8_t i = 0; i < 8; ++i)
    {
        if((elements = fread(&headerByte, 1, 1, file)) != 1)
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

        if(headerByte != header[i])
        {
            fprintf(stderr, "\n\033[31;1;7mERROR: QOI header at byte %u corrupted."
                    "\033[0m\n", i);
            fprintf(stderr, "got: %u\n", (uint8_t)headerByte);
            fprintf(stderr, "expected: %u\n", (uint8_t)header[i]);
            return 0;
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
    fprintf(stderr, "\n\033[33;1;7mWIP: PNG writer under construction!\033[0m\n");
    return 0;
}

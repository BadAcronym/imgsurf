#include "imgsurf_main.h"
#include "pd_print_macros.h"

uint8_t* loadBMP
(
    FILE        *file,
    uint32_t    *width,
    uint32_t    *height,
    uint8_t     channels
){
    PD_WARN("BMP loading not implemented yet.");
    return 0;
}

bool writeBMP
(
    FILE     *file,
    uint8_t  *data,
    uint32_t width,
    uint32_t height,
    uint8_t  channels
){
    PD_WARN("BMP writing not implemented yet.");
    return 0;
}

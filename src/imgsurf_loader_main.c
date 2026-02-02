#include "imgsurf_loader.h"

int main()
{
}

internal void loadPNG
(
){
}

internal void findFormat
(
    const char* path,
    uint8_t     *format
){
    //write back found format
}

uint8_t* imgsurf_load
(
    const char* path,
    uint8_t     channels,
    uint8_t     bitdepth
){
    uint8_t format = 0;
    findFormat(path, &format);

    if(!format)
    {
        fprintf(stderr, "File format is not supported. Try a .png/.bmp/.webp/.avif/.qoi/.jxl file.\n");
    }

    if(channels < IMGSURF_CHANNELS_MIN || channels > IMGSURF_CHANNELS_MIN)
    {
        fprintf(stderr, "Invalid colour channels specified.\n");
    }

    if(bitdepth == 0)
    {
        fprintf(stderr, "Bit depth cannot be null.\n");
    }

    if(format == IMGSURF_FILE_PNG && bitdepth > 2 && bitdepth != 4 && bitdepth != 8 && bitdepth != 16)
    {
        fprintf(stderr, "Only bit depths of 1, 2, 4, 8 or 16 are supported by .png!\n");
    }

    //delegate loading based on parsed format
}

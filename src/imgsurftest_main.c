#include "imgsurf_loader.h"

int main()
{
    uint8_t* png = imgsurf_load("assets/test.png", IMGSURF_CHANNELS_RGBA, 8);

    if(!png)
    {
        fprintf(stderr, "imgsurf_load failed to load a png.\n");
    }

    printf("R: %u", png[0]);
    printf("G: %u", png[0]);
    printf("B: %u", png[0]);
    printf("A: %u", png[0]);

    uint8_t* bmp = imgsurf_load("assets/test.bmp", IMGSURF_CHANNELS_RGB, 8);

    if(!bmp)
    {
        fprintf(stderr, "imgsurf_load failed to load a bmp.\n");
    }

    printf("R: %u", bmp[0]);
    printf("G: %u", bmp[0]);
    printf("B: %u", bmp[0]);
}

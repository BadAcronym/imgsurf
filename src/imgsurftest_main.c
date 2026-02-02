#include "imgsurf_load.h"

#include <stdio.h>

int main()
{
    uint32_t width  = 0;
    uint32_t height = 0;

    uint8_t* png = imgsurf_load("assets/tux.png", &width, &height, IMGSURF_CHANNELS_RGBA, 8);
    if(!png)
    {
        fprintf(stderr, "imgsurf_load failed to load a png.\n");
    }
    else
    {
        printf("PNG loaded!\n");
        printf("R: %u", png[0]);
        printf("G: %u", png[0]);
        printf("B: %u", png[0]);
        printf("A: %u", png[0]);
    }

    uint8_t* bmp = imgsurf_load("assets/tux.bmp", &width, &height, IMGSURF_CHANNELS_RGB, 8);
    if(!bmp)
    {
        fprintf(stderr, "imgsurf_load failed to load a bmp.\n");
    }
    else
    {
        printf("BMP loaded!\n");
        printf("R: %u", bmp[0]);
        printf("G: %u", bmp[0]);
        printf("B: %u", bmp[0]);
    }

    uint8_t* qoiRGBA = imgsurf_load("assets/tux.qoi", &width, &height, IMGSURF_CHANNELS_RGBA, 8);
    if(!qoiRGBA)
    {
        fprintf(stderr, "imgsurf_load failed to load a qoi in RGBA.\n");
    }
    else
    {
        printf("QOI loaded!\n");
        printf("R: %u", qoiRGBA[0]);
        printf("G: %u", qoiRGBA[0]);
        printf("B: %u", qoiRGBA[0]);
    }

    uint8_t* qoiBGRA = imgsurf_load("assets/tux.qoi", &width, &height, IMGSURF_CHANNELS_BGRA, 8);
    if(!qoiBGRA)
    {
        fprintf(stderr, "imgsurf_load failed to load a qoi in RGBA.\n");
    }
    else
    {
        printf("QOI loaded!\n");
        printf("R: %u", qoiBGRA[0]);
        printf("G: %u", qoiBGRA[0]);
        printf("B: %u", qoiBGRA[0]);
    }
}

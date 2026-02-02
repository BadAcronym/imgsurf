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
        printf("\nPNG loaded!\n");
        printf("R:%u ", png[0]);
        printf("G:%u ", png[1]);
        printf("B:%u ", png[2]);
        printf("A:%u\n", png[3]);
    }

    uint8_t* bmp = imgsurf_load("assets/tux.bmp", &width, &height, IMGSURF_CHANNELS_RGB, 8);
    if(!bmp)
    {
        fprintf(stderr, "imgsurf_load failed to load a bmp.\n");
    }
    else
    {
        printf("\nBMP loaded!\n");
        printf("R:%u ", bmp[0]);
        printf("G:%u ", bmp[1]);
        printf("B:%u\n", bmp[2]);
    }

    uint8_t* qoiRGBA = imgsurf_load("assets/tux.qoi", &width, &height, IMGSURF_CHANNELS_RGBA, 8);
    if(!qoiRGBA)
    {
        fprintf(stderr, "imgsurf_load failed to load a qoi in RGBA.\n");
    }
    else
    {
        printf("\nQOI in RGBA loaded!\n");
        printf("R:%u ", qoiRGBA[0]);
        printf("G:%u ", qoiRGBA[1]);
        printf("B:%u ", qoiRGBA[2]);
        printf("A:%u\n", qoiRGBA[3]);
    }

    uint8_t* qoiBGRA = imgsurf_load("assets/tux.qoi", &width, &height, IMGSURF_CHANNELS_BGRA, 8);
    if(!qoiBGRA)
    {
        fprintf(stderr, "imgsurf_load failed to load a qoi in RGBA.\n");
    }
    else
    {
        printf("\nQOI in BGRA loaded!\n");
        printf("B:%u ", qoiBGRA[0]);
        printf("G:%u ", qoiBGRA[1]);
        printf("R:%u ", qoiBGRA[2]);
        printf("A:%u\n", qoiBGRA[3]);
    }

    uint8_t* qoiRGB = imgsurf_load("assets/tux.qoi", &width, &height, IMGSURF_CHANNELS_RGB, 8);
    if(!qoiRGB)
    {
        fprintf(stderr, "imgsurf_load failed to load a qoi in RGBA.\n");
    }
    else
    {
        printf("\nQOI in RGB loaded!\n");
        printf("R:%u ", qoiBGRA[0]);
        printf("G:%u ", qoiBGRA[1]);
        printf("B:%u\n", qoiBGRA[2]);
    }

    uint8_t* qoiBGR = imgsurf_load("assets/tux.qoi", &width, &height, IMGSURF_CHANNELS_BGR, 8);
    if(!qoiBGR)
    {
        fprintf(stderr, "imgsurf_load failed to load a qoi in RGBA.\n");
    }
    else
    {
        printf("\nQOI in BGR loaded!\n");
        printf("B:%u ", qoiBGRA[0]);
        printf("G:%u ", qoiBGRA[1]);
        printf("R:%u\n", qoiBGRA[2]);
    }
}

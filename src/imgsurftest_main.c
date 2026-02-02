#include "imgsurf_load.h"

#include <stdio.h>
#include <stdlib.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

int main()
{
    uint32_t width  = 0;
    uint32_t height = 0;

    int stb_width  = 0;
    int stb_height = 0;
    int stb_channels = 0;

    uint8_t* stbTest = stbi_load("assets/tux.png", &stb_width, &stb_height, &stb_channels, STBI_rgb_alpha);
    if(!stbTest)
    {
        fprintf(stderr, "\x1b[1;31mstb_image failed to load the image!\033[0m\n");
        return -1;
    }

    uint8_t* qoiRGBA = imgsurf_load("assets/tux.qoi", &width, &height, IMGSURF_CHANNELS_RGBA, 8);
    if(!qoiRGBA)
    {
        fprintf(stderr, "imgsurf_load failed to load a qoi in RGBA.\n");
    }
    else
    {
        printf("\nQOI in RGBA loaded!\n");
        if(width != (uint32_t)stb_width || height != (uint32_t)stb_height)
        {
            fprintf(stderr, "\x1b[1;31mERROR: dimensions parsed incorrectly from QOI header.\033[0m\n");
            return -1;
        }

        for(uint32_t y = 0; y < height; ++y)
        {
            for(uint32_t x = 0; x < width * 4; x += 4)
            {
                if(qoiRGBA[y * width * 4 + x] != stbTest[y * width * 4 + x])
                {
                    fprintf(stderr, "\x1b[1;31mERROR: red channel at y:%u, x:%u mismatch.\n", y, x);
                    goto errmsg;
                }
                else if(qoiRGBA[y * width * 4 + x + 1] != stbTest[y * width * 4 + x + 1])
                {
                    fprintf(stderr, "\x1b[1;31mERROR: green channel at y:%u, x:%u mismatch.\033[0m\n", y, x);
                    goto errmsg;
                }
                else if(qoiRGBA[y * width * 4 + x + 2] != stbTest[y * width * 4 + x + 2])
                {
                    fprintf(stderr, "\x1b[1;31mERROR: blue channel at y:%u, x:%u mismatch.\033[0m\n", y, x);
                    goto errmsg;
                }
                else if(qoiRGBA[y * width * 4 + x + 3] != stbTest[y * width * 4 + x + 3])
                {
                    fprintf(stderr, "\x1b[1;31mERROR: alpha channel at y:%u, x:%u mismatch.\033[0m\n", y, x);
                    goto errmsg;
                }
                else
                {
                    continue;
                }

errmsg:
                fprintf(stderr, "got: \nR:%u ",      qoiRGBA[y * width * 4 + x]);
                fprintf(stderr, "G:%u ",             qoiRGBA[y * width * 4 + x + 1]);
                fprintf(stderr, "B:%u ",             qoiRGBA[y * width * 4 + x + 2]);
                fprintf(stderr, "A:%u\n",            qoiRGBA[y * width * 4 + x + 3]);

                fprintf(stderr, "expected: \nR:%u ", stbTest[y * width * 4 + x]);
                fprintf(stderr, "G:%u ",             stbTest[y * width * 4 + x + 1]);
                fprintf(stderr, "B:%u ",             stbTest[y * width * 4 + x + 2]);
                fprintf(stderr, "A:%u\033[0m\n",     stbTest[y * width * 4 + x + 3]);
                return -1;
            }
        }
    }
    free(qoiRGBA);

    uint8_t* qoiBGRA = imgsurf_load("assets/tux.qoi", &width, &height, IMGSURF_CHANNELS_BGRA, 8);
    if(!qoiBGRA)
    {
        fprintf(stderr, "imgsurf_load failed to load a qoi in BGRA.\n");
    }
    else
    {
        printf("\nQOI in BGRA loaded!\n");
        if(width != (uint32_t)stb_width || height != (uint32_t)stb_height)
        {
            fprintf(stderr, "\x1b[1;31mERROR: dimensions parsed incorrectly from QOI header.\033[0m\n");
        }
        //TODAY: validate against stb_image
    }
    free(qoiBGRA);

    uint8_t* qoiRGB = imgsurf_load("assets/tux.qoi", &width, &height, IMGSURF_CHANNELS_RGB, 8);
    if(!qoiRGB)
    {
        fprintf(stderr, "imgsurf_load failed to load a qoi in RGBA.\n");
    }
    else
    {
        printf("\nQOI in RGB loaded!\n");
        if(width != (uint32_t)stb_width || height != (uint32_t)stb_height)
        {
            fprintf(stderr, "\x1b[1;31mERROR: dimensions parsed incorrectly from QOI header.\033[0m\n");
        }
        //TODAY: validate against stb_image
    }
    free(qoiRGB);

    uint8_t* qoiBGR = imgsurf_load("assets/tux.qoi", &width, &height, IMGSURF_CHANNELS_BGR, 8);
    if(!qoiBGR)
    {
        fprintf(stderr, "imgsurf_load failed to load a qoi in RGBA.\n");
    }
    else
    {
        printf("\nQOI in BGR loaded!\n");
        if(width != (uint32_t)stb_width || height != (uint32_t)stb_height)
        {
            fprintf(stderr, "\x1b[1;31mERROR: dimensions parsed incorrectly from QOI header.\033[0m\n");
        }
        //TODAY: validate against stb_image
    }
    free(qoiBGR);

    // uint8_t* bmp = imgsurf_load("assets/tux.bmp", &width, &height, IMGSURF_CHANNELS_RGB, 8);
    // if(!bmp)
    // {
    //     fprintf(stderr, "imgsurf_load failed to load a bmp.\n");
    // }
    // else
    // {
    //     printf("\nBMP loaded!\n");
    // }

    stbi_image_free(stbTest);
}

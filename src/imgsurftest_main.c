#include "imgsurf_load.h"

#include <stdio.h>
#include <stdlib.h>

int main
(
    void
){
    uint32_t width  = 0;
    uint32_t height = 0;

    uint32_t expectedWidth = 0;
    uint32_t expectedHeight = 0;

    expectedWidth  = 1584;
    expectedHeight = 1920;

    uint8_t* qoiRGBA = imgsurf_load("assets/tux.qoi", &width, &height, IMGSURF_CHANNELS_RGBA, 8);
    if(!qoiRGBA)
    {
        fprintf(stderr, "imgsurf_load failed to load a qoi in RGBA.\n");
    }
    else
    {
        printf("\nQOI in RGBA loaded!\n");
        if(width != expectedWidth || height != expectedHeight)
        {
            fprintf(stderr, "\x1b[1;31mERROR: dimensions parsed incorrectly from QOI header.\033[0m\n");
            return -1;
        }

        // for(uint32_t y = 0; y < height; ++y)
        // {
        //     for(uint32_t x = 0; x < width * 4; x += 4)
        //     {
                // if(qoiRGBA[y * width * 4 + x] != unitTest[y * width * 4 + x])
                // {
                //     fprintf(stderr, "\x1b[1;31mERROR: red channel at y:%u, x:%u mismatch.\n", y, x);
                //     goto errmsg;
                // }
                // else if(qoiRGBA[y * width * 4 + x + 1] != unitTest[y * width * 4 + x + 1])
                // {
                //     fprintf(stderr, "\x1b[1;31mERROR: green channel at y:%u, x:%u mismatch.\033[0m\n", y, x);
                //     goto errmsg;
                // }
                // else if(qoiRGBA[y * width * 4 + x + 2] != unitTest[y * width * 4 + x + 2])
                // {
                //     fprintf(stderr, "\x1b[1;31mERROR: blue channel at y:%u, x:%u mismatch.\033[0m\n", y, x);
                //     goto errmsg;
                // }
                // else if(qoiRGBA[y * width * 4 + x + 3] != unitTest[y * width * 4 + x + 3])
                // {
                //     fprintf(stderr, "\x1b[1;31mERROR: alpha channel at y:%u, x:%u mismatch.\033[0m\n", y, x);
                //     goto errmsg;
                // }
                // else
                // {
                //     continue;
                // }

// errmsg:
                // fprintf(stderr, "got: \nR:%u ",      qoiRGBA[y * width * 4 + x]);
                // fprintf(stderr, "G:%u ",             qoiRGBA[y * width * 4 + x + 1]);
                // fprintf(stderr, "B:%u ",             qoiRGBA[y * width * 4 + x + 2]);
                // fprintf(stderr, "A:%u\n",            qoiRGBA[y * width * 4 + x + 3]);

                // fprintf(stderr, "expected: \nR:%u ", unitTest[y * width * 4 + x]);
                // fprintf(stderr, "G:%u ",             unitTest[y * width * 4 + x + 1]);
                // fprintf(stderr, "B:%u ",             unitTest[y * width * 4 + x + 2]);
                // fprintf(stderr, "A:%u\033[0m\n",     unitTest[y * width * 4 + x + 3]);
                // free(qoiRGBA);
                // return -1;
    //         }
    //     }
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
        //TODO: validate against qoiRGBA
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
        //TODO: validate against qoiRGBA
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
        //TODO: validate against qoiRGBA
    }
    free(qoiBGR);
    return 0;
}

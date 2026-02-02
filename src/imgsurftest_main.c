#include "imgsurf_main.h"

#include <stdio.h>
#include <stdlib.h>

// TODO: validate files not with other image loaders (if avoidable),
// but by loading the file, saving it, then comparing the data between files

int main
(
    void
){
    uint32_t width  = 0;
    uint32_t height = 0;

    uint32_t expectedWidth  = 10;
    uint32_t expectedHeight = 10;

    uint8_t* smallTestRGBA = imgsurf_load("assets/smallTest.qoi", &width, &height, IMGSURF_CHANNELS_RGBA, 8);
    if(!smallTestRGBA)
    {
        fprintf(stderr, "imgsurf_load failed to load small test image.\n");
        return -1;
    }
    uint8_t result = imgsurf_write_file("assets/smallTest_reconstructed.qoi", smallTestRGBA, width, height, IMGSURF_CHANNELS_RGBA, 8, IMGSURF_FILE_QOI);
    if(result)
    {
        fprintf(stderr, "imgsurf_load failed to write back small test image.\n");
        return result;
    }
    uint8_t *smallTestRGBA_verify = imgsurf_load("assets/smallTest_reconstructed.qoi", &width, &height, IMGSURF_CHANNELS_RGBA, 8);

    for(uint8_t i = 0; i < width * height * 4; i += 4)
    {
        if(smallTestRGBA_verify[i] != smallTestRGBA[i])
        {
            fprintf(stderr, "\x1b[1;31munit test failed @ pixel %u.red, expected: %u, got: %u\n", i / 4, smallTestRGBA[i], smallTestRGBA_verify[i]);
            return -1;
        }
        if(smallTestRGBA_verify[i + 1] != smallTestRGBA[i + 1])
        {
            fprintf(stderr, "\x1b[1;31munit test failed @ pixel %u.green, expected: %u, got: %u\n", i / 4, smallTestRGBA[i], smallTestRGBA_verify[i]);
            return -1;
        }
        if(smallTestRGBA_verify[i + 2] != smallTestRGBA[i + 2])
        {
            fprintf(stderr, "\x1b[1;31munit test failed @ pixel %u.blue, expected: %u, got: %u\n", i / 4, smallTestRGBA[i], smallTestRGBA_verify[i]);
            return -1;
        }
        if(smallTestRGBA_verify[i + 3] != smallTestRGBA[i + 3])
        {
            fprintf(stderr, "\x1b[1;31munit test failed @ pixel %u.alpha, expected: %u, got: %u\n", i / 4, smallTestRGBA[i], smallTestRGBA_verify[i]);
            return -1;
        }
    }

    //TESTING: returning early for ease-of-use
    return 0;

    width  = 0;
    height = 0;

    expectedWidth  = 1584;
    expectedHeight = 1920;

    uint8_t* qoiRGBA = imgsurf_load("assets/tux.qoi", &width, &height, IMGSURF_CHANNELS_RGBA, 8);
    if(!qoiRGBA)
    {
        fprintf(stderr, "imgsurf_load failed to load a qoi in RGBA.\n");
        return -1;
    }
    printf("\nQOI in RGBA loaded!\n");
    if(width != expectedWidth || height != expectedHeight)
    {
        fprintf(stderr, "\x1b[1;31mERROR: dimensions parsed incorrectly from QOI header.\033[0m\n");
        return -1;
    }

    //TODO: verify that the format actually just saves as that format, not reads
    result = imgsurf_write_file("assets/tux_reconstructed.qoi", qoiRGBA, width, height, IMGSURF_CHANNELS_RGBA, 8, IMGSURF_FILE_QOI);
    if(result)
    {
        fprintf(stderr, "imgsurf_load failed to write back tux test image.\n");
        return result;
    }

    free(qoiRGBA);
    qoiRGBA = imgsurf_load("assets/tux_reconstructed.qoi", &width, &height, IMGSURF_CHANNELS_RGBA, 8);
    if(!qoiRGBA)
    {
        fprintf(stderr, "imgsurf_load failed to load reconstructed RGBA QOI.\n");
        return -2;
    }
    printf("\nQOI in RGBA loaded!\n");
    if(width != expectedWidth || height != expectedHeight)
    {
        fprintf(stderr, "\x1b[1;31mERROR: dimensions parsed incorrectly from reconstructed QOI header.\033[0m\n");
        return -3;
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
    free(qoiRGBA);

    uint8_t* qoiBGRA = imgsurf_load("assets/tux.qoi", &width, &height, IMGSURF_CHANNELS_BGRA, 8);
    if(!qoiBGRA)
    {
        fprintf(stderr, "imgsurf_load failed to load a qoi in BGRA.\n");
        return -4;
    }
    printf("\nQOI in BGRA loaded!\n");

    // TODO: (imgsurf #1) validate against qoiRGBA, then other formats
    free(qoiBGRA);

    return 0;
}

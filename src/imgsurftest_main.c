#include "imgsurf_main.h"

#include <stdlib.h>

int verifyImage
(
    const char *name_qoi//,
    // const char *name_png
){
    fprintf(stderr, "\ntrying to verify image: %s\n", name_qoi);

    uint32_t width  = 0;
    uint32_t height = 0;

    uint8_t *testRGBA = imgsurf_load_file(name_qoi, &width, &height, IMGSURF_CHANNELS_RGBA, 8);
    if(!testRGBA)
    {
        fprintf(stderr, "\x1b[1;31mimgsurf_load failed.\033[0m\n");
        return 1;
    }
    // uint8_t *test_stb = stbi_load(name_png, &stb_width, &stb_height, &stb_channels, STBI_rgb_alpha);
    // if(!test_stb)
    // {
    //     fprintf(stderr, "\x1b[1;31mstb_image failed: %s\n", stbi_failure_reason());
    //     return -1;
    // }

    // if((int)width != stb_width)
    // {
    //     fprintf(stderr, "\x1b[1;31mimgsurf_load failed to correctly load width. Expected: %u, got: %u.\033[0m\n", stb_width, width);
    //     return 2;
    // }
    //
    // if((int)height != stb_height)
    // {
    //     fprintf(stderr, "\x1b[1;31mimgsurf_load failed to correctly load height. Expected: %u, got: %u.\033[0m\n", stb_height, height);
    //     return 3;
    // }

    // for(uint64_t i = 0; i < width * height * 4; i += 4)
    // {
    //     if(testRGBA[i] != test_stb[i])
    //     {
    //         fprintf(stderr, "\x1b[1;31mimgsurf_load failed @ pixel %lu.red, expected: %u, got: %u\n", i / 4, testRGBA[i], test_stb[i]);
    //         return -2;
    //     }
    //     if(testRGBA[i + 1] != test_stb[i + 1])
    //     {
    //         fprintf(stderr, "\x1b[1;31mimgsurf_load failed @ pixel %lu.green, expected: %u, got: %u\n", i / 4, testRGBA[i], test_stb[i]);
    //         return -3;
    //     }
    //     if(testRGBA[i + 2] != test_stb[i + 2])
    //     {
    //         fprintf(stderr, "\x1b[1;31mimgsurf_load failed @ pixel %lu.blue, expected: %u, got: %u\n", i / 4, testRGBA[i], test_stb[i]);
    //         return -4;
    //     }
    //     if(testRGBA[i + 3] != test_stb[i + 3])
    //     {
    //         fprintf(stderr, "\x1b[1;31mimgsurf_load failed @ pixel %lu.alpha, expected: %u, got: %u\n", i / 4, testRGBA[i], test_stb[i]);
    //         return -5;
    //     }
    // }

    uint8_t result = imgsurf_write_file("assets/reconstructed.qoi", testRGBA, width, height, IMGSURF_CHANNELS_RGBA, 8, IMGSURF_FILE_QOI);
    if(result)
    {
        return result;
    }

    uint8_t *reconstructed = imgsurf_load_file("assets/reconstructed.qoi", &width, &height, IMGSURF_CHANNELS_RGBA, 8);

    // for(uint64_t i = 0; i < width * height * 4; i += 4)
    // {
    //     if(reconstructed[i] != test_stb[i])
    //     {
    //         fprintf(stderr, "\x1b[1;31mimgsurf_write failed @ pixel %lu.red, expected: %u, got: %u\n", i / 4, reconstructed[i], test_stb[i]);
    //         return 2;
    //     }
    //     if(reconstructed[i + 1] != test_stb[i + 1])
    //     {
    //         fprintf(stderr, "\x1b[1;31mimgsurf_write failed @ pixel %lu.green, expected: %u, got: %u\n", i / 4, reconstructed[i], test_stb[i]);
    //         return 3;
    //     }
    //     if(reconstructed[i + 2] != test_stb[i + 2])
    //     {
    //         fprintf(stderr, "\x1b[1;31mimgsurf_write failed @ pixel %lu.blue, expected: %u, got: %u\n", i / 4, reconstructed[i], test_stb[i]);
    //         return 4;
    //     }
    //     if(reconstructed[i + 3] != test_stb[i + 3])
    //     {
    //         fprintf(stderr, "\x1b[1;31mimgsurf_write failed @ pixel %lu.alpha, expected: %u, got: %u\n", i / 4, reconstructed[i], test_stb[i]);
    //         return 5;
    //     }
    // }

    // free(test_stb);
    free(testRGBA);
    free(reconstructed);

    return 0;
}

int main
(
    void
){
    int result = 0;

    if((result = verifyImage("assets/smallTest.qoi"/*, "assets/smallTest.png"*/)))
    {
        fprintf(stderr, "\x1b[7;31mERROR: unit test not passed with image assets/smallTest!\n");
        return result;
    }
    fprintf(stderr, "\033[32;1;1mSUCCESS: unit test passed with image assets/smallTest!\033[0m\n");

    if((result = verifyImage("assets/black.qoi"/*, "assets/black.png"*/)))
    {
        fprintf(stderr, "\x1b[7;31mERROR: unit test not passed with image assets/black!\033[0m\n");
        return result;
    }
    fprintf(stderr, "\033[32;1;1mSUCCESS: unit test passed with image assets/black!\033[0m\n");

    if((result = verifyImage("assets/tux.qoi"/*, "assets/tux.png"*/)))
    {
        fprintf(stderr, "\x1b[7;31mERROR: unit tux not passed with image assets/tux!\033[0m\n");
        return result;
    }
    fprintf(stderr, "\033[32;1;1mSUCCESS: unit test passed with image assets/tux!\033[0m\n");
}

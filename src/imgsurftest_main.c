#include "imgsurf_main.h"

#include <stdlib.h>

int verifyImage
(
    const char *name_qoi,
    const char *name_png
){
    fprintf(stderr, "\ntrying to verify image: %s with %s\n", name_qoi, name_png);

    uint32_t width  = 0;
    uint32_t height = 0;

    uint8_t *testQOI = imgsurf_load_file(name_qoi, &width, &height, IMGSURF_CHANNELS_RGBA, 8);
    if(!testQOI)
    {
        fprintf(stderr, "\x1b[1;31mimgsurf_load failed on QOI file.\033[0m\n");
        return 1;
    }

    uint8_t *testPNG = imgsurf_load_file(name_png, &width, &height, IMGSURF_CHANNELS_RGBA, 8);
    if(!testPNG)
    {
        fprintf(stderr, "\x1b[1;31mimgsurf_load failed on PNG file.\033[0m\n");
        return 2;
    }

    for(uint64_t i = 0; i < width * height * 4; i += 4)
    {
        if(testQOI[i] != testPNG[i])
        {
            fprintf(stderr, "\x1b[1;31mimgsurf_load failed @ pixel %lu.red, expected: %u, got: %u\n", i / 4, testQOI[i], testPNG[i]);
            return -2;
        }
        if(testQOI[i + 1] != testPNG[i + 1])
        {
            fprintf(stderr, "\x1b[1;31mimgsurf_load failed @ pixel %lu.green, expected: %u, got: %u\n", i / 4, testQOI[i], testPNG[i]);
            return -3;
        }
        if(testQOI[i + 2] != testPNG[i + 2])
        {
            fprintf(stderr, "\x1b[1;31mimgsurf_load failed @ pixel %lu.blue, expected: %u, got: %u\n", i / 4, testQOI[i], testPNG[i]);
            return -4;
        }
        if(testQOI[i + 3] != testPNG[i + 3])
        {
            fprintf(stderr, "\x1b[1;31mimgsurf_load failed @ pixel %lu.alpha, expected: %u, got: %u\n", i / 4, testQOI[i], testPNG[i]);
            return -5;
        }
    }

    uint8_t result = imgsurf_write_file("assets/reconstructed.qoi", testQOI, width, height, IMGSURF_CHANNELS_RGBA, 8, IMGSURF_FILE_QOI);
    if(result)
    {
        return result;
    }

    uint8_t *reconstructedQOI = imgsurf_load_file("assets/reconstructed.qoi", &width, &height, IMGSURF_CHANNELS_RGBA, 8);

    result = imgsurf_write_file("assets/reconstructed.png", testPNG, width, height, IMGSURF_CHANNELS_RGBA, 8, IMGSURF_FILE_PNG);
    if(result)
    {
        return result;
    }

    uint8_t *reconstructedPNG = imgsurf_load_file("assets/reconstructed.png", &width, &height, IMGSURF_CHANNELS_RGBA, 8);

    for(uint64_t i = 0; i < width * height * 4; i += 4)
    {
        if(reconstructedQOI[i] != reconstructedPNG[i])
        {
            fprintf(stderr, "\x1b[1;31mimgsurf_write failed @ pixel %lu.red, expected: %u, got: %u\n", i / 4, reconstructedQOI[i], reconstructedPNG[i]);
            return 2;
        }
        if(reconstructedQOI[i + 1] != reconstructedPNG[i + 1])
        {
            fprintf(stderr, "\x1b[1;31mimgsurf_write failed @ pixel %lu.green, expected: %u, got: %u\n", i / 4, reconstructedQOI[i], reconstructedPNG[i]);
            return 3;
        }
        if(reconstructedQOI[i + 2] != reconstructedPNG[i + 2])
        {
            fprintf(stderr, "\x1b[1;31mimgsurf_write failed @ pixel %lu.blue, expected: %u, got: %u\n", i / 4, reconstructedQOI[i], reconstructedPNG[i]);
            return 4;
        }
        if(reconstructedQOI[i + 3] != reconstructedPNG[i + 3])
        {
            fprintf(stderr, "\x1b[1;31mimgsurf_write failed @ pixel %lu.alpha, expected: %u, got: %u\n", i / 4, reconstructedQOI[i], reconstructedPNG[i]);
            return 5;
        }
    }

    free(testQOI);
    free(testPNG);
    free(reconstructedQOI);

    return 0;
}

int main
(
    void
){
    int result = 0;

    if((result = verifyImage("assets/smallTest.qoi", "assets/smallTest.png")))
    {
        fprintf(stderr, "\x1b[7;31mERROR: unit test not passed with image assets/smallTest!\n");
        return result;
    }
    fprintf(stderr, "\033[32;1;1mSUCCESS: unit test passed with image assets/smallTest!\033[0m\n");

    if((result = verifyImage("assets/black.qoi", "assets/black.png")))
    {
        fprintf(stderr, "\x1b[7;31mERROR: unit test not passed with image assets/black!\033[0m\n");
        return result;
    }
    fprintf(stderr, "\033[32;1;1mSUCCESS: unit test passed with image assets/black!\033[0m\n");

    if((result = verifyImage("assets/tux.qoi", "assets/tux.png")))
    {
        fprintf(stderr, "\x1b[7;31mERROR: unit tux not passed with image assets/tux!\033[0m\n");
        return result;
    }
    fprintf(stderr, "\033[32;1;1mSUCCESS: unit test passed with image assets/tux!\033[0m\n");
}

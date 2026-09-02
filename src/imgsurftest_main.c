#include "imgsurf_main.h"

#include <stdlib.h>

f_internal bool verifyImage_read
(
    const char *name_qoi,
    const char *name_png
){
    fprintf(stderr, "\ntrying to verify reading image: %s with %s\n",
                    name_qoi, name_png);
    uint32_t width  = 0;
    uint32_t height = 0;

    uint8_t *testQOI = imgsurf_load_file(name_qoi, &width, &height,
                                         IMGSURF_CHANNELS_RGBA, 8);
    if(!testQOI)
    {
        fprintf(stderr, "\x1b[1;31m\nimgsurf_load failed on QOI file.\033[0m\n");
        goto fail;
    }

    uint8_t *testPNG = imgsurf_load_file(name_png, &width, &height,
                                         IMGSURF_CHANNELS_RGBA, 8);
    if(!testPNG)
    {
        fprintf(stderr, "\x1b[1;31m\nimgsurf_load failed on PNG file.\033[0m\n");
        goto fail;
    }

    for(uint64_t i = 0; i < width * height * 4; i += 4)
    {
        if(testQOI[i] != testPNG[i])
        {
            fprintf(stderr, "\x1b[1;31mimgsurf_load failed @ pixel %lu.red, expected: "
                    "%u, got: %u\n", i / 4, testQOI[i], testPNG[i]);
            goto fail;
        }
        if(testQOI[i + 1] != testPNG[i + 1])
        {
            fprintf(stderr, "\x1b[1;31mimgsurf_load failed @ pixel %lu.green, expected: "
                    "%u, got: %u\n", i / 4, testQOI[i], testPNG[i]);
            goto fail;
        }
        if(testQOI[i + 2] != testPNG[i + 2])
        {
            fprintf(stderr, "\x1b[1;31mimgsurf_load failed @ pixel %lu.blue, expected: "
                    "%u, got: %u\n", i / 4, testQOI[i], testPNG[i]);
            goto fail;
        }
        if(testQOI[i + 3] != testPNG[i + 3])
        {
            fprintf(stderr, "\x1b[1;31mimgsurf_load failed @ pixel %lu.alpha, expected:"
                    " %u, got: %u\n", i / 4, testQOI[i], testPNG[i]);
            goto fail;
        }
    }

    free(testQOI);
    free(testPNG);
    return true;

fail:
    if(testQOI)
    {
        free(testQOI);
    }
    if(testPNG)
    {
        free(testPNG);
    }

    return false;
}

f_internal bool verifyImage_write
(
    const char *name_qoi,
    const char *name_png
){
    fprintf(stderr, "\ntrying to verify writing image: %s with %s\n",
            name_qoi, name_png);

    uint32_t width  = 0;
    uint32_t height = 0;

    uint8_t *testPNG = imgsurf_load_file(name_png, &width, &height,
                                         IMGSURF_CHANNELS_RGBA, 8);
    if(!testPNG)
    {
        fprintf(stderr, "\x1b[1;31mimgsurf_load failed on PNG file.\033[0m\n");
        goto fail;
    }

    if(!imgsurf_write_file("assets/reconstructed.qoi", testPNG, width, height,
                           IMGSURF_CHANNELS_RGBA, 8, IMGSURF_FILE_QOI)
    ){
        fprintf(stderr, "\x1b[1;31mimgsurf_write_file failed on QOI file.\033[0m\n");
        goto fail;
    }

    uint8_t *reconstructedQOI = imgsurf_load_file("assets/reconstructed.qoi", &width,
                                                  &height, IMGSURF_CHANNELS_RGBA, 8);

    for(uint64_t i = 0; i < width * height * 4; i += 4)
    {
        if(reconstructedQOI[i] != testPNG[i])
        {
            fprintf(stderr, "\x1b[1;31mimgsurf_write failed @ pixel %lu.red, expected: "
                    "%u, got: %u\n", i / 4, reconstructedQOI[i], testPNG[i]);
            goto fail;
        }
        if(reconstructedQOI[i + 1] != testPNG[i + 1])
        {
            fprintf(stderr, "\x1b[1;31mimgsurf_write failed @ pixel %lu.green, "
                    "expected: %u, got: %u\n", i / 4, reconstructedQOI[i], testPNG[i]);
            goto fail;
        }
        if(reconstructedQOI[i + 2] != testPNG[i + 2])
        {
            fprintf(stderr, "\x1b[1;31mimgsurf_write failed @ pixel %lu.blue, expected:"
                    " %u, got: %u\n", i / 4, reconstructedQOI[i], testPNG[i]);
            goto fail;
        }
        if(reconstructedQOI[i + 3] != testPNG[i + 3])
        {
            fprintf(stderr, "\x1b[1;31mimgsurf_write failed @ pixel %lu.alpha, "
                    "expected: %u, got: %u\n", i / 4, reconstructedQOI[i], testPNG[i]);
            goto fail;
        }
    }

    free(testPNG);
    free(reconstructedQOI);

    return true;

fail:
    if(testPNG)
    {
        free(testPNG);
    }
    if(reconstructedQOI)
    {
        free(reconstructedQOI);
    }

    return false;
}

int main
(
    void
){
    bool result = true;

    if(!verifyImage_read("assets/smallTest.qoi", "assets/smallTest.png"))
    {
        fprintf(stderr, "\x1b[7;31mERROR: READ test not passed with image "
                "assets/smallTest!\033[0m\n");
        result = false;
    }
    else
    {
        fprintf(stderr, "\033[32;1;1mSUCCESS: READ test passed with image "
                "assets/smallTest!\033[0m\n");
    }

    if(!verifyImage_read("assets/black.qoi", "assets/black.png"))
    {
        fprintf(stderr, "\x1b[7;31mERROR: READ test not passed with image "
                "assets/black!\033[0m\n");
        result = false;
    }
    else
    {
        fprintf(stderr, "\033[32;1;1mSUCCESS: READ test passed with image "
                "assets/black!\033[0m\n");
    }

    if(!verifyImage_read("assets/tux.qoi", "assets/tux.png"))
    {
        fprintf(stderr, "\x1b[7;31mERROR: READ test not passed with image "
                "assets/tux!\033[0m\n");
        result = false;
    }
    else
    {
        fprintf(stderr, "\033[32;1;1mSUCCESS: READ test passed with image "
                "assets/tux!\033[0m\n");
    }

    if(!verifyImage_write("assets/smallTest.qoi", "assets/smallTest.png"))
    {
        fprintf(stderr, "\x1b[7;31mERROR: WRITE test not passed with image "
                "assets/smallTest!\033[0m\n");
        result = false;
    }
    else
    {
        fprintf(stderr, "\033[32;1;1mSUCCESS: WRITE test passed with image "
                "assets/smallTest!\033[0m\n");
    }

    if(!verifyImage_write("assets/black.qoi", "assets/black.png"))
    {
        fprintf(stderr, "\x1b[7;31mERROR: WRITE test not passed with image "
                "assets/black!\033[0m\n");
        result = false;
    }
    else
    {
        fprintf(stderr, "\033[32;1;1mSUCCESS: WRITE test passed with image "
                "assets/black!\033[0m\n");
    }

    if(!verifyImage_write("assets/tux.qoi", "assets/tux.png"))
    {
        fprintf(stderr, "\x1b[7;31mERROR: WRITE test not passed with image "
                "assets/tux!\033[0m\n");
        result = false;
    }
    else
    {
        fprintf(stderr, "\033[32;1;1mSUCCESS: WRITE test passed with image "
                "assets/tux!\033[0m\n");
    }

    return result;
}

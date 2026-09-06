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

    uint8_t *testQOI = imLoadFile(name_qoi, &width, &height, IM_CHANNELS_RGBA, 8);
    if(!testQOI)
    {
        fprintf(stderr, "\033[1;31m\nimLoadFile failed on QOI.\033[0m\n");
        return false;
    }

    uint8_t *testPNG = imLoadFile(name_png, &width, &height, IM_CHANNELS_RGBA, 8);
    if(!testPNG)
    {
        fprintf(stderr, "\033[1;31m\nimLoadFile failed on PNG.\033[0m\n");
        free(testQOI);
        return false;
    }

    for(uint64_t i = 0; i < width * height * 4; i += 4)
    {
        if(testQOI[i] != testPNG[i])
        {
            fprintf(stderr, "\033[1;31mimLoadFile failed @ pixel %lu.red, expected: "
                    "%u, got: %u\n", i / 4, testQOI[i], testPNG[i]);
            free(testQOI);
            free(testPNG);
            return false;
        }
        if(testQOI[i + 1] != testPNG[i + 1])
        {
            fprintf(stderr, "\033[1;31mimLoadFile failed @ pixel %lu.green, expected: "
                    "%u, got: %u\n", i / 4, testQOI[i + 1], testPNG[i + 1]);
            free(testQOI);
            free(testPNG);
            return false;
        }
        if(testQOI[i + 2] != testPNG[i + 2])
        {
            fprintf(stderr, "\033[1;31mimLoadFile failed @ pixel %lu.blue, expected: "
                    "%u, got: %u\n", i / 4, testQOI[i + 2], testPNG[i + 2]);
            free(testQOI);
            free(testPNG);
            return false;
        }
        if(testQOI[i + 3] != testPNG[i + 3])
        {
            fprintf(stderr, "\033[1;31mimLoadFile failed @ pixel %lu.alpha, expected:"
                    " %u, got: %u\n", i / 4, testQOI[i + 3], testPNG[i + 3]);
            free(testQOI);
            free(testPNG);
            return false;
        }
    }

    free(testQOI);
    free(testPNG);
    return true;
}

f_internal bool verifyImage_write
(
    const char *name
){
    fprintf(stderr, "\ntrying to verify writing loaded image: '%s'\n", name);

    uint32_t width  = 0;
    uint32_t height = 0;

    uint8_t *test = imLoadFile(name, &width, &height, IM_CHANNELS_RGBA, 8);
    if(!test)
    {
        fprintf(stderr, "\033[1;31mimLoadFile failed on QOI.\033[0m\n");
        return false;
    }

    if(!imWriteFile("assets/reconstructed.qoi", test, width, height,
                    IM_CHANNELS_RGBA, 8, IM_FILE_QOI)
    ){
        fprintf(stderr, "\033[1;31mimWriteFile failed on QOI.\033[0m\n");
        free(test);
        return false;
    }

    uint8_t *reconstructedQOI = imLoadFile("assets/reconstructed.qoi", &width, &height,
                                           IM_CHANNELS_RGBA, 8);
    if(!reconstructedQOI)
    {
        fprintf(stderr, "\033[1;31mimLoadFile failed to load reconstructed QOI image."
                "\033[0m\n");
        return false;
    }

    for(uint64_t i = 0; i < width * height * 4; i += 4)
    {
        if(reconstructedQOI[i] != test[i])
        {
            fprintf(stderr, "\033[1;31mimWriteFile failed @ pixel %lu.red, expected: "
                    "%u, got: %u\n", i / 4, reconstructedQOI[i], test[i]);
            free(test);
            free(reconstructedQOI);
            return false;
        }
        if(reconstructedQOI[i + 1] != test[i + 1])
        {
            fprintf(stderr, "\033[1;31mimWriteFile failed @ pixel %lu.green, expected: "
                    "%u, got: %u\n", i / 4, reconstructedQOI[i + 1], test[i + 1]);
            free(test);
            free(reconstructedQOI);
            return false;
        }
        if(reconstructedQOI[i + 2] != test[i + 2])
        {
            fprintf(stderr, "\033[1;31mimWriteFile failed @ pixel %lu.blue, expected: "
                    "%u, got: %u\n", i / 4, reconstructedQOI[i + 2], test[i + 2]);
            free(test);
            free(reconstructedQOI);
            return false;
        }
        if(reconstructedQOI[i + 3] != test[i + 3])
        {
            fprintf(stderr, "\033[1;31mimWriteFile failed @ pixel %lu.alpha, expected"
                    ": %u, got: %u\n", i / 4, reconstructedQOI[i + 3], test[i + 3]);
            free(test);
            free(reconstructedQOI);
            return false;
        }
    }

    free(reconstructedQOI);

    if(!imWriteFile("assets/reconstructed.png", test, width, height,
                    IM_CHANNELS_RGBA, 8, IM_FILE_PNG)
    ){
        fprintf(stderr, "\033[1;31mimWriteFile failed on PNG.\033[0m\n");
        free(test);
        return false;
    }

    uint8_t *reconstructedPNG = imLoadFile("assets/reconstructed.png", &width, &height,
                                           IM_CHANNELS_RGBA, 8);
    if(!reconstructedPNG)
    {
        fprintf(stderr, "\033[1;31mimLoadFile failed to load reconstructed PNG image."
                "\033[0m\n");
        return false;
    }

    for(uint64_t i = 0; i < width * height * 4; i += 4)
    {
        if(reconstructedPNG[i] != test[i])
        {
            fprintf(stderr, "\033[1;31mimWriteFile failed @ pixel %lu.red, expected: "
                    "%u, got: %u\n", i / 4, reconstructedPNG[i], test[i]);
            free(test);
            free(reconstructedPNG);
            return false;
        }
        if(reconstructedPNG[i + 1] != test[i + 1])
        {
            fprintf(stderr, "\033[1;31mimWriteFile failed @ pixel %lu.green, expected: "
                    "%u, got: %u\n", i / 4, reconstructedPNG[i + 1], test[i + 1]);
            free(test);
            free(reconstructedPNG);
            return false;
        }
        if(reconstructedPNG[i + 2] != test[i + 2])
        {
            fprintf(stderr, "\033[1;31mimWriteFile failed @ pixel %lu.blue, expected: "
                    "%u, got: %u\n", i / 4, reconstructedPNG[i + 2], test[i + 2]);
            free(test);
            free(reconstructedPNG);
            return false;
        }
        if(reconstructedPNG[i + 3] != test[i + 3])
        {
            fprintf(stderr, "\033[1;31mimWriteFile failed @ pixel %lu.alpha, expected"
                    ": %u, got: %u\n", i / 4, reconstructedPNG[i + 3], test[i + 3]);
            free(test);
            free(reconstructedPNG);
            return false;
        }
    }

    free(test);
    return true;
}

int main
(
    void
){
    bool result = true;

    if(!verifyImage_read("assets/smallTest.qoi", "assets/smallTest.png"))
    {
        fprintf(stderr, "\033[7;31mERROR: READ test not passed with image "
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
        fprintf(stderr, "\033[7;31mERROR: READ test not passed with image "
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
        fprintf(stderr, "\033[7;31mERROR: READ test not passed with image "
                "assets/tux!\033[0m\n");
        result = false;
    }
    else
    {
        fprintf(stderr, "\033[32;1;1mSUCCESS: READ test passed with image "
                "assets/tux!\033[0m\n");
    }

    if(!verifyImage_write("assets/smallTest.qoi"))
    {
        fprintf(stderr, "\033[7;31mERROR: WRITE test not passed with image "
                "assets/smallTest!\033[0m\n");
        result = false;
    }
    else
    {
        fprintf(stderr, "\033[32;1;1mSUCCESS: WRITE test passed with image "
                "assets/smallTest!\033[0m\n");
    }

    if(!verifyImage_write("assets/black.qoi"))
    {
        fprintf(stderr, "\033[7;31mERROR: WRITE test not passed with image "
                "assets/black!\033[0m\n");
        result = false;
    }
    else
    {
        fprintf(stderr, "\033[32;1;1mSUCCESS: WRITE test passed with image "
                "assets/black!\033[0m\n");
    }

    if(!verifyImage_write("assets/tux.qoi"))
    {
        fprintf(stderr, "\033[7;31mERROR: WRITE test not passed with image "
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

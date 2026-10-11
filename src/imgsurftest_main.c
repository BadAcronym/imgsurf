#include "imgsurf_main.h"
#include "im_print_macros.h"

#include <stdlib.h>

f_internal bool verifyImage_read
(
    const char *name_qoi,
    const char *name_png
){
    IM_DEBUG("trying to verify reading image: %s with %s", name_qoi, name_png);
    uint32_t width  = 0;
    uint32_t height = 0;

    uint8_t *testQOI = imLoadFile(name_qoi, &width, &height, IM_CHANNELS_RGBA, 8);
    if(!testQOI)
    {
        IM_FAIL("imLoadFile on QOI.");
        return false;
    }

    uint8_t *testPNG = imLoadFile(name_png, &width, &height, IM_CHANNELS_RGBA, 8);
    if(!testPNG)
    {
        IM_FAIL("imLoadFile on PNG.");
        free(testQOI);
        return false;
    }

    for(uint64_t i = 0; i < width * height * 4; i += 4)
    {
        if(testQOI[i] != testPNG[i])
        {
            IM_ERROR("imLoadFile failed @ pixel %lu.red, expected: "
                     "%u, got: %u", i / 4, testQOI[i], testPNG[i]);
            free(testQOI);
            free(testPNG);
            return false;
        }
        if(testQOI[i + 1] != testPNG[i + 1])
        {
            IM_ERROR("imLoadFile failed @ pixel %lu.green, expected: "
                     "%u, got: %u", i / 4, testQOI[i + 1], testPNG[i + 1]);
            free(testQOI);
            free(testPNG);
            return false;
        }
        if(testQOI[i + 2] != testPNG[i + 2])
        {
            IM_ERROR("imLoadFile failed @ pixel %lu.blue, expected: "
                     "%u, got: %u", i / 4, testQOI[i + 2], testPNG[i + 2]);
            free(testQOI);
            free(testPNG);
            return false;
        }
        if(testQOI[i + 3] != testPNG[i + 3])
        {
            IM_ERROR("imLoadFile failed @ pixel %lu.alpha, expected: "
                     "%u, got: %u", i / 4, testQOI[i + 3], testPNG[i + 3]);
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
    IM_DEBUG("trying to verify writing loaded image: '%s'", name);

    uint32_t width  = 0;
    uint32_t height = 0;

    uint8_t *test = imLoadFile(name, &width, &height, IM_CHANNELS_RGBA, 8);
    if(!test)
    {
        IM_ERROR("imLoadFile could not read QOI.");
        return false;
    }

    if(!imWriteFile("assets/reconstructed.qoi", test, width, height,
                    IM_CHANNELS_RGBA, 8, IM_FILE_QOI)
    ){
        IM_ERROR("imWriteFile could not write QOI.");
        free(test);
        return false;
    }

    uint8_t *reconstructedQOI = imLoadFile("assets/reconstructed.qoi", &width, &height,
                                           IM_CHANNELS_RGBA, 8);
    if(!reconstructedQOI)
    {
        IM_ERROR("imLoadFile could not load reconstructed QOI image.");
        return false;
    }

    for(uint64_t i = 0; i < width * height * 4; i += 4)
    {
        if(reconstructedQOI[i] != test[i])
        {
            IM_ERROR("imWriteFile failed @ pixel %lu.red, expected: "
                     "%u, got: %u", i / 4, reconstructedQOI[i], test[i]);
            free(test);
            free(reconstructedQOI);
            return false;
        }
        if(reconstructedQOI[i + 1] != test[i + 1])
        {
            IM_ERROR("imWriteFile failed @ pixel %lu.green, expected: "
                     "%u, got: %u", i / 4, reconstructedQOI[i + 1], test[i + 1]);
            free(test);
            free(reconstructedQOI);
            return false;
        }
        if(reconstructedQOI[i + 2] != test[i + 2])
        {
            IM_ERROR("imWriteFile failed @ pixel %lu.blue, expected: "
                     "%u, got: %u", i / 4, reconstructedQOI[i + 2], test[i + 2]);
            free(test);
            free(reconstructedQOI);
            return false;
        }
        if(reconstructedQOI[i + 3] != test[i + 3])
        {
            IM_ERROR("imWriteFile failed @ pixel %lu.alpha, expected: "
                     "%u, got: %u", i / 4, reconstructedQOI[i + 3], test[i + 3]);
            free(test);
            free(reconstructedQOI);
            return false;
        }
    }

    free(reconstructedQOI);

    if(!imWriteFile("assets/reconstructed.png", test, width, height,
                    IM_CHANNELS_RGBA, 8, IM_FILE_PNG)
    ){
        IM_ERROR("imWriteFile could not write PNG.");
        free(test);
        return false;
    }

    uint8_t *reconstructedPNG = imLoadFile("assets/reconstructed.png", &width, &height,
                                           IM_CHANNELS_RGBA, 8);
    if(!reconstructedPNG)
    {
        IM_ERROR("imLoadFile could not load reconstructed PNG image.");
        return false;
    }

    for(uint64_t i = 0; i < width * height * 4; i += 4)
    {
        if(reconstructedPNG[i] != test[i])
        {
            IM_ERROR("imWriteFile failed @ pixel %lu.red, expected: "
                     "%u, got: %u", i / 4, reconstructedQOI[i], test[i]);
            free(test);
            free(reconstructedPNG);
            return false;
        }
        if(reconstructedPNG[i + 1] != test[i + 1])
        {
            IM_ERROR("imWriteFile failed @ pixel %lu.green, expected: "
                     "%u, got: %u", i / 4, reconstructedQOI[i + 1], test[i + 1]);
            free(test);
            free(reconstructedPNG);
            return false;
        }
        if(reconstructedPNG[i + 2] != test[i + 2])
        {
            IM_ERROR("imWriteFile failed @ pixel %lu.blue, expected: "
                     "%u, got: %u", i / 4, reconstructedQOI[i + 2], test[i + 2]);
            free(test);
            free(reconstructedPNG);
            return false;
        }
        if(reconstructedPNG[i + 3] != test[i + 3])
        {
            IM_ERROR("imWriteFile failed @ pixel %lu.alpha, expected: "
                     "%u, got: %u", i / 4, reconstructedQOI[i + 3], test[i + 3]);
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
        IM_FAIL("verifyImage_read with image assets/smallTest.");
        result = false;
    }
    else
    {
        IM_SUCCESS("verifyImage_read with image assets/smallTest.");
    }

    if(!verifyImage_read("assets/black.qoi", "assets/black.png"))
    {
        IM_FAIL("verifyImage_read with image assets/black.");
        result = false;
    }
    else
    {
        IM_SUCCESS("verifyImage_read with image assets/black.");
    }

    if(!verifyImage_read("assets/tux.qoi", "assets/tux.png"))
    {
        IM_FAIL("verifyImage_read with image assets/tux.");
        result = false;
    }
    else
    {
        IM_SUCCESS("verifyImage_read with image assets/tux.");
    }

    if(!verifyImage_write("assets/smallTest.qoi"))
    {
        IM_FAIL("verifyImage_write with image assets/smallTest.");
        result = false;
    }
    else
    {
        IM_SUCCESS("verifyImage_write with image assets/smallTest.");
    }

    if(!verifyImage_write("assets/black.qoi"))
    {
        IM_FAIL("verifyImage_write with image assets/black.");
        result = false;
    }
    else
    {
        IM_SUCCESS("verifyImage_write with image assets/black.");
    }

    if(!verifyImage_write("assets/tux.qoi"))
    {
        IM_FAIL("verifyImage_write with image assets/tux.");
        result = false;
    }
    else
    {
        IM_SUCCESS("verifyImage_write with image assets/tux.");
    }

    return result;
}

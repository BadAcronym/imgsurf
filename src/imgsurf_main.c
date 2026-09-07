#include "imgsurf_main.h"

#include "pd_print_macros.h"
#include "pd_path.h"

bool same_pixel
(
    pixel pixel1,
    pixel pixel2
){
    return (pixel1.red  == pixel2.red  && pixel1.green == pixel2.green &&
            pixel1.blue == pixel2.blue && pixel1.alpha == pixel2.alpha);
}

f_internal void findFormat
(
    const char *path,
    uint8_t    *format
){
    uint32_t dot = 0;

    uint32_t i = 0;
    while(path[i] != '\0')
    {
        if(path[i] == '.')
        {
            dot = i;
        }
        ++i;
    }

    if(dot == 0)
    {
        PD_ERROR("path '%s' does not look like a file.", path);
        return;
    }

    if(dot + 3 > i || i > dot + 4)
    {
        PD_ERROR("file extension '%s' does not look valid.", path + dot);
        return;
    }

    if(path[dot + 1] == 'p' && path[dot + 2] == 'n' && path[dot + 3] == 'g')
    {
        *format = IM_FILE_PNG;
    }
    else if(path[dot + 1] == 'b' && path[dot + 2] == 'm' && path[dot + 3] == 'p')
    {
        *format = IM_FILE_BMP;
    }
    else if(path[dot + 1] == 'w' && path[dot + 2] == 'e' &&
            path[dot + 3] == 'b' && path[dot + 4] == 'p')
    {
        *format = IM_FILE_WEBP;
    }
    else if(path[dot + 1] == 'a' && path[dot + 2] == 'v' &&
            path[dot + 3] == 'i' && path[dot + 4] == 'f')
    {
        *format = IM_FILE_AVIF;
    }
    else if(path[dot + 1] == 'q' && path[dot + 2] == 'o' && path[dot + 3] == 'i')
    {
        *format = IM_FILE_QOI;
    }
    else if(path[dot + 1] == 'j' && path[dot + 2] == 'x' && path[dot + 3] == 'l')
    {
        *format = IM_FILE_JXL;
    }
    else
    {
        PD_ERROR("invalid file extension in path: '%s'.", path);
        return;
    }
}

f_internal bool verifyArgs
(
    uint8_t format,
    uint8_t channels,
    uint8_t bitdepth
){
    if(format == UINT8_MAX)
    {
        PD_ERROR("file format unsupported. Use .qoi/.png/.bmp/.webp/.avif/.jxl");
        return false;
    }

    if(!bitdepth)
    {
        PD_ERROR("bitdepth cannot be null.");
        return false;
    }

    if(channels > IM_CHANNELS_MAX)
    {
        PD_ERROR("invalid colour channels specified.");
        return false;
    }
    return true;
}

uint8_t* imLoadFile
(
    const char *path,
    uint32_t   *width,
    uint32_t   *height,
    uint8_t    channels,
    uint8_t    bitdepth
){
    uint8_t *image = 0;
    uint8_t format = UINT8_MAX;

    findFormat(path, &format);
    if(!verifyArgs(format, channels, bitdepth))
    {
        return 0;
    }

    StringView path_sv = cstr_sv(path);
    uint8_t    code    = pdVerifyPath(path_sv);
    if(code == PD_TYPE_ERROR)
    {
        PD_ERROR("path '%s' is not valid.", path);
        return 0;
    }
    if(code == PD_TYPE_DIRECTORY)
    {
        PD_ERROR("path '%s' is a directory.", path);
        return 0;
    }
    else if(code != PD_TYPE_FILE)
    {
        PD_ERROR("verifying the path '%s' has failed.", path);
        return 0;
    }

    FILE *file = fopen(path, "rb");
    if(!file)
    {
        PD_ERROR("could not open file '%s' for reading.", path);
        return 0;
    }

    switch(format)
    {
        case IM_FILE_QOI:
        {
            if(bitdepth != 8)
            {
                PD_ERROR("only a bitdepth of 8 is supported by QOI.");
                fclose(file);
                return 0;
            }

            image = loadQOI(file, width, height, channels);

            break;
        }
        case IM_FILE_PNG:
        {
            if(bitdepth > 2 && bitdepth != 4 && bitdepth != 8 && bitdepth != 16)
            {
                PD_ERROR("only bitdepths of 1, 2, 4, 8 or 16 are supported by PNG.");
                fclose(file);
                return 0;
            }

            image = loadPNG(file, width, height, channels);

            break;
        }
        case IM_FILE_BMP:
        {
            image = loadBMP(file, width, height, channels);
            break;
        }
        case IM_FILE_WEBP:
        {
            PD_WARN("format WEBP not implemented yet.");
            fclose(file);
            return 0;
        }
        case IM_FILE_AVIF:
        {
            PD_WARN("format AVIF not implemented yet.");
            fclose(file);
            return 0;
        }
        case IM_FILE_JXL:
        {
            PD_WARN("format JXL not implemented yet.");
            fclose(file);
            return 0;
        }
        default:
        {
            PD_ERROR("unknown format; unsupported.");
            fclose(file);
            return 0;
        }
    }

    fclose(file);
    return image;
}

uint8_t* imLoadPtr
(
    FILE       *file,
    uint8_t    fileFormat,
    uint32_t   *width,
    uint32_t   *height,
    uint8_t    channels,
    uint8_t    bitdepth
){
    if(fileFormat == IM_FILE_QOI)
    {
        if(bitdepth != 8)
        {
            PD_ERROR("only a bitdepth of 8 is supported by QOI.");
            return 0;
        }
        return loadQOI(file, width, height, channels);
    }
    else if(fileFormat == IM_FILE_PNG)
    {
        return loadPNG(file, width, height, channels);
    }
    else if(fileFormat == IM_FILE_BMP)
    {
        return loadBMP(file, width, height, channels);
    }

    PD_ERROR("format unsupported or not implemented yet. Try QOI/PNG/BMP.");
    return 0;
}

bool imWriteFile
(
    const char *path,
    void       *data,
    uint32_t   width,
    uint32_t   height,
    uint8_t    channels,
    uint8_t    bitdepth,
    uint8_t    fileFormat
){
    if(!data)
    {
        PD_ERROR("data cannot be null.");
        return false;
    }

    uint8_t readFileFormat = UINT8_MAX;
    findFormat(path, &readFileFormat);
    if(!verifyArgs(readFileFormat, channels, bitdepth))
    {
        return 0;
    }

    FILE *file = fopen(path, "wb");
    if(!file)
    {
        PD_ERROR("could not open the file '%s' for writing", path);
        return false;
    }

    switch(fileFormat)
    {
        case IM_FILE_QOI:
        {
            if(bitdepth != 8)
            {
                PD_ERROR("only a bitdepth of 8 is supported by QOI");
                fclose(file);
                return false;
            }

            if(!writeQOI(file, data, width, height, channels))
            {
                PD_ERROR("couldn't write .qoi file.");
                fclose(file);
                return false;
            }

            break;
        }
        case IM_FILE_PNG:
        {
            if(bitdepth > 2 && bitdepth != 4 && bitdepth != 8 && bitdepth != 16)
            {
                PD_ERROR("only bitdepths of 1, 2, 4, 8 or 16 are supported by PNG.");
                fclose(file);
                return false;
            }

            if(!writePNG(file, data, width, height, channels))
            {
                PD_ERROR("couldn't write .png file.");
                fclose(file);
                return false;
            }

            break;
        }
        case IM_FILE_BMP:
        {
            if(!writeBMP(file, data, width, height, channels))
            {
                PD_ERROR("couldn't write .bmp file.");
                fclose(file);
                return false;
            }

            break;
        }
        case IM_FILE_WEBP:
        {
            PD_WARN("format WEBP not implemented yet.");
            fclose(file);
            return false;
        }
        case IM_FILE_AVIF:
        {
            PD_WARN("format AVIF not implemented yet.");
            fclose(file);
            return false;
        }
        case IM_FILE_JXL:
        {
            PD_WARN("format JXL not implemented yet.");
            fclose(file);
            return false;
        }
        default:
        {
            PD_ERROR("unknown format; unsupported.");
            fclose(file);
            return false;
        }
    }

    fclose(file);
    return true;
}

void imWritePtr
(
    FILE     *file,
    void     *data,
    uint8_t  fileFormat,
    uint32_t width,
    uint32_t height,
    uint8_t  channels,
    uint8_t  bitdepth
){
    if(fileFormat == IM_FILE_QOI)
    {
        if(!writeQOI(file, data, width, height, channels))
        {
            PD_ERROR("imWritePtr failed to write QOI file to pointer.");
        }
        return;
    }
    else if(fileFormat == IM_FILE_PNG)
    {
        if(!writePNG(file, data, width, height, channels))
        {
            PD_ERROR("imWritePtr failed to write PNG file to pointer.");
        }
        return;
    }
    else if(fileFormat == IM_FILE_BMP)
    {
        if(!writeBMP(file, data, width, height, channels))
        {
            PD_ERROR("imWritePtr failed to write BMP file to pointer.");
        }
        return;
    }

    PD_ERROR("format not implemented yet. Try QOI/PNG/BMP.");
}

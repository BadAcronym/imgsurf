#include "imgsurf_load.h"
#include "linux_imgsurf_platform.h"

#include <sys/stat.h>

//TODO: expand paths to absolute location
uint8_t imgsurf_verifyPath
(
    const char *path
){
    struct stat pathInfo;

    if(stat(path, &pathInfo))
    {
        return IMGSURF_TYPE_ERROR;
    }

    if(S_ISDIR(pathInfo.st_mode))
    {
        return IMGSURF_TYPE_DIRECTORY;
    }

    if(S_ISREG(pathInfo.st_mode))
    {
        return IMGSURF_TYPE_FILE;
    }

    return IMGSURF_TYPE_OTHER;
}

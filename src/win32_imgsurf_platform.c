#include "imgsurf_load.h"
#include "linux_imgsurf_platform.h"

#include <sys/stat.h>

uint8_t imgsurf_verifyPath
(
    const char *path
){
    struct _stat pathInfo;

    if(_stat(path, &pathInfo))
    {
        return IMGSURF_TYPE_ERROR;
    }

    if(_S_IFDIR & pathInfo.st_mode)
    {
        return IMGSURF_TYPE_DIRECTORY;
    }

    if(_S_IFREG & pathInfo.st_mode)
    {
        return IMGSURF_TYPE_FILE;
    }

    return IMGSURF_TYPE_OTHER;
}

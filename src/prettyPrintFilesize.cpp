#define _CRT_SECURE_NO_WARNINGS
#include "prettyPrintFilesize.hpp"
#include <cstdio>

static const char * getUnitForSize(bla::s64 fsize)
{
    if(fsize < 1024)
        return "bytes";

    fsize /= 1024;
    if(fsize < 1024)
        return "KiB";

    fsize /= 1024;
    if(fsize < 1024)
        return "MiB";

    fsize /= 1024;
    if(fsize < 1024)
        return "GiB";

    fsize /= 1024;
    if(fsize < 1024)
        return "TiB";

    fsize /= 1024;
    if(fsize < 1024)
        return "PiB";

    fsize /= 1024;
    if(fsize < 1024)
        return "EiB";

    return "ZiB";
}

std::string prettyPrintFilesize(bla::s64 fsize)
{
    if(fsize < 0)
        return "negative-file-size";

    if(fsize == 1)
        return "1 byte";

    if(fsize < 1024)
        return std::to_string(fsize) + " bytes";

    char buff[50];
    const char * unit = getUnitForSize(fsize);

    while(fsize >= (1024 * 1024))
        fsize /= 1024;

    sprintf(buff, "%.3f %s", fsize / 1024.0, unit);
    return buff;
}

#include "blaDefines.hpp"
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

static double adjustSize(bla::s64 bytes)
{
    if(bytes < (1ll << 10)) return bytes / (double)(1ll << 0);
    if(bytes < (1ll << 20)) return bytes / (double)(1ll << 10);
    if(bytes < (1ll << 30)) return bytes / (double)(1ll << 20);
    if(bytes < (1ll << 40)) return bytes / (double)(1ll << 30);
    if(bytes < (1ll << 50)) return bytes / (double)(1ll << 40);
    if(bytes < (1ll << 60)) return bytes / (double)(1ll << 50);
    return (bytes >> 10) / (double)(1ll << 50);
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
    const double value = adjustSize(fsize);
    const char * unit = getUnitForSize(fsize);
    sprintf(buff, "%.3f %s", value, unit);
    return buff;
}

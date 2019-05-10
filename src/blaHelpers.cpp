#include "blaDefines.hpp"
#include "blaHelpers.hpp"
#include <FL/fl_draw.H>

int bla_text_width(const char * str)
{
    int w = 0, h = 0;
    fl_measure(str, w, h, 0);
    return w;
}

int bla_text_height(const char * str)
{
    int w = 0, h = 0;
    fl_measure(str, w, h, 0);
    return h;
}

bool isDisplayChar(bla::byte byte)
{
    //printable ascii range is [0x20, 0x7f)
    return byte >= 0x20 && byte < 0x7f;
}

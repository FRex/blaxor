#include "blaDefines.hpp"
#include "blaHelpers.hpp"
#include <FL/fl_draw.H>
#include <string>
#include <cassert>

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

int bla_text_width_charcount(int charcount)
{
    const std::string str(charcount, '@');
    assert(charcount == static_cast<int>(str.length()));
    return bla_text_width(str.c_str());
}

bool isDisplayChar(bla::byte byte)
{
    //printable ascii range is [0x20, 0x7f)
    return byte >= 0x20 && byte < 0x7f;
}

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

int bla_text_width_charcount(int charcount)
{
    return bla_text_width("@") * charcount;
}

bool isDisplayChar(bla::byte byte)
{
    //printable ascii range is [0x20, 0x7f)
    return byte >= 0x20 && byte < 0x7f;
}

void byteToBinaryString(bla::byte b, char * out)
{
    for(int i = 7; i >= 0; --i)
        out[7 - i] = ((1 << i) & b) ? '1' : '0';

    out[8] = '\0';
}

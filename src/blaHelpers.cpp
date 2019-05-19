#include "blaDefines.hpp"
#include "blaHelpers.hpp"
#include <FL/fl_draw.H>
#include "BlaFile.hpp"
#include "utf8dfa.hpp"
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

bool isUtf8SequenceHere(BlaFile& file, bla::s64 start, int * back)
{
    bla::u32 codepoint = 0u;
    for(int i = 0; i < 4; ++i)
    {
        if(hasUtf8Here(file, start - i, i + 1, &codepoint))
        {
            if(back)
                *back = i;

            return true;
        }//if
    }//for

    return false;
}

bool hasUtf8Here(BlaFile& file, bla::s64 s, int bytesneeded, bla::u32 * codepoint)
{
    bla::u32 state = 0;
    for(int i = 0; i < 4; ++i)
    {
        if(!file.goodIndex(s + i))
            return false;

        if(utf8dfa::decode(&state, codepoint, file.getByte(s + i)) == utf8dfa::kAcceptState)
            return (i + 1) >= bytesneeded;
    }

    return false;
}

int utf8CodepointLen(const char * str)
{
    assert(str);
    if(!str)
        return 0;

    int ret = 0;
    bla::u32 state = utf8dfa::kAcceptState;
    bla::u32 codepoint =0u;
    while(*str != '\0')
    {
        utf8dfa::decode(&state, &codepoint, static_cast<bla::u8>(*str));
        if(state == utf8dfa::kAcceptState)
            ++ret;

        ++str;
    }//while

    return ret;
}

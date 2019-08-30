#include "binaryParse.hpp"
#include "utf8dfa.hpp"

bla::u32 little_u32(void * data)
{
    bla::u32 ret = 0u;
    const unsigned char * b = (const unsigned char *)data;
    ret = (ret | b[3]) << 8;
    ret = (ret | b[2]) << 8;
    ret = (ret | b[1]) << 8;
    ret = (ret | b[0]);
    return ret;
}

bla::s32 little_s32(void * data)
{
    return static_cast<bla::s32>(little_u32(data));
}

bla::u32 big_u32(void * data)
{
    bla::u32 ret = 0u;
    const unsigned char * b = (const unsigned char *)data;
    ret = (ret | b[0]) << 8;
    ret = (ret | b[1]) << 8;
    ret = (ret | b[2]) << 8;
    ret = (ret | b[3]);
    return ret;
}

bla::s32 big_s32(void * data)
{
    return static_cast<bla::s32>(big_u32(data));
}

bla::u16 little_u16(void * data)
{
    bla::u16 ret = 0u;
    const unsigned char * b = (const unsigned char *)data;
    ret = (ret | b[1]) << 8;
    ret = (ret | b[0]);
    return ret;
}

bla::s16 little_s16(void * data)
{
    return static_cast<bla::s16>(little_u16(data));
}

bla::u16 big_u16(void * data)
{
    bla::u16 ret = 0u;
    const unsigned char * b = (const unsigned char *)data;
    ret = (ret | b[0]) << 8;
    ret = (ret | b[1]);
    return ret;
}

bla::s16 big_s16(void * data)
{
    return static_cast<bla::s16>(big_u16(data));
}

bla::u8 endianless_u8(void * data)
{
    const unsigned char * b = (const unsigned char *)data;
    return *b;
}

bla::s8 endianless_s8(void * data)
{
    return static_cast<bla::s8>(endianless_u8(data));
}

static bool isUtf8ContinuationChar(char c)
{
    const int cc = static_cast<unsigned char>(c);
    return 2 == (cc >> 6);
}

//TODO: asserts for UTF-8 consistency just in case?
std::string stripOneUtf8FromEnd(std::string s)
{
    //pop all the utf-8 continuation bytes off first, maybe none if last char is ascii...
    while(s.length() > 0u && isUtf8ContinuationChar(s.back()))
        s.pop_back();

    //...now pop one more off, the utf-8 starting byte, or the ascii
    if(s.length() > 0u)
        s.pop_back();

    return s;
}

static bla::byte asciiByteLower(bla::byte c)
{
    if('A' <= c && c <= 'Z')
        return c | 32;

    return c;
}

static bool sameMemoryAsciiNoCase(const void * a, const void * b, size_t c)
{
    const unsigned char * aa = static_cast<const unsigned char*>(a);
    const unsigned char * bb = static_cast<const unsigned char*>(b);
    while(c)
    {
        if(asciiByteLower(*aa) != asciiByteLower(*bb))
            return false;

        ++aa;
        ++bb;
        --c;
    }//while

    return true;
}

//TODO: for 1 char needle use memchr
const void * myMemmemNoAsciiCase(const void * h, size_t hs, const void * n, size_t ns)
{
    if(ns == 0u)
        return h;

    if(ns > hs)
        return 0x0;

    const unsigned char * hh = static_cast<const unsigned char*>(h);
    const unsigned char firstbyte = 32 | *static_cast<const unsigned char*>(n);
    //bit 5 (1 << 5 = 32) is bit that is set in ascii lowercase letter (same bit pattern with it unset = uppercase letter)
    //so we set that bit in first byte of needle, and set it in each byte of haystack we inspect no matter what, to
    //avoid ifs/cmp/branching that a strict function to lower an ascii letter byte that only sets bit 5 on [A..Z] has
    //once a potential match of needle is found that way, we use proper strict compare that only sets bit 5 on [A..Z]
    //benchmarking this: in debug 6-7x speed up (c.a. 60 -> 400 MiB/s), in release around 1.5x (c.a. 900 -> 1600 MiB/s)
    //well worth it in both: in release free speed up, in debug to not freeze on searching when testing while coding
    while(ns <= hs)
    {
        if((32 | *hh) == firstbyte && sameMemoryAsciiNoCase(hh, n, ns))
            return hh;

        --hs;
        ++hh;
    }//while

    return 0x0;
}

void utf8ToUtf16LEbytes(const char * s, std::vector<bla::byte>& out)
{
    unsigned state = utf8dfa::kAcceptState;
    unsigned codepoint = 0u;
    out.clear();

    while(*s)
    {
        utf8dfa::decode(&state, &codepoint, static_cast<bla::u8>(*s));
        ++s;
        if(utf8dfa::kAcceptState != state)
            continue;

        if(codepoint <= 0xffff)
        {
            out.push_back((codepoint >> 0) & 255);
            out.push_back((codepoint >> 8) & 255);
        }
        else
        {
            const unsigned c1 = 0xd7c0 + (codepoint >> 10);
            out.push_back((c1 >> 0) & 255);
            out.push_back((c1 >> 8) & 255);

            const unsigned c2 = 0xdc00 + (codepoint & 0x3ff);
            out.push_back((c2 >> 0) & 255);
            out.push_back((c2 >> 8) & 255);
        }
    }//while
}

const void * myMemmem(const void * h, size_t hs, const void * n, size_t ns)
{
    if(ns == 0u)
        return h;

    const unsigned char * hh = static_cast<const unsigned char*>(h);
    const unsigned char firstbyte = *static_cast<const unsigned char*>(n);
    while(ns <= hs)
    {
        if(*hh == firstbyte && 0 == std::memcmp(hh, n, ns))
            return hh;

        --hs;
        ++hh;
    }//while

    return 0x0;
}

const void * smallerFullPointer(const void * a, const void * b)
{
    if(a == 0x0)
        return b;

    if(b == 0x0)
        return a;

    //by now we know neither is null
    return (a < b) ? a : b;
}

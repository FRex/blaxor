#include "binaryParse.hpp"

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

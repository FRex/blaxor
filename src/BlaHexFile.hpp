#pragma once

namespace bla {
typedef unsigned long long i64;
}//bla

static_assert(sizeof(bla::i64) == 8, "i64 is not 8 bytes");

class BlaHexFile
{
public:
    virtual bla::i64 filesize() const
    {
        return 1234;
    }

    virtual unsigned char getByte(bla::i64 pos) const
    {
        return static_cast<unsigned char>(pos & 0xff);
    }

};

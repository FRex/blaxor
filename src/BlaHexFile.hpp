#pragma once
#include <vector>
#include "BlaNonCopyable.hpp"

namespace bla {
typedef unsigned long long s64;
}//bla

static_assert(sizeof(bla::s64) == 8, "s64 is not 8 bytes");

class BlaHexFile : BlaNonCopyable
{
public:
    bool open(const char * fname);
    bla::s64 filesize() const;
    unsigned char getByte(bla::s64 pos) const;

private:
    std::vector<unsigned char> m_buff;

};

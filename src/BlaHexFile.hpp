#pragma once
#include <vector>

namespace bla {
typedef unsigned long long i64;
}//bla

static_assert(sizeof(bla::i64) == 8, "i64 is not 8 bytes");

class BlaHexFile
{
public:
    bool open(const char * fname);
    bla::i64 filesize() const;
    unsigned char getByte(bla::i64 pos) const;

private:
    std::vector<unsigned char> m_buff;

};

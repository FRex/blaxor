#pragma once
#include <vector>
#include <cstdio>
#include "BlaNonCopyable.hpp"
#include "blatype.hpp"

class BlaHexFile : BlaNonCopyable
{
public:
    ~BlaHexFile();
    bool open(const char * fname);
    void close();
    bla::s64 filesize() const;
    bla::byte getByte(bla::s64 pos);
    bla::s64 readcount() const;

private:
    bool onFileOpen();

    std::FILE * m_file = 0x0;
    bla::s64 m_filesize = 0;
    bla::s64 m_readcount = 0;

};

#pragma once
#include <vector>
#include <cstdio>
#include "BlaNonCopyable.hpp"
#include "blatype.hpp"

class BlaFile : BlaNonCopyable
{
public:
    ~BlaFile();
    bool open(const char * fname);
    void close();
    bla::s64 filesize() const;
    bla::byte getByte(bla::s64 pos);
    bla::s64 readcount() const;
    bool goodIndex(bla::s64 idx) const;
    const bla::byte * getPtr() const;

private:

    //Windows file handle and mem mapping handle:
    void * m_winfile = 0x0;
    void * m_maphandle = 0x0;

    //linux file dscriptor
    int m_fd = 0; //0 so if(m_fd) fails if no open file, CAREFUL since 0 is stdin fd!!

    //all platforms
    void * m_mapptr = 0x0;
    bla::s64 m_filesize = 0;
    bla::s64 m_readcount = 0;

};

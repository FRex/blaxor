#include "BlaHexFile.hpp"
#include <fstream>

bool BlaHexFile::open(const char * fname)
{
    std::ifstream file(fname, std::ios::binary);
    m_buff.clear();
    if(!file.is_open())
        return false;

    m_buff.resize(1024 * 1024);
    file.read(reinterpret_cast<char*>(m_buff.data()), m_buff.size());
    const bla::s64 readcount = file.gcount();
    m_buff.resize(readcount);
    return true;
}

bla::s64 BlaHexFile::filesize() const
{
    return static_cast<bla::s64>(m_buff.size());
}

unsigned char BlaHexFile::getByte(bla::s64 pos) const
{
    if(pos < static_cast<bla::s64>(m_buff.size()))
        return m_buff[pos];

    return 0xff;
}

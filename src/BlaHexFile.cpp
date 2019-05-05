#define _CRT_SECURE_NO_WARNINGS
#include "BlaHexFile.hpp"
#include "osSpecific.hpp"

BlaHexFile::~BlaHexFile()
{
    close();
}

bool BlaHexFile::open(const char * fname)
{
    close();
    m_file = _wfopen(utf8ToUtf16(fname).c_str(), L"rb");
    if(!m_file)
        return false;

    return onFileOpen();
}

void BlaHexFile::close()
{
    m_filesize = 0;
    m_readcount = 0;
    if(m_file)
        std::fclose(m_file);
}

bla::s64 BlaHexFile::filesize() const
{
    return m_filesize;
}

static inline bool goodindex(bla::s64 pos, bla::s64 fsize)
{
    return (pos >= 0) && (pos < fsize);
}

unsigned char BlaHexFile::getByte(bla::s64 pos)
{
    ++m_readcount;
    if(!goodindex(pos, m_filesize))
        return 0xff;

    unsigned char ret;
    if(0 == _fseeki64(m_file, pos, SEEK_SET))
        if(1 == fread(&ret, 1, 1, m_file))
            return ret;

    return 0xff;
}

bla::s64 BlaHexFile::readcount() const
{
    return m_readcount;
}

bool BlaHexFile::onFileOpen()
{
    if(0 == _fseeki64(m_file, 0, SEEK_END))
    {
        const bla::s64 s = static_cast<bla::s64>(_ftelli64(m_file));
        if(s >= 0)
        {
            m_filesize = s;
        }
        else
        {
            close();
            return false;
        }
    }
    else
    {
        close();
        return false;
    }

    return true;
}

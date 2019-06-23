#include "blaDefines.hpp"
#include "BlaFile.hpp"
#include "osSpecific.hpp"

BlaFile::~BlaFile()
{
    close();
}

static std::FILE * my_fopen_utf8_rb(const char * fname)
{
#ifdef BLA_WINDOWS
    return _wfopen(utf8ToUtf16(fname).c_str(), L"rb");
#else
    return std::fopen(fname, "rb");
#endif
}

static int myfseek64(std::FILE * f, bla::s64 off, int from)
{
#ifdef BLA_WINDOWS
    return _fseeki64(f, off, from);
#else
    static_assert(sizeof(__off64_t) == 8, "fseeko64 isn't 64 bit?");
    return fseeko64(f, off, from);
#endif
}

static bla::s64 myftell64(std::FILE * f)
{
#ifdef BLA_WINDOWS
    return _ftelli64(f);
#else
    static_assert(sizeof(ftello64(f)) == 8, "ftello64 isn't 64 bit?");
    return ftello64(f);
#endif
}

bool BlaFile::open(const char * fname)
{
    close();
    m_file = my_fopen_utf8_rb(fname);
    if(!m_file)
        return false;

    return onFileOpen();
}

void BlaFile::close()
{
    m_filesize = 0;
    m_readcount = 0;
    if(m_file)
        std::fclose(m_file);

    m_file = 0x0;
}

bla::s64 BlaFile::filesize() const
{
    return m_filesize;
}

bla::byte BlaFile::getByte(bla::s64 pos)
{
    ++m_readcount;
    if(!goodIndex(pos))
        return 0xff;

    bla::byte ret;
    if(0 == myfseek64(m_file, pos, SEEK_SET))
        if(1 == fread(&ret, 1, 1, m_file))
            return ret;

    return 0xff;
}

bla::s64 BlaFile::readcount() const
{
    return m_readcount;
}

bool BlaFile::goodIndex(bla::s64 idx) const
{
    return (0 <= idx) && (idx < m_filesize);
}

bool BlaFile::onFileOpen()
{
    if(0 == myfseek64(m_file, 0, SEEK_END))
    {
        const bla::s64 s = static_cast<bla::s64>(myftell64(m_file));
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

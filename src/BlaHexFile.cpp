#include "blaDefines.hpp"
#include "BlaHexFile.hpp"
#include "osSpecific.hpp"

BlaHexFile::~BlaHexFile()
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
    static_assert(sizeof(__off64_t) == 8, "__off64_t isn't 64 bit");
    return fseeko64(f, off, from);
#endif
}


bool BlaHexFile::open(const char * fname)
{
    close();
    m_file = my_fopen_utf8_rb(fname);
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
    if(0 == myfseek64(m_file, pos, SEEK_SET))
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
    if(0 == myfseek64(m_file, 0, SEEK_END))
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

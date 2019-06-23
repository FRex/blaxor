#include "blaDefines.hpp"
#include "BlaFile.hpp"
#include "osSpecific.hpp"
#include <Windows.h>

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
    m_winfile = CreateFileW(utf8ToUtf16(fname).c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if(m_winfile == INVALID_HANDLE_VALUE)
        return false;

    LARGE_INTEGER lisize;
    if(0 == GetFileSizeEx(m_winfile, &lisize))
    {
        close();
        return false;
    }//0 == get file size ex

    m_filesize = lisize.QuadPart;
    return true;
}

void BlaFile::close()
{
    m_filesize = 0;
    m_readcount = 0;
    if(m_winfile && m_winfile != INVALID_HANDLE_VALUE)
        CloseHandle(m_winfile);

    m_winfile = 0x0;
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

    LARGE_INTEGER goal;
    goal.QuadPart = pos;
    if(!SetFilePointerEx(m_winfile, goal, NULL, FILE_BEGIN))
        return 0xff;

    bla::byte ret = 0xff;
    DWORD readcount;
    if(!ReadFile(m_winfile, &ret, 1, &readcount, NULL))
        return 0xff;

    if(readcount == 1)
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

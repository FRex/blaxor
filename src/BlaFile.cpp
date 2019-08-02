#include "blaDefines.hpp"
#include "BlaFile.hpp"
#include "osSpecific.hpp"

#ifdef BLA_WINDOWS
#include <Windows.h>
#endif //BLA_WINDOWS

BlaFile::~BlaFile()
{
    close();
}

const bla::s64 kFilesizeSafeToMemMap = 100 * 1024 * 1024;

bool BlaFile::open(const char * fname)
{
    close();

#ifdef BLA_WINDOWS
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

    if(m_filesize <= kFilesizeSafeToMemMap)
    {
        m_maphandle = CreateFileMappingW(m_winfile, NULL, PAGE_READONLY, 0, 0, NULL);
        if(m_maphandle)
            m_mapptr = MapViewOfFile(m_maphandle, FILE_MAP_READ, 0, 0, 0);
    }

    return true;
#endif //BLA_WINDOWS

    return false;
}

void BlaFile::close()
{
    m_filesize = 0;
    m_readcount = 0;

#ifdef BLA_WINDOWS
    if(m_mapptr)
        UnmapViewOfFile(m_mapptr);

    if(m_maphandle)
        CloseHandle(m_maphandle);

    if(m_winfile && m_winfile != INVALID_HANDLE_VALUE)
        CloseHandle(m_winfile);

    m_mapptr = 0x0;
    m_maphandle = 0x0;
    m_winfile = 0x0;
#endif //BLA_WINDOWS

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

#ifdef BLA_WINDOWS
    if(m_mapptr)
        return static_cast<bla::byte*>(m_mapptr)[pos];

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
#endif //BLA_WINDOWS

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

const bla::byte * BlaFile::getPtr() const
{
    return static_cast<bla::byte*>(m_mapptr);
}

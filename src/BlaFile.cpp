#include "blaDefines.hpp"
#include "BlaFile.hpp"
#include "osSpecific.hpp"

#ifdef BLA_WINDOWS
#include <Windows.h>
#endif //BLA_WINDOWS

#ifdef BLA_LINUX
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
static_assert(sizeof(off_t) == 8, "sizeof(off_t) != 8");
#endif //BLA_LINUX

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

#ifdef BLA_LINUX
    m_fd = ::open(fname, O_RDONLY);
    if(m_fd == -1)
    {
        m_fd = 0;
        fprintf(stderr, "FAIL: call ::open('%s', O_RDONLY); failed, errno = %d", fname, errno);
        return false;
    }//m fd == -1

    struct stat mystat;
    if(fstat(m_fd, &mystat) != 0)
    {
        fprintf(stderr, "FAIL: call fstat(%d = open('%s', O_RDONLY), &mystat); failed, errno = %d",
            m_fd, fname, errno);

        close(); //call after above since this ::close inside can fail and set errno too!
        return false;
    }//if fstat m_fd mystat != 0

    m_filesize = static_cast<bla::s64>(mystat.st_size);
    return true;
#endif //BLA_LINUX

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

#ifdef BLA_LINUX
    if(m_fd)
    {
        const int r = ::close(m_fd);
        if(r)
            fprintf(stderr, "FAIL: call ::close(m_fd); failed, errno = %d", errno);

        m_fd = 0; //leaking FD but above never happens since we read only? TODO: 'fix' later?
    } //if m fd
#endif //BLA_LINUX

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

#ifdef BLA_LINUX
    if(!m_fd)
        return 0xff;

    bla::byte ret = 0xff;
    const ssize_t ok = pread(m_fd, &ret, 1, static_cast<off_t>(pos));
    if(ok == 1)
        return ret;

    if(ok == -1)
        fprintf(stderr, "FAIL: call pread(%d, ptr, 1, static_cast<off_t>(%ldd)) failed, errno = %d",
            m_fd, pos, errno);
#endif //BLA_LINUX

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

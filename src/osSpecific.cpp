#include "osSpecific.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <Windows.h>
#define WIN32
#include <FL/x.H>

namespace {

class FileDropHandling
{
public:
    HWND win = 0x0;
    LONG_PTR originalWinCallback = 0x0;
    FileDropCallback callback;
    void * udata = 0x0;
};

static std::vector<FileDropHandling> callbacks;

static std::vector<FileDropHandling>::iterator findHandlingForWindow(HWND win)
{
    for(auto it = callbacks.begin(); it != callbacks.end(); ++it)
        if(it->win == win)
            return it;

    return callbacks.end();
}

}//unnamed namespace

LONG_PTR originalsfmlcallback = 0x0;

LRESULT CALLBACK mycallback(HWND handle, UINT message, WPARAM wParam, LPARAM lParam)
{
    const auto h = findHandlingForWindow(handle);
    if(message == WM_DROPFILES)
    {
        HDROP hdrop = reinterpret_cast<HDROP>(wParam);
        const UINT filescount = DragQueryFile(hdrop, 0xFFFFFFFF, NULL, 0);
        for(UINT i = 0; i < filescount; ++i)
        {
            const UINT bufsize = DragQueryFile(hdrop, i, NULL, 0);
            std::wstring str;
            str.resize(bufsize + 1);
            if(DragQueryFileW(hdrop, i, &str[0], bufsize + 1))
                h->callback(h->udata, str.c_str());
        }

        DragFinish(hdrop);
    }//if WM_DROPFILES

    return CallWindowProcW(reinterpret_cast<WNDPROC>(h->originalWinCallback), handle, message, wParam, lParam);
}

void enableFileDropOnWindow(Fl_Window * win, FileDropCallback callback, void * udata)
{
    HWND handle = fl_xid(win);
    DragAcceptFiles(handle, TRUE);

    auto h = findHandlingForWindow(handle);
    if(h != callbacks.end())
        callbacks.erase(h);

    FileDropHandling han;
    han.win = handle;
    han.callback = callback;
    han.udata = udata;
    LONG_PTR origcb = SetWindowLongPtrW(handle, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(mycallback));
    han.originalWinCallback = origcb;

    callbacks.push_back(han);
}

std::string utf16ToUtf8(const wchar_t * fname)
{
    const size_t utf8len = wcslen(fname) * 3 + 10;
    std::vector<char> ret;
    ret.resize(utf8len);
    WideCharToMultiByte(CP_UTF8, 0, fname, -1, ret.data(), utf8len, NULL, NULL);
    return ret.data();
}

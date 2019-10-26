#pragma once
#include "blaDefines.hpp"
#include <string>

class Fl_Window;

typedef void (*FileDropCallback)(void *, const char *, int);

void enableFileDropOnWindow(Fl_Window * win, FileDropCallback callback, void * udata);

void maximizeWindow(Fl_Window * win);

//these two functions are windows only, for opening a file and so on
#ifdef BLA_WINDOWS
std::string utf16ToUtf8(const wchar_t * str);
std::wstring utf8ToUtf16(const char * str);
#endif

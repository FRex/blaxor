#pragma once
#include <string>

class Fl_Window;

typedef void (*FileDropCallback)(void *, const char *);

void enableFileDropOnWindow(Fl_Window * win, FileDropCallback callback, void * udata);

void maximizeWindow(Fl_Window * win);

std::string utf16ToUtf8(const wchar_t * str);
std::wstring utf8ToUtf16(const char * str);

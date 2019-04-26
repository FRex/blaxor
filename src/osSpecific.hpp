#pragma once
#include <string>

class Fl_Window;

typedef void (*FileDropCallback)(void *, const wchar_t *);

void enableFileDropOnWindow(Fl_Window * win, FileDropCallback callback, void * udata);

std::string utf16ToUtf8(const wchar_t * fname);


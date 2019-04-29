#define WIN32
#include <FL/Fl.H>
#include "BlaxorApp.hpp"

int wmain(int argc, wchar_t ** argv)
{
    BlaxorApp app;
    if(argc > 1)
        app.openFile(argv[1]);

    app.setupGui();
    return Fl::run();
}

//call out wmain with right args using subsystem:windows entry point
#include <Windows.h>

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, INT)
{
    int argc = 0;
    wchar_t ** argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if(!argv)
        return 1;

    const int ret = wmain(argc, argv);
    LocalFree(argv);
    return ret;
}

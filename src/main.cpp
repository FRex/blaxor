#include "blaDefines.hpp"
#include <FL/Fl.H>
#include "BlaxorApp.hpp"

static int my_utf8_main(int argc, char ** argv)
{
    BlaxorApp app;
    if(argc > 1)
        app.openFile(argv[1]);

    app.setupGui();
    return Fl::run();
}

#ifndef BLA_WINDOWS

int main(int argc, char ** argv)
{
    return my_utf8_main(argc, argv);
}

#else

/* for wcslen and WideCharToMultiByte */
#include <wchar.h>
#include <Windows.h>

int wmain(int argc, wchar_t ** argv)
{
    int i, retcode;
    char ** utf8argv = (char **)calloc(argc + 1, sizeof(char*));
    if(!utf8argv)
    {
        fputs("Error: calloc error in wmain\n", stderr);
        return 1;
    }

    retcode = 0;
    for(i = 0; i < argc; ++i)
    {
        const size_t utf8len = wcslen(argv[i]) * 3 + 10;
        utf8argv[i] = (char*)malloc(utf8len);
        if(!utf8argv[i])
        {
            retcode = 1;
            fputs("Error: malloc error in wmain\n", stderr);
            break;
        }
        WideCharToMultiByte(CP_UTF8, 0, argv[i], -1, utf8argv[i], utf8len, NULL, NULL);
    }

    if(retcode == 0)
        retcode = my_utf8_main(argc, utf8argv);

    for(i = 0; i < argc; ++i)
        free(utf8argv[i]);

    free(utf8argv);
    return retcode;
}

//call out wmain with right args using subsystem:windows entry point

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

#endif //BLA_WINDOWS


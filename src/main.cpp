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

#define WIN32
#include <FL/Fl.H>
#include <cstdio>
#include "BlaxorApp.hpp"

int main(int argc, char ** argv)
{
    if(argc < 2)
    {
        std::fprintf(stderr, "Usage: %s filename\n", argv[0]);
        return 1;
    }

    BlaxorApp app;
    if(!app.openFile(argv[1]))
    {
        std::fprintf(stderr, "Failed to open '%s'\n", argv[1]);
        return 2;
    }

    app.setupGui();
    return Fl::run();
}

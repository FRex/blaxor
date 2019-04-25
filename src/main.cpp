#define WIN32
#include <FL/Fl.H>
#include "BlaxorApp.hpp"

int main(int argc, char ** argv)
{
    BlaxorApp app;
    if(argc > 1)
        app.openFile(argv[1]);

    app.setupGui();
    return Fl::run();
}

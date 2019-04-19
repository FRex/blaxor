#define WIN32
#define _CRT_SECURE_NO_WARNINGS

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Slider.H>
#include "BlaHexDisplay.hpp"
#include "BlaHexFile.hpp"

static void mycallback(Fl_Widget * widget, void * data)
{
    Fl_Slider * sb = static_cast<Fl_Slider*>(widget);
    BlaHexDisplay * my = static_cast<BlaHexDisplay*>(data);

    my->setFirstDisplayedLine(sb->value());
    my->redraw();
}

int main(int argc, char ** argv)
{
    if(argc < 2)
    {
        std::fprintf(stderr, "Usage: %s filename\n", argv[0]);
        return 1;
    }

    const int w = 900;
    const int h = 600;
    const int scrollballw = 20;
    Fl_Double_Window win(w, h, argv[1]);
    Fl_Slider * sb = new Fl_Slider(w - scrollballw, 0, scrollballw, h);
    auto my = new BlaHexDisplay(0, 0, w - scrollballw, h);
    win.resizable(my);
    my->take_focus();
    BlaHexFile file;
    file.open(argv[1]);
    my->setFile(&file);
    my->recalculateMetrics();
    my->setLineScrollbar(sb);
    sb->callback(mycallback, my);
    win.show(); //win.show(argc, argv);
    return Fl::run();
}

#define WIN32
#define _CRT_SECURE_NO_WARNINGS

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Scrollbar.H>
#include "BlaHexDisplay.hpp"
#include "BlaHexFile.hpp"

static void mycallback(Fl_Widget * widget, void * data)
{
    Fl_Scrollbar * sb = static_cast<Fl_Scrollbar*>(widget);
    BlaHexDisplay * my = static_cast<BlaHexDisplay*>(data);
    my->m_startingline = sb->value();
    my->redraw();
}

int main(int argc, char ** argv)
{
    const int w = 800;
    const int h = 600;
    Fl_Double_Window win(w, h, "Ha!");
    Fl_Scrollbar * sb = new Fl_Scrollbar(w - 20, 0, 20, h);
    auto my = new BlaHexDisplay(0, 0, w - 20, h);
    BlaHexFile file;
    my->setFile(&file);
    sb->callback(mycallback, my);
    sb->bounds(0, 20);
    sb->slider_size(0.1);
    sb->linesize(1);
    win.show(); //win.show(argc, argv);
    return Fl::run();
}

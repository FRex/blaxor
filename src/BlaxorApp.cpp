#define WIN32
#define _CRT_SECURE_NO_WARNINGS
#include "BlaxorApp.hpp"
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Slider.H>
#include "BlaHexDisplay.hpp"
#include "BlaHexFile.hpp"
#include "prettyPrintFilesize.hpp"

static void mycallback(Fl_Widget * widget, void * data)
{
    Fl_Slider * sb = static_cast<Fl_Slider*>(widget);
    BlaHexDisplay * my = static_cast<BlaHexDisplay*>(data);

    my->setFirstDisplayedLine(static_cast<bla::s64>(sb->value()));
    my->ensureSelectionInView();
    my->redraw();
}

BlaxorApp::~BlaxorApp()
{
    delete m_win;
}

bool BlaxorApp::openFile(const char * fname)
{
    if(!m_file.open(fname))
        return false;

    m_wintitle = fname + std::string(": ") + prettyPrintFilesize(m_file.filesize());
    return true;
}

void BlaxorApp::setupGui()
{
    const int w = 900;
    const int h = 600;
    const int scrollballw = 20;
    m_win = new Fl_Double_Window(w, h, m_wintitle.c_str());
    Fl_Slider * sb = new Fl_Slider(w - scrollballw, 0, scrollballw, h);
    BlaHexDisplay * my = new BlaHexDisplay(0, 0, w - scrollballw, h);
    m_win->resizable(my);
    my->take_focus();
    my->setFile(&m_file);
    my->recalculateMetrics();
    my->setLineScrollbar(sb);
    sb->callback(mycallback, my);
    m_win->show(); //win->show(argc, argv);
}

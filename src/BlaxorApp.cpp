#define WIN32
#define _CRT_SECURE_NO_WARNINGS
#include "BlaxorApp.hpp"
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Slider.H>
#include <FL/Fl_Box.H>
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

static void update_label_cb(Fl_Widget * widget, void * data)
{
    BlaHexFile * file = static_cast<BlaHexFile*>(data);
    const bla::s64 rc = file->readcount();
    char buff[50];
    sprintf(buff, "total bytes read: %lld", rc);
    widget->copy_label(buff);
}

static void update_label_to(void * data)
{
    Fl_Box * box = static_cast<Fl_Box*>(data);
    box->do_callback();
    Fl::repeat_timeout(1.0, &update_label_to, data);
}

void BlaxorApp::setupGui()
{
    const int w = 900;
    const int h = 600;
    const int scrollballw = 20;
    const int boxh = 50;
    m_win = new Fl_Double_Window(w, h, m_wintitle.c_str());
    Fl_Box * box = new Fl_Box(0, 0, w, boxh);
    box->callback(&update_label_cb, &m_file);
    box->do_callback();
    Fl::add_timeout(1.0, &update_label_to, box);
    Fl_Slider * sb = new Fl_Slider(w - scrollballw, boxh, scrollballw, h - boxh);
    BlaHexDisplay * my = new BlaHexDisplay(0, boxh, w - scrollballw, h - boxh);
    m_win->resizable(my);
    my->take_focus();
    my->setFile(&m_file);
    my->recalculateMetrics();
    my->setLineScrollbar(sb);
    sb->callback(mycallback, my);
    m_win->show(); //win->show(argc, argv);
}

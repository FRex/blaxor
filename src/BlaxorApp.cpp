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
#include "osSpecific.hpp"

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
    if(m_win)
        m_win->label(m_wintitle.c_str());

    redrawAll();
    return true;
}

static std::vector<bla::s64> findAsciiInFile(BlaHexFile& file, const std::string& ascii)
{
    std::vector<bla::s64> ret;

    for(bla::s64 i = 0; i < file.filesize(); ++i)
    {
        bool match = true;
        for(int j = 0; j < ascii.size(); ++j)
        {
            if((i + j) >= file.filesize())
                return ret;

            if(ascii[j] != static_cast<int>(file.getByte(i + j)))
                match = false;

        }//for j

        if(match)
            ret.push_back(i);
    }//for i

    return ret;
}

bool BlaxorApp::openFile(const wchar_t * fname)
{
    if(!m_file.open(fname))
        return false;

    m_wintitle = utf16ToUtf8(fname) + std::string(": ") + prettyPrintFilesize(m_file.filesize());
    if(m_win)
        m_win->label(m_wintitle.c_str());

    redrawAll();
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

const double kBoxLabelUpdateTimeout = 1.0;
const int kScrollbarWidth = 20;
const int kBoxInitialHeight = 50;

static void update_label_to(void * data)
{
    Fl_Box * box = static_cast<Fl_Box*>(data);
    box->do_callback();
    Fl::repeat_timeout(kBoxLabelUpdateTimeout, &update_label_to, data);
}

static void myfiledropcb(void * udata, const wchar_t * fname)
{
    BlaxorApp * app = static_cast<BlaxorApp*>(udata);
    app->openFile(fname);
}

void BlaxorApp::setupGui()
{
    const int w = 900;
    const int h = 600;
    const int scrollbarw = kScrollbarWidth;
    const int boxh = kBoxInitialHeight;
    m_win = new Fl_Double_Window(w, h, m_wintitle.c_str());
    m_box = new Fl_Box(0, 0, w, boxh);
    m_box->box(FL_BORDER_BOX);
    m_box->callback(&update_label_cb, &m_file);
    m_box->do_callback();
    Fl::add_timeout(kBoxLabelUpdateTimeout, &update_label_to, m_box);
    m_slider = new Fl_Slider(w - scrollbarw, boxh, scrollbarw, h - boxh);
    m_display = new BlaHexDisplay(0, boxh, w - scrollbarw, h - boxh);
    m_win->resizable(m_display);
    m_display->take_focus();
    m_display->setFile(&m_file);
    m_display->recalculateMetrics();
    m_display->setLineScrollbar(m_slider);
    m_slider->callback(mycallback, m_display);
    m_win->show(); //win->show(argc, argv);
    enableFileDropOnWindow(m_win, myfiledropcb, this);
}

void BlaxorApp::setBoxHeight(int newh)
{
    const int w = m_win->w();
    const int h = m_win->h();
    const int scrollbarw = kScrollbarWidth;
    const int boxh = newh;

    m_box->size(w, boxh);
    m_display->resize(0, boxh, w - scrollbarw, h - boxh);
    m_slider->resize(w - scrollbarw, boxh, scrollbarw, h - boxh);

    redrawAll();
}

void BlaxorApp::redrawAll()
{
    Fl_Widget * ws[] = { m_box, m_display, m_slider };
    for(auto w : ws)
        if(w)
            w->redraw();
}

#define WIN32
#define _CRT_SECURE_NO_WARNINGS
#include "BlaxorApp.hpp"
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Slider.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Box.H>
#include "BlaHexDisplay.hpp"
#include "BlaHexFile.hpp"
#include "prettyPrintFilesize.hpp"
#include "osSpecific.hpp"
#include "binaryParse.hpp"

const bla::s64 kMaxSearchableFileSize = 1024 * 1024 * 60;

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

    hideInputIfTooBigFile();
    redrawAll();
    return true;
}

bool BlaxorApp::openFile(const wchar_t * fname)
{
    if(!m_file.open(fname))
        return false;

    m_wintitle = utf16ToUtf8(fname) + std::string(": ") + prettyPrintFilesize(m_file.filesize());
    if(m_win)
        m_win->label(m_wintitle.c_str());

    hideInputIfTooBigFile();
    redrawAll();
    return true;
}

const double kBoxLabelUpdateTimeout = 1.0;
const int kScrollbarWidth = 20;
const int kBoxInitialHeight = 50;
const int kInputInitialHeight = 30;

static void update_label_to(void * data)
{
    BlaxorApp * app = static_cast<BlaxorApp*>(data);
    app->refreshBox();
    Fl::repeat_timeout(kBoxLabelUpdateTimeout, &update_label_to, data);
}

static void myfiledropcb(void * udata, const wchar_t * fname)
{
    BlaxorApp * app = static_cast<BlaxorApp*>(udata);
    app->openFile(fname);
}

static void inputcb(Fl_Widget * w, void * udata)
{
    Fl_Input * input = static_cast<Fl_Input*>(w);
    BlaxorApp * app = static_cast<BlaxorApp*>(udata);
    app->findNext(input->value());
}

void BlaxorApp::setupGui()
{
    const int w = 900;
    const int h = 600;
    const int scrollbarw = kScrollbarWidth;
    const int boxh = kBoxInitialHeight;
    const int inputh = kInputInitialHeight;
    m_win = new Fl_Double_Window(w, h, m_wintitle.c_str());
    m_box = new Fl_Box(0, 0, w, boxh);
    m_box->box(FL_BORDER_BOX);
    Fl::add_timeout(kBoxLabelUpdateTimeout, &update_label_to, this);
    m_input = new Fl_Input(0, boxh, w, inputh);
    m_input->callback(&inputcb, this);
    m_slider = new Fl_Slider(w - scrollbarw, boxh + inputh, scrollbarw, h - boxh - inputh);
    m_display = new BlaHexDisplay(0, boxh + inputh, w - scrollbarw, h - boxh - inputh);
    m_win->resizable(m_display);
    m_display->take_focus();
    m_display->setFile(&m_file);
    m_display->recalculateMetrics();
    m_display->setLineScrollbar(m_slider);
    m_slider->callback(mycallback, m_display);
    hideInputIfTooBigFile();
    m_win->show(); //win->show(argc, argv);
    enableFileDropOnWindow(m_win, myfiledropcb, this);
    refreshBox();
}

void BlaxorApp::setBoxHeight(int newh)
{
    const int w = m_win->w();
    const int h = m_win->h();
    const int scrollbarw = kScrollbarWidth;
    const int boxh = newh;
    const int inputh = kInputInitialHeight;

    m_box->size(w, boxh);
    m_input->resize(0, boxh, w, inputh);
    m_display->resize(0, boxh + inputh, w - scrollbarw, h - boxh - inputh);
    m_slider->resize(w - scrollbarw, boxh + inputh, scrollbarw, h - boxh - inputh);

    redrawAll();
}

static bla::s64 findAsciiInFileNext(BlaHexFile& file, bla::s64 start, const std::string& ascii)
{
    for(bla::s64 i = start; i < file.filesize(); ++i)
    {
        bool match = true;
        for(unsigned j = 0; j < ascii.size(); ++j)
        {
            if((i + j) >= file.filesize())
                return -1;

            if(static_cast<unsigned char>(ascii[j]) != file.getByte(i + j))
                match = false;

        }//for j

        if(match)
            return i;
    }//for i

    return -1;
}

void BlaxorApp::findNext(const char * text)
{
    //only do this on small files
    if(m_file.filesize() > kMaxSearchableFileSize)
        return;

    const bla::s64 curi = m_display->getSelectedByte();
    const bla::s64 newi = findAsciiInFileNext(m_file, curi + 1, text);
    if(newi >= 0)
    {
        m_display->setSelectedByte(newi);
        m_display->ensureSelectionInView();
        m_display->redraw();
    }
}

static bool isDisplayChar(unsigned char byte)
{
    //printable ascii range is [0x20, 0x7f)
    return byte >= 0x20 && byte < 0x7f;
}

void BlaxorApp::refreshBox()
{
    if(!m_box)
        return;

    const bla::s64 rc = m_file.readcount();
    char buff[900]; //move to member vector that is few megs, for total safety

    sprintf(buff, "total bytes read: %lld, ", rc);

    const bla::s64 fs = m_file.filesize();
    const bla::s64 selected = m_display->getSelectedByte();

    std::vector<char> asciihere;
    const int maxchars = 100;
    int alen = 0;
    bool gotmore = false;
    for(int i = 0; i < (maxchars + 1); ++i)
    {
        if(selected + i >= fs)
            break;

        const unsigned char c = m_file.getByte(selected + i);
        if(!isDisplayChar(c))
            break;

        if(i == maxchars)
        {
            gotmore = true;
            break;
        }

        ++alen;
        asciihere.push_back(static_cast<char>(c));
    }

    asciihere.push_back('\0');
    sprintf(buff + strlen(buff), "ascii here(%d%s) : %s\n", alen, gotmore?"+":"", asciihere.data());

    //handle if selection is out of file too or is that assumed to never happen?
    unsigned char data[4];
    data[0] = 0xff;
    data[1] = 0xff;
    data[2] = 0xff;
    data[3] = 0xff;
    for(int i = 0; i < 4; ++i)
        if(selected + i < m_file.filesize())
            data[i] = m_file.getByte(selected + i);

    char * buf2 = buff + strlen(buff);
    if(selected + 3 < fs)
    {
        sprintf(buff + strlen(buff), "u32le = %u, s32le = %d,", little_u32(data), little_s32(data));
        sprintf(buff + strlen(buff), "u32be = %u, s32be = %d,", big_u32(data), big_s32(data));
        strcat(buff, "\n");
    }

    if(selected + 1 < fs)
    {
        sprintf(buff + strlen(buff), "u16le = %u, s16le = %d,", little_u16(data), little_s16(data));
        sprintf(buff + strlen(buff), "u16be = %u, s16be = %d,", big_u16(data), big_s16(data));
    }

    sprintf(buff + strlen(buff), "u8 = %u, s8 = %d,", endianless_u8(data), endianless_s8(data));
    strcat(buff, "\n");

    m_box->copy_label(buff);
}

void BlaxorApp::redrawAll()
{
    Fl_Widget * ws[] = { m_box, m_display, m_input, m_slider };
    for(auto w : ws)
        if(w)
            w->redraw();
}

void BlaxorApp::hideInputIfTooBigFile()
{
    if(!m_input)
        return;

    if(m_file.filesize() > kMaxSearchableFileSize)
        m_input->hide();
    else
        m_input->show();
}

#include "blaDefines.hpp"
#include "BlaxorApp.hpp"
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Slider.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Native_File_Chooser.H>
#include "BlaHexDisplay.hpp"
#include "BlaBox.hpp"
#include "BlaFile.hpp"
#include "prettyPrintFilesize.hpp"
#include "osSpecific.hpp"
#include "binaryParse.hpp"
#include <cstring>
#include "blaHelpers.hpp"
#include "utf8dfa.hpp"
#include "BlaxorApp_callbacks.hpp"

const bla::s64 kMaxSearchableFileSize = 1024 * 1024 * 60;

BlaxorApp::BlaxorApp()
{
#ifndef BLA_WINDOWS
    //fix (?) for Linux where FL_SCREEN wasn't monospaced
    Fl::set_font(kHexFontFace, "monospace");
#endif
    setWinTitle("nofile");
}

BlaxorApp::~BlaxorApp()
{
    delete m_win;
}

bool BlaxorApp::openFile(const char * fname)
{
    if(null_or_empty_str(fname) || !m_file.open(fname))
        return false;

    setWinTitle(fname + std::string(": ") + prettyPrintFilesize(m_file.filesize()));
    if(m_display)
        m_display->recalculateMetrics();

    hideInputIfTooBigFile();
    refreshBox();
    redrawAll();
    return true;
}

const int kScrollbarWidth = 20;
const int kBoxInitialHeight = 50;
const int kInputInitialHeight = 30;

void BlaxorApp::setupGui()
{
    const int w = 900;
    const int h = 600;
    const int scrollbarw = kScrollbarWidth;
    const int boxh = kBoxInitialHeight;
    const int inputh = kInputInitialHeight;
    m_win = new Fl_Double_Window(w, h, m_wintitle.c_str());

    //top group with resizeable set to box only so that the button won't resize
    m_topgroup = new Fl_Group(0, 0, w, boxh);
    m_button = new Fl_Button(0, 0, boxh, boxh, "@+9fileopen");
    m_button->callback(&openfilebuttoncb, this);
    m_box = new BlaBox(boxh, 0, w - boxh, boxh);
    m_box->box(FL_BORDER_BOX);
    Fl::add_timeout(kBoxLabelUpdateTimeout, &update_label_to, this);
    m_topgroup->resizable(m_box);
    m_topgroup->end();

    //widgets in main window, with resizeable hex display area
    m_input = new Fl_Input(0, boxh, w, inputh);
    m_input->callback(&inputcb, this);
    m_slider = new Fl_Slider(w - scrollbarw, boxh + inputh, scrollbarw, h - boxh - inputh);
    m_display = new BlaHexDisplay(0, boxh + inputh, w - scrollbarw, h - boxh - inputh);
    m_display->setSelectionChangeCallback(&update_label_cb, this);
    m_win->resizable(m_display);
    m_display->take_focus();
    m_display->setFile(&m_file);
    m_display->recalculateMetrics();
    m_display->setLineScrollbar(m_slider);
    m_slider->callback(scroll_display_cb, m_display);
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

static bla::s64 findAsciiInFileNext(BlaFile& file, bla::s64 start, const std::string& ascii)
{
    for(bla::s64 i = start; i < file.filesize(); ++i)
    {
        bool match = true;
        for(unsigned j = 0; j < ascii.size(); ++j)
        {
            if((i + j) >= file.filesize())
                return -1;

            if(static_cast<bla::byte>(ascii[j]) != file.getByte(i + j))
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

static std::string getMaxUtf8At(BlaFile& file, bla::s64 start, int maxchars, bool * gotmore)
{
    int back = 0;
    if(maxchars <= 0 || !isUtf8SequenceHere(file, start, &back))
        return "";

    start -= back;
    const int utf8len = utf8ByteLenHere(file, start, maxchars, gotmore);
    std::string ret;
    for(int i = 0; i < utf8len; ++i)
        ret.push_back(static_cast<char>(file.getByte(start + i)));

    return ret;
}

static std::string firstLine(const std::string& str)
{
    return str.substr(0u, str.find('\n'));
}

void BlaxorApp::refreshBox()
{
    if(!m_box || m_file.filesize() == 0)
        return;

    const bla::s64 rc = m_file.readcount();
    char buff[900]; //move to member vector that is few megs, for total safety

    sprintf(buff, "sel = %lld, total bytes read: %lld, ", m_display->getSelectedByte(), rc);

    const bla::s64 fs = m_file.filesize();
    const bla::s64 selected = m_display->getSelectedByte();

    int offset = 0;
    const unsigned u8here = utf8Here(m_file, selected, &offset);
    sprintf(buff + strlen(buff), "utf8(%d) codepoint: 0x%06x,", -offset, u8here);

    bool gotmore = false;
    const std::string asciihere = firstLine(getMaxUtf8At(m_file, selected, 50, &gotmore));
    sprintf(
        buff + strlen(buff),
        "utf8 here(%d/%d%s) : %s\n",
        utf8CodepointLen(asciihere.c_str()),
        (int)asciihere.size(),
        gotmore ? "+" : "",
        asciihere.c_str()
    );

    //handle if selection is out of file too or is that assumed to never happen?
    bla::byte data[4];
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
    sprintf(buff + strlen(buff), "binary = ");
    byteToBinaryString(data[0], buff + strlen(buff));
    strcat(buff, "\n");

    m_box->copy_label(buff);
}

void BlaxorApp::redrawAll()
{
    Fl_Widget * ws[] = { m_box, m_display, m_input, m_slider, m_button, m_topgroup };
    for(auto w : ws)
        if(w)
            w->redraw();
}

void BlaxorApp::hideInputIfTooBigFile()
{
    if(!m_input)
        return;

    const int w = m_win->w();
    const int h = m_win->h();
    const int scrollbarw = kScrollbarWidth;
    const int boxh = kBoxInitialHeight;
    const int inputh = kInputInitialHeight;
    if(m_file.filesize() > kMaxSearchableFileSize)
    {
        m_input->hide();
        m_slider->resize(w - scrollbarw, boxh, scrollbarw, h - boxh);
        m_display->resize(0, boxh, w - scrollbarw, h - boxh);
    }
    else
    {
        m_input->show();
        m_slider->resize(w - scrollbarw, boxh + inputh, scrollbarw, h - boxh - inputh);
        m_display->resize(0, boxh + inputh, w - scrollbarw, h - boxh - inputh);
    }

    m_display->recalculateMetrics();
    redrawAll();
}

void BlaxorApp::setWinTitle(const std::string& title)
{
    m_wintitle = "Blaxor: " + title;
    if(m_win)
        m_win->copy_label(m_wintitle.c_str());
}

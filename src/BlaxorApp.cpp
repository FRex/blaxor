#include "blaDefines.hpp"
#include "BlaxorApp.hpp"
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Slider.H>
#include <FL/Fl_Button.H>
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

BlaxorApp::BlaxorApp()
{
#ifdef BLA_LINUX
    //fix (?) for Linux where FL_SCREEN wasn't monospaced
    Fl::set_font(kHexFontFace, "monospace");
#endif //BLA_LINUX
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
    {
        m_display->recalculateMetrics();

        //if a small file is loaded after a big file is loaded already and a byte with index
        //so high selcted that its not a valid index in the new file, the display messes up
        //it can be fixed manually with Ctrl + Home, and by scrolling the Mouse Scroll
        //this SHOULD fix it programatically
        m_display->setFirstDisplayedLine(0);
        m_display->setSelectedByte(0);
    }

    refreshBox();
    redrawAll();
    return true;
}

const int kScrollbarWidth = 20;
const int kBoxInitialHeight = 50;

void BlaxorApp::setupGui()
{
    const int w = 900;
    const int h = 600;
    const int scrollbarw = kScrollbarWidth;
    const int boxh = kBoxInitialHeight;
    m_win = new Fl_Double_Window(w, h, 0x0);
    m_win->copy_label(m_wintitle.c_str()); //to not store ptr to c_str of m_wintitle
    m_win->callback(&double_window_ignore_escape_cb); //TODO: uncomment this when escape is more used in prog itself

    //top group with resizeable set to box only so that the button won't resize
    m_topgroup = new Fl_Group(0, 0, w, boxh);
    m_button = new Fl_Button(0, 0, boxh, boxh, "@+9fileopen");
    m_button->callback(&open_file_button_cb, this);
    m_box = new BlaBox(boxh, 0, w - boxh, boxh);
    m_box->box(FL_BORDER_BOX);
    Fl::add_timeout(kBoxLabelUpdateTimeout, &update_label_to, this);
    m_topgroup->resizable(m_box);
    m_topgroup->end();

    //widgets in main window, with resizeable hex display area
    m_slider = new Fl_Slider(w - scrollbarw, boxh, scrollbarw, h - boxh);
    m_display = new BlaHexDisplay(0, boxh, w - scrollbarw, h - boxh);
    m_display->setSelectionChangeCallback(&update_label_cb, this);
    m_win->resizable(m_display);
    m_display->take_focus();
    m_display->setFile(&m_file);
    m_display->recalculateMetrics();
    m_display->setLineScrollbar(m_slider);
    m_slider->callback(scroll_display_cb, m_display);
    m_win->show(); //win->show(argc, argv);
    enableFileDropOnWindow(m_win, file_drop_cb, this);
    refreshBox();
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

void BlaxorApp::closeAllFiles()
{
    printf("close all files\n");
}

void BlaxorApp::redrawAll()
{
    Fl_Widget * ws[] = { m_box, m_display, m_slider, m_button, m_topgroup };
    for(auto w : ws)
        if(w)
            w->redraw();
}

void BlaxorApp::setWinTitle(const std::string& title)
{
    m_wintitle = "Blaxor: " + title;
    if(m_win)
        m_win->copy_label(m_wintitle.c_str());
}

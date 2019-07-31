#include "blaDefines.hpp"
#include "BlaHexDisplay.hpp"
#include <FL/Fl.H>
#include <cstring>
#include <cstdio>
#include <FL/fl_draw.H>
#include <FL/Fl_Slider.H>
#include "BlaFile.hpp"
#include <algorithm>
#include "blaHelpers.hpp"
#include <cassert>

BlaHexDisplay::BlaHexDisplay(int x, int y, int w, int h, const char * label) : Fl_Widget(x, y, w, h, label)
{
    box(FL_FLAT_BOX);
    recalculateMetrics();
}

void BlaHexDisplay::draw()
{
    if(!m_file || m_toosmall)
        return draw_box(FL_FLAT_BOX, FL_RED);

    draw_box();
    fl_color(FL_BLACK);
    fl_font(kHexFontFace, m_fontsize);
    const int ymax = fl_height() * m_linesdisplayed;
    fl_yxline(x() + m_line1, y(), y() + ymax);
    fl_yxline(x() + m_line2, y(), y() + ymax);

    for(int j = 0; j < m_linesdisplayed; ++j)
    {
        drawAddr(j);
        for(int i = 0; i < m_bytesperline; ++i)
        {
            drawHex(i, j);
            drawChar(i, j);
        }//for i
    }//for j

    drawBottomText();
    //draw_focus();
}

static bool eventinrect(int x, int y, BlaIntRect r)
{
    return Fl::event_inside(x + r.x, y + r.y, r.w, r.h);
    return false;
}

static bool isUtf8ContinuationChar(char c)
{
    const int cc = static_cast<unsigned char>(c);
    return 2 == (cc >> 6);
}

//TODO: asserts for UTF-8 consistency just in case?
static std::string stripOneUtf8FromEnd(std::string s)
{
    //pop all the utf-8 continuation bytes off first, maybe none if last char is ascii...
    while(s.length() > 0u && isUtf8ContinuationChar(s.back()))
        s.pop_back();

    //...now pop one more off, the utf-8 starting byte, or the ascii
    if(s.length() > 0u)
        s.pop_back();

    return s;
}

int BlaHexDisplay::handle(int event)
{
    switch(event)
    {
    case FL_PUSH:
        if(!Fl::event_inside(this))
            return 0;

        take_focus(); //always take focus if we get mouse clicked on

        if(eventinrect(x(), y(), m_hexareabox))
            if(selectByteInBoxOnPushEvent(m_hexareabox, m_onecharwidth / 2, m_onecharwidth))
                redraw();

        if(eventinrect(x(), y(), m_charareabox))
            if(selectByteInBoxOnPushEvent(m_charareabox, 0, 0))
                redraw();

        return 1; //always consume mouse push done into this widget
    case FL_FOCUS:
        //printf("FL_FOCUS\n");
        return 1;
    case FL_UNFOCUS:
        //printf("FL_UNFOCUS\n");
        return 1;
    case FL_KEYDOWN:
        switch(Fl::event_key())
        {
        case FL_Up:
        case FL_Down:
        case FL_Left:
        case FL_Right:
        case FL_Page_Up:
        case FL_Page_Down:
        case FL_Home:
        case FL_End:
            attemptSelectionMove(Fl::event_key(), Fl::event_state(FL_CTRL) != 0);
            return 1;
        case FL_BackSpace:
            if(m_enteringbottomtext && m_bottomtext.length() > 0u && m_bottomtext != "/")
            {
                m_bottomtext = stripOneUtf8FromEnd(m_bottomtext);
                redraw();
            }
            return 1;
        case FL_Escape:
            if(m_enteringbottomtext)
            {
                m_enteringbottomtext = false;
                m_bottomtext.clear();
                redraw();
            }
            return 1;
        break;
        }//switch event key

        if(m_enteringbottomtext)
        {
            if(Fl::event_key() == FL_Enter)
            {
                m_enteringbottomtext = false;
                searchForBottomText();
                redraw();
                return 1;
            }

            if(Fl::event_text())
            {
                const std::string s = Fl::event_text();
                const auto oldlen = m_bottomtext.length();
                for(char c : s)
                    if(!(0 < c && c < ' ')) //no ascii under space, negative is okay for utf-8 with signed char
                        m_bottomtext += c;

                redraw();
                return 1;
            }
            else
            {
                return 0;
            }
        }

        if(0 == std::strcmp("+", Fl::event_text()))
        {
            ++m_fontsize;
            printf("fontsize = %d\n", m_fontsize);
            recalculateMetrics();
            redraw();
            return 1;
        }

        if(0 == std::strcmp("-", Fl::event_text()))
        {
            --m_fontsize;
            printf("fontsize = %d\n", m_fontsize);
            recalculateMetrics();
            redraw();
            return 1;
        }

        if(0 == std::strcmp("b", Fl::event_text()))
        {
            m_binary = !m_binary;
            recalculateMetrics();
            redraw();
            return 1;
        }

        if(0 == std::strcmp("/", Fl::event_text()))
        {
            m_bottomtext = "/";
            m_enteringbottomtext = true;
            redraw();
            return 1;
        }

        break;
    case FL_MOUSEWHEEL:
        if(m_linescrollbar)
        {
            const int dy = Fl::event_dy();
            const int linesperwheelsegment = 10;
            const double newval = m_linescrollbar->value() + dy * linesperwheelsegment;
            const double clamped = m_linescrollbar->clamp(newval);
            m_linescrollbar->value(clamped);
            m_linescrollbar->do_callback();
        }
        return 1;
    }//switch

    return 0;
}

void BlaHexDisplay::resize(int nx, int ny, int nw, int nh)
{
    //call Fl_Widget::resize first to set the new xywh values for
    //recalculateMetrics to use via x(), y(), w(), h() getters
    Fl_Widget::resize(nx, ny, nw, nh);
    recalculateMetrics();
}

bla::s64 BlaHexDisplay::getSelectedByte() const
{
    return m_selectedbyte;
}

void BlaHexDisplay::setSelectedByte(bla::s64 byteidx)
{
    if(byteidx < 0)
        return;

    const bla::s64 old = m_selectedbyte;
    m_selectedbyte = byteidx;

    if(old != byteidx && m_selectionchangecb)
        m_selectionchangecb(this, m_selectionchangeud);
}

void BlaHexDisplay::setSelectionChangeCallback(Fl_Callback * callback, void * udata)
{
    m_selectionchangecb = callback;
    m_selectionchangeud = udata;
}

static char toDisplayChar(bla::byte byte)
{
    if(byte == 0x0)
        return '.';

    if(isDisplayChar(byte))
        return static_cast<char>(byte);
    else
        return '^';
}

void BlaHexDisplay::drawAddr(int yy)
{
    if(!gotByteAt(0, yy))
        return;

    const int xpos = x();
    const int ypos = y() + fl_height() - fl_descent() + yy * fl_height();
    const bla::s64 bytestart = byteIndexAt(0, yy);

    char buff[100];
    sprintf(buff, "%0*llX", m_addresschars, bytestart);

    const bool selline = yy == (m_selectedbyte / m_bytesperline);
    fl_color(selline ? FL_YELLOW : FL_BLACK);
    fl_draw(buff, xpos, ypos);
}

void BlaHexDisplay::drawHex(int xx, int yy)
{
    if(!gotByteAt(xx, yy))
        return;

    const bla::byte byte = getByteAt(xx, yy);
    const int charsbefore = (m_binary ? 9 : 3) * xx;

    const int xpos = x() + m_line1 + m_padding + bla_text_width_charcount(charsbefore);
    const int ypos = y() + fl_height() - fl_descent() + yy * fl_height();

    char buff[10];
    if(m_binary)
    {
        byteToBinaryString(byte, buff);
    }
    else
    {
        sprintf(buff, "%02x", byte);
    }

    setColorForByteDraw(byte, byteIndexAt(xx, yy));

    if(selectedByteAt(xx, yy))
        fl_color(FL_YELLOW);

    fl_draw(buff, xpos, ypos);
}

void BlaHexDisplay::drawChar(int xx, int yy)
{
    if(!gotByteAt(xx, yy))
        return;

    const bla::byte byte = getByteAt(xx, yy);
    const int xpos = x() + m_line2 + m_padding + bla_text_width_charcount(xx);
    const int ypos = y() + fl_height() - fl_descent() + yy * fl_height();

    char buff[2];
    buff[0] = toDisplayChar(byte);
    buff[1] = '\0';

    setColorForByteDraw(byte, byteIndexAt(xx, yy));

    if(selectedByteAt(xx, yy))
        fl_color(FL_YELLOW);

    fl_draw(buff, xpos, ypos);
}

void BlaHexDisplay::drawBottomText()
{
    if(!m_enteringbottomtext)
        return;

    fl_font(FL_HELVETICA, m_fontsize + m_fontsize / 2);
    const int xpos = x();
    const int ypos = y() + h();

    const int xx = bla_text_width(m_bottomtext.c_str());
    const int yy = bla_text_height(m_bottomtext.c_str());

    fl_draw_box(FL_FLAT_BOX, xpos, ypos - yy, xx, yy, FL_WHITE);
    fl_color(FL_BLACK);
    fl_draw(m_bottomtext.c_str(), xpos, ypos);
}

bla::s64 BlaHexDisplay::byteIndexAt(int xx, int yy) const
{
    return (yy + m_firstdisplayedline) * m_bytesperline + xx;
}

bla::byte BlaHexDisplay::getByteAt(int xx, int yy) const
{
    return m_file->getByte(byteIndexAt(xx, yy));
}

bool BlaHexDisplay::gotByteAt(int xx, int yy) const
{
    return byteIndexAt(xx, yy) < m_file->filesize();
}

bool BlaHexDisplay::selectedByteAt(int xx, int yy) const
{
    return byteIndexAt(xx, yy) == m_selectedbyte;
}

void BlaHexDisplay::attemptSelectionMove(int event, bool ctrldown)
{
    bla::s64 newselection = m_selectedbyte;

    if(event == FL_Up)
    {
        if(newselection - m_bytesperline >= 0)
            newselection = newselection - m_bytesperline;
    }//FL_Up

    if(event == FL_Down)
    {
        if(newselection + m_bytesperline < m_file->filesize())
            newselection = newselection + m_bytesperline;
    }//FL_Down

    if(event == FL_Left)
    {
        if(newselection - 1 >= 0)
            newselection = newselection - 1;
    }//FL_Left

    if(event == FL_Right)
    {
        if(newselection + 1 < m_file->filesize())
            newselection = newselection + 1;
    }//FL_Right

    if(event == FL_Page_Up)
    {
        for(int i = m_linesdisplayed; i > 0; --i)
        {
            if(newselection - m_bytesperline * i >= 0)
            {
                newselection = newselection - m_bytesperline * i;
                break;
            }
        }//for i
    }

    if(event == FL_Page_Down)
    {
        for(int i = m_linesdisplayed; i > 0; --i)
        {
            if(newselection + m_bytesperline * i < m_file->filesize())
            {
                newselection = newselection + m_bytesperline * i;
                break;
            }
        }//for i
    }

    if(event == FL_Home)
    {
        if(ctrldown)
            newselection = 0;
        else
            newselection = newselection - (newselection % m_bytesperline);
    }

    if(event == FL_End && !ctrldown)
    {
        newselection = newselection - (newselection % m_bytesperline) + (m_bytesperline - 1);
        while(newselection > 0 && newselection >= m_file->filesize())
            --newselection;
    }

    if(event == FL_End && ctrldown)
    {
        newselection = std::max<bla::s64>(0, m_file->filesize() - 1);
    }

    setSelectedByteAndMoveView(newselection);
}

//helper to calculate amount of hex digit to display any byte's position in a file
static int calcAddrHexNeeded(bla::s64 filesize)
{
    if(filesize <= 0)
        return 1;

    int ret = 0;
    while(filesize > 0)
    {
        filesize /= 16;
        ++ret;
    }
    return ret;
}

void BlaHexDisplay::recalculateMetrics()
{
    m_addresschars = m_file ? calcAddrHexNeeded(m_file->filesize()) : 1;
    m_padding = 1;

    fl_font(kHexFontFace, m_fontsize);
    const int addrwidth = bla_text_width_charcount(m_addresschars);

    m_bytesperline = -1;
    const int powers[] = { 2, 4, 8, 16, 32, 64 };
    int lastgoodattempt = 0;
    const int charsperbyte = m_binary ? 9 : 3;
    for(int i : powers)
    {
        const int attempt = addrwidth + bla_text_width_charcount(i * (charsperbyte + 1) - 1) + 4 * m_padding;
        if(attempt > w())
            break;

        lastgoodattempt = attempt;
        m_bytesperline = i;
    }

    //get maximum possible padding
    while((lastgoodattempt + 4) < w())
    {
        lastgoodattempt += 4;
        ++m_padding;
    }

    //padding and bytes per line are done so find out position of two dividing lines
    m_line1 = addrwidth + m_padding;
    m_line2 = m_line1 + m_padding + bla_text_width_charcount(m_bytesperline * charsperbyte - 1) + m_padding;

    //printf("(addr, bytes, padding) = (%d, %d, %d)\n", m_addresschars, m_bytesperline, m_padding);

    m_linesdisplayed = h() / fl_height();


    m_hexareabox.x = m_line1 + m_padding;
    m_hexareabox.y = 0;
    m_hexareabox.w = bla_text_width_charcount(m_bytesperline * charsperbyte - 1);
    m_hexareabox.h = m_linesdisplayed * fl_height();

    m_onecharwidth = bla_text_width("A");
    m_onecharheight = bla_text_height("A");

    m_charareabox.x = m_line2 + m_padding;
    m_charareabox.y = 0;
    m_charareabox.w = bla_text_width_charcount(m_bytesperline);
    m_charareabox.h = m_linesdisplayed * fl_height();

    ensureScrollbarSize();

    //if too small then set a bool + set to > 0 for safety from crashes
    m_toosmall = m_bytesperline <= 0;
    m_bytesperline = std::max<int>(m_bytesperline, 1);

    //move/scroll the view a bit if we are out of it now with selection
    const bla::s64 line = m_selectedbyte / m_bytesperline;
    if(!(m_firstdisplayedline <= line && line < m_firstdisplayedline + m_linesdisplayed))
    {
        m_firstdisplayedline = line;
        if(m_linescrollbar)
            m_linescrollbar->value(static_cast<double>(line));
    }
}

bla::s64 BlaHexDisplay::getDisplayLineCount() const
{
    if(!m_file)
        return 1;

    return (m_file->filesize() + m_bytesperline - 1) / m_bytesperline;
}

void BlaHexDisplay::setLineScrollbar(Fl_Slider * scrollbar)
{
    m_linescrollbar = scrollbar;
    ensureScrollbarSize();
}

void BlaHexDisplay::ensureScrollbarSize()
{
    if(!m_linescrollbar)
        return;

    const bla::s64 totallines = getDisplayLineCount();
    if(totallines <= m_linesdisplayed)
    {
        m_linescrollbar->bounds(0, 0);
        m_linescrollbar->slider_size(1.0);
        m_linescrollbar->value(0.0);
    }
    else
    {
        m_linescrollbar->bounds(0, static_cast<double>(totallines - m_linesdisplayed));
        const double r = static_cast<double>(m_linesdisplayed) / static_cast<double>(totallines);
        m_linescrollbar->slider_size(r);
        m_linescrollbar->value(0.0);
        //m_linescrollbar->linesize(1);
    }
}

void BlaHexDisplay::setColorForByteDraw(bla::byte b, bla::s64 idx)
{
    if(isDisplayChar(b))
    {
        fl_color(FL_RED);
    }
    else
    {
        if(b && isUtf8SequenceHere(*m_file, idx, 0x0))
            fl_color(FL_BLUE);
        else
            fl_color(FL_BLACK);
    }
}

bool BlaHexDisplay::selectByteInBoxOnPushEvent(const BlaIntRect& r, int xadd, int wadd)
{
    assert(Fl::event() == FL_PUSH);
    if(Fl::event() != FL_PUSH)
        return false;

    //local xy
    const int xx = Fl::event_x() - x() - r.x + xadd;
    const int yy = Fl::event_y() - y() - r.y;

    //cell xy
    const int cx = xx / ((r.w + wadd) / m_bytesperline);
    const int cy = yy / (r.h / m_linesdisplayed);

    if(!gotByteAt(cx, cy))
        return false;

    setSelectedByte(byteIndexAt(cx, cy));
    return true;
}

//TODO: for 1 char needle use memchr
static const void * mymemmem(const void * h, size_t hs, const void * n, size_t ns)
{
    if(ns == 0u)
        return h;

    const unsigned char * hh = static_cast<const unsigned char*>(h);
    const unsigned char firstbyte = *static_cast<const unsigned char*>(n);
    while(ns <= hs)
    {
        if(*hh == firstbyte && 0 == std::memcmp(hh, n, ns))
            return hh;

        --hs;
        ++hh;
    }//while

    return 0x0;
}

//TODO: optimize clean up and make sure it's totally correct
void BlaHexDisplay::searchForBottomText()
{
    if(!m_file)
        return;

    const char * s = m_bottomtext.c_str();
    if(s[0] != '/')
        return;

    s = s + 1;
    const size_t sl = std::strlen(s);
    if(sl == 0u)
        return;

    const bla::byte * f = m_file->getPtr();
    const bla::s64 fl = m_file->filesize();
    if(!f || m_file->filesize() > 10 * 1024 * 1024)
        return;

    const void * x = mymemmem(f + m_selectedbyte, static_cast<size_t>(fl - m_selectedbyte), s, sl);
    if(!x)
        x = mymemmem(f, static_cast<size_t>(fl), s, sl); //inefficient!

    if(x)
        setSelectedByteAndMoveView(static_cast<const bla::byte*>(x) - f);
}

void BlaHexDisplay::setSelectedByteAndMoveView(bla::s64 byteidx)
{
    if(byteidx == m_selectedbyte)
        return;

    const bla::s64 oldline = m_selectedbyte / m_bytesperline;
    setSelectedByte(byteidx);
    redraw();
    if(m_linescrollbar)
    {
        const bla::s64 newline = m_selectedbyte / m_bytesperline;

        if(newline < m_firstdisplayedline)
        {
            m_linescrollbar->value(static_cast<double>(newline));
            m_linescrollbar->do_callback();
        }

        if(newline >= m_firstdisplayedline + m_linesdisplayed)
        {
            m_linescrollbar->value(static_cast<double>(newline - m_linesdisplayed + 1));
            m_linescrollbar->do_callback();
        }
    }//if m_linescrollbar
}

void BlaHexDisplay::setFirstDisplayedLine(bla::s64 line)
{
    m_firstdisplayedline = line;
}

void BlaHexDisplay::ensureSelectionInView()
{
    const bla::s64 firstbyte = byteIndexAt(0, 0);
    const bla::s64 bytecount = m_linesdisplayed * m_bytesperline;
    if(m_selectedbyte < firstbyte)
        setSelectedByte(firstbyte);

    if(m_selectedbyte >= (firstbyte + bytecount))
        setSelectedByte(firstbyte);
}

void BlaHexDisplay::setFile(BlaFile * file)
{
    m_file = file;
}


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

BlaHexDisplay::BlaHexDisplay(int x, int y, int w, int h, const char * label) : Fl_Widget(x, y, w, h, label)
{
    box(FL_FLAT_BOX);
    recalculateMetrics();
}

void BlaHexDisplay::draw()
{
    if(!m_file)
        return;

    draw_box();
    //TODO: some check here that widget is big enough to work
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

    //draw_focus();
}

int BlaHexDisplay::handle(int event)
{
    switch(event)
    {
    case FL_PUSH:
        if(!Fl::event_inside(this))
            return 0;

        take_focus(); //always take focus if we get mouse clicked on
        if(Fl::event_inside(x() + m_hexareabox.x, y() + m_hexareabox.y, m_hexareabox.w, m_hexareabox.h))
        {
            //TODO: move this check to a function, and move index calc to a function
            const int xx = Fl::event_x() - x() - m_hexareabox.x + m_onecharwidth / 2;
            const int yy = Fl::event_y() - y() - m_hexareabox.y;
            const int cx = xx / ((m_hexareabox.w + m_onecharwidth) / m_bytesperline);
            const int cy = yy / ((m_hexareabox.h) / m_linesdisplayed);
            if(gotByteAt(cx, cy))
                setSelectedByte(byteIndexAt(cx, cy));

            redraw();
            return 1;
        }

        if(Fl::event_inside(x() + m_charareabox.x, y() + m_charareabox.y, m_charareabox.w, m_charareabox.h))
        {
            //TODO: move this check to a function, and move index calc to a function
            const int xx = Fl::event_x() - x() - m_charareabox.x;
            const int yy = Fl::event_y() - y() - m_charareabox.y;
            const int cx = xx / (m_charareabox.w / m_bytesperline);
            const int cy = yy / (m_charareabox.h / m_linesdisplayed);
            if(gotByteAt(cx, cy))
                setSelectedByte(byteIndexAt(cx, cy));

            redraw();
            return 1;
        }
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
        }//switch event key

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

    fl_color(FL_BLACK);
    fl_draw(buff, xpos, ypos);
}

void BlaHexDisplay::drawHex(int xx, int yy)
{
    if(!gotByteAt(xx, yy))
        return;

    const bla::byte byte = getByteAt(xx, yy);
    const int charsbefore = 3 * xx;

    const int xpos = x() + m_line1 + m_padding + bla_text_width_charcount(charsbefore);
    const int ypos = y() + fl_height() - fl_descent() + yy * fl_height();

    char buff[10];
    sprintf(buff, "%02x", byte);

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
        newselection =  std::max<bla::s64>(0, m_file->filesize() - 1);
    }

    if(newselection != m_selectedbyte)
    {
        const bla::s64 oldline = m_selectedbyte / m_bytesperline;
        setSelectedByte(newselection);
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
}

//helper to calculate amount of hex digit to display any byte's position in a file
static int calcAddrHexNeeded(bla::s64 filesize)
{
    if(filesize <= 0)
        return 1;

    int ret = 0;
    while(filesize > 0)
    {
        filesize = filesize >> 4;
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

    m_bytesperline = 1;
    const int powers[] = { 2, 4, 8, 16, 32, 64 };
    int lastgoodattempt = 0;
    for(int i : powers)
    {
        const int attempt = addrwidth + bla_text_width_charcount(i * 4 - 1) + 4 * m_padding;
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
    m_line2 = m_line1 + m_padding + bla_text_width_charcount(m_bytesperline * 3 - 1) + m_padding;

    //printf("(addr, bytes, padding) = (%d, %d, %d)\n", m_addresschars, m_bytesperline, m_padding);

    m_linesdisplayed = h() / fl_height();


    m_hexareabox.x = m_line1 + m_padding;
    m_hexareabox.y = 0;
    m_hexareabox.w = bla_text_width_charcount(m_bytesperline * 3 - 1);
    m_hexareabox.h = m_linesdisplayed * fl_height();

    m_onecharwidth = bla_text_width("A");
    m_onecharheight = bla_text_height("A");

    m_charareabox.x = m_line2 + m_padding;
    m_charareabox.y = 0;
    m_charareabox.w = bla_text_width_charcount(m_bytesperline);
    m_charareabox.h = m_linesdisplayed * fl_height();

    m_selectedbyte = 0;
    m_firstdisplayedline = 0;
    ensureScrollbarSize();
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


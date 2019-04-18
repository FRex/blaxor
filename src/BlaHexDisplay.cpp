#define WIN32
#define _CRT_SECURE_NO_WARNINGS
#define NOMINMAX
#include "BlaHexDisplay.hpp"
#include <FL/Fl.H>
#include <cstring>
#include <cstdio>
#include <FL/fl_draw.H>
#include <FL/Fl_Slider.H>
#include "BlaHexFile.hpp"
#include <algorithm>

const int kHexFontFace = FL_SCREEN;
const int kHexFontSize = 16;

static int bla_text_width(const char * str)
{
    int w = 0, h = 0;
    fl_measure(str, w, h, 0);
    return w;
}

static int bla_text_height(const char * str)
{
    int w = 0, h = 0;
    fl_measure(str, w, h, 0);
    return h;
}

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
    fl_font(kHexFontFace, kHexFontSize);
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
                m_selectedbyte = byteIndexAt(cx, cy);

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
                m_selectedbyte = byteIndexAt(cx, cy);

            redraw();
            return 1;
        }
        return 1; //always consume mouse push done into this widget
    case FL_FOCUS:
        printf("FL_FOCUS\n");
        return 1;
    case FL_UNFOCUS:
        printf("FL_UNFOCUS\n");
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
            attemptSelectionMove(Fl::event_key());
            return 1;
        }//switch event key
        break;
    }//switch

    return 0;
}

static bool isDisplayChar(unsigned char byte)
{
    //printable ascii range is [0x20, 0x7f)
    return byte >= 0x20 && byte < 0x7f;
}

static char toDisplayChar(unsigned char byte)
{
    if(isDisplayChar(byte))
        return static_cast<char>(byte);
    else
        return '.';
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

    const unsigned char byte = getByteAt(xx, yy);
    const int charsbefore = (xx == 0) ? 0 : (3 * xx - 1);
    const int xpos = x() + m_line1 + m_padding + charsbefore * bla_text_width("A");
    const int ypos = y() + fl_height() - fl_descent() + yy * fl_height();

    const char * padstr = (xx > 0) ? " " : "";
    char buff[10];
    sprintf(buff, "%s%02x", padstr, byte);

    if(isDisplayChar(byte))
        fl_color(FL_RED);
    else
        fl_color(FL_BLACK);

    if(selectedByteAt(xx, yy))
        fl_color(FL_YELLOW);

    fl_draw(buff, xpos, ypos);
}

void BlaHexDisplay::drawChar(int xx, int yy)
{
    if(!gotByteAt(xx, yy))
        return;

    const unsigned char byte = getByteAt(xx, yy);
    const int xpos = x() + m_line2 + m_padding + xx * bla_text_width("A");
    const int ypos = y() + fl_height() - fl_descent() + yy * fl_height();

    char buff[2];
    buff[0] = toDisplayChar(byte);
    buff[1] = '\0';

    if(isDisplayChar(byte))
        fl_color(FL_RED);
    else
        fl_color(FL_BLACK);

    if(selectedByteAt(xx, yy))
        fl_color(FL_YELLOW);

    fl_draw(buff, xpos, ypos);
}

bla::s64 BlaHexDisplay::byteIndexAt(int xx, int yy) const
{
    return (yy + m_firstdisplayedline) * m_bytesperline + xx;
}

unsigned char BlaHexDisplay::getByteAt(int xx, int yy) const
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

void BlaHexDisplay::attemptSelectionMove(int event)
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
        newselection = newselection - (newselection % m_bytesperline);
    }

    if(event == FL_End)
    {
        newselection = newselection - (newselection % m_bytesperline) + (m_bytesperline - 1);
        while(newselection > 0 && newselection >= m_file->filesize())
            --newselection;
    }

    if(newselection != m_selectedbyte)
    {
        const bla::s64 oldline = m_selectedbyte / m_bytesperline;
        m_selectedbyte = newselection;
        redraw();
        if(m_linescrollbar)
        {
            const bla::s64 newline = m_selectedbyte / m_bytesperline;

            if(newline < m_firstdisplayedline)
            {
                m_linescrollbar->value(newline);
                m_linescrollbar->do_callback();
            }

            if(newline >= m_firstdisplayedline + m_linesdisplayed)
            {
                m_linescrollbar->value(newline - m_linesdisplayed + 1);
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

    char buff[400];
    std::memset(buff, 'A', 400);
    buff[m_addresschars] = '\0';
    fl_font(kHexFontFace, kHexFontSize);
    const int addrwidth = bla_text_width(buff);

    m_bytesperline = 1;
    const int powers[] = { 2, 4, 8, 16, 32, 64 };
    int lastgoodattempt = 0;
    for(int i : powers)
    {
        std::memset(buff, 'A', i * 4 - 1);
        buff[i * 4 - 1] = '\0';
        const int attempt = addrwidth + bla_text_width(buff) + 4 * m_padding;
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
    buff[m_bytesperline * 3 - 1] = '\0';
    m_line1 = addrwidth + m_padding;
    m_line2 = m_line1 + m_padding + bla_text_width(buff) + m_padding;

    //printf("(addr, bytes, padding) = (%d, %d, %d)\n", m_addresschars, m_bytesperline, m_padding);

    m_linesdisplayed = h() / fl_height();


    m_hexareabox.x = m_line1 + m_padding;
    m_hexareabox.y = 0;
    m_hexareabox.w = bla_text_width(buff);
    m_hexareabox.h = m_linesdisplayed * fl_height();

    m_onecharwidth = bla_text_width("A");
    m_onecharheight = bla_text_height("A");

    m_charareabox.x = m_line2 + m_padding;
    m_charareabox.y = 0;
    m_charareabox.w = m_onecharwidth * m_bytesperline;
    m_charareabox.h = m_linesdisplayed * fl_height();

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

    const bla::s64 totallines = getDisplayLineCount();
    if(totallines <= m_linesdisplayed)
    {
        m_linescrollbar->bounds(0, 0);
        m_linescrollbar->slider_size(1.0);
    }
    else
    {
        m_linescrollbar->bounds(0, static_cast<double>(totallines - m_linesdisplayed));
        const double r = static_cast<double>(m_linesdisplayed) / static_cast<double>(totallines);
        m_linescrollbar->slider_size(r);
        //m_linescrollbar->linesize(1);
    }
}

void BlaHexDisplay::setFirstDisplayedLine(bla::s64 line)
{
    m_firstdisplayedline = line;
}

void BlaHexDisplay::setFile(BlaHexFile * file)
{
    m_file = file;
}


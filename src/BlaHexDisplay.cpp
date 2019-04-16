#define WIN32
#define _CRT_SECURE_NO_WARNINGS
#define NOMINMAX
#include "BlaHexDisplay.hpp"
#include <FL/Fl.H>
#include <cstring>
#include <cstdio>
#include <FL/fl_draw.H>
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


    if(1)
        fl_draw_box(
            FL_FLAT_BOX,
            x() + m_hexareabox.x - m_addresschars / 2,
            y() + m_hexareabox.y - m_addresschars / 2,
            m_hexareabox.w + m_addresschars,
            m_hexareabox.h + m_addresschars,
            FL_GREEN
        );
    else
        fl_draw_box(
            FL_FLAT_BOX,
            x() + m_hexareabox.x,
            y() + m_hexareabox.y,
            m_hexareabox.w,
            m_hexareabox.h,
            FL_GREEN
        );

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

        if(Fl::event_inside(x() + m_hexareabox.x, y() + m_hexareabox.y, m_hexareabox.w, m_hexareabox.h))
        {
            //TODO: move this check to a function, and move index calc to a function
            const int xx = Fl::event_x() - x() - m_hexareabox.x;
            const int yy = Fl::event_y() - y() - m_hexareabox.y;
            const int cx = xx / ((m_hexareabox.w + m_onecharwidth) / m_bytesperline);
            const int cy = yy / ((m_hexareabox.h + m_onecharheight) / m_linesdisplayed);
            if(gotByteAt(cx, cy))
                m_selectedbyte = (cy + m_startingline) * m_bytesperline + cx;

            redraw();
            return 1;
        }

        break;
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
    const int bytestart = (yy + m_startingline) * m_bytesperline;

    char buff[100];
    sprintf(buff, "%0*X", m_addresschars, bytestart);

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

unsigned char BlaHexDisplay::getByteAt(int xx, int yy) const
{
    const int idx = (yy + m_startingline) * m_bytesperline + xx;
    return m_file->getByte(idx);
}

bool BlaHexDisplay::gotByteAt(int xx, int yy) const
{
    const int idx = (yy + m_startingline) * m_bytesperline + xx;
    return idx < m_file->filesize();
}

bool BlaHexDisplay::selectedByteAt(int xx, int yy) const
{
    const int idx = (yy + m_startingline) * m_bytesperline + xx;
    return idx == m_selectedbyte;
}

void BlaHexDisplay::attemptSelectionMove(int event)
{
    if(event == FL_Up)
    {
        const int newselection = m_selectedbyte - m_bytesperline;
        if(newselection >= 0 && newselection != m_selectedbyte)
        {
            m_selectedbyte = newselection;
            redraw();
        }
    }//FL_Up

    if(event == FL_Down)
    {
        const int newselection = m_selectedbyte + m_bytesperline;
        if(newselection < m_file->filesize() && newselection != m_selectedbyte)
        {
            m_selectedbyte = newselection;
            redraw();
        }
    }//FL_Down

    if(event == FL_Left)
    {
        if(m_selectedbyte > 0)
        {
            m_selectedbyte = m_selectedbyte - 1;
            redraw();
        }
    }//FL_Left

    if(event == FL_Right)
    {
        if((m_selectedbyte + 1) < m_file->filesize())
        {
            m_selectedbyte = m_selectedbyte + 1;
            redraw();
        }
    }//FL_Right
}

//helper to calculate amount of hex digit to display any byte's position in a file
static int calcAddrHexNeeded(bla::i64 filesize)
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
}

int BlaHexDisplay::getDisplayLineCount() const
{
    if(!m_file)
        return 1;

    return (m_file->filesize() + m_bytesperline - 1) / m_bytesperline;
}

void BlaHexDisplay::setFile(BlaHexFile * file)
{
    m_file = file;
}


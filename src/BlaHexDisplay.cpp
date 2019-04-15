#define WIN32
#define _CRT_SECURE_NO_WARNINGS
#include "BlaHexDisplay.hpp"
#include <FL/Fl.H>
#include <cstring>
#include <cstdio>
#include <FL/fl_draw.H>
#include "BlaHexFile.hpp"
#include <algorithm>

static int bla_text_width(const char * str)
{
    int w = 0, h = 0;
    fl_measure(str, w, h, 0);
    return w;
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
    const int lines = 20;
    const int ymax = fl_height() * lines;
    fl_yxline(x() + m_line1, y(), y() + ymax);
    fl_yxline(x() + m_line2, y(), y() + ymax);
    for(int i = 0; i < lines; ++i)
        drawHexLine(i);

    //draw_focus();
}

void BlaHexDisplay::drawHexLine(int lineno)
{
    fl_color(FL_BLACK);
    fl_font(FL_SCREEN, 16);

    const int yoff = fl_height() - fl_descent() + lineno * fl_height();
    char buff[400];
    std::memset(buff, '\0', 400);

    const int bytestart = (lineno + m_startingline) * m_bytesperline;
    const int bytecount = std::min<int>(m_bytesperline, m_file->filesize() - bytestart);

    if(bytecount <= 0)
        return;

    //addr display
    sprintf(buff, "%0*X", m_addresschars, bytestart);
    fl_draw(buff, x(), y() + yoff);

    //hex display
    for(int i = 0; i < bytecount; ++i)
    {
        const unsigned char byte = m_file->getByte(bytestart + i);
        sprintf(buff + i * 3, "%02x ", byte);
    }//for
    buff[m_bytesperline * 3 - 1] = '\0';
    fl_draw(buff, x() + m_line1 + m_padding, y() + yoff);

    //char display
    std::memset(buff, '?', m_bytesperline);
    buff[m_bytesperline] = '\0';
    fl_draw(buff, x() + m_line2 + m_padding, y() + yoff);
}

void BlaHexDisplay::recalculateMetrics()
{
    m_addresschars = 8;  //TODO: param or from bytebuffersize
    m_bytesperline = 8; //TODO: calc max possible
    m_padding = 4;       //TODO: param or calc but keep low

    char buff[200];

    std::memset(buff, 'A', 200);
    buff[m_addresschars] = '\0';
    fl_font(FL_SCREEN, 16);
    m_line1 = bla_text_width(buff) + m_padding;

    std::memset(buff, 'A', 200);
    buff[m_bytesperline * 3 - 1] = '\0';
    m_line2 = m_line1 + m_padding + bla_text_width(buff) + m_padding;
}

void BlaHexDisplay::setFile(BlaHexFile * file)
{
    m_file = file;
}


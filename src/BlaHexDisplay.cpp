#define WIN32
#define _CRT_SECURE_NO_WARNINGS
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
    const int lines = h() / fl_height();
    const int ymax = fl_height() * lines;
    fl_yxline(x() + m_line1, y(), y() + ymax);
    fl_yxline(x() + m_line2, y(), y() + ymax);
    for(int i = 0; i < lines; ++i)
        drawHexLine(i);

    //draw_focus();
}

static char toDisplayChar(unsigned char byte)
{
    //printable ascii range is [0x20, 0x7f)
    if(byte >= 0x20 && byte < 0x7f)
        return static_cast<char>(byte);

    return '.';
}

void BlaHexDisplay::drawHexLine(int lineno)
{
    fl_color(FL_BLACK);
    fl_font(kHexFontFace, kHexFontSize);

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
    std::memset(buff, '\0', 400);
    for(int i = 0; i < bytecount; ++i)
    {
        const unsigned char byte = m_file->getByte(bytestart + i);
        buff[i] = toDisplayChar(byte);
    }
    fl_draw(buff, x() + m_line2 + m_padding, y() + yoff);
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
    const int powers[] = {2, 4, 8, 16, 32, 64};
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


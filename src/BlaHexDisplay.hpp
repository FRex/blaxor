#pragma once
#include <FL/Fl_Widget.H>
#include "blatype.hpp"

class BlaHexFile;

class BlaIntRect
{
public:
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;
};

class Fl_Slider;

class BlaHexDisplay : public Fl_Widget
{
public:
    BlaHexDisplay(int x, int y, int w, int h, const char * label = 0x0);
    void setFile(BlaHexFile * file);
    void recalculateMetrics();
    bla::s64 getDisplayLineCount() const;
    void setLineScrollbar(Fl_Slider * scrollbar);
    void setFirstDisplayedLine(bla::s64 line);

private:
    virtual void draw() override;
    virtual int handle(int event) override;
    virtual void resize(int nx, int ny, int nw, int nh) override;

    void drawAddr(int yy);
    void drawHex(int xx, int yy);
    void drawChar(int xx, int yy);

    bla::s64 byteIndexAt(int xx, int yy) const;
    unsigned char getByteAt(int xx, int yy) const;
    bool gotByteAt(int xx, int yy) const;
    bool selectedByteAt(int xx, int yy) const;

    void attemptSelectionMove(int event, bool ctrldown);

    int m_addresschars = 6;
    int m_bytesperline = 15;
    int m_padding = 4;
    int m_line1 = 10;
    int m_line2 = 20;
    BlaHexFile * m_file = 0x0;
    bla::s64 m_selectedbyte = 0;
    BlaIntRect m_hexareabox;
    int m_linesdisplayed = 0;
    int m_onecharwidth = 0;
    int m_onecharheight = 0;
    Fl_Slider * m_linescrollbar = 0x0;
    BlaIntRect m_charareabox;
    bla::s64 m_firstdisplayedline = 0;

};

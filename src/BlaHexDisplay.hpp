#pragma once
#include <FL/Fl_Widget.H>
#include "blatype.hpp"
#include <string>

class BlaFile;

class BlaIntRect
{
public:
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;
};

class Fl_Slider;

const int kHexFontFace = FL_SCREEN;

class BlaHexDisplay : public Fl_Widget
{
public:
    BlaHexDisplay(int x, int y, int w, int h, const char * label = 0x0);
    void setFile(BlaFile * file);
    void recalculateMetrics();
    bla::s64 getDisplayLineCount() const;
    void setLineScrollbar(Fl_Slider * scrollbar);
    void setFirstDisplayedLine(bla::s64 line);
    void ensureSelectionInView();
    virtual void resize(int nx, int ny, int nw, int nh) override;
    bla::s64 getSelectedByte() const;
    void setSelectedByte(bla::s64 byteidx);
    void setSelectionChangeCallback(Fl_Callback * callback, void * udata);

private:
    virtual void draw() override;
    virtual int handle(int event) override;

    void drawAddr(int yy);
    void drawHex(int xx, int yy);
    void drawChar(int xx, int yy);
    void drawBottomText();

    bla::s64 byteIndexAt(int xx, int yy) const;
    bla::byte getByteAt(int xx, int yy) const;
    bool gotByteAt(int xx, int yy) const;
    bool selectedByteAt(int xx, int yy) const;

    void attemptSelectionMove(int event, bool ctrldown);
    void ensureScrollbarSize();

    void setColorForByteDraw(bla::byte b, bla::s64 idx);
    bool selectByteInBoxOnPushEvent(const BlaIntRect& r, int xadd, int wadd);

    void searchForBottomText();
    void setSelectedByteAndMoveView(bla::s64 byteidx);

    int m_addresschars = 6;
    int m_bytesperline = 15;
    int m_padding = 4;
    int m_line1 = 10;
    int m_line2 = 20;
    BlaFile * m_file = 0x0;
    bla::s64 m_selectedbyte = 0;
    BlaIntRect m_hexareabox;
    int m_linesdisplayed = 0;
    int m_onecharwidth = 0;
    int m_onecharheight = 0;
    Fl_Slider * m_linescrollbar = 0x0;
    BlaIntRect m_charareabox;
    bla::s64 m_firstdisplayedline = 0;
    Fl_Callback * m_selectionchangecb = 0x0;
    void * m_selectionchangeud = 0x0;
    int m_fontsize = 16;
    bool m_binary = false;
    bool m_toosmall = true;
    std::string m_bottomtext;
    bool m_enteringbottomtext = false;

};

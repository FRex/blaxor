#pragma once
#include <FL/Fl_Widget.H>

class BlaHexFile;

class BlaIntRect
{
public:
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;
};

class BlaHexDisplay : public Fl_Widget
{
public:
    BlaHexDisplay(int x, int y, int w, int h, const char * label = 0x0);
    void setFile(BlaHexFile * file);
    void recalculateMetrics();
    int getDisplayLineCount() const;

private:
    virtual void draw() override;
    virtual int handle(int event) override;

    void drawAddr(int yy);
    void drawHex(int xx, int yy);
    void drawChar(int xx, int yy);

    unsigned char getByteAt(int xx, int yy) const;
    bool gotByteAt(int xx, int yy) const;
    bool selectedByteAt(int xx, int yy) const;

    void attemptSelectionMove(int event);

    int m_addresschars = 6;
    int m_bytesperline = 15;
    int m_padding = 4;
    int m_line1 = 10;
    int m_line2 = 20;
    BlaHexFile * m_file = 0x0;
    int m_selectedbyte = 0;
    BlaIntRect m_hexareabox;
    int m_linesdisplayed = 0;
    int m_onecharwidth = 0;
    int m_onecharheight = 0;

public: int m_startingline = 0;

};

#pragma once
#include <FL/Fl_Widget.H>

class BlaHexFile;

class BlaHexDisplay : public Fl_Widget
{
public:
    BlaHexDisplay(int x, int y, int w, int h, const char * label = 0x0);
    void setFile(BlaHexFile * file);

private:
    virtual void draw() override;
    void drawHexLine(int lineno);
    void recalculateMetrics();

    int m_addresschars = 6;
    int m_bytesperline = 15;
    int m_padding = 4;
    int m_line1 = 10;
    int m_line2 = 20;
    BlaHexFile * m_file = 0x0;

public: int m_startingline = 0;

};

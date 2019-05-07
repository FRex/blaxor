#pragma once
#include <FL/Fl_Widget.H>

class BlaBox : public Fl_Widget
{
public:
    BlaBox(int x, int y, int w, int h, const char * label = 0x0);

private:
    virtual void draw() override;
    virtual int handle(int event) override;

};

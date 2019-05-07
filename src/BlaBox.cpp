#include "BlaBox.hpp"

BlaBox::BlaBox(int x, int y, int w, int h, const char * label) :
    Fl_Widget(x, y, w, h, label)
{
}

void BlaBox::draw()
{
    draw_box();
    draw_label();
}

int BlaBox::handle(int event)
{
    if(event == FL_ENTER || event == FL_LEAVE)
        return 1;

    return 0;
}

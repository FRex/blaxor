#include "blaDefines.hpp"
#include "BlaBox.hpp"
#include <vector>
#include <string>
#include <sstream>
#include <FL/fl_draw.H>
#include <algorithm>
#include "blaHelpers.hpp"

BlaBox::BlaBox(int x, int y, int w, int h, const char * label) :
    Fl_Widget(x, y, w, h, label)
{
}

static std::vector<std::string> split(const std::string& str)
{
    std::vector<std::string> ret;
    std::istringstream ss(str);

    std::string s;
    while(std::getline(ss, s))
        ret.push_back(s);

    return ret;
}

std::vector<std::string> splitbycomma(std::string text)
{
    for(char& c : text)
        if(c == ',')
            c = '\n';

    return split(text);
}

void BlaBox::draw()
{
    draw_box();
    if(label() == 0x0)
        return;

    fl_color(labelcolor());
    fl_font(labelfont(), labelsize());

    const auto parts = splitbycomma(label());
    int line = 0;
    int wsofar = 0;
    const int ww = w();

    Fl_Color c1 = FL_BLACK;
    Fl_Color c2 = FL_RED;
    for(const std::string& p : parts)
    {
        const int tw = bla_text_width(p.c_str());
        if(tw + wsofar > ww)
        {
            ++line;
            wsofar = 0;
        }

        const int yy = y() + fl_height() - fl_descent() + line * fl_height();
        const int xx = x() + wsofar;
        std::swap(c1, c2);
        fl_color(c1);
        fl_draw(p.c_str(), xx, yy);
        wsofar += tw;
    }
}

int BlaBox::handle(int event)
{
    if(event == FL_ENTER || event == FL_LEAVE)
        return 1;

    return 0;
}

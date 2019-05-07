#include "blaDefines.hpp"
#include "BlaBox.hpp"
#include <vector>
#include <string>
#include <sstream>
#include <FL/fl_draw.H>
#include <algorithm>

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

static int bla_text_width(const char * str)
{
    int w = 0, h = 0;
    fl_measure(str, w, h, 0);
    return w;
}

void BlaBox::draw()
{
    draw_box();
    const auto lines = split(label());
    fl_color(labelcolor());
    fl_font(labelfont(), labelsize());
    for(int i = 0; i < static_cast<int>(lines.size()); ++i)
    {
        const int yy = y() + fl_height() - fl_descent() + i * fl_height();
        const int ww = bla_text_width(lines[i].c_str());
        const int xx = x() + std::max<int>(0, (w() - ww) / 2);
        fl_draw(lines[i].c_str(), xx, yy);
    }
}

int BlaBox::handle(int event)
{
    if(event == FL_ENTER || event == FL_LEAVE)
        return 1;

    return 0;
}

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

void BlaBox::draw()
{
    draw_box();
    if(label() == 0x0)
        return;

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

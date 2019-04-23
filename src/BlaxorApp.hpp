#pragma once
#include "BlaNonCopyable.hpp"
#include "BlaHexFile.hpp"
#include <string>

class Fl_Double_Window;
class Fl_Box;
class BlaHexDisplay;
class Fl_Slider;

class BlaxorApp : BlaNonCopyable
{
public:
    ~BlaxorApp();
    bool openFile(const char * fname);
    void setupGui();

private:
    BlaHexFile m_file;
    std::string m_wintitle;
    Fl_Double_Window * m_win = 0x0;
    Fl_Box * m_box = 0x0;
    BlaHexDisplay * m_display = 0x0;
    Fl_Slider * m_slider = 0x0;

};

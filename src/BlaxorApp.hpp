#pragma once
#include "BlaNonCopyable.hpp"
#include "BlaHexFile.hpp"
#include <string>

class Fl_Double_Window;
class Fl_Box;
class BlaHexDisplay;
class Fl_Slider;
class Fl_Input;

class BlaxorApp : BlaNonCopyable
{
public:
    ~BlaxorApp();
    bool openFile(const char * fname);
    void setupGui();
    void setBoxHeight(int newh);
    void findNext(const char * text);
    void refreshBox();

private:
    void redrawAll();
    void hideInputIfTooBigFile();

    BlaHexFile m_file;
    std::string m_wintitle = "nofile";
    Fl_Double_Window * m_win = 0x0;
    Fl_Box * m_box = 0x0;
    BlaHexDisplay * m_display = 0x0;
    Fl_Slider * m_slider = 0x0;
    Fl_Input * m_input = 0x0;

};

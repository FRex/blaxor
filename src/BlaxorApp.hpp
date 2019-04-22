#pragma once
#include "BlaNonCopyable.hpp"
#include "BlaHexFile.hpp"
#include <string>

class Fl_Double_Window;

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

};

#include "blaDefines.hpp"
#include "BlaxorApp_callbacks.hpp"
#include "BlaxorApp.hpp"
#include "BlaHexDisplay.hpp"
#include <FL/Fl_Slider.H>
#include <FL/Fl_Input.H>
#include <FL/Fl.H>
#include <FL/Fl_Native_File_Chooser.H>

void mycallback(Fl_Widget * widget, void * data)
{
    Fl_Slider * sb = static_cast<Fl_Slider*>(widget);
    BlaHexDisplay * my = static_cast<BlaHexDisplay*>(data);

    my->setFirstDisplayedLine(static_cast<bla::s64>(sb->value()));
    my->ensureSelectionInView();
    my->redraw();
}

void update_label_to(void * data)
{
    BlaxorApp * app = static_cast<BlaxorApp*>(data);
    app->refreshBox();
    Fl::repeat_timeout(kBoxLabelUpdateTimeout, &update_label_to, data);
}

void myfiledropcb(void * udata, const char * fname)
{
    BlaxorApp * app = static_cast<BlaxorApp*>(udata);
    app->openFile(fname);
}

void inputcb(Fl_Widget * w, void * udata)
{
    Fl_Input * input = static_cast<Fl_Input*>(w);
    BlaxorApp * app = static_cast<BlaxorApp*>(udata);
    app->findNext(input->value());
}

void update_label_cb(Fl_Widget * w, void * udata)
{
    (void)w;
    BlaxorApp * app = static_cast<BlaxorApp*>(udata);
    app->refreshBox();
}

void openfilebuttoncb(Fl_Widget * w, void * udata)
{
    Fl_Native_File_Chooser chooser;
    chooser.title("Choose a File...");
    chooser.type(Fl_Native_File_Chooser::BROWSE_FILE);
    const int s = chooser.show();
    //TODO: handle -1 (error, reason in errmsg()) and 1 (cancel)?
    if(s == 0)
    {
        BlaxorApp * app = static_cast<BlaxorApp*>(udata);
        app->openFile(chooser.filename());
    }
}

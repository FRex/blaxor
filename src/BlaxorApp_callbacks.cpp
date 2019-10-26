#include "blaDefines.hpp"
#include "BlaxorApp_callbacks.hpp"
#include "BlaxorApp.hpp"
#include "BlaHexDisplay.hpp"
#include <FL/Fl_Slider.H>
#include <FL/Fl_Input.H>
#include <FL/Fl.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_Double_Window.H>

void scroll_display_cb(Fl_Widget * widget, void * data)
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

void file_drop_cb(void * udata, const char * fname, int index)
{
    BlaxorApp * app = static_cast<BlaxorApp*>(udata);
    if(index == 0)
        app->closeAllFiles();

    app->openFile(fname);
}

void update_label_cb(Fl_Widget * w, void * udata)
{
    (void)w;
    BlaxorApp * app = static_cast<BlaxorApp*>(udata);
    app->refreshBox();
}

void open_file_button_cb(Fl_Widget * w, void * udata)
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

void double_window_ignore_escape_cb(Fl_Widget * w, void * udata)
{
    //callback like FLTK tutorial/docs say to use to not have escape close main window
    if(Fl::event() == FL_SHORTCUT && Fl::event_key() == FL_Escape)
        return;

    Fl_Double_Window * win = static_cast<Fl_Double_Window*>(w);
    win->hide();
    Fl_Widget::default_callback(win, udata); // put on Fl::read_queue() ???
}

#pragma once

const double kBoxLabelUpdateTimeout = 1.0;

class Fl_Widget;

void scroll_display_cb(Fl_Widget * widget, void * data);
void update_label_to(void * data);
void file_drop_cb(void * udata, const char * fname);
void search_input_cb(Fl_Widget * w, void * udata);
void update_label_cb(Fl_Widget * w, void * udata);
void open_file_button_cb(Fl_Widget * w, void * udata);
void double_window_ignore_escape_cb(Fl_Widget * w, void * udata);

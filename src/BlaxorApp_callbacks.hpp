#pragma once

const double kBoxLabelUpdateTimeout = 1.0;

class Fl_Widget;

void scroll_display_cb(Fl_Widget * widget, void * data);
void update_label_to(void * data);
void myfiledropcb(void * udata, const char * fname);
void inputcb(Fl_Widget * w, void * udata);
void update_label_cb(Fl_Widget * w, void * udata);
void openfilebuttoncb(Fl_Widget * w, void * udata);

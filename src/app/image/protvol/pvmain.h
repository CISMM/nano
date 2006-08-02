// pvmain.h

#ifndef pvmain_h
#define pvmain_h

#include <FL/Fl.H>
#include <FL/Fl_Widget.h>

#include "MicroscopeFlavors.h"

void pv_exit(Fl_Widget* caller, void* unused);

void open_cb(Fl_Widget* caller, void* unused);

void close_cb(Fl_Widget* caller, void* unused);

void add_canvas(char* filename);

typedef struct {
    Fl_Window* current_canvas;
    Fl_Window* main_window;
} global_struct;

extern global_struct globals;

#endif  // ndef pvmain_h

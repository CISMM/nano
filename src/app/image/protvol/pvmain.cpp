// pvmain.cpp
// Mark Foskey
// 5/99
//
// Displays a height image, lets a user select blobs with the mouse,
// and attempts to compute the volume of the blobs.  The user
// interface code is declared in the files fl_*.h below, and the code
// is generated by Fluid from the corresponding fl_*.fl files.  The
// code for user interaction and computation is mostly in pvcanvas.cpp.

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <FL/fl_file_chooser.h>
#include "fl_main_window.h"
#include "pvcanvas.h"
#include "pvmain.h"

// Variables not declared here are typically in fl*.h 
// and are best explored by opening the corresponding .fl 
// files with the UI editor Fluid.

TopoFile GTF;

global_struct globals = {(Fl_Window*) 0, (Fl_Window*) 0};

Fl_Window* next_canv(Fl_Window* canvas);


int
main(int argc, char* argv[])
{
    Fl::visual(FL_RGB);
    globals.main_window = create_main_window();
    globals.main_window->show();

    char* image_filename = 0;
    if (argc > 1) {
	image_filename = argv[1];
    } else {
	image_filename = fl_file_chooser("Open", 0, 0);
    }
    add_canvas(image_filename);
    return Fl::run();
}

//=============================================================
// Various callbacks.  None of them use the arguments, which
// are required by FLTK.

void
pv_exit(Fl_Widget* caller, void* user_data)
{
    exit(0);
}

void 
open_cb(Fl_Widget* caller, void* user_data) {
    char* newfile = fl_file_chooser("Open", "*", "");
    if (newfile != NULL) add_canvas(newfile);
}

// The various canvases aren't in any real order.  This just returns
// some other canvas.  It's used by close_cb() below to ensure that
// globals.current_canvas always has some value in it.
Fl_Window*
next_canv(Fl_Window* canvas) {
    Fl_Window* next = Fl::first_window();
    if (next == globals.main_window) {
	next = Fl::next_window(next);
    }
    if (next == canvas) {
	next = Fl::next_window(next);
    }
    assert(printf("next = %p.\n", next));
    return next;
}

void 
close_cb(Fl_Widget* caller, void* user_data) {
    Fl_Window* victim = globals.current_canvas;
    assert(printf("victim = %p.\n", victim));
    if (victim == 0) {
	pv_exit((Fl_Widget*) Fl::first_window(), 0);
    }
    Fl_Window* next = next_canv(victim);
    assert(printf("cc = %p.\n", globals.current_canvas));
    victim->hide();
    delete victim;
    globals.current_canvas = next;
    assert(printf("cc = %p.\n", globals.current_canvas));
    if(globals.current_canvas != 0) {
	globals.current_canvas->show();
    }
}

void
add_canvas(char* filename)
{
    PVCanvas* canv = new PVCanvas(filename);
    canv->show();
    Fl_Window* temp = Fl::first_window();
    while (temp != 0) {
	printf("temp = %p.\n", temp);
	temp = Fl::next_window(temp);
    }
    printf("temp = %p.\n", temp);
}

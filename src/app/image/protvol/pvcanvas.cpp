// pvcanvas.cpp

#include "fl_main_window.h"
#include "pvcanvas.h"
#include "pvmain.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream.h>
#include <FL/fl_draw.H>

#include "dbimage.h"
#include "sqr.h"

//=============================================================
// Public methods

//-------------------------------------------------------------
// Constructors.

PVCanvas::
PVCanvas(char* filename)
    : Fl_Overlay_Window(0, 0), _im(new DBImage(filename)), 
      _xcirc(10), _ycirc(10), _rcirc(10), _pvol(0), _outputlist()
{
    callback(close_cb, 0);
    size(_im->xdim(), _im->ydim());
    when(14); // Handle keyboard and mouse events whether or 
              // not data has changed.  (Not that it can change.)
}	

//-------------------------------------------------------------
// Event handler

// handle(int event)
// Draws a circle centered at the user's mouse click and passes
// any keyboard input to the function handle_keypress.  It returns 1
// if the event is handled, and zero otherwise.  (Sometimes it
// "handles" the event by returning 1 and doing nothing.  Otherwise,
// the event had a way of getting resubmitted in a different guise.)
int PVCanvas::
handle(int event)
{
    assert(printf("handle: event = 0x%x.\n", event));
    switch (event) {
    case FL_FOCUS:
        globals.current_canvas = this;
	printf("handle: cc = %p.\n", globals.current_canvas);
	show();
	return 1;
	break;
    case FL_ENTER:
    case FL_PUSH:
    case FL_MOVE :
	return 1;
	break;
    case FL_RELEASE :
	move_circle();
	return 1;
	break;
    case FL_KEYBOARD :
    case FL_SHORTCUT :
	return handle_keypress(Fl::event_key());
	break;
    default :
	return 0;
    }
}

//=============================================================
// Protected methods

//-------------------------------------------------------------
// Drawing callbacks called by the window manager 

void PVCanvas::
draw()
{
    fl_draw_image(_im->byte_array(), 0, 0, _im->xdim(), _im->ydim());
}

void PVCanvas::
draw_overlay()
{
    fl_color(255, 0, 0);
    // Simultaneously compute the volume and draw a circle.
    _pvol = compute_volume();
    assert(printf("_pvol = %g\n", _pvol));
    // Ensure that the output valuators always are consistent
    // with the circle.
    x_out->value(_xcirc);
    y_out->value(_ycirc);
    vol_out->value(_pvol);
    volume_list_displayer->value(_outputlist.c_str());
}

//=============================================================
// Private methods

void PVCanvas::
move_circle()
{
    _xcirc = Fl::event_x();
    _ycirc = Fl::event_y();
    redraw_overlay();
}

// handle_keypress(int command)
// Adjust the size of the circle in response to user input, and call
// compute_volume() when Enter is pressed.
int PVCanvas::
handle_keypress(int command)
{
    int return_val = 1;
    printf("command = 0x%x.\n", command);
    switch(command) {
    case FL_Enter:
	char xyvol[80];
	sprintf(xyvol, "%d\t%d\t%g\n", _xcirc, _ycirc, _pvol);
	_outputlist += xyvol;
	break;
    case 'z':
    case ',':
    case FL_Down:
    case FL_Left:
	_rcirc--;
	break;
    case 'x':
    case '.':
    case FL_Up:
    case FL_Right:
	_rcirc++;
	break;

    // I couldn't figure out a way to send these events to the menu
    // which already had them as shortcuts.
    case 'o':
	if (Fl::event_state(FL_CTRL)) {
	    open_cb((Fl_Widget*) this, 0);
	}
	break;
    case 'w':
	if (Fl::event_state(FL_CTRL)) {
	    close_cb((Fl_Widget*) this, 0);
	}
	break;
    case 'q':
	if (Fl::event_state(FL_CTRL)) {
	    pv_exit((Fl_Widget*) this, 0);
	}
	break;
    default:
	return_val = 0;
    }
    redraw_overlay();
    return return_val;
}

// compute_volume()
// Compute the volume of the protein by summing the heights at the
// given pixels and multiplying by the area of each pixel.  The height
// is measured above the background height, which is estimated as the
// average height of the pixels on the boundary of the user-determined
// circle.
//
// While doing this it also draws the circle.
double PVCanvas::
compute_volume()
{
    // Establish boundaries for the for loop.  As with iterators,
    // the max values point one past the last point you want to
    // consider.
    int xmin = _xcirc - _rcirc;
    if (xmin < 0) 
	xmin = 0;
    int xmax = _xcirc + _rcirc + 1;
    if (xmax > _im->xdim()) 
	xmax = _im->xdim();
    int ymin = _ycirc - _rcirc;
    if (ymin < 0) 
	ymin = 0;
    int ymax = _ycirc + _rcirc + 1;
    if (ymax > _im->ydim()) 
	ymax = _im->ydim();

    int interior_count = 0;
    int boundary_count = 0;
    double interior_sum = 0.0;
    double boundary_sum = 0.0;

    // For each pixel in the box determined above, determine whether
    // the pixel is inside or on the circle and increment the
    // appropriate variables.  Treats the boundary as a 1-pixel-wide
    // strip centered on the true boundary.
    for (int y = ymin; y < ymax; y++) {
	for (int x = xmin; x < xmax; x++) {
	    double r_squared = sqr(x - _xcirc) + sqr(y - _ycirc);
	    if (r_squared < sqr(_rcirc - 0.5)) {
		interior_count++;
		interior_sum += _im->pixval(x, y);
	    }
	    else if (r_squared < sqr(_rcirc + 0.5)) {
		boundary_count++;
		boundary_sum += _im->pixval(x, y);
		fl_line(x, y, x, y);
	    }
	}
    }

    double background_height = boundary_sum / boundary_count;

    assert(printf("bsum = %g, bcount = %d, bh = %g.\n", boundary_sum, 
	boundary_count, background_height));
    assert(printf("isum = %g, icount = %d.  ", 
	interior_sum, interior_count));

    // Effectively subtract the background height from each summand of
    // interior_sum.
    interior_sum -= background_height * interior_count;

    assert(printf("corr: %g.\n", interior_sum));

    return interior_sum * _im->x_nm_perpixel() * _im->y_nm_perpixel();

}

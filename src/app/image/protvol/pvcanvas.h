// pvcanvas.h
// 5/99
// Mark Foskey
// This is where the work gets done.  This class defines an overlay
// window for which the main layer is the image to be analyzed, and
// the overlay shows a circle after the user clicks somewhere.  

#ifndef PVCANVAS_H
#define PVCANVAS_H

#include <string>
#include <FL/Fl.H>
#include <FL/Fl_Overlay_Window.H>

#include "dbimage.h"

using namespace std;

class PVCanvas : public Fl_Overlay_Window {
 public:
    PVCanvas(char* filename);
    virtual DBImage* const image() { return _im; }
    virtual int handle(int event);

 protected:
    // Called when a refresh occurs.
    virtual void draw();
    virtual void draw_overlay();

 private:
    void move_circle();
    int handle_keypress(int which_way);
    double compute_volume();

    DBImage* _im;
    int _xcirc, _ycirc, _rcirc;  // Center and radius of current circle
    double _pvol;                   // Measured volume of circle contents
    string _outputlist;

};

#endif // PVCANVAS_H

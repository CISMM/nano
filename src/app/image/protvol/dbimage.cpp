// dbimage.cpp
// Mark Foskey
// 5/99
// Contains the class for reading images for my program

#include "dbimage.h"
#include "BCGrid.h"
#include "BCPlane.h"

DBImage::
DBImage(char* filename)
    : _da(0), _ba(0), _min_pixval(0), _max_pixval(0), 
      _x_nm_perpixel(0), _y_nm_perpixel(0)
{

    //--------------------------------------------------------------------
    // Read the file into a Grid object, and then put the data into
    // the format for this class.  The reading in of the file is done
    // by the BCGrid constructor, which has no way of communicating a
    // failure to us.  So we just cross our fingers and hope.

    const char** fnp = (const char**) &filename;

    BCGrid *grid = new BCGrid((short)512,(short)512, 0.0, 1.0, 0.0, 1.0, 
			      READ_FILE, fnp, 1);

    BCPlane *plane = grid->head();

    _xdim = grid->numX();
    _ydim = grid->numY();
    _min_pixval = plane->minValue();
    _max_pixval = plane->maxValue();
    _x_nm_perpixel = (plane->maxX()-plane->minX())/((double)grid->numX()-1.0);
    _y_nm_perpixel = (plane->maxY()-plane->minY())/((double)grid->numY()-1.0);

    // Copy data into an array of doubles.
    double *dpixels = new double[_xdim * _ydim];
    for (int y = 0; y < _ydim; y++) {
	for (int x = 0; x < _xdim; x++) {
	    dpixels[x + y*_xdim] = plane->value(x,y);
	}
    }
    _da = dpixels;

    // Copy data into an array of bytes, scaled to fit [0, 255].
    unsigned char* bpixels = new unsigned char[_xdim * _ydim * 3];
    double shifted_max = _max_pixval - _min_pixval;
    for (int i = 0; i < _xdim * _ydim; i++) {
	bpixels[3*i+2] = bpixels[3*i+1] = bpixels[3*i] = (unsigned char)
		     ((dpixels[i] - _min_pixval) / shifted_max * 255 + .5);
    }
    _ba = bpixels;

    delete grid;

}

DBImage::
~DBImage()
{
    delete [] _da;
    delete [] _ba;
}

#include <stdlib.h>
#include <stdio.h>
#include "cimage.h"
#include "cimage_util.h"

extern	FILE *outfile;
// conversion factors in nm per pixel, set when a file is read
extern double xConvFactor,yConvFactor;

int tracker(void *buffer, int xdim, int ydim, float x, float y, float s,
	    int (*func)(int, int, void*), void *AppData, int trace_mode)
{
	Curve C = curve_init(1000);
	cimage im = cimage_new(CREAL, xdim, ydim, 1);
	cimage_pixels(im) = (char *) buffer;
	int	ret;
	int	i;

	ret = cimage_construct_edge(im, C, x, y, s, func, AppData, trace_mode);

	if (outfile) {
//	    fprintf(outfile,"%i\n%i\n", 1, curve_length(C));
	    for (i = 0; i < curve_length(C); i++) {
		fprintf(outfile,"%f\t%f\t%f\n", curve_point_getx(C,i)*xConvFactor,
			curve_point_gety(C,i)*yConvFactor, s);
	    }
	}
	curve_destroy(C);

	return ret;
}


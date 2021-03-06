/* The nanoManipulator and its source code have been released under the
 * Boost software license when nanoManipulator, Inc. ceased operations on
 * January 1, 2014.  At this point, the message below from 3rdTech (who
 * sublicensed from nanoManipulator, Inc.) was superceded.
 * Since that time, the code can be used according to the following
 * license.  Support for this system is now through the NIH/NIBIB
 * National Research Resource at cismm.org.

Boost Software License - Version 1.0 - August 17th, 2003

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#ifndef X_UTIL_H
#define X_UTIL_H
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "x_aux.h"

#define LEVELS 256
#define RED 256

extern	int	WINSIZE;

extern int createAndInstallGrayMap ();
extern void colorMapTest ();
extern int createWindow (int argc, char** argv);
extern int freeWindow ();
extern void clearWindow ();
extern void colorMapFullTest ();
extern void put_line_xwin (int sx,int sy, int ex,int ey);
extern void synchronize_xwin (int);	/* True or false */
extern void flush_xwin (void);
extern int restore_window (TwoDLineStrip *);
extern void put_dot(double, double, int, int);
extern void x_put_value (int x, int y, double value);
extern void x_set_scale(double minval, double maxval);
extern int button_pressed_in_window(int*, int*);
extern void mouse_xy_to_region(int, int, double*, double*);
extern int button2_moved_in_window(int*, int*);
extern int button_released_in_window(int *, int*);

extern void xc_enable (void);  // enable contour map

//extern int x_draw_marker (const nmb_LocationInfo &, void *);
extern int x_draw_marker (const PointType, const PointType, void *);
#endif /* X_UTIL_H */

#if 0
/*
 *	This file contains definitions needed when using the Scanning
 * Tunnelling Microscope.
 */

#ifndef	STM_H
#define	STM_H

#include 	<stdio.h>
#include	<math.h>
#include	"stm_file.h"

/* Mode of operation for the microscope */

#define	STM_NULL_MODE			(0)
#define	STM_CONSTANT_HEIGHT_MODE	(1)
#define	STM_CONSTANT_CURRENT_MODE	(2)

/* Which line to read the Z value from (in local mode) */
#define	STM_Z_CHANNEL			(6)

/*********
 * Functions defined in some .c files (added by KPJ to satisfy g++)...
 *********/

/* defined in x_util.c, x_scanline.c, or x_uscape.c */
extern void put_pixel(int, int, int);
extern void x_put_value(int x, int y, double value);
extern void x_set_scale(double minval, double maxval);
extern int button_pressed_in_window(int*, int*);
extern int button2_moved_in_window(int*, int*);
extern int button_released_in_window(int *, int*);
extern void mouse_xy_to_region(int, int, double*, double*);

//#ifndef MICROSCAPE_GP
#include "BCPlane.h"
#include "Point.h"
extern BCGrid* inputGrid;
extern BCGrid* snapshotGrid;
//#endif

#endif
#endif

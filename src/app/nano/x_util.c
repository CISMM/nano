#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <string.h>
#include <fcntl.h>
#ifndef __CYGWIN__
#include <sys/time.h>
#endif

#include <BCPlane.h>
#include <nmb_Dataset.h>
#include <nmb_Decoration.h>
#include <nmb_Globals.h>

//#include <nmm_Globals.h>  // for USE_VRPN_MICROSCOPE, now controlled
                            // by Makefile

#ifndef USE_VRPN_MICROSCOPE
#include <Microscope.h>
#else
#include <nmm_MicroscopeRemote.h>
#endif

#include "x_util.h"
#include "microscape.h"  // for xPlaneName

static  int xcenable = 0;  // enable contour map

static	Display *display;
static	int screen;
static	Window window;
static	GC gc;
static	Colormap mapId;
static	unsigned long myforeground, mybackground;

static	double	min_value = 0;			/* Value mapped to black */
static	double	value_scale = (double)(LEVELS-1)/1.0;	/* Scales input */

#define      NEAR(x0,x1)     (fabs(x0-x1) < 0.001)
/*******************
 * Global variables
 *******************/

// used in x_aux, microscape.c to compute points_per_pixel
int	WINSIZE = 512;		/* Size of the window */

static char window_name[] = "x_uscape";

#ifndef max
#define max(a,b) ((a)<(b)?(b):(a))
#endif

void region_xy_to_mouse (double x, double y, int * mx, int * my);


void	PrintXerr(char* s, int err)
{
	char	reason[100];

	fprintf(stderr,"%s (",s);
	switch (err) {
#define	Xerr_case(x)	case x: {strcpy(reason,"x"); break;}

		Xerr_case(BadRequest);
		Xerr_case(BadValue);
		Xerr_case(BadWindow);
		Xerr_case(BadPixmap);
		Xerr_case(BadAtom);
		Xerr_case(BadCursor);
		Xerr_case(BadFont);
		Xerr_case(BadMatch);
		Xerr_case(BadDrawable);
		Xerr_case(BadAccess);
		Xerr_case(BadAlloc);
		Xerr_case(BadColor);
		Xerr_case(BadGC);
		Xerr_case(BadIDChoice);
		Xerr_case(BadName);
		Xerr_case(BadLength);
		Xerr_case(BadImplementation);

		default:
			sprintf(reason,"Unrecognized err [%d]",err);
			break;
	}
	fprintf(stderr,"%s)\n",reason);
}

void	synchronize_xwin(int boolvar)
{
	XSynchronize(display, boolvar);
}

void	flush_xwin(void)
{
	XFlush(display);
}

int createAndInstallGrayMap()
{
  XColor ccell_def;
  unsigned long Plane_Masks[1];
  unsigned long Pixels[256];
  int I;
  double val, scaleup;
  double tempval;

  mapId = XCreateColormap(	display,
				window,
				DefaultVisual(display, screen),
				AllocNone);

  if (!XAllocColorCells( display, mapId, True,
			 Plane_Masks, 0, Pixels, LEVELS)) {
        puts("Cannot allocate color cells");
	return(0);
    }

  ccell_def.flags = DoRed | DoGreen | DoBlue;

  scaleup = 65536.0;

  if(xcenable)
    {
      for (I = 0; I < LEVELS; I++) {
	if((I%10)==0)
	  {
	    ccell_def.pixel = I;
	    val = scaleup-1;
	    ccell_def.red = (unsigned int) val;
	    ccell_def.green = (unsigned int) val;
	    ccell_def.blue = (unsigned int) val;
	    XStoreColor(display, mapId, &ccell_def);
	  }
	else if((I%10)==1 || (I%10)==9)
	  {
	    ccell_def.pixel = I;
	    val = (scaleup-1)/2.0;
	    ccell_def.red = (unsigned int) val;
	    ccell_def.green = (unsigned int) val;
	    ccell_def.blue = (unsigned int) val;
	    XStoreColor(display, mapId, &ccell_def);
	  }
	else
	  {
	    ccell_def.pixel = I;
	    ccell_def.red =   0;
	    ccell_def.green = 0;
	    ccell_def.blue =  0;
	    XStoreColor(display, mapId, &ccell_def);
	  }
      }
   }
  else
    {
      for (I = 0; I < LEVELS; I++) {
	if(I<(LEVELS-1))
	  {
	    ccell_def.pixel = I;
	    tempval = (double) I / (double) LEVELS;
	    val = tempval * scaleup;
	    ccell_def.red = (unsigned int) val;
	    ccell_def.green = (unsigned int) val;
	    ccell_def.blue = (unsigned int) val;
	    XStoreColor(display, mapId, &ccell_def);
	  }
	else
	  {
	    ccell_def.pixel = I;
	    tempval = (double) I / (double) LEVELS;
	    val = tempval * scaleup;
	    ccell_def.red = (unsigned int) val;
/*	    if(read_mode!=READ_FILE_MODE) 
	      {*/
		ccell_def.green = 0;
		ccell_def.blue =  0;
/*	      }
	    else
	      {
		ccell_def.green=(unsigned int) val;
		ccell_def.blue = (unsigned int) val;
	      }*/
	    XStoreColor(display, mapId, &ccell_def);
	  }
      }
    }

  XSetWindowColormap(display, window, mapId);

  XInstallColormap(display, mapId);

  return(1); 
}

int createWindow(int argc, char** argv)
{

  XSizeHints myhint;

  if ( (display = XOpenDisplay("")) == NULL) {
	fprintf(stderr,"Could not open display\n");
	return(-1);
  }
  screen = DefaultScreen(display);

  synchronize_xwin(True);

  mybackground = WhitePixel(display, screen);
  myforeground = BlackPixel(display, screen);

  myhint.x = 256; myhint.y = 256;
  myhint.width = WINSIZE; myhint.height = WINSIZE;
  myhint.flags = PPosition | PSize;

  window = XCreateSimpleWindow(display,
	DefaultRootWindow (display),
	myhint.x, myhint.y, myhint.width, myhint.height,
	5, myforeground, mybackground);

  if (!window) {
    fprintf(stderr,"Bad window allocation\n");
    return(-1);
    }
  else puts("Window created.");

  XSetStandardProperties(display, window, window_name, window_name,
	None, argv, argc, &myhint);
  
  gc = XCreateGC(display, window, 0, 0);

  XSelectInput(display, window,
	ButtonPressMask | ButtonReleaseMask | KeyPressMask | ExposureMask |
	ButtonMotionMask);

  XMapRaised(display, window);

  return(0);
}

int freeWindow(void)
{
  XFreeGC(display, gc);
  XDestroyWindow(display, window);
  XCloseDisplay(display);
  return 0;
}

void clearWindow()
{
 XSetForeground(display, gc, BlackPixel(display, screen));
 XFillRectangle(display, window, gc, 0, 0, WINSIZE, WINSIZE);
 XFlush(display);
 XSetForeground(display, gc, WhitePixel(display, screen));
}


/* The origin of the X window is in the upper left corner. */
/* This differs from Xlib's view of the world, which is in the
 * lower left.  As a result, the y axis must be subtracted from the
 * bottom. */

void	put_pixel(int x, int y, int color)
{
	int size = WINSIZE / max(dataset->inputGrid->numX(),
                                 dataset->inputGrid->numY());
	int screen_y = (WINSIZE-size) - y*size;

	/* Make sure the color doesn't go out of range */
	if (color < 0) color = 0;
	if (color > (LEVELS-2)) color = (LEVELS-2);

	XSetForeground(display, gc, color);
	XFillRectangle(display, window, gc, x*size, screen_y, size, size);
}

/* This routine is called to set the range of values to be displayed on
 * the window.  This is used to determine how to scale values for the
 * put_pixel() routine.
 * If max < min, they are swapped. */

void	x_set_scale(double min, double max) {
	if (min < max) {
		min_value = min;
		value_scale = (double)(LEVELS-1) / (max-min);
	} else {
		min_value = max;
		value_scale = (double)(LEVELS-1) / (min-max);
	}
}

/* This routine will plot an incoming value based on the scale that has
 * been set for the x window. */

void	x_put_value(int x, int y, double value)
{
	put_pixel(x,y, (int)((value-min_value)*value_scale));
}

// TCH 6 May 98
//
// Draws one pulse/scrape marker.

int x_draw_marker (const nmb_LocationInfo & m, void *) {
  double frac_x, frac_y;

// check that there really is an x plane;  otherwise there isn't
// an x window to draw into!
  if (!dataset->inputGrid->getPlaneByName(xPlaneName.string()))
    return 0;

  frac_x = (m.x - dataset->inputGrid->minX()) /
           (dataset->inputGrid->maxX() - dataset->inputGrid->minX());
  frac_y = (m.y - dataset->inputGrid->minY()) /
           (dataset->inputGrid->maxY() - dataset->inputGrid->minY());

  if ((frac_x < 0.0) || (frac_x > 1.0) ||
      (frac_y < 0.0) || (frac_y > 1.0)) return 0;

  put_dot(frac_x, frac_y, RED, 1);
//fprintf(stderr, "Drew marker at (%.4f, %.4f)\n", frac_x, frac_y);
  return 0;
}

int x_draw_marker (const PointType Top, const PointType, void *) {
  double frac_x, frac_y;

// check that there really is an x plane;  otherwise there isn't
// an x window to draw into!
  if (!dataset->inputGrid->getPlaneByName(xPlaneName.string()))
    return 0;

  frac_x = (Top[0] - dataset->inputGrid->minX()) /
           (dataset->inputGrid->maxX() - dataset->inputGrid->minX());
  frac_y = (Top[1] - dataset->inputGrid->minY()) /
           (dataset->inputGrid->maxY() - dataset->inputGrid->minY());
  put_dot(frac_x, frac_y, RED, 1);
//fprintf(stderr, "Added x scrape marker at (%.4f, %.4f)\n", frac_x, frac_y);

  return 0;
}


static void x_draw_strip (TwoDLineStrip * strip) {

  TwoDVertexPtr a;
  double x, y;
  int sx, sy, ex, ey;
  int i;

  a = strip->vertices;
  x = a->point[0];
  y = a->point[1];
  region_xy_to_mouse(x, y, &ex, &ey);

  for (i = 0;
       (i < strip->num_segments) && a && a->next;
        i++, a = a->next) {
    sx = ex;  sy = ey;
    x = a->next->point[0];
    y = a->next->point[1];
    region_xy_to_mouse(x, y, &ex, &ey);
    put_line_xwin(sx, sy, ex, ey);
  }

}


/*****************************************************************************

  restore_window() - init x graphics display (qliu 8/12/95)

******************************************************************************/
int
restore_window(TwoDLineStrip * strip)
{
  XEvent event;
  int x,y;

  if(XCheckWindowEvent(display,window,ExposureMask,&event)!=True)
    {
      return(-1);
    }
  fprintf(stderr,"Exposure!  xPlane is \"%s\"\n", xPlaneName.string());
  synchronize_xwin(False);   /* Allow asynchronous writes */

  // TOPOGRAPHY

  BCPlane* xPlane = dataset->inputGrid->getPlaneByName(xPlaneName.string());

  if(xPlane!=NULL) {
      for (x = 0; x < dataset->inputGrid->numX(); x++) {
         for (y = 0; y < dataset->inputGrid->numY(); y++) {
            x_put_value(x,y,xPlane->value(x,y));
         }
      }
  }

  // MODIFICATION MARKERS

  decoration->traverseVisiblePulses(x_draw_marker, NULL);
  decoration->traverseVisibleScrapes(x_draw_marker, NULL);

  // LINE OF CROSS-SECTION

  if (strip)
    x_draw_strip(strip);


  XFlush(display);              /* Flush the last output */
  synchronize_xwin(True);	/* Disable asynchronous writes*/
  return(0);
}

/* This function puts a dot on the screen. It takes parameters x,y ranging from 0 to 1 and maps them to a screen coordinate (qliu 7/10/95)*/
void	put_dot(double x, double y, /* point from 0 to 1 */
		int color, /* Cmap entry for the pixel */
		int ratio)
{
	int size = WINSIZE / max(dataset->inputGrid->numX(),
                                 dataset->inputGrid->numY());
	int x_fill=size*dataset->inputGrid->numX()-1;
	int y_fill=size*dataset->inputGrid->numY()-1;
	int screen_x=(int)(x_fill*x);
	int temp_y=(int)(y_fill*y);
	int screen_y = (WINSIZE-1) - temp_y;

	/* Make sure the color doesn't go out of range */
	if (color < 0) color = 0;
	if (color > (LEVELS-1)) color = (LEVELS-1);

	XSetForeground(display, gc, color);
	XFillRectangle(display, window, gc, screen_x, screen_y, ratio, ratio);
}


/* This routine will put a line in the highlight color (red) from the
 * start (sx,sy) to the end (ex,ey).  Coordinates are the same as for
 * put_pixel() and put_dot(). */
void	put_line_xwin(int sx,int sy, int ex,int ey)
{
	XSetForeground(display,gc,LEVELS-1);
	XDrawLine(display,window,gc, sx,(WINSIZE-1)-sy, ex,(WINSIZE-1)-ey);
}

/*	This routine will return >0 if there was a button press event
 * in the event queue for the window and 0 if there was not.  If there
 * was an event, then the x and y coordinates of the press are returned.
 *	The number of the button that was pressed is returned.
 *	If there is an error, then -1 is returned. */

int	
button_pressed_in_window(int *x, int* y)
{
	XEvent		event;		/* The event that is checked */
	XButtonEvent	button;		/* The button event */

	/* See if there is a button press event in the queue */
	if (XCheckWindowEvent(display, window, ButtonPressMask,
	    &event) != True) {
		return(0);
	}
	memcpy(&button, &event, sizeof(button));

	/* Return the values */
	/* Note that the y origin of the mouse event is the upper left
	 * corner of the window, but we want the origin in the lower left */
	*x = button.x;
	*y = (WINSIZE-1) - button.y;
	return(button.button);
}

/*	This routine will return >0 if there was a button motion event
 * in the event queue for the window and 0 if there was not.  If there
 * was an event, then the x and y coordinates of the press are returned.
 *	The number of the button that was pressed is returned.
 *	If there is an error, then -1 is returned. */

int	button2_moved_in_window(int* x, int* y)
{
	XEvent		event;		/* The event that is checked */

	/*printf("testing the motion of the second Button\n");*/
	/* See if there is a button press event in the queue */
	if (XCheckWindowEvent(display, window, Button2MotionMask,
	    &event) != True) {
		return(0);
	}

	/*printf("successful\n");*/
	/* Return the values */
	/* Note that the y origin of the mouse event is the upper left
	 * corner of the window, but we want the origin in the lower left */
	*x = event.xmotion.x;
	*y = (WINSIZE-1) - event.xmotion.y;
	/*printf("successful set x and y\n");*/
	return(1);
}

/*	This routine will return >0 if there was a button RELEASE event
 * in the event queue for the window and 0 if there was not.  If there
 * was an event, then the x and y coordinates of the press are returned.
 *	The number of the button that was pressed is returned.
 *	If there is an error, then -1 is returned.
 *	Note this is identical to button_pressed_in_window but only
 *	accepts release events */

int	button_released_in_window(int *x, int* y)
{
	XEvent		event;		/* The event that is checked */
	XButtonEvent	button;		/* The button event */

	/* See if there is a button press event in the queue */
	if (XCheckWindowEvent(display, window, ButtonReleaseMask,
	    &event) != True) {
		return(0);
	}
	memcpy(&button, &event, sizeof(button));

	/* Return the values */
	/* Note that the y origin of the mouse event is the upper left
	 * corner of the window, but we want the origin in the lower left */
	*x = button.x;
	*y = (WINSIZE-1) - button.y;
	return(button.button);
}


/*	This routine will map from the window coordinates returned from the
 * button_xxx_in_window routines into region space.
 *	The location will be in nanometers, since that is what the surface
 * is in.
 *	NOTE: For regions that are not a power of 2 on a side, it is
 * possible to click the mouse outside the scanned region.  This routine
 * will then return a value outside the normally-scanned region. */

void	mouse_xy_to_region(int x, int y, double* rx, double* ry)
{
	int	points_per_pixel, covered_x, covered_y;

	/* Find location from 0 to 1 in covered pixels of buttondown */
	points_per_pixel = WINSIZE / max(dataset->inputGrid->numX(),
                                         dataset->inputGrid->numY());
	covered_x = points_per_pixel * dataset->inputGrid->numX();
	covered_y = points_per_pixel * dataset->inputGrid->numY();
	*rx = (double)(x)/covered_x;
	*ry = (double)(y)/covered_y;

	/* Convert this into location within the region */
	/* This will be in nanometers, since region is */
	*rx = dataset->inputGrid->minX() +
              *rx * (dataset->inputGrid->maxX() - dataset->inputGrid->minX());
	*ry = dataset->inputGrid->minY() +
              *ry * (dataset->inputGrid->maxY() - dataset->inputGrid->minY());
}

// Convert from nm in the dataset to xwindow coordinates 

void region_xy_to_mouse (double x, double y, int * mx, int * my) {

  int points_per_pixel, covered_x, covered_y;

  /* Find location from 0 to 1 in covered pixels of buttondown */
  points_per_pixel = WINSIZE / max(dataset->inputGrid->numX(),
                                   dataset->inputGrid->numY());
  covered_x = points_per_pixel * dataset->inputGrid->numX();
  covered_y = points_per_pixel * dataset->inputGrid->numY();

  x = (x - dataset->inputGrid->minX()) /
      (dataset->inputGrid->maxX() - dataset->inputGrid->minX());
  y = (y - dataset->inputGrid->minY()) /
      (dataset->inputGrid->maxY() - dataset->inputGrid->minY());
  *mx = int(x * covered_x);
  *my = int(y * covered_y);

//fprintf(stderr, "(%.4f, %.4f) nm = screen coordinates (%d, %d)\n",
//x, y, *mx, *my);
}

// enable contour map
void xc_enable (void) {
  xcenable = 1;
}



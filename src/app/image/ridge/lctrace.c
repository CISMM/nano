/*----------------------------------------------------------------------------
 File:    lctrace.c  (set tab stops = 4 for proper viewing)
 Purpose: Program traces level curves in a 2D image function
 Date:    July 1, 1994
 Author:  Dan Fritsch (tracking code courtesy of Dave Eberly)
----------------------------------------------------------------------------*/

#include <math.h>
#include <Xm/Xm.h>
#include <Xm/MainW.h>
#include <Xm/Form.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/DrawingA.h>
#include <Xm/ToggleB.h>
#include <Xm/Frame.h>
#include <Xm/Label.h>

#include <stdio.h>

#include "misc_X.h"
#include "cimage_X.h"
#include "cimage.h"
#include "cimage_io.h"
#include "cimage_ops.h"
#include "cimage_util.h"
#include "cimage_curve.h"
#include "curve_io.h"
#include "constants.h"

#include "BCGrid.h"
#include "BCPlane.h"
#include "Topo.h"

cimage im;
int prev_x, prev_y;

/* X and Motif globals */
GC gc;
Colormap cmap;
Cmap_info cmap_info;
Curve C;
Widget imageWidget;

/* Application defaults */
static	float scale = 1.0;
float sampling = 0.1;
int mode = 0;
char edgefile[128];
char imagefile[128];
char tmpDir[128];
FILE* outfile = NULL;

// global topo file
TopoFile GTF;

// conversion factors in nm per pixel, set when a file is read
double xConvFactor,yConvFactor;

int tracker(void *buffer, int xdim, int ydim, float x, float y, float s,
            int (*func)(int, int, void*), void *AppData, int mode);


int draw_func(int x, int y, void *AppData)
{
	Widget *w = (Widget*) AppData;
	XEvent event;
	XDrawPoint(XtDisplay(*w), XtWindow(*w), gc, x, y);
	XFlush(XtDisplay(*w));

        if (XCheckWindowEvent(XtDisplay(*w), XtWindow(*w), 
            ButtonPressMask, &event)) return 0;
        else return 1;
}

	
/*--------------------------------------------------------------------------- */
void Usage ()
{
    printf("\nusage:  lctrace [options] imagefile\n");
    printf("  -f (string) : output ASCII file of labeled segments\n");
    printf("  -s (float)  : sampling factor (default = 0.2)\n");
    printf("  -p (float)  : preblur factor (default = 1.0)\n");
    printf("  -t (string) : tmpDir for IMS\n");
    printf("  -m (int)    : mode (default = 0)\n");
    printf("                  0 = Laplacian zero crossings\n");
    printf("                  1 = Directional derivative zero crossings\n");
    printf("                  2 = Intensity level curves\n");
    printf("                  3 = Gradient flow lines\n");
    printf("  imagefile   : 2D grey image (Topometrix file format)\n");
    exit(-1);
}
/*--------------------------------------------------------------------------- */

void draw_circ(Widget w, int x, int y, int radius)
{
        Display *display = XtDisplay(w);
        GC gc;
        XGCValues values;
        unsigned long valuemask = GCForeground | GCFunction | GCPlaneMask;
        values.foreground = 255;
        values.function = GXxor;
        values.plane_mask = 0x80;
        gc = XCreateGC(display, XtWindow(w), valuemask, &values);
        XDrawArc(display, XtWindow(w), gc, x-radius, y-radius,
                2*radius, 2*radius, 360*64, 360*64);
        XFreeGC(display, gc);
        XFlush(display);
}


void Buttonpress(Widget w, XtPointer client_data, XEvent* event,
                    Boolean *continue_to_dispatch)
{
	static float old_scale = 0.0;
        Display *display = XtDisplay(w);
        XColor red, blue, green;
	Point p;
        red.flags = blue.flags = green.flags = DoRed | DoGreen | DoBlue;
        red.red = 65535; red.green = 0; red.blue = 0;
	blue.red = 0; blue.green = 0; blue.blue = 65535;
	green.red = 0; green.blue = 0; green.green = 65535;
        XAllocColor(display, cmap, &blue);
	XAllocColor(display, cmap, &red);
	XAllocNamedColor(display, cmap, "lime green", &green, &green);

        XSetLineAttributes(display, gc, 2, LineSolid, CapButt, JoinRound);
	if (mode == LEVEL_CURVE)
        	XSetForeground(display, gc, red.pixel);
	else if (mode == GRADIENT_FLOW_LINE)
		XSetForeground(display, gc, blue.pixel);
	else if (mode == HEIGHT_RIDGE || mode == HEIGHT_VALLEY)
		XSetForeground(display, gc, green.pixel);
	else 
	XSetForeground(display, gc, red.pixel);

        p.x = event->xbutton.x;
        p.y = event->xbutton.y;
	curve_length(C) = 0;
	switch(event->xbutton.button) {
                case 1:
                        draw_circ(w, (int) p.x, (int) p.y, (int) scale);
			if (scale != old_scale && scale != 0.0) {
				old_scale = scale;
   			}
			tracker(cimage_pixels(im), 
				cimage_xdim(im), cimage_ydim(im), 
				p.x, p.y, scale, 
				draw_func, (void *) &imageWidget, mode);
                        draw_circ(w, (int) p.x, (int) p.y, (int) scale);
                break;
                case 2:
			old_scale = scale;
                        draw_circ(w, (int) p.x, (int) p.y, (int) scale);
                        scale -= 1;
			if (scale < 0.0) scale = 0.0;
			printf("Scale is %d\n", (int) scale);
                        draw_circ(w, (int) p.x, (int) p.y, (int) scale);
                break;
                case 3:
			old_scale = scale;
                        draw_circ(w, (int) p.x, (int) p.y, (int) scale);
                        scale += 1;
			printf("Scale is %d\n", (int) scale);
                        draw_circ(w, (int) p.x, (int) p.y, (int) scale);
                break;
        }

}

void quitCB(Widget w, XtPointer client_data, XtPointer call_data)
{
        exit(0);
}

void clearCB(Widget w, XtPointer client_data, XtPointer call_data)
{
	XClearWindow(XtDisplay(w), XtWindow(imageWidget));
}

void levelCB(Widget w, XtPointer client_data, XtPointer call_data)
{
        mode = LEVEL_CURVE;
        printf("Computing level curves\n");
}

void gradflowCB(Widget w, XtPointer client_data, XtPointer call_data)
{
        mode = GRADIENT_FLOW_LINE;
        printf("Computing gradient flow lines\n");
}

void lapCB(Widget w, XtPointer client_data, XtPointer call_data)
{
	mode = LAPLACIAN_ZERO;
	printf("Computing laplacian zero crossing\n");
}
void lwwCB(Widget w, XtPointer client_data, XtPointer call_data)
{
        mode = SECOND_DIRECTIONAL_DERIVATIVE_ZERO;
        printf("Computing Lpp zero crossing\n");
}

void ridgeCB(Widget w, XtPointer client_data, XtPointer call_data)
{
        mode = HEIGHT_RIDGE;
        printf("Computing intensity ridges\n");
}

void valleyCB(Widget w, XtPointer client_data, XtPointer call_data)
{
        mode = HEIGHT_VALLEY;
        printf("Computing intensity valleys\n");
}

void ridgeongradmagCB(Widget w, XtPointer client_data, XtPointer call_data)
{
        mode = RIDGE_ON_GRAD_MAG;
        printf("Computing ridges on gradient magnitude\n");
}

void ridgeonlaplacianCB(Widget w, XtPointer client_data, XtPointer call_data)
{
        mode = RIDGE_ON_LAPLACIAN;
        printf("Computing ridges on laplacian\n");
}


void ridgeflowCB(Widget w, XtPointer client_data, XtPointer call_data)
{
        mode = RIDGE_FLOW_LINES;
        printf("Computing ridge flow lines\n");
}

void Pointermotion(Widget w, XtPointer client_data, XEvent* event,
                    Boolean *continue_to_dispatch)
{
        Point p;
        p.x = event->xbutton.x;
        p.y = event->xbutton.y;
        draw_circ(w, prev_x, prev_y, (int) scale);
        draw_circ(w, (int) p.x, (int) p.y, (int) scale);
        prev_x = (int) p.x; prev_y = (int) p.y;
}

void Enternotify(Widget w, XtPointer client_data, XEvent* event,
                    Boolean *continue_to_dispatch)
{
        Point p;
        p.x = event->xbutton.x;
        p.y = event->xbutton.y;
        draw_circ(w, (int) p.x, (int) p.y, (int) scale);
        prev_x = (int) p.x; prev_y = (int) p.y;
}

void Leavenotify(Widget w, XtPointer client_data, XEvent* event,
                    Boolean *continue_to_dispatch)
{
        draw_circ(w, prev_x, prev_y, (int) scale);
}


void ParseCommLine(int argc, char **argv);

main (int argc, char **argv)
{
    int x,y;
    XtAppContext app_context;
    Widget topLevel, mainWidget, controlWidget;
    Widget mainWindow;
    Widget modeFrame;
    Widget modeWidget, lapWidget, lwwWidget, levelWidget, gradflowWidget,
	   ridgeWidget, valleyWidget, ridgeflowWidget, ridgeongradmagWidget,
	   ridgeonlaplacianWidget;
    Widget buttonFrame, buttonForm;
    Widget frameTitle;
    cimage kernel=cimage_init();
    cimage work=cimage_init();
    im = cimage_init();

    if ( argc == 1 ) Usage();

    ParseCommLine(argc, argv);

    C = curve_init(2000);

	//--------------------------------------------------------------------
	// Read the Grid and pack its first frame into a data array of float
	// type, then fill in the image format.
	char *filenames[2];
	filenames[0] = imagefile;

//	BCGrid *grid = new BCGrid(512,512, 0,0, 1,1, 1, READ_FILE, filenames,1);
	BCGrid *grid = new BCGrid((short)512,(short)512, 0.0,1.0,0.0,1.0, 
		READ_FILE, (const char **)filenames, 1);
	BCPlane *plane = grid->head();

	printf("Using plane %s\n",plane->name()->c_str());
	CREALTYPE *pixels = new float[grid->numX() * grid->numY()];
	for (y = 0; y < grid->numY(); y++) {
	  for (x = 0; x < grid->numX(); x++) {
		pixels[x + y*grid->numX()] = plane->value(x,y);
	  }
	}

	printf("Filling in /usr/image structure\n");
	cimage_setformat(im, CREAL);
	im->pixels = (char*)pixels;
	im->fname = imagefile;
	im->xdim = grid->numX();
	im->ydim = grid->numY();
	im->zdim = 1;
	im->imin = plane->minValue();
	im->imax = plane->maxValue();
	im->appdata = NULL;

        // conversion factors which give number of nm per pixel for x and y
        xConvFactor = (plane->maxX()-plane->minX())/((double)grid->numX()-1.0);
        yConvFactor = (plane->maxY()-plane->minY())/((double)grid->numY()-1.0);

//XXX    cimage_read(im, imagefile);
//XXX    cimage_info(imagefile, im);

    printf("Initializing...");
    topLevel = XtInitialize(argv[0],
		"lctrace", 
		NULL, 0,
		&argc, argv);
    printf("Done\n");
    gc = DefaultGC(XtDisplay(topLevel), DefaultScreen(XtDisplay(topLevel)));

	cmap_info.num_greys = 94;
        cmap = init_color_table(&cmap_info, XtDisplay(topLevel));

        XtVaSetValues(topLevel,
                        XmNcolormap, cmap,
                        NULL);

	mainWidget = XtVaCreateManagedWidget(
                        "main",
                        xmFormWidgetClass,
                        topLevel,
                        NULL);

        imageWidget = XtVaCreateManagedWidget(
                        "image",
                        xmDrawingAreaWidgetClass,
                        mainWidget,
                        XmNwidth, cimage_xdim(im),
                        XmNheight, cimage_ydim(im),
                        NULL);

        controlWidget = XtVaCreateManagedWidget(
                        "controlWidget",
                        xmRowColumnWidgetClass,
                        mainWidget,
                        XmNtopAttachment, XmATTACH_WIDGET,
                        XmNtopWidget, imageWidget,
                        XmNorientation, XmHORIZONTAL,
                        NULL);

	modeFrame = XtVaCreateManagedWidget(
                        "modeFrame",
                        xmFrameWidgetClass,
                        controlWidget,
                        XmNshowSeparator, True,
                        NULL);

	buttonFrame = XtVaCreateManagedWidget(
                        "buttonFrame",
                        xmFrameWidgetClass,
                        controlWidget,
                        XmNshowSeparator, True,
                        NULL);

	buttonForm = XtVaCreateManagedWidget(
                        "buttonForm",
                        xmRowColumnWidgetClass,
                        buttonFrame,
                        NULL);

	modeWidget = XtVaCreateManagedWidget(
                        "modeWidget",
                        xmRowColumnWidgetClass,
                        modeFrame,
			XmNradioBehavior, True,
			XmNradioAlwaysOne, True,
                        NULL);

	levelWidget = XtVaCreateManagedWidget(
                        "Level curves",
                        xmToggleButtonWidgetClass,
                        modeWidget,
                        NULL);
	XtAddCallback(levelWidget,
                        XmNvalueChangedCallback,
                        (XtCallbackProc) levelCB,
                        (XtPointer) NULL);	
	gradflowWidget = XtVaCreateManagedWidget(
                        "Gradient flow lines",
                        xmToggleButtonWidgetClass,
                        modeWidget,
                        NULL);
        XtAddCallback(gradflowWidget,
                        XmNvalueChangedCallback,
                        (XtCallbackProc) gradflowCB,
                        (XtPointer) NULL);
	ridgeWidget = XtVaCreateManagedWidget(
                        "Intensity ridges",
                        xmToggleButtonWidgetClass,
                        modeWidget,
                        NULL);
        XtAddCallback(ridgeWidget,
                        XmNvalueChangedCallback,
                        (XtCallbackProc) ridgeCB,
                        (XtPointer) NULL);
	valleyWidget = XtVaCreateManagedWidget(
                        "Intensity valleys",
                        xmToggleButtonWidgetClass,
                        modeWidget,
                        NULL);
        XtAddCallback(valleyWidget,
                        XmNvalueChangedCallback,
                        (XtCallbackProc) valleyCB,
                        (XtPointer) NULL);

	ridgeongradmagWidget = XtVaCreateManagedWidget(
                        "Ridge on grad mag",
                        xmToggleButtonWidgetClass,
                        modeWidget,
                        NULL);
        XtAddCallback(ridgeongradmagWidget,
                        XmNvalueChangedCallback,
                        (XtCallbackProc) ridgeongradmagCB,
                        (XtPointer) NULL);


	ridgeonlaplacianWidget = XtVaCreateManagedWidget(
                        "Ridge on Laplacian",
                        xmToggleButtonWidgetClass,
                        modeWidget,
                        NULL);
        XtAddCallback(ridgeonlaplacianWidget,
                        XmNvalueChangedCallback,
                        (XtCallbackProc) ridgeonlaplacianCB,
                        (XtPointer) NULL);
	/*ridgeflowWidget = XtVaCreateManagedWidget(
                        "Ridge flow lines",
                        xmToggleButtonWidgetClass,
                        modeWidget,
                        NULL);
        XtAddCallback(ridgeflowWidget,
                        XmNvalueChangedCallback,
                        (XtCallbackProc) ridgeflowCB,
                        (XtPointer) NULL);*/
	lapWidget = XtVaCreateManagedWidget(
                        "Laplacian zeros",
                        xmToggleButtonWidgetClass,
                        modeWidget,
                        NULL);
        XtAddCallback(lapWidget,
                        XmNvalueChangedCallback,
                        (XtCallbackProc) lapCB,
                        (XtPointer) NULL);

        lwwWidget = XtVaCreateManagedWidget(
                        "Second directional derivative zeros",
                        xmToggleButtonWidgetClass,
                        modeWidget,
                        NULL);
	XtAddCallback(lwwWidget,
                        XmNvalueChangedCallback,
                        (XtCallbackProc) lwwCB,
                        (XtPointer) NULL);
	XtVaSetValues(levelWidget, XmNset, True, NULL);		

#if (XmVersion > 1001)
        frameTitle = XtVaCreateManagedWidget("Trace Options",
                        xmLabelWidgetClass, buttonFrame,
                        XmNchildType, XmFRAME_TITLE_CHILD,
                        XmNchildHorizontalAlignment,
                                XmALIGNMENT_CENTER,
                        XmNchildVerticalAlignment,
                                XmALIGNMENT_BASELINE_BOTTOM,
                        NULL);
	frameTitle = XtVaCreateManagedWidget("Mode Options",
                        xmLabelWidgetClass, modeFrame,
                        XmNchildType, XmFRAME_TITLE_CHILD,
                        XmNchildHorizontalAlignment,
                                XmALIGNMENT_CENTER,
                        XmNchildVerticalAlignment,
                                XmALIGNMENT_BASELINE_BOTTOM,
                        NULL);
#endif


	XtAddEventHandler(imageWidget, ButtonPressMask, False,
                          (XtEventHandler) Buttonpress,
                          (XtPointer) NULL);
	XtAddEventHandler(imageWidget, PointerMotionMask, False,
                          (XtEventHandler) Pointermotion,
                          (XtPointer) NULL);
        XtAddEventHandler(imageWidget, EnterWindowMask, False,
                          (XtEventHandler) Enternotify,
                          (XtPointer) NULL);
        XtAddEventHandler(imageWidget, LeaveWindowMask, False,
                          (XtEventHandler) Leavenotify,
                          (XtPointer) NULL);

	pixmap_to_widget(cimage_pixmap(im, &cmap_info, XtDisplay(topLevel)),
                         imageWidget);

	if (cimage_format(im) != CREAL) {
                cimage work = cimage_init();
                cimage_warning("Converting image to real format");
                cimage_convert(im, work, CREAL);
                cimage_copy(work, im);
                cimage_destroy(work);
        }
	

	CreatePushbutton(buttonForm, "CLEAR",
			(XtCallbackProc) clearCB, (XtPointer) NULL);

	CreatePushbutton(buttonForm, "QUIT", 
			(XtCallbackProc) quitCB, (XtPointer) NULL);


        XtRealizeWidget(topLevel);
        XtMainLoop();
}


void ParseCommLine(int argc, char **argv)
{
  char *s;
  printf("Parsing command line\n");
  tmpDir[0] = '\0';
  while (--argc > 0 && (*++argv)[0] == '-')
    {
      for (s=argv[0]+1; *s; s++)
        switch(*s) {
        case 's' :
          if (sscanf(*++argv, "%f", &sampling)==0)
            {
              cimage_warning("Missing sampling.");
              Usage();
            }
          argc--;
          break;
        case 'p' :
          if (sscanf(*++argv, "%f", &scale)==0)
            {
              cimage_warning("Missing preblur parameter.");
              Usage();
            }
          argc--;
        break;
        case 't' :
          if (sscanf(*++argv, "%s", tmpDir)==0)
            {
	      cimage_warning("Missing tmpDir name.");
              Usage();
            }
          argc--;
          break;
        case 'f' :
          if (sscanf(*++argv, "%s", edgefile)==0)
            {
	      cimage_warning("Missing edge filename.");
              Usage();
            }

            if (tmpDir[0] != '\0')
            {
                strcat(tmpDir,edgefile);
                outfile = fopen(tmpDir, "w+");
            }
            else
                outfile = fopen(edgefile, "w+");
            if (outfile == NULL)
		cimage_error("Problem opening output file.");

          argc--;
          break;
	case 'm' :
	  if (sscanf(*++argv, "%d", &mode)==0)
            {
	      cimage_warning("Missing mode.");
              Usage();
            }
	  argc--;
	 break;

        }
  }
  if(sscanf(*argv++, "%s", imagefile)==0)
    cimage_error("Cannot get input filename");
}


/*===3rdtech===
  Copyright (c) 2001 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(__CYGWIN__) || !defined(_WIN32)
#include <strings.h>
#endif

#ifdef V_GLUT
#	include <GL/glut_UNC.h>
#else
#	include <GL/glx.h> 
#	include <GL/gl.h>
#       include <X11/cursorfont.h>
#endif

#ifndef WIN32
#include <unistd.h>
#endif

#include <assert.h>
#include <math.h>

#include <vrpn_Types.h>
#include <nmb_ColorMap.h>

#include "imageViewer.h"

#ifndef M_PI
#define M_PI           3.14159265358979323846
#endif

ImageViewer *ImageViewer::theViewer = NULL;	// pointer to singleton

ImageWindow::ImageWindow():
    id(0),
#ifdef V_GLUT
    win_id(0), left_button_down(VRPN_FALSE),
    middle_button_down(VRPN_FALSE), right_button_down(VRPN_FALSE),
#else
    win(NULL),
#endif
    win_width(0), win_height(0), im_width(0), im_height(0),
    display_index(0), visible(VRPN_FALSE), needs_redisplay(VRPN_FALSE),
    image(NULL), min_value(0), max_value(1.0),
    event_handler(NULL), event_handler_ud(NULL),
    display_handler(NULL), display_handler_ud(NULL),
    d_colormap(NULL), 
    d_data_min(0), d_data_max(1), d_color_min(0), d_color_max(1)
{}

ImageDisplay::ImageDisplay():
    dpy_name(NULL)
#ifdef V_GLUT
#else
    ,x_dpy(NULL), vi(NULL),
    cx(0), cmap(0)
#endif
{}

ImageViewer *ImageViewer::getImageViewer(){
    if (!ImageViewer::theViewer)
	ImageViewer::theViewer = new ImageViewer();
    return ImageViewer::theViewer;
}

ImageViewer::ImageViewer():
    num_displays(0),
    num_windows(0)
#ifndef V_GLUT
    ,font_dlist_base(0)
#endif
{
    if (theViewer) {
	fprintf(stderr, "Error: ImageViewer: cannot create more than one "
		"instance\n");
	return;
    }


#ifdef V_GLUT
#else
    event_mask = StructureNotifyMask|KeyPressMask|ExposureMask|
		ButtonPressMask|ButtonReleaseMask|PointerMotionMask;
#endif
}

int ImageViewer::init(char *display) {
    // avoid duplication of display entries
    for (int dpy_index = 0; dpy_index < num_displays; dpy_index++){
         if (display == NULL){
             if (dpy[dpy_index].dpy_name == NULL) return 0;
         }
         else if (strcmp(display, dpy[dpy_index].dpy_name) == 0) return 0;
    }

#ifdef V_GLUT
#else
    static int attributeList[] = { GLX_RGBA,GLX_DOUBLEBUFFER,
					GLX_DEPTH_SIZE, 8, None};
    // Return values dpy, vi, cx, cmap, and win should all be tested
    Display *x_dpy;
    XVisualInfo *vi;
    GLXContext cx;
    Colormap cmap;
    
    x_dpy = XOpenDisplay(display);
    if (!x_dpy) {
	fprintf(stderr, "Error: couldn't open display %s\n", display);
	return -1;
    }
    vi = glXChooseVisual(x_dpy, DefaultScreen(x_dpy), attributeList);
    if (!vi) {
	fprintf(stderr, "Error: couldn't get required visual on display %s\n",
		display);
	return -1;
    }
    cx = glXCreateContext(x_dpy, vi, 0, GL_TRUE);
    if (!cx) {
	fprintf(stderr, "Error: couldn't create a GLX context for display %s\n",
		display);
	return -1;
    }
    cmap= XCreateColormap(x_dpy, RootWindow(x_dpy, 
					vi->screen),
				vi->visual, AllocNone);


    dpy[num_displays].x_dpy = x_dpy;
    dpy[num_displays].vi = vi;
    dpy[num_displays].cx = cx;
    dpy[num_displays].cmap = cmap;
#endif

    if (display == NULL) dpy[num_displays].dpy_name = NULL;
    else
	dpy[num_displays].dpy_name = strdup(display);
    num_displays++;

    return 0;
}

#ifndef V_GLUT
static Bool WaitForMapNotify(Display *d, XEvent *e, char *arg) {
      return (e->type == MapNotify) && (e->xmap.window == (Window)arg); }

static Bool WaitForUnmapNotify(Display *d, XEvent *e, char *arg) {
      return (e->type == UnmapNotify) && (e->xmap.window == (Window)arg); }
#endif

/**
 * creates a window on the given display
 * @param display_name name of a display (e.g. unix:0.0)
 * @param x x position of window
 * @param y y position of window
 * @param w width of window
 * @param h height of window
 * @param name title for window
 * @param pixelType can either be GL_FLOAT or GL_UNSIGNED_BYTE. If you use 
 *                 GL_UNSIGNED_BYTE then it is possible to use the more
 *                 efficient setScanline() function to update the image data
 * @return The window id which references this window or 0 if error
*/
int ImageViewer::createWindow(char *display_name,
                      int x, int y, int w, int h, char *win_name,
                      int pixelType){
    int dpy_index;
    for (dpy_index = 0; dpy_index < num_displays; dpy_index++){
		if (display_name == NULL){
			if (dpy[dpy_index].dpy_name == NULL) break;
		}
		else if (dpy[dpy_index].dpy_name == NULL) continue;
		else if (strcmp(display_name, dpy[dpy_index].dpy_name) == 0) break;
    }
    if (dpy_index == num_displays){
	if (init(display_name)) return 0;
	dpy_index = num_displays - 1;
    }

#ifndef V_GLUT
    window[num_windows].swa.colormap = dpy[dpy_index].cmap;
    window[num_windows].swa.border_pixel = 0;
    window[num_windows].swa.event_mask = event_mask;

    Window *win;
    win = new Window;
    *win = XCreateWindow(dpy[dpy_index].x_dpy, 
				RootWindow(dpy[dpy_index].x_dpy,
					dpy[dpy_index].vi->screen),
					x, y, w, h, 0, 
					dpy[dpy_index].vi->depth,
					InputOutput, dpy[dpy_index].vi->visual,
					CWBorderPixel|CWColormap|CWEventMask, 
					&(window[num_windows].swa));

    XStoreName(dpy[dpy_index].x_dpy, *win, win_name);
    window[num_windows].cursor = 
	XCreateFontCursor(dpy[dpy_index].x_dpy, XC_crosshair);
    XDefineCursor(dpy[dpy_index].x_dpy, *win, window[num_windows].cursor);
    window[num_windows].win = win;
#else
    //glutInitWindowSize(w,h); - this should be done by user
    //glutInitWindowPosition(x,y);
    //glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    //int win_save = glutGetWindow(); // this was necessary until I added
	// v_gl_set_context_to_vlib_window() and inserted it before all
	// display list generators for stuff to be drawn to vlib window
	// If something doesn't draw and you're using display lists then
	// try uncommenting this and the corresponding glutSetWindow
	// below to see if that fixes it (AAS)
    int wid = glutCreateWindow(win_name);
    glutHideWindow();
    window[num_windows].win_id = wid;

    glutReshapeWindow(w, h);
    glutPositionWindow(x,y);
    //glutIdleFunc(ImageViewer::idleCallbackForGLUT); 
    glutDisplayFunc(ImageViewer::displayCallbackForGLUT);

    glutVisibilityFunc(ImageViewer::visibilityCallbackForGLUT);
    glutReshapeFunc(ImageViewer::reshapeCallbackForGLUT);
    glutMotionFunc(ImageViewer::motionCallbackForGLUT);
    glutMouseFunc(ImageViewer::mouseCallbackForGLUT);
    glutKeyboardFunc(ImageViewer::keyboardCallbackForGLUT);
    glutSpecialFunc(ImageViewer::specialCallbackForGLUT);
    //glutSetWindow(win_save);
#endif

    window[num_windows].d_pixelMode = pixelType;
    window[num_windows].display_index = dpy_index;
    window[num_windows].id = num_windows+1;
    window[num_windows].win_width = w;
    window[num_windows].win_height = h;
    window[num_windows].im_width = 0;
    window[num_windows].im_height = 0;
    window[num_windows].im_x_per_pixel = (float)window[num_windows].im_width /
                               (float)window[num_windows].win_width;
    window[num_windows].im_y_per_pixel = (float)window[num_windows].im_height /
                               (float)window[num_windows].win_height;
    window[num_windows].image = NULL;

    int i;
    if ((window[num_windows].im_width > 0) && 
        (window[num_windows].im_height > 0)) {
      if (pixelType == GL_FLOAT) {
        window[num_windows].image = 
                             new float[window[num_windows].im_width*
					window[num_windows].im_height];
        for (i = 0; i < window[num_windows].im_width*
			window[num_windows].im_height; i++){
          ((float *)(window[num_windows].image))[i] = 0.0;
        }
      } else if (pixelType == GL_UNSIGNED_BYTE){
        window[num_windows].image =   
                   new vrpn_uint8[window[num_windows].im_width*
                                  window[num_windows].im_height];
        for (i = 0; i < window[num_windows].im_width*
                        window[num_windows].im_height; i++){
          ((vrpn_uint8 *)(window[num_windows].image))[i] = 0;
        }
      } else {
        fprintf(stderr,
             "ImageViewer::createWindow: Error, unknown pixel type\n");
        return 0;
      }
    }
    num_windows++;
//    showWindow(num_windows);
    return num_windows;
}

int ImageViewer::destroyWindow(int winID)
{
    if (!validWinID(winID)) return -1;
    int win_index = get_window_index_from_winID(winID);

#ifdef V_GLUT
    glutSetWindow(window[win_index].win_id);
    glFinish();
    glutDestroyWindow(window[win_index].win_id);
#else
    glXMakeCurrent(dpy[window[win_index].display_index].x_dpy,
          *(window[win_index].win), dpy[window[win_index].display_index].cx);
    glFinish();
    XDestroyWindow(dpy[window[win_index].display_index].x_dpy,
              *(window[win_index].win));
    delete window[win_index].win;
#endif
 
    if (window[win_index].image){
      delete [] window[win_index].image;
    }

    window[win_index] = window[num_windows-1];

    num_windows--;
    return 0;
}

int ImageViewer::setWindowSize(int winID, int w, int h)
{
    if (!validWinID(winID)) return -1;
    int win_index = get_window_index_from_winID(winID);

#ifdef V_GLUT
    glutSetWindow(window[win_index].win_id);
    glutReshapeWindow(w, h);
#else
    XResizeWindow(dpy[window[win_index].display_index].x_dpy,
             *(window[win_index].win), w, h);
#endif
    return 0;
}

#ifdef V_GLUT
int ImageViewer::get_window_index_from_glut_id(int glut_id) {

    int i;
    for (i = 0; i < num_windows; i++)
	if (window[i].win_id == glut_id)
	    return i;
    return -1;
}
#endif

int ImageViewer::validWinID(int winID)
{
  return (winID > 0 && winID <= num_windows);
}

int ImageViewer::get_window_index_from_winID(int winID) {
    return winID-1;
}

int ImageViewer::get_winID_from_window_index(int win_index) {
    return win_index+1;
}

int ImageViewer::showWindow(int winID){
    if (!validWinID(winID)) return -1;
    int win_index = get_window_index_from_winID(winID);

    //window[win_index].win_width = window[win_index].im_width;
    //window[win_index].win_height = window[win_index].im_height;
    window[win_index].im_x_per_pixel = (float)window[win_index].im_width /
                               (float)window[win_index].win_width;
    window[win_index].im_y_per_pixel = (float)window[win_index].im_height /
                               (float)window[win_index].win_height;

    int x_loc, y_loc;

    if (window[win_index].visible) {
#ifdef V_GLUT
        glutSetWindow(window[win_index].win_id);
        glutPopWindow();
#else
	XRaiseWindow(dpy[window[win_index].display_index].x_dpy,
		(*(window[win_index].win)));	
#endif
	return 0;
    }

#ifdef V_GLUT
    int curr_win = glutGetWindow();
    glutSetWindow(window[win_index].win_id); // the GLUT window ID
    glutShowWindow();
#else
    XMapWindow(dpy[window[win_index].display_index].x_dpy, 
		(*(window[win_index].win)));
    printf(".");
    XIfEvent(dpy[window[win_index].display_index].x_dpy, &event, 
		WaitForMapNotify, (char *)(*(window[win_index].win)));
    printf(".");
#endif
    if (win_index == 1) printf("\n");

    // some code to tile the windows
    int i;
    x_loc = 100; y_loc = 100;
    int height_max = 0;
    for (i = 0; i < num_windows; i++){
	if (window[i].visible && 
		window[win_index].display_index == window[i].display_index) {
		x_loc += window[i].win_width + 20;
		if (height_max < window[i].win_height)
			height_max = window[i].win_height;
		if ((x_loc+window[win_index].im_width) > 1280){
			x_loc = 100;
			y_loc += height_max + 30; // add a little for border
			height_max = 0;
	    	}
	}
    }


#ifdef V_GLUT
    // this caused problems in non-cygwin winNT
//#if (!defined(_WIN32) || defined(__CYGWIN__))
/*
    glutPositionWindow(x_loc,y_loc);
    glutReshapeWindow(window[win_index].im_width, window[win_index].im_height);
*/
//#endif
#else
/*
    XMoveResizeWindow(dpy[window[win_index].display_index].x_dpy,
                (*(window[win_index].win)), x_loc, y_loc,
            window[win_index].im_width,
            window[win_index].im_height);
*/
#endif

    window[win_index].visible = VRPN_TRUE;

#ifdef V_GLUT
    glutSetWindow(window[win_index].win_id);
    glutPopWindow();
    glutSetWindow(curr_win);
#else
    XRaiseWindow(dpy[window[win_index].display_index].x_dpy,
                 (*(window[win_index].win)));
#endif



    dirtyWindow(winID);
    return 0;
}

int ImageViewer::hideWindow(int winID){
    if (!validWinID(winID)) return -1;
    int win_index = get_window_index_from_winID(winID);

    if (!(window[win_index].visible)) {
	fprintf(stderr, "Warning: window wasn't visible\n");
	return 0;
    }
#ifdef V_GLUT
    int curr_win = glutGetWindow();
    glutSetWindow(window[win_index].win_id); // the GLUT window ID
    glutHideWindow();
    glutSetWindow(curr_win);
#else
    XUnmapWindow(dpy[window[win_index].display_index].x_dpy, 
	(*(window[win_index].win)));
    XIfEvent(dpy[window[win_index].display_index].x_dpy, &event, 
	WaitForUnmapNotify, (char *)(*(window[win_index].win)));
#endif
    window[win_index].visible = VRPN_FALSE;
    return 0;
}

int ImageViewer::dirtyWindow(int winID) {
    if (!validWinID(winID)) return -1;
    int win_index = get_window_index_from_winID(winID);

    window[win_index].needs_redisplay = VRPN_TRUE;

#ifdef V_GLUT
    int curr_win = glutGetWindow();
    glutSetWindow(window[win_index].win_id);
    glutPostRedisplay();
    glutSetWindow(curr_win);
#endif

    return 0;
}

void ImageViewer::mainloop() {
#ifdef V_GLUT // event processing will be done in glut callbacks
#else
    int i;
    ImageViewerWindowEvent ivw_event;
    for (i = 0; i < num_windows; i++){

	    glXMakeCurrent(dpy[window[i].display_index].x_dpy,
                *(window[i].win), dpy[window[i].display_index].cx);

	while (XCheckWindowEvent(dpy[window[i].display_index].x_dpy, 
		*(window[i].win), event_mask, &event)) {
	    switch(event.type) {
		case Expose:
		    window[i].needs_redisplay = VRPN_TRUE;
		    break;
		case ConfigureNotify:
		    window[i].win_width = event.xconfigure.width;
		    window[i].win_height = event.xconfigure.height;
		    window[i].im_x_per_pixel = (float)window[i].im_width /
                               (float)window[i].win_width;
    		    window[i].im_y_per_pixel = (float)window[i].im_height /
                               (float)window[i].win_height;
		    window[i].needs_redisplay = VRPN_TRUE;
		    ivw_event.type = RESIZE_EVENT;
		    ivw_event.width = window[i].win_width;
		    ivw_event.height = window[i].win_height;
		    ivw_event.winID = get_winID_from_window_index(i);
		    if (window[i].event_handler)
			window[i].event_handler(ivw_event, 
				window[i].event_handler_ud);
		    break;
		case ButtonPress:
		    ivw_event.type = BUTTON_PRESS_EVENT;
		    ivw_event.winID = get_winID_from_window_index(i);
		    ivw_event.mouse_x = event.xbutton.x;
		    ivw_event.mouse_y = event.xbutton.y;
		    switch(event.xbutton.button) {
			case Button1: 
				ivw_event.button = IV_LEFT_BUTTON;
				break;
			case Button2:
				ivw_event.button = IV_MIDDLE_BUTTON;
				break;	
			case Button3:
				ivw_event.button = IV_RIGHT_BUTTON;
				break;
			default:
				break;
		    }
		    ivw_event.state = 0;
		    if (event.xbutton.state & Button1Mask)
			ivw_event.state |= IV_LEFT_BUTTON_MASK;
		    if (event.xbutton.state & Button2Mask)
			ivw_event.state |= IV_MIDDLE_BUTTON_MASK;
		    if (event.xbutton.state & Button3Mask)
			ivw_event.state |= IV_RIGHT_BUTTON_MASK;
		    if (window[i].event_handler)
			window[i].event_handler(ivw_event, 
				window[i].event_handler_ud);
		    break;
		case ButtonRelease:
		    ivw_event.type = BUTTON_RELEASE_EVENT;
		    ivw_event.winID = get_winID_from_window_index(i);
		    ivw_event.mouse_x = event.xbutton.x;
                    ivw_event.mouse_y = event.xbutton.y;
		    switch(event.xbutton.button) {
			case Button1:
				ivw_event.button = IV_LEFT_BUTTON;
				break;
			case Button2:
				ivw_event.button = IV_MIDDLE_BUTTON;
				break;
			case Button3:
				ivw_event.button = IV_RIGHT_BUTTON;
				break;
			default:
				break;
		    }
                    ivw_event.state = 0;
                    if (event.xbutton.state & Button1Mask)
                        ivw_event.state |= IV_LEFT_BUTTON_MASK;
                    if (event.xbutton.state & Button2Mask)
                        ivw_event.state |= IV_MIDDLE_BUTTON_MASK;
                    if (event.xbutton.state & Button3Mask)
                        ivw_event.state |= IV_RIGHT_BUTTON_MASK;

                    if (window[i].event_handler)
                        window[i].event_handler(ivw_event, 
				window[i].event_handler_ud);    
                    break;
		case MotionNotify:
		    ivw_event.type = MOTION_EVENT;
                    ivw_event.winID = get_winID_from_window_index(i);
                    ivw_event.mouse_x = event.xmotion.x;
                    ivw_event.mouse_y = event.xmotion.y;
                    ivw_event.state = event.xmotion.state;

                    ivw_event.state = 0;
                    if (event.xmotion.state & Button1Mask)
                        ivw_event.state |= IV_LEFT_BUTTON_MASK;
                    if (event.xmotion.state & Button2Mask)
                        ivw_event.state |= IV_MIDDLE_BUTTON_MASK;
                    if (event.xmotion.state & Button3Mask)
                        ivw_event.state |= IV_RIGHT_BUTTON_MASK;

		    if (window[i].event_handler)
			window[i].event_handler(ivw_event, 
				window[i].event_handler_ud);
		    break;
		case KeyPress:
		    ivw_event.type = KEY_PRESS_EVENT;
                    ivw_event.winID = get_winID_from_window_index(i);
		    ivw_event.mouse_x = event.xkey.x;
                    ivw_event.mouse_y = event.xkey.y;

                    ivw_event.state = 0;
                    if (event.xkey.state & Button1Mask)
                        ivw_event.state |= IV_LEFT_BUTTON_MASK;
                    if (event.xkey.state & Button2Mask)
                        ivw_event.state |= IV_MIDDLE_BUTTON_MASK;
                    if (event.xkey.state & Button3Mask)
                        ivw_event.state |= IV_RIGHT_BUTTON_MASK;

		   //ivw_event.keycode = event.xkey.keycode;

         // Above line replaced with following block.
         // Stolen from glut ... this handles key mappings under X.
         // This makes most special keys send '\0' to the handler ...
         // Do we care?
         // Weigle - 02/10/00
         {
            char tmp[1];
            XLookupString((XKeyEvent *)&event.xkey, tmp, sizeof(tmp), NULL, NULL);
            ivw_event.keycode = tmp[0];
         }
                    if (window[i].event_handler)
                        window[i].event_handler(ivw_event, 
				window[i].event_handler_ud);
                    break;
		default:
		    break;
	    }
	}
	// now we should have exausted the queue for this window so we go on to
	// updating the image if necessary
	if (window[i].needs_redisplay && window[i].visible){
//	    printf("calling glXWaitX()\n");
	    glXWaitX();
	    glXMakeCurrent(dpy[window[i].display_index].x_dpy,
                *(window[i].win), dpy[window[i].display_index].cx);
	    glDrawBuffer(GL_BACK_LEFT); // for double buffering
	    glClearColor(0.0, 0.0, 0.0,0.0);

	    ImageViewerDisplayData ivdd;
	    ivdd.winID = get_winID_from_window_index(i);
	    ivdd.winWidth = window[i].win_width;
	    ivdd.winHeight = window[i].win_height;
	    ivdd.imWidth = window[i].im_width;
	    ivdd.imHeight = window[i].im_height;

	    // HERE'S WHERE THE IMAGE IS DRAWN
	    if (window[i].display_handler)
		window[i].display_handler(ivdd, window[i].display_handler_ud);
	    else
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	    glXSwapBuffers(dpy[window[i].display_index].x_dpy, 
		*(window[i].win));
//	    printf("calling glXWaitGL()\n");
	    glXWaitGL();
//	    printf("done with update\n");
	    window[i].needs_redisplay = VRPN_FALSE; 
	}
    }
#endif
}

#ifdef V_GLUT
// static
void ImageViewer::displayCallbackForGLUT() {
    int curr_win = glutGetWindow();
    ImageViewer *v = theViewer;
    int i = v->get_window_index_from_glut_id(curr_win);
    if (i < 0) {
	fprintf(stderr, "Error, ImageViewer::displayCallback, window not found\n");
	return;
    }
    glDrawBuffer(GL_BACK_LEFT);
    glClearColor(0.0, 0.0, 0.0, 0.0);
    ImageViewerDisplayData ivdd;
    ivdd.winID = v->get_winID_from_window_index(i);
    ivdd.winWidth = v->window[i].win_width;
    ivdd.winHeight = v->window[i].win_height;
    ivdd.imWidth = v->window[i].im_width;
    ivdd.imHeight = v->window[i].im_height;

    // HERE'S WHERE THE IMAGE IS DRAWN
    if (v->window[i].display_handler)
         v->window[i].display_handler(ivdd, 
		v->window[i].display_handler_ud);
    else
         glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glutSwapBuffers();
    v->window[i].needs_redisplay = VRPN_FALSE; 
}

void ImageViewer::visibilityCallbackForGLUT(int state) {
    int curr_win = glutGetWindow();
    ImageViewer *v = theViewer;
    int i = v->get_window_index_from_glut_id(curr_win);
    if (i < 0) {
        fprintf(stderr,
                "Error, ImageViewer::visibilityCallback, window not found\n");
        return;
    }
    switch(state) {
	case GLUT_NOT_VISIBLE:
	    break;
	case GLUT_VISIBLE:
	    v->window[i].needs_redisplay = VRPN_TRUE;
	    break;
    }
}

// static
void ImageViewer::reshapeCallbackForGLUT(int w, int h) {
    int curr_win = glutGetWindow();
    ImageViewer *v = theViewer;
    int i = v->get_window_index_from_glut_id(curr_win);
    if (i < 0) {
	fprintf(stderr, 
		"Error, ImageViewer::reshapeCallback, window not found\n");
	return;
    }
    v->window[i].win_width = w;
    v->window[i].win_height = h;
    v->window[i].im_x_per_pixel = (float)v->window[i].im_width /
			(float)v->window[i].win_width;
    v->window[i].im_y_per_pixel = (float)v->window[i].im_height /
			(float)v->window[i].win_height;
    v->window[i].needs_redisplay = VRPN_TRUE;
    ImageViewerWindowEvent ivw_event;
    ivw_event.type = RESIZE_EVENT;
    ivw_event.width = w;
    ivw_event.height = h;
    ivw_event.winID = v->get_winID_from_window_index(i);
    if (v->window[i].event_handler)
	v->window[i].event_handler(ivw_event, v->window[i].event_handler_ud);
    return;
}

// static
void ImageViewer::motionCallbackForGLUT(int x, int y) {
    int curr_win = glutGetWindow();
    ImageViewer *v = theViewer;
    int i = v->get_window_index_from_glut_id(curr_win);
    if (i < 0) {
        fprintf(stderr,
                "Error, ImageViewer::motionCallback, window not found\n");
        return;
    }
    ImageViewerWindowEvent ivw_event;
    ivw_event.type = MOTION_EVENT;
    ivw_event.winID = v->get_winID_from_window_index(i);
    ivw_event.mouse_x = x;
    ivw_event.mouse_y = y;
    ivw_event.state = 0;
    if (v->window[i].left_button_down)
        ivw_event.state |= IV_LEFT_BUTTON_MASK;
    if (v->window[i].middle_button_down)
        ivw_event.state |= IV_MIDDLE_BUTTON_MASK;
    if (v->window[i].right_button_down)
        ivw_event.state |= IV_RIGHT_BUTTON_MASK;
    if (v->window[i].event_handler)
	v->window[i].event_handler(ivw_event,
		v->window[i].event_handler_ud);
    return;
}

// static
void ImageViewer::mouseCallbackForGLUT(int button, int state, int x, int y) {
    int curr_win = glutGetWindow();
    ImageViewer *v = theViewer;
    int i = v->get_window_index_from_glut_id(curr_win);
    if (i < 0) {
        fprintf(stderr,
                "Error, ImageViewer::mouseCallback, window not found\n");
        return;
    }
 
    ImageViewerWindowEvent ivw_event;
    switch(state) {
	case GLUT_DOWN:
	    ivw_event.type = BUTTON_PRESS_EVENT;
	    break;
	case GLUT_UP:
	    ivw_event.type = BUTTON_RELEASE_EVENT;
	    break;
    }
    switch(button) {
	case GLUT_LEFT_BUTTON:
	    ivw_event.button = IV_LEFT_BUTTON;
	    v->window[i].left_button_down = !v->window[i].left_button_down;
	    break;
	case GLUT_MIDDLE_BUTTON:
	    ivw_event.button = IV_MIDDLE_BUTTON;
	    v->window[i].middle_button_down = !v->window[i].middle_button_down;
	    break;
	case GLUT_RIGHT_BUTTON:
	    ivw_event.button = IV_RIGHT_BUTTON;
	    v->window[i].right_button_down = !v->window[i].right_button_down;
	    break;
    }
    ivw_event.winID = v->get_winID_from_window_index(i);
    ivw_event.mouse_x = x;
    ivw_event.mouse_y = y;
    ivw_event.state = 0;
    if (v->window[i].left_button_down)
    	ivw_event.state |= IV_LEFT_BUTTON_MASK;
    if (v->window[i].middle_button_down)
	ivw_event.state |= IV_MIDDLE_BUTTON_MASK;
    if (v->window[i].right_button_down)
	ivw_event.state |= IV_RIGHT_BUTTON_MASK;

    if (v->window[i].event_handler)
	v->window[i].event_handler(ivw_event,
		v->window[i].event_handler_ud);
}

// static
void ImageViewer::keyboardCallbackForGLUT(unsigned char key, int x, int y) {
    int curr_win = glutGetWindow();
    ImageViewer *v = theViewer;
    int i = v->get_window_index_from_glut_id(curr_win);
    if (i < 0) {
        fprintf(stderr,
                "Error, ImageViewer::keyboardCallback, window not found\n");
        return;
    }
    //printf("Ascii key '%c' 0x%02x\n", key, key);
    ImageViewerWindowEvent ivw_event;
    ivw_event.winID = v->get_winID_from_window_index(i);
    ivw_event.mouse_x = x;
    ivw_event.mouse_y = y;
    ivw_event.state = 0;
    ivw_event.type = KEY_PRESS_EVENT;
    ivw_event.keycode = key;
    if (v->window[i].event_handler)
	v->window[i].event_handler(ivw_event,
		v->window[i].event_handler_ud);

}

// static
void ImageViewer::specialCallbackForGLUT(int key, int x, int y) {
    //printf("special key %d\n", key);

}

#endif

int ImageViewer::setWindowEventHandler(int winID,
					ImageViewerWindowEventHandler f, 
						void *ud) {
    if (!validWinID(winID)) return -1;
    int win_index = get_window_index_from_winID(winID);

    window[win_index].event_handler = f;
    window[win_index].event_handler_ud = ud;
    return 0;
}

int ImageViewer::setWindowDisplayHandler(int winID,
			ImageViewerDisplayHandler f, void *ud)
{
    if (!validWinID(winID)) return -1;
    int win_index = get_window_index_from_winID(winID);

    window[win_index].display_handler = f;
    window[win_index].display_handler_ud = ud;
    return 0;
}

int ImageViewer::setWindowImageSize(int winID, int im_w, int im_h) {
    if (!validWinID(winID)) return -1;
    int win_index = get_window_index_from_winID(winID);

    int i;
    // don't do anything if the resolution isn't changing
    if (window[win_index].im_width == im_w && 
        window[win_index].im_height == im_h)
        return 0;

    if (window[win_index].d_pixelMode == GL_FLOAT){
        if (window[win_index].image)
            delete [] (float *)window[win_index].image;
        window[win_index].image = (void *)new float[im_w*im_h];
        for (i = 0; i < im_w*im_h; i++){
             ((float *)(window[win_index].image))[i] = 0;
        }
    } else if (window[win_index].d_pixelMode == GL_UNSIGNED_BYTE){
        if (window[win_index].image)
            delete [] (vrpn_uint8 *)window[win_index].image;
        window[win_index].image = (void *)new vrpn_uint8[im_w*im_h];
        for (i = 0; i < im_w*im_h; i++){
             ((vrpn_uint8 *)(window[win_index].image))[i] = 0;
        }
    } else {
        fprintf(stderr, "ImageViewer::setWindowImageSize: Error, "
               "unknown pixel type\n");
        return -1;
    }

    window[win_index].im_width = im_w;
    window[win_index].im_height = im_h;
    window[win_index].im_x_per_pixel = (float)window[win_index].im_width /
                                       (float)window[win_index].win_width;
    window[win_index].im_y_per_pixel = (float)window[win_index].im_height /
                                       (float)window[win_index].win_height;
#ifdef V_GLUT
      // this caused problems in winNT
    //int curr_win = glutGetWindow();
    //glutSetWindow(window[win_index].win_id);
    //glutReshapeWindow(window[win_index].im_width, window[win_index].im_height); 
    //glutSetWindow(curr_win);
#else
    XResizeWindow(dpy[window[win_index].display_index].x_dpy,
       	*(window[win_index].win), window[win_index].im_width, 
	window[win_index].im_height);
#endif
    return 0;
}

int ImageViewer::imageWidth(int winID) {
    if (!validWinID(winID)) return -1;
    int win_index = get_window_index_from_winID(winID);

    return window[win_index].im_width;
}

int ImageViewer::imageHeight(int winID) {
    if (!validWinID(winID)) return -1;
    int win_index = get_window_index_from_winID(winID);

    return window[win_index].im_height;
}

int ImageViewer::setValueRange(int winID, double min, double max) {
    if (!validWinID(winID)) return -1;
    int win_index = get_window_index_from_winID(winID);

    window[win_index].min_value = min;
    window[win_index].max_value = max;
    return 0;
}

int ImageViewer::setValue(int winID, int x, int y, double value) {
    if (!validWinID(winID)) return -1;
    int win_index = get_window_index_from_winID(winID);

    double actual_value = value;
    double scaled_value;
    if (value > window[win_index].max_value)
	actual_value = window[win_index].max_value;
    if (value < window[win_index].min_value)
	actual_value = window[win_index].min_value;
    scaled_value = (actual_value - window[win_index].min_value)/
		(window[win_index].max_value - window[win_index].min_value);
    if (window[win_index].d_pixelMode == GL_FLOAT){
        float *im = (float *)window[win_index].image;
        im[x + y*window[win_index].im_width] = scaled_value;
    } else if (window[win_index].d_pixelMode == GL_UNSIGNED_BYTE) {
        vrpn_uint8 *im = (vrpn_uint8 *)(window[win_index].image);
        im[x + y*window[win_index].im_width] = (vrpn_uint8)(255.0*scaled_value);
    } else {
        fprintf(stderr, "setValue: unknown pixel type\n");
        return -1;
    }
    window[win_index].needs_redisplay = VRPN_TRUE;
    return 0;
}

int ImageViewer::setScanline(int winID, int y, vrpn_uint8 *line){
    if (!validWinID(winID)) return -1;
    int win_index = get_window_index_from_winID(winID);

    if (window[win_index].d_pixelMode != GL_UNSIGNED_BYTE){
        fprintf(stderr, "setScanline: wrong pixel size\n");
        return -1;
    }
    memmove(
      (void *)(&
          ((char *)window[win_index].image)[y*window[win_index].im_width]),
      (const void *)line,
      window[win_index].im_width);
      window[win_index].needs_redisplay = VRPN_TRUE;
      return 0;
}

// draw into a window the current image for that window
int ImageViewer::drawImage(int winID) {
    if (!validWinID(winID)) return -1;
    int win_index = get_window_index_from_winID(winID);

#ifdef V_GLUT
    int curr_win = glutGetWindow();
    if (curr_win != window[win_index].win_id){
	fprintf(stderr, "Error: ImageViewer::drawImage - window not current\n");
	return 0;
    }
#endif

    if (!window[win_index].image) return -1;
    glViewport(0,0,window[win_index].win_width, 
		   window[win_index].win_height);
    float pix_per_im_x = 1.0/(window[win_index].im_x_per_pixel);
    float pix_per_im_y = 1.0/(window[win_index].im_y_per_pixel);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(1, -1, -1, 1, -1, 1);
    glRasterPos2f(-1.0, -1.0);
    // XXXX Adam's image flip. 
    glPixelZoom(-pix_per_im_x, pix_per_im_y);
    //glPixelZoom(pix_per_im_x, pix_per_im_y);
    glPixelStorei(GL_UNPACK_ALIGNMENT,1);
    if (window[win_index].d_colormap) {
        // gl first converts the luminance pixels to RGBA, then
        // applies the maps we specify. These are created from our
        // color maps above. We're letting GL do the work for us. 
        glPixelTransferi(GL_MAP_COLOR, GL_TRUE);
        glPixelMapfv(GL_PIXEL_MAP_R_TO_R, CMAP_SIZE_GL, window[win_index].rmap);
        glPixelMapfv(GL_PIXEL_MAP_G_TO_G, CMAP_SIZE_GL, window[win_index].gmap);
        glPixelMapfv(GL_PIXEL_MAP_B_TO_B, CMAP_SIZE_GL, window[win_index].bmap);

    }
      if (window[win_index].d_pixelMode == GL_FLOAT) {
        glDrawPixels(window[win_index].im_width, window[win_index].im_height,
            GL_LUMINANCE, GL_FLOAT, (float *)(window[win_index].image));
      } else if (window[win_index].d_pixelMode == GL_UNSIGNED_BYTE){
	vrpn_uint8 *im = (vrpn_uint8 *)(window[win_index].image);
	glDrawPixels(window[win_index].im_width, window[win_index].im_height,
            GL_LUMINANCE, GL_UNSIGNED_BYTE, im);
      } else {
          fprintf(stderr, "drawImage: Error, unknown pixel type\n");
          return -1;
      }
      if (window[win_index].d_colormap) {
          glPixelTransferi(GL_MAP_COLOR, GL_FALSE);
      }
    return 0;
}

// draw any image into a window
int ImageViewer::drawImage(int winID, nmb_Image *image, 
              double red, double green, double blue, double alpha,
              double *left, double *right, double *bottom, double *top, 
              nmb_TransformMatrix44 *worldToImage)
{
    if (!validWinID(winID)) return -1;
    int win_index = get_window_index_from_winID(winID);

#ifdef V_GLUT
    int curr_win_save = glutGetWindow();
    if (curr_win_save != window[win_index].win_id){
      glutSetWindow(window[win_index].win_id);
    }
#endif

    // default values for optional parameters depend on the image so 
    // we set them here
    nmb_TransformMatrix44 W2I;
    double l, r, b, t;
    if (left == NULL || right == NULL || bottom == NULL || top == NULL) {
      l = 0;
      r = 1;
      b = 0;
      t = 1;
    } else {
      l = *left;
      r = *right;
      b = *bottom;
      t = *top;
    }
    if (worldToImage != NULL) {
      W2I = *worldToImage;
    }
    // done with setting values for optional parameters

    double scx, scy, shz, phi, tx, ty;
    W2I.getTScShR_2DParameters(0.0, 0.0, tx, ty, phi, shz, scx, scy);

    double shMag = fabs(shz);
    double phiMag = fmod(phi, 2.0*M_PI);
    if (phiMag > M_PI) {
      phiMag = fabs(2.0*M_PI - phiMag);
    } else {
      phiMag = fabs(phiMag);
    }

    glViewport(0,0,window[win_index].win_width,
                   window[win_index].win_height);

    if (shMag > 0.0001 || phiMag > 0.0001) {
      drawImageAsTexture(winID, image, red, green, blue, alpha, 
                         l, r, b, t, W2I);
    } else { // try using glDrawPixels instead of texture-mapping
      drawImageAsPixels(winID, image, red, green, blue, alpha, 
             l, r, b, t, tx, ty, scx, scy);
    }

#ifdef V_GLUT
    glutSetWindow(curr_win_save);
#endif

    return 0;
}

int ImageViewer::drawImageAsTexture(int winID, nmb_Image *image, 
      double red, double green, double blue, double alpha, 
      double l, double r, double b, double t, nmb_TransformMatrix44 &W2I)
{
    int win_index = get_window_index_from_winID(winID);
  void *texture;
  int texwidth, texheight;
  vrpn_bool textureOkay = VRPN_TRUE;
  texture = image->pixelData();
  nmb_ColorMap * colormap = window[win_index].d_colormap;
  int pixType;
  switch (image->pixelType()) {
    case NMB_UINT8:
      pixType = GL_UNSIGNED_BYTE;
      break;
    case NMB_UINT16:
      pixType = GL_UNSIGNED_SHORT;
      break;
    case NMB_FLOAT32:
      pixType = GL_FLOAT;
      break;
    default:
      textureOkay = VRPN_FALSE;
      fprintf(stderr, "mainWinDisplayHandler::"
                      "Error, unrecognized pixel type\n");
      break;
  }
  if (!texture) {
    textureOkay = VRPN_FALSE;
  }

  if (textureOkay) {
    texwidth = image->width() +
               image->borderXMin()+image->borderXMax();
    texheight = image->height() +
                image->borderYMin()+image->borderYMax();
    float scaleFactorX = (float)(image->width())/(float)texwidth;
    float scaleFactorY = (float)(image->height())/(float)texheight;

    int bordSizeXPixels = image->borderXMin();
    int bordSizeYPixels = image->borderYMin();
    // in texture coordinates
    float bordSizeX = (float)bordSizeXPixels/(float)texwidth;
    float bordSizeY = (float)bordSizeYPixels/(float)texheight;

//    printf("begin draw texture for %s\n", image->name()->Characters());

/*    printf("Loading texture %s with type %d\n",
           image->name()->Characters(), image->pixelType());
    printf("width=%d,height=%d,border = (%d,%d)(%d,%d)\n",
      texwidth, texheight, image->borderXMin(),
      image->borderXMax(), image->borderYMin(),
      image->borderYMax());
*/
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
    glEnable(GL_TEXTURE_GEN_R);
    glEnable(GL_TEXTURE_GEN_Q);

/*
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
              GL_LINEAR_MIPMAP_LINEAR);

    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, texwidth, texheight,
           GL_LUMINANCE,
           pixType, texture);
*/
    //glTexImage2D(GL_PROXY_TEXTURE_2D, ...
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, texwidth);

    if (!colormap) {

        glPixelTransferf(GL_RED_SCALE, (float)red);
        glPixelTransferf(GL_GREEN_SCALE, (float)green);
        glPixelTransferf(GL_BLUE_SCALE, (float)blue);
        glPixelTransferf(GL_ALPHA_SCALE, (float)alpha);
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
             texwidth, texheight, 0, GL_LUMINANCE,
             pixType, texture);
        glPixelTransferf(GL_RED_SCALE, 1.0);
        glPixelTransferf(GL_GREEN_SCALE, 1.0);
        glPixelTransferf(GL_BLUE_SCALE, 1.0);
        glPixelTransferf(GL_ALPHA_SCALE, 1.0);
    } else {
        glPixelTransferi(GL_MAP_COLOR, GL_TRUE);
        glPixelMapfv(GL_PIXEL_MAP_R_TO_R, CMAP_SIZE_GL, window[win_index].rmap);
        glPixelMapfv(GL_PIXEL_MAP_G_TO_G, CMAP_SIZE_GL, window[win_index].gmap);
        glPixelMapfv(GL_PIXEL_MAP_B_TO_B, CMAP_SIZE_GL, window[win_index].bmap);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
             texwidth, texheight, 0, GL_LUMINANCE,
             pixType, texture);
        
        glPixelTransferi(GL_MAP_COLOR, GL_FALSE);
    }

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(l, r, b, t, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_TEXTURE);
    glPushMatrix();
    glLoadIdentity();

    double regionVertex[4][4] = 
             {{0, 0, 0, 1}, {1, 0, 0, 1}, {1, 1, 0, 1},{0, 1, 0, 1}};

    W2I.invTransform(regionVertex[0]);
    W2I.invTransform(regionVertex[1]);
    W2I.invTransform(regionVertex[2]);
    W2I.invTransform(regionVertex[3]);

    // compensation for the border:
    glTranslatef(bordSizeX, bordSizeY, 0.0);
    glScalef(scaleFactorX, scaleFactorY, 1.0);
    // now we can use the xform defined for the actual image part of the
    // texture
    double worldToImage[16];
    W2I.getMatrix(worldToImage);
    glMultMatrixd(worldToImage);

    glBegin(GL_POLYGON);
      glNormal3f(0.0, 0.0, 1.0);
      glColor4f(1.0, 1.0, 1.0, (float)alpha);
      // draw a parallelogram fit to the image
      glVertex2f(regionVertex[0][0], regionVertex[0][1]);
      glVertex2f(regionVertex[1][0], regionVertex[1][1]);
      glVertex2f(regionVertex[2][0], regionVertex[2][1]);
      glVertex2f(regionVertex[3][0], regionVertex[3][1]);
    glEnd();

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
    glDisable(GL_TEXTURE_GEN_R);
    glDisable(GL_TEXTURE_GEN_Q);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_TEXTURE);
    glPopMatrix();
    
    glFinish();
//    printf("end draw texture\n");
  }
  return 0;
}

int ImageViewer::drawImageAsPixels(int winID, nmb_Image *image,
      double red, double green, double blue, double alpha,
      double l, double r, double b, double t, 
      double tx, double ty, double scx, double scy)
{
  int win_index = get_window_index_from_winID(winID);
  void *texture;
  int texwidth, texheight;
  vrpn_bool textureOkay = VRPN_TRUE;
  texture = image->pixelData();
  int pixType;
  int winWidth = window[win_index].win_width;
  int winHeight = window[win_index].win_height; 
  nmb_ColorMap * colormap = window[win_index].d_colormap;

  switch (image->pixelType()) {
    case NMB_UINT8:
      pixType = GL_UNSIGNED_BYTE;
      break;
    case NMB_UINT16:
      pixType = GL_UNSIGNED_SHORT;
      break;
    case NMB_FLOAT32:
      pixType = GL_FLOAT;
      break;
    default:
      textureOkay = VRPN_FALSE;
      fprintf(stderr, "mainWinDisplayHandler::"
                      "Error, unrecognized pixel type\n");
      break;
  }
  if (!texture) {
    textureOkay = VRPN_FALSE;
  }

  if (textureOkay) {
    double matrix[16];
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(l, r, b, t, -1, 1);
    glGetDoublev(GL_PROJECTION_MATRIX, matrix);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // these are the actual dimensions of the texture as stored in memory
    // note that the border may be different sizes on the left and right and
    // top and bottom
    texwidth = image->width() +
               image->borderXMin()+image->borderXMax();
    texheight = image->height() +
               image->borderYMin()+image->borderYMax();

    // when drawing the texture, we really only care about the border size
    // to the extent that it affects how it determines the offset to the
    // start of the actual image in the texture array and this only depends
    // on these two border parameters:
    int skipPixels = image->borderXMin();
    int skipRows = image->borderYMin();

//    printf("begin drawPixels for %s\n", image->name()->Characters());

    // these are the dimensions of the texture minus the border
    int imageWidth = image->width();
    int imageHeight = image->height();

    // compute some useful ratios:
    // world units per window pixels in x
    double wuPerWindowPixelX = fabs(r - l)/(double)winWidth;
    // world units per window pixels in y
    double wuPerWindowPixelY = fabs(t - b)/(double)winHeight;
    // world units per image pixels in x
    double wuPerImagePixelX = 1.0/(fabs(scx)*imageWidth);
    // world units per image pixels in y
    double wuPerImagePixelY = 1.0/(fabs(scy)*imageHeight);

    // and the following ratio tells us the pixel zoom
    double windowPixelPerImagePixelX = wuPerImagePixelX/wuPerWindowPixelX;
    double windowPixelPerImagePixelY = wuPerImagePixelY/wuPerWindowPixelY;

    // compute the raster position in world units 
    GLdouble rasterPos_wu[2] = {-tx/scx, -ty/scy};

    // if the raster position as we have currently calculated it would be
    // clipped but there is still some part of the image that shouldn't be 
    // clipped then we will detect that here 
    // (first checking in X and then in Y)
    double boundX, boundY;
    int skipIncX, skipIncY;

    // figure out whether the image should be drawn starting at the left or
    // right side of the screen - that determines where the image gets cut in
    // the x direction
    if (scx*(r-l) < 0) {
       boundX = r;
    } else {
       boundX = l;
    }
    // now we check whether in fact the raster position is clipped in x and if
    // so then we adjust it by shifting in the drawing direction until it
    // won't be clipped; we do this by cutting out skipIncX pixels from either
    // the right or left side of the image as it appears on the screen
    if (scx*(rasterPos_wu[0] - boundX) <= 0) {
      skipIncX = (int)ceil(fabs(rasterPos_wu[0] - boundX)/wuPerImagePixelX);
      skipPixels += skipIncX;
      rasterPos_wu[0] += (scx > 0)*skipIncX*wuPerImagePixelX;
    }

    // figure out whether the image should be drawn starting at the top or
    // bottom side of the screen - that determines where the image gets cut in
    // the y direction
    if (scy*(t-b) < 0) {
       boundY = t;
    } else {
       boundY = b;
    }
    // now we check whether in fact the raster position is clipped in y and if
    // so then we adjust it by shifting in the drawing direction until it
    // won't be clipped; we do this by cutting out skipIncY pixels from either
    // the top or bottom side of the image as it appears on the screen
    if (scy*(rasterPos_wu[1] - boundY) <= 0) {
      skipIncY = (int)ceil(fabs(rasterPos_wu[1] - boundY)/wuPerImagePixelY);
      skipRows += skipIncY;
      rasterPos_wu[1] += (scy > 0)*skipIncY*wuPerImagePixelY;
    }

    // check to see if the raster position is outside the viewport by
    // transforming it into screen coordinates
    // this is essentially duplicating what would happen in openGL if we
    // called glRasterPos2dv(rasterPos_wu) at this point
    GLdouble rasterPos_sc[2];
    // we only have a scale and translation
    // (assuming the axis aligned orthographic projection as specified above)
    double projScaleX = matrix[0], projScaleY = matrix[5];
    double projTransX = matrix[12], projTransY = matrix[13];
    rasterPos_sc[0] = rasterPos_wu[0]*projScaleX + projTransX;
    rasterPos_sc[1] = rasterPos_wu[1]*projScaleY + projTransY;

    // we will get clipping of the raster position if the it is outside of
    // the rectangle [-1,1]x[-1,1] so
    // figure out what increment to add to the raster position in world units
    // to ensure that the transformed raster position isn't clipped due to
    // roundoff error
    GLdouble rasterPos_wu_incr[2] = {0.0, 0.0};
    if (rasterPos_sc[0] <= -0.9999) {
      rasterPos_wu_incr[0] = +0.9999/projScaleX;
    } else if (rasterPos_sc[0] >= 0.9999) {
      rasterPos_wu_incr[0] = -0.001/projScaleX;
    }
    if (rasterPos_sc[1] <= -0.9999) {
      rasterPos_wu_incr[1] = +0.001/projScaleY;
    } else if (rasterPos_sc[1] >= 0.9999) {
      rasterPos_wu_incr[1] = -0.001/projScaleY;
    }
    // now add the fudge factor
    rasterPos_wu[0] += rasterPos_wu_incr[0];
    rasterPos_wu[1] += rasterPos_wu_incr[1];

    // if we cut up the image to avoid clipping the starting raster postion
    // then we need to recompute the actual size of the image which gets
    // drawn on the screen
    // compute the number of border pixels (pixels that don't get drawn) 
    // in x and y
    int totalRowBorder = image->borderXMax() + skipPixels;
    int totalColBorder = image->borderYMax() + skipRows;
    // the number of the pixels in x and y that do get drawn are the total
    // width and height of the array minus the above:
    int rowWidth = texwidth - totalRowBorder;
    int colHeight = texheight - totalColBorder;

    // set the starting position on the screen where the image will be drawn
    glRasterPos2dv(rasterPos_wu);

    // just in case, check to see if the raster position is valid and if not
    // output some debugging info
    GLboolean valid[1];
    glGetBooleanv(GL_CURRENT_RASTER_POSITION_VALID, valid);
    if (valid[0] != GL_TRUE) {
      GLfloat actual_pos[4];
      glGetFloatv(GL_CURRENT_RASTER_POSITION, actual_pos);
      rasterPos_sc[0] = projScaleX*rasterPos_wu[0] + projTransX;
      rasterPos_sc[1] = projScaleY*rasterPos_wu[1] + projTransY;
      printf("raster pos: (%g, %g)->(%g, %g), actual:(%g, %g) not valid\n",
          rasterPos_wu[0], rasterPos_wu[1], rasterPos_sc[0], rasterPos_sc[1],
          actual_pos[0], actual_pos[1]);
      return -1;
    }

    // set some parameters that tell openGL which pixels not to draw
    glPixelStorei(GL_UNPACK_SKIP_ROWS, skipRows);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, skipPixels);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, texwidth);

    // now set the pixel zoom; the image may be flipped in x or y depending on
    // the scale factors and the orientation of the world view so we calculate
    // whether or not to flip the image here:
    if ((r-l)*scx < 0) {
      windowPixelPerImagePixelX *= -1;
    }
    if ((t-b)*scy < 0) {
      windowPixelPerImagePixelY *= -1;
    }
    glPixelZoom(windowPixelPerImagePixelX, windowPixelPerImagePixelY);

    if (!colormap) {
        glPixelTransferf(GL_RED_SCALE, (float)red);
        glPixelTransferf(GL_GREEN_SCALE, (float)green);
        glPixelTransferf(GL_BLUE_SCALE, (float)blue);
        glPixelTransferf(GL_ALPHA_SCALE, (float)alpha);
        
        glDrawPixels(rowWidth, colHeight, GL_LUMINANCE, pixType, texture);
        
        glPixelTransferf(GL_RED_SCALE, 1.0);
        glPixelTransferf(GL_GREEN_SCALE, 1.0);
        glPixelTransferf(GL_BLUE_SCALE, 1.0);
        glPixelTransferf(GL_ALPHA_SCALE, 1.0);
    } else {
        // gl first converts the luminance pixels to RGBA, then
        // applies the maps we specify. These are created from our
        // color maps above. We're letting GL do the work for us. 
        glPixelTransferi(GL_MAP_COLOR, GL_TRUE);
        glPixelMapfv(GL_PIXEL_MAP_R_TO_R, CMAP_SIZE_GL, window[win_index].rmap);
        glPixelMapfv(GL_PIXEL_MAP_G_TO_G, CMAP_SIZE_GL, window[win_index].gmap);
        glPixelMapfv(GL_PIXEL_MAP_B_TO_B, CMAP_SIZE_GL, window[win_index].bmap);
        glDrawPixels(rowWidth, colHeight, GL_LUMINANCE, pixType, texture);
        
        glPixelTransferi(GL_MAP_COLOR, GL_FALSE);
    }
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glFinish();
    //printf("end draw pixels\n");
  }
  return 0;
}

int ImageViewer::drawString(int x, int y, char *str) {
#ifdef V_GLUT
    void *font = GLUT_BITMAP_8_BY_13;	// should be same as that used below
    int len = strlen(str);
    int i;
    glRasterPos2i(x,y);
    for (i = 0; i < len; i++){
	glutBitmapCharacter(font, str[i]);
    }
#else
    if (!font_dlist_base) {
	Font f = XLoadFont(dpy[0].x_dpy,
		"-misc-fixed-medium-r-normal--13-120-75-75-C-80-iso8859-1");
        font_dlist_base = glGenLists(94);
	if (!font_dlist_base){
	    font_dlist_base = glGenLists(10);
	    if (!font_dlist_base){
		fprintf(stderr, "Error: Couldn't create font display lists\n");
		return -1;
	    }
	    font_base = 48;
	    num_font_chars = 10;
	}
	else{
	    font_base = 33;
	    num_font_chars = 94;
	}
	glXUseXFont(f, font_base, num_font_chars, font_dlist_base);
    }

    int len = strlen(str);

    GLint pos_x, pos_y;

    pos_x = x; pos_y = y;

    int i;
    for (i = 0; i < len; i++){
    	glRasterPos2f(pos_x, pos_y);
	// This assumes that the font conforms to ascii standard
	// which is the case for
	// -misc-fixed-medium-r-normal--13-120-75-75-C-80-iso8859-1
	if (str[i] >= font_base && str[i] < font_base+num_font_chars)
	    glCallList(font_dlist_base + str[i] - font_base);
	else {
	    fprintf(stderr, "Error: loaded font bitmaps not sufficient to"
                    " display string: %s\n", str);
	    return -1;
	}
	pos_x += FONT_WIDTH;
    }
#endif
    return 0;
}

int ImageViewer::toImage(int winID, double *x, double *y){
    if (!validWinID(winID)) return -1;
    int win_index = get_window_index_from_winID(winID);

    // XXXX Adam flips X too, why?
    *x /= (double)window[win_index].win_width;
    *x = 1.0-*x;
    *y /= (double)window[win_index].win_height;
    *y = 1.0-*y;
    return 0;
}

int ImageViewer::toPixels(int winID, double *x, double *y){
    if (!validWinID(winID)) return -1;
    int win_index = get_window_index_from_winID(winID);

    *x = 1.0-*x;
    *y = 1.0-*y;
    *x *= (double)window[win_index].win_width;
    *y *= (double)window[win_index].win_height;
    return 0;
}

/** Clamp coords to window size.
    @return true if clamping performed, false otherwise. 
 */ 
int ImageViewer::clampToWindow(int winID, double *x, double *y) {
    int clamped = 0;
    if (!validWinID(winID)) return -1;
    int win_index = get_window_index_from_winID(winID);

    if (*x > window[win_index].win_width) {
	*x = window[win_index].win_width;
        clamped = 1;
    } else if (*x < 0) {
	*x = 0;
        clamped = 1;
    }
    if (*y > window[win_index].win_height) {
	*y = window[win_index].win_height;
        clamped = 1;
    } else if (*y < 0) {
	*y = 0;
        clamped = 1;
    }
    return clamped;
}

/** Set a colormap to use to display pixels. Pass in NULL to disable. 
 */
int ImageViewer::setColorMap(int winID, nmb_ColorMap * cmap) {
    if (!validWinID(winID)) return -1;
    int win_index = get_window_index_from_winID(winID);
    window[win_index].d_colormap = cmap;
    return (calcColorMap(winID));
}

/** Scale the colormap using values from the colormap control widget
 */
int ImageViewer::setColorMinMax(int winID,
                                vrpn_float64 dmin, vrpn_float64 dmax,
                                vrpn_float64 cmin, vrpn_float64 cmax)
{
    //printf("Got %f %f %f %f\n", dmin, dmax, cmin, cmax);
    if (!validWinID(winID)) return -1;
    int win_index = get_window_index_from_winID(winID);
    window[win_index].d_data_min = dmin;
    window[win_index].d_data_max = dmax;
    window[win_index].d_color_min = cmin;
    window[win_index].d_color_max = cmax;
    return (calcColorMap(winID));
}

/** Perform the work of calculating a new color-mapped image
 */
int ImageViewer::calcColorMap(int winID) 
{
    int win_index = get_window_index_from_winID(winID);
    double scaled_value = 0.0;
    if (window[win_index].d_colormap) {
        float dummy;
        for (int i = 0; i < CMAP_SIZE_GL; i++) {
          window[win_index].d_colormap->lookup(
              i, 
              0, CMAP_SIZE_GL,
              window[win_index].d_data_min, window[win_index].d_data_max, 
              window[win_index].d_color_min, window[win_index].d_color_max, 
              &window[win_index].rmap[i], 
              &window[win_index].gmap[i], 
              &window[win_index].bmap[i], &dummy);
        }
    }
    return 0;
}


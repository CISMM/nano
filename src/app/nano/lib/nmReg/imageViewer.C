#include <stdio.h>
#include <stdlib.h>

#define USING_REG_WINDOWS

#ifdef V_GLUT
#	include <GL/glut.h>
#else
#	include <GL/glx.h> 
#	include <GL/gl.h>
#endif
#include <unistd.h>
#include <assert.h>
#include <X11/cursorfont.h>
#include <math.h>

#include "imageViewer.h"

ImageViewer *ImageViewer::theViewer = NULL;	// pointer to singleton

ImageWindow::ImageWindow():
    id(0),
#ifdef V_GLUT
    win_id(0),left_button_down(VRPN_FALSE),
	middle_button_down(VRPN_FALSE), right_button_down(VRPN_FALSE),
#else
    win(NULL),
#endif
    win_width(0),win_height(0), im_width(0), im_height(0),
    display_index(0),visible(VRPN_FALSE),needs_redisplay(VRPN_FALSE),
    image(NULL),min_value(0), max_value(1.0),
    event_handler(NULL), event_handler_ud(NULL),
    display_handler(NULL), display_handler_ud(NULL)
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

// returns window id or 0 if error occurred
int ImageViewer::createWindow(char *display_name,
                      int x, int y, int w, int h, char *win_name){
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
#ifdef USING_REG_WINDOWS
    //glutInitWindowSize(w,h);
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

    //glutIdleFunc(ImageViewer::idleCallbackForGLUT); 
    glutDisplayFunc(ImageViewer::displayCallbackForGLUT);

    glutVisibilityFunc(ImageViewer::visibilityCallbackForGLUT);
    glutReshapeFunc(ImageViewer::reshapeCallbackForGLUT);
    glutMotionFunc(ImageViewer::motionCallbackForGLUT);
    glutMouseFunc(ImageViewer::mouseCallbackForGLUT);
    glutKeyboardFunc(ImageViewer::keyboardCallbackForGLUT);
    //glutSetWindow(win_save);
#endif
#endif

    window[num_windows].display_index = dpy_index;
    window[num_windows].id = num_windows+1;
    window[num_windows].win_width = w;
    window[num_windows].win_height = h;
    window[num_windows].im_width = window[num_windows].win_width;
    window[num_windows].im_height = window[num_windows].win_height;
    window[num_windows].im_x_per_pixel = (float)window[num_windows].im_width /
                               (float)window[num_windows].win_width;
    window[num_windows].im_y_per_pixel = (float)window[num_windows].im_height /
                               (float)window[num_windows].win_height;
    if (window[num_windows].image)
        delete window[num_windows].image;
    window[num_windows].image = new float[window[num_windows].im_width*
					window[num_windows].im_height];
    int i;
    for (i = 0; i < window[num_windows].im_width*
			window[num_windows].im_height; i++){
        window[num_windows].image[i] = 0.0;
    }

    num_windows++;
//    showWindow(num_windows);
    return num_windows;
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

int ImageViewer::showWindow(int winID){
    assert(winID > 0 && winID <= num_windows);

    int win_index = winID-1;
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
#ifdef USING_REG_WINDOWS
    int curr_win = glutGetWindow();
    glutSetWindow(window[win_index].win_id); // the GLUT window ID
    glutShowWindow();
#endif
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

    window[win_index].win_width = window[win_index].im_width;
    window[win_index].win_height = window[win_index].im_height;
    window[win_index].im_x_per_pixel = (float)window[win_index].im_width /
                               (float)window[win_index].win_width;
    window[win_index].im_y_per_pixel = (float)window[win_index].im_height /
                               (float)window[win_index].win_height;

#ifdef V_GLUT
#ifdef USING_REG_WINDOWS
    glutPositionWindow(x_loc,y_loc);
    glutReshapeWindow(window[win_index].im_width, window[win_index].im_height);
    glutSetWindow(curr_win);
#endif
#else
    XMoveResizeWindow(dpy[window[win_index].display_index].x_dpy,
                (*(window[win_index].win)), x_loc, y_loc,
            window[win_index].im_width,
            window[win_index].im_height);
#endif

    window[win_index].visible = VRPN_TRUE;
    dirtyWindow(winID);
    return 0;
}

int ImageViewer::hideWindow(int winID){
    assert(winID > 0 && winID <= num_windows);

    int win_index = winID-1;
    if (!(window[win_index].visible)) {
	fprintf(stderr, "Warning: window wasn't visible\n");
	return 0;
    }
#ifdef V_GLUT
#ifdef USING_REG_WINDOWS
    int curr_win = glutGetWindow();
    glutSetWindow(window[win_index].win_id); // the GLUT window ID
    glutHideWindow();
    glutSetWindow(curr_win);
#endif
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
    assert(winID > 0 && winID <= num_windows);

    int win_index = winID-1;
    window[win_index].needs_redisplay = VRPN_TRUE;

#ifdef V_GLUT
#ifdef USING_REG_WINDOWS
    int curr_win = glutGetWindow();
    glutSetWindow(window[win_index].win_id);
    glutPostRedisplay();
    glutSetWindow(curr_win);
#endif
#endif

    return 0;
}

void ImageViewer::mainloop() {
    int i;
    ImageViewerWindowEvent ivw_event;
#ifdef V_GLUT // event processing will be done in glut callbacks
#else
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
		    ivw_event.winID = i+1;
		    if (window[i].event_handler)
			window[i].event_handler(ivw_event, 
				window[i].event_handler_ud);
		    break;
		case ButtonPress:
		    ivw_event.type = BUTTON_PRESS_EVENT;
		    ivw_event.winID = i+1;
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
		    ivw_event.winID = i+1;
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
                    ivw_event.winID = i+1;
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
                    ivw_event.winID = i+1;
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
	    ivdd.winID = i+1;
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
    ivdd.winID = i+1;
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
    ivw_event.winID = i+1;
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
    ivw_event.winID = i+1;
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
    ivw_event.winID = i+1;
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
    ImageViewerWindowEvent ivw_event;
    ivw_event.winID = i+1;
    ivw_event.mouse_x = x;
    ivw_event.mouse_y = y;
    ivw_event.state = 0;
    ivw_event.type = KEY_PRESS_EVENT;
    ivw_event.keycode = key;
    if (v->window[i].event_handler)
	v->window[i].event_handler(ivw_event,
		v->window[i].event_handler_ud);

}
#endif

int ImageViewer::setWindowEventHandler(int winID,
					ImageViewerWindowEventHandler f, 
						void *ud) {
    int win_index = winID-1;
    window[win_index].event_handler = f;
    window[win_index].event_handler_ud = ud;
    return 0;
}

int ImageViewer::setWindowDisplayHandler(int winID,
			ImageViewerDisplayHandler f, void *ud)
{
    int win_index = winID-1;
    window[win_index].display_handler = f;
    window[win_index].display_handler_ud = ud;
    return 0;
}

int ImageViewer::setWindowImageSize(int winID, int im_w, int im_h) {
    int win_index = winID-1;
    int i;
    // don't do anything if the resolution isn't changing
    if (window[win_index].im_width == im_w && 
        window[win_index].im_height == im_h)
        return 0;

    if (window[win_index].image)
	delete window[win_index].image;
    window[win_index].image = new float[im_w*im_h];
    for (i = 0; i < im_w*im_h; i++){
	window[win_index].image[i] = 0.0;
    }
    window[win_index].im_width = im_w;
    window[win_index].im_height = im_h;
    window[win_index].im_x_per_pixel = (float)window[win_index].im_width /
                                       (float)window[win_index].win_width;
    window[win_index].im_y_per_pixel = (float)window[win_index].im_height /
                                       (float)window[win_index].win_height;
#ifdef V_GLUT
#ifdef USING_REG_WINDOWS
    int curr_win = glutGetWindow();
    glutSetWindow(window[win_index].win_id);
    glutReshapeWindow(window[win_index].im_width, window[win_index].im_height); 
    glutSetWindow(curr_win);
#endif
#else
    XResizeWindow(dpy[window[win_index].display_index].x_dpy,
       	*(window[win_index].win), window[win_index].im_width, 
	window[win_index].im_height);
#endif
    return 0;
}

int ImageViewer::imageWidth(int winID) {
    int win_index = winID-1;
    return window[win_index].im_width;
}

int ImageViewer::imageHeight(int winID) {
    int win_index = winID-1;
    return window[win_index].im_height;
}

int ImageViewer::setValueRange(int winID, double min, double max) {
    int win_index = winID-1;
    window[win_index].min_value = min;
    window[win_index].max_value = max;
    return 0;
}

int ImageViewer::setValue(int winID, int x, int y, double value) {
    int win_index = winID-1;
    double actual_value = value;
    double scaled_value;
    if (value > window[win_index].max_value)
	actual_value = window[win_index].max_value;
    if (value < window[win_index].min_value)
	actual_value = window[win_index].min_value;
    scaled_value = (actual_value - window[win_index].min_value)/
		(window[win_index].max_value - window[win_index].min_value);
    window[win_index].image[x + y*window[win_index].im_width] = scaled_value;
    window[win_index].needs_redisplay = VRPN_TRUE;
    return 0;
}

// draw into a window the current image for that window
int ImageViewer::drawImage(int winID) {
    int win_index = winID-1;
    
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
    glRasterPos2f(-1.0, -1.0);
    glPixelZoom(pix_per_im_x, pix_per_im_y);
    glDrawPixels(window[win_index].im_width, window[win_index].im_height,
	GL_LUMINANCE, GL_FLOAT, window[win_index].image);

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
    int win_index = winID-1;
    *x /= (double)window[win_index].win_width;
    *y /= (double)window[win_index].win_height;
    return 0;
}

int ImageViewer::toPixels(int winID, double *x, double *y){
    int win_index = winID-1;
    *x *= (double)window[win_index].win_width;
    *y *= (double)window[win_index].win_height;
    return 0;
}

int ImageViewer::clampToWindow(int winID, double *x, double *y) {
    int win_index = winID-1;
    if (*x > window[win_index].win_width)
	*x = window[win_index].win_width;
    else if (*x < 0)
	*x = 0;
    if (*y > window[win_index].win_height)
	*y = window[win_index].win_height;
    else if (*y < 0)
	*y = 0;
    return 0;
}

#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

/*
ImageViewer -

This class is designed to provide the current functionality required in 
microscape for the x-window (used to display a grayscale image of the 
surface).
In addition it allows you to make multiple windows and provides the
ability to draw using OpenGL. This allows us to create additional windows
which will be used as part of the user interface to the image registration
functions.

This code will compile to use either GLX or GLUT
Callbacks may be registered to handle user interaction events or
to draw stuff in the window.
Special utility functions are provided to handle drawing images
into windows and to convert from window to image coordinates and vice versa.
*/

#ifdef V_GLUT
#	include <GL/glut_UNC.h>
#else
#	include <GL/glx.h>
#	include <GL/gl.h>
#endif

#ifndef _WIN32
#	include <unistd.h>
#endif

//#include "BCPlane.h"
//#include "PNMImage.h"
//#include "PPM.h"
#include "vrpn_Types.h"


#define MAX_WIN (10)
#define MAX_DISPLAYS (4)
#define DEFAULT_MAX_POINTS (100)
#define FONT_WIDTH (8)	// width of characters in pixels for font used
			// by drawString()

enum {RESIZE_EVENT, BUTTON_PRESS_EVENT, BUTTON_RELEASE_EVENT,
	KEY_PRESS_EVENT, MOTION_EVENT};

enum {IV_LEFT_BUTTON, IV_MIDDLE_BUTTON, IV_RIGHT_BUTTON};

enum {IV_LEFT_BUTTON_MASK = 4, IV_MIDDLE_BUTTON_MASK = 2, 
		IV_RIGHT_BUTTON_MASK = 1};

typedef struct _ImageViewerWindowEvent {
    int type;
    int winID;
    int width, height;
    int mouse_x, mouse_y;
    unsigned int button;
    unsigned int state;
    unsigned int keycode;
} ImageViewerWindowEvent;

typedef int (*ImageViewerWindowEventHandler) 
	(const ImageViewerWindowEvent &ivwe, void *ud);
		

typedef struct _ImageViewerDisplayData {
    int winID;
    int winWidth, winHeight;
    int imWidth, imHeight;
} ImageViewerDisplayData;

typedef int (*ImageViewerDisplayHandler)
		(const ImageViewerDisplayData &data, void *ud);


/// This class is used internally by ImageViewer to represent the state
/// for each window.
class ImageWindow {
  public:
    ImageWindow();
    int id;
#ifdef V_GLUT
    int win_id;
    vrpn_bool left_button_down;
    vrpn_bool middle_button_down;
    vrpn_bool right_button_down;
#else
    Window *win;
    Cursor cursor;
    XSetWindowAttributes swa;
#endif
    int win_width;
    int win_height;
    int im_width;
    int im_height;
    float im_x_per_pixel, im_y_per_pixel;
    int display_index;
    vrpn_bool visible;
    vrpn_bool needs_redisplay;
    void *image;
    int d_pixelMode;
    double min_value, max_value;
    ImageViewerWindowEventHandler event_handler;
    void *event_handler_ud;
    ImageViewerDisplayHandler display_handler;
    void *display_handler_ud;
};

/// This class is used internally by ImageViewer to represent the state
/// for each display that is used to display windows.
class ImageDisplay{
  public:
    ImageDisplay();
    char *dpy_name;
#ifdef V_GLUT
#else
    Display *x_dpy;
    XVisualInfo *vi;
    GLXContext cx;
    Colormap cmap;
#endif
};

/// This class is responsible for displaying images in a collection of
/// windows. It manages graphics updates and user interaction with the
/// windows. Arbitrary graphics can be drawn and keyboard/mouse interaction
/// can be handled through user callbacks but some special utility functions 
/// are provided to simplify display of greyscale images.
class ImageViewer {
  public:
    static ImageViewer *getImageViewer();
    int init(char *display);
    void mainloop(); // updates displays, checks for user interaction
    /// creates a window on the given display 
    int createWindow(char *display_name,
		int x, int y, int w, int h, char *name, 
                int pixelType = GL_FLOAT);
    /// destroy a window
    int destroyWindow(int winID);
    /// resize a window
    int setWindowSize(int winID, int w, int h); 
    /// resize the image associated with a window
    int setWindowImageSize(int winID, int im_w, int im_h);
    /// get width of image associated with a window
    int imageWidth(int winID);
    /// get height of a image associated with a window
    int imageHeight(int winID);
    /// this determines how values passed into setValue() will be
    /// scaled before display. It should be called before setting 
    /// any values in the image using setValue()
    int setValueRange(int winID, double min, double max);
    /// This sets a single pixel in the image associated with a window
    int setValue(int winID, int x, int y, double value);
    /// This sets an entire line in the image associated with a window and
    /// can only be used if the pixel type passed to createWindow 
    /// is GL_UNSIGNED_BYTE
    int setScanline(int winID, int y, vrpn_uint8 *line);
    /// tells the window manager to make the window visible
    int showWindow(int winID);
    /// tells the window manager to hide the window
    int hideWindow(int winID);
    /// sets up a callback to be called whenever user clicks or moves mouse 
    /// or types on keyboard while window is in focus. also called for window
    /// events such as a resize
    int setWindowEventHandler(int winID,
		ImageViewerWindowEventHandler f, void *ud);
    /// sets up callback which lets you draw stuff in the window
    int setWindowDisplayHandler(int winID,
		ImageViewerDisplayHandler f, void *ud);
    /// triggers call of display handler and redraw of the window
    int dirtyWindow(int winID);
    /// utility function to be called in window display handler which draws
    /// the image into the window scaled to the dimensions of the window 
    /// whatever they may be
    int drawImage(int winID);
    /// utility function that draws a string in some default font at a given
    /// location in the image
    int drawString(int x, int y, char *str);
    /// converts from the range (0..1) to window pixels
    int toPixels(int winID, double *x, double *y);
    /// converts from window pixels to the range (0..1)
    int toImage(int winID, double *x, double *y);
    /// clamps to the range of image pixel positions
    int clampToWindow(int winID, double *x, double *y);
    /// get the display for a given window
    ImageDisplay getDisplay(int winID) {return 
	dpy[window[winID-1].display_index];}

#ifdef V_GLUT
    static void displayCallbackForGLUT(void);
    static void idleCallbackForGLUT(void) {};
    static void visibilityCallbackForGLUT(int state);
    static void reshapeCallbackForGLUT(int w, int h);
    static void motionCallbackForGLUT(int x, int y);
    static void mouseCallbackForGLUT(int button, int state, int x, int y);
    static void keyboardCallbackForGLUT(unsigned char key, int x, int y);
#endif

  protected:
    ImageViewer();
    static ImageViewer *theViewer; // singleton pointer

  private:
#ifdef V_GLUT
    int get_window_index_from_glut_id(int glut_id);
#endif
    int num_displays;
    long event_mask;
    ImageDisplay dpy[MAX_DISPLAYS];

    int num_windows;
    ImageWindow window[MAX_WIN];
#ifndef V_GLUT
	GLuint font_dlist_base;
	int font_base, num_font_chars;
    XEvent event;
#endif
};

#endif

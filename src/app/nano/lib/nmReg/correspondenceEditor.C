/*===3rdtech===
  Copyright (c) 2001 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#include "correspondenceEditor.h"
#include <PPM.h>
#include <nmb_ColorMap.h>

CorrespondenceWindowParameters::CorrespondenceWindowParameters(): 
        left(0.0), right(1.0),
        bottom(0.0), top(1.0), im(NULL), winID(-1)
{}

CorrespondenceEditor::CorrespondenceEditor(int num_im,
                                                ImageViewer *view,
                                                Correspondence *corr,
                                                int *winIDs) {

    viewer = view;
    correspondence = corr;
    // set up the mapping from image numbers in correspondence to windows in
    // viewer and set up callbacks for events and display
    num_images = num_im;
    winParam = new CorrespondenceWindowParameters[num_im];
    int i;
    for (i = 0; i < num_images; i++){
        winParam[i].winID = winIDs[i];
        viewer->setWindowEventHandler(winParam[i].winID,
                CorrespondenceEditor::eventHandler, (void *)this);
        viewer->setWindowDisplayHandler(winParam[i].winID,
                CorrespondenceEditor::displayHandler, (void *)this);
    }
    selectedPointIndex = 0;
    grabbedPointIndex = 0;
    draggingPoint = VRPN_FALSE;
    grab_offset_x = 0;
    grab_offset_y = 0;
    point_marker_dlist = 0;
    change_handler = NULL;
}

CorrespondenceEditor::CorrespondenceEditor(int num_im, char **win_names) {
    viewer = ImageViewer::getImageViewer();
    correspondence = new Correspondence(num_im, DEFAULT_MAX_POINTS);
    char *display_name;
    display_name = (char *)getenv("V_X_DISPLAY");
    if (!display_name) {
       display_name = (char *)getenv("DISPLAY");
       if (!display_name) {
          display_name = "unix:0";
       }
    }

    char win_name[64];
    //printf("CorrespondenceEditor: opening display %s\n", display_name);
    viewer->init(display_name);
    num_images = num_im;
    winParam = new CorrespondenceWindowParameters[num_images];
    int i;
    for (i = 0; i < num_images; i++){
	if (!win_names)
        	sprintf(win_name, "data_registration%d", i);
	else
		sprintf(win_name, "%s", win_names[i]);
        winParam[i].winID = 
             viewer->createWindow(display_name,i*110 + 200,200,
                                  100,100,win_name);
        viewer->setWindowEventHandler(winParam[i].winID,
                CorrespondenceEditor::eventHandler, (void *)this);
        viewer->setWindowDisplayHandler(winParam[i].winID,
                CorrespondenceEditor::displayHandler, (void *)this);
    }
    selectedPointIndex = 0;
    grabbedPointIndex = 0;
    draggingPoint = VRPN_FALSE;
    grab_offset_x = 0;
    grab_offset_y = 0;
    point_marker_dlist = 0;
    change_handler = NULL;
}

// here is where user interaction is defined:
int CorrespondenceEditor::eventHandler(
                        const ImageViewerWindowEvent &event, void *ud)
{
    CorrespondenceEditor *me = (CorrespondenceEditor *)ud;

    // map the windowID to a spaceIndex:
    int spaceIndex= me->getSpaceIndex(event.winID);

    double max_x_grab_dist = 4*POINT_SIZE, max_y_grab_dist = 4*POINT_SIZE;

    me->viewer->toImageVec(event.winID, &max_x_grab_dist, &max_y_grab_dist);
    // convert to absolute distance
    max_y_grab_dist = fabs(max_y_grab_dist);
    max_x_grab_dist = fabs(max_x_grab_dist);

    switch(event.type) {
      case RESIZE_EVENT:
        break;
      case BUTTON_PRESS_EVENT:
        if (event.button == IV_LEFT_BUTTON){
            double x_im = event.mouse_x, y_im = event.mouse_y;
            me->viewer->toImagePnt(event.winID, &x_im, &y_im);
            // Inside or outside existing point?
            if (me->correspondence->findNearestPoint(spaceIndex, x_im, y_im,
                max_x_grab_dist, max_y_grab_dist,
                &(me->grabbedPointIndex))){
                // We are inside- prepare to drag this point
                corr_point_t grabbed_pnt;
                if (me->correspondence->getPoint(spaceIndex, 
                    me->grabbedPointIndex, &grabbed_pnt)) {
                    fprintf(stderr, "CorrespondenceEditor::eventHandler: "
                                    "getPoint failed\n");
                    return -1;
                }
                //printf("CE: point %f %f %f %f\n", x_im, y_im, 
                //       grabbed_pnt.x,grabbed_pnt.y); 
                me->draggingPoint = VRPN_TRUE;
                me->viewer->toPixelsPnt(event.winID, &(grabbed_pnt.x),
			&(grabbed_pnt.y));
                me->grab_offset_x = (int)(grabbed_pnt.x - event.mouse_x);
                me->grab_offset_y = (int)(grabbed_pnt.y - event.mouse_y);
                me->selectedPointIndex = me->grabbedPointIndex;
                int i;
                for (i = 0; i < me->num_images; i++)
                    me->viewer->dirtyWindow((me->winParam)[i].winID);
            } else {
                // We are outside, create a new point. 
                double x_im = event.mouse_x, y_im = event.mouse_y;
                me->viewer->toImagePnt(event.winID, &x_im, &y_im);
                corr_point_t p(x_im, y_im);
                int new_pntIdx = me->correspondence->addPoint(p);
                me->selectedPointIndex = me->grabbedPointIndex = new_pntIdx;
                // Also prepare to drag this new point. 
                me->draggingPoint = VRPN_TRUE;
                me->viewer->toPixelsPnt(event.winID, &(p.x),
                                     &(p.y));
                me->grab_offset_x = 0;
                me->grab_offset_y = 0;
                int i;
                for (i = 0; i < me->num_images; i++) {
                    me->viewer->dirtyWindow((me->winParam)[i].winID);
                }
                me->notifyCallbacks();
            }
        }
        break;
      case BUTTON_RELEASE_EVENT:
        if (event.button == IV_LEFT_BUTTON && me->draggingPoint) {
            double x_im = event.mouse_x + me->grab_offset_x;
            double y_im = event.mouse_y + me->grab_offset_y;
            // allow a 10 pixel tolerance for dragging outside the window to
            // make it easier for the user to set a point exactly on the 
            // border of the image without deleting it by accident
            if(me->viewer->clampToWindow(event.winID, &x_im, &y_im) > 10) {
                // If we dragged outside the window, delete the point. 
                me->correspondence->deletePoint(me->selectedPointIndex);
                me->selectedPointIndex = me->correspondence->numPoints()-1;
                // deleted point affects all windows. 
                int i;
                for (i = 0; i < me->num_images; i++) {
                    me->viewer->dirtyWindow((me->winParam)[i].winID);
                }
            } else {
                me->viewer->toImagePnt(event.winID, &x_im, &y_im);
                corr_point_t p(x_im, y_im);
                //printf("added point %g,%g\n", x_im, y_im);
                me->correspondence->setPoint(spaceIndex, 
                                    me->grabbedPointIndex, p);
                // moved point only affects active window. 
                me->viewer->dirtyWindow(event.winID);
            }
            me->draggingPoint = VRPN_FALSE;
            me->notifyCallbacks();
        } 
        break;
      case MOTION_EVENT:
        if ((event.state & IV_LEFT_BUTTON_MASK) && me->draggingPoint) {
            double x_im = event.mouse_x + me->grab_offset_x;
            double y_im = event.mouse_y + me->grab_offset_y;
            me->viewer->toImagePnt(event.winID, &x_im, &y_im);
	    corr_point_t p(x_im, y_im);
            me->correspondence->setPoint(spaceIndex, me->grabbedPointIndex, p);
            me->viewer->dirtyWindow(event.winID);
            me->notifyCallbacks();
        }
        break;
      case KEY_PRESS_EVENT:
        //printf("CorrespondenceEditor: got a key: %d\n", event.keycode);
        if (event.keycode == 127 ||
	    event.keycode == 8){ // 8 is for backspace, ^H. 
            // 127 for delete key. Delete key needs glut 3.7 or greater. 
            if (me->correspondence->numPoints() > 0){
                me->correspondence->deletePoint(me->selectedPointIndex);
            }
            me->selectedPointIndex = me->correspondence->numPoints()-1;
            int i;
            for (i = 0; i < me->num_images; i++) {
                me->viewer->dirtyWindow((me->winParam)[i].winID);
            }
            // Allow update of the registration automatically. 
            me->notifyCallbacks();
        } else if (event.keycode == 'z') {
          me->scaleImageRegion(event.winID, event.mouse_x, event.mouse_y, 0.5);
          me->viewer->dirtyWindow(event.winID);
        } else if (event.keycode == 'Z') {
          me->scaleImageRegion(event.winID, event.mouse_x, event.mouse_y, 2.0);
          me->viewer->dirtyWindow(event.winID);
        }
        break;
      default:
        break;
    }
    return 0;
}

int CorrespondenceEditor::getSpaceIndex(int winID){
    int i;
    for (i = 0; i < num_images; i++){
        if (winParam[i].winID == winID) break;
    }
    if (i == num_images) return -1;
    else
        return i;
}

int CorrespondenceEditor::scaleImageRegion(int winID, 
                          double x_win, double y_win, double scale)
{
  int index = getSpaceIndex(winID);
  if (index < 0) return -1;

  double x_im = x_win, y_im = y_win;
  if (viewer->toImagePnt(winID, winParam[index].left, winParam[index].right,
                                winParam[index].bottom, winParam[index].top,
                                &x_im, &y_im)) {
    return -1;
  }

  winParam[index].left = x_im + scale*(winParam[index].left - x_im);
  winParam[index].right = x_im + scale*(winParam[index].right - x_im);
  winParam[index].bottom = y_im + scale*(winParam[index].bottom - y_im);
  winParam[index].top = y_im + scale*(winParam[index].top - y_im);

  // the following is to ensure that the region falls inside (0..1,0..1) 
  // while preserving the scale if possible
  clampImageRegion(index);
  return 0;
}

int CorrespondenceEditor::clampImageRegion(int index)
{
  vrpn_bool xFlipped, yFlipped;
  getImageOrientation(index, xFlipped, yFlipped);

  if ((fabs(winParam[index].right - winParam[index].left) > 1.0) ||
      (fabs(winParam[index].top - winParam[index].bottom) > 1.0)) {
    if (xFlipped) {
      winParam[index].left = 1.0;
      winParam[index].right = 0.0;
    } else {
      winParam[index].left = 0.0;
      winParam[index].right = 1.0;
    }
    if (yFlipped) {
      winParam[index].bottom = 1.0;
      winParam[index].top = 1.0;
    } else {
      winParam[index].bottom = 0;
      winParam[index].top = 1.0;
    }
  }
  if (xFlipped) {
    if (winParam[index].right < 0) {
      winParam[index].left -= winParam[index].right;
      winParam[index].right = 0.0;
    } else if (winParam[index].left > 1) {
      winParam[index].right += 1.0 - winParam[index].left;
      winParam[index].left = 1.0;
    }
  } else {
    if (winParam[index].left < 0) {
      winParam[index].right -= winParam[index].left;
      winParam[index].left = 0.0;
    } else if (winParam[index].right > 1) {
      winParam[index].left += 1.0 - winParam[index].right;
      winParam[index].right = 1.0;
    }
  }
  if (yFlipped) {
    if (winParam[index].top < 0) {
      winParam[index].bottom -= winParam[index].top;
      winParam[index].top = 0.0;
    } else if (winParam[index].bottom > 1) {
      winParam[index].top += 1.0 - winParam[index].bottom;
      winParam[index].bottom = 1.0;
    }
  } else {
    if (winParam[index].bottom < 0) {
      winParam[index].top -= winParam[index].bottom;
      winParam[index].bottom = 0.0;
    } else if (winParam[index].top > 1) {
      winParam[index].bottom += 1.0 - winParam[index].top;
      winParam[index].top = 1.0;
    }
  }
  return 0;
}

int CorrespondenceEditor::displayHandler(
                        const ImageViewerDisplayData &data, void *ud)
{
    CorrespondenceEditor *me = (CorrespondenceEditor *)ud;

    glClearColor(0.0, 0.0, 0.0,0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // draw the image for this window:
    // but first figure out which image is displayed in this window
    int spaceIndex = me->getSpaceIndex(data.winID);

    glViewport(0,0,data.winWidth, data.winHeight);
    nmb_TransformMatrix44 W2I; // assumed to be initialized as identity
    CorrespondenceWindowParameters *params = &(me->winParam[spaceIndex]);
    double left, right, bottom, top;
    left = params->left;
    right = params->right;
    bottom = params->bottom;
    top = params->top;

    me->viewer->drawImage(data.winID, params->im,
            1.0, 1.0, 1.0, 1.0, 
            &left, &right, &bottom, &top, 
            &W2I);

//    me->viewer->drawImage(data.winID);   

    int i;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // set up projection so we can draw on top of image such that
    // vertices are in image coordinates scaled by the window size relative
    // to the image size
    glOrtho(data.winWidth, 0, data.winHeight, 0, -1, 1);
    glPixelZoom(1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glColor3f(1.0, 1.0, 1.0);
    corr_point_t image_pnt;

    char num_str[16];

    int num_pnts = me->correspondence->numPoints();
    for (i = 0; i < num_pnts; i++){
        me->correspondence->getPoint(spaceIndex, i, &image_pnt);
        // convert point to the right location in the window by scaling it
        me->viewer->toPixelsPnt(data.winID, &(image_pnt.x), &(image_pnt.y));
        // now draw the point
        me->drawCrosshair(data.winWidth-image_pnt.x, image_pnt.y);
        sprintf(num_str, "%d", i);
        glColor3f(1.0, 1.0, 1.0);
        me->viewer->drawString(data.winWidth-image_pnt.x-1, 
                               image_pnt.y - 3, num_str);
        if (i == me->selectedPointIndex)
            me->drawSelectionBox(data.winWidth-image_pnt.x, image_pnt.y);
    }
    return 0;
}

void CorrespondenceEditor::notifyCallbacks()
{
    Correspondence copy;
    getCorrespondence(copy);
    if (change_handler) {
      change_handler(copy, userdata);
    }
}

void CorrespondenceEditor::drawCrosshair(float x, float y) {
    glPushAttrib(GL_PIXEL_MODE_BIT);
    if (point_marker_dlist){
        glPushMatrix();
        glTranslatef(x+0.5,y+0.5, 0.0);
        glCallList(point_marker_dlist);
        glPopMatrix();
    }
    else {
#ifndef V_GLUT	// this is because of some strange bug that causes display
		// lists not to draw correctly, perhaps they are getting
		// corrupted - see problems in vlib window
        point_marker_dlist = glGenLists(1);
#endif
        glPushMatrix();
        glTranslatef(x+0.5,y+0.5,0.0);
#ifndef V_GLUT
        glNewList(point_marker_dlist, GL_COMPILE_AND_EXECUTE);
#endif
        glLineWidth(1.0);
        glColor3f(0.0, 0.0, 0.0);
        glBegin(GL_LINES);
        glVertex2f(-POINT_SIZE*4, 1.0);
        glVertex2f(+POINT_SIZE*4, 1.0);
        glVertex2f(1.0,-POINT_SIZE*4);
        glVertex2f(1.0,+POINT_SIZE*4);
        glEnd();

        glColor3f(0.0, 0.0, 0.0);
        glBegin(GL_LINES);
        glVertex2f(-POINT_SIZE*4, -1.0);
        glVertex2f(+POINT_SIZE*4, -1.0);
        glVertex2f(-1.0,-POINT_SIZE*4);
        glVertex2f(-1.0,+POINT_SIZE*4);
        glEnd();

        glColor3f(1.0, 1.0, 1.0);
        glBegin(GL_LINES);
        glVertex2f(-POINT_SIZE*4, 0.0);
        glVertex2f(+POINT_SIZE*4, 0.0);
        glVertex2f(0.0,-POINT_SIZE*4);
        glVertex2f(0.0,+POINT_SIZE*4);
        glEnd();
#ifndef V_GLUT
        glEndList();
#endif
        glPopMatrix();

    }

    glPopAttrib();
}

void CorrespondenceEditor::drawSelectionBox(int xp, int yp){
    int x = xp;
    int y = yp+1;
    glColor3f(1.0, 1.0, 0.0);
    glPushAttrib(GL_PIXEL_MODE_BIT);
    glPixelZoom(1.0, 1.0);
    glLineWidth(2.0);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x-4*POINT_SIZE,y-4*POINT_SIZE);
    glVertex2f(x+4*POINT_SIZE,y-4*POINT_SIZE);
    glVertex2f(x+4*POINT_SIZE,y+4*POINT_SIZE);
    glVertex2f(x-4*POINT_SIZE,y+4*POINT_SIZE);
    glEnd();
    glPopAttrib();
}

void CorrespondenceEditor::show() {
    int i;
    for (i = 0; i < num_images; i++){
        viewer->showWindow(winParam[i].winID);
    }
    //printf("CorrespondenceEditor: finished opening windows\n");
}

void CorrespondenceEditor::hide() {
    int i;
    for (i = 0; i < num_images; i++){
        viewer->hideWindow(winParam[i].winID);
    }
}

/** 
   x, y are in the range 0..1 and z is in nanometers

*/

void CorrespondenceEditor::addFiducial(float *x, float *y, float *z)
{
    corr_point_t p;
    if (z) {
      p.is2D = vrpn_FALSE;
    }
    int pntIndex = correspondence->addPoint(p);
    int i;
    if (pntIndex >= 0) {
      for (i = 0; i < num_images; i++) {
         p.x = x[i]; p.y = y[i]; 
         if (z) {
           p.z = z[i];
         }
         correspondence->setPoint(i, pntIndex, p);
      }
    }

    for (i = 0; i < num_images; i++) {
      viewer->dirtyWindow((winParam)[i].winID);
    }
}

int CorrespondenceEditor::setImage(int spaceIndex, nmb_Image *im) {
    int width, height;
    //int i,j;
    int im_w, im_h;
    //double val;

    if (winParam[spaceIndex].im) {
      nmb_Image::deleteImage(winParam[spaceIndex].im);
    }
    winParam[spaceIndex].im = new nmb_ImageGrid(im);
    winParam[spaceIndex].im->normalize();

    width = 300;
    height = (width*im->height())/(double)(im->width());

    im_w = im->width();
    im_h = im->height();

    viewer->setWindowSize(winParam[spaceIndex].winID, width, height);
/*
    viewer->setWindowImageSize(winParam[spaceIndex].winID, im_w, im_h);
    // printf("CorrespondenceEditor::setImage: \n");
    viewer->setValueRange(winParam[spaceIndex].winID, 
                          im->minNonZeroValue(), im->maxValue());
    // printf(" min,max = %f, %f\n", im->minNonZeroValue(), im->maxValue());

    for (i = 0; i < im_w; i++){
        for (j = 0; j < im_h; j++){
            //val = im->getValue(i,im_h -j -1);   // we need to flip y
            val = im->getValue(i,j);
            viewer->setValue(winParam[spaceIndex].winID, i, j, val);
        }
    }
*/

    viewer->dirtyWindow(winParam[spaceIndex].winID);
    return 0;
}

int CorrespondenceEditor::setImageOrientation(int spaceIndex, 
                          vrpn_bool flipX, vrpn_bool flipY)
{
    double temp;
    if (( flipX && (winParam[spaceIndex].left < winParam[spaceIndex].right)) ||
        (!flipX && (winParam[spaceIndex].left > winParam[spaceIndex].right))) {
      temp = winParam[spaceIndex].left;
      winParam[spaceIndex].left = winParam[spaceIndex].right;
      winParam[spaceIndex].right = temp;
    }

    if (( flipY && (winParam[spaceIndex].bottom < winParam[spaceIndex].top)) ||
        (!flipY && (winParam[spaceIndex].bottom > winParam[spaceIndex].top))) {
      temp = winParam[spaceIndex].bottom;
      winParam[spaceIndex].bottom = winParam[spaceIndex].top;
      winParam[spaceIndex].top = temp;
    }

    viewer->dirtyWindow(winParam[spaceIndex].winID);
    return 0;
}

int CorrespondenceEditor::getImageOrientation(int spaceIndex,
                          vrpn_bool &flipX, vrpn_bool &flipY)
{
  flipX = (winParam[spaceIndex].left > winParam[spaceIndex].right);
  flipY = (winParam[spaceIndex].bottom > winParam[spaceIndex].top);
  return 0;
}

/* ************************************
int CorrespondenceEditor::setImageFromPlane(int spaceIndex, BCPlane *p) {
    int im_w, im_h;
    int i,j;
    double val;

    im_w = p->numX();
    im_h = p->numY();
    viewer->setWindowImageSize(winParam[spaceIndex].winID, im_w, im_h);
    viewer->setValueRange(winParam[spaceIndex].winID, 
                          p->minValue(), p->maxValue());
    for (i = 0; i < im_w; i++){
        for (j = 0; j < im_h; j++){
            //val = p->value(i,im_h -j -1);	// we need to flip y
            val = p->value(i,j);
            viewer->setValue(winParam[spaceIndex].winID, i, j, val);
        }
    }
    viewer->setWindowSize(win_ids[spaceIndex], im_w, im_h);
    return 0;
}

int CorrespondenceEditor::setImageFromPNM(int spaceIndex, PNMImage &im)
{
    int im_w, im_h;
    int i,j;
    double val;

    im_w = im.Columns();
    im_h = im.Rows();
    double im_min, im_max;
    im_min = im.Pixel(0, 0, 0);
    im_max = im_min;
    for (i = 0; i < im_w; i++){
        for (j = 0; j < im_h; j++){
            if (im.Pixel(j, i, 0) > im_max)
                im_max = im.Pixel(j, i, 0);
            else if (im.Pixel(j, i, 0) < im_min)
                im_min = im.Pixel(j, i, 0);
        }
    }
    viewer->setWindowImageSize(winParam[spaceIndex].winID, im_w, im_h);
    viewer->setValueRange(winParam[spaceIndex].winID, im_min, im_max);
    for (i = 0; i < im_w; i++){
        for (j = 0; j < im_h; j++){
            val = im.Pixel(j, i, 0);
            viewer->setValue(winParam[spaceIndex].winID, i, j, val);
        }
    }
    viewer->setWindowSize(win_ids[spaceIndex], im_w, im_h);
    return 0;
}

int CorrespondenceEditor::setImageFromPNM(int spaceIndex, PPM *im)
{
    int im_w, im_h;
    int i,j;
    double val;

    im_w = im->nx;
    im_h = im->ny;
    int r,g,b;
    im->Tellppm(0, 0, &r, &g, &b);
    double im_min = r, im_max = r;
    for (i = 0; i < im_w; i++){
        for (j = 0; j < im_h; j++){
            im->Tellppm(i, j, &r, &g, &b);
            if (r > im_max)
                im_max = r;
            else if (r < im_min)
                im_min = r;
        }
    }
    viewer->setWindowImageSize(winParam[spaceIndex].winID, im_w, im_h);
    viewer->setValueRange(winParam[spaceIndex].winID, im_min, im_max);
    for (i = 0; i < im_w; i++){
        for (j = 0; j < im_h; j++){
            im->Tellppm(i, j, &r, &g, &b);  // flip y??
            val = r;
            viewer->setValue(winParam[spaceIndex].winID, i, j, val);
        }
    }
    viewer->setWindowSize(win_ids[spaceIndex], im_w, im_h);
    return 0;
}
************************************************ */

void CorrespondenceEditor::mainloop() {
    if (viewer) viewer->mainloop();
}

// this returns a copy of the current correspondence
void CorrespondenceEditor::getCorrespondence(Correspondence &corr){
    corr = (*correspondence);
}

void CorrespondenceEditor::registerCallback(CorrespondenceCallback handler,
                                            void *ud)
{
    change_handler = handler;
    userdata = ud;
}

int CorrespondenceEditor::setColorMap(int spaceIndex, nmb_ColorMap * cmap)
{
    viewer->setColorMap(winParam[spaceIndex].winID, cmap);
    viewer->dirtyWindow(winParam[spaceIndex].winID);
    return 0;
}

int CorrespondenceEditor::setColorMinMax(int spaceIndex, 
                       vrpn_float64 dmin, vrpn_float64 dmax,
                       vrpn_float64 cmin, vrpn_float64 cmax)
{
    viewer->setColorMinMax(winParam[spaceIndex].winID, dmin, dmax, cmin, cmax);
    viewer->dirtyWindow(winParam[spaceIndex].winID);
    return 0;
}

// This is some driver code I used to test this stuff:
/*main ()
{
    Correspondence *c = new Correspondence(2, 100);
    ImageViewer *v = new ImageViewer();
    v->init(0);

    int window_ids[2];
    window_ids[0] = v->createWindow(NULL, 100, 100, 100, 100, "win0");
    window_ids[1] = v->createWindow(NULL, 200, 100, 100, 100, "win1");

    v->setWindowImageSize(window_ids[0], 100, 100);
    int i;
    double val;
    for (i = 0; i < 10000; i++) {
        val = sin((double)i*2.0*M_PI/2000.0);
        v->setValue(window_ids[0], i % 100, i/100, val);
    }

    CorrespondenceEditor *e = new CorrespondenceEditor(
                2, v, c, window_ids);


    int j;
    while(1){
        v->mainloop();
        for (i = 0; i < 10000; i++) {
            val = sin((double)(i+j*30)*2.0*M_PI/2000.0);
            v->setValue(window_ids[0], i % 100, i/100, val);
        }
        j++;
    }
}
*/


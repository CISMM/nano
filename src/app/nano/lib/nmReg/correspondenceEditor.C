#include "correspondenceEditor.h"

CorrespondenceEditor::CorrespondenceEditor(int num_im,
                                                ImageViewer *view,
                                                Correspondence *corr,
                                                int *winIDs) {

    viewer = view;
    correspondence = corr;
    // set up the mapping from image numbers in correspondence to windows in
    // viewer and set up callbacks for events and display
    num_images = num_im;
    win_ids = new int[num_images];
    int i;
    for (i = 0; i < num_images; i++){
        win_ids[i] = winIDs[i];
        viewer->setWindowEventHandler(win_ids[i],
                CorrespondenceEditor::eventHandler, (void *)this);
        viewer->setWindowDisplayHandler(win_ids[i],
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
    win_ids = new int[num_images];
    int i;
    for (i = 0; i < num_images; i++){
	if (!win_names)
        	sprintf(win_name, "data_registration%d", i);
	else
		sprintf(win_name, "%s", win_names[i]);
        win_ids[i] = viewer->createWindow(display_name,0,0,400,400,win_name);
        viewer->setWindowEventHandler(win_ids[i],
                CorrespondenceEditor::eventHandler, (void *)this);
        viewer->setWindowDisplayHandler(win_ids[i],
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

    me->viewer->toImage(event.winID, &max_x_grab_dist, &max_y_grab_dist);

    switch(event.type) {
      case RESIZE_EVENT:
        break;
      case BUTTON_PRESS_EVENT:
        if (event.button == IV_LEFT_BUTTON){
            double x_im = event.mouse_x, y_im = event.mouse_y;
            me->viewer->toImage(event.winID, &x_im, &y_im);
            if (me->correspondence->findNearestPoint(spaceIndex, x_im, y_im,
                max_x_grab_dist, max_y_grab_dist,
                &(me->grabbedPointIndex))){
                corr_point_t grabbed_pnt;
                if (me->correspondence->getPoint(spaceIndex, 
                    me->grabbedPointIndex, &grabbed_pnt)) {
                    fprintf(stderr, "CorrespondenceEditor::eventHandler: "
                                    "getPoint failed\n");
                    return -1;
                }
                me->draggingPoint = VRPN_TRUE;
                me->viewer->toPixels(event.winID, &(grabbed_pnt.x),
			&(grabbed_pnt.y));
                me->grab_offset_x = (int)(grabbed_pnt.x - event.mouse_x);
                me->grab_offset_y = (int)(grabbed_pnt.y - event.mouse_y);
                me->selectedPointIndex = me->grabbedPointIndex;
                int i;
                for (i = 0; i < me->num_images; i++)
                    me->viewer->dirtyWindow((me->win_ids)[i]);
            }
        }
        break;
      case BUTTON_RELEASE_EVENT:
        if (event.button == IV_RIGHT_BUTTON){
            double x_im = event.mouse_x, y_im = event.mouse_y;
            me->viewer->toImage(event.winID, &x_im, &y_im);
	    corr_point_t p(x_im, y_im);
            int new_pntIdx = me->correspondence->addPoint(p);
            me->selectedPointIndex = new_pntIdx;
            int i;
            for (i = 0; i < me->num_images; i++)
                me->viewer->dirtyWindow((me->win_ids)[i]);
            me->notifyCallbacks();
        }
        else if (event.button == IV_LEFT_BUTTON && me->draggingPoint) {
            double x_im = event.mouse_x + me->grab_offset_x;
            double y_im = event.mouse_y + me->grab_offset_y;
            me->viewer->clampToWindow(event.winID, &x_im, &y_im);
            me->viewer->toImage(event.winID, &x_im, &y_im);
	    corr_point_t p(x_im, y_im);
            me->correspondence->setPoint(spaceIndex, me->grabbedPointIndex, p);
            me->draggingPoint = VRPN_FALSE;
            me->viewer->dirtyWindow(event.winID);
            me->notifyCallbacks();
        }
        break;
      case MOTION_EVENT:
        if ((event.state & IV_LEFT_BUTTON_MASK) && me->draggingPoint) {
            double x_im = event.mouse_x + me->grab_offset_x;
            double y_im = event.mouse_y + me->grab_offset_y;
            me->viewer->toImage(event.winID, &x_im, &y_im);
	    corr_point_t p(x_im, y_im);
            me->correspondence->setPoint(spaceIndex, me->grabbedPointIndex, p);
            me->viewer->dirtyWindow(event.winID);
            me->notifyCallbacks();
        }
        break;
      case KEY_PRESS_EVENT:
        printf("CorrespondenceEditor: got a key: %d\n", event.keycode);
        if (event.keycode == 108 ||
	    event.keycode == 8){ // X windows number for delete key
            if (me->correspondence->numPoints() > 0){
                me->correspondence->deletePoint(me->selectedPointIndex);
            }
            me->selectedPointIndex = me->correspondence->numPoints()-1;
            int i;
            for (i = 0; i < me->num_images; i++)
                me->viewer->dirtyWindow((me->win_ids)[i]);
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
        if (win_ids[i] == winID) break;
    }
    if (i == num_images) return -1;
    else
        return i;
}

int CorrespondenceEditor::displayHandler(
                        const ImageViewerDisplayData &data, void *ud)
{
    CorrespondenceEditor *me = (CorrespondenceEditor *)ud;

    glClearColor(0.0, 0.0, 0.0,0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // draw the image for this window:
    me->viewer->drawImage(data.winID);

    int i;
    glViewport(0,0,data.winWidth, data.winHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // set up projection so we can draw on top of image such that
    // vertices are in image coordinates scaled by the window size relative
    // to the image size
    glOrtho(0, data.winWidth, data.winHeight, 0, -1, 1);
    glPixelZoom(1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glColor3f(1.0, 1.0, 1.0);
    corr_point_t image_pnt;

    char num_str[16];

    // figure out which image is displayed in this window
    int spaceIndex = me->getSpaceIndex(data.winID);
    int num_pnts = me->correspondence->numPoints();
    for (i = 0; i < num_pnts; i++){
        me->correspondence->getPoint(spaceIndex, i, &image_pnt);
        // convert point to the right location in the window by scaling it
        me->viewer->toPixels(data.winID, &(image_pnt.x), &(image_pnt.y));
        // now draw the point
        me->drawCrosshair(image_pnt.x, image_pnt.y);
        sprintf(num_str, "%d", i);
        glColor3f(1.0, 1.0, 1.0);
        me->viewer->drawString(image_pnt.x+1, image_pnt.y - 3, num_str);
        if (i == me->selectedPointIndex)
            me->drawSelectionBox(image_pnt.x, image_pnt.y);
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
        viewer->showWindow(win_ids[i]);
    }
    //printf("CorrespondenceEditor: finished opening windows\n");
}

void CorrespondenceEditor::hide() {
    int i;
    for (i = 0; i < num_images; i++){
        viewer->hideWindow(win_ids[i]);
    }
}

/** 
   x, y are in the range 0..1 and z is in nanometers

*/

void CorrespondenceEditor::addFiducial(int spaceIndex, 
                                       float x, float y, float z)
{
    corr_point_t p(x, y, z);
    correspondence->addPoint(p);
}

int CorrespondenceEditor::setImage(int spaceIndex, nmb_Image *im) {
    int im_w, im_h;
    int i,j;
    double val;

    im_w = im->width();
    im_h = im->height();
    viewer->setWindowImageSize(win_ids[spaceIndex], im_w, im_h);
    // printf("CorrespondenceEditor::setImage: \n");
    viewer->setValueRange(win_ids[spaceIndex], im->minNonZeroValue(), im->maxValue());
    // printf(" min,max = %f, %f\n", im->minNonZeroValue(), im->maxValue());
    for (i = 0; i < im_w; i++){
        for (j = 0; j < im_h; j++){
            val = im->getValue(i,im_h -j -1);   // we need to flip y
            viewer->setValue(win_ids[spaceIndex], i, j, val);
        }
    }
    viewer->dirtyWindow(win_ids[spaceIndex]);
    return 0;
}

int CorrespondenceEditor::setImageFromPlane(int spaceIndex, BCPlane *p) {
    int im_w, im_h;
    int i,j;
    double val;

    im_w = p->numX();
    im_h = p->numY();
    viewer->setWindowImageSize(win_ids[spaceIndex], im_w, im_h);
    viewer->setValueRange(win_ids[spaceIndex], p->minValue(), p->maxValue());
    for (i = 0; i < im_w; i++){
        for (j = 0; j < im_h; j++){
            val = p->value(i,im_h -j -1);	// we need to flip y
            viewer->setValue(win_ids[spaceIndex], i, j, val);
        }
    }
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
    viewer->setWindowImageSize(win_ids[spaceIndex], im_w, im_h);
    viewer->setValueRange(win_ids[spaceIndex], im_min, im_max);
    for (i = 0; i < im_w; i++){
        for (j = 0; j < im_h; j++){
            val = im.Pixel(j, i, 0);
            viewer->setValue(win_ids[spaceIndex], i, j, val);
        }
    }
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
    viewer->setWindowImageSize(win_ids[spaceIndex], im_w, im_h);
    viewer->setValueRange(win_ids[spaceIndex], im_min, im_max);
    for (i = 0; i < im_w; i++){
        for (j = 0; j < im_h; j++){
            im->Tellppm(i, j, &r, &g, &b);  // flip y
            val = r;
            viewer->setValue(win_ids[spaceIndex], i, j, val);
        }
    }
    return 0;
}

void CorrespondenceEditor::mainloop() {
    if (viewer) viewer->mainloop();
}

// this returns a copy of the current correspondence
void CorrespondenceEditor::getCorrespondence(Correspondence &corr){
    corr.init(correspondence->numSpaces(), correspondence->numPoints());
    int i,j;
    corr_point_t p;
    for (j = 0; j < correspondence->numPoints(); j++){
        corr.addPoint(p);
        for (i = 0; i < correspondence->numSpaces(); i++){
            correspondence->getPoint(i, j, &p);
            corr.setPoint(i,j, p);
        }
    }
}

void CorrespondenceEditor::registerCallback(CorrespondenceCallback handler,
                                            void *ud)
{
    change_handler = handler;
    userdata = ud;
}

// This is some driver code I used to test this stuff:
/*main ()
{
    Correspondence *c = new Correspondence(2, 100);
    ImageViewer *v = new ImageViewer();
    v->init(0);

    int window_ids[2];
    window_ids[0] = v->createWindow(0, 0, 0, 400, 400, "win0");
    window_ids[1] = v->createWindow(0, 140, 0, 400, 400, "win1");

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


#include "patternEditor.h"
#include "GL/gl.h"

PatternEditor::PatternEditor()
{
   d_viewer = ImageViewer::getImageViewer();
   char *display_name = (char *)getenv("DISPLAY");
   if (!display_name) {
      display_name = "unix:0";
   }
   d_viewer->init(display_name);
   d_mainWinID = d_viewer->createWindow(display_name, 
         0, 0, 300, 300, "Pattern Editor");
   d_viewer->setWindowEventHandler(d_mainWinID, 
         PatternEditor::mainWinEventHandler, this);
   d_viewer->setWindowDisplayHandler(d_mainWinID, 
         PatternEditor::mainWinDisplayHandler, this);

   d_navWinID = d_viewer->createWindow(display_name,
         300, 0, 100, 100, "Navigation");
   d_viewer->setWindowEventHandler(d_navWinID,
         PatternEditor::navWinEventHandler, this);
   d_viewer->setWindowDisplayHandler(d_navWinID,
         PatternEditor::navWinDisplayHandler, this);

   d_worldMinX_nm = -1;
   d_worldMinY_nm = -1;
   d_worldMaxX_nm = 2;
   d_worldMaxY_nm = 2;

   d_mainWinMinX_nm = 0;
   d_mainWinMinY_nm = 0;
   d_mainWinMaxX_nm = 1;
   d_mainWinMaxY_nm = 1;
   
   d_settingRegion = VRPN_FALSE;
   d_settingTranslation = VRPN_FALSE;

   d_dragStartX_nm = d_worldMinX_nm;
   d_dragStartY_nm = d_worldMinY_nm;
   d_dragEndX_nm = d_worldMaxX_nm;
   d_dragEndY_nm = d_worldMaxY_nm;
   
}

PatternEditor::~PatternEditor()
{
   d_viewer->destroyWindow(d_navWinID);
   d_viewer->destroyWindow(d_mainWinID);
}

void PatternEditor::addImage(nmb_Image *im)
{
   nmb_ImageBounds ib;
   im->getBounds(ib);
   nmb_ImageBounds::ImageBoundPoint points[4] = 
     {nmb_ImageBounds::MIN_X_MIN_Y, nmb_ImageBounds::MIN_X_MAX_Y,
      nmb_ImageBounds::MAX_X_MIN_Y, nmb_ImageBounds::MAX_X_MAX_Y};
   
   d_images.insert(d_images.begin(), im);

   for (int i = 0; i < 4; i++) {
       d_worldMinX_nm = min(d_worldMinX_nm, ib.getX(points[i]));
       d_worldMaxX_nm = max(d_worldMaxX_nm, ib.getX(points[i]));
       d_worldMinY_nm = min(d_worldMinY_nm, ib.getY(points[i]));
       d_worldMaxY_nm = max(d_worldMaxY_nm, ib.getY(points[i]));
   }
   d_mainWinMinX_nm = d_worldMinX_nm;
   d_mainWinMinY_nm = d_worldMinY_nm;
   d_mainWinMaxX_nm = d_worldMaxX_nm;
   d_mainWinMaxY_nm = d_worldMaxY_nm;
}

void PatternEditor::removeImage(nmb_Image *im)
{
   d_images.remove(im);
}

void PatternEditor::show() 
{
   d_viewer->showWindow(d_mainWinID);
   d_viewer->showWindow(d_navWinID);
}

int PatternEditor::mainWinEventHandler(
                   const ImageViewerWindowEvent &event, void *ud)
{
    PatternEditor *me = (PatternEditor *)ud;

    double x = event.mouse_x, y = event.mouse_y;
    double centerX_nm, centerY_nm;

    switch(event.type) {
      case RESIZE_EVENT:
         break;
      case MOTION_EVENT:
         break;
      case BUTTON_PRESS_EVENT:
         break;
      case BUTTON_RELEASE_EVENT:
         break;
      case KEY_PRESS_EVENT:
         switch(event.keycode) {
           case 'z':
             me->d_viewer->toImage(event.winID, &x, &y);
             me->mainWinPositionToWorld(x, y,
                 centerX_nm, centerY_nm);
             me->zoomBy(centerX_nm, centerY_nm, 2.0);
             me->d_viewer->dirtyWindow(me->d_navWinID);
             me->d_viewer->dirtyWindow(event.winID);
             break;
           case 'Z':
             me->d_viewer->toImage(event.winID, &x, &y);
             me->mainWinPositionToWorld(x, y,
                 centerX_nm, centerY_nm);
             me->zoomBy(centerX_nm, centerY_nm, 0.5);
             me->d_viewer->dirtyWindow(me->d_navWinID);
             me->d_viewer->dirtyWindow(event.winID);
             break;
           default:
             break;
         }
  
         break;
      default:
         break;
    }
    return 0;
}

void PatternEditor::zoomBy(double centerX_nm, double centerY_nm,
                           double magFactor)
{
    double width, height;
    if (magFactor == 0) {
       fprintf(stderr, "PatternEditor::zoomBy: Error, can't zoom by 0\n");
       return;
    }
    width = (d_mainWinMaxX_nm - d_mainWinMinX_nm)/magFactor;
    height = (d_mainWinMaxY_nm - d_mainWinMinY_nm)/magFactor;

    d_mainWinMinX_nm = centerX_nm - 0.5*width;
    d_mainWinMaxX_nm = centerX_nm + 0.5*width;
    d_mainWinMinY_nm = centerY_nm - 0.5*height;
    d_mainWinMaxY_nm = centerY_nm + 0.5*height;

    if (d_mainWinMinX_nm < d_worldMinX_nm) {
       d_mainWinMinX_nm = d_worldMinX_nm;
    }
    if (d_mainWinMinY_nm < d_worldMinY_nm) {
       d_mainWinMinY_nm = d_worldMinY_nm;
    }
    if (d_mainWinMaxX_nm > d_worldMaxX_nm) {
       d_mainWinMaxX_nm = d_worldMaxX_nm;
    }
    if (d_mainWinMaxY_nm > d_worldMaxY_nm) {
       d_mainWinMaxY_nm = d_worldMaxY_nm;
    }
}

int PatternEditor::mainWinDisplayHandler(
                   const ImageViewerDisplayData &data, void *ud)
{
  PatternEditor *me = (PatternEditor *)ud;

  glPushAttrib(GL_ENABLE_BIT | GL_TEXTURE_BIT | GL_VIEWPORT_BIT);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glMatrixMode(GL_TEXTURE);
  glPushMatrix();

  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


  GLfloat border_color[] = {0.0, 0.0, 0.0, 1.0};
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color);
  glTexParameterf(GL_TEXTURE_2D,
                  GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexEnvi(GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_BLEND);
  GLfloat textureColor[] = {0.5, 0.5, 0.5, 0.5};
  glTexEnvfv(GL_TEXTURE_2D, GL_TEXTURE_ENV_COLOR, textureColor);

  glEnable(GL_BLEND);

  // XXX - may need to change this in the future
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  GLfloat eyePlaneS[] =
       {1.0, 0.0, 0.0, 0.0};
  GLfloat eyePlaneT[] =
       {0.0, 1.0, 0.0, 0.0};
  GLfloat eyePlaneR[] =
       {0.0, 0.0, 1.0, 0.0};
  GLfloat eyePlaneQ[] =
       {0.0, 0.0, 0.0, 1.0};

  glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
  glTexGenfv(GL_S, GL_OBJECT_PLANE, eyePlaneS);

  glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
  glTexGenfv(GL_T, GL_OBJECT_PLANE, eyePlaneT);

  glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
  glTexGenfv(GL_R, GL_OBJECT_PLANE, eyePlaneR);

  glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
  glTexGenfv(GL_Q, GL_OBJECT_PLANE, eyePlaneQ);

  glEnable(GL_TEXTURE_2D);
  glEnable(GL_TEXTURE_GEN_S);
  glEnable(GL_TEXTURE_GEN_T);
  glEnable(GL_TEXTURE_GEN_R);
  glEnable(GL_TEXTURE_GEN_Q);

  // setup viewing/model projection
  glViewport(0, 0, data.winWidth, data.winHeight);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glOrtho(me->d_mainWinMinX_nm, me->d_mainWinMaxX_nm, 
          me->d_mainWinMinY_nm, me->d_mainWinMaxY_nm, -1, 1);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // setup texture transformation
  glMatrixMode(GL_TEXTURE);
  glLoadIdentity();

  double worldToImage[16];
  void *texture;
  int width, height;

  // for each image, load the texture matrix, setup automatic texture-mapping
  // of that image, draw a simple polygon on which to texture the image
  list<nmb_Image *>::iterator currImage;
  int i = 0;
  for (currImage = me->d_images.begin(); 
       currImage != me->d_images.end(); currImage++)
  {
     vrpn_bool textureOkay = VRPN_TRUE;
     texture = (*currImage)->pixelData();
     width = (*currImage)->width();
     height = (*currImage)->height();
     int pixType;
     switch ((*currImage)->pixelType()) {
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
       printf("Loading texture %s with type %d\n", 
              (*currImage)->name()->Characters(), (*currImage)->pixelType());
       gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, width, height, GL_LUMINANCE,
              pixType, texture);
       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
              GL_LINEAR_MIPMAP_LINEAR);
       (*currImage)->getWorldToImageTransform(worldToImage);
       nmb_ImageBounds ib;
       (*currImage)->getBounds(ib);
       for (int j = 0; j < 4; j++){
           printf("%g %g %g %g\n", worldToImage[j*4], worldToImage[j*4+1],
                                   worldToImage[j*4+2], worldToImage[j*4+3]);
       }
       printf("bounds : (%g,%g) -> (%g,%g)\n",
            ib.getX(nmb_ImageBounds::MIN_X_MIN_Y), 
            ib.getY(nmb_ImageBounds::MIN_X_MIN_Y),
            ib.getX(nmb_ImageBounds::MAX_X_MAX_Y), 
            ib.getY(nmb_ImageBounds::MAX_X_MAX_Y));
       glLoadMatrixd(worldToImage);
       glBegin(GL_POLYGON);
       glNormal3f(0.0, 0.0, 1.0);
       glColor4f(1.0, 1.0, 1.0, 0.5);
       glVertex3f(me->d_mainWinMinX_nm, me->d_mainWinMinY_nm, 0);
       glVertex3f(me->d_mainWinMaxX_nm, me->d_mainWinMinY_nm, 0);
       glVertex3f(me->d_mainWinMaxX_nm, me->d_mainWinMaxY_nm, 0);
       glVertex3f(me->d_mainWinMinX_nm, me->d_mainWinMaxY_nm, 0);
       glEnd();
     }
     i++;
  }

  glPopAttrib();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glMatrixMode(GL_TEXTURE);
  glPopMatrix();

  return 0;
}


int PatternEditor::navWinEventHandler(
                   const ImageViewerWindowEvent &event, void *ud)
{
    PatternEditor *me = (PatternEditor *)ud;
    double x = event.mouse_x, y = event.mouse_y;

    switch(event.type) {
      case RESIZE_EVENT:
         break;
      case MOTION_EVENT:
         if (event.state & IV_LEFT_BUTTON_MASK ||
             event.state & IV_RIGHT_BUTTON_MASK) {
             me->d_viewer->toImage(event.winID, &x, &y);
             me->navWinPositionToWorld(x, y,
                 me->d_dragEndX_nm, me->d_dragEndY_nm);
             me->d_viewer->dirtyWindow(event.winID);
             printf("got motion event\n");
         }
         break;
      case BUTTON_PRESS_EVENT:
         switch(event.button) {
           case IV_LEFT_BUTTON:
             // start dragging a rectangle
             me->d_settingRegion = VRPN_TRUE;
             me->d_viewer->toImage(event.winID, &x, &y);
             me->navWinPositionToWorld(x, y, x, y);
             me->d_dragStartX_nm = x;
             me->d_dragStartY_nm = y;
             me->d_dragEndX_nm = me->d_dragStartX_nm;
             me->d_dragEndY_nm = me->d_dragStartY_nm;
             me->d_viewer->dirtyWindow(event.winID);
             break;
           case IV_RIGHT_BUTTON:
             me->d_viewer->toImage(event.winID, &x, &y);
             me->navWinPositionToWorld(x, y, x, y);
             if (x > me->d_mainWinMinX_nm && x < me->d_mainWinMaxX_nm &&
                 y > me->d_mainWinMinY_nm && y < me->d_mainWinMaxY_nm) {
                // start translating
                me->d_settingTranslation = VRPN_TRUE;
                me->d_dragStartX_nm = x;
                me->d_dragStartY_nm = y;
                me->d_dragEndX_nm = me->d_dragStartX_nm;
                me->d_dragEndY_nm = me->d_dragStartY_nm;
                me->d_viewer->dirtyWindow(event.winID);
             }
             break;
           default:
             break;
         }
         break;
      case BUTTON_RELEASE_EVENT:
         switch(event.button) {
           case IV_LEFT_BUTTON:
             // copy the dragged rectangle into the main displayed rectangle
             me->d_settingRegion = VRPN_FALSE;
             me->d_viewer->toImage(event.winID, &x, &y);
             me->navWinPositionToWorld(x, y, 
                 me->d_dragEndX_nm, me->d_dragEndY_nm);
             me->d_mainWinMaxX_nm = max(me->d_dragStartX_nm,
                                    me->d_dragEndX_nm);
             me->d_mainWinMinX_nm = min(me->d_dragStartX_nm,
                                    me->d_dragEndX_nm);
             me->d_mainWinMaxY_nm = max(me->d_dragStartY_nm,
                                    me->d_dragEndY_nm);
             me->d_mainWinMinY_nm = min(me->d_dragStartY_nm,
                                    me->d_dragEndY_nm);
             me->d_viewer->dirtyWindow(event.winID);
             me->d_viewer->dirtyWindow(me->d_mainWinID);
             break;
           case IV_RIGHT_BUTTON:
             if (me->d_settingTranslation) {
               // copy the translated rectangle into the main 
               // displayed rectangle
               me->d_settingTranslation = VRPN_FALSE;
               me->d_viewer->toImage(event.winID, &x, &y);
               me->navWinPositionToWorld(x, y,
                                       me->d_dragEndX_nm, me->d_dragEndY_nm);
               double t_x = me->d_dragEndX_nm - me->d_dragStartX_nm;
               double t_y = me->d_dragEndY_nm - me->d_dragStartY_nm;
               me->d_mainWinMaxX_nm += t_x;
               me->d_mainWinMinX_nm += t_x;
               me->d_mainWinMaxY_nm += t_y;
               me->d_mainWinMinY_nm += t_y;
               me->d_viewer->dirtyWindow(event.winID);
               me->d_viewer->dirtyWindow(me->d_mainWinID);
             }
             break;
           default:
             break;
         }
         break;
      case KEY_PRESS_EVENT:
         me->d_viewer->dirtyWindow(event.winID);
         break;
      default:
         break;
    }
    return 0;
}

int PatternEditor::navWinDisplayHandler(
                   const ImageViewerDisplayData &data, void *ud)
{
  PatternEditor *me = (PatternEditor *)ud;

  glPushAttrib(GL_ENABLE_BIT | GL_TEXTURE_BIT | GL_VIEWPORT_BIT);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glMatrixMode(GL_TEXTURE);
  glPushMatrix();

  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // setup viewing/model projection
  glViewport(0, 0, data.winWidth, data.winHeight);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glOrtho(me->d_worldMinX_nm, me->d_worldMaxX_nm,
          me->d_worldMinY_nm, me->d_worldMaxY_nm, -1, 1);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // setup texture transformation
  glMatrixMode(GL_TEXTURE);
  glLoadIdentity();

  if (me->d_settingRegion) {
    // draw the current tentative setting
    glBegin(GL_LINE_LOOP);
    glLineWidth(1);
    glColor3f(1.0, 1.0, 0.0);
    glVertex3f(me->d_dragStartX_nm, me->d_dragStartY_nm, 0);
    glVertex3f(me->d_dragEndX_nm, me->d_dragStartY_nm, 0);
    glVertex3f(me->d_dragEndX_nm, me->d_dragEndY_nm, 0);
    glVertex3f(me->d_dragStartX_nm, me->d_dragEndY_nm, 0);
    glEnd();
  } else if (me->d_settingTranslation) {
    // draw the current tentative setting
    glBegin(GL_LINE_LOOP);
    glLineWidth(1);
    glColor3f(1.0, 1.0, 0.0);
    double t_x = me->d_dragEndX_nm - me->d_dragStartX_nm;
    double t_y = me->d_dragEndY_nm - me->d_dragStartY_nm;
    glVertex3f(me->d_mainWinMinX_nm + t_x, me->d_mainWinMinY_nm + t_y, 0);
    glVertex3f(me->d_mainWinMaxX_nm + t_x, me->d_mainWinMinY_nm + t_y, 0);
    glVertex3f(me->d_mainWinMaxX_nm + t_x, me->d_mainWinMaxY_nm + t_y, 0);
    glVertex3f(me->d_mainWinMinX_nm + t_x, me->d_mainWinMaxY_nm + t_y, 0);
    glEnd();
  } else {
    // draw the area covered by the main window
    glBegin(GL_LINE_LOOP);
    glLineWidth(1);
    glColor3f(0.0, 1.0, 0.0);
    glVertex3f(me->d_mainWinMinX_nm, me->d_mainWinMinY_nm, 0);
    glVertex3f(me->d_mainWinMaxX_nm, me->d_mainWinMinY_nm, 0);
    glVertex3f(me->d_mainWinMaxX_nm, me->d_mainWinMaxY_nm, 0);
    glVertex3f(me->d_mainWinMinX_nm, me->d_mainWinMaxY_nm, 0);
    glEnd();
  }


  glPopAttrib();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glMatrixMode(GL_TEXTURE);
  glPopMatrix();

  return 0;
}

void PatternEditor::navWinPositionToWorld(double x, double y,
                                       double &x_nm, double &y_nm)
{
  x_nm = d_worldMinX_nm + x*(d_worldMaxX_nm - d_worldMinX_nm);
  y_nm = d_worldMinY_nm + (1.0-y)*(d_worldMaxY_nm - d_worldMinY_nm);
}

void PatternEditor::mainWinPositionToWorld(double x, double y,
                                       double &x_nm, double &y_nm)
{
  x_nm = d_mainWinMinX_nm + x*(d_mainWinMaxX_nm - d_mainWinMinX_nm);
  y_nm = d_mainWinMinY_nm + (1.0-y)*(d_mainWinMaxY_nm - d_mainWinMinY_nm);
}

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
   
   d_userMode = IDLE;

   d_navDragStartX_nm = d_worldMinX_nm;
   d_navDragStartY_nm = d_worldMinY_nm;
   d_navDragEndX_nm = d_worldMaxX_nm;
   d_navDragEndY_nm = d_worldMaxY_nm;
   
}

PatternEditor::~PatternEditor()
{
   d_viewer->destroyWindow(d_navWinID);
   d_viewer->destroyWindow(d_mainWinID);
}

void PatternEditor::addImage(nmb_Image *im, double opacity,
      double r, double g, double b)
{
   nmb_ImageBounds ib;
   im->getBounds(ib);
   nmb_ImageBounds::ImageBoundPoint points[4] = 
     {nmb_ImageBounds::MIN_X_MIN_Y, nmb_ImageBounds::MIN_X_MAX_Y,
      nmb_ImageBounds::MAX_X_MIN_Y, nmb_ImageBounds::MAX_X_MAX_Y};
   
   ImageElement ie(im, r,g,b,opacity);
   d_images.insert(d_images.begin(), ie);

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
   // this depends on the fact that we defined the equality 
   // operator to return true if the images are equal
   ImageElement ie(im);
   d_images.remove(ie);
}

void PatternEditor::setImageEnable(nmb_Image *im, vrpn_bool displayEnable)
{
  list<ImageElement>::iterator imIter;
  for (imIter = d_images.begin();
       imIter != d_images.end(); imIter++)
  {
     if ((*imIter).d_image == im) {
          (*imIter).d_enabled = displayEnable;
     }
  }
  d_viewer->dirtyWindow(d_mainWinID);
}

void PatternEditor::show() 
{
   d_viewer->showWindow(d_mainWinID);
   d_viewer->showWindow(d_navWinID);
}

void PatternEditor::newPosition(nmb_Image *im)
{
  // really this is only necessary if the image is currently displayed
  d_viewer->dirtyWindow(d_mainWinID);
}

int PatternEditor::mainWinEventHandler(
                   const ImageViewerWindowEvent &event, void *ud)
{
    PatternEditor *me = (PatternEditor *)ud;

    double x = event.mouse_x, y = event.mouse_y;
    double centerX_nm, centerY_nm;

    switch(event.type) {
      case RESIZE_EVENT:
         me->d_mainWinWidth = event.width;
         me->d_mainWinHeight = event.height;
         break;
      case MOTION_EVENT:
         if (event.state & IV_LEFT_BUTTON_MASK) {
             // adjust current line being drawn
         } else if (event.state & IV_RIGHT_BUTTON_MASK) {
             // move the currently grabbed object if there is one
         }
         break;
      case BUTTON_PRESS_EVENT:
         switch(event.button) {
           case IV_LEFT_BUTTON:
             // start dragging a line
             break;
           case IV_RIGHT_BUTTON:
             // see if we are near something and if so then select it
             // and grab it, otherwise deselect the currently selected object

             me->d_nearDistX_nm = me->d_nearDistX_pix*
                (me->d_mainWinMaxX_nm-me->d_mainWinMinX_nm)/me->d_mainWinWidth;

             break;
           default:
             break;
         }
         break;
      case BUTTON_RELEASE_EVENT:
         switch(event.button) {
           case IV_LEFT_BUTTON:
             // set the end point
             break;
           case IV_RIGHT_BUTTON:
             // release the currently grabbed object
             break;
           default:
             break;
         }
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


  GLfloat border_color[] = {0.0, 0.0, 0.0, 0.0};
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color);
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

  if (me->d_userMode == SET_REGION || me->d_userMode == SET_TRANSLATE) {
    glOrtho(me->d_mainWinMinXadjust_nm, me->d_mainWinMaxXadjust_nm,
            me->d_mainWinMaxYadjust_nm, me->d_mainWinMinYadjust_nm, -1, 1);
  } else {
    glOrtho(me->d_mainWinMinX_nm, me->d_mainWinMaxX_nm, 
            me->d_mainWinMaxY_nm, me->d_mainWinMinY_nm, -1, 1);
  }

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // setup texture transformation
  glMatrixMode(GL_TEXTURE);
  glLoadIdentity();

  // for each image, load the texture matrix, setup automatic texture-mapping
  // of that image, draw a simple polygon on which to texture the image
  list<ImageElement>::iterator currImage;
  int i = 0;
  for (currImage = me->d_images.begin(); 
       currImage != me->d_images.end(); currImage++)
  {
     if ((*currImage).d_enabled) {
          me->drawImage(*currImage);
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


void PatternEditor::drawImage(const ImageElement &ie)
{

     double worldToImage[16];
     void *texture;
     int texwidth, texheight;

     vrpn_bool textureOkay = VRPN_TRUE;
     texture = ie.d_image->pixelData();
     texwidth = ie.d_image->width() + 2*ie.d_image->border();
     texheight = ie.d_image->height() + 2*ie.d_image->border();
     int pixType;
     switch (ie.d_image->pixelType()) {
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
/*
       printf("Loading texture %s with type %d\n",
              ie.d_image->name()->Characters(), ie.d_image->pixelType());
*/
       gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, texwidth, texheight, 
              GL_LUMINANCE,
              pixType, texture);

       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
              GL_LINEAR_MIPMAP_LINEAR);
       ie.d_image->getWorldToImageTransform(worldToImage);
       nmb_ImageBounds ib;
       ie.d_image->getBounds(ib);
/*
       for (int j = 0; j < 4; j++){
           printf("%g %g %g %g\n", worldToImage[j*4], worldToImage[j*4+1],
                                   worldToImage[j*4+2], worldToImage[j*4+3]);
       }
       printf("bounds : (%g,%g) -> (%g,%g)\n",
            ib.getX(nmb_ImageBounds::MIN_X_MIN_Y),
            ib.getY(nmb_ImageBounds::MIN_X_MIN_Y),
            ib.getX(nmb_ImageBounds::MAX_X_MAX_Y),
            ib.getY(nmb_ImageBounds::MAX_X_MAX_Y));
*/
       float scaleFactorX = (float)(ie.d_image->width())/(float)texwidth;
       float scaleFactorY = (float)(ie.d_image->height())/(float)texheight;
       // in texture coordinates
       float bordSizeX = (float)(ie.d_image->border())/(float)texwidth;
       float bordSizeY = (float)(ie.d_image->border())/(float)texheight;
       glLoadIdentity();
       // compensation for the border:
       glTranslatef(bordSizeX, bordSizeY, 0.0);
       glScalef(scaleFactorX, scaleFactorY, 1.0);
       // now we can use the xform defined for the actual image part of the
       // texture
       glMultMatrixd(worldToImage);
       glBegin(GL_POLYGON);
       glNormal3f(0.0, 0.0, 1.0);
       glColor4f(1.0, 1.0, 1.0, 0.5);
       if (d_userMode == SET_REGION || d_userMode == SET_TRANSLATE) {
         glVertex3f(d_mainWinMinXadjust_nm, d_mainWinMinYadjust_nm, 0);
         glVertex3f(d_mainWinMaxXadjust_nm, d_mainWinMinYadjust_nm, 0);
         glVertex3f(d_mainWinMaxXadjust_nm, d_mainWinMaxYadjust_nm, 0);
         glVertex3f(d_mainWinMinXadjust_nm, d_mainWinMaxYadjust_nm, 0);
       } else {
         glVertex3f(d_mainWinMinX_nm, d_mainWinMinY_nm, 0);
         glVertex3f(d_mainWinMaxX_nm, d_mainWinMinY_nm, 0);
         glVertex3f(d_mainWinMaxX_nm, d_mainWinMaxY_nm, 0);
         glVertex3f(d_mainWinMinX_nm, d_mainWinMaxY_nm, 0);
       }
       glEnd();
     }
}


int PatternEditor::navWinEventHandler(
                   const ImageViewerWindowEvent &event, void *ud)
{
    PatternEditor *me = (PatternEditor *)ud;
    double x = event.mouse_x, y = event.mouse_y;

    switch(event.type) {
      case RESIZE_EVENT:
         me->d_navWinWidth = event.width;
         me->d_navWinHeight = event.height;
         break;
      case MOTION_EVENT:
         if (event.state & IV_LEFT_BUTTON_MASK ||
             event.state & IV_RIGHT_BUTTON_MASK) {
             me->d_viewer->toImage(event.winID, &x, &y);
             me->navWinPositionToWorld(x, y,
                 me->d_navDragEndX_nm, me->d_navDragEndY_nm);
             me->d_viewer->dirtyWindow(event.winID);
//             printf("got motion event\n");
         }
         if (event.state & IV_LEFT_BUTTON_MASK) {
             me->d_mainWinMaxXadjust_nm = max(me->d_navDragStartX_nm,
                                    me->d_navDragEndX_nm);
             me->d_mainWinMinXadjust_nm = min(me->d_navDragStartX_nm,
                                    me->d_navDragEndX_nm);
             me->d_mainWinMaxYadjust_nm = max(me->d_navDragStartY_nm,
                                    me->d_navDragEndY_nm);
             me->d_mainWinMinYadjust_nm = min(me->d_navDragStartY_nm,
                                    me->d_navDragEndY_nm);
             me->d_viewer->dirtyWindow(me->d_mainWinID);
         } else if (event.state & IV_RIGHT_BUTTON_MASK) {
             double t_x = me->d_navDragEndX_nm - me->d_navDragStartX_nm;
             double t_y = me->d_navDragEndY_nm - me->d_navDragStartY_nm;
             me->d_mainWinMaxXadjust_nm = me->d_mainWinMaxX_nm;
             me->d_mainWinMinXadjust_nm = me->d_mainWinMinX_nm;
             me->d_mainWinMaxYadjust_nm = me->d_mainWinMaxY_nm;
             me->d_mainWinMinYadjust_nm = me->d_mainWinMinY_nm;
             me->d_mainWinMaxXadjust_nm += t_x;
             me->d_mainWinMinXadjust_nm += t_x;
             me->d_mainWinMaxYadjust_nm += t_y;
             me->d_mainWinMinYadjust_nm += t_y;
             me->d_viewer->dirtyWindow(me->d_mainWinID);
         }
         break;
      case BUTTON_PRESS_EVENT:
         switch(event.button) {
           case IV_LEFT_BUTTON:
             // start dragging a rectangle
             me->d_userMode = SET_REGION;
             me->d_viewer->toImage(event.winID, &x, &y);
             me->navWinPositionToWorld(x, y, x, y);
             me->d_navDragStartX_nm = x;
             me->d_navDragStartY_nm = y;
             me->d_navDragEndX_nm = me->d_navDragStartX_nm;
             me->d_navDragEndY_nm = me->d_navDragStartY_nm;
             me->d_viewer->dirtyWindow(event.winID);
             break;
           case IV_RIGHT_BUTTON:
             me->d_viewer->toImage(event.winID, &x, &y);
             me->navWinPositionToWorld(x, y, x, y);
             if (x > me->d_mainWinMinX_nm && x < me->d_mainWinMaxX_nm &&
                 y > me->d_mainWinMinY_nm && y < me->d_mainWinMaxY_nm) {
                // start translating
                me->d_userMode = SET_TRANSLATE;
                me->d_navDragStartX_nm = x;
                me->d_navDragStartY_nm = y;
                me->d_navDragEndX_nm = me->d_navDragStartX_nm;
                me->d_navDragEndY_nm = me->d_navDragStartY_nm;
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
             if (me->d_userMode == SET_REGION){
               me->d_userMode = IDLE;
               me->d_viewer->toImage(event.winID, &x, &y);
               me->navWinPositionToWorld(x, y, 
                 me->d_navDragEndX_nm, me->d_navDragEndY_nm);
               me->d_mainWinMaxX_nm = max(me->d_navDragStartX_nm,
                                    me->d_navDragEndX_nm);
               me->d_mainWinMinX_nm = min(me->d_navDragStartX_nm,
                                    me->d_navDragEndX_nm);
               me->d_mainWinMaxY_nm = max(me->d_navDragStartY_nm,
                                    me->d_navDragEndY_nm);
               me->d_mainWinMinY_nm = min(me->d_navDragStartY_nm,
                                    me->d_navDragEndY_nm);
               me->d_viewer->dirtyWindow(event.winID);
               me->d_viewer->dirtyWindow(me->d_mainWinID);
             }
             break;
           case IV_RIGHT_BUTTON:
             if (me->d_userMode == SET_TRANSLATE) {
               // copy the translated rectangle into the main 
               // displayed rectangle
               me->d_userMode = IDLE;
               me->d_viewer->toImage(event.winID, &x, &y);
               me->navWinPositionToWorld(x, y,
                                       me->d_navDragEndX_nm, me->d_navDragEndY_nm);
               double t_x = me->d_navDragEndX_nm - me->d_navDragStartX_nm;
               double t_y = me->d_navDragEndY_nm - me->d_navDragStartY_nm;
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
          me->d_worldMaxY_nm, me->d_worldMinY_nm, -1, 1);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // setup texture transformation
  glMatrixMode(GL_TEXTURE);
  glLoadIdentity();

  double t_x = 0.0, t_y = 0.0;
  switch (me->d_userMode) {
   case (SET_REGION):
    // draw the current tentative setting
    glBegin(GL_LINE_LOOP);
    glLineWidth(1);
    glColor3f(1.0, 1.0, 0.0);
    glVertex3f(me->d_navDragStartX_nm, me->d_navDragStartY_nm, 0);
    glVertex3f(me->d_navDragEndX_nm, me->d_navDragStartY_nm, 0);
    glVertex3f(me->d_navDragEndX_nm, me->d_navDragEndY_nm, 0);
    glVertex3f(me->d_navDragStartX_nm, me->d_navDragEndY_nm, 0);
    glEnd();
    break;
   case (SET_TRANSLATE):
    // draw the current tentative setting
    glBegin(GL_LINE_LOOP);
    glLineWidth(1);
    glColor3f(1.0, 1.0, 0.0);
    t_x = me->d_navDragEndX_nm - me->d_navDragStartX_nm;
    t_y = me->d_navDragEndY_nm - me->d_navDragStartY_nm;
    glVertex3f(me->d_mainWinMinX_nm + t_x, me->d_mainWinMinY_nm + t_y, 0);
    glVertex3f(me->d_mainWinMaxX_nm + t_x, me->d_mainWinMinY_nm + t_y, 0);
    glVertex3f(me->d_mainWinMaxX_nm + t_x, me->d_mainWinMaxY_nm + t_y, 0);
    glVertex3f(me->d_mainWinMinX_nm + t_x, me->d_mainWinMaxY_nm + t_y, 0);
    glEnd();
    break;
  default:
    // draw the area covered by the main window
    glBegin(GL_LINE_LOOP);
    glLineWidth(1);
    glColor3f(0.0, 1.0, 0.0);
    glVertex3f(me->d_mainWinMinX_nm, me->d_mainWinMinY_nm, 0);
    glVertex3f(me->d_mainWinMaxX_nm, me->d_mainWinMinY_nm, 0);
    glVertex3f(me->d_mainWinMaxX_nm, me->d_mainWinMaxY_nm, 0);
    glVertex3f(me->d_mainWinMinX_nm, me->d_mainWinMaxY_nm, 0);
    glEnd();
    break;
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
  y_nm = d_worldMaxY_nm + (1.0-y)*(d_worldMinY_nm - d_worldMaxY_nm);
}

void PatternEditor::mainWinPositionToWorld(double x, double y,
                                       double &x_nm, double &y_nm)
{
  x_nm = d_mainWinMinX_nm + x*(d_mainWinMaxX_nm - d_mainWinMinX_nm);
  y_nm = d_mainWinMaxY_nm + (1.0-y)*(d_mainWinMinY_nm - d_mainWinMaxY_nm);
}

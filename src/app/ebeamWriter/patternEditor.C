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

   // 20 microns should be a reasonable size - maybe should make this adjustable
   // since it affects the ease of use of the navigator window
   d_worldMinX_nm = 0;
   d_worldMinY_nm = 0;
   d_worldMaxX_nm = 0;
   d_worldMaxY_nm = 0;

   d_mainWinMinX_nm = 0;
   d_mainWinMinY_nm = 0;
   d_mainWinMaxX_nm = 0;
   d_mainWinMaxY_nm = 0;
 
   d_mainWinMinXadjust_nm = d_mainWinMinX_nm;
   d_mainWinMinYadjust_nm = d_mainWinMinY_nm;
   d_mainWinMaxXadjust_nm = d_mainWinMaxX_nm;
   d_mainWinMaxYadjust_nm = d_mainWinMaxY_nm;
  
   d_mainWinWidth = 0;
   d_mainWinHeight = 0;
   d_navWinWidth = 0;
   d_navWinHeight = 0;

   d_nearDistX_pix = 10;
   d_nearDistY_pix = 10;
   d_nearDistX_nm = 100;
   d_nearDistY_nm = 100;

   d_drawingTool = PE_POLYLINE;
   d_userMode = PE_IDLE;
   d_lineWidth_nm = 0.0;
   d_exposure_uCoulombs_per_square_cm = 0.0;

   d_displaySingleImage = vrpn_FALSE;
   d_currentSingleDisplayImage = NULL;

   d_navDragStartX_nm = d_worldMinX_nm;
   d_navDragStartY_nm = d_worldMinY_nm;
   d_navDragEndX_nm = d_worldMaxX_nm;
   d_navDragEndY_nm = d_worldMaxY_nm;
 
   d_mainDragStartX_nm = 0.0;
   d_mainDragStartY_nm = 0.0;
   d_mainDragEndX_nm = 0.0;
   d_mainDragEndY_nm = 0.0;

   d_grabOffsetX = 0.0;
   d_grabOffsetY = 0.0;

   d_shapeInProgress = vrpn_FALSE;
   d_pointInProgress = vrpn_FALSE;
   d_currShape = NULL;

   d_grabInProgress = vrpn_FALSE;
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
  double newArea = im->areaInWorld();

  list<ImageElement>::iterator insertPnt;
  if (d_images.empty()) {
    insertPnt = d_images.begin();
  } else {
    list<ImageElement>::iterator testElt = d_images.begin();
    while (testElt != d_images.end()) {
      if ((*testElt).d_image->areaInWorld() > newArea) break;
      testElt++;
    }
    insertPnt = testElt;
  }

   d_images.insert(insertPnt, ie);

   for (int i = 0; i < 4; i++) {
       d_worldMinX_nm = min(d_worldMinX_nm, ib.getX(points[i]));
       d_worldMaxX_nm = max(d_worldMaxX_nm, ib.getX(points[i]));
       d_worldMinY_nm = min(d_worldMinY_nm, ib.getY(points[i]));
       d_worldMaxY_nm = max(d_worldMaxY_nm, ib.getY(points[i]));
   }
/*
   d_mainWinMinX_nm = d_worldMinX_nm;
   d_mainWinMinY_nm = d_worldMinY_nm;
   d_mainWinMaxX_nm = d_worldMaxX_nm;
   d_mainWinMaxY_nm = d_worldMaxY_nm;
*/
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

void PatternEditor::setImageOpacity(nmb_Image *im, double opacity)
{
  list<ImageElement>::iterator imIter;
  for (imIter = d_images.begin();
       imIter != d_images.end(); imIter++)
  {
     if ((*imIter).d_image == im) {
          (*imIter).d_opacity = opacity;
     }
  }
  d_viewer->dirtyWindow(d_mainWinID);
}

void PatternEditor::setImageColor(nmb_Image *im, double r, double g, double b)
{
  list<ImageElement>::iterator imIter;
  for (imIter = d_images.begin();
       imIter != d_images.end(); imIter++)
  {
     if ((*imIter).d_image == im) {
          (*imIter).d_red = r;
          (*imIter).d_green = g;
          (*imIter).d_blue = b;
     }
  }
  d_viewer->dirtyWindow(d_mainWinID);
}

ImageElement *PatternEditor::getImageParameters(nmb_Image *im)
{
  list<ImageElement>::iterator imIter;
  for (imIter = d_images.begin();
       imIter != d_images.end(); imIter++)
  {
     if ((*imIter).d_image == im) {
       return &(*imIter); // Warning: assert(imIter != &(*imIter))
     }
  }
  return NULL;
}

void PatternEditor::showSingleImage(nmb_Image *im)
{
  if (im == NULL) {
    d_displaySingleImage = vrpn_FALSE;
  } else {
    list<ImageElement>::iterator imIter;
    for (imIter = d_images.begin();
       imIter != d_images.end(); imIter++)
    {
       if ((*imIter).d_image == im) {
          d_displaySingleImage = vrpn_TRUE;
          d_currentSingleDisplayImage = im;
       }
    }
  }
  d_viewer->dirtyWindow(d_mainWinID);
}

void PatternEditor::show() 
{
   d_viewer->showWindow(d_mainWinID);
   d_viewer->showWindow(d_navWinID);
}

void PatternEditor::newPosition(nmb_Image * im)
{
  
  // really this is only necessary if the image is currently displayed
  d_images.sort();
  d_viewer->dirtyWindow(d_mainWinID);
}

void PatternEditor::setDrawingParameters(double lineWidth_nm, double exposure)
{
    d_lineWidth_nm = lineWidth_nm;
    d_exposure_uCoulombs_per_square_cm = exposure;
    if (d_currShape) {
      d_currShape->d_lineWidth_nm = lineWidth_nm;
      d_currShape->d_exposure_uCoulombs_per_square_cm = exposure;
      d_viewer->dirtyWindow(d_mainWinID);
    }
}

void PatternEditor::setDrawingTool(PE_DrawTool tool)
{
    d_drawingTool = tool;
}

void PatternEditor::clearShape()
{
  if (d_shapeInProgress){
     clearDrawingState();
     d_userMode = PE_IDLE;
     d_viewer->dirtyWindow(d_mainWinID);
  }
  else if (!d_pattern.empty()) {
    d_pattern.pop_back();
    d_viewer->dirtyWindow(d_mainWinID);
  }
  return;
}

void PatternEditor::addDumpPoint(const double x, const double y)
{
  d_dumpPoints.push_back(PatternPoint(x, y));
}

void PatternEditor::updateDumpPoint(double const x, double const y)
{
  if (!d_dumpPoints.empty()) {
    d_dumpPoints.pop_back();
    d_dumpPoints.push_back(PatternPoint(x, y));
  } else {
    fprintf(stderr, "Error, can't update dump point: none in list\n");
  }
  return;
}

int PatternEditor::findNearestShapePoint(double x, double y, 
                    list<PatternShape>::iterator &nearestShape,
                    list<PatternPoint>::iterator &nearestPoint,
                    double &minDist)
{
  // search all shape and dump points for the closest one
  // and return it
  if (d_pattern.empty()) return -1;
 
  list<PatternShape>::iterator currShape;
  list<PatternPoint>::iterator pointRef;
  currShape = d_pattern.begin();
  double currDist;

  nearestShape = currShape;
  findNearestPoint((*currShape).d_points, x, y, nearestPoint, minDist);
  
  currShape++;
  while (currShape != d_pattern.end()) {
    findNearestPoint((*currShape).d_points, x, y, pointRef, currDist);
    if (currDist < minDist) {
      minDist = currDist;
      nearestShape = currShape;
      nearestPoint = pointRef;
    }
    currShape++;
  }
  return 0;
}

int PatternEditor::findNearestPoint(list<PatternPoint> points, 
                                     const double x, const double y, 
                                     list<PatternPoint>::iterator &nearestPoint,
                                     double &minDist)
{
  if (points.empty()) return -1;

  list<PatternPoint>::iterator currPnt;
  currPnt = points.begin();
  double dx, dy, currDist;
  dx = x-(*currPnt).d_x;
  dy = y-(*currPnt).d_y;
  minDist = dx*dx + dy*dy;
  nearestPoint = currPnt;
  currPnt++;
  while (currPnt != points.end()) {
    dx = x-(*currPnt).d_x;
    dy = y-(*currPnt).d_y;
    currDist = dx*dx + dy*dy;
    if (currDist < minDist) {
      minDist = currDist;
      nearestPoint = currPnt;
    }
  }
  minDist = sqrt(minDist);
  return 0;
}

void PatternEditor::saveImageBuffer(const char *filename, 
                                    const ImageType filetype)
{
  glutProcessEvents_UNC();
  d_viewer->dirtyWindow(d_mainWinID);
  glutProcessEvents_UNC();
  int w = d_mainWinWidth;
  int h = d_mainWinHeight;
  unsigned char *pixels = new unsigned char [w*h*3];

  glReadBuffer(GL_FRONT);
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, pixels);
  glPixelStorei(GL_PACK_ALIGNMENT, 4);
  glReadBuffer(GL_FRONT);

  AbstractImage *ai = ImageMaker(filetype, h, w, 3, pixels, true);
  delete [] pixels;
  if (ai)
  {
    if (!ai->Write(filename))
       fprintf(stderr, "Failed to write screen to '%s'!\n", filename);
    delete ai;
  }
}

void PatternEditor::setViewport(double minX_nm, double minY_nm, 
                                double maxX_nm, double maxY_nm)
{
  d_mainWinMinX_nm = minX_nm;
  d_mainWinMinY_nm = minY_nm;
  d_mainWinMaxX_nm = maxX_nm;
  d_mainWinMaxY_nm = maxY_nm;
  d_worldMinX_nm = min(d_worldMinX_nm, minX_nm);
  d_worldMinY_nm = min(d_worldMinY_nm, minY_nm);
  d_worldMaxX_nm = max(d_worldMaxX_nm, maxX_nm);
  d_worldMaxY_nm = max(d_worldMaxY_nm, maxY_nm);

  d_viewer->dirtyWindow(d_mainWinID);
  d_viewer->dirtyWindow(d_navWinID);
}

list<PatternShape> PatternEditor::shapeList()
{
  return d_pattern;
}

list<PatternPoint> PatternEditor::dumpPointList()
{
  return d_dumpPoints;
}

int PatternEditor::mainWinEventHandler(
                   const ImageViewerWindowEvent &event, void *ud)
{

    PatternEditor *me = (PatternEditor *)ud;
    return me->handleMainWinEvent(event);
}

int PatternEditor::handleMainWinEvent(
                    const ImageViewerWindowEvent &event)
{
    double x = 0, y = 0;
    double centerX_nm, centerY_nm;
    double x_world_nm, y_world_nm;

    switch(event.type) {
      case RESIZE_EVENT:
         d_mainWinWidth = event.width;
         d_mainWinHeight = event.height;
         break;
      case MOTION_EVENT:
         x = event.mouse_x; y = event.mouse_y;
         //if (event.state & IV_LEFT_BUTTON_MASK) {
             // adjust current line being drawn
             if (getUserMode() == PE_DRAWMODE && 
                 (event.state & IV_LEFT_BUTTON_MASK)) {
               d_viewer->toImage(event.winID, &x, &y);
               mainWinPositionToWorld(x,y,x_world_nm, y_world_nm);
               updatePoint(x_world_nm, y_world_nm);
               d_viewer->dirtyWindow(event.winID);
             }
         //} else if (event.state & IV_RIGHT_BUTTON_MASK) {
             // move the currently grabbed object if there is one
             else if (getUserMode() == PE_GRABMODE) {
                 d_viewer->toImage(event.winID, &x, &y);
                 mainWinPositionToWorld(x,y,x_world_nm, y_world_nm);
                 updateGrab(x_world_nm, y_world_nm);
             }
         //}
         break;
      case BUTTON_PRESS_EVENT:
         x = event.mouse_x; y = event.mouse_y;
         switch(event.button) {
           case IV_LEFT_BUTTON:
             // start dragging an object
             if (getUserMode() != PE_DRAWMODE) {
               if (d_drawingTool == PE_POLYLINE) {
                 startShape(PS_POLYLINE);
               } else if (d_drawingTool == PE_POLYGON) {
                 startShape(PS_POLYGON);
               }
               d_viewer->toImage(event.winID, &x, &y);
               setUserMode(PE_DRAWMODE);
             }
             d_viewer->toImage(event.winID, &x, &y);
             mainWinPositionToWorld(x, y, x_world_nm, y_world_nm);
             startPoint(x_world_nm, y_world_nm);
             d_viewer->dirtyWindow(event.winID);
             break;
           case IV_RIGHT_BUTTON:
             // if drawing, terminate the drawing
             if (getUserMode() == PE_DRAWMODE) {
               endShape();
               setUserMode(PE_IDLE);
             }
/*
             // otherwise, look for something close by to select
             else {
               d_viewer->toImage(event.winID, &x, &y);
               mainWinPositionToWorld(x, y,
                                          x_world_nm, y_world_nm);
               if (grab(x_world_nm, y_world_nm)) {
                  setUserMode(PE_GRABMODE);
               }
             }
*/
             break;
           default:
             break;
         }
         break;
      case BUTTON_RELEASE_EVENT:
         x = event.mouse_x; y = event.mouse_y;
         switch(event.button) {
           case IV_LEFT_BUTTON:
             d_viewer->toImage(event.winID, &x, &y);
             mainWinPositionToWorld(x, y,
                     x_world_nm, y_world_nm);
             if (getUserMode() == PE_DRAWMODE) {
               // set the point
               finishPoint(x_world_nm, y_world_nm);
             }
             d_viewer->dirtyWindow(event.winID);
             break;
           case IV_RIGHT_BUTTON:
/*
             // release the currently grabbed object
             if (getUserMode() == PE_GRABMODE) {
               d_viewer->toImage(event.winID, &x, &y);
               mainWinPositionToWorld(x,y,x_world_nm, y_world_nm);
               updateGrab(x_world_nm, y_world_nm);
             }
*/
             break;
           default:
             break;
         }
         break;
      case KEY_PRESS_EVENT:
         x = event.mouse_x; y = event.mouse_y;
         switch(event.keycode) {
           case 'z':
             d_viewer->toImage(event.winID, &x, &y);
             mainWinPositionToWorld(x, y,
                 centerX_nm, centerY_nm);
             zoomBy(centerX_nm, centerY_nm, 2.0);
             d_viewer->dirtyWindow(d_navWinID);
             d_viewer->dirtyWindow(event.winID);
             break;
           case 'Z':
             d_viewer->toImage(event.winID, &x, &y);
             mainWinPositionToWorld(x, y,
                 centerX_nm, centerY_nm);
             zoomBy(centerX_nm, centerY_nm, 0.5);
             d_viewer->dirtyWindow(d_navWinID);
             d_viewer->dirtyWindow(event.winID);
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
    double desired_width, desired_height;
    double clamped_width, clamped_height;
    double xmin,xmax,ymin,ymax;

    if (magFactor == 0) {
       fprintf(stderr, "PatternEditor::zoomBy: Error, can't zoom by 0\n");
       return;
    }

    desired_width = (d_mainWinMaxX_nm - d_mainWinMinX_nm)/magFactor;
    desired_height = (d_mainWinMaxY_nm - d_mainWinMinY_nm)/magFactor;

    double worldWidth = d_worldMaxX_nm - d_worldMinX_nm;
    double worldHeight = d_worldMaxY_nm - d_worldMinY_nm;

    if (desired_width > worldWidth || 
        desired_height > worldHeight) {
       // then we must clamp
       if (desired_width/worldWidth > desired_height/worldHeight) {
         clamped_height = desired_height*worldWidth/desired_width;
         clamped_width = worldWidth;
       } else {
         clamped_height = worldHeight;
         clamped_width = desired_width*worldHeight/desired_height;
       }
       xmin = centerX_nm - 0.5*clamped_width;
       xmax = centerX_nm + 0.5*clamped_width;
       ymin = centerY_nm - 0.5*clamped_height;
       ymax = centerY_nm + 0.5*clamped_height;
    } else {
       xmin = centerX_nm - 0.5*desired_width;
       xmax = centerX_nm + 0.5*desired_width;
       ymin = centerY_nm - 0.5*desired_height;
       ymax = centerY_nm + 0.5*desired_height;
    }

    // now we can fix everything by shifting: 
    double x_shift = 0.0, y_shift = 0.0;

    if (xmin < d_worldMinX_nm) {
       x_shift = d_worldMinX_nm - xmin;
       xmin += x_shift;
       xmax += x_shift;
    }
    if (ymin < d_worldMinY_nm) {
       y_shift = d_worldMinY_nm - ymin;
       ymin += y_shift;
       ymax += y_shift;
    }
    if (xmax > d_worldMaxX_nm) {
       x_shift = d_worldMaxX_nm - xmax;
       xmin += x_shift;
       xmax += x_shift;
    }
    if (ymax > d_worldMaxY_nm) {
       y_shift = d_worldMaxY_nm - ymax;
       ymin += y_shift;
       ymax += y_shift;
    }

    d_mainWinMinX_nm = xmin;
    d_mainWinMaxX_nm = xmax;
    d_mainWinMinY_nm = ymin;
    d_mainWinMaxY_nm = ymax;

    double temp;
    if ((d_mainWinMinX_nm > d_mainWinMaxX_nm) || 
        (d_mainWinMinY_nm > d_mainWinMaxY_nm)) {
       printf("zoomBy: min/max swapped: this shouldn't happen\n");
       if (d_mainWinMinX_nm > d_mainWinMaxX_nm) {
           temp = d_mainWinMinX_nm;
           d_mainWinMinX_nm = d_mainWinMaxX_nm;
           d_mainWinMaxX_nm = temp;
       }
       if (d_mainWinMinY_nm > d_mainWinMaxY_nm) {
           temp = d_mainWinMinY_nm;
           d_mainWinMinY_nm = d_mainWinMaxY_nm;
           d_mainWinMaxY_nm = temp;
       }
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

  glPolygonMode(GL_FRONT, GL_FILL);

  GLfloat border_color[] = {0.0, 0.0, 0.0, 0.0};
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  glPixelStorei(GL_UNPACK_ALIGNMENT, 4);


  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  GLfloat textureColor[] = {0.5, 0.5, 0.5, 0.5};
  glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, textureColor);

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

  PE_UserMode currMode = me->getUserMode();
  if (currMode == PE_SET_REGION || currMode == PE_SET_TRANSLATE) {
    glOrtho(me->d_mainWinMaxXadjust_nm, me->d_mainWinMinXadjust_nm,
            me->d_mainWinMinYadjust_nm, me->d_mainWinMaxYadjust_nm, -1, 1);
  } else {
    glOrtho(me->d_mainWinMaxX_nm, me->d_mainWinMinX_nm, 
            me->d_mainWinMinY_nm, me->d_mainWinMaxY_nm, -1, 1);
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
     if (me->d_displaySingleImage) {
        if (((*currImage).d_image == me->d_currentSingleDisplayImage) && 
            (*currImage).d_enabled) {
           me->drawImage(*currImage);
        }
     } else if ((*currImage).d_enabled) {
          me->drawImage(*currImage);
     }
     i++;
  }

  glDisable(GL_TEXTURE_2D);
  me->drawPattern();

  me->drawScale();

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
     texwidth = ie.d_image->width() + 
                ie.d_image->borderXMin()+ie.d_image->borderXMax();
     texheight = ie.d_image->height() +
                ie.d_image->borderYMin()+ie.d_image->borderYMax();
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

/*       printf("Loading texture %s with type %d\n",
              ie.d_image->name()->Characters(), ie.d_image->pixelType());
       printf("width=%d,height=%d,border = (%d,%d)(%d,%d)\n", 
         texwidth, texheight, ie.d_image->borderXMin(),
         ie.d_image->borderXMax(), ie.d_image->borderYMin(), 
         ie.d_image->borderYMax());
*/
       glPixelTransferf(GL_RED_SCALE, (float)ie.d_red);
                                       // *(float)ie.d_opacity);
       glPixelTransferf(GL_GREEN_SCALE, (float)ie.d_green);
                                       // *(float)ie.d_opacity);
       glPixelTransferf(GL_BLUE_SCALE, (float)ie.d_blue);
                                       // *(float)ie.d_opacity);
       glPixelTransferf(GL_ALPHA_SCALE, (float)ie.d_opacity);

/*
       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                 GL_LINEAR_MIPMAP_LINEAR);

       gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, texwidth, texheight, 
              GL_LUMINANCE,
              pixType, texture);
*/
       glTexImage2D(GL_PROXY_TEXTURE_2D, ...
       glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 
                texwidth, texheight, 0, GL_LUMINANCE,
                pixType, texture);

       ie.d_image->getWorldToImageTransform(worldToImage);
       nmb_ImageBounds ib;
       ie.d_image->getBounds(ib);
/*
       for (int j = 0; j < 4; j++){
           printf("%g %g %g %g\n", worldToImage[j*4], worldToImage[j*4+1],
                                   worldToImage[j*4+2], worldToImage[j*4+3]);
       }
*/
/*
       printf("bounds : (%g,%g),(%g,%g),(%g,%g),(%g,%g)\n",
            ib.getX(nmb_ImageBounds::MIN_X_MIN_Y),
            ib.getY(nmb_ImageBounds::MIN_X_MIN_Y),
            ib.getX(nmb_ImageBounds::MAX_X_MIN_Y),
            ib.getY(nmb_ImageBounds::MAX_X_MIN_Y),
            ib.getX(nmb_ImageBounds::MAX_X_MAX_Y),
            ib.getY(nmb_ImageBounds::MAX_X_MAX_Y),
            ib.getX(nmb_ImageBounds::MIN_X_MAX_Y),
            ib.getY(nmb_ImageBounds::MIN_X_MAX_Y));
*/
       float scaleFactorX = (float)(ie.d_image->width())/(float)texwidth;
       float scaleFactorY = (float)(ie.d_image->height())/(float)texheight;
       // in texture coordinates
       float bordSizeX = (float)(ie.d_image->borderXMin())/(float)texwidth;
       float bordSizeY = (float)(ie.d_image->borderYMin())/(float)texheight;
       glLoadIdentity();
       // compensation for the border:
       glTranslatef(bordSizeX, bordSizeY, 0.0);
       glScalef(scaleFactorX, scaleFactorY, 1.0);
       // now we can use the xform defined for the actual image part of the
       // texture
       glMultMatrixd(worldToImage);


       glBegin(GL_POLYGON);
       glNormal3f(0.0, 0.0, 1.0);
       glColor4f(1.0, 1.0, 1.0, (float)ie.d_opacity);
       // draw a parallelogram fit to the image
       // I'm so glad I generalized the image bounds the way I did
       // since it matches so well the use of affine transformations
       glVertex2f(ib.getX(nmb_ImageBounds::MIN_X_MIN_Y),
                  ib.getY(nmb_ImageBounds::MIN_X_MIN_Y));
       glVertex2f(ib.getX(nmb_ImageBounds::MAX_X_MIN_Y),
                  ib.getY(nmb_ImageBounds::MAX_X_MIN_Y));
       glVertex2f(ib.getX(nmb_ImageBounds::MAX_X_MAX_Y),
                  ib.getY(nmb_ImageBounds::MAX_X_MAX_Y));
       glVertex2f(ib.getX(nmb_ImageBounds::MIN_X_MAX_Y),
                  ib.getY(nmb_ImageBounds::MIN_X_MAX_Y));
/*
       PE_UserMode currMode = getUserMode();
       if (currMode == PE_SET_REGION || currMode == PE_SET_TRANSLATE) {
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
*/
       glEnd();

       glPixelTransferf(GL_RED_SCALE, 1.0);
       glPixelTransferf(GL_GREEN_SCALE, 1.0);
       glPixelTransferf(GL_BLUE_SCALE, 1.0);
     }
}


void PatternEditor::drawPattern()
{
  double units_per_pixel_x, units_per_pixel_y;

  units_per_pixel_x = (d_mainWinMaxX_nm - d_mainWinMinX_nm)/
                      (double)d_mainWinWidth;
  units_per_pixel_y = (d_mainWinMaxY_nm - d_mainWinMinY_nm)/
                      (double)d_mainWinHeight;

  list<PatternShape>::iterator shapeIter;
  int numShapes = 0;
  if (d_currShape) {
    numShapes++;
    d_currShape->draw(units_per_pixel_x, units_per_pixel_y);
  }
  for (shapeIter = d_pattern.begin();
       shapeIter != d_pattern.end(); shapeIter++)
  {
     (*shapeIter).draw(units_per_pixel_x, units_per_pixel_y);
     numShapes++;
  }
//  printf("drawing %d shapes\n", numShapes);
}

void PatternEditor::drawScale()
{
  float xSpan = d_mainWinMaxX_nm - d_mainWinMinX_nm;
  float ySpan = d_mainWinMaxY_nm - d_mainWinMinY_nm;
  char str[64];
  if (xSpan == 0.0) return;
  double t = 1.0;
  double scale_length;
  // set t to the first power of 10 greater than xSpan
  while (t >= xSpan) {
    t = 0.1*t;
  }
  while (t < xSpan) {
    t = 10.0*t;
  }
  if (xSpan < 0.3*t) {
    scale_length = 0.03*t;
  } else {
    scale_length = 0.1*t;
  }

  // draw a line from d_mainWinMaxX_nm to d_mainWin
  float x_end = 0.9*d_mainWinMaxX_nm + 0.1*d_mainWinMinX_nm;
  float x_start = x_end - scale_length;
  float y_start = 0.9*d_mainWinMaxY_nm + 0.1*d_mainWinMinY_nm;
  float y_end = y_start;
  glColor4f(1.0, 1.0, 1.0, 1.0);
  glBegin(GL_LINES);
  glVertex3f(x_start, y_start, 0.0);
  glVertex3f(x_end, y_end, 0.0);
  glEnd();
  
  int length = (int)scale_length;
  if (length < 1000) {
     sprintf(str, "%d nm", length);
  } else {
     length = length/1000;
     sprintf(str, "%d um", length);
  }
  glRasterPos2d(0.8*x_start + 0.2*x_end, y_start - 0.05*ySpan);
  int i;
  for (i = 0; i < strlen(str); i++) {
    glutBitmapCharacter(GLUT_BITMAP_8_BY_13, str[i]);
  }
}

int PatternEditor::navWinEventHandler(
                   const ImageViewerWindowEvent &event, void *ud)
{
    PatternEditor *me = (PatternEditor *)ud;
    double x = 0, y = 0;

    switch(event.type) {
      case RESIZE_EVENT:
         me->d_navWinWidth = event.width;
         me->d_navWinHeight = event.height;
         break;
      case MOTION_EVENT:
	 x = event.mouse_x; y = event.mouse_y;
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
         x = event.mouse_x; y = event.mouse_y;
         switch(event.button) {
           case IV_LEFT_BUTTON:
             // start dragging a rectangle
             me->setUserMode(PE_SET_REGION);
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
                me->setUserMode(PE_SET_TRANSLATE);
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
         x = event.mouse_x; y = event.mouse_y;
         switch(event.button) {
           case IV_LEFT_BUTTON:
             // copy the dragged rectangle into the main displayed rectangle
             if (me->getUserMode() == PE_SET_REGION){
               me->setUserMode(PE_IDLE);
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
             if (me->getUserMode() == PE_SET_TRANSLATE) {
               // copy the translated rectangle into the main 
               // displayed rectangle
               me->setUserMode(PE_IDLE);
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
          me->d_worldMinY_nm, me->d_worldMaxY_nm, -1, 1);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // setup texture transformation
  glMatrixMode(GL_TEXTURE);
  glLoadIdentity();

  double t_x = 0.0, t_y = 0.0;
  switch (me->getUserMode()) {
   case (PE_SET_REGION):
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
   case (PE_SET_TRANSLATE):
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
  x_nm = d_worldMaxX_nm - x*(d_worldMaxX_nm - d_worldMinX_nm);
  y_nm = d_worldMaxY_nm - y*(d_worldMaxY_nm - d_worldMinY_nm);
}

void PatternEditor::mainWinPositionToWorld(double x, double y,
                                       double &x_nm, double &y_nm)
{
  x_nm = d_mainWinMaxX_nm - x*(d_mainWinMaxX_nm - d_mainWinMinX_nm);
  y_nm = d_mainWinMaxY_nm - y*(d_mainWinMaxY_nm - d_mainWinMinY_nm);
}

void PatternEditor::worldToMainWinPosition(const double x_nm,
                                     const double y_nm, 
                             double &x_norm, double &y_norm)
{
  double delX = d_mainWinMaxX_nm - d_mainWinMinX_nm;
  double delY = d_mainWinMaxY_nm - d_mainWinMinY_nm;
  if (delX != 0) {
    x_norm = 1.0 - (x_nm - d_mainWinMinX_nm)/delX;
  } else {
    fprintf(stderr, "Warning, x range is 0\n");
  }
  if (delY != 0) {
    y_norm = 1.0 - (y_nm - d_mainWinMinY_nm)/delY;
  } else {
    fprintf(stderr, "Warning, y range is 0\n");
  }
}

void PatternEditor::mainWinNMToPixels(const double x_nm, const double y_nm,
                                      double &x_pixels, double &y_pixels)
{
  worldToMainWinPosition(x_nm, y_nm, x_pixels, y_pixels);
  d_viewer->toPixels(d_mainWinID, &x_pixels, &y_pixels);
}

void PatternEditor::mainWinNMToPixels(const double dist_nm,
                                      double &dist_pixels)
{
  double d0 = 0, d1 = 0;
  worldToMainWinPosition(dist_nm, d0, dist_pixels, d1);
  d_viewer->toPixels(d_mainWinID, &dist_pixels, &d1);
}


PE_UserMode PatternEditor::getUserMode()
{
  return d_userMode;
}

void PatternEditor::setUserMode(PE_UserMode mode)
{
  if (mode == d_userMode) {
    printf("warning: setUserMode called with same value as current mode\n");
    return;
  }

  // clean up from the previous mode
  switch(d_userMode) {
    case PE_IDLE:
      break;
    case PE_SET_REGION:
      break;
    case PE_SET_TRANSLATE:
      break;
    case PE_DRAWMODE:
      if (d_shapeInProgress) {
        clearDrawingState();
      }
      break;
    case PE_GRABMODE:
      break;
  }

  // initialize for the next mode
  switch(mode) {
    case PE_IDLE:
      break;
    case PE_SET_REGION:
      break;
    case PE_SET_TRANSLATE:
      break;
    case PE_DRAWMODE:
      break;
    case PE_GRABMODE:
      break;
  }

  d_userMode = mode;

}

void PatternEditor::clearDrawingState()
{
  printf("clearing shape\n");
  if (d_currShape) {
    delete d_currShape;
    d_currShape = NULL;
  }
  d_shapeInProgress = vrpn_FALSE;
  d_pointInProgress = vrpn_FALSE;
}

int PatternEditor::startShape(ShapeType type)
{
  if (d_shapeInProgress) {
     printf("Error, startShape called while shape is being specified\n");
     return -1;
  }

  printf("starting shape\n");
  d_shapeInProgress = vrpn_TRUE;
  d_currShape = new PatternShape(d_lineWidth_nm, 
                                 d_exposure_uCoulombs_per_square_cm,
                                 type);
  return 0;
}

vrpn_bool PatternEditor::selectPoint(const double x_nm, const double y_nm)
{
  list<PatternShape>::iterator shapeRef;
  list<PatternPoint>::iterator pntRef;
  list<PatternPoint>::iterator dumpPntRef;

  double minDist, minDistPixels, dumpMinDist;
  double x_pnt, y_pnt;
  //double x_offset, y_offset;
  vrpn_bool selectedSomething = vrpn_FALSE;
  if (findNearestShapePoint(x_nm, y_nm, shapeRef, pntRef, minDist) &&
      findNearestPoint(d_dumpPoints, x_nm, y_nm, dumpPntRef, dumpMinDist)) {
      return vrpn_FALSE;
  }
  if (minDist > dumpMinDist) {
    mainWinNMToPixels(dumpMinDist, minDistPixels);
    if (minDistPixels < PE_SELECT_DIST) {
      d_selectedPoint = dumpPntRef;
      x_pnt = (*d_selectedPoint).d_x;
      y_pnt = (*d_selectedPoint).d_y;
      d_grabOffsetX = x_nm - x_pnt;
      d_grabOffsetY = y_nm - y_pnt;
      d_grabInProgress = vrpn_TRUE;
      selectedSomething = vrpn_TRUE;
    }
  } else {
    mainWinNMToPixels(minDist, minDistPixels);
    if (minDistPixels < PE_SELECT_DIST) {
      d_selectedShape = shapeRef;
      d_selectedPoint = pntRef;
      x_pnt = (*d_selectedPoint).d_x;
      y_pnt = (*d_selectedPoint).d_y;
      d_grabOffsetX = x_nm - x_pnt;
      d_grabOffsetY = y_nm - y_pnt;
      d_grabInProgress = vrpn_TRUE;
      selectedSomething = vrpn_TRUE;
    }
  }
  return selectedSomething;
}

int PatternEditor::updateGrab(const double x_nm, const double y_nm)
{
  (*d_selectedPoint).d_x = x_nm - d_grabOffsetX;
  (*d_selectedPoint).d_y = y_nm - d_grabOffsetY;
  return 0;
}

int PatternEditor::startPoint(const double x_nm, const double y_nm)
{
  d_pointInProgress = vrpn_TRUE;
  if (d_shapeInProgress) {
    printf("adding point to current shape\n");
    d_currShape->addPoint(x_nm, y_nm);
    return 0;
  } else if (d_drawingTool == PE_DUMP_POINT){
    addDumpPoint(x_nm, y_nm);
  } else if (d_drawingTool == PE_SELECT){
    selectPoint(x_nm, y_nm);
  } else {
    d_pointInProgress = vrpn_FALSE;
    printf("Error, startPoint called when not drawing shape\n");
    return -1;
  }
  return 0;
}

int PatternEditor::updatePoint(const double x_nm, const double y_nm)
{
  if (d_pointInProgress) {
    if (d_shapeInProgress) {
      d_currShape->removePoint();
      d_currShape->addPoint(x_nm, y_nm);
      return 0;
    } else if (d_drawingTool == PE_DUMP_POINT){
      updateDumpPoint(x_nm, y_nm);
      return 0;
    } else if (d_drawingTool == PE_SELECT && d_grabInProgress){
      updateGrab(x_nm, y_nm);
      return 0;
    }
  } else {
    printf("Error, updatePoint called when not setting point\n");
    return -1;
  }
  return 0;
}

int PatternEditor::finishPoint(const double x_nm, const double y_nm)
{
  int result = updatePoint(x_nm, y_nm);
//  printf("finishPoint\n");
  d_pointInProgress = vrpn_FALSE;
  d_grabInProgress = vrpn_FALSE;
  return result;
}

int PatternEditor::endShape()
{
  //printf("ending shape\n");
  // put the shape into the pattern and clear drawing state
  d_pattern.push_back(*d_currShape);
  clearDrawingState();
  return 0;
}

#include "patternEditor.h"
#include "GL/gl.h"

int PatternShape::s_nextID = 0;

PatternShape::PatternShape(double lw, double exp, 
                          ShapeType type): d_ID(s_nextID),
             d_lineWidth_nm(lw),
             d_exposure_uCoulombs_per_square_cm(exp),
             d_type(type), d_trans_x(0.0), d_trans_y(0.0)
{
  s_nextID++;
}

PatternShape::PatternShape(const PatternShape &sh): d_ID(s_nextID),
   d_lineWidth_nm(sh.d_lineWidth_nm),
   d_exposure_uCoulombs_per_square_cm(sh.d_exposure_uCoulombs_per_square_cm),
   d_type(sh.d_type), d_trans_x(sh.d_trans_x), d_trans_y(sh.d_trans_y)
{
  d_points = sh.d_points;
  s_nextID++;
}

void PatternShape::addPoint(double x, double y)
{
  d_points.push_back(PatternPoint(x, y));
}

void PatternShape::removePoint()
{
  if (!d_points.empty())
    d_points.pop_back();
  return;
}

void PatternShape::drawThinPolyline()
{
  if (d_points.empty()) return;

  double x, y;
  glLineWidth(1);
  glColor4f(1.0, 0.0, 0.0, 1.0);

  list<PatternPoint>::iterator pntIter = d_points.begin();
  
  x = (*pntIter).d_x;
  y = (*pntIter).d_y;
  pntIter++;
  if (pntIter == d_points.end()) {
    glBegin(GL_POINTS);
    glVertex3f(x,y,0.0);
    glEnd();
  } else {
    glBegin(GL_LINE_STRIP);
    glVertex3f(x,y,0.0);
    while (pntIter != d_points.end()) {
      x = (*pntIter).d_x;
      y = (*pntIter).d_y;
      glVertex3f(x,y,0.0);
      pntIter++;
    }
    glEnd();
  }
}

void PatternShape::drawPolygon()
{
  if (d_points.empty()) return;

  double x, y;
  glLineWidth(1);
  glColor4f(1.0, 0.0, 0.0, 1.0);

  list<PatternPoint>::iterator pntIter = d_points.begin();

  x = (*pntIter).d_x;
  y = (*pntIter).d_y;
  pntIter++;
  if (pntIter == d_points.end()) {
    glBegin(GL_POINTS);
    glVertex3f(x,y,0.0);
    glEnd();
  } else {
    glBegin(GL_LINE_LOOP);
    glVertex3f(x,y,0.0);
    while (pntIter != d_points.end()) {
      x = (*pntIter).d_x;
      y = (*pntIter).d_y;
      glVertex3f(x,y,0.0);
      pntIter++;
    }
    glEnd();
  }
}

void PatternShape::drawThickPolyline()
{
  double x_start, y_start;
  double x0, y0, x1, y1, x2, y2;
  double lenA, lenB;
  double os_x0 = 0.0, os_y0 = 0.0, os_x1 = 0.0, os_y1 = 0.0;
  double dxA, dyA, dxB, dyB, dx_avg, dy_avg, lenAvg;
  double widthCorrection = 1.0;
  double cprod;

  glLineWidth(1);
  glColor4f(1.0, 0.0, 0.0, 1.0);

  list<PatternPoint>::iterator pntIter;
  pntIter = d_points.begin();
  x2 = (*pntIter).d_x;
  y2 = (*pntIter).d_y;
  x_start = x2;
  y_start = y2;
  pntIter++;
  if (pntIter == d_points.end()) {
    // just draw one point here
    glBegin(GL_POINTS);
    glVertex3f(x2, y2, 0.0);
    glEnd();
    return;
  } else {
    x1 = x2;
    y1 = y2;
    x2 = (*pntIter).d_x;
    y2 = (*pntIter).d_y;
    if (d_lineWidth_nm > 0 && d_type == PS_POLYLINE) {
      dxB = x2-x1; dyB = y2-y1;
      lenB = sqrt(dxB*dxB + dyB*dyB);
      os_x0 = -0.5*d_lineWidth_nm*dyB/lenB;
      os_y0 = 0.5*d_lineWidth_nm*dxB/lenB;
    }
    pntIter++;
  }
  while (pntIter != d_points.end()) {
    x0 = x1;
    y0 = y1;
    x1 = x2;
    y1 = y2;
    x2 = (*pntIter).d_x;
    y2 = (*pntIter).d_y;
    if (d_lineWidth_nm > 0 && d_type == PS_POLYLINE) {
      lenA = lenB;
      dxA = dxB, dyA = dyB;
      dxB = x2-x1; dyB = y2-y1;
      lenB = sqrt(dxB*dxB + dyB*dyB);
      dx_avg = 0.5*(dxA/lenA + dxB/lenB);
      dy_avg = 0.5*(dyA/lenA + dyB/lenB);
      lenAvg = sqrt(dx_avg*dx_avg + dy_avg*dy_avg);
      os_x1 = -0.5*d_lineWidth_nm*dy_avg/lenAvg;
      os_y1 = 0.5*d_lineWidth_nm*dx_avg/lenAvg;
      cprod = dxA*os_y1 - dyA*os_x1;
      if (cprod < 0) {
        os_x1 = -os_x1;
        os_y1 = -os_y1;
      }
      // project onto segment perpendicular - should get something equal to 
      // 0.5*d_lineWidth_nm
      widthCorrection = (0.5*d_lineWidth_nm*lenA)/
                        (os_x1*(-dyA) + os_y1*dxA);
      if (widthCorrection > 2.0) widthCorrection = 2.0;
      os_x1 *= widthCorrection;
      os_y1 *= widthCorrection;

      // draw the segment from (x0,y0) to (x1,y1) with offsets os_x0,os_y0,
      // and os_x1, os_y1
/*
      printf("line: (%g,%g):(%g,%g) to (%g,%g):(%g,%g)\n",
              x0, y0, os_x0, os_y0, x1, y1, os_x1, os_y1);
*/
      glBegin(GL_LINE_LOOP);
      glVertex3f(x0+os_x0, y0+os_y0, 0.0);
      glVertex3f(x1+os_x1, y1+os_y1, 0.0);
      glVertex3f(x1-os_x1, y1-os_y1, 0.0);
      glVertex3f(x0-os_x0, y0-os_y0, 0.0);
      glEnd();
      os_x0 = os_x1;
      os_y0 = os_y1;

    } else {

      glBegin(GL_LINES);
      glVertex3f(x0, y0, 0.0);
      glVertex3f(x1, y1, 0.0);
      glEnd();
    }
    pntIter++;
  }

  if (d_lineWidth_nm > 0 && d_type == PS_POLYLINE) {
    os_x1 = -0.5*d_lineWidth_nm*dyB/lenB;
    os_y1 = 0.5*d_lineWidth_nm*dxB/lenB;

    // draw the segment from (x1,y1) to (x2,y2)
    glBegin(GL_LINE_LOOP);
    glVertex3f(x1+os_x0, y1+os_y0, 0.0);
    glVertex3f(x2+os_x1, y2+os_y1, 0.0);
    glVertex3f(x2-os_x1, y2-os_y1, 0.0);
    glVertex3f(x1-os_x0, y1-os_y0, 0.0);
    glEnd();

  } else {

    glBegin(GL_LINES);
    glVertex3f(x1, y1, 0.0);
    glVertex3f(x2, y2, 0.0);

    if (d_type == PS_POLYGON) {
      glVertex3f(x2, y2, 0.0);
      glVertex3f(x_start, y_start, 0.0);
    }
    glEnd();
  }
}

void PatternShape::draw() {
  if (d_type == PS_POLYGON) {
    drawPolygon();
  } else if (d_lineWidth_nm > 0){
    drawThickPolyline();
  } else {
    drawThinPolyline();
  }
}

list<PatternPoint>::iterator PatternShape::pointListBegin()
{
  return d_points.begin();
}

list<PatternPoint>::iterator PatternShape::pointListEnd()
{
  return d_points.end();
}

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

   d_nearDistX_pix;
   d_nearDistY_pix;
   d_nearDistX_nm;
   d_nearDistY_nm;

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

   d_grabX_nm = 0.0;
   d_grabY_nm = 0.0;

   d_shapeInProgress = vrpn_FALSE;
   d_pointInProgress = vrpn_FALSE;
   d_currShape = NULL;

   d_grabShape = NULL;
   d_grabPoint = NULL;

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

void PatternEditor::newPosition(nmb_Image *im)
{
  // really this is only necessary if the image is currently displayed
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
  d_viewer->dirtyWindow(d_mainWinID);
  d_viewer->dirtyWindow(d_navWinID);
}

list<PatternShape> PatternEditor::shapeList()
{
  return d_pattern;
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
    double x = event.mouse_x, y = event.mouse_y;
    double centerX_nm, centerY_nm;
    double x_world_nm, y_world_nm;

    switch(event.type) {
      case RESIZE_EVENT:
         d_mainWinWidth = event.width;
         d_mainWinHeight = event.height;
         break;
      case MOTION_EVENT:
         //if (event.state & IV_LEFT_BUTTON_MASK) {
             // adjust current line being drawn
             if (getUserMode() == PE_DRAWMODE) {
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
             // otherwise, look for something close by to select
             else {
               d_viewer->toImage(event.winID, &x, &y);
               mainWinPositionToWorld(x, y,
                                          x_world_nm, y_world_nm);
               if (grab(x_world_nm, y_world_nm)) {
                  setUserMode(PE_GRABMODE);
               }
             }
             break;
           default:
             break;
         }
         break;
      case BUTTON_RELEASE_EVENT:
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
             // release the currently grabbed object
             if (getUserMode() == PE_GRABMODE) {
               d_viewer->toImage(event.winID, &x, &y);
               mainWinPositionToWorld(x,y,x_world_nm, y_world_nm);
               updateGrab(x_world_nm, y_world_nm);
             }
             break;
           default:
             break;
         }
         break;
      case KEY_PRESS_EVENT:
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

  glPolygonMode(GL_FRONT, GL_FILL);

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

  PE_UserMode currMode = me->getUserMode();
  if (currMode == PE_SET_REGION || currMode == PE_SET_TRANSLATE) {
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
       printf("width=%d,height=%d,border = %d\n", 
         texwidth, texheight, ie.d_image->border());
*/
       glPixelTransferf(GL_RED_SCALE, (float)ie.d_red);
       glPixelTransferf(GL_GREEN_SCALE, (float)ie.d_green);
       glPixelTransferf(GL_BLUE_SCALE, (float)ie.d_blue);
       gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, texwidth, texheight, 
              GL_LUMINANCE,
              pixType, texture);
       glPixelTransferf(GL_RED_SCALE, 1.0);
       glPixelTransferf(GL_GREEN_SCALE, 1.0);
       glPixelTransferf(GL_BLUE_SCALE, 1.0);

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
       glEnd();
     }
}


void PatternEditor::drawPattern()
{
  list<PatternShape>::iterator shapeIter;
  int numShapes = 0;
  if (d_currShape) {
    numShapes++;
    d_currShape->draw();
  }
  for (shapeIter = d_pattern.begin();
       shapeIter != d_pattern.end(); shapeIter++)
  {
     (*shapeIter).draw();
     numShapes++;
  }
//  printf("drawing %d shapes\n", numShapes);
}

void PatternEditor::drawScale()
{
  float xSpan = d_mainWinMaxX_nm - d_mainWinMinX_nm;
  float ySpan = d_mainWinMaxY_nm - d_mainWinMinY_nm;
  char str[64];
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
          me->d_worldMaxY_nm, me->d_worldMinY_nm, -1, 1);

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
  x_nm = d_worldMinX_nm + x*(d_worldMaxX_nm - d_worldMinX_nm);
  y_nm = d_worldMaxY_nm + (1.0-y)*(d_worldMinY_nm - d_worldMaxY_nm);
}

void PatternEditor::mainWinPositionToWorld(double x, double y,
                                       double &x_nm, double &y_nm)
{
  x_nm = d_mainWinMinX_nm + x*(d_mainWinMaxX_nm - d_mainWinMinX_nm);
  y_nm = d_mainWinMaxY_nm + (1.0-y)*(d_mainWinMinY_nm - d_mainWinMaxY_nm);
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

int PatternEditor::startPoint(const double x_nm, const double y_nm)
{
  if (d_shapeInProgress) {
    printf("adding point to current shape\n");
    d_pointInProgress = vrpn_TRUE;
    d_currShape->addPoint(x_nm, y_nm);
    return 0;
  } else {
    printf("Error, startPoint called when not drawing shape\n");
    return -1;
  }
}

int PatternEditor::updatePoint(const double x_nm, const double y_nm)
{
  if (d_shapeInProgress && d_pointInProgress) {
    d_currShape->removePoint();
    d_currShape->addPoint(x_nm, y_nm);
    return 0;
  } else {
    printf("Error, updatePoint called when not setting point\n");
    return -1;
  }
}

int PatternEditor::finishPoint(const double x_nm, const double y_nm)
{
  int result = updatePoint(x_nm, y_nm);
//  printf("finishPoint\n");
  d_pointInProgress = vrpn_FALSE;
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

vrpn_bool PatternEditor::grab(const double x_nm, const double y_nm)
{
  d_grabX_nm = x_nm;
  d_grabY_nm = y_nm;
  return vrpn_FALSE;
}

void PatternEditor::updateGrab(const double x_nm, const double y_nm)
{
  double delX_nm = x_nm - d_grabX_nm;
  double delY_nm = y_nm - d_grabY_nm;

  d_grabX_nm = x_nm;
  d_grabY_nm = y_nm;

  if (d_grabShape) {
    d_grabShape->translate(delX_nm, delY_nm);
  }
  if (d_grabPoint) {
    d_grabPoint->translate(delX_nm, delY_nm);
  }
}

#include "patternEditor.h"
#include "GL/gl.h"

#ifndef M_PI
#define M_PI           3.14159265358979323846
#endif

PatternEditor::PatternEditor(int startX, int startY)
{
   d_patternColorMap.setMinLineExposureColor(0.0, 1.0, 1.0);
   d_patternColorMap.setMaxLineExposureColor(1.0, 1.0, 0.0);
   d_patternColorMap.setMinAreaExposureColor(0.0, 1.0, 1.0);
   d_patternColorMap.setMaxAreaExposureColor(1.0, 1.0, 0.0);

   d_viewer = ImageViewer::getImageViewer();
   char *display_name = (char *)getenv("DISPLAY");
   if (!display_name) {
      display_name = "unix:0";
   }
   d_viewer->init(display_name);
   d_mainWinID = d_viewer->createWindow(display_name, 
         startX, startY, 300, 300, "Pattern Editor");
   d_viewer->setWindowEventHandler(d_mainWinID, 
         PatternEditor::mainWinEventHandler, this);
   d_viewer->setWindowDisplayHandler(d_mainWinID, 
         PatternEditor::mainWinDisplayHandler, this);

   d_navWinID = d_viewer->createWindow(display_name,
         startX + 500, startY, 100, 100, "Navigation");
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
  
   d_mainWinWidth = 300;
   d_mainWinHeight = 300;
   d_navWinWidth = 100;
   d_navWinHeight = 100;

   d_nearDistX_pix = 10;
   d_nearDistY_pix = 10;
   d_nearDistX_nm = 100;
   d_nearDistY_nm = 100;

   d_drawingTool = PE_THINPOLYLINE;
   d_userMode = PE_IDLE;
   d_lineWidth_nm = 0.0;
   d_line_exposure_pCoulombs_per_cm = 0.0;
   d_area_exposure_uCoulombs_per_square_cm = 0.0;

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
   d_viewportSet = vrpn_FALSE;
 
   d_numExposurePointsRecorded = 0;
   d_exposurePointsDisplayed = vrpn_FALSE;
}

void PatternEditor::setWindowStartPosition(int startX, int startY)
{
  d_viewer->setWindowPosition(d_mainWinID, startX, startY);
  d_viewer->setWindowPosition(d_navWinID, startX + 400, startY);
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

  ImageElement ie(im);
  double newArea = im->areaInWorld();

  list<ImageElement>::iterator insertPnt;
  if (d_images.empty()) {
    insertPnt = d_images.begin();
  } else {
    list<ImageElement>::iterator testElt = d_images.begin();
    while (testElt != d_images.end()) {
      if ((*testElt).d_image->areaInWorld() < newArea) break;
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

vrpn_bool PatternEditor::getImageEnable(nmb_Image *im)
{
    list<ImageElement>::iterator imIter;
  for (imIter = d_images.begin();
       imIter != d_images.end(); imIter++)
  {
     if ((*imIter).d_image == im) {
          return (*imIter).d_enabled;
     }
  }
  return vrpn_FALSE;
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
  d_images.sort();
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

void PatternEditor::setDrawingParameters(double lineWidth_nm, 
                                         double area_exposure,
                                         double line_exposure)
{
 d_lineWidth_nm = lineWidth_nm;
 d_area_exposure_uCoulombs_per_square_cm = area_exposure;
 d_line_exposure_pCoulombs_per_cm = line_exposure;

 if (d_currShape) {
   d_currShape->setExposure(d_line_exposure_pCoulombs_per_cm,
                            d_area_exposure_uCoulombs_per_square_cm);
   updateExposureLevels();
   if (d_currShape->type() == PS_POLYLINE && 
       d_drawingTool == PE_THICKPOLYLINE) {
     ((PolylinePatternShape *)d_currShape)->setLineWidth(d_lineWidth_nm);
   }
   d_viewer->dirtyWindow(d_mainWinID);
 }
}

void PatternEditor::setDrawingTool(PE_DrawTool tool)
{
    if (d_shapeInProgress) {
      endShape();
    }
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
    d_pattern.removeSubShape();
    updateExposureLevels();
    d_viewer->dirtyWindow(d_mainWinID);
  }
  return;
}

void PatternEditor::addShape(PatternShape *shape)
{
  d_pattern.addSubShape(shape);
  updateExposureLevels();
  d_viewer->dirtyWindow(d_mainWinID);
}

void PatternEditor::updateExposureLevels()
{
  list<double> lineExposureLevels, areaExposureLevels;
  d_pattern.getExposureLevels(lineExposureLevels, areaExposureLevels);
  if (d_currShape) {
    list<double> lineExposureLevels2, areaExposureLevels2;
    d_currShape->getExposureLevels(lineExposureLevels2, areaExposureLevels2);
    lineExposureLevels.merge(lineExposureLevels2);
    areaExposureLevels.merge(areaExposureLevels2);
  }
  d_patternColorMap.setExposureLevels(lineExposureLevels, areaExposureLevels);
}

void PatternEditor::addTestGrid(double minX_nm, double minY_nm,
                                double maxX_nm, double maxY_nm,
                                int numHorizontal, int numVertical)
{
  double xIncrement_nm = 
                         (maxX_nm - minX_nm)/(double)(numVertical-1);
  double yIncrement_nm = 
                         (maxY_nm - minY_nm)/(double)(numHorizontal-1);
  if (d_shapeInProgress) {
    endShape();
  }
  CompositePatternShape *grid = new CompositePatternShape();

  PolylinePatternShape gridline;
  gridline.setExposure(d_line_exposure_pCoulombs_per_cm,
                       d_area_exposure_uCoulombs_per_square_cm);

  double x_begin, y_begin, x_end, y_end;
  int i; 

  // do horizontal lines first
  x_begin = minX_nm;
  y_begin = minY_nm;
  x_end = maxX_nm;
  y_end = y_begin;
  for (i = 0; i < numHorizontal; i++) {
    gridline.addPoint(x_begin, y_begin);
    gridline.addPoint(x_end, y_end);
    grid->addSubShape(new PolylinePatternShape(gridline));
    gridline.clearPoints();
    y_begin += yIncrement_nm;
    y_end += yIncrement_nm;
  }
  // now do vertical lines
  x_begin = minX_nm;
  y_begin = minY_nm;
  x_end = x_begin;
  y_end = maxY_nm;
  for (i = 0; i < numVertical; i++) {
    gridline.addPoint(x_begin, y_begin);
    gridline.addPoint(x_end, y_end);
    grid->addSubShape(new PolylinePatternShape(gridline));
    gridline.clearPoints();
    x_begin += xIncrement_nm;
    x_end += xIncrement_nm;
  }

  addShape(grid);
}

/*
int PatternEditor::findNearestShapePoint(double x, double y, 
                    list<PatternShapeListElement>::iterator &nearestShape,
                    list<PatternPoint>::iterator &nearestPoint,
                    double &minDist)
{
  // search all shape and dump points for the closest one
  // and return it
  if (d_pattern.empty()) return -1;
 
  list<PatternShapeListElement>::iterator currShape;
  list<PatternPoint>::iterator pointRef;
  currShape = d_pattern.getSubShapes().begin();
  double currDist;

  nearestShape = currShape;
  findNearestPoint((*currShape).d_points, x, y, nearestPoint, minDist);
  
  currShape++;
  while (currShape != d_pattern.getSubShapes().end()) {
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
*/

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

  int newWinWidth = d_mainWinWidth, newWinHeight = d_mainWinHeight;
  double viewAspect = (maxX_nm - minX_nm)/(maxY_nm - minY_nm);
  newWinWidth = d_mainWinHeight*viewAspect;
  d_viewer->setWindowSize(d_mainWinID, newWinWidth, newWinHeight);
  d_viewer->dirtyWindow(d_mainWinID);
  d_viewer->dirtyWindow(d_navWinID);
  d_viewportSet = vrpn_TRUE;
}

void PatternEditor::getViewport(double &minX_nm, double &minY_nm,
                                double &maxX_nm, double &maxY_nm)
{
  minX_nm = d_mainWinMinX_nm;
  minY_nm = d_mainWinMinY_nm;
  maxX_nm = d_mainWinMaxX_nm;
  maxY_nm = d_mainWinMaxY_nm;
}

void PatternEditor::setPattern(ExposurePattern &pattern)
{
  d_pattern = pattern;
  updateExposureLevels();
  d_viewer->dirtyWindow(d_mainWinID);
}

void PatternEditor::setExposurePointDisplayEnable(vrpn_int32 enable)
{
  if (enable && !d_exposurePointsDisplayed ||
      !enable && d_exposurePointsDisplayed) {
    d_exposurePointsDisplayed = !d_exposurePointsDisplayed;
    d_viewer->dirtyWindow(d_mainWinID);
  }
}

void PatternEditor::addExposurePoint(double x_nm, double y_nm)
{
  d_exposurePoints.push_back(PatternPoint(x_nm, y_nm));
  d_viewer->dirtyWindow(d_mainWinID);

  if (d_numExposurePointsRecorded == 40000) {
    d_exposurePoints.pop_front();   
  } else {
    d_numExposurePointsRecorded++;
  }
}

void PatternEditor::clearExposurePoints()
{
  d_exposurePoints.clear();
  d_viewer->dirtyWindow(d_mainWinID);
  d_numExposurePointsRecorded = 0;
}

void PatternEditor::addImageToDisplay(nmb_Image *im)
{
  vrpn_bool foundImage = vrpn_FALSE;
  list<ImageElement>::iterator imIter;
  for (imIter = d_images.begin();
       imIter != d_images.end(); imIter++) {
     if ((*imIter).d_image == im) {
          (*imIter).d_enabled = vrpn_TRUE;
          foundImage = vrpn_TRUE;
     }
  }
  if (!foundImage) {
    addImage(im);
    setImageEnable(im, vrpn_TRUE);
  }
  d_viewer->dirtyWindow(d_mainWinID);
}

void PatternEditor::removeImageFromDisplay(nmb_Image *image)
{
  setImageEnable(image, vrpn_FALSE);
}

void PatternEditor::updateDisplayTransform(nmb_Image *image, double *transform)
{
  // really this is only necessary if the image is currently displayed
  list<ImageElement>::iterator imIter;
  for (imIter = d_images.begin();
       imIter != d_images.end(); imIter++) {
    if ((*imIter).d_image == image) {
      image->setWorldToImageTransform(transform);
      if ((*imIter).d_enabled) {
        d_images.sort();
        d_viewer->dirtyWindow(d_mainWinID);
      }
    }
  }
}

void PatternEditor::setDisplayColorMap(nmb_Image *image,
                const char *map, const char *mapdir)
{
  printf("Sorry, setDisplayColorMap is not implemented\n");
}

void PatternEditor::setDisplayColorMapRange(nmb_Image *image,
             float data_min, float data_max,
             float color_min, float color_max)
{
  printf("Sorry, setDisplayColorMapRange is not implemented\n");
}

void PatternEditor::updateImage(nmb_Image *image)
{
  d_viewer->dirtyWindow(d_mainWinID);
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
    int desired_width;
    switch(event.type) {
      case RESIZE_EVENT:
         d_mainWinWidth = event.width;
         d_mainWinHeight = event.height;
         desired_width = d_mainWinHeight*
                         (d_mainWinMaxX_nm - d_mainWinMinX_nm)/
                         (d_mainWinMaxY_nm - d_mainWinMinY_nm);
         if (desired_width != d_mainWinWidth) {
           d_viewer->setWindowSize(event.winID, desired_width, d_mainWinHeight);
         }
         break;
      case MOTION_EVENT:
         x = event.mouse_x; y = event.mouse_y;
         //if (event.state & IV_LEFT_BUTTON_MASK) {
             // adjust current line being drawn
             if (getUserMode() == PE_DRAWMODE && 
                 (event.state & IV_LEFT_BUTTON_MASK)) {
               mainWinPositionToWorld(x,y,x_world_nm, y_world_nm);
               updatePoint(x_world_nm, y_world_nm);
               d_viewer->dirtyWindow(event.winID);
             }
         //} else if (event.state & IV_RIGHT_BUTTON_MASK) {
             // move the currently grabbed object if there is one
             else if (getUserMode() == PE_GRABMODE) {
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
               if (d_drawingTool == PE_THINPOLYLINE) {
                 startShape(PS_POLYLINE);
               } else if (d_drawingTool == PE_THICKPOLYLINE) {
                 startShape(PS_POLYLINE);
               } else if (d_drawingTool == PE_POLYGON) {
                 startShape(PS_POLYGON);
               } else if (d_drawingTool == PE_DUMP_POINT) {
                 startShape(PS_DUMP); 
               } else if (d_drawingTool == PE_SELECT) {

               }
               setUserMode(PE_DRAWMODE);
             }
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
               mainWinPositionToWorld(x, y, x_world_nm, y_world_nm);
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
             mainWinPositionToWorld(x, y, x_world_nm, y_world_nm);
             if (getUserMode() == PE_DRAWMODE) {
               // set the point
               finishPoint(x_world_nm, y_world_nm);
               if (d_drawingTool == PE_DUMP_POINT) {
                 endShape();
                 setUserMode(PE_IDLE);
               }
             }
             d_viewer->dirtyWindow(event.winID);
             break;
           case IV_RIGHT_BUTTON:
/*
             // release the currently grabbed object
             if (getUserMode() == PE_GRABMODE) {
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
             mainWinPositionToWorld(x, y, centerX_nm, centerY_nm);
             zoomBy(centerX_nm, centerY_nm, 2.0);
             d_viewer->dirtyWindow(d_navWinID);
             d_viewer->dirtyWindow(event.winID);
             break;
           case 'Z':
             mainWinPositionToWorld(x, y, centerX_nm, centerY_nm);
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


    double temp;
    if ((xmin > xmax) || 
        (ymin > ymax)) {
       printf("zoomBy: min/max swapped: this shouldn't happen\n");
       if (xmin > xmax) {
           temp = xmin;
           xmin = xmax;
           xmax = temp;
       }
       if (ymin > ymax) {
           temp = ymin;
           ymin = ymax;
           ymax = temp;
       }
    }
    setViewport(xmin, ymin, xmax, ymax);
}

void PatternEditor::clampMainWinRectangle(double &xmin, double &ymin,
                              double &xmax, double &ymax)
{
  xmin = max(xmin, d_worldMinX_nm);
  ymin = max(ymin, d_worldMinY_nm);
  xmax = min(xmax, d_worldMaxX_nm);
  ymax = ymin + (xmax - xmin)*(d_mainWinHeight)/
                (double)(d_mainWinWidth);
  if (ymax > d_worldMaxY_nm) {
    ymin -= (ymax - d_worldMaxY_nm);
    ymax = d_worldMaxY_nm;
  }
}

int PatternEditor::mainWinDisplayHandler(
                   const ImageViewerDisplayData &data, void *ud)
{
  PatternEditor *me = (PatternEditor *)ud;

  glPushAttrib(GL_ENABLE_BIT | GL_TEXTURE_BIT | GL_VIEWPORT_BIT);

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

  // setup viewing/model projection
  glViewport(0, 0, data.winWidth, data.winHeight);

  double minXSave, maxXSave, minYSave, maxYSave;
  minXSave = me->d_mainWinMinX_nm;
  maxXSave = me->d_mainWinMaxX_nm;
  minYSave = me->d_mainWinMinY_nm;
  maxYSave = me->d_mainWinMaxY_nm;

  PE_UserMode currMode = me->getUserMode();
  if (currMode == PE_SET_REGION || currMode == PE_SET_TRANSLATE) {
    me->d_mainWinMinX_nm = me->d_mainWinMinXadjust_nm;
    me->d_mainWinMaxX_nm = me->d_mainWinMaxXadjust_nm;
    me->d_mainWinMinY_nm = me->d_mainWinMinYadjust_nm;
    me->d_mainWinMaxY_nm = me->d_mainWinMaxYadjust_nm;
  }

  // draw each image currently enabled
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

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(me->d_mainWinMaxX_nm, me->d_mainWinMinX_nm, 
          me->d_mainWinMinY_nm, me->d_mainWinMaxY_nm, -1, 1);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  me->drawPattern();

  me->drawScale();

  me->drawExposureLevels();

  if (me->d_exposurePointsDisplayed) {
    me->drawExposurePoints();
  }

  if (currMode == PE_SET_REGION || currMode == PE_SET_TRANSLATE) {
    me->d_mainWinMinX_nm = minXSave;
    me->d_mainWinMaxX_nm = maxXSave;
    me->d_mainWinMinY_nm = minYSave;
    me->d_mainWinMaxY_nm = maxYSave;
  }

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glPopAttrib();

  return 0;
}


void PatternEditor::drawImage(const ImageElement &ie)
{
  nmb_TransformMatrix44 W2I;
  double l, r, b, t;
/*
  some debugging test code
  W2I.set(0, 0, -1.0); // flip X
  W2I.set(0, 3, 1.5);  // and translate
  l = 0; r = 1; b = 0; t = 1;
  d_viewer->drawImage(d_mainWinID, ie.d_image,
      ie.d_red, ie.d_green, ie.d_blue, ie.d_opacity, &l, &r, &b, &t, &W2I);
*/


  ie.d_image->getWorldToImageTransform(W2I);
  l = d_mainWinMaxX_nm; r = d_mainWinMinX_nm;
  b = d_mainWinMinY_nm; t = d_mainWinMaxY_nm;
  d_viewer->drawImage(d_mainWinID, ie.d_image,
      ie.d_red, ie.d_green, ie.d_blue, ie.d_opacity, &l, &r, &b, &t, &W2I);

}

void PatternEditor::drawPattern()
{
  double units_per_pixel_x, units_per_pixel_y;

  units_per_pixel_x = (d_mainWinMaxX_nm - d_mainWinMinX_nm)/
                      (double)d_mainWinWidth;
  units_per_pixel_y = (d_mainWinMaxY_nm - d_mainWinMinY_nm)/
                      (double)d_mainWinHeight;
  if (d_currShape) {
    d_currShape->drawToDisplay(units_per_pixel_x, units_per_pixel_y,
                           d_patternColorMap);
  }
  d_pattern.drawToDisplay(units_per_pixel_x, units_per_pixel_y, 
                           d_patternColorMap);

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

void PatternEditor::drawExposureLevels()
{
  char str[64];
  double x, y;
  // XXX - should do this without so much magic
  x = 0.3*d_mainWinMaxX_nm + 0.7*d_mainWinMinX_nm;
  y = 0.9*d_mainWinMaxY_nm + 0.1*d_mainWinMinY_nm;

  double units_per_pixel_x, units_per_pixel_y;

  units_per_pixel_x = (d_mainWinMaxX_nm - d_mainWinMinX_nm)/
                      (double)d_mainWinWidth;
  units_per_pixel_y = (d_mainWinMaxY_nm - d_mainWinMinY_nm)/
                      (double)d_mainWinHeight;

  d_patternColorMap.draw(x, y, units_per_pixel_x, units_per_pixel_y);
}


void PatternEditor::drawExposurePoints()
{
  list<PatternPoint>::iterator point = d_exposurePoints.begin();

  if (point == d_exposurePoints.end()) return;

  glColor4f(0.0, 1.0, 0.0, 1.0);
  glBegin(GL_POINTS);
  while (point != d_exposurePoints.end()) {
    glVertex2d((*point).d_x, (*point).d_y);
    point++;
  }
  glEnd();
}

int PatternEditor::navWinEventHandler(
                   const ImageViewerWindowEvent &event, void *ud)
{
    PatternEditor *me = (PatternEditor *)ud;
    double x = 0, y = 0;
    double xmin = 0, ymin = 0, xmax = 0, ymax = 0;

    switch(event.type) {
      case RESIZE_EVENT:
         me->d_navWinWidth = event.width;
         me->d_navWinHeight = event.height;
         break;
      case MOTION_EVENT:
	 x = event.mouse_x; y = event.mouse_y;
         if (event.state & IV_LEFT_BUTTON_MASK ||
             event.state & IV_RIGHT_BUTTON_MASK) {
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
             me->clampMainWinRectangle(me->d_mainWinMinXadjust_nm,
                                       me->d_mainWinMinYadjust_nm,
                                       me->d_mainWinMaxXadjust_nm,
                                       me->d_mainWinMaxYadjust_nm);
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
             me->clampMainWinRectangle(me->d_mainWinMinXadjust_nm,
                                       me->d_mainWinMinYadjust_nm,
                                       me->d_mainWinMaxXadjust_nm,
                                       me->d_mainWinMaxYadjust_nm);
             me->d_viewer->dirtyWindow(me->d_mainWinID);
         }
         break;
      case BUTTON_PRESS_EVENT:
         x = event.mouse_x; y = event.mouse_y;
         switch(event.button) {
           case IV_LEFT_BUTTON:
             // start dragging a rectangle
             me->setUserMode(PE_SET_REGION);
             me->navWinPositionToWorld(x, y, x, y);
             me->d_navDragStartX_nm = x;
             me->d_navDragStartY_nm = y;
             me->d_navDragEndX_nm = me->d_navDragStartX_nm;
             me->d_navDragEndY_nm = me->d_navDragStartY_nm;
             me->d_viewer->dirtyWindow(event.winID);
             break;
           case IV_RIGHT_BUTTON:
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
               me->navWinPositionToWorld(x, y, 
                 me->d_navDragEndX_nm, me->d_navDragEndY_nm);
               xmin = min(me->d_navDragStartX_nm, me->d_navDragEndX_nm);
               ymin = min(me->d_navDragStartY_nm, me->d_navDragEndY_nm);
               xmax = max(me->d_navDragStartX_nm, me->d_navDragEndX_nm);
               ymax = max(me->d_navDragStartY_nm, me->d_navDragEndY_nm);
              
               me->clampMainWinRectangle(xmin, ymin, xmax, ymax);
               me->setViewport(xmin, ymin, xmax, ymax);
/*
               me->d_mainWinMaxX_nm = max(me->d_navDragStartX_nm,
                                    me->d_navDragEndX_nm);
               me->d_mainWinMinX_nm = min(me->d_navDragStartX_nm,
                                    me->d_navDragEndX_nm);
               me->d_mainWinMaxY_nm = max(me->d_navDragStartY_nm,
                                    me->d_navDragEndY_nm);
               me->d_mainWinMinY_nm = min(me->d_navDragStartY_nm,
                                    me->d_navDragEndY_nm);
*/
               me->d_viewer->dirtyWindow(event.winID);
               me->d_viewer->dirtyWindow(me->d_mainWinID);
             }
             break;
           case IV_RIGHT_BUTTON:
             if (me->getUserMode() == PE_SET_TRANSLATE) {
               // copy the translated rectangle into the main 
               // displayed rectangle
               me->setUserMode(PE_IDLE);
               me->navWinPositionToWorld(x, y,
                                       me->d_navDragEndX_nm, me->d_navDragEndY_nm);
               double t_x = me->d_navDragEndX_nm - me->d_navDragStartX_nm;
               double t_y = me->d_navDragEndY_nm - me->d_navDragStartY_nm;
               xmin = me->d_mainWinMinX_nm + t_x;
               ymin = me->d_mainWinMinY_nm + t_y;
               xmax = me->d_mainWinMaxX_nm + t_x;
               ymax = me->d_mainWinMaxY_nm + t_y;
               me->clampMainWinRectangle(xmin, ymin, xmax, ymax);
               me->setViewport(xmin, ymin, xmax, ymax);

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

  glOrtho(me->d_worldMaxX_nm, me->d_worldMinX_nm,
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
  x_nm = x; y_nm = y;
  d_viewer->toImagePnt(d_navWinID, d_worldMaxX_nm, d_worldMinX_nm,
                    d_worldMinY_nm, d_worldMaxY_nm, &x_nm, &y_nm);
/*
  x_nm = d_worldMinX_nm + x*(d_worldMaxX_nm - d_worldMinX_nm);
  y_nm = d_worldMinY_nm + y*(d_worldMaxY_nm - d_worldMinY_nm);
*/
}

void PatternEditor::mainWinPositionToWorld(double x, double y,
                                       double &x_nm, double &y_nm)
{
  x_nm = x; y_nm = y;
  d_viewer->toImagePnt(d_mainWinID, d_mainWinMaxX_nm, d_mainWinMinX_nm,
                    d_mainWinMinY_nm, d_mainWinMaxY_nm, &x_nm, &y_nm);
/*
  x_nm = d_mainWinMinX_nm + x*(d_mainWinMaxX_nm - d_mainWinMinX_nm);
  y_nm = d_mainWinMinY_nm + y*(d_mainWinMaxY_nm - d_mainWinMinY_nm);
*/
}

void PatternEditor::worldToMainWinPosition(const double x_nm,
                                     const double y_nm, 
                             double &x_norm, double &y_norm)
{

  double delX = d_mainWinMaxX_nm - d_mainWinMinX_nm;
  double delY = d_mainWinMaxY_nm - d_mainWinMinY_nm;
  if (delX != 0) {
    x_norm = (x_nm - d_mainWinMinX_nm)/delX;
  } else {
    fprintf(stderr, "Warning, x range is 0\n");
  }
  if (delY != 0) {
    y_norm = (y_nm - d_mainWinMinY_nm)/delY;
  } else {
    fprintf(stderr, "Warning, y range is 0\n");
  }

}

void PatternEditor::mainWinNMToPixels(const double x_nm, const double y_nm,
                                      double &x_pixels, double &y_pixels)
{
  //worldToMainWinPosition(x_nm, y_nm, x_pixels, y_pixels);
  //d_viewer->toPixels(d_mainWinID, &x_pixels, &y_pixels);
  x_pixels = x_nm; y_pixels = y_nm;
  d_viewer->toPixelsPnt(d_mainWinID, d_mainWinMaxX_nm, d_mainWinMinX_nm,
                     d_mainWinMinY_nm, d_mainWinMaxY_nm, &x_pixels, &y_pixels);
}

void PatternEditor::mainWinNMToPixels(const double dist_nm,
                                      double &dist_pixels)
{
  double d0 = 0, d1 = 0;
  mainWinNMToPixels(dist_nm, d0, dist_pixels, d1);
//  worldToMainWinPosition(dist_nm, d0, dist_pixels, d1);
//  d_viewer->toPixels(d_mainWinID, &dist_pixels, &d1);
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

  PatternShape *newShape = NULL;
  PolygonPatternShape *newPolygon = NULL;
  PolylinePatternShape *newPolyline = NULL;
  DumpPointPatternShape *newDumpPoint = NULL;

  switch(type) {
    case PS_POLYLINE:
      newPolyline = new PolylinePatternShape();
      newPolyline->setExposure(d_line_exposure_pCoulombs_per_cm,
                               d_area_exposure_uCoulombs_per_square_cm);
      if (d_drawingTool == PE_THICKPOLYLINE) {
        newPolyline->setLineWidth(d_lineWidth_nm);
      } else {
        newPolyline->setLineWidth(0);
      }
      newShape = (PatternShape *)newPolyline;
      break;
    case PS_POLYGON:
      newPolygon = new PolygonPatternShape();
      newPolygon->setExposure(d_line_exposure_pCoulombs_per_cm,
                               d_area_exposure_uCoulombs_per_square_cm);
      newShape = (PatternShape *)newPolygon;
      break;
    case PS_DUMP:
      newDumpPoint = new DumpPointPatternShape();
      newShape = (PatternShape *)newDumpPoint;
      break;
  }
  d_currShape = newShape;
  return 0;
}

vrpn_bool PatternEditor::selectPoint(const double x_nm, const double y_nm)
{
  list<PatternShapeListElement>::iterator shapeRef;
  list<PatternPoint>::iterator pntRef;

  double minDist, minDistPixels;
  double x_pnt, y_pnt;
  //double x_offset, y_offset;
  vrpn_bool selectedSomething = vrpn_FALSE;
/*
  if (findNearestShapePoint(x_nm, y_nm, shapeRef, pntRef, minDist)) {
      return vrpn_FALSE;
  }
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
*/
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
    if (d_currShape->type() == PS_POLYGON) {
      ((PolygonPatternShape *)d_currShape)->addPoint(x_nm, y_nm);
    } else if (d_currShape->type() == PS_POLYLINE) {
      ((PolylinePatternShape *)d_currShape)->addPoint(x_nm, y_nm);
    } else if (d_currShape->type() == PS_DUMP) {
      ((DumpPointPatternShape *)d_currShape)->setLocation(x_nm, y_nm);
    }
    return 0;
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
      if (d_currShape->type() == PS_POLYGON) {
        ((PolygonPatternShape *)d_currShape)->removePoint();
        ((PolygonPatternShape *)d_currShape)->addPoint(x_nm, y_nm);
      } else if (d_currShape->type() == PS_POLYLINE) {
        ((PolylinePatternShape *)d_currShape)->removePoint();
        ((PolylinePatternShape *)d_currShape)->addPoint(x_nm, y_nm);
      } else if (d_currShape->type() == PS_DUMP) {
        ((DumpPointPatternShape *)d_currShape)->setLocation(x_nm, y_nm);
      }
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
  addShape(d_currShape->duplicate());
  clearDrawingState();
  return 0;
}

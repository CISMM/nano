#ifndef PATTERNEDITOR_H
#define PATTERNEDITOR_H

/* important note:

PatternShape objects on the client store points in image coordinates
for the current canvas image 

PatternShape objects on the server (SEM computer) store points in world
coordinates (basically a function of the SEM magnification)

*/


#include "imageViewer.h"
#include "nmb_Image.h"
#include <list>
#include "Tcl_Linkvar.h"
#include "ImageMaker.h"
#include "exposurePattern.h"
#include "nmb_ImageDisplay.h"

class ControlPanels;

class ImageElement {
  public:
   ImageElement():d_red(1.0), d_green(1.0), d_blue(1.0),
                  d_opacity(1.0), d_enabled(vrpn_FALSE),
                  d_image(NULL) {}
   ImageElement(nmb_Image *im, 
                double r=1.0, double g=1.0, double b=1.0, 
                double o=1.0, vrpn_bool e=vrpn_FALSE):
            d_red(r), d_green(g), d_blue(b), d_opacity(o), 
            d_enabled(e), d_image(im) {}

   int operator== (const ImageElement& ie) {return (d_image == ie.d_image);}
   int operator< (const ImageElement& ie);
   
   double d_red;
   double d_green;
   double d_blue;
   double d_opacity;
   vrpn_bool d_enabled;
   nmb_Image *d_image;
};

typedef enum {PE_IDLE, PE_SET_REGION, PE_SET_TRANSLATE,
                 PE_DRAWMODE, PE_GRABMODE} PE_UserMode;
typedef enum {PE_THINPOLYLINE, PE_THICKPOLYLINE, PE_POLYGON, PE_DUMP_POINT, 
              PE_SELECT} PE_DrawTool;

const int PE_SELECT_DIST = 10;

class PatternEditor : public nmb_ImageDisplay {
  public:
   PatternEditor(int startX = 20, int startY = 100);
   ~PatternEditor();
   void setWindowStartPosition(int startX, int startY);
   void show();
   /// add image to the list of possibly displayed images but don't display it
   void addImage(nmb_Image *im);
   /// remove image along with any of its display settings
   void removeImage(nmb_Image *im);
   /// enable the display of an image in the list
   void setImageEnable(nmb_Image *im, vrpn_bool displayEnable);
   vrpn_bool getImageEnable(nmb_Image *im);
   void setImageOpacity(nmb_Image *im, double opacity);
   void setImageColor(nmb_Image *im, double r, double g, double b);
   ImageElement *getImageParameters(nmb_Image *im);
   void showSingleImage(nmb_Image *im);
   int mainWinID() {return d_mainWinID;}
   void setDrawingParameters(double lineWidth_nm, 
                             double area_exposure, double line_exposure);
   void setDrawingTool(PE_DrawTool tool);
   void undoPoint();
   void undoShape();
   void clearPattern();
   void addShape(PatternShape *shape);
   void updateExposureLevels();
   void addTestGrid(double startX_nm, double startY_nm,
                    double endX_nm, double endY_nm,
                    int numHorizontal, int numVertical);
   void saveImageBuffer(const char *filename,
                const char *filetype);
   void setViewport(double minX_nm, double minY_nm, 
                    double maxX_nm, double maxY_nm);
   void getViewport(double &minX_nm, double &minY_nm,
                    double &maxX_nm, double &maxY_nm);
   void setControlPanels(ControlPanels *cp) { d_controlPanels = cp; }
   void setScanRange(double minX_nm, double minY_nm, 
                    double maxX_nm, double maxY_nm);
   ExposurePattern &getPattern() {return d_pattern;}
   void setPattern(ExposurePattern &pattern);
   void setExposurePointDisplayEnable(vrpn_int32 enable);
   void addExposurePoint(double x_nm, double y_nm);
   void clearExposurePoints();

   void setCanvasImage(nmb_Image *image);
   void setLiveImage(nmb_Image *image);
   void updatePatternTransform();
   void setPatternColor(double r, double g, double b);

   // more general ImageDisplay interface used by nmr_RegistrationUI:

   /// if an image is already registered, enable it, otherwise, add it to the
   /// list of registered images and then enable it
   virtual void addImageToDisplay(nmb_Image *image);
   /// disable display but don't remove display settings
   virtual void removeImageFromDisplay(nmb_Image *image);
   virtual void updateDisplayTransform(nmb_Image *image, double *transform,
	   bool transformIsSelfReferential);
   virtual void setDisplayColorMap(nmb_Image *image, 
                           const char *map, const char *mapdir);
   virtual void setDisplayColorMapRange(nmb_Image *image,
                        float data_min, float data_max,
                        float color_min, float color_max);
   virtual void updateColorMapTextureAlpha(float alpha);
   virtual void updateImage(nmb_Image *image);

  protected:
   static int mainWinEventHandler(const ImageViewerWindowEvent &event, 
                                  void *ud);
   int handleMainWinEvent(const ImageViewerWindowEvent &event);
   static int mainWinDisplayHandler(const ImageViewerDisplayData &data, 
                                    void *ud);
   // helper for mainWinDisplayHandler:
   void drawImage(const ImageElement &ie);
   void drawPattern();
   void drawScale();
   void drawExposureLevels();
   void drawExposurePoints();
   void exposureLevelColor(double exposure, double *color);
   void exposureLevelColor(int rank, double *color);

   static int navWinEventHandler(const ImageViewerWindowEvent &event,
                                 void *ud);
   static int navWinDisplayHandler(const ImageViewerDisplayData &data,
                                    void *ud);
   void navWinPositionToWorld(double x, double y,
                                       double &x_nm, double &y_nm);
   void mainWinPositionToWorld(double x, double y,
                                       double &x_nm, double &y_nm);
   void worldToMainWinPosition(const double x_nm, const double y_nm,
                               double &x_norm, double &y_norm);
   void mainWinNMToPixels(const double x_nm, const double y_nm,
                          double &x_pixels, double &y_pixels);
   void mainWinNMToPixels(const double dist_nm,double &dist_pixels);
   void zoomBy(double centerX_nm, double centerY_nm,
                           double magFactor);
   void zoomAndReCenter(double centerX_nm, double centerY_nm,
                           double magFactor);

   void clampMainWinRectangle(double &xmin, double &ymin,
                              double &xmax, double &ymax);

   void updateWorldExtents();

   // stuff for creating a pattern
   int startShape(ShapeType type);
   int startPoint(const double x_nm, const double y_nm);
   int updatePoint(const double x_nm, const double y_nm);
   int finishPoint(const double x_nm, const double y_nm);
   int endShape();
   void clearDrawingState();
   void addDumpPoint(const double x, const double y);
   void updateDumpPoint(const double x, const double y);
   void updateLengthIndicator();

   // stuff for manipulating a pattern

   // translate currently selected shape to the specified world coordinates
   int updateGrab(const double x_nm, const double y_nm);

   // rotate currently selected shape
   int updateRotation(const double x_nm, const double y_nm);

   // select the shape to which the point nearest the cursor belongs
   vrpn_bool selectPoint(const double x_nm, const double y_nm);

   // do the actual finding of nearest point
   int findNearestShapePoint(const double x_nm, const double y_nm,
     PatternShape* &nearestShape,
     PatternPoint* &nearestPoint,
     double &minDist);
   int findNearestPoint(list<PatternPoint*> &points,
     const double x_obj, const double y_obj,
     PatternPoint* &nearestPoint,
     double &minDist);

   void setUserMode(PE_UserMode mode);
   PE_UserMode getUserMode();

   int d_mainWinWidth, d_mainWinHeight;
   int d_navWinWidth, d_navWinHeight;

   ImageViewer *d_viewer;
   ControlPanels *d_controlPanels;
   int d_mainWinID;
   int d_navWinID;
   
   double d_scanMinX_nm;
   double d_scanMinY_nm;
   double d_scanMaxX_nm;
   double d_scanMaxY_nm;

   double d_worldMinX_nm;
   double d_worldMinY_nm;
   double d_worldMaxX_nm;
   double d_worldMaxY_nm;

   double d_mainWinMinX_nm;
   double d_mainWinMinY_nm;
   double d_mainWinMaxX_nm;
   double d_mainWinMaxY_nm;

   double d_mainWinMinXadjust_nm;
   double d_mainWinMinYadjust_nm;
   double d_mainWinMaxXadjust_nm;
   double d_mainWinMaxYadjust_nm;

   // these determine grab behavior in main window
   double d_nearDistX_pix;
   double d_nearDistY_pix;
   double d_nearDistX_nm;
   double d_nearDistY_nm;

   PE_DrawTool d_drawingTool;
   PE_UserMode d_userMode;
   double d_lineWidth_nm;
   double d_area_exposure_uCoulombs_per_square_cm;
   double d_line_exposure_pCoulombs_per_cm;

   vrpn_bool d_displaySingleImage;
   nmb_Image *d_currentSingleDisplayImage;

   double d_navDragStartX_nm;
   double d_navDragStartY_nm;
   double d_navDragEndX_nm;
   double d_navDragEndY_nm;

   double d_mainDragStartX_nm;
   double d_mainDragStartY_nm;
   double d_mainDragEndX_nm;
   double d_mainDragEndY_nm;

   vrpn_bool d_shapeInProgress;
   vrpn_bool d_pointInProgress;
   PatternShape *d_currShape;

   vrpn_bool d_shapeSelected;

   // for grabs: represents the grab offset
   // for rotations: defines the center of rotation
   double d_objectCenterX;
   double d_objectCenterY;

   // the previous rotation values
   double d_objectRotateCos;
   double d_objectRotateSin;

   // should be obvious
   // TODO: probably remove selectedPoint, make selectedShape a list of pointers
   PatternPoint *d_selectedPoint;
   PatternShape *d_selectedShape;

   list<ImageElement> d_images;
   ExposurePattern d_pattern;

   list<PatternPoint> d_exposurePoints;

   vrpn_bool d_viewportSet;

   int d_numExposurePointsRecorded;
   vrpn_bool d_exposurePointsDisplayed;

   PatternShapeColorMap d_patternColorMap;

   nmb_Image *d_canvasImage;
   nmb_Image *d_liveImage;

   Tclvar_string d_segmentLength_nm;

   double d_patternRed, d_patternGreen, d_patternBlue;

   // texture stuff
   vrpn_bool d_canvasTextureNeedsUpdate;
   vrpn_bool d_liveTextureNeedsUpdate;
   GLuint d_textureID[16];
   vrpn_bool d_allocatedTextures; // initially false
   static int s_CANVAS_TEXTURE; // = 0
   static int s_LIVE_TEXTURE; // = 1
   static int s_SHARED_TEXTURE; // = 2
   static GLsizei s_numTextures; // = 3

};

#endif

#ifndef PATTERNEDITOR_H
#define PATTERNEDITOR_H

#include "imageViewer.h"
#include "nmb_Image.h"
#include "list.h"
#include "Tcl_Linkvar.h"

class PatternElement {
  public:
   PatternElement():d_ID(s_nextID), d_lineWidth_nm(0), 
                    d_exposure_uCoulombs_per_square_cm(0) {s_nextID++;};

   operator== (const PatternElement &pe) {return (d_ID == pe.d_ID);}
   int d_ID;
   double d_lineWidth_nm;
   double d_exposure_uCoulombs_per_square_cm;
   static int s_nextID;
};

class PatternPoint : public PatternElement {
  public:
   PatternPoint():PatternElement(), d_x(0.0), d_y(0.0) {};
   operator== (const PatternPoint &ppt) {return (d_ID == ppt.d_ID);}

   double d_x, d_y;
};

class PatternPolyLine : public PatternElement {
  public:
   operator== (const PatternPolyLine &ppl) {return (d_ID == ppl.d_ID);}

   list<PatternPoint> points;
};

class ImageElement {
  public:
   ImageElement():d_red(1.0), d_green(1.0), d_blue(1.0),
                  d_opacity(1.0), d_enabled(vrpn_FALSE),d_image(NULL) {}
   ImageElement(nmb_Image *im, double r=1.0, double g=1.0, double b=1.0, 
                double o=1.0, vrpn_bool e=vrpn_FALSE):
            d_red(r), d_green(g), d_blue(b), d_opacity(o), 
            d_enabled(e), d_image(im) {}
   operator== (const ImageElement& ie) {return (d_image == ie.d_image);}
   
   double d_red;
   double d_green;
   double d_blue;
   double d_opacity;
   vrpn_bool d_enabled;
   nmb_Image *d_image;
};

class PatternEditor {
  public:
   PatternEditor();
   ~PatternEditor();
   void show();
   void newPosition(nmb_Image *im);
   void addImage(nmb_Image *im, double opacity = 1.0, 
                 double r=1.0, double g=1.0, double b=1.0);
   void removeImage(nmb_Image *im);
   void setImageEnable(nmb_Image *im, vrpn_bool displayEnable);

  protected:
   static int mainWinEventHandler(const ImageViewerWindowEvent &event, 
                                  void *ud);
   static int mainWinDisplayHandler(const ImageViewerDisplayData &data, 
                                    void *ud);
   // helper for mainWinDisplayHandler:
   void drawImage(const ImageElement &ie);

   static int navWinEventHandler(const ImageViewerWindowEvent &event,
                                 void *ud);
   static int navWinDisplayHandler(const ImageViewerDisplayData &data,
                                    void *ud);
   void navWinPositionToWorld(double x, double y,
                                       double &x_nm, double &y_nm);
   void mainWinPositionToWorld(double x, double y,
                                       double &x_nm, double &y_nm);
   void zoomBy(double centerX_nm, double centerY_nm,
                           double magFactor);

   int d_mainWinWidth, d_mainWinHeight;
   int d_navWinWidth, d_navWinHeight;

   ImageViewer *d_viewer;
   int d_mainWinID;
   int d_navWinID;
   
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

   typedef enum {IDLE, SET_REGION, SET_TRANSLATE, POLYLINE, GRAB} UserMode;
   UserMode d_userMode;

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

   list<ImageElement> d_images;
   list<PatternElement> d_pattern;

};

#endif

#ifndef PATTERNEDITOR_H
#define PATTERNEDITOR_H

#include "imageViewer.h"
#include "nmb_Image.h"
#include "list.h"
#include "Tcl_Linkvar.h"

class PatternElement {

   double d_startX_nm;
   double d_startY_nm;
   double d_endX_nm;
   double d_endY_nm;
   double d_lineWidth_nm;
   double d_exposure_uCoulombs_per_square_cm;
};


class PatternEditor {
  public:
   PatternEditor();
   ~PatternEditor();
   void show();
   void addImage(nmb_Image *im);
   void removeImage(nmb_Image *im);

  protected:
   static int mainWinEventHandler(const ImageViewerWindowEvent &event, 
                                  void *ud);
   static int mainWinDisplayHandler(const ImageViewerDisplayData &data, 
                                    void *ud);
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

   vrpn_bool d_settingRegion;
   vrpn_bool d_settingTranslation;
   double d_navDragStartX_nm;
   double d_navDragStartY_nm;
   double d_navDragEndX_nm;
   double d_navDragEndY_nm;

   double d_mainDragStartX_nm;
   double d_mainDragStartY_nm;
   double d_mainDragEndX_nm;
   double d_mainDragEndY_nm;

   list<nmb_Image *> d_images;
   list<PatternElement> d_pattern;

   // Tcl variables linked to control panels
   Tclvar_float d_lineWidth_nm;
   Tclvar_float d_exposure_uCoulombs_per_square_cm;
};

#endif

#ifndef IMAGECORRESPONDENCEEDITOR_H
#define IMAGECORRESPONDENCEEDITOR_H
/*===3rdtech===
  Copyright (c) 2001 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/

#include <imageViewer.h>
#include <vector>

#include "correspondence.h"

class nmb_ColorMap;
class PPM;
class spot_tracker_XY;
class image_wrapperAdapter;

class CorrespondenceWindowParameters {
 public:
  CorrespondenceWindowParameters();
  double left, right, bottom, top;
  nmb_Image *im;
  int winID;
  GLuint texID;
};

class SpotTrackerParameters {
public:
	SpotTrackerParameters();
	spot_tracker_XY *tracker;
	bool   invert;
	bool   optimizeRadius;
	double radiusAccuracy;
	double pixelAccuracy;
	double sampleSeparation;
};

typedef void (*CorrespondenceCallback)(Correspondence &c, void *ud);

#define POINT_SIZE (4)
/* this class is responsible for displaying and allowing the manipulation of
   a Correspondence through user interaction with windows in an
   ImageViewer
*/
class CorrespondenceEditor {
  public:
    CorrespondenceEditor(int num_im, char **win_names = NULL);
    CorrespondenceEditor(int num_im, ImageViewer *view,
                              Correspondence *corr, int *winIDs);
	virtual ~CorrespondenceEditor();
    void showAll();
	void show(int image_index);
    void hideAll();
	void hide(int image_index);
    void clearFiducials();
    void addFiducial(float *x, float *y, float *z);
    int setImage(int image_index, nmb_Image *im);
    int setImageOrientation(int image_index, vrpn_bool flipX, vrpn_bool flipY);
    int getImageOrientation(int spaceIndex, 
                            vrpn_bool &flipX, vrpn_bool &flipY);
//    int setImageFromPlane(int image_index, BCPlane *p);
//    int setImageFromPNM(int image_index, PNMImage &im);
//    int setImageFromPNM(int image_index, PPM *im);
    int setColorMap(int image_index, nmb_ColorMap * cmap);
    int setColorMinMax(int image_index,
                       vrpn_float64 dmin, vrpn_float64 dmax,
                       vrpn_float64 cmin, vrpn_float64 cmax);
	int setFiducialSpotTracker(int image_index, int tracker_type);
	int setOptimizeSpotTrackerRadius(int image_index, vrpn_bool enable);
	int setSpotTrackerRadius(int image_index, vrpn_float64 radius);
	int setSpotTrackerPixelAccuracy(int image_index, vrpn_float64 accuracy);
	int setSpotTrackerRadiusAccuracy(int image_index, vrpn_float64 accuracy);

    void mainloop();
    void getCorrespondence(Correspondence &corr);
	int numImages() {return num_images;}
    void registerCallback(CorrespondenceCallback handler, void *ud);
	void enableEdit(vrpn_bool enableAddAndDelete, 
										 vrpn_bool enableMove);

	void recenterFiducials(int spaceIndex);
	void unpaired_fluoro_recenterFiducials(int spaceIndex);
	void centerWithSpotTracker(int spaceIndex, int pointIndex);
	void unpaired_fluoro_centerWithSpotTracker(int spaceIndex, int pointIndex);
	void readAllTest(int spaceIndex, const char * filename);
	vector< vector <float> > decideOnUsingMedianFilter(int spaceIndex);
	float getWidth(int spaceIndex);
	float getHeight(int spaceIndex);
	vector< vector <float> > comparePixelsWithNeighbors(int spaceIndex, const char * filename);

//	void showMarkersInSingleImage(); // new

  private:
    // eventHandler is responsible for handling user interaction with image
    // windows
    static int eventHandler(const ImageViewerWindowEvent &event, void *ud);

    static int displayHandler(const ImageViewerDisplayData &data, void *ud);

    void notifyCallbacks();
	void drawRoundCrosshair(double x, double y, double radius, double scaleX, double scaleY);
    void drawCrosshair(float x, float y);
    void drawSelectionBox(int xp, int yp);
    int getSpaceIndex(int winID);
    int scaleImageRegion(int winID,
                          double x_win, double y_win, double scale);
    int clampImageRegion(int index);

    ImageViewer *viewer;
    Correspondence *correspondence;
    int num_images;
    CorrespondenceWindowParameters *winParam;
    int selectedPointIndex;
    int grabbedPointIndex;
	int unpaired_fluoro_selectedPointIndex;
    int unpaired_fluoro_grabbedPointIndex;
    vrpn_bool draggingPoint;
    int grab_offset_x, grab_offset_y;

    GLuint point_marker_dlist;
    CorrespondenceCallback change_handler;
    void *userdata;

	vrpn_bool enableMovingPoints;
	vrpn_bool enableAddDeletePoints;

	image_wrapperAdapter *imageAdapter; // Adapts images between nano and spot tracker lib.

	// Settings for spot trackers
	SpotTrackerParameters spotTrackerParams[2];

//	bool show_markers_in_single_image; // new

};

#endif

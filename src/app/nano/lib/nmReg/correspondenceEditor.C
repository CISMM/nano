/*===3rdtech===
  Copyright (c) 2001 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#include "correspondenceEditor.h"
#include <PPM.h>
#include <nmb_ColorMap.h>

// Include spot tracking for high resolution fiducial centering.
#include "spot_tracker.h"
#include "image_wrapperAdapter.h"

#include <algorithm>
#include <vector>

#ifndef	M_PI
#ifndef M_PI_DEFINED
const double M_PI = 2*asin(1.0);
#define M_PI_DEFINED
#endif
#endif

CorrespondenceWindowParameters::CorrespondenceWindowParameters(): 
        left(0.0), right(1.0),
        bottom(0.0), top(1.0), im(NULL), winID(-1), texID(0) {
}

SpotTrackerParameters::SpotTrackerParameters() :
        tracker(NULL), invert(false), optimizeRadius(false),
        radiusAccuracy(0.05), pixelAccuracy(0.05), sampleSeparation(1.0) {
}

CorrespondenceEditor::CorrespondenceEditor(int num_im,
                                                ImageViewer *view,
                                                Correspondence *corr,
                                                int *winIDs) {

    viewer = view;
    correspondence = corr;
    // set up the mapping from image numbers in correspondence to windows in
    // viewer and set up callbacks for events and display
    num_images = num_im;
    winParam = new CorrespondenceWindowParameters[num_im];
    int i;
    for (i = 0; i < num_images; i++){
        winParam[i].winID = winIDs[i];
        viewer->setWindowEventHandler(winParam[i].winID,
                CorrespondenceEditor::eventHandler, (void *)this);
        viewer->setWindowDisplayHandler(winParam[i].winID,
                CorrespondenceEditor::displayHandler, (void *)this);
    }
    selectedPointIndex = 0;
    grabbedPointIndex = 0;
	unpaired_fluoro_selectedPointIndex = 0;
    unpaired_fluoro_grabbedPointIndex = 0;
    draggingPoint = VRPN_FALSE;
    grab_offset_x = 0;
    grab_offset_y = 0;
    point_marker_dlist = 0;
    change_handler = NULL;
    enableMovingPoints = vrpn_TRUE;
    enableAddDeletePoints = vrpn_TRUE;
    imageAdapter = new image_wrapperAdapter();
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
    viewer->init(display_name);
    num_images = num_im;
    winParam = new CorrespondenceWindowParameters[num_images];
    int i;
    for (i = 0; i < num_images; i++){
        if (!win_names) {
            sprintf(win_name, "data_registration%d", i);
        } else {
            sprintf(win_name, "%s", win_names[i]);
        }
        winParam[i].winID = 
            viewer->createWindow(display_name,i*110 + 200,200,
                                 400,400,win_name);
        viewer->setWindowEventHandler(winParam[i].winID,
                CorrespondenceEditor::eventHandler, (void *)this);
        viewer->setWindowDisplayHandler(winParam[i].winID,
                CorrespondenceEditor::displayHandler, (void *)this);
    }
    selectedPointIndex = 0;
    grabbedPointIndex = 0;
	unpaired_fluoro_selectedPointIndex = 0;
    unpaired_fluoro_grabbedPointIndex = 0;
    draggingPoint = VRPN_FALSE;
    grab_offset_x = 0;
    grab_offset_y = 0;
    point_marker_dlist = 0;
    change_handler = NULL;
    enableMovingPoints = vrpn_TRUE;
    enableAddDeletePoints = vrpn_TRUE;
    imageAdapter = new image_wrapperAdapter();
}

CorrespondenceEditor::~CorrespondenceEditor() {
    for (int i = 0; i < 2; i++) {
        if (spotTrackerParams[i].tracker) delete spotTrackerParams[i].tracker;
    }
    delete imageAdapter;
}

// here is where user interaction is defined:
int CorrespondenceEditor::eventHandler(
                        const ImageViewerWindowEvent &event, void *ud)
{
    CorrespondenceEditor *me = (CorrespondenceEditor *)ud;

    // map the windowID to a spaceIndex:
    int spaceIndex= me->getSpaceIndex(event.winID);

    double scaleX = 2.0, scaleY = 2.0;
    me->viewer->toImageVec(event.winID, &scaleX, &scaleY);
    scaleX = fabs(scaleX); // convert to absolute scale
    scaleY = fabs(scaleY);

    switch(event.type) {
      case RESIZE_EVENT:
          break;
      case BUTTON_PRESS_EVENT:
          if (event.button == IV_LEFT_BUTTON){
              double x_im = event.mouse_x, y_im = event.mouse_y;
              me->viewer->toImagePnt(event.winID, &x_im, &y_im);
              // Inside or outside existing point?
              if (me->correspondence->findNearestPoint(spaceIndex, x_im, y_im,
                  scaleX, scaleY, &(me->grabbedPointIndex))) {
                      // We are inside- prepare to drag this point
                      corr_point_t grabbed_pnt;
                      if (me->correspondence->getPoint(spaceIndex, 
                          me->grabbedPointIndex, &grabbed_pnt)) {
                              fprintf(stderr, "CorrespondenceEditor::eventHandler: "
                                  "getPoint failed\n");
                              return -1;
                      }
                      me->draggingPoint = VRPN_TRUE;
                      me->viewer->toPixelsPnt(event.winID, &(grabbed_pnt.x),
                          &(grabbed_pnt.y));
                      me->grab_offset_x = (int)(grabbed_pnt.x - event.mouse_x);
                      me->grab_offset_y = (int)(grabbed_pnt.y - event.mouse_y);
                      me->selectedPointIndex = me->grabbedPointIndex;
                      int i;
                      for (i = 0; i < me->num_images; i++)
                          me->viewer->dirtyWindow((me->winParam)[i].winID);
              } else if (me->enableAddDeletePoints) {
                  // We are outside, create a new point. 
                  double x_im = event.mouse_x, y_im = event.mouse_y;
                  me->viewer->toImagePnt(event.winID, &x_im, &y_im);

                  corr_point_t p(x_im, y_im);
                  int new_pntIdx = me->correspondence->addPoint(p);
                  me->selectedPointIndex = me->grabbedPointIndex = new_pntIdx;
                  me->centerWithSpotTracker(spaceIndex, me->selectedPointIndex);

                  // Also prepare to drag this new point. 
                  me->draggingPoint = VRPN_TRUE;
                  me->viewer->toPixelsPnt(event.winID, &(p.x),
                      &(p.y));
                  me->grab_offset_x = 0;
                  me->grab_offset_y = 0;
                  int i;

				  // new outer if statement
				  //if (show_markers_in_single_image == false)
				 // {
	                 for (i = 0; i < me->num_images; i++) {
		                  me->viewer->dirtyWindow((me->winParam)[i].winID);
			          }
				 // }
                  me->notifyCallbacks();
              }
          }
		  else if (event.button == IV_RIGHT_BUTTON){
			  if(spaceIndex == 1)
			  {
				  double x_im = event.mouse_x, y_im = event.mouse_y;
				  me->viewer->toImagePnt(event.winID, &x_im, &y_im);
				  // Inside or outside existing point?
				  if (me->correspondence->unpaired_fluoro_findNearestPoint(spaceIndex, x_im, y_im,
					  scaleX, scaleY, &(me->unpaired_fluoro_grabbedPointIndex))) {
						  // We are inside- prepare to drag this point
						  corr_point_t grabbed_pnt;
						  if (me->correspondence->unpaired_fluoro_getPoint(spaceIndex, 
							  me->unpaired_fluoro_grabbedPointIndex, &grabbed_pnt)) {
								  fprintf(stderr, "CorrespondenceEditor::eventHandler: "
									  "getPoint failed\n");
								  return -1;
						  }
						  me->draggingPoint = VRPN_TRUE;
						  me->viewer->toPixelsPnt(event.winID, &(grabbed_pnt.x),
							  &(grabbed_pnt.y));
						  me->grab_offset_x = (int)(grabbed_pnt.x - event.mouse_x);
						  me->grab_offset_y = (int)(grabbed_pnt.y - event.mouse_y);
						  me->unpaired_fluoro_selectedPointIndex = me->unpaired_fluoro_grabbedPointIndex;
						  int i;
						  for (i = 0; i < me->num_images; i++)
							  me->viewer->dirtyWindow((me->winParam)[i].winID);
				  } else if (me->enableAddDeletePoints) {
					  // We are outside, create a new point. 
					  double x_im = event.mouse_x, y_im = event.mouse_y;
					  me->viewer->toImagePnt(event.winID, &x_im, &y_im);

					  corr_point_t p(x_im, y_im);
					  int new_pntIdx = me->correspondence->unpaired_fluoro_addPoint(p);
					  me->unpaired_fluoro_selectedPointIndex = me->unpaired_fluoro_grabbedPointIndex = new_pntIdx;
					  me->unpaired_fluoro_centerWithSpotTracker(spaceIndex, me->unpaired_fluoro_selectedPointIndex);

					  // Also prepare to drag this new point. 
					  me->draggingPoint = VRPN_TRUE;
					  me->viewer->toPixelsPnt(event.winID, &(p.x),
						  &(p.y));
					  me->grab_offset_x = 0;
					  me->grab_offset_y = 0;
					  int i;

					  // new outer if statement
					  //if (show_markers_in_single_image == false)
					 // {
						 for (i = 0; i < me->num_images; i++) {
							  me->viewer->dirtyWindow((me->winParam)[i].winID);
						  }
					 // }
					  me->notifyCallbacks();
				  }
			  }
          }
          break;
      case BUTTON_RELEASE_EVENT:
          if (event.button == IV_LEFT_BUTTON && me->draggingPoint) {
              double x_im = event.mouse_x + me->grab_offset_x;
              double y_im = event.mouse_y + me->grab_offset_y;
              // allow a 10 pixel tolerance for dragging outside the window to
              // make it easier for the user to set a point exactly on the 
              // border of the image without deleting it by accident
              if(me->viewer->clampToWindow(event.winID, &x_im, &y_im) > 10 &&
                  me->enableAddDeletePoints) {
                      // If we dragged outside the window, delete the point. 
                      me->correspondence->deletePoint(me->selectedPointIndex);
                      me->selectedPointIndex = me->correspondence->numPoints()-1;
                      // deleted point affects all windows. 
                      int i;
                      for (i = 0; i < me->num_images; i++) {
                          me->viewer->dirtyWindow((me->winParam)[i].winID);
                      }
                      me->draggingPoint = VRPN_FALSE;
                      me->notifyCallbacks();
              } else if (me->enableMovingPoints) {
                  me->viewer->toImagePnt(event.winID, &x_im, &y_im);

                  corr_point_t p;
                  me->correspondence->getPoint(spaceIndex, me->grabbedPointIndex, &p);
                  p.x = x_im;
                  p.y = y_im;
                  me->correspondence->setPoint(spaceIndex, me->grabbedPointIndex, p);
                  me->centerWithSpotTracker(spaceIndex, me->grabbedPointIndex);

                  // moved point only affects active window. 
                  me->viewer->dirtyWindow(event.winID);
                  me->draggingPoint = VRPN_FALSE;
                  me->notifyCallbacks();
              }
          }
		  else if (event.button == IV_RIGHT_BUTTON && me->draggingPoint && spaceIndex == 1) {
              double x_im = event.mouse_x + me->grab_offset_x;
              double y_im = event.mouse_y + me->grab_offset_y;
              // allow a 10 pixel tolerance for dragging outside the window to
              // make it easier for the user to set a point exactly on the 
              // border of the image without deleting it by accident
              if(me->viewer->clampToWindow(event.winID, &x_im, &y_im) > 10 &&
                  me->enableAddDeletePoints) {
                      // If we dragged outside the window, delete the point. 
                      me->correspondence->unpaired_fluoro_deletePoint(me->unpaired_fluoro_selectedPointIndex);
                      me->unpaired_fluoro_selectedPointIndex = me->correspondence->unpaired_fluoro_numPoints()-1;
                      // deleted point affects all windows. 
                      int i;
                      for (i = 0; i < me->num_images; i++) {
                          me->viewer->dirtyWindow((me->winParam)[i].winID);
                      }
                      me->draggingPoint = VRPN_FALSE;
                      me->notifyCallbacks();
              } else if (me->enableMovingPoints) {
                  me->viewer->toImagePnt(event.winID, &x_im, &y_im);

                  corr_point_t p;
                  me->correspondence->unpaired_fluoro_getPoint(spaceIndex, me->unpaired_fluoro_grabbedPointIndex, &p);
                  p.x = x_im;
                  p.y = y_im;
                  me->correspondence->unpaired_fluoro_setPoint(spaceIndex, me->unpaired_fluoro_grabbedPointIndex, p);
                  me->unpaired_fluoro_centerWithSpotTracker(spaceIndex, me->unpaired_fluoro_grabbedPointIndex);

                  // moved point only affects active window. 
                  me->viewer->dirtyWindow(event.winID);
                  me->draggingPoint = VRPN_FALSE;
                  me->notifyCallbacks();
              }
          }
          break;
      case MOTION_EVENT:
          if (me->enableMovingPoints) {
              if ((event.state & IV_LEFT_BUTTON_MASK) && me->draggingPoint) {
                  double x_im = event.mouse_x + me->grab_offset_x;
                  double y_im = event.mouse_y + me->grab_offset_y;
                  me->viewer->toImagePnt(event.winID, &x_im, &y_im);		

                  corr_point_t p;
                  me->correspondence->getPoint(spaceIndex, me->grabbedPointIndex, &p);
                  p.x = x_im;
                  p.y = y_im;
                  me->correspondence->setPoint(spaceIndex, me->grabbedPointIndex, p);
                  me->centerWithSpotTracker(spaceIndex, me->grabbedPointIndex);
                  
                  me->viewer->dirtyWindow(event.winID);
                  me->notifyCallbacks();
              }
			  else if ((event.state & IV_RIGHT_BUTTON_MASK) && me->draggingPoint && spaceIndex == 1) {
                  double x_im = event.mouse_x + me->grab_offset_x;
                  double y_im = event.mouse_y + me->grab_offset_y;
                  me->viewer->toImagePnt(event.winID, &x_im, &y_im);		

                  corr_point_t p;
                  me->correspondence->unpaired_fluoro_getPoint(spaceIndex, me->unpaired_fluoro_grabbedPointIndex, &p);
                  p.x = x_im;
                  p.y = y_im;
                  me->correspondence->unpaired_fluoro_setPoint(spaceIndex, me->unpaired_fluoro_grabbedPointIndex, p);
                  me->unpaired_fluoro_centerWithSpotTracker(spaceIndex, me->unpaired_fluoro_grabbedPointIndex);
                  
                  me->viewer->dirtyWindow(event.winID);
                  me->notifyCallbacks();
              }
          }
          break;
      case KEY_PRESS_EVENT:
          if (event.keycode == 8) // 8 is for backspace, ^H.
		  { // this is for regular markers that were previously put with left button
                  if (me->enableAddDeletePoints) {
                      if (me->correspondence->numPoints() > 0){
                          me->correspondence->deletePoint(me->selectedPointIndex);
                      }
                      me->selectedPointIndex = me->correspondence->numPoints()-1;
                      int i;
                      for (i = 0; i < me->num_images; i++) {
                          me->viewer->dirtyWindow((me->winParam)[i].winID);
                      }
                      // Allow update of the registration automatically. 
                      me->notifyCallbacks();
                  }
		  }
		  else if (event.keycode == 127) // 127 for delete key. Delete key needs glut 3.7 or greater.
		  {  // this is for unpaired regular markers that were previously put with right button
                  if (me->enableAddDeletePoints) {
                      if (me->correspondence->unpaired_fluoro_numPoints() > 0){
                          me->correspondence->unpaired_fluoro_deletePoint(me->unpaired_fluoro_selectedPointIndex);
                      }
                      me->unpaired_fluoro_selectedPointIndex = me->correspondence->unpaired_fluoro_numPoints()-1;
                      int i;
                      for (i = 0; i < me->num_images; i++) {
                          me->viewer->dirtyWindow((me->winParam)[i].winID);
                      }
                      // Allow update of the registration automatically. 
                      me->notifyCallbacks();
                  }
		  }
		  else if (event.keycode == 'z') {
              me->scaleImageRegion(event.winID, event.mouse_x, event.mouse_y, 0.5);
              me->viewer->dirtyWindow(event.winID);
          } else if (event.keycode == 'Z') {
              me->scaleImageRegion(event.winID, event.mouse_x, event.mouse_y, 2.0);
              me->viewer->dirtyWindow(event.winID);
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
        if (winParam[i].winID == winID) break;
    }
    if (i == num_images) return -1;
    else
        return i;
}

int CorrespondenceEditor::scaleImageRegion(int winID, 
                                           double x_win, double y_win, double scale) {
    int index = getSpaceIndex(winID);
    if (index < 0) return -1;

    double x_im = x_win, y_im = y_win;
    if (viewer->toImagePnt(winID, winParam[index].left, winParam[index].right,
        winParam[index].bottom, winParam[index].top,
        &x_im, &y_im)) {
            return -1;
    }

    winParam[index].left = x_im + scale*(winParam[index].left - x_im);
    winParam[index].right = x_im + scale*(winParam[index].right - x_im);
    winParam[index].bottom = y_im + scale*(winParam[index].bottom - y_im);
    winParam[index].top = y_im + scale*(winParam[index].top - y_im);

    // the following is to ensure that the region falls inside (0..1,0..1) 
    // while preserving the scale if possible
    clampImageRegion(index);
    return 0;
}

int CorrespondenceEditor::clampImageRegion(int index) {
    vrpn_bool xFlipped, yFlipped;
    getImageOrientation(index, xFlipped, yFlipped);

    if ((fabs(winParam[index].right - winParam[index].left) > 1.0) ||
        (fabs(winParam[index].top - winParam[index].bottom) > 1.0)) {
            if (xFlipped) {
                winParam[index].left = 1.0;
                winParam[index].right = 0.0;
            } else {
                winParam[index].left = 0.0;
                winParam[index].right = 1.0;
            }
            if (yFlipped) {
                winParam[index].bottom = 1.0;
                winParam[index].top = 1.0;
            } else {
                winParam[index].bottom = 0;
                winParam[index].top = 1.0;
            }
    }
    if (xFlipped) {
        if (winParam[index].right < 0) {
            winParam[index].left -= winParam[index].right;
            winParam[index].right = 0.0;
        } else if (winParam[index].left > 1) {
            winParam[index].right += 1.0 - winParam[index].left;
            winParam[index].left = 1.0;
        }
    } else {
        if (winParam[index].left < 0) {
            winParam[index].right -= winParam[index].left;
            winParam[index].left = 0.0;
        } else if (winParam[index].right > 1) {
            winParam[index].left += 1.0 - winParam[index].right;
            winParam[index].right = 1.0;
        }
    }
    if (yFlipped) {
        if (winParam[index].top < 0) {
            winParam[index].bottom -= winParam[index].top;
            winParam[index].top = 0.0;
        } else if (winParam[index].bottom > 1) {
            winParam[index].top += 1.0 - winParam[index].bottom;
            winParam[index].bottom = 1.0;
        }
    } else {
        if (winParam[index].bottom < 0) {
            winParam[index].top -= winParam[index].bottom;
            winParam[index].bottom = 0.0;
        } else if (winParam[index].top > 1) {
            winParam[index].bottom += 1.0 - winParam[index].top;
            winParam[index].top = 1.0;
        }
    }
    return 0;
}

void CorrespondenceEditor::centerWithSpotTracker(int spaceIndex, int pointIndex) {
    if (!spotTrackerParams[spaceIndex].tracker) {
        printf("Tracking off for space index %d\n", spaceIndex);
        return;
    }

    corr_point_t spot;
    correspondence->getPoint(spaceIndex, pointIndex, &spot);

    nmb_Image *image = this->winParam[spaceIndex].im;
    imageAdapter->setImage(image);

    // Convert to pixel coordinates from image coordinates.
    double width = image->width();
    double height = image->height();
    double imageI = (width-1) * spot.x;
    double imageJ = (height-1) * spot.y;
    printf("Starting position = %lf, %lf\n", imageI, imageJ);
    if (imageAdapter->read_pixel_nocheck(imageI, imageJ) > 0.0)
        printf("%f ********************************************************************\n", imageAdapter->read_pixel_nocheck(imageI, imageJ));

    spotTrackerParams[spaceIndex].tracker->set_radius(spot.radius);
    spotTrackerParams[spaceIndex].tracker->set_invert(spotTrackerParams[spaceIndex].invert);
    spotTrackerParams[spaceIndex].tracker->set_radius_accuracy(spotTrackerParams[spaceIndex].radiusAccuracy);
    spotTrackerParams[spaceIndex].tracker->set_pixel_accuracy(spotTrackerParams[spaceIndex].pixelAccuracy);
    spotTrackerParams[spaceIndex].tracker->set_sample_separation(spotTrackerParams[spaceIndex].sampleSeparation);

    printf("Starting radius to %f\n", spot.radius);

    double opt_x, opt_y;
    if (spotTrackerParams[spaceIndex].optimizeRadius) {
        spotTrackerParams[spaceIndex].tracker->optimize(*imageAdapter, 0, opt_x, opt_y, imageI, imageJ);
        // Store radius.
        spot.radius = spotTrackerParams[spaceIndex].tracker->get_radius();

    } else {
        spotTrackerParams[spaceIndex].tracker->optimize_xy(*imageAdapter, 0, opt_x, opt_y, imageI, imageJ);
    }

    printf("Ending radius: %lf\n", spotTrackerParams[spaceIndex].tracker->get_radius());
    printf("Ending position   = %lf, %lf\n", spotTrackerParams[spaceIndex].tracker->get_x(), 
        spotTrackerParams[spaceIndex].tracker->get_y());

    // Convert from pixel coordinates in the image back to image coordinates.
    spot.x = opt_x / (width-1);
    spot.y = opt_y / (height-1);
    correspondence->setPoint(spaceIndex, pointIndex, spot);
}

void CorrespondenceEditor::unpaired_fluoro_centerWithSpotTracker(int spaceIndex, int pointIndex) {
    if (!spotTrackerParams[spaceIndex].tracker) {
        printf("Tracking off for space index %d\n", spaceIndex);
        return;
    }

    corr_point_t spot;
    correspondence->unpaired_fluoro_getPoint(spaceIndex, pointIndex, &spot);

    nmb_Image *image = this->winParam[spaceIndex].im;
    imageAdapter->setImage(image);

    // Convert to pixel coordinates from image coordinates.
    double width = image->width();
    double height = image->height();
    double imageI = (width-1) * spot.x;
    double imageJ = (height-1) * spot.y;
    printf("Starting position = %lf, %lf\n", imageI, imageJ);
    if (imageAdapter->read_pixel_nocheck(imageI, imageJ) > 0.0)
        printf("%f ********************************************************************\n", imageAdapter->read_pixel_nocheck(imageI, imageJ));

    spotTrackerParams[spaceIndex].tracker->set_radius(spot.radius);
    spotTrackerParams[spaceIndex].tracker->set_invert(spotTrackerParams[spaceIndex].invert);
    spotTrackerParams[spaceIndex].tracker->set_radius_accuracy(spotTrackerParams[spaceIndex].radiusAccuracy);
    spotTrackerParams[spaceIndex].tracker->set_pixel_accuracy(spotTrackerParams[spaceIndex].pixelAccuracy);
    spotTrackerParams[spaceIndex].tracker->set_sample_separation(spotTrackerParams[spaceIndex].sampleSeparation);

    printf("Starting radius to %f\n", spot.radius);

    double opt_x, opt_y;
    if (spotTrackerParams[spaceIndex].optimizeRadius) {
        spotTrackerParams[spaceIndex].tracker->optimize(*imageAdapter, 0, opt_x, opt_y, imageI, imageJ);
        // Store radius.
        spot.radius = spotTrackerParams[spaceIndex].tracker->get_radius();

    } else {
        spotTrackerParams[spaceIndex].tracker->optimize_xy(*imageAdapter, 0, opt_x, opt_y, imageI, imageJ);
    }

    printf("Ending radius: %lf\n", spotTrackerParams[spaceIndex].tracker->get_radius());
    printf("Ending position   = %lf, %lf\n", spotTrackerParams[spaceIndex].tracker->get_x(), 
        spotTrackerParams[spaceIndex].tracker->get_y());

    // Convert from pixel coordinates in the image back to image coordinates.
    spot.x = opt_x / (width-1);
    spot.y = opt_y / (height-1);
    correspondence->unpaired_fluoro_setPoint(spaceIndex, pointIndex, spot);
}

/*void CorrespondenceEditor::showMarkersInSingleImage()
{
	show_markers_in_single_image = true;
}*/ // new

int CorrespondenceEditor::displayHandler(
    const ImageViewerDisplayData &data, void *ud) {
    CorrespondenceEditor *me = (CorrespondenceEditor *)ud;

    glClearColor(0.0, 0.0, 0.9,0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // draw the image for this window:
    // but first figure out which image is displayed in this window
    int spaceIndex = me->getSpaceIndex(data.winID);

    glViewport(0,0,data.winWidth, data.winHeight);
    nmb_TransformMatrix44 W2I; // assumed to be initialized as identity
    CorrespondenceWindowParameters *params = &(me->winParam[spaceIndex]);
    double left, right, bottom, top;
    left = params->left;
    right = params->right;
    bottom = params->bottom;
    top = params->top;

    me->viewer->drawImage(data.winID, params->im,
            1.0, 1.0, 1.0, 1.0, 
            &left, &right, &bottom, &top, 
            &W2I);

//    me->viewer->drawImage(data.winID);   

    int i;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // set up projection so we can draw on top of image such that
    // vertices are in image coordinates scaled by the window size relative
    // to the image size
    glOrtho(data.winWidth, 0, data.winHeight, 0, -1, 1);
    glPixelZoom(1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glColor3f(1.0, 1.0, 1.0);
    corr_point_t image_pnt;

    char num_str[16];

    int num_pnts = me->correspondence->numPoints();
    for (i = 0; i < num_pnts; i++){
        me->correspondence->getPoint(spaceIndex, i, &image_pnt);
        // convert point to the right location in the window by scaling it
        me->viewer->toPixelsPnt(data.winID, &(image_pnt.x), &(image_pnt.y));

        // Now set the color and draw the tracker icon.
        if (i == me->selectedPointIndex)
            glColor3f(1.0f, 0.0f, 0.0f);
        else
            glColor3f(0.0f, 0.0f, 1.0f);
        double scaleX = 1.0, scaleY = 1.0;
        if (me->winParam[spaceIndex].im) {
            scaleX = (double) data.winWidth / (double) me->winParam[spaceIndex].im->width();
            scaleY = (double) data.winHeight / (double) me->winParam[spaceIndex].im->height();
        }
        me->drawRoundCrosshair(data.winWidth-image_pnt.x, image_pnt.y, image_pnt.radius, scaleX, scaleY);

        // Draw label number.
        sprintf(num_str, "%d", i);
        glColor3f(1.0, 1.0, 1.0);
        me->viewer->drawString(data.winWidth-image_pnt.x - 1 - 1.414*image_pnt.radius, 
            image_pnt.y - 3 - 1.414*image_pnt.radius, num_str);
    }

	if(spaceIndex == 1)
	{
		corr_point_t unpaired_fluoro_image_pnt;

		char unpaired_fluoro_num_str[16];
		
		int unpaired_fluoro_num_pnts = me->correspondence->unpaired_fluoro_numPoints();
		for (i = 0; i < unpaired_fluoro_num_pnts; i++){
			me->correspondence->unpaired_fluoro_getPoint(spaceIndex, i, &unpaired_fluoro_image_pnt);
			// convert point to the right location in the window by scaling it
			me->viewer->toPixelsPnt(data.winID, &(unpaired_fluoro_image_pnt.x), &(unpaired_fluoro_image_pnt.y));

			// Now set the color and draw the tracker icon.
			if (i == me->unpaired_fluoro_selectedPointIndex)
				glColor3f(1.0f, 0.0f, 0.0f);
			else
				glColor3f(0.0f, 0.0f, 1.0f);
			double scaleX = 1.0, scaleY = 1.0;
			if (me->winParam[spaceIndex].im) {
				scaleX = (double) data.winWidth / (double) me->winParam[spaceIndex].im->width();
				scaleY = (double) data.winHeight / (double) me->winParam[spaceIndex].im->height();
			}
			me->drawRoundCrosshair(data.winWidth-unpaired_fluoro_image_pnt.x, unpaired_fluoro_image_pnt.y, unpaired_fluoro_image_pnt.radius, scaleX, scaleY);

			// Draw label number.
			sprintf(unpaired_fluoro_num_str, "%d", i);
			glColor3f(1.0, 1.0, 1.0);
			me->viewer->drawString(data.winWidth-unpaired_fluoro_image_pnt.x - 1 - 1.414*unpaired_fluoro_image_pnt.radius, 
				unpaired_fluoro_image_pnt.y - 3 - 1.414*unpaired_fluoro_image_pnt.radius, unpaired_fluoro_num_str);
		}
	}
    return 0;
}

void CorrespondenceEditor::notifyCallbacks() {
    Correspondence copy;
    getCorrespondence(copy);
    if (change_handler) {
        change_handler(copy, userdata);
    }
}

void CorrespondenceEditor::drawRoundCrosshair(double x, double y, double radius, double scaleX, double scaleY) {
    // I'm confused why this scaling is necessary. It shouldn't be.
    x *= 0.5;
    y *= 0.5;

    glPushAttrib(GL_VIEWPORT_BIT);
    glPushMatrix();
    glTranslatef(x+0.5, y+0.5, 0.0);
    //glTranslatef(x, y, 0.0f);

    // Use smooth lines here to avoid aliasing showing spot in wrong place
    glEnable (GL_BLEND);
    glEnable(GL_LINE_SMOOTH);
    glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);
    glLineWidth(1.0);

    double dx = scaleX * radius;
    double dy = scaleY * radius;

    // First, make a ring that is twice the radius so that it does not obscure the border.
    double stepsize = M_PI / radius;
    double runaround;
    glBegin(GL_LINE_STRIP); {
        for (runaround = 0; runaround <= 2*M_PI; runaround += stepsize) {
            glVertex2f(x + 2*dx*cos(runaround),y + 2*dy*sin(runaround));
        }
        glVertex2f(x + 2*dx, y);  // Close the circle
    } glEnd();

    // Then, make four lines coming from the cirle in to the radius of the spot
    // so we can tell what radius we have set
    glBegin(GL_LINES); {
        glVertex2f(x+dx,y); glVertex2f(x+2*dx,y);
        glVertex2f(x,y+dy); glVertex2f(x,y+2*dy);
        glVertex2f(x-dx,y); glVertex2f(x-2*dx,y);
        glVertex2f(x,y-dy); glVertex2f(x,y-2*dy);
    } glEnd();

    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_BLEND);
    glPopMatrix();
    glPopAttrib();
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

void CorrespondenceEditor::showAll() {
    int i;
    for (i = 0; i < num_images; i++){
        viewer->showWindow(winParam[i].winID);
    }
}

void CorrespondenceEditor::show(int spaceIndex) {
    viewer->showWindow(winParam[spaceIndex].winID);
}

void CorrespondenceEditor::hideAll() {
    int i;
    for (i = 0; i < num_images; i++){
        viewer->hideWindow(winParam[i].winID);
    }
}

void CorrespondenceEditor::hide(int spaceIndex) {
    viewer->hideWindow(winParam[spaceIndex].winID);
}

void CorrespondenceEditor::clearFiducials()
{
    correspondence->clear();
    int i;
    for (i = 0; i < num_images; i++) {
        viewer->dirtyWindow((winParam)[i].winID);
    }
}

/** 
   x, y are in the range 0..1 and z is in nanometers

*/
void CorrespondenceEditor::addFiducial(float *x, float *y, float *z)
{
    corr_point_t p;
    if (z) {
        p.is2D = vrpn_FALSE;
    }
    int pntIndex = correspondence->addPoint(p);
    int i;
    if (pntIndex >= 0) {
        for (i = 0; i < num_images; i++) {
            p.x = x[i]; p.y = y[i]; 
            if (z) {
                p.z = z[i];
            }
            correspondence->setPoint(i, pntIndex, p);
        }
    }

    for (i = 0; i < num_images; i++) {
        viewer->dirtyWindow((winParam)[i].winID);
    }
}

int CorrespondenceEditor::setImage(int spaceIndex, nmb_Image *im) {
    int width = 100, height = 100;
    //int i,j;
    int im_w = 100, im_h = 100;
    //double val;

    if (winParam[spaceIndex].im) {
        nmb_Image::deleteImage(winParam[spaceIndex].im);
        winParam[spaceIndex].im = NULL;
    }
    if (im) {
        winParam[spaceIndex].im = new nmb_ImageGrid(im);
        winParam[spaceIndex].im->normalize();
        height = (width*im->height())/(double)(im->width());
        im_w = im->width();
        im_h = im->height();
    }

//    viewer->setWindowSize(winParam[spaceIndex].winID, width, height);
/*
    viewer->setWindowImageSize(winParam[spaceIndex].winID, im_w, im_h);
    // printf("CorrespondenceEditor::setImage: \n");
    viewer->setValueRange(winParam[spaceIndex].winID, 
                          im->minNonZeroValue(), im->maxValue());
    // printf(" min,max = %f, %f\n", im->minNonZeroValue(), im->maxValue());

    for (i = 0; i < im_w; i++){
        for (j = 0; j < im_h; j++){
            //val = im->getValue(i,im_h -j -1);   // we need to flip y
            val = im->getValue(i,j);
            viewer->setValue(winParam[spaceIndex].winID, i, j, val);
        }
    }
*/

    viewer->dirtyWindow(winParam[spaceIndex].winID);
    return 0;
}

int CorrespondenceEditor::setImageOrientation(int spaceIndex, 
                                              vrpn_bool flipX, vrpn_bool flipY) {
    double temp;
    if (( flipX && (winParam[spaceIndex].left < winParam[spaceIndex].right)) ||
        (!flipX && (winParam[spaceIndex].left > winParam[spaceIndex].right))) {
            temp = winParam[spaceIndex].left;
            winParam[spaceIndex].left = winParam[spaceIndex].right;
            winParam[spaceIndex].right = temp;
    }

    if (( flipY && (winParam[spaceIndex].bottom < winParam[spaceIndex].top)) ||
        (!flipY && (winParam[spaceIndex].bottom > winParam[spaceIndex].top))) {
            temp = winParam[spaceIndex].bottom;
            winParam[spaceIndex].bottom = winParam[spaceIndex].top;
            winParam[spaceIndex].top = temp;
    }

    viewer->dirtyWindow(winParam[spaceIndex].winID);
    return 0;
}

int CorrespondenceEditor::getImageOrientation(int spaceIndex,
                                              vrpn_bool &flipX, vrpn_bool &flipY) {
    flipX = (winParam[spaceIndex].left > winParam[spaceIndex].right);
    flipY = (winParam[spaceIndex].bottom > winParam[spaceIndex].top);
    return 0;
}

/* ************************************
int CorrespondenceEditor::setImageFromPlane(int spaceIndex, BCPlane *p) {
    int im_w, im_h;
    int i,j;
    double val;

    im_w = p->numX();
    im_h = p->numY();
    viewer->setWindowImageSize(winParam[spaceIndex].winID, im_w, im_h);
    viewer->setValueRange(winParam[spaceIndex].winID, 
                          p->minValue(), p->maxValue());
    for (i = 0; i < im_w; i++){
        for (j = 0; j < im_h; j++){
            //val = p->value(i,im_h -j -1);	// we need to flip y
            val = p->value(i,j);
            viewer->setValue(winParam[spaceIndex].winID, i, j, val);
        }
    }
    viewer->setWindowSize(win_ids[spaceIndex], im_w, im_h);
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
    viewer->setWindowImageSize(winParam[spaceIndex].winID, im_w, im_h);
    viewer->setValueRange(winParam[spaceIndex].winID, im_min, im_max);
    for (i = 0; i < im_w; i++){
        for (j = 0; j < im_h; j++){
            val = im.Pixel(j, i, 0);
            viewer->setValue(winParam[spaceIndex].winID, i, j, val);
        }
    }
    viewer->setWindowSize(win_ids[spaceIndex], im_w, im_h);
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
    viewer->setWindowImageSize(winParam[spaceIndex].winID, im_w, im_h);
    viewer->setValueRange(winParam[spaceIndex].winID, im_min, im_max);
    for (i = 0; i < im_w; i++){
        for (j = 0; j < im_h; j++){
            im->Tellppm(i, j, &r, &g, &b);  // flip y??
            val = r;
            viewer->setValue(winParam[spaceIndex].winID, i, j, val);
        }
    }
    viewer->setWindowSize(win_ids[spaceIndex], im_w, im_h);
    return 0;
}
************************************************ */

void CorrespondenceEditor::mainloop() {
    if (viewer) viewer->mainloop();
}

// this returns a copy of the current correspondence
void CorrespondenceEditor::getCorrespondence(Correspondence &corr) {
    corr = (*correspondence);
}

void CorrespondenceEditor::registerCallback(CorrespondenceCallback handler,
                                            void *ud) {
    change_handler = handler;
    userdata = ud;
}

int CorrespondenceEditor::setColorMap(int spaceIndex, nmb_ColorMap * cmap) {
    viewer->setColorMap(winParam[spaceIndex].winID, cmap);
    viewer->dirtyWindow(winParam[spaceIndex].winID);
    return 0;
}

int CorrespondenceEditor::setColorMinMax(int spaceIndex, 
                       vrpn_float64 dmin, vrpn_float64 dmax,
                       vrpn_float64 cmin, vrpn_float64 cmax) {
    viewer->setColorMinMax(winParam[spaceIndex].winID, dmin, dmax, cmin, cmax);
    viewer->dirtyWindow(winParam[spaceIndex].winID);
    return 0;
}

void CorrespondenceEditor::enableEdit(vrpn_bool addAndDelete, 
                                         vrpn_bool move) {
    enableAddDeletePoints = addAndDelete;
    enableMovingPoints = move;
}

int CorrespondenceEditor::setFiducialSpotTracker(int image_index, int tracker_type) {
    if (spotTrackerParams[image_index].tracker) delete spotTrackerParams[image_index].tracker;
    switch (tracker_type) {
        case 0:
            printf("Setting spot tracker to None\n");
            spotTrackerParams[image_index].tracker = NULL;
            break;

        case 1:
            printf("Setting spot tracker to local max\n");
            spotTrackerParams[image_index].tracker = new local_max_spot_tracker(1.0); // Replace with maximizer tracker
            break;

        case 2:
            printf("Setting spot tracker to cone\n");
            spotTrackerParams[image_index].tracker = new cone_spot_tracker_interp(1.0);
            break;

        case 3:
            printf("Setting spot tracker to disk\n");
            spotTrackerParams[image_index].tracker = new disk_spot_tracker_interp(1.0);
            break;

        case 4:
            printf("Setting spot tracker to FIONA\n");
            spotTrackerParams[image_index].tracker = new FIONA_spot_tracker(1.0); // Replace OPT_RADIUS
            break;

        case 5:
            printf("Setting spot tracker to symmetric\n");
            spotTrackerParams[image_index].tracker = new symmetric_spot_tracker_interp(1.0);
            break;
    }

    // Set options for the new tracker.
    if (spotTrackerParams[image_index].tracker) {
        spotTrackerParams[image_index].tracker->set_invert(spotTrackerParams[image_index].invert);
        spotTrackerParams[image_index].tracker->set_pixel_accuracy(spotTrackerParams[image_index].pixelAccuracy);
        spotTrackerParams[image_index].tracker->set_radius_accuracy(spotTrackerParams[image_index].radiusAccuracy);
        spotTrackerParams[image_index].tracker->set_sample_separation(spotTrackerParams[image_index].sampleSeparation);

        // Now update all the fiducial markers 
        recenterFiducials(image_index);
		unpaired_fluoro_recenterFiducials(image_index);
    }

    return 0;
}

int CorrespondenceEditor::setOptimizeSpotTrackerRadius(int image_index, vrpn_bool enable) {
    printf("Setting optimization of spot tracker radius for %d %s\n", image_index, enable ? "on" : "off");
    spotTrackerParams[image_index].optimizeRadius = enable ? true : false;

    // Now update all the fiducial markers 
    recenterFiducials(image_index);

    return 0;
}

int CorrespondenceEditor::setSpotTrackerRadius(int image_index, vrpn_float64 radius) {
    printf("Setting spot tracker radius for %d to %lf\n", image_index, radius);
    corr_point_t spot;
    correspondence->getPoint(image_index, selectedPointIndex, &spot);
    spot.radius = radius;
    correspondence->setPoint(image_index, selectedPointIndex, spot);

    // Now update all the fiducial markers 
    recenterFiducials(image_index);

    return 0;
}

int CorrespondenceEditor::setSpotTrackerPixelAccuracy(int image_index, vrpn_float64 accuracy) {
    printf("Setting spot tracker pixel accuracy for %d to %lf\n", image_index, accuracy);
    spotTrackerParams[image_index].pixelAccuracy = accuracy;

    // Now update all the fiducial markers 
    recenterFiducials(image_index);

    return 0;
}

int CorrespondenceEditor::setSpotTrackerRadiusAccuracy(int image_index, vrpn_float64 accuracy) {
    printf("Setting spot tracker radius accuracy for %d to %lf\n", image_index, accuracy);
    spotTrackerParams[image_index].radiusAccuracy = accuracy;

    return 0;
}

void CorrespondenceEditor::recenterFiducials(int spaceIndex) {
    nmb_Image *image = winParam[spaceIndex].im;
    int numPts = correspondence->numPoints();
    for (int i = 0; i < numPts; i++) {
        centerWithSpotTracker(spaceIndex, i);
    }

    if (numPts > 0) {
        viewer->dirtyWindow(winParam[spaceIndex].winID);
        notifyCallbacks();
    } 
}

void CorrespondenceEditor::unpaired_fluoro_recenterFiducials(int spaceIndex) {
    nmb_Image *image = winParam[spaceIndex].im;
    int numPts = correspondence->unpaired_fluoro_numPoints();
    for (int i = 0; i < numPts; i++) {
        unpaired_fluoro_centerWithSpotTracker(spaceIndex, i);
    }

    if (numPts > 0) {
        viewer->dirtyWindow(winParam[spaceIndex].winID);
        notifyCallbacks();
    } 
}

void CorrespondenceEditor::readAllTest(int spaceIndex, const char * filename) 
{
    nmb_Image *image = this->winParam[spaceIndex].im;

	FILE * pFile;
	pFile = fopen (filename,"w");
	fprintf(pFile,"min value: %f\n", image->minValue());
	fprintf(pFile,"max value: %f\n", image->maxValue());
	fprintf(pFile,"width: %d\n", image->width());
	fprintf(pFile,"height: %d\n", image->height());

	for(int i = 0; i < image->height(); i++)
	{
		for(int j = 0; j < image->width(); j++)
		{
			fprintf(pFile,"%f ", image->getValue(i,j));
		}
		fprintf(pFile,"\n\n");
	}

	fclose (pFile);
//  imageAdapter->setImage(image);
//	imageAdapter->get_num_rows();
}

vector< vector <float> > CorrespondenceEditor::decideOnUsingMedianFilter(int spaceIndex)
{
	vector< vector <float> > pixelMatrix;

    nmb_Image *image = this->winParam[spaceIndex].im;

	float img_width = image->width();
	float img_height = image->height();

//	if(spaceIndex == 0) // do not use median filter for afm images (mapped to height)
	if(spaceIndex == 0  || spaceIndex == 1)
	{
		printf("burda_1 %d", image->height());
		for(int i = 0; i < image->height(); i++)
		{
			vector<float> pixelRow;
			
			for(int j = 0; j < image->width(); j++)
			{
	//			printf("burda_2 %d %d\n", j , image->getValue(i,j));
				pixelRow.push_back(image->getValue(i,j));
			}
	//		printf("burda_3 %d", image->width());
			pixelMatrix.push_back(pixelRow);
		}
	}
	else // use median filter for fluoro images (mapped to color)
	{
		for(int i = 0; i < image->height(); i++)
		{
			vector<float> pixelRow;

			for(int j = 0; j < image->width(); j++)
			{
				float current_pixel = image->getValue(i,j);
				vector<float> tempSort;

				tempSort.push_back(current_pixel);

				if(i>0 && i<(image->height()-1) && j>0 && j<(image->width()-1))
				{
					tempSort.push_back(image->getValue(i-1,j-1)); tempSort.push_back(image->getValue(i-1,j)); tempSort.push_back(image->getValue(i-1,j+1));
					tempSort.push_back(image->getValue(i,j+1));	tempSort.push_back(image->getValue(i+1,j+1)); tempSort.push_back(image->getValue(i+1,j));
					tempSort.push_back(image->getValue(i+1,j-1)); tempSort.push_back(image->getValue(i,j-1));
					sort(tempSort.begin(),tempSort.begin()+9);
					pixelRow.push_back(tempSort[4]);
				}
				else if(i>0 && i<(image->height()-1) && j>0)
				{
					tempSort.push_back(image->getValue(i-1,j-1)); tempSort.push_back(image->getValue(i-1,j)); tempSort.push_back(image->getValue(i+1,j));
					tempSort.push_back(image->getValue(i+1,j-1)); tempSort.push_back(image->getValue(i,j-1));
					sort(tempSort.begin(),tempSort.begin()+6);
					pixelRow.push_back((tempSort[2]+tempSort[3])/2);
				}
				else if(i>0 && i<(image->height()-1) && j<(image->width()-1))
				{
					tempSort.push_back(image->getValue(i-1,j)); tempSort.push_back(image->getValue(i-1,j+1)); tempSort.push_back(image->getValue(i,j+1));
					tempSort.push_back(image->getValue(i+1,j+1)); tempSort.push_back(image->getValue(i+1,j));
					sort(tempSort.begin(),tempSort.begin()+6);
					pixelRow.push_back((tempSort[2]+tempSort[3])/2);
				}
				else if(i>0 && j>0 && j<(image->width()-1))
				{	
					tempSort.push_back(image->getValue(i-1,j-1)); tempSort.push_back(image->getValue(i-1,j)); tempSort.push_back(image->getValue(i-1,j+1));
					tempSort.push_back(image->getValue(i,j+1)); tempSort.push_back(image->getValue(i,j-1));
					sort(tempSort.begin(),tempSort.begin()+6);
					pixelRow.push_back((tempSort[2]+tempSort[3])/2);
				}
				else if(i<(image->height()-1) && j>0 && j<(image->width()-1))
				{
					tempSort.push_back(image->getValue(i,j+1)); tempSort.push_back(image->getValue(i+1,j+1)); tempSort.push_back(image->getValue(i+1,j));
					tempSort.push_back(image->getValue(i+1,j-1)); tempSort.push_back(image->getValue(i,j-1));
					sort(tempSort.begin(),tempSort.begin()+6);
					pixelRow.push_back((tempSort[2]+tempSort[3])/2);
				}///////////////////////////
				else if(i>0 && j>0)
				{
					tempSort.push_back(image->getValue(i-1,j-1)); tempSort.push_back(image->getValue(i-1,j)); tempSort.push_back(image->getValue(i,j-1));
					sort(tempSort.begin(),tempSort.begin()+4);
					pixelRow.push_back((tempSort[1]+tempSort[2])/2);
				}
				else if(i<(image->height()-1)&& j<(image->width()-1))
				{
					tempSort.push_back(image->getValue(i,j+1)); tempSort.push_back(image->getValue(i+1,j+1)); tempSort.push_back(image->getValue(i+1,j));
					sort(tempSort.begin(),tempSort.begin()+4);
					pixelRow.push_back((tempSort[1]+tempSort[2])/2);
				}
				else if(i>0 && j<(image->width()-1))
				{
					tempSort.push_back(image->getValue(i-1,j)); tempSort.push_back(image->getValue(i-1,j+1)); tempSort.push_back(image->getValue(i,j+1));
					sort(tempSort.begin(),tempSort.begin()+4);
					pixelRow.push_back((tempSort[1]+tempSort[2])/2);
				}
				else if(i<(image->height()-1) && j>0)
				{
					tempSort.push_back(image->getValue(i+1,j)); tempSort.push_back(image->getValue(i+1,j-1)); tempSort.push_back(image->getValue(i,j-1));
					sort(tempSort.begin(),tempSort.begin()+4);
					pixelRow.push_back((tempSort[1]+tempSort[2])/2);
				}
				else
				{
					printf("***should not happen***\n");
				}
			}

			pixelMatrix.push_back(pixelRow);
		}
	}

	return pixelMatrix;
}

float CorrespondenceEditor::getWidth(int spaceIndex)
{
    nmb_Image *image = this->winParam[spaceIndex].im;
	float img_width = image->width();
	return img_width;
}

float CorrespondenceEditor::getHeight(int spaceIndex)
{
	nmb_Image *image = this->winParam[spaceIndex].im;
	float img_height = image->height();
	return img_height;
}

vector< vector <float> > CorrespondenceEditor::comparePixelsWithNeighbors(int spaceIndex, const char * filename)
{
    nmb_Image *image = this->winParam[spaceIndex].im;
	vector< vector <float> > pixMatrix;
	pixMatrix = decideOnUsingMedianFilter(spaceIndex);

	FILE * pFile;
	pFile = fopen (filename,"w");
//	fprintf(pFile,"width: %d\n", image->width());
//	fprintf(pFile,"height: %d\n", image->height());
	fprintf(pFile,"%d\n", image->width());
	fprintf(pFile,"%d\n", image->height());

	float img_width = image->width();
	float img_height = image->height();

	int count = 0;

	vector< vector <float> > points;

	// locate the pixels in the image. values are scaled down (they can be between 0 and 1).
	for(int i = 0; i < image->height(); i++)
	{
		for(int j = 0; j < image->width(); j++)
		{
			float current_pixel = pixMatrix[i][j];
			
			vector <float> initialRansacPoint;

			bool validPoint = false;

			if(current_pixel > 0.1)
			{
				if(i>0 && i<(image->height()-1) && j>0 && j<(image->width()-1))
				{
					if(current_pixel >= pixMatrix[i-1][j-1] && current_pixel >= pixMatrix[i-1][j] && current_pixel >= pixMatrix[i-1][j+1] && current_pixel >= pixMatrix[i][j+1] && current_pixel >= pixMatrix[i+1][j+1] && current_pixel >= pixMatrix[i+1][j] && current_pixel >= pixMatrix[i+1][j-1] && current_pixel >= pixMatrix[i][j-1])
					{
//						fprintf(pFile,"%f ", current_pixel);
						validPoint = true;
					}
				}
				else if(i>0 && i<(image->height()-1) && j>0)
				{
					if(current_pixel >= pixMatrix[i-1][j-1] && current_pixel >= pixMatrix[i-1][j] && current_pixel >= pixMatrix[i+1][j] && current_pixel >= pixMatrix[i+1][j-1] && current_pixel >= pixMatrix[i][j-1])
					{
//						fprintf(pFile,"%f ", current_pixel);
						validPoint = true;
					}
				}
				else if(i>0 && i<(image->height()-1) && j<(image->width()-1))
				{
					if(current_pixel >= pixMatrix[i-1][j] && current_pixel >= pixMatrix[i-1][j+1] && current_pixel >= pixMatrix[i][j+1] && current_pixel >= pixMatrix[i+1][j+1] && current_pixel >= pixMatrix[i+1][j])
					{
//						fprintf(pFile,"%f ", current_pixel);
						validPoint = true;
					}
				}
				else if(i>0 && j>0 && j<(image->width()-1))
				{
					if(current_pixel >= pixMatrix[i-1][j-1] && current_pixel >= pixMatrix[i-1][j] && current_pixel >= pixMatrix[i-1][j+1] && current_pixel >= pixMatrix[i][j+1] && current_pixel >= pixMatrix[i][j-1])
					{
//						fprintf(pFile,"%f ", current_pixel);
						validPoint = true;
					}
				}
				else if(i<(image->height()-1) && j>0 && j<(image->width()-1))
				{
					if(current_pixel >= pixMatrix[i][j+1] && current_pixel >= pixMatrix[i+1][j+1] && current_pixel >= pixMatrix[i+1][j] && current_pixel >= pixMatrix[i+1][j-1] && current_pixel >= pixMatrix[i][j-1])
					{
//						fprintf(pFile,"%f ", current_pixel);
						validPoint = true;
					}
				}///////////////////////////
				else if(i>0 && j>0)
				{
					if(current_pixel >= pixMatrix[i-1][j-1] && current_pixel >= pixMatrix[i-1][j] && current_pixel >= pixMatrix[i][j-1])
					{
//						fprintf(pFile,"%f ", current_pixel);
						validPoint = true;
					}
				}
				else if(i<(image->height()-1)&& j<(image->width()-1))
				{
					if(current_pixel >= pixMatrix[i][j+1] && current_pixel >= pixMatrix[i+1][j+1] && current_pixel >= pixMatrix[i+1][j])
					{
//						fprintf(pFile,"%f ", current_pixel);
						validPoint = true;
					}
				}
				else if(i>0 && j<(image->width()-1))
				{
					if(current_pixel >= pixMatrix[i-1][j] && current_pixel >= pixMatrix[i-1][j+1] && current_pixel >= pixMatrix[i][j+1])
					{
//						fprintf(pFile,"%f ", current_pixel);
						validPoint = true;
					}
				}
				else if(i<(image->height()-1) && j>0)
				{
					if(current_pixel >= pixMatrix[i+1][j] && current_pixel >= pixMatrix[i+1][j-1] && current_pixel >= pixMatrix[i][j-1])
					{
//						fprintf(pFile,"%f ", current_pixel);
						validPoint = true;
					}
				}
				else
				{
					printf("***should not happen***\n");
				}
			}

			if(validPoint == true)
			{
				bool too_close = false;
				for(int ind = 0; ind < points.size() && too_close == false; ind++)
				{
					float dist = sqrt(((points[ind][0]-i)*(points[ind][0]-i)) + ((points[ind][1]-j)*(points[ind][1]-j)));
					if(dist <= 5)
					{
						too_close = true;
					}
				}
				if(too_close == false)
				{
					fprintf(pFile,"%f %f ", i/(img_height-1), j/(img_width-1));
//					fprintf(pFile,"%d %d ", i, j);
					initialRansacPoint.push_back(i);
					initialRansacPoint.push_back(j);
					points.push_back(initialRansacPoint);
					count++;
				}
			}
		}
	}

	fclose (pFile);

	printf("%s: %d\n", filename, count);

	return points;
}

// This is some driver code I used to test this stuff:
/*main ()
{
    Correspondence *c = new Correspondence(2, 100);
    ImageViewer *v = new ImageViewer();
    v->init(0);

    int window_ids[2];
    window_ids[0] = v->createWindow(NULL, 100, 100, 100, 100, "win0");
    window_ids[1] = v->createWindow(NULL, 200, 100, 100, 100, "win1");

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


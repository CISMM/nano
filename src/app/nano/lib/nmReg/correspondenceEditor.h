#ifndef IMAGECORRESPONDENCEEDITOR_H
#define IMAGECORRESPONDENCEEDITOR_H

#include "imageViewer.h"
#include "correspondence.h"
#include "PNMImage.h"
#include "PPM.h"

class CorrespondenceWindowParameters {
 public:
  CorrespondenceWindowParameters();
  double left, right, bottom, top;
  nmb_Image *im;
  int winID;
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
    void show();
    void hide();
    void addFiducial(int spaceIndex, float x, float y, float z);
    int setImage(int image_index, nmb_Image *im);
//    int setImageFromPlane(int image_index, BCPlane *p);
//    int setImageFromPNM(int image_index, PNMImage &im);
//    int setImageFromPNM(int image_index, PPM *im);
    void mainloop();
    void getCorrespondence(Correspondence &corr);
	int numImages() {return num_images;}
    void registerCallback(CorrespondenceCallback handler, void *ud);

  private:
    // eventHandler is responsible for handling user interaction with image
    // windows
    static int eventHandler(const ImageViewerWindowEvent &event, void *ud);

    static int displayHandler(const ImageViewerDisplayData &data, void *ud);

    void notifyCallbacks();
    void drawCrosshair(float x, float y);
    void drawSelectionBox(int xp, int yp);
    int getSpaceIndex(int winID);

    ImageViewer *viewer;
    Correspondence *correspondence;
    int num_images;
    CorrespondenceWindowParameters *winParam;
    int selectedPointIndex;
    int grabbedPointIndex;
    vrpn_bool draggingPoint;
    int grab_offset_x, grab_offset_y;

    GLuint point_marker_dlist;
    CorrespondenceCallback change_handler;
    void *userdata;
};

#endif

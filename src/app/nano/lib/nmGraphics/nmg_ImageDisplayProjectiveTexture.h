#ifndef NMG_IMAGEDISPLAYPROJECTIVETEXTURE_H
#define NMG_IMAGEDISPLAYPROJECTIVETEXTURE_H

#include "nmb_ImageDisplay.h"
#include "nmg_Graphics.h"

class nmg_ImageDisplayProjectiveTexture: public nmb_ImageDisplay {
 public:
  nmg_ImageDisplayProjectiveTexture(nmg_Graphics *g);
  virtual void addImageToDisplay(nmb_Image *image);
  virtual void removeImageFromDisplay(nmb_Image *image);
  virtual void updateDisplayTransform(nmb_Image *image, double *transform,
	  bool transformIsSelfReferential);
  virtual void setDisplayColorMap(nmb_Image *image,
                       const char *map, const char *mapdir);
  virtual void setDisplayColorMapRange(nmb_Image *image,
                       float data_min, float data_max,
                       float color_min, float color_max);
  virtual void updateImage(nmb_Image *image);

 protected:
  nmg_Graphics *d_graphicsDisplay;
  vrpn_bool d_projectiveTexturesEnabled;
};

#endif

#ifndef NMG_IMAGEDISPLAYPROJECTIVETEXTURE_H
#define NMG_IMAGEDISPLAYPROJECTIVETEXTURE_H

#include "nmb_ImageDisplay.h"
#include "nmg_Graphics.h"
#include "nmg_GraphicsImpl.h"

class nmg_ImageDisplayProjectiveTexture: public nmb_ImageDisplay {
 public:
  nmg_ImageDisplayProjectiveTexture(nmg_Graphics *g);
  virtual void addImageToDisplay(nmb_Image *image);
  virtual void removeImageFromDisplay(nmb_Image *image);
  virtual void updateDisplayTransform(nmb_Image *image, double *transform,
	  bool transformIsSelfReferential);
  virtual void setDisplayColorMap(nmb_Image *image,
                       const char *map, const char *mapdir);
  virtual void setVideoColorMap(nmb_Image *image,
                       const char *map, const char *mapdir);
  virtual void setDisplayColorMapRange(nmb_Image *image,
                       float data_min, float data_max,
                       float color_min, float color_max);
  virtual void setVideoColorMapRange(nmb_Image *image,
                       float data_min, float data_max,
                       float color_min, float color_max);
  virtual void updateColorMapTextureAlpha(float alpha);
  virtual void updateVideoTextureAlpha(float alpha);

  virtual void updateImage(nmb_Image *image);

 protected:
  nmg_Graphics *d_graphicsDisplay;
  vrpn_bool d_projectiveTexturesEnabled;
};

#endif

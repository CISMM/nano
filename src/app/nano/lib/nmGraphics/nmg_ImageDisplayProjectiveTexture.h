#ifndef NMG_IMAGEDISPLAYPROJECTIVETEXTURE_H
#define NMG_IMAGEDISPLAYPROJECTIVETEXTURE_H

#include "nmb_ImageDisplay.h"
#include "nmg_Graphics.h"
#include "nmg_GraphicsImpl.h"

/*
This class is intended to provide an adapter interface to 
the "COLORMAP" texture in nmg_Graphics. The registration code goes
through this interface instead of accessing nmg_Graphics directly
because the registration code is also used in app/ebeamWriter and
there is a no nmg_Graphics there - this interface is implemented
by a different class in that application. In ebeamWriter, multiple
images are displayed at the same time using textures. In nano only
a single image can be displayed as a texture so that changes the semantics
a little bit - you can't call addImageToDisplay twice and expect to see two 
images in nano but this is what happens in ebeamWriter.

see nmb_ImageDisplay.h for more details.
*/

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
  virtual void updateColorMapTextureAlpha(float alpha);

  virtual void updateImage(nmb_Image *image);

 protected:
  nmg_Graphics *d_graphicsDisplay;
  vrpn_bool d_projectiveTexturesEnabled;
};

#endif

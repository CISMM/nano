#ifndef NMB_IMAGEDISPLAY_H
#define NMB_IMAGEDISPLAY_H

/* an interface for modules that display images.
   I created this to remove dependence on nmg_Graphics classes in the
   nmReg module so it can be used by itself in other programs
   
   Specific instances of this are implemented in:

   1) nmg_ImageDisplayProjectiveTexture based on the realign texture feature
      in nmg_Graphics (nmg_ImageDisplayProjectiveTexture.[hC])

   2) PatternEditor in the seegerizer program (PatternEditor.[hC])
*/

#include "nmb_Image.h"

class nmb_ImageDisplay {
 public:
  nmb_ImageDisplay() {}; 

  /// add or enable the display of an image to the user
  virtual void addImageToDisplay(nmb_Image *image) = 0;

  /// remove or disable the display of an image to the user
  virtual void removeImageFromDisplay(nmb_Image *image) = 0;

  /// updateDisplayTransform:
  /// tell the display to change the coordinates of the image
  /// (e.g. by changing texture coordinates)
  /// transform argument is a 4x4 matrix in column-major order (as in openGL)
  /// If the transform argument is NULL, the display should
  /// use transformation information from the image structure

  /// transformIsSelfReferential: if true it means you should avoid storing
  /// the transform in the worldToImage transform of the image object
  /// because the transform was computed using that value and you can get
  /// into some feedback behavior if you store it in there
  /// In the seegerizer (patternEditor.C) we normally store the transform in
  /// the image so we need to be careful about this but in nano we store it
  /// separate from the image so this isn't an issue
  virtual void updateDisplayTransform(nmb_Image *image, 
                                      double *transform=NULL,
									  bool transformIsSelfReferential=false) = 0;

  /// tell the display what mapping to use for translating values in the 
  /// image into the colors seen by the user
  virtual void setDisplayColorMap(nmb_Image *image,
                           const char *map, const char *mapdir) = 0;

  /// change the scale and offset for the colormap
  virtual void setDisplayColorMapRange(nmb_Image *image,
                        float data_min, float data_max,
                        float color_min, float color_max) = 0;

  /// display the image with all current colormap settings updated
  virtual void updateImage(nmb_Image *image) = 0;

  /// change the alpha of the projective texture for the colormap texture
  virtual void updateAlpha(float alpha) = 0;
};

#endif


#ifndef URPROJECTIVETEXTURE_H
#define URPROJECTIVETEXTURE_H

/*
This texture object class is intended as a member of each geometry object.
Each geometry object is responsible for keeping track of what transformation
it will use for the texture and whether or not this transformation is tied to
world coordinates or the object coordinates. This way a texture may be shared
among multiple objects without requiring that all those objects share the
same texture transformation.
This class takes care of managing all the openGL details of installing a
2D texture and the transformation that compensates for the fact that an image
does not necessarily fit the exact dimensions of the array in which it is stored
or the texture matrix in texture memory (power of 2 restriction).
It doesn't currently let the user choose many texture display options (like
whether to use a mipmap or colormap) but those could
easily be added if needed.
(AAS)
*/

#include "nmb_ColorMap.h"
#include "nmb_Image.h"
#include "PPM.h"
#include <GL/gl.h>

class URProjectiveTexture {
 public:
  URProjectiveTexture();
  int doFastUpdates(bool enable);
  int setOpacity(double opacity);
  int setImage(nmb_Image *image);
  int setImage(PPM *image);

  int setTextureBlendFunction(GLuint blendFunc);
  int createTexture(bool doFastUpdates, nmb_ColorMap *colormap=NULL,
			float data_min=0, float data_max=0, 
			float color_min=0, float color_max=0);

  int installTexture(int width, int height, void *data,
					GLuint internalFormat, GLuint dataFormat, 
					GLuint dataType, GLuint wrapMode);
  // by default the textureTransform is assumed to transform
  // points in object coordinates to texture coordinates
  // with the optional arguments filled in you make this function
  // treat textureTransform as the transformation that takes
  // points in world coordinates to texture coordinates
  int enable(double *textureTransform = NULL,
			 double *objectToWorldTransform = NULL,
			 bool textureInWorldCoordinates = false);
  int disable();
  void changeDataset(nmb_Dataset *data);

 private:
  int updateTextureNoMipmap();
  int initTextureMatrixNoMipmap();
  int updateTextureMipmap(nmb_ColorMap *colormap,
			float data_min, float data_max, 
			float color_min, float color_max);
  void loadColorImageMipmap();
  bool d_textureCreated;
  bool d_imageChangedSinceTextureInstalled;
  bool d_enabled;
  bool d_doingFastUpdates;

  GLuint d_textureID;

  nmb_Image *d_greyscaleImage;
  PPM *d_colorImage;

  // texture matrix size - typically a power of two because thats what
  // openGL requires
  int d_textureMatrixNumX;
  int d_textureMatrixNumY;

  // texture blending attributes
  GLuint d_textureBlendFunction;
  double d_opacity;
};

#endif

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

const int GL_CMAP_SIZE = 512;

class URProjectiveTexture {
 public:
  URProjectiveTexture();
  int doFastUpdates(bool enable);
  int setOpacity(double opacity);
  int setImage(nmb_Image *image);
  int setImage(PPM *image);

  int setTextureBlendFunction(GLuint blendFunc);
  int createTexture(bool doFastUpdates);

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
  
  int width();      // width of the Image
  int height();     // height of the Image

  void setColorMap(nmb_ColorMap* colormap);
  void setColorMapMinMax(float data_min, float data_max, float color_min, float color_max);
  void setUpdateColorMap(bool value);

 private:
  int updateTextureNoMipmap();
  int initTextureMatrixNoMipmap();
  int updateTextureMipmap();
  void loadColorImageMipmap();
  bool d_textureObjectCreated;
  bool d_imageChangedSinceTextureInstalled;
  bool d_enabled;
  bool d_doingFastUpdates;

  GLuint d_textureID;

  nmb_Image *d_greyscaleImage;
  bool d_greyscaleImageTooBig;
  PPM *d_colorImage;
  bool d_colorImageTooBig;

  // texture matrix size - typically a power of two because thats what
  // openGL requires
  int d_textureMatrixNumX;
  int d_textureMatrixNumY;

  // texture blending attributes
  GLuint d_textureBlendFunction;
  double d_opacity;

  // colormap stuff

  float d_colormap_data_min, d_colormap_data_max, d_colormap_color_min, d_colormap_color_max;
  float d_rmap[GL_CMAP_SIZE], d_gmap[GL_CMAP_SIZE], d_bmap[GL_CMAP_SIZE], d_amap[GL_CMAP_SIZE];
  nmb_ColorMap* d_colormap;

  bool d_update_colormap;
};

#endif

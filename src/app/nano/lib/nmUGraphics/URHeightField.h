#ifndef URHEIGHTFIELD_H
#define URHEIGHTFIELD_H

#include "URender.h"
#include "nmb_Image.h"

class URHeightField : public URender{
private:
 public:
  URHeightField();
  virtual ~URHeightField();
  virtual int Render(void *userdata=NULL);
  void setSurface(nmb_Image *heightValues, double xmin, double ymin,
	  double xmax, double ymax, int stride = 1);
  void renderWithoutDisplayList(nmb_Image *heightValues,
	  double xmin, double ymin, double xmax, double ymax,
	  int stride = 1);
  void buildDisplayList(nmb_Image *heightValues, int stride);
  void setWorldFromObjectTransform(double *matrix);
  /// This is an alternative way to set the worldFromObject transformation
  /// and it sets the transformation so that the surface spans and is 
  /// bounded by the specified rectangle in world coordinates
  void setSurfaceRegion(double minX, double minY, double maxX, double maxY);
  void setTextureEnable(bool enable);

  int SetProjTextureAll(void *userdata=NULL);
  int SetTextureTransformAll(void *userdata=NULL);
 
 private:
   static int computeNormal(nmb_Image *heightValues, 
	   int x, int y, double normal[3]);

   GLuint d_displayListID;
   GLdouble d_worldFromObject[16];
   bool d_textureEnabled;
   double d_minX, d_minY, d_maxX, d_maxY;
};

#endif

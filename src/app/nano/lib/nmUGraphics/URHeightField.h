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

  void setSurface(nmb_Image *heightValues, int stride = 1);
  static void renderWithoutDisplayList(nmb_Image *heightValues,
	  int stride = 1);
  void buildDisplayList(nmb_Image *heightValues, int stride);
  void setWorldFromObjectTransform(double *matrix);
 
 private:
   static int computeNormal(nmb_Image *heightValues, 
	   int x, int y, double normal[3]);

   GLuint d_displayListID;
   GLdouble d_worldFromObject[16];
};

#endif

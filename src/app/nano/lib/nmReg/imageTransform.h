#ifndef IMAGETRANSFORM_H
#define IMAGETRANSFORM_H

#include "nmb_Image.h"
#include <assert.h>

/*
	This class is for representing an arbitrary transformation
that takes points in one image to points in another image. In some cases
we may need something more general than a 4x4 transformation matrix.

*/

class ImageTransform {
  public:
    ImageTransform(int d_src, int d_dest):dim_src(d_src), dim_dest(d_dest){}
    virtual void transform(double *p_src, double *p_dest) const = 0;
	virtual ImageTransform *duplicate() const = 0;
  protected:
    int dim_src, dim_dest;	// image dimensions
};

typedef ImageTransform *ImageTransformPtr;

// this transformation represents as much as a 4x4 transformation matrix
class ImageTransformAffine : public ImageTransform {
  public:
    // initialize to identity transform
    ImageTransformAffine(int d_src, int d_dest):
	ImageTransform(d_src, d_dest) 
    {
	int i,j;
	for (i = 0; i < 4; i++)
	    for (j = 0; j < 4; j++)
		xform[i][j] = (double)(i==j);
    }
    void set(int i_dest, int i_src, double value) {
	assert(i_dest >= 0 && i_dest < 4 && i_src >= 0 && i_src < 4);
	xform[i_dest][i_src] = value;
    }
    virtual void transform(double *p_src, double *p_dest) const;
	virtual ImageTransform *duplicate() const {
		ImageTransformAffine *ita = new ImageTransformAffine(dim_src, dim_dest);

		for (int i = 0; i < 4; i++){
			for (int j = 0; j < 4; j++){
				ita->xform[i][j] = xform[i][j];
			}
		}
		return (ImageTransform *)ita;
	}

  protected:
    double xform[4][4];
};



// this transformation represents a mapping from each pixel in the source
// image to a point in the destination image
/*
class ImageTransformPerPixel : public ImageTransform {
};
*/

#endif

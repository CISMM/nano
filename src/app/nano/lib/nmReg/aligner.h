#ifndef ALIGNER_H
#define ALIGNER_H

#include "imageTransform.h"
#include "vrpn_Types.h"
#include "assert.h"
#include "correspondence.h"

class Aligner {
  public:
    Aligner(int n_images);

	// Tell the Aligner what the data is for a particular image index,
	// whether the data is simply an n-dimensional scalar field or
	// an n-dimensional height field embedded in an n+1-dimensional
	// space (n=2 for BCGrid) and what the scale factor should be for
	// the height field if that is the case:
    void setData(int whichImage, nmb_Image *data, int is_surf = VRPN_FALSE,
									double h_scale = 1.0) {
		assert(whichImage >= 0 && whichImage < num_images);
		aligner_data_images[whichImage] = data;
		is_surface[whichImage] = is_surf;
		if (is_surf)
    		dimension[whichImage] = 3;
		else
    		dimension[whichImage] = 2;
		height_scale[whichImage] = h_scale;
	}
    void setCorrespondence(Correspondence c) {corr = c;corr_set = VRPN_TRUE;}
    int updateTransform(int src, int dest);
    void transform(int src, double *p_src, int dest, double *p_dest);
    void getGLtransform(int src, int dest, double *xform);
    ImageTransform *getTransform(int src, int dest) {
		return xform[src + dest*num_images];}
    int computeResampleExtents(const nmb_Image &source_image,
         const nmb_Image &target_image, ImageTransform &xform,
         int &min_i, int &min_j, int &max_i, int &max_j);
    int computeResampleExtents(const nmb_Image &source_image,
         const nmb_Image &target_image, ImageTransform &xform,
         double &min_i, double &min_j, double &max_i, double &max_j);
    void resample2DImageTo2DImage(
                        const nmb_Image &source_image,
                        const nmb_Image &target_image,
                        const ImageTransform &xform,
                        nmb_Image &resample_image,
                        double mixingRatio);

	/* projection_image - this is assumed to be a projection of some scalar
		value from the surface represented by depth_image
	   depth_image - this is a height field
	   xform - this transforms points from the depth_image into points in
		the projection_image
	   orthogonal_projection_image - this should typically have the same
		world dimensions as the depth_image but may have a different
		resolution
	   desired algorithm: Each pixel in the orthogonal_projection_image is 
		converted into a patch of the surface from depth_image based
		on the x,y values at the pixel corners. The region formed by
		projecting this piece of the surface onto the projection_image
		(using xform) is computed and then a sample value is computed
		by integrating over this region.
	   cheap algorithm (implemented): The center of each pixel in the 
		orthogonal_projection_image is converted into a surface point
		using depth_image. This point is transformed into the 
		projection_image and the sample value is computed using
		bilinear interpolation.
	*/
	void resampleImageToDepthImage(
				const nmb_Image &projection_image,
				const nmb_Image &depth_image,
				const ImageTransform &xform, 
				nmb_Image &orthogonal_projection_image);

	/* projects corners of each pixel in projection image onto depth
	   image and computes the size of the pixel in the orthogonal
	   projection image; the dimensions of the smallest such transformed
	   pixel are returned as a ratio to the size of pixels in
	   the depth_image
	   (to preserve nearly as much information as possible when using 
	   resampleImageToDepthImage(),
	   you should multiply the x and y resolution of the depth image
	   by the given factors or by the maximum of the two factors if you
	   want to have square pixels)
	*/
	void computeMaxRequiredResampleResolution( 
				const nmb_Image &projection_image,
				const nmb_Image &depth_image,
				const ImageTransform &xform,
				double &x_factor,
				double &y_factor);

  private:
    void setTransform(int src, int dest, ImageTransform *x) {
	if (xform[src + dest*num_images]) delete xform[src + dest*num_images];
	xform[src + dest*num_images] = x->duplicate();}

    vrpn_bool *is_surface;
    double *height_scale;
    int *dimension;
    int num_images;

    Correspondence corr;
	vrpn_bool corr_set;
    int xform_src, xform_dest;	// src and dest image indices for xform
    ImageTransformPtr *xform;
    vrpn_bool *xform_set;
	nmb_Image **aligner_data_images;
};

#endif

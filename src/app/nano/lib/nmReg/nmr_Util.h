#ifndef NMR_UTIL_H
#define NMR_UTIL_H

#include "nmb_Image.h"
#include "nmb_ImageTransform.h"

/* note: src=AFM, target=SEM 
   (AFM coordinates are transformed into SEM coordinates)
*/

/// This class contains a bunch of utility functions for doing things 
/// with registration results (basically resampling)
class nmr_Util {
  public:
    /// if xform is invertible, then this function will attempt to
    /// compute the corners of the image in pixel indices of the src image
    /// that will contain the region formed by transforming the
    /// corners of the target image using the inverse transformation.
    /// The returned indices tell you what resolution you should use to
    /// create a resampled image containing the target image but with the 
    /// same resolution as the source image
    static int computeResampleExtents(const nmb_Image &src, 
                        const nmb_Image &target,
                        nmb_ImageTransform &xform, int &min_i, int &min_j,
                        int &max_i, int &max_j);

    /// given srcImage and resampleImage and indices referring to points in 
    /// units of pixels of srcImage, this function sets the region of the
    /// resampleImage so that it equals the specified region taken from the
    /// srcImage (this may be a subregion or it could contain points outside
    /// the region contained in the srcImage)
    static void setRegionRelative(const nmb_Image &srcImage, 
                           nmb_Image &resampleImage, int min_i, int min_j,
                           int max_i, int max_j);

    /// for 2D->2D transformations
    /// this function resamples the target image into the resampleImage 
    /// according to the given transformation using bilinear interpolation
    /// xform is used to transform pixels in resampleImage into their
    /// locations in the target image in order to determine pixel values
    /// The nmb_ImageTransform argument is actually a transformation from
    /// world coordinates to world coordinates for the two images
    static void createResampledImage(nmb_Image &target,
                        const nmb_ImageTransform &xform, 
                        nmb_Image &resampleImage);
    

    /// for 2D->2D transformations
    /// this function resamples the target image into the resampleImage
    /// according to the given transformation using bilinear interpolation
    /// xform is used to transform pixels in resampleImage into their
    /// locations in the target image in order to determine pixel values
    /// The nmb_ImageTransform argument is a transformation from
    /// normalized image coordinates to normalized image coordinates
    /// for the two images (normalized image coordinates means that 
    /// pixel coordinates always span the unit square)
    static void createResampledImageWithImageSpaceTransformation(
          nmb_Image &target, const nmb_ImageTransform &xform,
          nmb_Image &resampleImage);

    /// for 3D->2D transformations
    /// computes interpolated height for points in the resampleImage by
    /// using the given height field (source); these 3D points are then 
    /// transformed using the given transformation to get a position
    /// in the target image and then the value for this position is found
    /// by interpolating the values in the target image
    /// (I'll bet there's a more efficient way to do this using 1D warping
    /// operations (ask ibr people) but this is easier to understand and 
    /// it doesn't need to be that fast - plus it allows the transformation
    /// to be completely general - although we're obviously assuming some
    /// smoothness since we use interpolation)
    static void createResampledImage(const nmb_Image &target, 
                        const nmb_Image &source,
                        const nmb_ImageTransform &xform,
                        nmb_Image &resampleImage);


/*
    // uses openGL to scan convert the height field as viewed with a
    // projection matrix from the given transformation
    // rgba will be used to store the polygon ids
    // NOT YET IMPLEMENTED
    static computeOcclusionMap(const nmb_Image &heightField,
                        const nmb_ImageTransform &xform,
                        const nmb_Image &occlusionMap,
                        vrpn_bool useOCO = vrpn_FALSE,
                        float cop_x = 0.0, float cop_y = 0.0);

    // for 3D->2D transformations
    // 
    // NOT YET IMPLEMENTED
    static void createResampledImage(const nmb_Image &heightField,
                              const nmb_Image &occlusionMap,
                              const nmb_Image &projectionImage,
                              const nmb_ImageTransform &xform,
                              nmb_Image &resampleImage);
*/

    /// sum = wa*(interpolated addend or 0 if outside of addend) + ws*sum
    static void addImage(nmb_Image &addend, nmb_Image &sum, float wa,
                  float ws);

};

#endif

#ifndef NMR_UTIL_H
#define NMR_UTIL_H

#include "nmb_Image.h"
#include "nmb_TransformMatrix44.h"

/* note: src=AFM, target=SEM 
   (AFM coordinates are transformed into SEM coordinates)
*/

/// This class contains a bunch of utility functions for doing things 
/// with registration results (basically resampling)
class nmr_Util {
  public:
    /// this function converts a transformation between scaled image 
    /// coordinates in A and scaled image coordinates in image B
    /// to one between world coordinates in A and world coordinates in B
// see comment in nmr_Util.C file
//    static int computeResampleTransformInWorldCoordinates(
//           nmb_Image *imA, nmb_Image *imB,
//           nmb_TransformMatrix44 &scaledImA_from_scaledImB,
//           nmb_TransformMatrix44 &worldA_from_worldB);
    /// this function converts a transformation between scaled image
    /// coordinates in A and scaled image coordinates in image B
    /// to one between normalized image coordinates in A and normalized
    /// image coordinates in B
    static int computeResampleTransformInImageCoordinates(
           nmb_Image *imA, nmb_Image *imB,
           nmb_TransformMatrix44 &scaledImA_from_scaledImB,
           nmb_TransformMatrix44 &imA_from_imB);

    /// if xform is invertible, then this function will attempt to
    /// compute the corners of the image in pixel indices of the src image
    /// that will contain the region formed by transforming the
    /// corners of the target image using the inverse transformation.
    /// The returned indices tell you what resolution you should use to
    /// create a resampled image containing the target image but with the 
    /// same resolution as the source image
    /// xform should be from normalized image coordinates of src to 
    /// normalized image coordinates of target
    static int computeResampleExtents(const nmb_Image &src, 
                        const nmb_Image &target,
                        nmb_TransformMatrix44 &xform, int &min_i, int &min_j,
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
    /// The nmb_TransformMatrix44 argument is actually a transformation from
    /// world coordinates to world coordinates for the two images
    static void createResampledImage(nmb_Image &target,
                        const nmb_TransformMatrix44 &xform, 
                        nmb_Image &resampleImage);
    

    /// for 2D->2D transformations
    /// this function resamples the target image into the resampleImage
    /// according to the given transformation using bilinear interpolation
    /// xform is used to transform pixels in resampleImage into their
    /// locations in the target image in order to determine pixel values
    /// The nmb_TransformMatrix44 argument is a transformation from
    /// normalized image coordinates to normalized image coordinates
    /// for the two images (normalized image coordinates means that 
    /// pixel coordinates always span the unit square)
    static void createResampledImageWithImageSpaceTransformation(
          nmb_Image &target, const nmb_TransformMatrix44 &xform,
          nmb_Image &resampleImage,
		  double nonZeroIntensityOffset);

    /// xform defines a transformation from source to target
    /// resampleImage is assumed to have the same world coordinate system as
    /// the source image but may cover a larger region in the world than the
    /// source image (as may be necessary to completely cover the target)
    static void createResampledImageWithImageSpaceTransformation(
          nmb_Image &target, nmb_Image &source, 
          const nmb_TransformMatrix44 &xform, nmb_Image &resampleImage,
		  double nonZeroIntensityOffset);

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
                        const nmb_TransformMatrix44 &xform,
                        nmb_Image &resampleImage);


/*
    // uses openGL to scan convert the height field as viewed with a
    // projection matrix from the given transformation
    // rgba will be used to store the polygon ids
    // NOT YET IMPLEMENTED
    static computeOcclusionMap(const nmb_Image &heightField,
                        const nmb_TransformMatrix44 &xform,
                        const nmb_Image &occlusionMap,
                        vrpn_bool useOCO = vrpn_FALSE,
                        float cop_x = 0.0, float cop_y = 0.0);

    // for 3D->2D transformations
    // 
    // NOT YET IMPLEMENTED
    static void createResampledImage(const nmb_Image &heightField,
                              const nmb_Image &occlusionMap,
                              const nmb_Image &projectionImage,
                              const nmb_TransformMatrix44 &xform,
                              nmb_Image &resampleImage);
*/

    /// sum = wa*(interpolated addend or 0 if outside of addend) + ws*sum
    static void addImage(nmb_Image &addend, nmb_Image &sum, float wa,
                  float ws);

    /// this will linearly interpolate source and then integrate the
    /// result into each pixel in resampledImage - useful for subsampling
    static void resample(nmb_Image &source, nmb_Image &resampledImage);
 
    /// allocate numLevels images and put blurred versions of the argument
    /// into them
    static void buildGaussianPyramid(nmb_Image &src, int numLevels, 
                              float *stddev,
                              nmb_Image **pyramid);

    /// blur image with a Gaussian kernel - takes advantage of separability
    /// in x and y by composing two 1D convolutions
    static void blur(nmb_Image &im, double std_dev_x, double std_dev_y);

    static double sampleUniformDistribution(double min, double max);
 
    static void createGradientImages(nmb_Image &source,
              nmb_Image &grad_x, nmb_Image &grad_y);
 
    static double computeMean(nmb_Image &im);
    static double computeVariance(nmb_Image &im, double mean);

    static double maximumOffset2D(nmb_TransformMatrix44 xform,
                                double minX, double maxX,
                                double minY, double maxY,
                                double &x, double &y);
    static double approxMeanOffset2D(nmb_TransformMatrix44 xform,
                                double minX, double maxX,
                                double minY, double maxY);
};

#endif

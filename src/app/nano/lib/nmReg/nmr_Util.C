#include "nmr_Util.h"
#include "stdlib.h"
#include "nmr_Gaussian.h"

/**
 computeResampleExtents:
   given target image, source image and ImageTransform, this function
   computes the extents of the source image in the pixel coordinates of the
   target image

   example:

   (1) source image (diamond shape) aligned with target image region (square)
   (2) new image to contain both regions (the resample image) with image axes
       and pixels aligned to target but sized and translated appropriately

                                ____________
       (1)  /\             (2)  |    /\    |
           /  \                 |   /  \   |
          /    \                |  /    \  |
         / ---- \               | / ---- \ |
        /  |  |  \       --->   |/  |  |  \|
        \  |  |  /              |\  |  |  /|
         \ ---- /               | \ ---- / |
          \    /                |  \    /  |
           \  /                 |   \  /   |
            \/                  |    \/    |
                                ------------

*/

//static 
int nmr_Util::computeResampleExtents(const nmb_Image &src,
                        const nmb_Image &target,
                        nmb_TransformMatrix44 &xform, int &min_i, int &min_j,
                        int &max_i, int &max_j)
{
    /* approach: invert the transformation and find extrema by transforming
                 corners of source_image - this works for affine
                 transformations but not in general. I don't expect
                 we'll be using any transformations that would differ
                 much from an affine approximation so its probably okay
     */
    if (!xform.hasInverse()) {
        fprintf(stderr, "Aligner::computeResampleResolutionAndOffset: Error, "
                "transformation not invertible\n");
        return -1;
    }

    /* now we transform corners of the target image into the source space */
    nmb_ImageBounds ib;
    target.getBounds(ib);
    // this array is to contain the 4 corners of the image
    // both before and after transforming into the source world coordinates
    double corner_pnt[4][4] = {{0, 0, 0, 1},
                               {0, 0, 0, 1},
                               {0, 0, 0, 1},
                               {0, 0, 0, 1}};

    corner_pnt[0][0] = ib.getX(nmb_ImageBounds::MIN_X_MIN_Y);
    corner_pnt[0][1] = ib.getY(nmb_ImageBounds::MIN_X_MIN_Y);
    corner_pnt[1][0] = ib.getX(nmb_ImageBounds::MIN_X_MAX_Y);
    corner_pnt[1][1] = ib.getY(nmb_ImageBounds::MIN_X_MAX_Y);
    corner_pnt[2][0] = ib.getX(nmb_ImageBounds::MAX_X_MIN_Y);
    corner_pnt[2][1] = ib.getY(nmb_ImageBounds::MAX_X_MIN_Y);
    corner_pnt[3][0] = ib.getX(nmb_ImageBounds::MAX_X_MAX_Y);
    corner_pnt[3][1] = ib.getY(nmb_ImageBounds::MAX_X_MAX_Y);

    int i;
    printf("nmr_Util::computeResampleExtents corners:\n");
    for (i = 0; i < 4; i++){
       printf("(%g,%g)\n", corner_pnt[i][0], corner_pnt[i][1]);
    }

    xform.invTransform(corner_pnt[0], corner_pnt[0]);
    xform.invTransform(corner_pnt[1], corner_pnt[1]);
    xform.invTransform(corner_pnt[2], corner_pnt[2]);
    xform.invTransform(corner_pnt[3], corner_pnt[3]);

    for (i = 0; i < 4; i++){
       printf("(%g,%g)\n", corner_pnt[i][0], corner_pnt[i][1]);
    }

    /* now, corner_pnt gives the positions of the corners of the target image
       in the world coordinates of the source image ,
       convert these into pixels in source image */
    double i_corner[4], j_corner[4];
    for (i = 0; i < 4; i++) {
        src.worldToPixel(corner_pnt[i][0], corner_pnt[i][1],
                                  i_corner[i], j_corner[i]);
    }

    // find the extremes:
    min_i = max_i = i_corner[0];
    min_j = max_j = j_corner[0];
    for (i = 1; i < 4; i++) {
        if (i_corner[i] < min_i) min_i = i_corner[i];
        else if (i_corner[i] > max_i) max_i = i_corner[i];
        if (j_corner[i] < min_j) min_j = j_corner[i];
        else if (j_corner[i] > max_j) max_j = j_corner[i];
    }

    return 0;
}

//static 
void nmr_Util::setRegionRelative(const nmb_Image &srcImage,
                           nmb_Image &resampleImage, int min_i, int min_j,
                           int max_i, int max_j)
{
    nmb_ImageBounds resampleImageBounds;
    double x, y;
    srcImage.pixelToWorld((double)min_i, (double)min_j, x, y);
    resampleImageBounds.setX(nmb_ImageBounds::MIN_X_MIN_Y, x);
    resampleImageBounds.setY(nmb_ImageBounds::MIN_X_MIN_Y, y);
    srcImage.pixelToWorld((double)min_i, (double)(max_j+1), x, y);
    resampleImageBounds.setX(nmb_ImageBounds::MIN_X_MAX_Y, x);
    resampleImageBounds.setY(nmb_ImageBounds::MIN_X_MAX_Y, y);
    srcImage.pixelToWorld((double)(max_i+1), (double)min_j, x, y);
    resampleImageBounds.setX(nmb_ImageBounds::MAX_X_MIN_Y, x);
    resampleImageBounds.setY(nmb_ImageBounds::MAX_X_MIN_Y, y);
    srcImage.pixelToWorld((double)(max_i+1), (double)(max_j+1), x, y);
    resampleImageBounds.setX(nmb_ImageBounds::MAX_X_MAX_Y, x);
    resampleImageBounds.setY(nmb_ImageBounds::MAX_X_MAX_Y, y);
    resampleImage.setBounds(resampleImageBounds);
}

/** this function treats xform as a transformation from 2D to 2D
 */
//static 
void nmr_Util::createResampledImage(nmb_Image &targetImage,
                        const nmb_TransformMatrix44 &xform,
                        nmb_Image &resampleImage)
{
  int i,j;
  int w,h;
  w = resampleImage.width();
  h = resampleImage.height();
  double i_center, j_center;
  double p_source[4] = {0,0,0,1}, p_target[4]; // world coordinates
  double i_target, j_target; // pixel coordinates
  double value_target;

  fprintf(stderr, "(min, minNZ, max) = (%f,%f,%f)\n",
        targetImage.minValue(), targetImage.minNonZeroValue(),
        targetImage.maxValue());

  for (i = 0, i_center = 0.5; i < w; i++, i_center += 1.0){
    for (j = 0, j_center = 0.5; j < h; j++, j_center += 1.0){
      resampleImage.pixelToWorld(i_center, j_center, p_source[0], p_source[1]);
      xform.transform(p_source, p_target);
      targetImage.worldToPixel(p_target[0], p_target[1], i_target, j_target);
      if (i_target >= 0 && j_target >= 0 &&
          i_target < targetImage.width() &&
          j_target < targetImage.height()) {
          value_target = targetImage.getValueInterpolatedNZ(i_target, j_target);
          if ((value_target < targetImage.minNonZeroValue()) ||
              (value_target > targetImage.maxValue())) {
              fprintf(stderr, "Warning: resampleImage:"
                      " not getting expected values from interpolation\n");
          }
      } else {
          value_target = 0.0;
      }
      resampleImage.setValue(i,j, value_target);
    }
  }
}

// static
void nmr_Util::createResampledImageWithImageSpaceTransformation(
                        nmb_Image &targetImage,
                        const nmb_TransformMatrix44 &xform,
                        nmb_Image &resampleImage)
{
  int i,j;
  int w,h;
//  double i_targ_min, i_targ_max, j_targ_min, j_targ_max;
  w = resampleImage.width();
  h = resampleImage.height();
  double i_center, j_center;
  double i_target, j_target; // pixel coordinates
  double value_target;
  double p_source_norm[4] = {0,0,0,1}; // normalized pixel coordinates
  double p_target_norm[4] = {0,0,0,1};
  double x_incr, y_incr;

  x_incr = 1.0/(double)w;
  y_incr = 1.0/(double)h;

  fprintf(stderr, "(min, minNZ, max) = (%f,%f,%f)\n", 
	targetImage.minValue(), targetImage.minNonZeroValue(),
	targetImage.maxValue());

  for (i = 0, i_center = 0.5*x_incr; i < w; i++, i_center += x_incr){
    for (j = 0, j_center = 0.5*y_incr; j < h; j++, j_center += y_incr){

      p_source_norm[0] = i_center;
      p_source_norm[1] = j_center;
      xform.transform(p_source_norm, p_target_norm);
      i_target = p_target_norm[0]*targetImage.width();
      j_target = p_target_norm[1]*targetImage.height();

/*
      for debugging:
      if (i == 0 && j== 0) {
         i_targ_min = i_targ_max = i_target;
         j_targ_min = j_targ_max = j_target;
      } else {
         if (i_target > i_targ_max) i_targ_max = i_target;
         if (i_target < i_targ_min) i_targ_min = i_target;
         if (j_target > j_targ_max) j_targ_max = j_target;
         if (j_target < j_targ_min) j_targ_min = j_target;
      }
*/
      if (i_target >= 0 && j_target >= 0 &&
          i_target < targetImage.width() &&
          j_target < targetImage.height()) {
          value_target = targetImage.getValueInterpolatedNZ(i_target, j_target);
          if ((value_target < targetImage.minNonZeroValue()) ||
	      (value_target > targetImage.maxValue())) {
              fprintf(stderr, "Warning: resampleImageIST:"
                      " not getting expected values from interpolation\n");
          }
      } else {
          value_target = 0.0;
      }


      resampleImage.setValue(i,j, value_target);
    }
  }
 // fprintf(stderr, "(%g,%g)-(%g,%g)\n", i_targ_min, j_targ_min, i_targ_max, j_targ_max);
}


/** this function treats xform as a transformation from 3D to 2D
    3D points are constructed for each point in the resampleImage by
    looking up the z value in the source image for that point
    if the point doesn't lie in the source image then the value in the
    resampleImage is set to 0; otherwise, the 3D point is transformed using
    xform and the value for the resulting 2D point is interpolated in the
    target image and the corresponding pixel in the resampleImage is set to
    the interpolated value;

    normally, resampleImage should have a resolution that is some multiple
    of the source image resolution and its region should match or be a 
    subregion of that of the source image
 */
//static 
void nmr_Util::createResampledImage(const nmb_Image &targetImage,
                        const nmb_Image &sourceImage,
                        const nmb_TransformMatrix44 &xform,
                        nmb_Image &resampleImage)
{
  int i,j;
  int w,h;
  w = resampleImage.width();
  h = resampleImage.height();
  double i_center, j_center, i_source, j_source;
  double p_source[4] = {0,0,0,1}, p_target[4]; // world coordinates
  double i_target, j_target; // pixel coordinates
  double value_target;

  for (i = 0, i_center = 0.5; i < w; i++, i_center += 1.0){
    for (j = 0, j_center = 0.5; j < h; j++, j_center += 1.0){
      resampleImage.pixelToWorld(i_center, j_center, p_source[0], p_source[1]);
      sourceImage.worldToPixel(p_source[0], p_source[1], i_source, j_source);
      if (i_source > 0 && i_source < sourceImage.width() &&
          j_source > 0 && j_source < sourceImage.height()) {
        p_source[2] = sourceImage.getValueInterpolatedNZ(i_source, j_source);
        xform.transform(p_source, p_target);
        targetImage.worldToPixel(p_target[0], p_target[1], i_target, j_target);
        if (i_target >= 0 && j_target >= 0 &&
            i_target < targetImage.width() &&
            j_target < targetImage.height()) {
          value_target = targetImage.getValueInterpolatedNZ(i_target, j_target);
        } else {
          value_target = 0.0;
        }
      } else {
        value_target = 0.0;
      }
      resampleImage.setValue(i,j, value_target);
    }
  }
}

//static 
void nmr_Util::addImage(nmb_Image &addend, nmb_Image &sum, float wa,
                  float ws)
{
  int i,j;
  float val;
  double x_world, y_world;
  double i_center, j_center, i_addend, j_addend;

  double rangeFactor = (sum.maxValue()-sum.minNonZeroValue())/
                       (addend.maxValue()-addend.minNonZeroValue());
  double offset = addend.minNonZeroValue();
  double sumOffset = sum.minNonZeroValue();

/*
  int w= sum.width(), h = sum.height();
  w = addend.width();
  h = addend.height();

  double sum_avg = sum.getValue(w/2, h/2) + sum.getValue(w/4, h/2)+
                   sum.getValue(w/2, h/4) + sum.getValue(w/4, h/4);
  double val_avg = addend.getValue(w/2, h/2) + addend.getValue(w/4, h/2)+
                   addend.getValue(w/2, h/4) + addend.getValue(w/4, h/4);
  val_avg *= 0.25;
  sum_avg *= 0.25;
*/

  for (i = 0, i_center = 0.5; i < sum.width(); i++, i_center += 1.0) {
    for (j = 0, j_center = 0.5; j < sum.height(); j++, j_center += 1.0) {
      val = ws*sum.getValue(i,j);
      // get world coordinates for this point in sum image
      sum.pixelToWorld(i_center, j_center, x_world, y_world);
      addend.worldToPixel(x_world, y_world, i_addend, j_addend);
      if (i_addend >= 0 && j_addend >= 0 &&
          i_addend < addend.width() && 
          j_addend < addend.height()) {
          val = ws*sum.getValue(i,j) + 
                wa*(rangeFactor*
                    (addend.getValueInterpolatedNZ(i_addend, j_addend)-offset)
                    + sumOffset);
/*
                wa*(sum_avg + 
                    val_scale*
                     (addend.getValueInterpolatedNZ(i_addend, j_addend)-val_avg)
                   );
*/
          sum.setValue(i,j, val);
      }
    }
  }
}

/* resample:
  conceptually, this is supposed to reconstruct a continuous image from
  src by linear interpolation. dest is then computed as
  a box filtered version of this continuous image by integrating over each
  pixel in dest (this integration mostly involves summing over pixels in
  src except at the edges of the dest pixel where only a fractional part of
  a pixel in dest falls in the dest pixel. 
*/

// static
void nmr_Util::resample(nmb_Image &src, nmb_Image &dest)
{
  double x_lo_fract, x_hi_fract, y_lo_fract, y_hi_fract;
  double k_min, k_max, l_min, l_max;
  int i,j,k,l;

  int destx, desty, srcx, srcy;       // image dimensions
  srcx = src.width();
  srcy = src.height();
  destx = dest.width();
  desty = dest.height();

  double stride_x = (double)(srcx)/(double)(destx);
  double stride_y = (double)(srcy)/(double)(desty);
  double dxmin, dxmax, dymin, dymax;
  
  double inv_stride_area = 1.0/(stride_x*stride_y);
  double sum;

  for (i = 0, dxmin = 0.0, dxmax = stride_x; 
       i < destx; i++, dxmin += stride_x, dxmax += stride_x) {
    k_min = (int)floor(dxmin);
    k_max = (int)ceil(dxmax);
    x_lo_fract = k_min + 1.0 - dxmin;
    x_hi_fract = dxmax - k_max + 1.0;
    if (k_max > srcx) {
      k_max = srcx;
      x_hi_fract = 1.0;
    }
    for (j = 0, dymin = 0.0, dymax = stride_y; 
         j < desty; j++, dymin += stride_y, dymax += stride_y) {
      l_min = (int)floor(dymin);
      l_max = (int)ceil(dymax);
      y_lo_fract = l_min + 1.0 - dymin;
      y_hi_fract = dymax - l_max + 1.0;
      if (l_max > srcy) {
        l_max = srcy;
        y_hi_fract = 1.0;
      }
      sum = 0.0;
      for (k = k_min; k < k_max; k++) {
        double colsum = 0.0;
        colsum += y_lo_fract*src.getValue(k, l_min);
        for (l = l_min+1; l < l_max-1; l++) {
          colsum += src.getValue(k, l);
        }
        colsum += y_hi_fract*src.getValue(k, l_max-1);
        if (k == k_min) {
          colsum *= x_lo_fract;
        } else if (k == k_max-1) {
          colsum *= x_hi_fract;
        }
        sum += colsum;
      }
      sum *= inv_stride_area;
      dest.setValue(i, j, sum);
    }
  }
}

void nmr_Util::buildGaussianPyramid(nmb_Image &src, 
                 int numLevels, float *stddevs, nmb_Image **pyramid)
{
  int i;
  char new_name[256];
  double stddev;
  for (i = 0; i < numLevels; i++) {
    stddev = stddevs[i];
    sprintf(new_name, "%s_%d", src.name()->Characters(), i);
    pyramid[i] = new nmb_ImageGrid(&src);
    *(((nmb_ImageGrid *)pyramid[i])->name()) = BCString(new_name);
    if (stddev > 0.0) {
      blur(*(pyramid[i]), stddev, stddev);
    }
  }
}

void nmr_Util::blur(nmb_Image &im, double std_dev_x, double std_dev_y)
{
  double numStdDev = 3.0;
  int im_width = im.width(), im_height = im.height();
  int rowFilterLength = 2*(int)ceil(numStdDev*std_dev_x);
  int colFilterLength = 2*(int)ceil(numStdDev*std_dev_y);
  if (rowFilterLength%2==0) rowFilterLength += 1;
  if (colFilterLength%2==0) colFilterLength += 1;

  double *rowFilter = new double[rowFilterLength];
  double *colFilter = new double[colFilterLength];
  float *lineCopy = NULL;
  if (im_width > im_height) {
    lineCopy = new float[im_width];
  } else {
    lineCopy = new float[im_height];
  }
  nmr_Gaussian::makeFilter(rowFilterLength, rowFilter, 1.0/std_dev_x);
  nmr_Gaussian::makeFilter(colFilterLength, colFilter, 1.0/std_dev_y);

  int i,j,k;
  double blurredVal = 0.0;
  double total_weight;
  // blur rows
  for (j = 0; j < im_height; j++) {
    // make a temporary copy of the row
    for (i = 0; i < im_width; i++) {
      lineCopy[i] = im.getValue(i,j);
    }
    // convolve the row with a gaussian filter
    for (i = 0; i < im_width; i++) {
      int i_conv;
      int i_conv_min;
      i_conv_min = i - rowFilterLength/2;
      total_weight = 1.0;
      blurredVal = 0.0;
      vrpn_bool renormalize = vrpn_FALSE;
      for (k = 0, i_conv = i_conv_min; k < rowFilterLength; k++, i_conv++) {
        if (i_conv >= 0 && i_conv < im_width) {
          blurredVal += rowFilter[k]*lineCopy[i_conv];
        } else {
          renormalize = vrpn_TRUE;
          total_weight -= rowFilter[k];
        }
      }
      if (renormalize) {
        if (total_weight == 0) {
          printf("nmr_Util::blur: row blur: this shouldn't happen\n");
        } else {
          blurredVal /= total_weight;
        }
      }
      im.setValue(i, j, blurredVal);
    }
  }

  // now blur columns
  for (i = 0; i < im_width; i++) {
    // make temporary copy of the column
    for (j = 0; j < im_height; j++) {
      lineCopy[j] = im.getValue(i,j);
    }
    // convolve the row with a gaussian filter with std. dev. equal to
    // d_sigmaTestTest/deltaTestVal with filter extent of testFilterExtent
    for (j = 0; j < im_height; j++) {
      int j_conv;
      int j_conv_min;
      j_conv_min = j - colFilterLength/2;
      total_weight = 1.0;
      blurredVal = 0.0;
      vrpn_bool renormalize = vrpn_FALSE;
      for (k = 0, j_conv = j_conv_min; k < colFilterLength; k++, j_conv++) {
        if (j_conv >= 0 && j_conv < im_height) {
          blurredVal += colFilter[k]*lineCopy[j_conv];
        } else {
          total_weight -= colFilter[k];
          renormalize = vrpn_TRUE;
        }
      }
      if (renormalize) {
        if (total_weight == 0) {
          printf("nmr_Util::blur: column blur: this shouldn't happen\n");
        }
        blurredVal /= total_weight;
      }
      im.setValue(i, j, blurredVal);
    }
  }
  delete [] lineCopy;
  delete [] rowFilter;
  delete [] colFilter;
}

// select value randomly from a uniform distribution between min and max
// call this twice to select a random point in an image
double nmr_Util::sampleUniformDistribution(double min, double max) {
#ifndef _WIN32
  return min + (max-min)*drand48();
#else 
  double randval = ((double)rand())/(double)RAND_MAX;
  return min + (max-min)*randval;
#endif
}

void nmr_Util::createGradientImages(nmb_Image &source,
                                  nmb_Image &grad_xIm, nmb_Image &grad_yIm)
{
  int i,j;
  double grad_x, grad_y;
  for (i = 0; i < source.width(); i++) {
    for (j = 0; j < source.height(); j++) {
      source.getGradient(i, j, grad_x, grad_y);
      grad_xIm.setValue(i,j, grad_x);
      grad_yIm.setValue(i,j, grad_y); 
    }
  }
}

double nmr_Util::computeMean(nmb_Image &im)
{
  double mean = 0;
  double num_samples_inv = 1.0/(im.width()*im.height());
  int i,j;
  for (i = 0; i < im.width(); i++) {
    for (j = 0; j < im.height(); j++) {
      mean += im.getValue(i,j);
    }
  }
  mean *= num_samples_inv;
  return mean;
}

double nmr_Util::computeVariance(nmb_Image &im, double mean)
{
  double num_samples_inv = 1.0/(im.width()*im.height());
  double var = 0;
  double diff;
  int i,j;
  for (i = 0; i < im.width(); i++) {
    for (j = 0; j < im.height(); j++) {
      diff = im.getValue(i,j) - mean;
      var += diff*diff;
    }
  }
  var *= num_samples_inv;
  return var;
}

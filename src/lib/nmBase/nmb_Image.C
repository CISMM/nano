/*===3rdtech===
  Copyright (c) 2001 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#include "Topo.h" // for TopoFile
#include "nmb_Image.h"
#include "nmb_TransformMatrix44.h"
#include "AbstractImage.h"
#include "nmb_ImgMagick.h"

#include <limits.h>
#ifdef _WIN32
#include <float.h> // for FLT_MAX
#endif

#define PAD_IMAGE_TO_POWER_OF_TWO

nmb_ImageBounds::nmb_ImageBounds() 
{
    nmb_ImageBounds(0.0, 0.0, 0.0, 0.0);
}

nmb_ImageBounds::nmb_ImageBounds(double x0,double y0,double x1,double y1){
    x[MIN_X_MIN_Y] = x0; x[MIN_X_MAX_Y] = x0;
    y[MIN_X_MIN_Y] = y0; y[MAX_X_MIN_Y] = y0;
    x[MAX_X_MIN_Y] = x1; x[MAX_X_MAX_Y] = x1;
    y[MIN_X_MAX_Y] = y1; y[MAX_X_MAX_Y] = y1;
}


//static 
int nmb_Image::deleteImage(nmb_Image *im)
{
  if (im->num_referencing_lists > 0) {
    return -1;
  }
  else {
    delete im;
    return 0;
  }
}

// virtual
nmb_Image::~nmb_Image (void) {

}

float nmb_Image::getValueInterpolated(double i, double j) const {
	int i_ip, j_ip;
	double i_fp, j_fp;

	if (i <= 0.5) {
		i_ip = 0;
		i_fp = 0.0;
	} else if (i >= (double)width()-0.5) {
		i_ip = width() - 2;
		i_fp = 1.0;
	} else {
		i_ip = (int)floor(i-0.5);
		i_fp = (i - 0.5) - (double)i_ip;
	}

	if (j <= 0.5) {
		j_ip = 0;
		j_fp = 0.0;
	} else if (j >= (double)height()-0.5) {
		j_ip = height() - 2;
		j_fp = 1.0;
	} else {
    	j_ip = (int)floor(j-0.5);
		j_fp = (j - 0.5) - (double)j_ip;
	}

    double val;
    val = (1-i_fp)*(1-j_fp)*getValue(i_ip, j_ip);
    val += (1-i_fp)*j_fp*getValue(i_ip, j_ip+1);
    val += i_fp*(1-j_fp)*getValue(i_ip+1,j_ip);
    val += i_fp*j_fp*getValue(i_ip+1,j_ip+1);
    return val;
}

float nmb_Image::getValueInterpolatedNZ(double i, double j) const {
        int i_ip, j_ip;
        double i_fp, j_fp;

        if (i <= 0.5) {
                i_ip = 0;
                i_fp = 0.0;
        } else if (i >= (double)width()-0.5) {
                i_ip = width() - 2;
                i_fp = 1.0;
        } else {
                i_ip = (int)floor(i-0.5);
                i_fp = (i - 0.5) - (double)i_ip;
        }

        if (j <= 0.5) {
                j_ip = 0;
                j_fp = 0.0;
        } else if (j >= (double)height()-0.5) {
                j_ip = height() - 2;
                j_fp = 1.0;
        } else {
        j_ip = (int)floor(j-0.5);
                j_fp = (j - 0.5) - (double)j_ip;
        }

    double val, v00, v01, v10, v11;
    v00 = getValue(i_ip, j_ip);
    v01 = getValue(i_ip, j_ip+1);
    v10 = getValue(i_ip+1,j_ip);
    v11 = getValue(i_ip+1,j_ip+1);
    if (v00 == 0 || v01 == 0 || v10 == 0 || v11 == 0) {
        val = 0;
    } else {
        val = (1-i_fp)*(1-j_fp)*getValue(i_ip, j_ip);
        val += (1-i_fp)*j_fp*getValue(i_ip, j_ip+1);
        val += i_fp*(1-j_fp)*getValue(i_ip+1,j_ip);
        val += i_fp*j_fp*getValue(i_ip+1,j_ip+1);
    }
    return val;
}

void nmb_Image::getGradient(int i, int j, double &grad_x, double &grad_y)
{
  if (i < 1 || j < 1 || i > width()-2 || j > height()-2) {
    grad_x = 0;
    grad_y = 0;
  } else {
    grad_x = (0.166666)*
           (getValue(i+1,j+1) - getValue(i-1,j+1) +
            getValue(i+1,j)   - getValue(i-1,j) +
            getValue(i+1,j-1) - getValue(i-1,j-1));
    grad_y = (0.166666)*
           (getValue(i+1,j+1) - getValue(i+1,j-1) +
            getValue(i,j+1)   - getValue(i,j-1) +
            getValue(i-1,j+1) - getValue(i-1,j-1));
  }
}

void nmb_Image::getGradient(double i, double j, double &grad_x, double &grad_y)
{
  if (i < 1 || j < 1 || i > width()-2 || j > height()-2) {
    grad_x = 0;
    grad_y = 0;
  } else {
    double v[8];
    v[0] = getValueInterpolated(i-1, j-1);
    v[1] = getValueInterpolated(i-1, j);
    v[2] = getValueInterpolated(i-1, j+1);
    v[3] = getValueInterpolated(i, j-1);
    v[4] = getValueInterpolated(i, j+1);
    v[5] = getValueInterpolated(i+1, j-1);
    v[6] = getValueInterpolated(i+1, j);
    v[7] = getValueInterpolated(i+1, j+1);
    grad_x = (0.166666)*
           (v[7] - v[2] +
            v[6] - v[1] +
            v[5] - v[0]);
    grad_y = (0.166666)*
           (v[7] + v[4] + v[2] 
          - v[5] - v[3] - v[0]);
  }
}

double nmb_Image::widthWorld() const {
    double dx,dy;
    dx = boundX(nmb_ImageBounds::MAX_X_MIN_Y) -
                boundX(nmb_ImageBounds::MIN_X_MIN_Y);
    dy = boundY(nmb_ImageBounds::MAX_X_MIN_Y) -
                boundY(nmb_ImageBounds::MIN_X_MIN_Y);
    return sqrt(dx*dx + dy*dy);
}

double nmb_Image::heightWorld() const {
    double dx,dy;
    dx = boundX(nmb_ImageBounds::MIN_X_MAX_Y) -
                boundX(nmb_ImageBounds::MIN_X_MIN_Y);
    dy = boundY(nmb_ImageBounds::MIN_X_MAX_Y) -
                boundY(nmb_ImageBounds::MIN_X_MIN_Y);
    return sqrt(dx*dx + dy*dy);
}

/*
these functions add unnecessary complexity
void nmb_Image::setWidthWorld(double newWidth, double centerX)
{
    d_dimensionUnknown = vrpn_FALSE;
    double dx, dy;
    dx = boundX(nmb_ImageBounds::MAX_X_MIN_Y) -
                boundX(nmb_ImageBounds::MIN_X_MIN_Y);
    dy = boundY(nmb_ImageBounds::MAX_X_MIN_Y) -
                boundY(nmb_ImageBounds::MIN_X_MIN_Y);
    double currWidth = sqrt(dx*dx + dy*dy);
    dx /= currWidth;
    dy /= currWidth;
    double a = centerX, b = 1.0-centerX;
    double centerWorldXMinY, centerWorldYMinY;
    double centerWorldXMaxY, centerWorldYMaxY;
    centerWorldXMinY = a*boundX(nmb_ImageBounds::MAX_X_MIN_Y) +
                       b*boundX(nmb_ImageBounds::MIN_X_MIN_Y);
    centerWorldYMinY = a*boundY(nmb_ImageBounds::MAX_X_MIN_Y) +
                       b*boundY(nmb_ImageBounds::MIN_X_MIN_Y);
    centerWorldXMaxY = a*boundX(nmb_ImageBounds::MAX_X_MAX_Y) +
                       b*boundX(nmb_ImageBounds::MIN_X_MAX_Y);
    centerWorldYMaxY = a*boundY(nmb_ImageBounds::MAX_X_MAX_Y) +
                       b*boundY(nmb_ImageBounds::MIN_X_MAX_Y);

    setBoundX(nmb_ImageBounds::MIN_X_MIN_Y, centerWorldXMinY - a*dx*newWidth);
    setBoundY(nmb_ImageBounds::MIN_X_MIN_Y, centerWorldYMinY - a*dy*newWidth);
    setBoundX(nmb_ImageBounds::MAX_X_MIN_Y, centerWorldXMinY + b*dx*newWidth);
    setBoundY(nmb_ImageBounds::MAX_X_MIN_Y, centerWorldYMinY + b*dy*newWidth);
    setBoundX(nmb_ImageBounds::MIN_X_MAX_Y, centerWorldXMaxY - a*dx*newWidth);
    setBoundY(nmb_ImageBounds::MIN_X_MAX_Y, centerWorldYMaxY - a*dy*newWidth);
    setBoundX(nmb_ImageBounds::MAX_X_MAX_Y, centerWorldXMaxY + b*dx*newWidth);
    setBoundY(nmb_ImageBounds::MAX_X_MAX_Y, centerWorldYMaxY + b*dy*newWidth);
}

void nmb_Image::setHeightWorld(double newHeight, double centerY)
{
    d_dimensionUnknown = vrpn_FALSE;
    double dx, dy;
    dx = boundX(nmb_ImageBounds::MIN_X_MAX_Y) -
                boundX(nmb_ImageBounds::MIN_X_MIN_Y);
    dy = boundY(nmb_ImageBounds::MIN_X_MAX_Y) -
                boundY(nmb_ImageBounds::MIN_X_MIN_Y);
    double currHeight = sqrt(dx*dx + dy*dy);
    dx /= currHeight;
    dy /= currHeight;
    double a = centerY, b = 1.0-centerY;
    double centerWorldXMinX, centerWorldYMinX;
    double centerWorldXMaxX, centerWorldYMaxX;
    centerWorldXMinX = a*boundX(nmb_ImageBounds::MIN_X_MAX_Y) +
                       b*boundX(nmb_ImageBounds::MIN_X_MIN_Y);
    centerWorldYMinX = a*boundY(nmb_ImageBounds::MIN_X_MAX_Y) +
                       b*boundY(nmb_ImageBounds::MIN_X_MIN_Y);
    centerWorldXMaxX = a*boundX(nmb_ImageBounds::MAX_X_MAX_Y) +
                       b*boundX(nmb_ImageBounds::MAX_X_MIN_Y);
    centerWorldYMaxX = a*boundY(nmb_ImageBounds::MAX_X_MAX_Y) +
                       b*boundY(nmb_ImageBounds::MAX_X_MIN_Y);

    setBoundX(nmb_ImageBounds::MIN_X_MIN_Y, centerWorldXMinX - a*dx*newHeight);
    setBoundY(nmb_ImageBounds::MIN_X_MIN_Y, centerWorldYMinX - a*dy*newHeight);
    setBoundX(nmb_ImageBounds::MIN_X_MAX_Y, centerWorldXMinX + b*dx*newHeight);
    setBoundY(nmb_ImageBounds::MIN_X_MAX_Y, centerWorldYMinX + b*dy*newHeight);
    setBoundX(nmb_ImageBounds::MAX_X_MIN_Y, centerWorldXMaxX - a*dx*newHeight);
    setBoundY(nmb_ImageBounds::MAX_X_MIN_Y, centerWorldYMaxX - a*dy*newHeight);
    setBoundX(nmb_ImageBounds::MAX_X_MAX_Y, centerWorldXMaxX + b*dx*newHeight);
    setBoundY(nmb_ImageBounds::MAX_X_MAX_Y, centerWorldYMaxX + b*dy*newHeight);
}
*/

void nmb_Image::setAcquisitionDimensions(double distX, double distY)
{
  d_acquisitionDistX = distX;
  d_acquisitionDistY = distY;
  return;
}

void nmb_Image::getAcquisitionDimensions(double &distX, double &distY)
{
  distX = d_acquisitionDistX;
  distY = d_acquisitionDistY;
  return;
}

/** This function assumes that (i,j) are coordinates for the basis vectors (u,v)
   where u = pnt_max_i_min_j - pnt_min_i_min_j
         v = pnt_min_i_max_j - pnt_min_i_min_j
   pnt_max_i_min_j, pnt_min_i_min_j, pnt_min_i_max_j are the world positions of
   three of the corners of the image
*/
void nmb_Image::pixelToWorld(const double i, const double j,
                  double &x, double &y) const {
        // just bilinear interpolation:
        double y_frac = j/((double)height());
        double x_frac = i/((double)width());
        double x_min = boundX(nmb_ImageBounds::MIN_X_MIN_Y)*(1.0-y_frac) +
             boundX(nmb_ImageBounds::MIN_X_MAX_Y)*(y_frac);
        double x_max = boundX(nmb_ImageBounds::MAX_X_MIN_Y)*(1.0-y_frac) +
             boundX(nmb_ImageBounds::MAX_X_MAX_Y)*(y_frac);
        x = x_min*(1.0-x_frac) + x_max*(x_frac);
        double y_min = boundY(nmb_ImageBounds::MIN_X_MIN_Y)*(1.0-y_frac) +
             boundY(nmb_ImageBounds::MIN_X_MAX_Y)*(y_frac);
        double y_max = boundY(nmb_ImageBounds::MAX_X_MIN_Y)*(1.0-y_frac) +
             boundY(nmb_ImageBounds::MAX_X_MAX_Y)*(y_frac);
        y = y_min*(1.0-x_frac) + y_max*(x_frac);
}

/** This function assumes that (i,j) are coordinates for the basis vectors (u,v)
   where u = pnt_max_i_min_j - pnt_min_i_min_j
         v = pnt_min_i_max_j - pnt_min_i_min_j
   pnt_max_i_min_j, pnt_min_i_min_j, pnt_min_i_max_j are the world positions of
   three of the corners of the image
*/
void nmb_Image::worldToPixel(const double x, const double y,
                  double &i, double &j) const {
        // project x,y onto i,j coordinate axes
        double x_diff = boundX(nmb_ImageBounds::MAX_X_MIN_Y) -
                        boundX(nmb_ImageBounds::MIN_X_MIN_Y);
        double y_diff = boundY(nmb_ImageBounds::MAX_X_MIN_Y) -
                        boundY(nmb_ImageBounds::MIN_X_MIN_Y);
        double temp = (x - boundX(nmb_ImageBounds::MIN_X_MIN_Y))*x_diff +
                      (y - boundY(nmb_ImageBounds::MIN_X_MIN_Y))*y_diff;

        double x_frac = temp/(x_diff*x_diff + y_diff*y_diff);
        i = x_frac*(double)width();

        x_diff = boundX(nmb_ImageBounds::MIN_X_MAX_Y) -
                 boundX(nmb_ImageBounds::MIN_X_MIN_Y);
        y_diff = boundY(nmb_ImageBounds::MIN_X_MAX_Y) -
                 boundY(nmb_ImageBounds::MIN_X_MIN_Y);
        temp = (x - boundX(nmb_ImageBounds::MIN_X_MIN_Y))*x_diff +
               (y - boundY(nmb_ImageBounds::MIN_X_MIN_Y))*y_diff;
        double y_frac = temp/(x_diff*x_diff + y_diff*y_diff);
        j = y_frac*(double)height();
}

/**
 returns transformation matrix that takes points in the world which lie in the
 image into points in
 image coordinates with (x, y, z) in the range [0..widthWorld(),0..heightWorld]

 The matrix is in column major order so one may use this for the texture matrix
 in openGL
*/

void nmb_Image::getWorldToImageTransform(double *matrix44)
{
  /* What this function is doing:
      If the worldToImage matrix has been set then we just return that but
      otherwise we construct an orthographic projection matrix from our 
      knowledge of the positions of three of the corners in the world.

      Start with the system:
        (0,0) = M*(x00, y00, 1)
        (1,0) = M*(x10, y10, 1)
        (0,1) = M*(x01, y01, 1)

        where M is a matrix | a b c |
                            | d e f |
        In each equation:
        the left side represents the position in normalized image coordinates
        and the vector to the right of M represents the corresponding points
        in world coordinates.

        Given x00, y00, x10, y10, x01, y01, 
        solve for a,b,c,d,e,f and stuff these into a 4x4 matrix in the 
        order col0row0-3, col1row0-3, col2row0-3, col3row0-3
  */

  if (d_worldToImageMatrixSet) {
    int i; 
    for (i = 0; i < 16; i++){
      matrix44[i] = d_worldToImageMatrix[i];
    }
  } else {

    double a, b, c, d, e, f;
    double x00, y00, x10, y10, x01, y01;
    x00 = boundX(nmb_ImageBounds::MIN_X_MIN_Y);
    y00 = boundY(nmb_ImageBounds::MIN_X_MIN_Y);
    x10 = boundX(nmb_ImageBounds::MAX_X_MIN_Y);
    y10 = boundY(nmb_ImageBounds::MAX_X_MIN_Y);
    x01 = boundX(nmb_ImageBounds::MIN_X_MAX_Y);
    y01 = boundY(nmb_ImageBounds::MIN_X_MAX_Y);
    
    double det_inv;
    det_inv = 1.0/((x10-x00)*(y01-y00) - (y10-y00)*(x01-x00));
    a = (y01-y00)*det_inv;
    b = (x00-x01)*det_inv;
    c = -a*x00 - b*y00;

    d = (y00 - y10)*det_inv;
    e = (x10 - x00)*det_inv;
    f = -d*x00 - e*y00;

    // first row:
    matrix44[0] = a;
    matrix44[4] = b;
    matrix44[8] = 0.0;
    matrix44[12] = c;
    // second row:
    matrix44[1] = d;
    matrix44[5] = e;
    matrix44[9] = 0.0;
    matrix44[13] = f;

    matrix44[2] = 0.0;
    matrix44[6] = 0.0;

    // if you take out the following 6 lines then the result will be the
    // transformation to go to normalized image coordinates
/*
    double imWidth = sqrt((x10-x00)*(x10-x00) + (y10-y00)*(y10-y00));
    double imHeight = sqrt((x01-x00)*(x01-x00) + (y01-y00)*(y01-y00));
    matrix44[0] *= imWidth;
    matrix44[4] *= imWidth;
    matrix44[12] *= imWidth;
    matrix44[1] *= imHeight;
    matrix44[5] *= imHeight;
    matrix44[13] *= imHeight;
*/

    matrix44[10] = 1.0;
    matrix44[14] = 0.0;

    matrix44[3] = 0.0;
    matrix44[7] = 0.0;
    matrix44[11] = 0.0;
    matrix44[15] = 1.0;
  }
}

void nmb_Image::getWorldToImageTransform(nmb_TransformMatrix44 &xform)
{
  double matrix[16];
  getWorldToImageTransform(matrix);
  xform.setMatrix(matrix);
}

void nmb_Image::getImageToTextureTransform(double *matrix44)
{
  int texwidth = width() + borderXMin() + borderXMax();
  int texheight = height() + borderYMin() + borderYMax();
  double scaleFactorX = (double)(width())/(double)texwidth;
  double scaleFactorY = (double)(height())/(double)texheight;
  double borderOffsetX = (double)borderXMin()/(double)texwidth;
  double borderOffsetY = (double)borderYMin()/(double)texheight;

  // image_to_texture
  matrix44[0] = scaleFactorX;
  matrix44[4] = 0.0;
  matrix44[8] = 0.0;
  matrix44[12] = borderOffsetX;

  matrix44[1] = 0.0;
  matrix44[5] = scaleFactorY;
  matrix44[9] = 0.0;
  matrix44[13] = borderOffsetY;
  
  matrix44[2] = 0.0;
  matrix44[6] = 0.0;
  matrix44[10] = 1.0;
  matrix44[14] = 0.0;
  
  matrix44[3] = 0.0;
  matrix44[7] = 0.0;
  matrix44[11] = 0.0;
  matrix44[15] = 1.0;
}

void nmb_Image::getImageToTextureTransform(nmb_TransformMatrix44 &xform)
{
  double matrix[16];
  getImageToTextureTransform(matrix);
  xform.setMatrix(matrix);
}

void nmb_Image::setWorldToImageTransform(double *matrix44)
{
    d_worldToImageMatrixSet = VRPN_TRUE;
    d_dimensionUnknown = vrpn_FALSE;
    int i;
    for (i = 0; i < 16; i++){
        d_worldToImageMatrix[i] = matrix44[i];
    }

    // now we need to set the boundary of the image to keep any
    // boundary state consistent with the worldToImage transformation

    nmb_TransformMatrix44 worldToImage;
    worldToImage.setMatrix(matrix44);

    double width = 1.0;
    double height = 1.0;

    if (worldToImage.hasInverse()) {
      double imagePnt[4] = {0.0, 0.0, 0.0, 1.0}, worldPnt[4];
      // (0,0) -> ?
      worldToImage.invTransform(imagePnt, worldPnt);
      setBoundX(nmb_ImageBounds::MIN_X_MIN_Y, worldPnt[0]);
      setBoundY(nmb_ImageBounds::MIN_X_MIN_Y, worldPnt[1]);
      imagePnt[0] = width;
      // (width,0) -> ?
      worldToImage.invTransform(imagePnt, worldPnt);
      setBoundX(nmb_ImageBounds::MAX_X_MIN_Y, worldPnt[0]);
      setBoundY(nmb_ImageBounds::MAX_X_MIN_Y, worldPnt[1]);
      imagePnt[0] = 0.0;
      imagePnt[1] = height;
      // (0,height) -> ?
      worldToImage.invTransform(imagePnt, worldPnt);
      setBoundX(nmb_ImageBounds::MIN_X_MAX_Y, worldPnt[0]);
      setBoundY(nmb_ImageBounds::MIN_X_MAX_Y, worldPnt[1]);
      imagePnt[0] = width;
      // (width,height) -> ?
      worldToImage.invTransform(imagePnt, worldPnt);
      setBoundX(nmb_ImageBounds::MAX_X_MAX_Y, worldPnt[0]);
      setBoundY(nmb_ImageBounds::MAX_X_MAX_Y, worldPnt[1]);
    } else {
      fprintf(stderr, "nmb_Image::setWorldToImageTransform:"
            " Error: non-invertible transformation\n");
    }
}

void nmb_Image::setWorldToImageTransform(nmb_TransformMatrix44 &xform)
{
  double matrix[16];
  xform.getMatrix(matrix);
  setWorldToImageTransform(matrix);
}

double nmb_Image::areaInWorld()
{
  getBounds(d_imagePosition);
  return d_imagePosition.area();
}

vrpn_bool nmb_Image::dimensionUnknown()
{
  return d_dimensionUnknown;
}

const int nmb_ImageGrid::num_export_formats = 5;
const char *nmb_ImageGrid::export_formats_list[] = {	"ThermoMicroscopes",
                                 		"TIFF Image",
                                 		"PPM Image",
                                 		"Other Image",
                                 		"Text(MathCAD)",
                                 		"SPIP",
                                 		"UNCA Image" };
const nmb_ImageGrid::FileExportingFunction 
	nmb_ImageGrid::file_exporting_function[] = 
				{nmb_ImageGrid::writeTopoFile,
                                 nmb_ImageGrid::writeTIFFile,
                                 nmb_ImageGrid::writePPMFile,
                                 nmb_ImageGrid::writeOtherImageFile,
                                 nmb_ImageGrid::writeTextFile,
                                 nmb_ImageGrid::writeSPIPFile,
                                 nmb_ImageGrid::writeUNCAFile};

BCPlane *nmb_ImageGrid::s_openFilePlane = NULL;
BCGrid *nmb_ImageGrid::s_openFileGrid = NULL;
TopoFile nmb_ImageGrid::s_openFileTopoHeader;

/* assumes that the file contains a single grid but possibly more than
   one plane - we wouldn't know how to read it if this weren't the case
   anyway (returns -1 if such an error is detected)
*/
// static 
int nmb_ImageGrid::openFile(const char *filename)
{
  s_openFileGrid = new BCGrid();
  if (s_openFileGrid->loadFile(filename, s_openFileTopoHeader) == NULL) {
    delete s_openFileGrid;
    s_openFileGrid = NULL;
    return -1;
  }
  s_openFilePlane = s_openFileGrid->head();
  return 0;
}

/* gets the next plane out of the last file opened or NULL if there are no
   more -
   note: the user is responsible for deleting stuff that gets returned by
   this method - we replicate the grid to make this job easier
*/
// static
nmb_ImageGrid *nmb_ImageGrid::getNextImage()
{
  if (s_openFilePlane == NULL) return NULL;
  BCGrid *g = new BCGrid(s_openFileGrid->numX(), s_openFileGrid->numY(),
                       s_openFileGrid->minX(), s_openFileGrid->maxX(),
                       s_openFileGrid->minY(), s_openFileGrid->maxY(), 
                         READ_FILE);
  g->setMinX(s_openFileGrid->minX());
  g->setMaxX(s_openFileGrid->maxX());
  g->setMinY(s_openFileGrid->minY());
  g->setMaxY(s_openFileGrid->maxY());

  g->addPlaneCopy(s_openFilePlane);

/*
  // now we need to copy the values because addPlaneCopy doesn't copy them
  for (int i = 0; i < s_openFilePlane->numX(); i++) {
    for (int j = 0; j < s_openFilePlane->numY(); j++) {
      g->head()->setValue(i, j, s_openFilePlane->value(i, j));
    }
  }
*/

  nmb_ImageGrid *result = new nmb_ImageGrid(g);
  result->setTopoFileInfo(s_openFileTopoHeader);

  s_openFilePlane = s_openFilePlane->next();
  if (s_openFilePlane == NULL) {
    delete s_openFileGrid;
    s_openFileGrid = NULL;
    s_openFilePlane = NULL;
  }
  return result;
}

nmb_ImageGrid::nmb_ImageGrid(const char *name, const char *units, 
	short x, short y):
            nmb_Image(),
            units_x("nm"), units_y("nm"),
            d_imagePositionSet(vrpn_FALSE)
{
    BCString name_str(name), units_str(units);
    grid = new BCGrid(x, y, 0.0, 1.0, 0.0, 1.0, READ_FILE);
    plane = grid->addNewPlane(name_str, units_str, 0);
    min_x_set = SHRT_MAX; min_y_set = SHRT_MAX;
    max_x_set = -SHRT_MAX; max_y_set = -SHRT_MAX;
    for (int i = 0; i < numExportFormats(); i++){
        BCString name = exportFormatType(i);
        formatNames.addEntry(name);
    }
    d_acquisitionDistX = x;
    d_acquisitionDistY = y;

}

nmb_ImageGrid::nmb_ImageGrid(BCPlane *p):nmb_Image(),
    units_x("nm"), units_y("nm"),
    d_imagePositionSet(vrpn_FALSE)
{
    // WARNING: assumes (non-zero value <==> value was set) as
    // did BCPlane::findValidDataRange()

    plane = p;
    grid = NULL;
    min_x_set = SHRT_MAX; min_y_set = SHRT_MAX;
     max_x_set = -SHRT_MAX; max_y_set = -SHRT_MAX;

    int i,j;
    for (i = 0; i < plane->numX(); i++){
        for (j = 0; j < plane->numY(); j++){
            if (plane->value(i,j) != 0.0){
                min_x_set = MIN(min_x_set, i);
                max_x_set = MAX(max_x_set, i);
                min_y_set = MIN(min_y_set, j);
                max_y_set = MAX(max_y_set, j);
            }
        }
    }
    for (i = 0; i < numExportFormats(); i++){
        BCString name = exportFormatType(i);
        formatNames.addEntry(name);
    }

    d_imagePosition.setX(nmb_ImageBounds::MIN_X_MIN_Y, plane->minX());
    d_imagePosition.setX(nmb_ImageBounds::MIN_X_MAX_Y, plane->minX());
    d_imagePosition.setX(nmb_ImageBounds::MAX_X_MIN_Y, plane->maxX());
    d_imagePosition.setX(nmb_ImageBounds::MAX_X_MAX_Y, plane->maxX());
    d_imagePosition.setY(nmb_ImageBounds::MIN_X_MIN_Y, plane->minY());
    d_imagePosition.setY(nmb_ImageBounds::MIN_X_MAX_Y, plane->maxY());
    d_imagePosition.setY(nmb_ImageBounds::MAX_X_MIN_Y, plane->minY());
    d_imagePosition.setY(nmb_ImageBounds::MAX_X_MAX_Y, plane->maxY());
    d_dimensionUnknown = vrpn_FALSE;
    d_DAC_scale = p->tm_scale;
    d_DAC_offset = p->tm_offset;
    d_acquisitionDistX = plane->numX();
    d_acquisitionDistY = plane->numY();
}

nmb_ImageGrid::nmb_ImageGrid(BCGrid *g):nmb_Image(),
    units_x("nm"), units_y("nm"),
    d_imagePositionSet(vrpn_FALSE)
{
    // WARNING: assumes (non-zero value <==> value was set) as
    // did BCPlane::findValidDataRange()

    plane = g->head();
    grid = g;
    min_x_set = SHRT_MAX; min_y_set = SHRT_MAX;
     max_x_set = -SHRT_MAX; max_y_set = -SHRT_MAX;

    int i,j;
    for (i = 0; i < plane->numX(); i++){
        for (j = 0; j < plane->numY(); j++){
            if (plane->value(i,j) != 0.0){
                min_x_set = MIN(min_x_set, i);
                max_x_set = MAX(max_x_set, i);
                min_y_set = MIN(min_y_set, j);
                max_y_set = MAX(max_y_set, j);
            }
        }
    }
    for (i = 0; i < numExportFormats(); i++){
        BCString name = exportFormatType(i);
        formatNames.addEntry(name);
    }

    d_imagePosition.setX(nmb_ImageBounds::MIN_X_MIN_Y, plane->minX());
    d_imagePosition.setX(nmb_ImageBounds::MIN_X_MAX_Y, plane->minX());
    d_imagePosition.setX(nmb_ImageBounds::MAX_X_MIN_Y, plane->maxX());
    d_imagePosition.setX(nmb_ImageBounds::MAX_X_MAX_Y, plane->maxX());
    d_imagePosition.setY(nmb_ImageBounds::MIN_X_MIN_Y, plane->minY());
    d_imagePosition.setY(nmb_ImageBounds::MIN_X_MAX_Y, plane->maxY());
    d_imagePosition.setY(nmb_ImageBounds::MAX_X_MIN_Y, plane->minY());
    d_imagePosition.setY(nmb_ImageBounds::MAX_X_MAX_Y, plane->maxY());
    d_dimensionUnknown = vrpn_FALSE;
    d_DAC_scale = plane->tm_scale;
    d_DAC_offset = plane->tm_offset;
    d_acquisitionDistX = plane->numX();
    d_acquisitionDistY = plane->numY();
}


nmb_ImageGrid::nmb_ImageGrid(nmb_Image *im):
    d_imagePositionSet(vrpn_FALSE)
{
  int i,j;
  grid = new BCGrid(im->width(), im->height(), 0.0, 1.0, 0.0, 1.0, READ_FILE);
  plane = grid->addNewPlane(*(im->name()), *(im->unitsValue()), 0);
  min_x_set = SHRT_MAX; min_y_set = SHRT_MAX;
  max_x_set = -SHRT_MAX; max_y_set = -SHRT_MAX;
  for (i = 0; i < numExportFormats(); i++){
      BCString name = exportFormatType(i);
      formatNames.addEntry(name);
  }

  units_x = *(im->unitsX());
  units_y = *(im->unitsY());

  for (i = 0; i < width(); i++) {
    for (j = 0; j < height(); j++) {
      setValue(i,j, im->getValue(i,j));
    }
  }
  d_units_scale = im->valueScale();
  d_units_offset = im->valueOffset();
  im->validDataRange(&max_y_set, &min_x_set, &min_y_set, &max_x_set);
  im->getTopoFileInfo(d_topoFileDefaults);
  im->getBounds(d_imagePosition);
  d_imagePositionSet = vrpn_TRUE;
  d_DAC_scale = im->valueScaleDAC();
  d_DAC_offset = im->valueOffsetDAC();
  d_dimensionUnknown = im->dimensionUnknown();
  im->getAcquisitionDimensions(d_acquisitionDistX, d_acquisitionDistY);
}

nmb_ImageGrid::~nmb_ImageGrid()
{   
    if (num_referencing_lists != 0) {
      fprintf(stderr, "nmb_ImageGrid::~nmb_ImageGrid: Programmer Error: "
                      " creating a dangling pointer\n");
      exit(-1);
    }
    if (grid) {
        delete grid;
    }
}

int nmb_ImageGrid::width() const {return plane->numX();}

int nmb_ImageGrid::height() const {return plane->numY();}

float nmb_ImageGrid::getValue(int i, int j) const
             {return plane->value(i,j);}

float nmb_ImageGrid::maxValue() {return plane->maxValue();}

float nmb_ImageGrid::minValue() {return plane->minValue();}

int nmb_ImageGrid::normalize()
{
  int i,j;
  float min = minValue();
  float max = maxValue();
  float range = max-min;
  if (range == 0) return -1;
  float inv_range = 1.0/range;

  /* 
   data_value = array_value*d_units_scale + d_units_offset
   array_value_new = (array_value-min)*inv_range
   -->
   data_value = (array_value_new/inv_range+min)*d_units_scale + d_units_offset
   data_value = array_value_new*(d_units_scale/inv_range) +
                                 (min*d_units_scale + d_units_offset)
   d_units_scale_new = d_units_scale/inv_range
   d_units_offset_new = d_units_offset + min*d_units_scale
  */
  d_units_offset += min*d_units_scale;
  d_units_scale *= range;

  for (i = 0; i < width(); i++) {
    for (j = 0; j < height(); j++) {
      setValue(i,j, inv_range*(getValue(i,j)-min));
    }
  }
  return 0;
}

float nmb_ImageGrid::maxValidValue() {
    short top, left, bottom, right;
    if (validDataRange(&top, &left, &bottom, &right)) {
        return 0;
    }
    if ((top == height()-1) && (bottom == 0) && 
        (right == width()-1) && (left == 0)) {
        return maxValue();
    }

    fprintf(stderr, "nmb_ImageGrid::maxValidValue:: Warning, "
           "this function should be implemented more efficiently\n");

    int i,j;
    float result = getValue(left, bottom);
    float val;
    for (i = left; i <= right; i++) {
        for (j = bottom; j <= top; j++) {
            val = getValue(i,j);
            if (val > result) result = val;
        }
    }
    return result;
}

float nmb_ImageGrid::minValidValue() {
    short top, left, bottom, right;
    float result;
    if (validDataRange(&top, &left, &bottom, &right)) {
        return 0;
    }


    if ((top == height()-1) && (bottom == 0) && 
        (right == width()-1) && (left == 0)) {
        result = minValue();
    } else {

      fprintf(stderr, "nmb_ImageGrid::minValidValue:: Warning, "
              "this function should be implemented more efficiently\n");

      int i,j;
      result = getValue(left, bottom);
      float val;
      for (i = left; i <= right; i++) {
          for (j = bottom; j <= top; j++) {
              val = getValue(i,j);
              if (val < result) result = val;
          }
      }  
    }

    return result;
}

float nmb_ImageGrid::maxNonZeroValue() {
    return maxValidValue();
}

float nmb_ImageGrid::minNonZeroValue() {
    return plane->minNonZeroValue();
}

void nmb_ImageGrid::setValue(int i, int j, float val)
{
     plane->setValue(i,j,val);
     min_x_set = MIN(min_x_set, i);
     max_x_set = MAX(max_x_set, i);
     min_y_set = MIN(min_y_set, j);
     max_y_set = MAX(max_y_set, j);
}

int nmb_ImageGrid::validDataRange(short* o_top, short* o_left,
                                   short* o_bottom, short*o_right){
     int result = 0;
     // if we are not the allocator then assume someone else is setting
     // values directly in the plane object without going through us and
     // so take the valid region from what the plane says
     if (!grid) {
         result = plane->findValidDataRange(o_top, o_left, o_bottom, o_right);
     }
     // if no valid data:
     else if (min_y_set > max_y_set || min_x_set > max_x_set) {
         result = -1;
     } else {
         // otherwise at least one valid data point:
         *o_bottom = min_y_set; *o_top = max_y_set;
         *o_left = min_x_set; *o_right = max_x_set;
     }
     return result;
}

float nmb_ImageGrid::minAttainableValue() const {
           return plane->minAttainableValue();}

float nmb_ImageGrid::maxAttainableValue() const {
           return plane->maxAttainableValue();}

double nmb_ImageGrid::boundX(nmb_ImageBounds::ImageBoundPoint ibp) const
{
    if (d_imagePositionSet) {
      return d_imagePosition.getX(ibp);
    } else {
      if (ibp == nmb_ImageBounds::MIN_X_MIN_Y || 
          ibp == nmb_ImageBounds::MIN_X_MAX_Y)
        return plane->minX();
      else
        return plane->maxX();
    }
}

double nmb_ImageGrid::boundY(nmb_ImageBounds::ImageBoundPoint ibp) const
{
    if (d_imagePositionSet) {
      return d_imagePosition.getY(ibp);
    } else {
      if (ibp == nmb_ImageBounds::MIN_X_MIN_Y ||
          ibp == nmb_ImageBounds::MAX_X_MIN_Y) 
        return plane->minY();
      else 
        return plane->maxY();
    }
}

void nmb_ImageGrid::setBoundX(nmb_ImageBounds::ImageBoundPoint ibp, double x)
{
    if (!d_imagePositionSet) {
      getBounds(d_imagePosition);
    }
    d_imagePositionSet = vrpn_TRUE;
    d_imagePosition.setX(ibp, x);
    // maintain some kind of backward consistency
    plane->_grid->setMinX(min(plane->minX(), x));
    plane->_grid->setMaxX(max(plane->maxX(), x));
    d_dimensionUnknown = vrpn_FALSE;
}

void nmb_ImageGrid::setBoundY(nmb_ImageBounds::ImageBoundPoint ibp, double y)
{
    if (!d_imagePositionSet) {
      getBounds(d_imagePosition);
    }
    d_imagePositionSet = vrpn_TRUE;
    d_imagePosition.setY(ibp, y);
    // maintain some kind of backward consistency
    plane->_grid->setMinY(min(plane->minY(), y));
    plane->_grid->setMaxY(max(plane->maxY(), y));
    d_dimensionUnknown = vrpn_FALSE;
}

void nmb_ImageGrid::getBounds(nmb_ImageBounds &ib)  const
{
    if (d_imagePositionSet) {
      ib = d_imagePosition;
    } else {
      double xmin, xmax, ymin, ymax;
      xmin = plane->minX();
      xmax = plane->maxX();
      ymin = plane->minY();
      ymax = plane->maxY();
      ib.setX(nmb_ImageBounds::MIN_X_MIN_Y, xmin);
      ib.setY(nmb_ImageBounds::MIN_X_MIN_Y, ymin);
      ib.setX(nmb_ImageBounds::MIN_X_MAX_Y, xmin);
      ib.setY(nmb_ImageBounds::MIN_X_MAX_Y, ymax);
      ib.setX(nmb_ImageBounds::MAX_X_MIN_Y, xmax);
      ib.setY(nmb_ImageBounds::MAX_X_MIN_Y, ymin);
      ib.setX(nmb_ImageBounds::MAX_X_MAX_Y, xmax);
      ib.setY(nmb_ImageBounds::MAX_X_MAX_Y, ymax);
    }
}

void nmb_ImageGrid::setBounds(const nmb_ImageBounds &ib)
{
    d_imagePosition = ib;
    d_imagePositionSet = vrpn_TRUE;
    plane->_grid->setMinX(ib.minX());
    plane->_grid->setMinY(ib.minY());
    plane->_grid->setMaxX(ib.maxX());
    plane->_grid->setMaxY(ib.maxY());
    d_dimensionUnknown = vrpn_FALSE;
}

BCString *nmb_ImageGrid::name() {return plane->name();}
BCString *nmb_ImageGrid::unitsValue() {return plane->units();}
BCString *nmb_ImageGrid::unitsX() {return &units_x;}
BCString *nmb_ImageGrid::unitsY() {return &units_y;}

void nmb_ImageGrid::setTopoFileInfo(TopoFile &tf)
{
    d_topoFileDefaults = tf;
}

void nmb_ImageGrid::getTopoFileInfo(TopoFile &tf)
{
    tf = d_topoFileDefaults;
}


void *nmb_ImageGrid::pixelData() { return (void *)(plane->flatValueArray());}

int nmb_ImageGrid::borderXMin() { return plane->_borderXMin;}
int nmb_ImageGrid::borderXMax() { return plane->_borderXMax;}
int nmb_ImageGrid::borderYMin() { return plane->_borderYMin;}
int nmb_ImageGrid::borderYMax() { return plane->_borderYMax;}

int nmb_ImageGrid::arrayLength() 
{ 
   return ((borderXMin()+width()+borderXMax())*
           (borderYMin()+height()+borderYMax()));
}


nmb_PixelType nmb_ImageGrid::pixelType() {return NMB_FLOAT32;}

int nmb_ImageGrid::numExportFormats() {return num_export_formats;}

nmb_ListOfStrings *nmb_ImageGrid::exportFormatNames()
                {return &formatNames;}

const char *nmb_ImageGrid::exportFormatType(int type)
            {return (const char *)(export_formats_list[type]);}

int nmb_ImageGrid::exportToFile(FILE *f, const char *export_type,
                                const char * filename){

    int my_export_type;
    for (my_export_type = 0; my_export_type < numExportFormats(); 
         my_export_type++){
	if (strcmp(export_type, exportFormatType(my_export_type)) == 0)
	    break;
    }
    // if didn't find a match to export_type
    if (my_export_type == numExportFormats()) {
	fprintf(stderr, "nmb_ImageGrid::Error, unknown file type: %s\n",
	    export_type);
	return -1;
    }
    else {  // we have a function for exporting this type
	if (file_exporting_function[my_export_type](f, this, filename)) {
	    fprintf(stderr, "nmb_ImageGrid::Error writing file of type %s\n",
		export_type);
	    return -1;
	}
	return 0;
    }
}

//static 
int nmb_ImageGrid::writeTopoFile(FILE *file, nmb_ImageGrid *im, const char * )
{
 //what about microscope->d_topoFile? - should somehow be using this info here
    TopoFile tf = im->d_topoFileDefaults;
    tf.imageToTopoData((nmb_Image *)im);
    if (tf.writeTopoFile(file) < 0) {
	//error occured
	return -1;
    }
    return 0;
}

//static
int nmb_ImageGrid::writeTextFile(FILE *file, nmb_ImageGrid *im, const char * )
{
    if (im->plane->_grid->writeTextFile(file, im->plane)) {
	return -1;
    }
    return 0;
}

//static 
int nmb_ImageGrid::writePPMFile(FILE *file, nmb_ImageGrid *im, const char * filename)
{
    if (im->plane->_grid->writeImageFile(file, im->plane, filename, "PPM")) {
	return -1;
    }
    return 0;
}

//static 
int nmb_ImageGrid::writeTIFFile(FILE *file, nmb_ImageGrid *im, const char * filename)
{
    if (im->plane->_grid->writeImageFile(file, im->plane, filename, "TIF")) {
	return -1;
    }
    return 0;
}

//static 
int nmb_ImageGrid::writeOtherImageFile(FILE *file, nmb_ImageGrid *im, const char * filename)
{
    // User specifies the image type using filename's extention (.jpg, .bmp)
    if (im->plane->_grid->writeImageFile(file, im->plane, filename, NULL)) {
	return -1;
    }
    return 0;
}

//static 
int nmb_ImageGrid::writeSPIPFile(FILE *file, nmb_ImageGrid *im, const char * )
{
    if (im->plane->_grid->writeSPIPFile(file, im->plane)) {
	return -1;
    }
    return 0;
}

//static 
int nmb_ImageGrid::writeUNCAFile(FILE *file, nmb_ImageGrid *im, const char * )
{
    if (im->plane->_grid->writeUNCAFile(file, im->plane)) {
	return -1;
    }
    return 0;
}

const int nmb_ImageArray::num_export_formats = 1;

const char *nmb_ImageArray::export_formats_list[] = {"TIFF"};

const nmb_ImageArray::FileExportingFunction
        nmb_ImageArray::file_exporting_function[] = {
               nmb_ImageArray::exportToTIFF};

nmb_ImageArray::nmb_ImageArray(const char *name,
                                          const char * /*units*/,
                                          short x, short y,
                                          nmb_PixelType pixType):
        nmb_Image(), 
        fData(NULL),ucData(NULL), usData(NULL), data(NULL),
        num_x(x), num_y(y), d_borderXMin(1), d_borderXMax(1),
        d_borderYMin(1), d_borderYMax(1),
        units_x("none"), units_y("none"), units("ADC"),
        my_name(name),
        d_minNonZeroValueComputed(VRPN_FALSE),
        d_minNonZeroValue(0),
        d_minValueComputed(VRPN_FALSE),
        d_minValue(0),
        d_maxValueComputed(VRPN_FALSE),
        d_maxValue(0),
        d_minValidValueComputed(VRPN_FALSE),
        d_minValidValue(0),
        d_maxValidValueComputed(VRPN_FALSE),
        d_maxValidValue(0),
        d_pixelType(pixType)
{
    min_x_set = SHRT_MAX; min_y_set = SHRT_MAX;
    max_x_set = -SHRT_MAX; max_y_set = -SHRT_MAX;
    int array_size = 0;
    // pick a border that preserves 32-bit-alignment of rows
    // and pads the size up to a power of 2

#ifdef PAD_IMAGE_TO_POWER_OF_TWO
    // figure out how much to add to nx and ny to get to the next power of two
    int nx_test = num_x;
    int ny_test = num_y;
    int nx_round = 1;
    int ny_round = 1;
    while (nx_test > 1) {
       nx_test /= 2;
       nx_round *= 2;
    }
    if (nx_round <= num_x) {
       nx_round *= 2;
    }
    while (ny_test > 1) {
       ny_test /= 2;
       ny_round *= 2;
    }
    if (ny_round <= num_y) {
       ny_round *= 2;
    }
    d_borderXMin = (nx_round-num_x)/2;
    d_borderXMax = (nx_round - num_x - d_borderXMin);
    d_borderYMin = (ny_round-num_y)/2;
    d_borderYMax = (ny_round - num_y - d_borderYMin);
#else
    if (d_pixelType == NMB_UINT8) {
       d_borderXMin = 2;
       d_borderXMax = 2;
       d_borderYMin = 2;
       d_borderYMax = 2;
    }
#endif

    switch (d_pixelType) {
      case NMB_FLOAT32:
        array_size = arrayLength();
        data = new vrpn_float32[array_size];
        break;
      case NMB_UINT8:
        array_size = arrayLength();
        data = new vrpn_uint8[array_size];
        break;
      case NMB_UINT16:
        array_size = arrayLength();
        data = new vrpn_uint16[array_size];
        break;
      default:
        fprintf(stderr, "nmb_ImageArray::nmb_ImageArray:"
           " Error, unknown type\n");
        break;
    }
    if (!data) {
        fprintf(stderr, "nmb_ImageArray::nmb_ImageArray:"
                        " Error, out of memory\n");
        return;
    }

    fData = (vrpn_float32 *)data;
    ucData = (vrpn_uint8 *)data;
    usData = (vrpn_uint16 *)data;

    int j;
    switch (d_pixelType) {
      case NMB_FLOAT32:
        for (j = 0; j < array_size; j++) {
           fData[j] = 0;
        }
        break;
      case NMB_UINT8:
        for (j = 0; j < array_size; j++) {
           ucData[j] = 255;
        }
        break;
      case NMB_UINT16:
        for (j = 0; j < array_size; j++) {
           usData[j] = 0;
        }
        break;
      default:
        fprintf(stderr, "nmb_ImageArray::nmb_ImageArray:"
           " Error, unknown type\n");
        break;
    }

    for (int i = 0; i < numExportFormats(); i++){
        BCString name = exportFormatType(i);
        formatNames.addEntry(name);
    }
}

nmb_ImageArray::nmb_ImageArray(nmb_Image *im)
{
  nmb_ImageArray(im->name()->Characters(),
                  im->unitsValue()->Characters(),
                  im->width(), im->height(), im->pixelType());
  units_x = *(im->unitsX());
  units_y = *(im->unitsY());
  d_units_scale = im->valueScale();
  d_units_offset = im->valueOffset();
  int i,j;
  for (i = 0; i < width(); i++) {
    for (j = 0; j < height(); j++) {
      setValue(i,j, im->getValue(i,j));
    }
  }
  im->validDataRange(&max_y_set, &min_x_set, &min_y_set, &max_x_set);
}

nmb_ImageArray::~nmb_ImageArray() {
  if (data) {
    delete [] data;
    data = NULL;
    fData = NULL;
    ucData = NULL;
    usData = NULL;
  }
}

int nmb_ImageArray::width() const {return num_x;}

int nmb_ImageArray::height() const {return num_y;}

int nmb_ImageArray::borderXMin() {return d_borderXMin;}
int nmb_ImageArray::borderXMax() {return d_borderXMax;}
int nmb_ImageArray::borderYMin() {return d_borderYMin;}
int nmb_ImageArray::borderYMax() {return d_borderYMax;}

int nmb_ImageArray::arrayLength() 
{
   return ((d_borderXMin+num_x+d_borderXMax)*(d_borderYMin+num_y+d_borderYMax));
}

nmb_PixelType nmb_ImageArray::pixelType() {return d_pixelType;}

void *nmb_ImageArray::pixelData() {return data;}

float nmb_ImageArray::getValue(int i, int j) const
{
  int index = arrIndex(i,j);
  switch (d_pixelType) {
    case NMB_FLOAT32:
      return (float)fData[index];
    case NMB_UINT8:
      return (float)ucData[index];
    case NMB_UINT16:
      return (float)usData[index];
    default:
      fprintf(stderr, "nmb_ImageArray::getValue:"
           " Error, unknown type\n");
      break;
  }
  return 0.0;
}

void nmb_ImageArray::setValue(int i, int j, float val)
{
  float clampedValue;
  if (val > maxAttainableValue()) {
      clampedValue = maxAttainableValue(); 
  } else if (val < minAttainableValue()) {
      clampedValue = minAttainableValue();
  } else {
      clampedValue = val;
  }
  int index = arrIndex(i,j);
  switch (d_pixelType) {
    case NMB_FLOAT32:
      fData[index] = (float)clampedValue;
      break;
    case NMB_UINT8:
      ucData[index] = (vrpn_uint8)clampedValue;
      break;
    case NMB_UINT16:
      usData[index] = (vrpn_uint16)clampedValue;
      break;
    default:
      fprintf(stderr, "nmb_ImageArray::setValue:"
           " Error, unknown type\n");
      break;
  }

  d_minValueComputed = VRPN_FALSE;
  d_maxValueComputed = VRPN_FALSE;
  d_minNonZeroValueComputed = VRPN_FALSE;
  d_minValidValueComputed = VRPN_FALSE;
  d_maxValidValueComputed = VRPN_FALSE;
} 

void nmb_ImageArray::setLine(int line, void *line_data) {
  int index = (line+d_borderYMin)*
              (num_x+d_borderXMin+d_borderXMax)+d_borderXMin;
  switch(d_pixelType) {
    case NMB_FLOAT32:
      memcpy(&(fData[index]),
               line_data, num_x*sizeof(vrpn_float32));
      break;
    case NMB_UINT8:
      memcpy(&(ucData[index]),
               line_data, num_x*sizeof(vrpn_uint8));
      break;
    case NMB_UINT16:
      memcpy(&(usData[index]),
               line_data, num_x*sizeof(vrpn_uint16));
      break;
    default:
      fprintf(stderr, "nmb_ImageArray::getLine:"
           " Error, unknown type\n");
      break;
  }
}

void nmb_ImageArray::setImage(void *newdata) {
  int i;
  switch(d_pixelType) {
    case NMB_FLOAT32:
      for (i = 0; i < num_y; i++){
        setLine(i, &(((vrpn_float32*)newdata)[num_x*i]));
      }
      break;
    case NMB_UINT8:
      for (i = 0; i < num_y; i++){
        setLine(i, &(((vrpn_uint8*)newdata)[num_x*i]));
      }
      break;
    case NMB_UINT16:
      for (i = 0; i < num_y; i++){
        setLine(i, &(((vrpn_uint16*)newdata)[num_x*i]));
      }
      break;
    default:
      fprintf(stderr, "nmb_ImageArray::setImage:"
           " Error, unknown type\n");
      break;
  }
}

int nmb_ImageArray::validDataRange(short* o_top, short* o_left,
                                   short* o_bottom, short*o_right){
     // if no valid data:
     if (min_y_set > max_y_set || min_x_set > max_x_set)
        return -1;
     // otherwise at least one valid data point:
     *o_bottom = min_y_set; *o_top = max_y_set;
     *o_left = min_x_set; *o_right = max_x_set;
     return 0;
}

float nmb_ImageArray::maxAttainableValue() const 
{
  switch(d_pixelType) {
    case NMB_FLOAT32:
      return FLT_MAX;
    case NMB_UINT8:
      return 255.0f;
    case NMB_UINT16:
      return 65535.0f;
    default:
      fprintf(stderr, "nmb_ImageArray::maxAttainableValue:"
           " Error, unknown type\n");
      return 0.0;
  }
}

float nmb_ImageArray::minAttainableValue() const
{
  switch(d_pixelType) {
    case NMB_FLOAT32:
      return -FLT_MAX;
    case NMB_UINT8:
      return 0.0f;
    case NMB_UINT16:
      return 0.0f;
    default:
      fprintf(stderr, "nmb_ImageArray::minAttainableValue:"
           " Error, unknown type\n");
      return 0.0;
  }
}

float nmb_ImageArray::maxValue()
{
    if (!d_maxValueComputed) {
        d_maxValue = getValue(0,0);
        for (int i = 0; i < num_x; i++){
            for (int j = 0; j < num_y; j++){
                float val = getValue(i,j);
                if (val > d_maxValue) {
		    d_maxValue = val;
                }
            }
        }
        d_maxValueComputed = VRPN_TRUE;
    }
    return d_maxValue;
}

float nmb_ImageArray::minValue() 
{
    if (!d_minValueComputed) {
        d_minValue = getValue(0,0);
        for (int i = 0; i < num_x; i++){
            for (int j = 0; j < num_y; j++){
                float val = getValue(i,j);
                if (val < d_minValue) {
                    d_minValue = val;
                }
            }
        }
        d_minValueComputed = VRPN_TRUE;
    }
    return d_minValue;
}

int nmb_ImageArray::normalize() {
  float min = minValue();
  float range = (maxValue() - min);
  float inv_range;
  if (range == 0) {
     inv_range = 0;
  } else {
     inv_range = 1.0/range;
  }

  d_units_offset += min*d_units_scale;
  d_units_scale *= range;

  // special case for floats - normalize to the range 0..1
  if (d_pixelType == NMB_FLOAT32) {
    for (int i = 0; i < num_x; i++) {
      for (int j = 0; j < num_y; j++) {
        setValue(i,j, (inv_range*(getValue(i,j)-min)));
      }
    }
  } else {
    assert(minAttainableValue() == 0.0);
    for (int i = 0; i < num_x; i++) {
      for (int j = 0; j < num_y; j++) {
        setValue(i,j,
            (inv_range * (getValue(i,j) - min) * maxAttainableValue()));
      }
    }
  }

  return 0;
}

float nmb_ImageArray::maxValidValue() {
    short top, left, bottom, right;
    if (validDataRange(&top, &left, &bottom, &right)) {
        return 0;
    }
    if ((top == height()-1) && (bottom == 0) && 
        (right == width()-1) && (left == 0)) {
        return maxValue();
    } else if (!d_maxValidValueComputed) {
        int i,j;
        d_maxValidValue = getValue(left, bottom);
        float val;
        for (i = left; i <= right; i++) {
            for (j = bottom; j <= top; j++) {
                val = getValue(i,j);
                if (val > d_maxValidValue) d_maxValidValue = val;
            }
        }
        d_maxValidValueComputed = VRPN_TRUE;
    }
    return d_maxValidValue;
}

float nmb_ImageArray::minValidValue() {
    short top, left, bottom, right;
    if (validDataRange(&top, &left, &bottom, &right)) {
        return 0;
    }
    if ((top == height()-1) && (bottom == 0) &&
        (right == width()-1) && (left == 0)) {
        return minValue();
    } else if (!d_maxValidValueComputed) {
        int i,j;
        d_minValidValue = getValue(left, bottom);
        float val;
        for (i = left; i <= right; i++) {
            for (j = bottom; j <= top; j++) {
                val = getValue(i,j);
                if (val < d_minValidValue) d_minValidValue = val;
            }
        }
        d_minValidValueComputed = VRPN_TRUE;
    }
    return d_minValidValue;
}

float nmb_ImageArray::maxNonZeroValue() {
    return maxValidValue();
}

float nmb_ImageArray::minNonZeroValue() {
    if (!d_minNonZeroValueComputed) {
      int i,j;
      float val = 0, d_minNonZeroValue = 0;
      for (i = 0; i < width(); i++) {
        for (j = 0; j < height(); j++) {
          val = getValue(i,j);
          if (val != 0 && val < d_minNonZeroValue) {
            d_minNonZeroValue = val;
          }
        }
      }
      d_minNonZeroValueComputed = VRPN_TRUE;
    }
    return d_minNonZeroValue;
}

double nmb_ImageArray::boundX(
                    nmb_ImageBounds::ImageBoundPoint ibp) const
{
    return d_imagePosition.getX(ibp);
}

double nmb_ImageArray::boundY(
                   nmb_ImageBounds::ImageBoundPoint ibp) const
{
    return d_imagePosition.getY(ibp);
}

void nmb_ImageArray::setBoundX(
    nmb_ImageBounds::ImageBoundPoint ibp,
    double x)
{
    d_imagePosition.setX(ibp, x);
}

void nmb_ImageArray::setBoundY(
    nmb_ImageBounds::ImageBoundPoint ibp,
    double y)
{
    d_imagePosition.setY(ibp, y);
}

void nmb_ImageArray::setBounds(const nmb_ImageBounds & ib) 
{
    d_imagePosition = ib;
}

void nmb_ImageArray::getBounds(nmb_ImageBounds &ib)  const
{
    ib = d_imagePosition;
}

BCString *nmb_ImageArray::name() {return &my_name;}
BCString *nmb_ImageArray::unitsValue() {return &units;}
BCString *nmb_ImageArray::unitsX() {return &units_x;}
BCString *nmb_ImageArray::unitsY() {return &units_y;}

int nmb_ImageArray::numExportFormats() 
{
  return num_export_formats;
}

nmb_ListOfStrings *nmb_ImageArray::exportFormatNames() 
{
  return &formatNames;
}

const char *nmb_ImageArray::exportFormatType(int type) 
{
  return (const char *)(export_formats_list[type]);
}

int nmb_ImageArray::exportToFile(FILE *f, const char *export_type, 
                                const char * filename){

    int my_export_type;
    for (my_export_type = 0; my_export_type < numExportFormats();
         my_export_type++){
        if (strcmp(export_type, exportFormatType(my_export_type)) == 0)
            break;
    }
    // if didn't find a match to export_type
    if (my_export_type == numExportFormats()) {
        fprintf(stderr, "nmb_ImageArray::Error, unknown file type: %s\n",
            export_type);
        return -1;
    }
    else {  // we have a function for exporting this type
        if (file_exporting_function[my_export_type](f, this, filename)) {
            fprintf(stderr, "nmb_ImageArray::Error writing file of type %s\n",
                export_type);
            return -1;
        }
        return 0;
    }
}

// static
int nmb_ImageArray::exportToTIFF(FILE *file, nmb_ImageArray *im, const char * filename)
{
  if (im->pixelType() != NMB_UINT8) {
    printf("error, can't write images that aren't 8 bits per pixel\n");
    return 0;
  }
  int w = im->width();
  int h = im->height();
  // use color image because greyscale export is broken
  unsigned char *pixels = new unsigned char [w*h*3];

  int i, j;
  for (i = 0; i < w; i++){
    for (j = 0; j < h; j++) {
       float val = im->getValue(i,j);
       unsigned char byteval = (unsigned char)val;
       pixels[(i+j*w)*3] = byteval;
       pixels[(i+j*w)*3+1] = byteval;
       pixels[(i+j*w)*3+2] = byteval;
    }
  }

  if(nmb_ImgMagick::writeFileMagick(filename, NULL, w, h, 3, pixels)) {
      fprintf(stderr, "Failed to write image to file\n");
  }
  delete [] pixels;
  return 0;
}

nmb_ImageList::nmb_ImageList(nmb_ListOfStrings *namelist) :
       num_images(0),
       imageNames(namelist)
{

}

nmb_ImageList::nmb_ImageList(nmb_ListOfStrings *namelist,
                             const char **file_names, int num_files,
                             TopoFile &topoFile) :
       num_images(0),
       imageNames(namelist)
{
    //printf("nmb_ImageList::nmb_ImageList building list\n");

    for (int i = 0; i < num_files; i++) {
//          printf("nmb_ImageList::nmb_ImageList - creating grid for file %s\n",
//  		file_names[i]);
	BCGrid *g = new BCGrid(0, 0, 0, 1, 0, 1,
                               READ_FILE);
        g->loadFile( file_names[i], topoFile);
	nmb_Image *im;
	BCPlane *p;
//          printf("file contained: \n");
	for (p = g->head(); p != NULL; p = p->next()){
	    im = new nmb_ImageGrid(p);
            im->setTopoFileInfo(topoFile);
	    addImage(im);
//              printf("  %s\n", (const char *)(*(im->name())));
//  	    printf("nmb_Image min,max=%f,%f\n", im->minValue(),im->maxValue());
	}
    }
}

nmb_ImageList::~nmb_ImageList()
{
    for (int i = 0; i < num_images; i++) {
        images[i]->num_referencing_lists--;
        if (images[i]->num_referencing_lists == 0) {
	    delete images[i];
        }
    }
}

int nmb_ImageList::addImage(nmb_Image *im)
{
    // don't add this if name is not unique
    if (getImageByName(*(im->name())))
	return -2;
    // don't add if list is full
    BCString name = (*(im->name()));
    if (num_images == NMB_MAX_IMAGELIST_LENGTH){
      return -1;
    }

    images[num_images] = im;
    num_images++;
    im->num_referencing_lists++;

    // it is important to do this last because this can have side effects
    // in tcl that trigger C-callbacks to get called that expect the image
    // to actually be in the image list when its name is in the corresponding
    // string list
    if (imageNames->addEntry(name)) {
      return -1;
    }

    return 0;
}

nmb_Image *nmb_ImageList::removeImageByName(BCString name) {
    int i;
    nmb_Image *im = getImageByName(name, i);
    if (im == NULL) return NULL;
    // getImageByName() succeeds ==> num_images >= 1
    images[i] = images[num_images-1];
    imageNames->deleteEntry((const char *)(*(im->name())));
    num_images--;
    im->num_referencing_lists--;
    return im;
}

nmb_Image *nmb_ImageList::getImageByName(BCString name, int &index) {
    for (int i = 0; i < num_images; i++) {
        if (*(images[i]->name()) == name){
             index = i;
             return images[i];
        }
    }
    return NULL;
}

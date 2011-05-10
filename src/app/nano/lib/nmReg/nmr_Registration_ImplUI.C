/*===3rdtech===
  Copyright (c) 2001 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#include "nmr_Registration_ImplUI.h"
#include <nmb_ColorMap.h>

int nmr_Registration_ImplUI::s_numImages = 2;
char *nmr_Registration_ImplUI::s_imageWinNames[] = {
	"Registration: Topography Image",
	"Registration: Projection Image"
};
int nmr_Registration_ImplUI::s_sourceImageIndex = 0;
int nmr_Registration_ImplUI::s_targetImageIndex = 1;

nmr_Registration_ImplUI::nmr_Registration_ImplUI():
         d_ce(s_numImages, s_imageWinNames)
{
}

void nmr_Registration_ImplUI::registerCorrespondenceHandler(
                     CorrespondenceCallback handler, void *ud)
{
  d_ce.registerCallback(handler, ud);
}

nmr_Registration_ImplUI::~nmr_Registration_ImplUI()
{
}

void nmr_Registration_ImplUI::enable(vrpn_bool enable, vrpn_int32 window)
{
	if (window == NMR_ALLWINDOWS) {
		if (enable) {
			d_ce.showAll();
		} else {
			d_ce.hideAll();
		}
	} else {
		int spaceIndex = 0;
		if (window == NMR_SOURCEWINDOW) {
			spaceIndex = s_sourceImageIndex;
		} else if (window == NMR_TARGETWINDOW) {
			spaceIndex = s_targetImageIndex;
		} else {
			fprintf(stderr, "enableUI: Error, unknown window: %d\n", window);
			return;
		}
		if (enable) {
			d_ce.show(spaceIndex);
		} else {
			d_ce.hide(spaceIndex);
		}
	}
}

void nmr_Registration_ImplUI::enableEdit(vrpn_bool enableAddAndDelete, 
										 vrpn_bool enableMove)
{
	d_ce.enableEdit(enableAddAndDelete, enableMove);
}

void nmr_Registration_ImplUI::mainloop()
{
  d_ce.mainloop();
}

void nmr_Registration_ImplUI::newScanline(nmr_ImageType whichImage,
                                vrpn_int32 row, nmb_Image *im)
{
   if (im->height() == row+1) {
       nmb_ImageGrid *adjustedIm = new nmb_ImageGrid(im);
       // we may want to do some histogram eq. here
       if (whichImage == NMR_SOURCE) {
           d_ce.setImage(s_sourceImageIndex, (nmb_Image *)adjustedIm);
       } else if (whichImage == NMR_TARGET) {
           d_ce.setImage(s_targetImageIndex, (nmb_Image *)adjustedIm);
       }
       nmb_Image::deleteImage(adjustedIm);
   }
}

void nmr_Registration_ImplUI::clearImage(nmr_ImageType whichImage)
{
   if (whichImage == NMR_SOURCE) {
       d_ce.setImage(s_sourceImageIndex, NULL);
   } else if (whichImage == NMR_TARGET) {
       d_ce.setImage(s_targetImageIndex, NULL);
   }
}

// x and y are in range 0..1, z is in nm
void nmr_Registration_ImplUI::setFiducial(vrpn_int32 replace, vrpn_int32 num,
          vrpn_float32 *x_src, vrpn_float32 *y_src, vrpn_float32 *z_src,
          vrpn_float32 *x_tgt, vrpn_float32 *y_tgt, vrpn_float32 *z_tgt)
{
  float x[2], y[2], z[2];
  if (replace) {
    d_ce.clearFiducials();
  }
  int i;
  for (i = 0; i < num; i++) {
    x[s_sourceImageIndex] = x_src[i];
    y[s_sourceImageIndex] = y_src[i];
    z[s_sourceImageIndex] = z_src[i];
    x[s_targetImageIndex] = x_tgt[i];
    y[s_targetImageIndex] = y_tgt[i];
    z[s_targetImageIndex] = z_tgt[i];

	if(i == 0)
	{
		printf ("xsource: %f ysource: %f xtarget: %f ytarget: %f\n", x[s_sourceImageIndex], y[s_sourceImageIndex], x[s_targetImageIndex], y[s_targetImageIndex]);
	}

    d_ce.addFiducial(x, y, z);
  }
}

void nmr_Registration_ImplUI::getFiducial(vrpn_int32 &num,
          vrpn_float32 *x_src, vrpn_float32 *y_src, vrpn_float32 *z_src,
          vrpn_float32 *x_tgt, vrpn_float32 *y_tgt, vrpn_float32 *z_tgt)
{
  Correspondence c;
  int srcIndex, tgtIndex;
  getCorrespondence(c, srcIndex, tgtIndex);
  num = c.numPoints();
  if (num > NMR_MAX_FIDUCIAL) {
	num = NMR_MAX_FIDUCIAL;
  }
  corr_point_t pnt;
  int i;
  for (i = 0; i < num; i++) {
	  c.getPoint(srcIndex, i, &pnt);
	  x_src[i] = pnt.x;
	  y_src[i] = pnt.y;
	  z_src[i] = pnt.z;
	  c.getPoint(tgtIndex, i, &pnt);
	  x_tgt[i] = pnt.x;
	  y_tgt[i] = pnt.y;
	  z_tgt[i] = pnt.z;
  }
}

void nmr_Registration_ImplUI::getCorrespondence(Correspondence &c, 
                              int &srcIndex, int &tgtIndex)
{
   d_ce.getCorrespondence(c);
   srcIndex = s_sourceImageIndex;
   tgtIndex = s_targetImageIndex;
   return;
}

void nmr_Registration_ImplUI::setColorMap(nmr_ImageType whichImage,
                                          nmb_ColorMap * cmap)
{
  if (whichImage == NMR_SOURCE) {
      d_ce.setColorMap(s_sourceImageIndex, cmap);
  } else if (whichImage == NMR_TARGET) {
      d_ce.setColorMap(s_targetImageIndex, cmap);
  }
}

void nmr_Registration_ImplUI::setColorMinMax(nmr_ImageType whichImage, 
                              vrpn_float64 dmin, vrpn_float64 dmax,
                              vrpn_float64 cmin, vrpn_float64 cmax)
{
  if (whichImage == NMR_SOURCE) {
    d_ce.setColorMinMax(s_sourceImageIndex, dmin, dmax, cmin, cmax);
  } else if (whichImage == NMR_TARGET) {
    d_ce.setColorMinMax(s_targetImageIndex, dmin, dmax, cmin, cmax);
  }
}

void  nmr_Registration_ImplUI::setFiducialSpotTracker(nmr_ImageType whichImage, vrpn_int32 tracker)
{
	if (whichImage == NMR_SOURCE) {
		d_ce.setFiducialSpotTracker(s_sourceImageIndex, tracker);
	} else if (whichImage == NMR_TARGET) {
		d_ce.setFiducialSpotTracker(s_targetImageIndex, tracker);
	}
}

void nmr_Registration_ImplUI::setOptimizeSpotTrackerRadius(nmr_ImageType whichImage, vrpn_bool enable)
{
	if (whichImage == NMR_SOURCE) {
		d_ce.setOptimizeSpotTrackerRadius(s_sourceImageIndex, enable);
	} else if (whichImage == NMR_TARGET) {
		d_ce.setOptimizeSpotTrackerRadius(s_targetImageIndex, enable);
	}
}

void nmr_Registration_ImplUI::setSpotTrackerRadius(nmr_ImageType whichImage, vrpn_float64 radius)
{
	if (whichImage == NMR_SOURCE) {
		d_ce.setSpotTrackerRadius(s_sourceImageIndex, radius);
	} else if (whichImage == NMR_TARGET) {
		d_ce.setSpotTrackerRadius(s_targetImageIndex, radius);
	}
}

void nmr_Registration_ImplUI::setSpotTrackerPixelAccuracy(nmr_ImageType whichImage, vrpn_float64 accuracy)
{
	if (whichImage == NMR_SOURCE) {
		d_ce.setSpotTrackerPixelAccuracy(s_sourceImageIndex, accuracy);
	} else if (whichImage == NMR_TARGET) {
		d_ce.setSpotTrackerPixelAccuracy(s_targetImageIndex, accuracy);
	}
}

void nmr_Registration_ImplUI::setSpotTrackerRadiusAccuracy(nmr_ImageType whichImage, vrpn_float64 accuracy)
{
	if (whichImage == NMR_SOURCE) {
		d_ce.setSpotTrackerRadiusAccuracy(s_sourceImageIndex, accuracy);
	} else if (whichImage == NMR_TARGET) {
		d_ce.setSpotTrackerRadiusAccuracy(s_targetImageIndex, accuracy);
	}
}

void nmr_Registration_ImplUI::setImageOrientation(nmr_ImageType whichImage,
                              vrpn_bool flipX, vrpn_bool flipY)
{
  if (whichImage == NMR_SOURCE) {
    d_ce.setImageOrientation(s_sourceImageIndex, flipX, flipY);
  } else if (whichImage == NMR_TARGET) {
    d_ce.setImageOrientation(s_targetImageIndex, flipX, flipY);
  }
}
/*
CorrespondenceEditor nmr_Registration_ImplUI::get_d_ce()
{
	return d_ce; 
}//new
*/

void nmr_Registration_ImplUI::setTopoIntensityThreshold(float intensity)
{
	d_ce.setIntensityThreshold(0,intensity);
}

void nmr_Registration_ImplUI::setProjIntensityThreshold(float intensity)
{
	d_ce.setIntensityThreshold(1,intensity);
}

float nmr_Registration_ImplUI::getIntensityValue(int x, int y)
{
	return d_ce.getIntensity(x,y);
}

vector< vector< vector <float> > > nmr_Registration_ImplUI::readPixels()
{
//	const char * image_topography = "output/pixelvalues_topography.txt";
//	d_ce.readAllTest(0,image_topography);

//	const char * image_projection = "output/pixelvalues_projection.txt";
//	d_ce.readAllTest(1,image_projection);

	vector< vector< vector <float> > > initialRansac;

	const char * ransac_points_topography = "ransac_points_topography.txt";
//	const char * ransac_points_topography = "output/ransac_points_topography.txt";
	vector< vector <float> > initialRansacTopography;
	initialRansacTopography = d_ce.comparePixelsWithNeighbors(0,ransac_points_topography);

	const char * ransac_points_projection = "ransac_points_projection.txt";
//	const char * ransac_points_projection = "output/ransac_points_projection.txt";
	vector< vector <float> > initialRansacProjection;
	initialRansacProjection = d_ce.comparePixelsWithNeighbors(1,ransac_points_projection);

	initialRansac.push_back(initialRansacTopography);
	initialRansac.push_back(initialRansacProjection);

	return initialRansac;
}

vector < vector <float> > nmr_Registration_ImplUI::getWidthHeight()
{
	vector<float> wh0(2,0);
	wh0[0] = d_ce.getWidth(0);
	wh0[1] = d_ce.getHeight(0);

	vector<float> wh1(2,0);
	wh1[0] = d_ce.getWidth(1);
	wh1[1] = d_ce.getHeight(1);

	vector < vector <float> > wh;
	wh.push_back(wh0);
	wh.push_back(wh1);

	return wh;
}
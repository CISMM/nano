/*===3rdtech===
  Copyright (c) 2001 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#include "nmr_Registration_ImplUI.h"
#include <nmb_ColorMap.h>

int nmr_Registration_ImplUI::s_numImages = 2;
char *nmr_Registration_ImplUI::s_imageWinNames[] = 
                  {"registration:reference",
                   "registration:adjustable"};
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

void nmr_Registration_ImplUI::enable(vrpn_bool enable)
{
  if (enable){
     d_ce.show();
  } else {
     d_ce.hide();
  }
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

// x and y are in range 0..1, z is in nm
void nmr_Registration_ImplUI::setFiducial(
          vrpn_float32 x_src, vrpn_float32 y_src, vrpn_float32 z_src,
          vrpn_float32 x_tgt, vrpn_float32 y_tgt, vrpn_float32 z_tgt)
{
  float x[2], y[2], z[2];
  x[s_sourceImageIndex] = x_src;
  y[s_sourceImageIndex] = y_src;
  z[s_sourceImageIndex] = z_src;
  x[s_targetImageIndex] = x_tgt;
  y[s_targetImageIndex] = y_tgt;
  z[s_targetImageIndex] = z_tgt;

  d_ce.addFiducial(x, y, z);
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

void nmr_Registration_ImplUI::setImageOrientation(nmr_ImageType whichImage,
                              vrpn_bool flipX, vrpn_bool flipY)
{
  if (whichImage == NMR_SOURCE) {
    d_ce.setImageOrientation(s_sourceImageIndex, flipX, flipY);
  } else if (whichImage == NMR_TARGET) {
    d_ce.setImageOrientation(s_targetImageIndex, flipX, flipY);
  }
}

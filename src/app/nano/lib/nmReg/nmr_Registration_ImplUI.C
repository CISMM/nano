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

nmr_Registration_ImplUI::nmr_Registration_ImplUI(
         nmr_Registration_Impl *impl):
         d_impl(impl)
{

  d_ce = new CorrespondenceEditor(s_numImages, s_imageWinNames);
  d_ce->registerCallback(handle_CorrespondenceChange, (void *)this);

  return;
}

nmr_Registration_ImplUI::~nmr_Registration_ImplUI()
{
  delete d_ce;
}

void nmr_Registration_ImplUI::enable(vrpn_bool enable)
{
  if (enable){
     d_ce->show();
  } else {
     d_ce->hide();
  }
}

void nmr_Registration_ImplUI::mainloop()
{
  d_ce->mainloop();
}

//static
void nmr_Registration_ImplUI::handle_CorrespondenceChange(Correspondence &c,
                                                          void *ud)
{
  double xform_matrix[16];
  nmr_Registration_ImplUI *me = (nmr_Registration_ImplUI *)ud;
  me->d_impl->registerImagesFromPointCorrespondence(xform_matrix);
  me->d_impl->sendResult(xform_matrix);
}

void nmr_Registration_ImplUI::registerImages()
{
  double xform_matrix[16];
  d_impl->registerImagesFromPointCorrespondence(xform_matrix);
  d_impl->sendResult(xform_matrix);
}

void nmr_Registration_ImplUI::newScanline(nmr_ImageType whichImage,
                                vrpn_int32 row, nmb_Image *im)
{
   if (im->height() == row+1) {
       nmb_ImageGrid *adjustedIm = new nmb_ImageGrid(im);
       // we may want to do some histogram eq. here
       if (whichImage == NMR_SOURCE) {
           d_ce->setImage(s_sourceImageIndex, (nmb_Image *)adjustedIm);
       } else if (whichImage == NMR_TARGET) {
           d_ce->setImage(s_targetImageIndex, (nmb_Image *)adjustedIm);
       }
       nmb_Image::deleteImage(adjustedIm);
   }
}

// x and y are in range 0..1, z is in nm
void nmr_Registration_ImplUI::setFiducial(nmr_ImageType whichImage,
          vrpn_float32 x_n, vrpn_float32 y_n, vrpn_float32 z)
{
  if (whichImage == NMR_SOURCE) {
     d_ce->addFiducial(s_sourceImageIndex, x_n, y_n, z);
  } else if (whichImage == NMR_TARGET) {
     d_ce->addFiducial(s_targetImageIndex, x_n, y_n, z);
  }
}

void nmr_Registration_ImplUI::getCorrespondence(Correspondence &c, 
                              int &srcIndex, int &tgtIndex)
{
   d_ce->getCorrespondence(c);
   srcIndex = s_sourceImageIndex;
   tgtIndex = s_targetImageIndex;
   return;
}

void nmr_Registration_ImplUI::setColorMap(nmr_ImageType whichImage,
                                          nmb_ColorMap * cmap)
{
  if (whichImage == NMR_SOURCE) {
      d_ce->setColorMap(s_sourceImageIndex, cmap);
  } else if (whichImage == NMR_TARGET) {
      d_ce->setColorMap(s_targetImageIndex, cmap);
  }
}

void nmr_Registration_ImplUI::setColorMinMax(nmr_ImageType whichImage, 
                              vrpn_float64 dmin, vrpn_float64 dmax,
                              vrpn_float64 cmin, vrpn_float64 cmax)
{
  if (whichImage == NMR_SOURCE) {
    d_ce->setColorMinMax(s_sourceImageIndex, dmin, dmax, cmin, cmax);
  } else if (whichImage == NMR_TARGET) {
    d_ce->setColorMinMax(s_targetImageIndex, dmin, dmax, cmin, cmax);
  }
}

#include "nmr_Registration_ImplUI.h"

int nmr_Registration_ImplUI::s_numImages = 2;
char *nmr_Registration_ImplUI::s_imageWinNames[] = 
                  {"registration:topography",
                   "registration:texture"};
int nmr_Registration_ImplUI::s_sourceImageIndex = 0;
int nmr_Registration_ImplUI::s_targetImageIndex = 1;

nmr_Registration_ImplUI::nmr_Registration_ImplUI(
         nmr_Registration_Impl *impl):
         d_impl(impl)
{

  d_ce = new CorrespondenceEditor(s_numImages, s_imageWinNames);

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

void nmr_Registration_ImplUI::registerImages()
{
  Correspondence c;
  d_ce->getCorrespondence(c);
  d_impl->registerImages(c, s_sourceImageIndex, s_targetImageIndex);
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
       delete adjustedIm;
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

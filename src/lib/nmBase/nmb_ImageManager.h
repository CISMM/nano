#ifndef NMB_IMAGEMANAGER_H
#define NMB_IMAGEMANAGER_H

/* this class manages a list of images
   in which some of the images belong to a special set corresponding to what
   has been called the input grid

   it distills out that part of nmb_Dataset which is actually used by 
   nmr_RegistrationUI so that the registration code can be used in other 
   applications without pulling in a whole slew of unnecessary code and
   unused variables
*/

#include "nmb_Image.h"
#include "BCGrid.h"
#include "nmb_String.h"

class nmb_ImageManager {
  friend class nmb_Dataset;

  public:
    nmb_ImageManager(nmb_ImageList *images, BCGrid *inputGrid = NULL,
                     nmb_ListOfStrings *inputPlaneNames = NULL);
    ~nmb_ImageManager() {};
    int addImageToGrid(nmb_ImageGrid * new_image);
    nmb_ImageList *dataImages() {return d_images;}

  protected:
    nmb_ImageList * d_images; // flat list of all images
    BCGrid * d_inputGrid; // references to a subset of d_images
    // list of names for images in inputGrid
    nmb_ListOfStrings *d_inputPlaneNames; 
};

#endif


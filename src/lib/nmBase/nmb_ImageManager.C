#include "nmb_ImageManager.h"

nmb_ImageManager::nmb_ImageManager(nmb_ImageList *images, BCGrid *inputGrid,
                 nmb_ListOfStrings *inputPlaneNames):
  d_images(images), d_inputGrid(inputGrid), d_inputPlaneNames(inputPlaneNames)
{

}

int nmb_ImageManager::addImageToGrid(nmb_ImageGrid * new_image)
{
  if (d_inputGrid) {
    BCPlane *newplane;
    BCString name;
    d_inputGrid->findUniquePlaneName(*(new_image->name()),&name);
    newplane = d_inputGrid->addPlaneCopy(new_image->getPlane());
    newplane->rename(name);

    if (d_inputPlaneNames) {
      d_inputPlaneNames->addEntry(name);
    }
    d_images->addImage(new nmb_ImageGrid(newplane));

  } else {
    d_images->addImage(new nmb_ImageGrid(new_image));
  }
  return 0;
}

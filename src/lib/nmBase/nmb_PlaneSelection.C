#include "nmb_PlaneSelection.h"

#include <stdlib.h>  // for NULL

#include "BCPlane.h"
#include "BCGrid.h"
#include "nmb_Dataset.h"
#include "nmb_String.h"


nmb_PlaneSelection::nmb_PlaneSelection (void) :
  height (NULL),
  color (NULL),
  contour (NULL),
  alpha (NULL),
  red (NULL),
  green (NULL),
  blue (NULL) {

}

nmb_PlaneSelection::~nmb_PlaneSelection (void) {

}

void nmb_PlaneSelection::lookup (nmb_Dataset * data) {
  lookup(data->inputGrid,
         data->heightPlaneName->string(),
         data->colorPlaneName->string(),
         data->contourPlaneName->string(),
         data->alphaPlaneName->string());
}

void nmb_PlaneSelection::lookup (BCGrid * inputGrid,
                                 const char * heightName,
                                 const char * colorName,
                                 const char * contourName,
                                 const char * alphaName) {
  height = inputGrid->getPlaneByName (heightName);
  color = inputGrid->getPlaneByName (colorName);
  contour = inputGrid->getPlaneByName (contourName);
  alpha = inputGrid->getPlaneByName (alphaName);
}

void nmb_PlaneSelection::lookupPrerenderedColors (BCGrid * grid) {
  red = grid->getPlaneByName("prerendered red");
  green = grid->getPlaneByName("prerendered green");
  blue = grid->getPlaneByName("prerendered blue");
}

void nmb_PlaneSelection::lookupPrerenderedDepth (BCGrid * grid) {
  height = grid->getPlaneByName("captured depth");
}

#include "nmb_PlaneSelection.h"

#include <stdlib.h>  // for NULL

#include "BCPlane.h"
#include "BCGrid.h"
#include "nmb_Dataset.h"
#include "nmb_Selector.h"


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
  height = data->inputGrid->getPlaneByName
                    (data->heightPlaneName->string());
  color = data->inputGrid->getPlaneByName
                    (data->colorPlaneName->string());
  contour = data->inputGrid->getPlaneByName
                    (data->contourPlaneName->string());
  alpha = data->inputGrid->getPlaneByName
                    (data->alphaPlaneName->string());
}

void nmb_PlaneSelection::lookupPrerendered (BCGrid * grid) {
  height = grid->getPlaneByName("captured depth");
  red = grid->getPlaneByName("prerendered red");
  green = grid->getPlaneByName("prerendered green");
  blue = grid->getPlaneByName("prerendered blue");
}

#include "nmui_Haptics.h"

#include <stdlib.h>  // for NULL!

nmui_HapticsManager::nmui_HapticsManager (void) :
    d_canned  (NULL),
    d_measurePlane  (NULL),
    d_livePlane  (NULL),
    d_feelAhead  (NULL),
    d_pseudoFA  (NULL),
    d_directZ  (NULL),
    d_warpedPlane (NULL),

    d_gridFeatures  (NULL),
    d_pointFeatures  (NULL),

    d_surfaceFeatures (),
    d_hapticSurface (NULL) {

}

nmui_HapticsManager::~nmui_HapticsManager (void) {

}

nmui_SurfaceFeatures & nmui_HapticsManager::surfaceFeatures (void) {
  return d_surfaceFeatures;
}

nmui_HapticSurface * nmui_HapticsManager::surface (void) {
  return d_hapticSurface;
}

void nmui_HapticsManager::setSurface (nmui_HapticSurface * s) {
  d_hapticSurface = s;
}


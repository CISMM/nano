#include "nmui_Haptics.h"

#include <stdlib.h>  // for NULL!

nmui_HapticsManager::nmui_HapticsManager (void) :
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


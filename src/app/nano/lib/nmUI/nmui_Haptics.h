#ifndef NMUI_HAPTICS_H
#define NMUI_HAPTICS_H

#include <vrpn_Types.h>  // for vrpn_bool

#include "nmui_SurfaceFeatures.h"  // for nmui_SurfaceFeatures

class nmb_Dataset;  // from <nmb_Dataset.h>
class nmm_Microscope_Remote;  // from <nmm_MicroscopeRemote.h>

class nmui_HapticSurface;  // from "nmui_HapticSurface.h"

class nmui_HapticsManager {

  public:

    nmui_HapticsManager (void);
    virtual ~nmui_HapticsManager (void);

    // ACCESSORS




    // MANIPULATORS

    nmui_SurfaceFeatures & surfaceFeatures (void);
    nmui_HapticSurface * surface (void);

    void setSurface (nmui_HapticSurface *);

  protected:

    nmui_SurfaceFeatures d_surfaceFeatures;
    nmui_HapticSurface * d_hapticSurface;


};



#endif  // NMUI_HAPTICS_H


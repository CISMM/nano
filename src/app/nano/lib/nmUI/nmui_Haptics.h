#ifndef NMUI_HAPTICS_H
#define NMUI_HAPTICS_H

#include <vrpn_Types.h>  // for vrpn_bool

#include "nmui_SurfaceFeatures.h"  // for nmui_SurfaceFeatures

class nmb_Dataset;  // from <nmb_Dataset.h>
class nmm_Microscope_Remote;  // from <nmm_MicroscopeRemote.h>

class nmui_HapticSurface;  // from "nmui_HapticSurface.h"
class nmui_HSCanned;
class nmui_HSMeasurePlane;
class nmui_HSLivePlane;
class nmui_HSFeelAhead;
class nmui_HSDirectZ;
class nmui_HSDirectZPlane;

class nmui_GridFeatures;
class nmui_PointFeatures;

class nmui_HapticsManager {

  public:

    nmui_HapticsManager (void);
    virtual ~nmui_HapticsManager (void);

    // ACCESSORS




    // MANIPULATORS

    nmui_SurfaceFeatures & surfaceFeatures (void);
    nmui_HapticSurface * surface (void);

    void setSurface (nmui_HapticSurface *);

    // Some of these need to be accessible, so for the moment we make
    // them ALL accessible.

    nmui_HSCanned * d_canned;
    nmui_HSMeasurePlane * d_measurePlane;
    nmui_HSLivePlane * d_livePlane;
    nmui_HSFeelAhead * d_feelAhead;
    nmui_HSDirectZ * d_directZ;
	nmui_HSDirectZPlane * d_directZPlane;

    nmui_GridFeatures * d_gridFeatures;
    nmui_PointFeatures * d_pointFeatures;

  protected:

    nmui_SurfaceFeatures d_surfaceFeatures;
    nmui_HapticSurface * d_hapticSurface;


};



#endif  // NMUI_HAPTICS_H


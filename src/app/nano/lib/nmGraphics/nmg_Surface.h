#ifndef NMG_SURFACE_H
#define NMG_SURFACE_H

#include <vrpn_Types.h>

class nmg_SurfaceRegion;
class nmg_SurfaceMask;
class nmb_Dataset;
class BCPlane;

class nmg_Surface
{
public:
    nmg_Surface();
    ~nmg_Surface();

    int init(unsigned int width, unsigned int height);
    void changeDataset(nmb_Dataset *dataset);

    void renderSurface();
    int rebuildRegion(int region);
    int rebuildSurface(vrpn_bool force = VRPN_FALSE);
    int rebuildInterval(int low_row, int high_row, int strips_in_x);

    //Region managing functions
    int createNewRegion();
    void destroyRegion(int region);
    nmg_SurfaceRegion* getRegion(int region);

    void setRegionControl(BCPlane *control, int region);
    void setMaskPlane(nmg_SurfaceMask* mask, int region);
    void deriveMaskPlane(float min_height, float max_height, int region);
    void rederive(int region);

    //Appearance mutator functions
    void setAlpha(float alpha, int region);
    void enableFilledPolygons(int enable, int region);
    void setTextureDisplayed(int display, int region);
    void setTextureMode(int mode, int region);
    void setTextureTransformMode(int mode, int region);
    void setStride(unsigned int stride, int region);

    //Behavior locks
    void lockAlpha(vrpn_bool lock, int region);
    void lockFilledPolygons(vrpn_bool lock, int region);
    void lockTextureDisplayed(vrpn_bool lock, int region);
    void lockTextureMode(vrpn_bool lock, int region);
    void lockTextureTransformMode(vrpn_bool lock, int region);
    void lockStride(vrpn_bool lock, int region);

private:
    nmb_Dataset *d_dataset;
    int d_numSubRegions;
    int d_maxNumRegions;
    nmg_SurfaceRegion **d_subRegions;
    nmg_SurfaceRegion *d_defaultRegion;
    unsigned int d_initHeight, d_initWidth;
};

#endif

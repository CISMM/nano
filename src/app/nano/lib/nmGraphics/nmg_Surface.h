#ifndef NMG_SURFACE_H
#define NMG_SURFACE_H

#include <vrpn_Types.h>

class nmg_SurfaceRegion;
class nmg_SurfaceMask;
class nmb_Dataset;
class BCPlane;
class nmg_State;

class nmg_Surface
{
public:
    nmg_Surface();
    ~nmg_Surface();

    /// Change width and height of surface, reallocate vertex arrays
    int init(unsigned int width, unsigned int height);
    void changeDataset(nmb_Dataset *dataset);
    nmb_Dataset *getDataset();

    /// Draw the surface, called once per frame. 
    void renderSurface(nmg_State * state);

    ///rebuild display lists for a single region
    //int rebuildRegion(nmg_State * state, int region);

    /// Redraw surface next time rebuildInterval is called. 
    int redrawSurface(nmg_State * state);

    ///rebuild display lists for whole surface, all subregions 
    /// (calls rebuildInterval)
    int rebuildSurface(nmg_State * state);
    ///rebuild a few strips of the surface, based on dataset->range_of_change,
    ///rebuilds whole surface if graphics->causeGridRedraw was called. 
    int rebuildInterval(nmg_State * state);

    ///only recolor the surface, don't re-calc vertices or normals
    int recolorSurface();

    //Region managing functions
    /// Return index for new region used as parameter for other 
    /// region managing functions. 
    int createNewRegion();
    void destroyRegion(int region);
    nmg_SurfaceRegion* getRegion(int region);

    void setRegionControl(BCPlane *control, int region);
    void setMaskPlane(nmg_SurfaceMask* mask, int region);
    void deriveMaskPlane(float min_height, float max_height, int region);
    void deriveMaskPlane(float center_x, float center_y, float width,float height, 
                         float angle, int region);
    void deriveMaskPlane(int region);

    //Appearance mutator functions
    void setAlpha(float alpha, int region);
    void enableFilledPolygons(int enable, int region);
    void setTextureDisplayed(int display, int region);
    void setTextureMode(int mode, int region);
    void setTextureTransformMode(int mode, int region);
    void setStride(unsigned int stride, int region);

    //Behavior associates
    void associateAlpha(vrpn_bool associate, int region);
    void associateFilledPolygons(vrpn_bool associate, int region);
    void associateTextureDisplayed(vrpn_bool associate, int region);
    void associateTextureMode(vrpn_bool associate, int region);
    void associateTextureTransformMode(vrpn_bool associate, int region);
    void associateStride(vrpn_bool associate, int region);

private:
    nmb_Dataset *d_dataset;
    int d_numSubRegions; ///< number of valid regions in d_subRegions
    int d_maxNumRegions; ///< currently allocated length of d_subRegions
    nmg_SurfaceRegion **d_subRegions;
    nmg_SurfaceRegion *d_defaultRegion;
    unsigned int d_initHeight, d_initWidth;
    int d_display_lists_in_x;

    ///Helper function
    int updateDefaultMask();

    /// Called whenever data might have changed, once each gfx loop.
    //int updateMask(int low_row, int high_row, int region);

};

#endif


#ifndef NMG_SURFACE_REGION_H
#define NMG_SURFACE_REGION_H

#include <vrpn_Types.h>
#include <nmb_Interval.h>

class BCPlane;
class nmb_Dataset;
class nmb_PlaneSelection;
class nmg_SurfaceMask;
class nmg_Surface;
struct Vertex_Struct;

typedef struct _GraphicsState
{
    int stride;
    vrpn_bool justColor;
    int filledPolygonsEnabled;
    float alpha;
    //These should really be Enums that are 
    //defined in nmg_Graphics.h, but want 
    //to avoid a circular include.
    int textureDisplayed;
    int textureMode;
    int textureTransformMode;

} GraphicsState;

typedef struct _BehaviorLocks
{
    vrpn_bool stride;
    vrpn_bool justColor;
    vrpn_bool filledPolygonsEnabled;
    vrpn_bool alpha;
    vrpn_bool textureDisplayed;
    vrpn_bool textureMode;
    vrpn_bool textureTransformMode;

} BehaviorLocks;

class nmg_SurfaceRegion
{
public:
    nmg_SurfaceRegion(nmg_Surface *parent, int region_id);
    ~nmg_SurfaceRegion();
    
    //This function is to ensure that things start off in 
    //the right state, by being able to copy a default region's
    //settings
    void copy(nmg_SurfaceRegion *other);

    void forceRebuildCondition();

    /*For the moment, to prevent needing to reallocate memory
      dynamically as a person is changing the region, make sure
      to pass in the full width and height of the entire surface.
      Later, may want to make this an automatically handled process
      based on the mask plane*/
    int init(int width, int height);
    
    /*These functions are for defining the area that this region
      is defined over.  There is no restriction that this be a
      connected region*/
    void setRegionControl(BCPlane *control);
    void setMaskPlane(nmg_SurfaceMask* mask);    
    void deriveMaskPlane(float min_height, float max_height);
    void deriveMaskPlane(float center_x, float center_y, float width,float height, 
                         float angle, nmb_Dataset *dataset);
    
    nmg_SurfaceMask* getMaskPlane() {return d_regionalMask;}

    //Appearance mutator functions
    void setAlpha(float alpha, vrpn_bool respect_lock);
    void enableFilledPolygons(int enable, vrpn_bool respect_lock);
    void setTextureDisplayed(int display, vrpn_bool respect_lock);
    void setTextureMode(int mode, vrpn_bool respect_lock);
    void setTextureTransformMode(int mode, vrpn_bool respect_lock);
    void setStride(int stride, vrpn_bool respect_lock);

    //Accessor functions
    float getAlpha() const {return d_currentState.alpha;}
    int getFilledPolygonsEnabled() const {return d_currentState.filledPolygonsEnabled;}
    int getTextureDisplayed() const {return d_currentState.textureDisplayed;}
    int getTextureMode() const {return d_currentState.textureMode;}
    int getTextureTransformMode() const {return d_currentState.textureTransformMode;}
    int getStride() const {return d_currentState.stride;}

    //Behavior locks
    //Lock is probably a bad word for these functions.  What they effectively control is
    //whether particular settings in question are tied to the default region of the surface
    //or not.
    void lockAlpha(vrpn_bool lock) {d_currentLocks.alpha = lock;}
    void lockFilledPolygons(vrpn_bool lock) {d_currentLocks.filledPolygonsEnabled = lock;}
    void lockTextureDisplayed(vrpn_bool lock) {d_currentLocks.textureDisplayed = lock;}
    void lockTextureMode(vrpn_bool lock) {d_currentLocks.textureMode = lock;}
    void lockTextureTransformMode(vrpn_bool lock) {d_currentLocks.textureTransformMode = lock;}
    void lockStride(vrpn_bool lock) {d_currentLocks.stride = lock;}
    
    //Rendering functions
    int rebuildRegion(nmb_Dataset *dataset, vrpn_bool force = VRPN_FALSE);
    int rebuildInterval(nmb_Dataset *dataset, int low_row, int high_row, int strips_in_x);	
    void renderRegion();

    Vertex_Struct ** getRegionData();
    
private:
    nmg_Surface *d_parent;
    int d_regionID;
    vrpn_bool d_needsFullRebuild;
    nmg_SurfaceMask *d_regionalMask;
    
    //Display list variables
    unsigned int d_list_base;
    int d_num_lists;
    //The surface region
    Vertex_Struct ** d_vertexPtr;
    unsigned int d_VertexArrayDim;
    
    //Variables to control partial surface rebuilding
    nmb_Interval last_marked;
    nmb_Interval update;
    nmb_Interval todo;	
    
    //Region's Graphics State
    GraphicsState d_currentState;
    BehaviorLocks d_currentLocks;

    //Need to be able to save all that graphics state due to
    //the persnickety globals
    GraphicsState d_savedState;
    
    //Miscellaneous internal functions
    void setUpdateAndTodo(int low_row, int high_row);
    //Since we are doing multi-pass, and some variables
    //are automatically turned off after being used, we need
    //to be able to save them beforehand
    void SaveBuildState();
    void RestoreBuildState();
    void SaveRenderState();
    void RestoreRenderState();
    void setTexture();
    void cleanUp();
};

#endif



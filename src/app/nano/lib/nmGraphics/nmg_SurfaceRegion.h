#ifndef NMG_SURFACE_REGION_H
#define NMG_SURFACE_REGION_H

#include <v.h>

#include <vrpn_Types.h>
#include <nmb_Interval.h>

#ifndef NMB_PLANE_SELECTION_H
#include <nmb_PlaneSelection.h>
#endif

class nmb_Interval;
class BCPlane;
class nmb_Dataset;
class nmb_PlaneSelection;
class nmg_SurfaceMask;
class nmg_Surface;
class nmg_State;

// structure for vertex array
struct Vertex_Struct {
        GLfloat Texcoord [3];
        GLubyte Color [4];
        GLshort Normal [3];
        GLfloat Vertex [3];
};

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

typedef struct _BehaviorAssociations
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
    nmg_SurfaceRegion(int region_id);
    ~nmg_SurfaceRegion();
    
    ///This function is to ensure that things start off in 
    ///the right state, by being able to copy a default region's
    ///settings
    void copy(nmg_SurfaceRegion *other);

    void forceRebuildCondition();

    /**For the moment, to prevent needing to reallocate memory
      dynamically as a person is changing the region, make sure
      to pass in the full width and height of the entire surface.
      Later, may want to make this an automatically handled process
      based on the mask plane*/
    int init(int width, int height);
    
    /**These functions are for defining the area that this region
      is defined over.  There is no restriction that this be a
      connected region*/
    void setRegionControl(BCPlane *control);
    void setMaskPlane(nmg_SurfaceMask* mask);    
    int deriveMaskPlane(float min_height, float max_height);
    int deriveMaskPlane(float center_x, float center_y, float width,float height, 
                        float angle);
    int deriveMaskPlane();

    int updateMaskPlane(nmb_Dataset *dataset, 
                         int low_row, int high_row, int strips_in_x);
    int needsUpdate();  ///< Mask changed? We need updating...
    
    nmg_SurfaceMask* getMaskPlane() {return d_regionalMask;}

    //Appearance mutator functions
    void setAlpha(float alpha, vrpn_bool force_set);
    void setFilledPolygons(int enable, vrpn_bool force_set);
    void setTextureDisplayed(int display, vrpn_bool force_set);
    void setTextureMode(int mode, vrpn_bool force_set);
    void setTextureTransformMode(int mode, vrpn_bool force_set);
    void setStride(int stride, vrpn_bool force_set);

    //Accessor functions
    float getAlpha() const {return d_currentState.alpha;}
    int getFilledPolygonsEnabled() const {return d_currentState.filledPolygonsEnabled;}
    int getTextureDisplayed() const {return d_currentState.textureDisplayed;}
    int getTextureMode() const {return d_currentState.textureMode;}
    int getTextureTransformMode() const {return d_currentState.textureTransformMode;}
    int getStride() const {return d_currentState.stride;}

    //Behavior associations
    
    void associateAlpha(vrpn_bool associate) 
            {d_currentAssociations.alpha = associate;}
    void associateFilledPolygons(vrpn_bool associate) 
            {d_currentAssociations.filledPolygonsEnabled = associate;}
    void associateTextureDisplayed(vrpn_bool associate)
            {d_currentAssociations.textureDisplayed = associate;}
    void associateTextureMode(vrpn_bool associate)
            {d_currentAssociations.textureMode = associate;}
    void associateTextureTransformMode(vrpn_bool associate)
            {d_currentAssociations.textureTransformMode = associate;}
    void associateStride(vrpn_bool associate)
            {d_currentAssociations.stride = associate;}
    
    //Rendering functions
    int determineInterval(nmb_Dataset *dataset, 
                          int low_row, int high_row, int strips_in_x,
                          bool do_update_mask = true);
    ///< Set what strips we're going to work on. 

    int rebuildRegion(nmb_Dataset *dataset, nmg_State * state, 
                      int display_lists_in_x, vrpn_bool force = VRPN_FALSE);
    ///< rebuild display lists for the whole region
    int rebuildInterval(nmb_Dataset *dataset, nmg_State * state,
                        int low_row, int high_row, int strips_in_x);	
    ///< rebuild display lists for only part of the region. 
    void renderRegion(nmg_State * state, nmb_Dataset *dataset);

    int recolorRegion();

    Vertex_Struct ** getRegionData();
    
private:
    int d_regionID;

    nmg_SurfaceMask *d_regionalMask;
    
    ///Display list variables
    unsigned int d_list_base;
    ///Display list variables
    int d_num_lists;
    ///The surface region
    Vertex_Struct ** d_vertexPtr;
    ///The surface region
    unsigned int d_VertexArrayDim;
    
    /// Strips that need recomputed, but may be overwritten with new data.
    nmb_Interval d_last_marked;
    /// Strips of surface to update this frame.
    nmb_Interval d_update;
    /// Strips of surface still left to recompute after full rebuild.
    nmb_Interval d_todo;	
    /// Apparent direction of the scan
    int d_scanDirection;
    /// Last strips updated;
    nmb_Interval d_last_nonempty_update;

    /// Strips of surface to update color this frame.
    nmb_Interval d_color_update;
    /// Strips of surface still left to recolor. 
    nmb_Interval d_color_todo;	

    
    ///Region's Graphics State
    GraphicsState d_currentState;
    ///Region's Graphics State
    BehaviorLocks d_currentAssociations;

    ///Need to be able to save all that graphics state due to
    ///the persnickety globals
    GraphicsState d_savedState;
    
    //Miscellaneous internal functions
    //Since we are doing multi-pass, and some variables
    //are automatically turned off after being used, we need
    //to be able to save them beforehand
    void SaveBuildState(nmg_State * state);
    void RestoreBuildState(nmg_State * state);
    void SaveRenderState(nmg_State * state);
    void RestoreRenderState(nmg_State * state);
    void setTexture(nmg_State * state, nmb_Dataset *data);
    void cleanUp();

    /// Build display lists!
    int build_list_set (nmb_Dataset *data,
                        const nmb_PlaneSelection &planes, 
                        nmg_SurfaceMask *mask,
                        nmg_State * state,
                        int strips_in_x,
                        Vertex_Struct **surface);
    int build_nulldata_polygon (const nmb_PlaneSelection &planes, 
                        nmg_SurfaceMask *mask,
                        nmg_State * state,
                        int strips_in_x,
                        Vertex_Struct **surface);

};

#endif



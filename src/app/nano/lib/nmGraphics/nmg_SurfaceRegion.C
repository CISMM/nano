/*===3rdtech===
  Copyright (c) 2001 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#include "nmg_SurfaceRegion.h"

#include <nmb_Dataset.h>
#include <nmb_PlaneSelection.h>
#include <PPM.h>
#include <BCGrid.h>
#include <BCPlane.h>

#include "nmg_State.h"
#include "globjects.h"
#include "surface_strip_create.h"  
#include "surface_util.h"  
#include "openGL.h"  // for check_extension()
#include "nmg_Graphics.h"

#include "nmg_Surface.h"
#include "nmg_SurfaceMask.h"

#include	"Timer.h"
// M_PI not defined for VC++, for some reason. 
#ifndef M_PI
#define M_PI		3.14159265358979323846
#endif

#define VERBOSECHECK(level)	if (spm_graphics_verbosity >= level) report_gl_errors()

#include "UTree.h"
#include "URPolygon.h"

extern UTree World;

/**
 * Access: Public
 */
nmg_SurfaceRegion::
nmg_SurfaceRegion(nmg_Surface * /*parent*/, int region_id)
{
    //d_parent = parent;
    d_regionID = region_id;
    d_needsFullRebuild = VRPN_FALSE;
    d_regionalMask = new nmg_SurfaceMask;
    d_vertexPtr = (Vertex_Struct **)NULL;
    d_VertexArrayDim = 0;

    d_currentState.stride = 1;
    d_currentState.justColor = VRPN_FALSE;
    d_currentState.alpha = 1.0f;
    d_currentState.filledPolygonsEnabled = 1;
    d_currentState.textureDisplayed = nmg_Graphics::NO_TEXTURES;
    d_currentState.textureTransformMode = nmg_Graphics::RULERGRID_COORD;
    d_currentState.textureMode = GL_FALSE;

    d_currentAssociations.stride = true;
    d_currentAssociations.justColor = true;
    d_currentAssociations.alpha = true;
    d_currentAssociations.filledPolygonsEnabled = true;
    d_currentAssociations.textureDisplayed = true;
    d_currentAssociations.textureTransformMode = true;
    d_currentAssociations.textureMode = true;

    d_list_base = 0;
    d_num_lists = 0;

}

/**
 * Access: Public
 * 
 */
nmg_SurfaceRegion::
~nmg_SurfaceRegion()
{
    for(unsigned int i=0;i < d_VertexArrayDim; i++) {
        delete [] d_vertexPtr[i];
    }
    delete [] d_vertexPtr;

    if (d_regionalMask) {
        delete d_regionalMask;
    }
}

/**
 * Allocates memory for each "vertex" array that
 *              the class needs
 * @return -1 on error, 0 on success. 
 * Access: Public
 */
int nmg_SurfaceRegion::
init(int width, int height)
{
    unsigned int dim;
 
    if(width<=height) {
        dim=height;
    }
    else {
        dim=width;
    }       
    
    if (dim == d_VertexArrayDim) {
        return 0;
    }

    if (d_regionalMask->init(width, height)) {
        fprintf(stderr, 
               "nmg_SurfaceRegion::init: Error, out of memory [0]\n");
        return -1;
    }
    
    unsigned int i;
    if (d_vertexPtr) {
        for (i = 0; i < d_VertexArrayDim; i++) {
          delete [] (d_vertexPtr[i]);
        }
        delete [] d_vertexPtr;
    }
    
    d_VertexArrayDim = dim;

    //d_vertexPtr = (Vertex_Struct **)malloc(d_VertexArrayDim * sizeof(Vertex_Struct **));
    d_vertexPtr = (Vertex_Struct **)new Vertex_Struct[d_VertexArrayDim];
    
    if (d_vertexPtr == NULL) {
        fprintf(stderr, 
               "nmg_SurfaceRegion::init: Error, out of memory [1]\n");
        return -1;
    }
    
    for (i=0; i < d_VertexArrayDim; i++) {
        d_vertexPtr[i] = new Vertex_Struct[d_VertexArrayDim * 2];
        
        if (d_vertexPtr[i] == NULL ) {
            fprintf(stderr, 
                 "nmg_SurfaceRegion::init: Error, out of memory [2]\n");
            return -1;
        }
    }
    
    return 0;
}

/**
 * Access: Public
 * 
 */
void nmg_SurfaceRegion::
copy(nmg_SurfaceRegion *other)
{
    d_currentState = other->d_currentState;
}

/**
 * Access: Public
 * 
 */
void nmg_SurfaceRegion::
forceRebuildCondition()
{
    d_needsFullRebuild = VRPN_TRUE;
}


/**
 * Access: Protected, Virtual
 * 
 */
void nmg_SurfaceRegion::
setTexture(nmg_State * state, nmb_Dataset *data)
{
    double surface_z_scale = 1.0;
    if (data) {
        BCPlane *heightPlane =
            data->inputGrid->getPlaneByName(state->heightPlaneName);
        if (heightPlane) surface_z_scale = heightPlane->scale();
    }
    
    switch (d_currentState.textureDisplayed) {
    case nmg_Graphics::NO_TEXTURES:
        // nothing to do here
        break;
    case nmg_Graphics::CONTOUR:
        glBindTexture(GL_TEXTURE_1D, state->tex_ids[CONTOUR_1D_TEX_ID]);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, 
                  state->tex_blend_func[CONTOUR_1D_TEX_ID]);
        break;
    case nmg_Graphics::ALPHA:
#if !(defined(__CYGWIN__) || defined(hpux) || defined(_WIN32))
        glBindTexture(GL_TEXTURE_3D, state->tex_ids[ALPHA_3D_TEX_ID]);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, 
                  state->tex_blend_func[ALPHA_3D_TEX_ID]);
#endif
    case nmg_Graphics::BUMPMAP:
#ifdef FLOW
        glBindTexture(GL_TEXTURE_2D, shader_tex_ids[BUMP_DATA_TEX_ID]);
#endif
        break;
    case nmg_Graphics::HATCHMAP:
#ifdef FLOW
        glBindTexture(GL_TEXTURE_2D, shader_tex_ids[HATCH_DATA_TEX_ID]);
#endif
        break;
    case nmg_Graphics::PATTERNMAP:
#ifdef FLOW
        glBindTexture(GL_TEXTURE_2D, shader_tex_ids[PATTERN_DATA_TEX_ID]);
#endif
        break;
    case nmg_Graphics::RULERGRID:
        glBindTexture(GL_TEXTURE_2D, state->tex_ids[RULERGRID_TEX_ID]);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, 
                  state->tex_blend_func[RULERGRID_TEX_ID]);
        break;
    case nmg_Graphics::COLORMAP:
        glBindTexture(GL_TEXTURE_2D, state->tex_ids[COLORMAP_TEX_ID]);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, 
                  state->tex_blend_func[COLORMAP_TEX_ID]);
        glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR,
                   state->tex_env_color[COLORMAP_TEX_ID]);
        break;
    case nmg_Graphics::SEM_DATA:
        glBindTexture(GL_TEXTURE_2D, state->tex_ids[SEM_DATA_TEX_ID]);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, 
                  state->tex_blend_func[SEM_DATA_TEX_ID]);
        glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR,
                   state->tex_env_color[SEM_DATA_TEX_ID]);
        break;
    case nmg_Graphics::REMOTE_DATA:
        glBindTexture(GL_TEXTURE_2D, state->tex_ids[REMOTE_DATA_TEX_ID]);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, 
                  state->tex_blend_func[REMOTE_DATA_TEX_ID]);
        break;
    case nmg_Graphics::VISUALIZATION:
        glBindTexture(GL_TEXTURE_2D, state->tex_ids[VISUALIZATION_TEX_ID]);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, 
                  state->tex_blend_func[VISUALIZATION_TEX_ID]);
        break;
    default:
        fprintf(stderr, "Error, unknown texture set for display\n");
        break;
    }
    
#if !(defined(__CYGWIN__) || defined(_WIN32))
	//#ifndef __CYGWIN__
    if ((state->texture_mode == GL_TEXTURE_1D) ||
        (state->texture_mode == GL_TEXTURE_3D_EXT)) {
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    }
#endif
    
    
#ifdef PROJECTIVE_TEXTURE
    
    if (d_currentState.textureMode == GL_TEXTURE_2D) {
        glPushAttrib(GL_TRANSFORM_BIT | GL_TEXTURE_BIT);
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_TEXTURE_GEN_S);
        glEnable(GL_TEXTURE_GEN_T);
        glEnable(GL_TEXTURE_GEN_R);
        glEnable(GL_TEXTURE_GEN_Q);
        
        glMatrixMode(GL_TEXTURE);
        glPushMatrix();
        glLoadIdentity();
        
        double theta = 0.0;
        GLdouble texture_matrix[16];
        double x_scale_factor = 1.0, y_scale_factor = 1.0;
        double x_translate = 0.0, y_translate = 0.0;

        switch (d_currentState.textureTransformMode) {
        case nmg_Graphics::RULERGRID_COORD:
            // use values from the older rulergrid adjustment interface
            glScalef(1.0/state->rulergrid_scale,1.0/state->rulergrid_scale,1.0);
            // 1.0/SCALE
            theta = asin(state->rulergrid_sin);
            if (state->rulergrid_cos < 0)
                theta = M_PI - theta;
            glRotated(theta*180.0/M_PI, 0.0, 0.0, 1.0);
              // -ROTATION
            glTranslatef(-state->rulergrid_xoffset, -state->rulergrid_yoffset, 0.0);
            // -TRANS.
            break;
        case nmg_Graphics::VIZTEX_COORD:
            glScalef(1.0/state->viztex_scale,1.0/state->viztex_scale,1.0);
            // 1.0/SCALE
            break;
        case nmg_Graphics::MANUAL_REALIGN_COORD:
            compute_texture_matrix(state->translate_tex_x, state->translate_tex_y,
                                   state->tex_theta_cumulative, state->scale_tex_x,
                                   state->scale_tex_y, state->shear_tex_x, state->shear_tex_y,
                                   state->tex_coord_center_x, state->tex_coord_center_y,
                                   texture_matrix);
            glLoadMatrixd(texture_matrix);
            break;
        case nmg_Graphics::PER_QUAD_COORD:
            break;
        case nmg_Graphics::REGISTRATION_COORD:
            glLoadIdentity();
            // scale by actual texture image given divided by texture image
            // used (i.e. the one actually in texture memory is a power of 2
            // but the one we were given had some smaller size)
            switch (state->texture_displayed) {
            case nmg_Graphics::NO_TEXTURES:
            case nmg_Graphics::BUMPMAP:
            case nmg_Graphics::HATCHMAP:
            case nmg_Graphics::PATTERNMAP:
				// nothing to do here
                break;
            case nmg_Graphics::RULERGRID:
                x_scale_factor = (double)state->tex_image_width[RULERGRID_TEX_ID]/
                    (double)state->tex_installed_width[RULERGRID_TEX_ID];
                y_scale_factor = (double)state->tex_image_height[RULERGRID_TEX_ID]/
                    (double)state->tex_installed_height[RULERGRID_TEX_ID];
                x_translate = (double)state->tex_image_offsetx[RULERGRID_TEX_ID]/
                    (double)state->tex_installed_width[RULERGRID_TEX_ID];
                y_translate = (double)state->tex_image_offsety[RULERGRID_TEX_ID]/
                    (double)state->tex_installed_height[RULERGRID_TEX_ID];
                break;
            case nmg_Graphics::COLORMAP:
                x_scale_factor = (double)state->tex_image_width[COLORMAP_TEX_ID]/
                    (double)state->tex_installed_width[COLORMAP_TEX_ID];
                y_scale_factor = (double)state->tex_image_height[COLORMAP_TEX_ID]/
                    (double)state->tex_installed_height[COLORMAP_TEX_ID];
                x_translate = (double)state->tex_image_offsetx[COLORMAP_TEX_ID]/
                    (double)state->tex_installed_width[COLORMAP_TEX_ID];
                y_translate = (double)state->tex_image_offsety[COLORMAP_TEX_ID]/
                    (double)state->tex_installed_height[COLORMAP_TEX_ID];
                break;
            case nmg_Graphics::SEM_DATA:
                x_scale_factor = (double)state->tex_image_width[SEM_DATA_TEX_ID]/
                    (double)state->tex_installed_width[SEM_DATA_TEX_ID];
                y_scale_factor = (double)state->tex_image_height[SEM_DATA_TEX_ID]/
                    (double)state->tex_installed_height[SEM_DATA_TEX_ID];
                x_translate = (double)state->tex_image_offsetx[SEM_DATA_TEX_ID]/
                    (double)state->tex_installed_width[SEM_DATA_TEX_ID];
                y_translate = (double)state->tex_image_offsety[SEM_DATA_TEX_ID]/
                    (double)state->tex_installed_height[SEM_DATA_TEX_ID];
                break;
            case nmg_Graphics::REMOTE_DATA:
                x_scale_factor = (double)state->tex_image_width[REMOTE_DATA_TEX_ID]/
                    (double)state->tex_installed_width[REMOTE_DATA_TEX_ID];
                y_scale_factor = (double)state->tex_image_height[REMOTE_DATA_TEX_ID]/
                    (double)state->tex_installed_height[REMOTE_DATA_TEX_ID];
                x_translate = (double)state->tex_image_offsetx[REMOTE_DATA_TEX_ID]/
                    (double)state->tex_installed_width[REMOTE_DATA_TEX_ID];
                y_translate = (double)state->tex_image_offsety[REMOTE_DATA_TEX_ID]/
                    (double)state->tex_installed_height[REMOTE_DATA_TEX_ID];
                break;
            case nmg_Graphics::VISUALIZATION:
                x_scale_factor = (double)state->tex_image_width[VISUALIZATION_TEX_ID]/
                    (double)state->tex_installed_width[VISUALIZATION_TEX_ID];
                y_scale_factor = (double)state->tex_image_height[VISUALIZATION_TEX_ID]/
                    (double)state->tex_installed_height[VISUALIZATION_TEX_ID];
                x_translate = (double)state->tex_image_offsetx[VISUALIZATION_TEX_ID]/
                    (double)state->tex_installed_width[VISUALIZATION_TEX_ID];
                y_translate = (double)state->tex_image_offsety[VISUALIZATION_TEX_ID]/
                    (double)state->tex_installed_height[VISUALIZATION_TEX_ID];
                break;
            default:
                fprintf(stderr, "Error, unknown texture set for display\n");
                break;
            }
            // XXX the following two lines to compensate for the texture border
            // does nearly the same thing as the code in 
            // nmb_Image::getImageToTextureTransform() so we could
            // replace all the above related calculations and 
            // image-related state with an nmb_Image object per texture
            // See code in ImageViewer::drawImageAsTexture() for example.
            // (AAS, 8-10-01)

            glTranslated(x_translate, y_translate, 0.0);
            glScaled(x_scale_factor, y_scale_factor, 1.0);

            glMultMatrixd(state->texture_transform);
            glScaled(1.0, 1.0, 1.0/surface_z_scale);
            break;
            //case nmg_Graphics::REMOTE_COORD;
            //glLoadIdentity();
        default:
            fprintf(stderr, "Error, unknown texture coordinate mode\n");
            break;
        }
    }
#endif
    
    if (d_currentState.textureMode == GL_TEXTURE_1D) {
        glEnable(GL_TEXTURE_1D);
    }
}

/**
 * Unset or disable whatever needs to be after 
 *              rendering is complete
 * Access: Protected
 */
void nmg_SurfaceRegion::
cleanUp()
{
    if (d_currentState.textureMode == GL_TEXTURE_1D) {
        glDisable(GL_TEXTURE_1D);
    }
    
    
#ifdef PROJECTIVE_TEXTURE
    
    if (d_currentState.textureMode == GL_TEXTURE_2D){
        glMatrixMode(GL_TEXTURE);
        glPopMatrix();
        
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_TEXTURE_GEN_S);
        glDisable(GL_TEXTURE_GEN_T);
        glDisable(GL_TEXTURE_GEN_R);
        glDisable(GL_TEXTURE_GEN_Q);
        glPopAttrib();
    }
    
#endif

    
#ifdef RENDERMAN
    // Save the viewing/modeling matrix to be used in RenderMan.c
    glGetFloatv(GL_PROJECTION_MATRIX, cur_projection_matrix);
    glGetFloatv(GL_MODELVIEW_MATRIX,  cur_modelview_matrix);
#endif
}

/**
 * Set the plane that controls the functions that
 *              automatically derive the masking plane
@Note for default region should always be the height plane. 
 * Access: Public
 */
void nmg_SurfaceRegion::
setRegionControl(BCPlane *control)
{
    d_regionalMask->setControlPlane(control);
}

/**
 * Manually set the mask plane
 * Access: Public
 @alert Don't call this on the default region!!!
 */
void nmg_SurfaceRegion::
setMaskPlane(nmg_SurfaceMask *mask)
{
    if (d_regionalMask) {
        delete d_regionalMask;
    }
    d_regionalMask = mask;
    d_needsFullRebuild = VRPN_TRUE;
}

/**
 * Create a masking plane, using a range of
 *              height values
 * Access: Public
 */
int nmg_SurfaceRegion::
deriveMaskPlane(float min_height, float max_height)
{   
    int ret = d_regionalMask->deriveMask(min_height, max_height);
    if (ret) d_needsFullRebuild = true;
    return ret;
            
}

/**
 * Create a masking plane, using an oriented box
 * Access: Public
 */
int nmg_SurfaceRegion::
deriveMaskPlane(float center_x, float center_y, float width,float height, 
                float angle)
{
     int ret = d_regionalMask->deriveMask(center_x, center_y, width, height, angle);
    if (ret) d_needsFullRebuild = true;
    return ret;
}

/**
 * Create a masking plane, using invalid data range
 * Access: Public
 */
int nmg_SurfaceRegion::
deriveMaskPlane()
{   
    int ret = d_regionalMask->deriveMask();
    if (ret) d_needsFullRebuild = true;
    return ret;
}

/**
 * Access: Public
 * @return 1 if whole region needs rebuilt, 0 if only strips between
 * low_row and high_row needs rebuilt. 
 */
int nmg_SurfaceRegion::
updateMaskPlane(nmb_Dataset *dataset, int low_row, int high_row, int strips_in_x) 
{
    return(d_regionalMask->update(dataset, d_currentState.stride,
                                  low_row, high_row, strips_in_x));
}

/**
 * Access: Public
 */
int nmg_SurfaceRegion::
needsUpdate() 
{
    return d_regionalMask->needsUpdate();
}

/**
 * Set the alpha value to use
 * Access: Public
 */
void nmg_SurfaceRegion::
setAlpha(float alpha, vrpn_bool respect_unassociate)
{
    if (d_currentAssociations.alpha || !respect_unassociate) {
        d_currentState.alpha = alpha;
        d_needsFullRebuild = VRPN_TRUE;
    }
}

/**
 * Determine whether wire frame is enabled for
 *				this region
 * Access: Public
 */
void nmg_SurfaceRegion::
enableFilledPolygons(int enable, vrpn_bool respect_unassociate)
{
    if (d_currentAssociations.filledPolygonsEnabled || !respect_unassociate) {
        d_currentState.filledPolygonsEnabled = enable;
    }
}

/**
 * Set the tesselation stride to use for this
 * region
 * Access: Public
 */
void nmg_SurfaceRegion::
setStride(int stride, vrpn_bool respect_unassociate)
{
    if (d_currentAssociations.stride || !respect_unassociate) {
        d_currentState.stride = stride;
        d_needsFullRebuild = VRPN_TRUE;
    }
}

/**
 * Set which texture is to be displayed in this
 * region
 * Access: Public
 */
void nmg_SurfaceRegion::
setTextureDisplayed(int display, vrpn_bool respect_unassociate)
{
    if (d_currentAssociations.textureDisplayed || !respect_unassociate) {
        d_currentState.textureDisplayed = display;
        d_needsFullRebuild = VRPN_TRUE;
    }
}

/**
 * Access: Public
 * 
 */
void nmg_SurfaceRegion::
setTextureMode(int mode, vrpn_bool respect_unassociate)
{
    if (d_currentAssociations.textureMode || !respect_unassociate) {
        d_currentState.textureMode = mode;
        d_needsFullRebuild = VRPN_TRUE;
    }
}

/**
 * Access: Public
 * 
 */
void nmg_SurfaceRegion::
setTextureTransformMode(int mode, vrpn_bool respect_unassociate)
{
    if (d_currentAssociations.textureTransformMode || !respect_unassociate) {
        d_currentState.textureTransformMode = mode;
        d_needsFullRebuild = VRPN_TRUE;
    }
}


/**
 * The registration code needs explicit access 
 * to this structure
 * Access: Public
 */
Vertex_Struct ** nmg_SurfaceRegion::
getRegionData()
{
    return d_vertexPtr;
}

/**
 * Sets a flag that the next rebuild needs to color the whole surface, 
 * but not necessarily change the vertices and normals. 
 * Access: Public
 * @return -1 on error, 0 on success. 
 */
int nmg_SurfaceRegion::
recolorRegion()
{  
    d_currentState.justColor = VRPN_TRUE;
    return 0;
}


/**
 * Saves the state of global variables
 * Access: Protected
 * @note This function wil be unnecessary once we get
 *            rid of all the Global variables!
 */
void nmg_SurfaceRegion::
SaveBuildState(nmg_State * state)
{
    // Taken care of in recolorRegion and rebuildRegion
  // d_savedState.justColor = state->just_color;  //This one gets
                                     //automatically changed
    d_savedState.stride = state->stride;
    d_savedState.alpha = state->surface_alpha;
}

/**
 * Access: Protected
 * @note This function wil be unnecessary once we get
 *              rid of all the Global variables!
 */
void nmg_SurfaceRegion::
RestoreBuildState(nmg_State * state)
{
    // Taken care of in recolorRegion and rebuildRegion
  // state->just_color = d_savedState.justColor;
    state->stride = d_savedState.stride;
    state->surface_alpha = d_savedState.alpha;
}

/**
 * Saves the state of global variables
 * Access: Protected
 * @note This function wil be unnecessary once we get
 *              rid of all the Global variables!
 */
void nmg_SurfaceRegion::
SaveRenderState(nmg_State * state)
{
    d_savedState.filledPolygonsEnabled = state->config_filled_polygons;
    d_savedState.textureDisplayed = state->texture_displayed;
    d_savedState.textureMode = state->texture_mode;
    d_savedState.textureTransformMode = state->texture_transform_mode;
}

/**
 * Access: Protected
 * @note This function wil be unnecessary once we get
 *              rid of all the Global variables!
 */
void nmg_SurfaceRegion::
RestoreRenderState(nmg_State * state)
{
    state->config_filled_polygons = d_savedState.filledPolygonsEnabled;
    state->texture_displayed = d_savedState.textureDisplayed;
    state->texture_mode = d_savedState.textureMode;
    state->texture_transform_mode = d_savedState.textureTransformMode;
}


// Define EXPENSIVE_DISPLAY_LISTS if recomputing display lists is very
// expensive on the current architecture (but you still want to use them).
// It recomputes only those lists which are not likely to have to be
// recomputed in the near future as more data comes in from the scope.

// This feature has not been used in any of our releases (since 2000?)
// #define EXPENSIVE_DISPLAY_LISTS

/**
 * Access: Public
 * Determine what strips we are going to work on.
 * @return true if there are any strips to work on, i.e.
 * either todo or update is not empty. 
 */
int nmg_SurfaceRegion::
determineInterval(nmb_Dataset *dataset, 
                  int low_row, int high_row, int strips_in_x) 
{
    // Update our mask, and maybe rebuild entire region. 
    if (updateMaskPlane(dataset, low_row, high_row, strips_in_x) ||
        d_needsFullRebuild) {
        // Yup, the whole region needs to be rebuilt. 
        low_row = 0;
        if (strips_in_x) {
            high_row = dataset->inputGrid->numY() -1;
        } else {
            high_row = dataset->inputGrid->numX() -1;
        }
        // The whole region will now be rebuilt, so clear flag
        d_needsFullRebuild = VRPN_FALSE;
    } else {
        // Check for empty range of change
        if (low_row > high_row) {

        }
    }
    int low_strip =  low_row / d_currentState.stride;
    int high_strip = high_row / d_currentState.stride;

    // Let's make it a run-time decision, based on grid resolution.
    //#ifdef EXPENSIVE_DISPLAY_LISTS
    //if (dataset->inputGrid->numX() > 499) {
    // Ah, well. I currently believe it doesn't work, but has the
    // potential to work, valuable for large grids. (11 Mar 2002)
    if (0) {
    nmb_Interval mark;

    // Figure out what direction the scan has apparently progressed
    // since the last time.  Heuristic, error-prone, but safe.  
    
    int direction = 0;
    if (low_strip >= last_marked.low()) direction++;
    if (high_strip <= last_marked.high()) direction--;
    
    if (spm_graphics_verbosity >= 6)
        fprintf(stderr, "  Drawing in direction %d (from %d to %d).\n",
		direction, low_strip, high_strip);
        
    // Recompute as few display lists as necessary.  The rest will
    // be recomputed on the next screen refresh, unless more data
    // comes in from the scope.  Some vertical "tears" will be left
    // in the rendered image.
    switch (direction) {
        case 1:
            update = nmb_Interval (low_strip - 2, high_strip - 1);
            mark = nmb_Interval (high_strip, high_strip + 2);
            break;
        case 0:
            // leave update empty!
            update = nmb_Interval ();
            // XXX safer (but more work for the graphics pipe,
            //     and probably unnecessary) might be
            //     update = nmb_Interval (low_strip, high_strip);
            mark = nmb_Interval (low_strip - 2, high_strip + 2);
            break;
        case -1:
            update = nmb_Interval (low_strip - 1, high_strip + 2);
            mark = nmb_Interval (low_strip - 2, low_strip - 2);
            break;
    }
    if (spm_graphics_verbosity >= 6)
        fprintf(stderr, "   Update set is %d - %d, last_marked is %d - %d, "
		"mark is %d - %d.\n",
		update.low(), update.high(), last_marked.low(),
		last_marked.high(), mark.low(), mark.high());
    VERBOSECHECK(6);
        
    // Draw this time what we have to update due to the most recent
    // changes, plus what we delayed updating last time expecting them
    // to overlap with the most recent changes, minus what we're delaying
    // another frame expecting it to overlap with the next set of changes.
    // NOTE:  this is not a correct implementation of interval subtraction.
    
    todo = last_marked - mark;
    
    last_marked = mark;

  } else {
    // Recompute every display list that needs to be modified
    // to reflect this change.  If we're in the middle of the
    // scan many of these may have to be recomputed after the
    // next set of data is received.
    update = nmb_Interval ( low_strip-2, high_strip+1);
    // Yes, this can go outside the surface! Clamped later...

    // leave mark empty!
  }
    
    return (!update.empty() || !todo.empty());
}

/**
 *  Replace the display lists that have had points changed.
 * This includes those that are in the region of changed
 * points and also those that are past the edge of this
 * region, as these points will have had their normals
 * adjusted (in the picture below, '+' represents a point
 * that is on the stride, '.' indicates a point off stride,
 * and the lists that need to be redrawn because of a point
 * change is shown).  The normal of the two on-stride points
 * around the changed point is changed, requiring the redraw
 * of all strips that include these points.
 *
 * If the "changed" row is r, then the index of 
 * "list" is r/stride (integer division)
 * @code
 *            norm   changed  norm                        
 * 	+ . . .	+ . . .	+ . . .	+ . . .	+ . . .	+
 * 	|_______|_______|_______|_______|_______|
 *       list-2  list-1  list    list+1
 * @endcode
 * @return -1 on error, 0 on success. 
 * Access: Public
 */
int nmg_SurfaceRegion::
rebuildInterval(nmb_Dataset *dataset, nmg_State * state, int low_row, int high_row, int strips_in_x)
{
    // First determine if we need rebuild anything. 
    if (!determineInterval(dataset, low_row, high_row, strips_in_x) && !d_currentState.justColor) {
        // Nothing to do. 
        return 0;
    }
    // Debug
//      printf("update %d %d\ttodo %d %d\t last mrk %d %d\n",
//             update.low(), update.high(), todo.low(), todo.high(), 
//             last_marked.low(), last_marked.high());

    nmb_PlaneSelection planes;
    planes.lookup(dataset);
    
    SaveBuildState(state);

    state->just_color = d_currentState.justColor;
    state->stride = d_currentState.stride;
    state->surface_alpha = d_currentState.alpha;

    if (d_regionalMask->nullDataType()) {
        // For a null data region, only render one polygon. 
        if (build_nulldata_polygon(planes, d_regionalMask, state,
                               strips_in_x, d_vertexPtr)) return -1;
    
    } else if (update.overlaps(todo) || update.adjacent(todo)) {
        // If update and todo are contiguous, combine them and do them
        // as a single interval...
        if (build_list_set(update + todo, planes, d_regionalMask, state,
                           strips_in_x, d_vertexPtr)) return -1;
    } 
    else {
        if (build_list_set(update, planes, d_regionalMask, state, 
                           strips_in_x, d_vertexPtr)) return -1;
        // Note: "todo" is only used with a seldom(never) used #define above.
        if (!todo.empty()) {
            if (build_list_set(todo, planes, d_regionalMask, state, 
                               strips_in_x, d_vertexPtr)) return -1;
        }
    }
    
    RestoreBuildState(state);
    d_currentState.justColor = VRPN_FALSE;

    return 0;
}

/**
 * Renders the region. Called once each frame. 
 * Access: Public
 */
void nmg_SurfaceRegion::
renderRegion(nmg_State * state, nmb_Dataset *dataset)
{
    //	static bool set = false;
    int i;
    
    SaveRenderState(state);

    setTexture(state, dataset); 


    state->config_filled_polygons = d_currentState.filledPolygonsEnabled;
    state->texture_mode = d_currentState.textureMode;
    state->texture_displayed = d_currentState.textureDisplayed;
    state->texture_transform_mode = d_currentState.textureTransformMode;

    set_gl_surface_materials(state);
    setFilled(state);

    for (i = 0; i < d_num_lists; i++) {
        glCallList(d_list_base + i);
    }

	// Drawing Objects...
//	if (state->config_enableUber) {
    int proj_text = (d_currentState.textureMode == GL_TEXTURE_2D) ? 1 : 0;
    World.Do(&URender::Render, &proj_text);
//	}
 
    cleanUp();
    RestoreRenderState(state);
}

/**
* Called by rebuildInterval, calls the other build_list_set.
* It's useful to not have to pass in the strip function to this
* procedure.  It makes the Visualization classes simpler.
* @return -1 on error, 0 on success. 
*/
int nmg_SurfaceRegion::build_list_set (
    const nmb_Interval &insubset,
    const nmb_PlaneSelection &planes, nmg_SurfaceMask *mask,
    nmg_State * state,
    int strips_in_x, Vertex_Struct **surface)
{
    
    v_gl_set_context_to_vlib_window(); 
    
    int (* stripfn)
      (nmg_State * state, const nmb_PlaneSelection&, nmg_SurfaceMask *, 
	 GLdouble [3], int, Vertex_Struct *);
    
    // If we have a very small grid size, make sure state->stride doesn't tell
    // us to skip any.
    if ((planes.height->numY() <= 10) || (planes.height->numX() <= 10)) {
        state->stride = 1;
    }

    int new_num_lists; // used later
    if (strips_in_x) {
        new_num_lists = (planes.height->numY() - 1) / state->stride;
        stripfn = spm_x_strip_masked;
    } else {
        new_num_lists = (planes.height->numX() - 1) / state->stride;
        stripfn = spm_y_strip_masked;
    }
    
    // Handle being called for the first time when there are no display lists,
    // or when number of display lists change. 
    if (d_num_lists != new_num_lists) {
        if (d_num_lists != 0) {
            glDeleteLists(d_list_base, d_num_lists);
        }
        // Generate a new set of display list indices
        if ( (d_list_base = glGenLists(new_num_lists)) == 0) {
            fprintf(stderr,
                    "build_list_set(): Couldn't get indices\n");
            return(-1);
        }
        if (spm_graphics_verbosity >= 6) {
            fprintf(stderr, "    allocated display lists from %d for %d.\n",
                    d_list_base, new_num_lists);
        }
#if defined(sgi) || defined(_WIN32)
        // use vertex array extension
        if (state->VERTEX_ARRAY) {
            glEnableClientState(GL_VERTEX_ARRAY);
            // Color arrays are enabled/disabled dynamically depending
            // on whether or not planes.color or state->PRERENDERED_COLORS are
            // valid.
            if (!state->PRERENDERED_COLORS && !state->PRERENDERED_TEXTURE) {
                glEnableClientState(GL_NORMAL_ARRAY);
            }
        }
#endif
        d_num_lists = new_num_lists;
        
    }
    
    nmb_Interval subset (MAX(0, insubset.low()),
        MIN(d_num_lists - 1, insubset.high()));
    
    return build_list_set(subset, planes, mask, state, 
                          d_list_base, d_num_lists,
                          state->surfaceColor, stripfn, surface);
}

/**
* This routine creates display lists for the grid.
* It relies on an external routine to determine the set of lists to
* make, the direction (whether x or y is faster), and other important
* variables.
* @return -1 on error, 0 on success. 
*/

int nmg_SurfaceRegion::build_list_set(
    const nmb_Interval &subset,
    const nmb_PlaneSelection &planes, nmg_SurfaceMask *mask,
    nmg_State * state,
    GLuint base,
    GLsizei num_lists,
    GLdouble * surfaceColor,
    int (* stripfn)
    (nmg_State * state, const nmb_PlaneSelection&, nmg_SurfaceMask *, GLdouble [3], int, Vertex_Struct *),
    Vertex_Struct **surface)
{
    
    v_gl_set_context_to_vlib_window(); 
    // globals:
    // surface
    // min/maxColor
    
    int i;
    
    if (!state->just_color && subset.empty()) return 0;
    
#if defined(sgi) || defined(_WIN32)
    //#if defined(sgi)
    if (state->VERTEX_ARRAY) { // same extension is for COLOR_ARRAY
        
        //The checks for whether to use GL_COLOR_ARRAY or not have been
        //removed because of the surface alpha being a global AND not
        //compiled into the display lists when the color isn't set over the
        //entire array.  Since I've moved the real surface color into the
        //new Surface and SurfaceRegion classes, this meant that if you set
        //the surface alpha it would get overriden.  This is only a problem
        //right now because I am trying to avoid sweeping changes, so I am
        //leaving the globals alone and just pushing and poping state appropriately.
        //That fails though in this case, however, when we get rid of globals
        //and refactor all of this appropriately, this problem will disappear.
        //if (planes.color || state->PRERENDERED_COLORS || state->PRERENDERED_TEXTURE ||
        //    state->null_data_alpha_toggle || state->transparent) {
            glEnableClientState(GL_COLOR_ARRAY);
        //} else {
        //    glDisableClientState(GL_COLOR_ARRAY);
        //}
        if (glGetError()!=GL_NO_ERROR) {
            printf(" Error setting GL_COLOR_ARRAY_EXT.\n");
        }
    }
    
#ifdef PROJECTIVE_TEXTURE
    GLfloat eyePlaneS[] =
    {1.0, 0.0, 0.0, 0.0};
    GLfloat eyePlaneT[] =
    {0.0, 1.0, 0.0, 0.0};
    GLfloat eyePlaneR[] =
    {0.0, 0.0, 1.0, 0.0};
    GLfloat eyePlaneQ[] =
    {0.0, 0.0, 0.0, 1.0};
    
    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
    glTexGenfv(GL_S, GL_OBJECT_PLANE, eyePlaneS);
    
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
    glTexGenfv(GL_T, GL_OBJECT_PLANE, eyePlaneT);
    
    glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
    glTexGenfv(GL_R, GL_OBJECT_PLANE, eyePlaneR);
    
    glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
    glTexGenfv(GL_Q, GL_OBJECT_PLANE, eyePlaneQ);
    if (report_gl_errors()) {
        printf(" Error calling glTexGen.\n");
    } 
    



#endif
    
    if (state->VERTEX_ARRAY) { // same extension is for TEXTURE_COORD_ARRAY
#ifndef PROJECTIVE_TEXTURE
        if (state->texture_displayed != nmg_Graphics::NO_TEXTURES) {
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        } else {
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        }
        if (glGetError() != GL_NO_ERROR) {
            printf(" Error setting GL_TEXTURE_COORD_ARRAY_EXT.\n");
        } 
#else
        if ( planes.contour) {
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
#ifdef sgi
        } else if ( planes.alpha ) {
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
#endif
        } else {
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        }
        if (glGetError() != GL_NO_ERROR) {
            printf(" Error setting GL_TEXTURE_COORD_ARRAY_EXT.\n");
        } 
#endif // PROJECTIVE_TEXTURE
    }
    
#endif // sgi or win32
    
    if (spm_graphics_verbosity >= 15) { 
        fprintf(stderr, "  updating %d - %d", subset.low(), subset.high());
    } 
    // Store state->just_color
    vrpn_bool just_color_was_on = state->just_color;
    // If we are re-doing the whole surface, we don't need to then
    // re-do the color, so turn flag off.
    if ( (subset.low() == 0) && (subset.high() == num_lists -1) ) {
        just_color_was_on = 0;
    }

    // turn state->just_color off so only geometry gets re-generated
    state->just_color = 0;
    for (i = subset.low(); i <= subset.high(); i++) {
        
        if (spm_graphics_verbosity >= 10) {
            fprintf(stderr, "    newing list %d for strip %d.\n", base + i, i);
        }
        
        // Delete lists we are going to create. Frees memory for new lists. 
        glDeleteLists(base + i, 1);
        // Create or replace existing list with new strip. 
        glNewList(base + i, GL_COMPILE);

        
        VERBOSECHECK(10);
        
        if ((*stripfn)(state, planes, mask, surfaceColor, i*state->stride, surface[i])) {
            if (state->VERTEX_ARRAY) {
                fprintf(stderr, "build_list_set():  "
                    "Internal error - bad strip(vertex array)\n");
            }
            else {
                fprintf(stderr, "build_list_set():  "
                    "Internal error - bad strip\n");
            }
            return -1;
        }
        
        if (spm_graphics_verbosity >= 10) {
            fprintf(stderr, "    updated %d.\n", i);
        }
        VERBOSECHECK(10);
        
        glEndList();
    }
    if ( just_color_was_on ) {
        // Flag tells stripfn to only regenerate color, and use cached normals 
        // and vertices. 
        state->just_color = 1;
        // Delete lists we are going to create. Frees memory for new lists. 
        glDeleteLists(base, num_lists);
        // re-color the whole surface
        for (i = 0; i < num_lists; i++) {
            
            if (spm_graphics_verbosity >= 10) {
                fprintf(stderr, "    newing list %d for strip %d.\n", base + i, i);
            }
            
            glNewList(base + i, GL_COMPILE);

            VERBOSECHECK(10);
            
            if ((*stripfn)(state, planes, mask, surfaceColor, i*state->stride, surface[i])) {
                if (state->VERTEX_ARRAY) {
                    fprintf(stderr, "build_list_set():  "
                        "Internal error - bad strip(vertex array)\n");
                }
                else {
                    fprintf(stderr, "build_list_set():  "
                        "Internal error - bad strip\n");
                }
                return -1;
            }          
            
            if (spm_graphics_verbosity >= 10) {
                fprintf(stderr, "    updated %d.\n", i);
            }
            VERBOSECHECK(10);
            
            glEndList();
        }
    }

    state->just_color = 0;
    return 0;
}

/**
* Called by rebuildInterval, 
* creates a single polygon to represent null data region. 
* @return -1 on error, 0 on success. 
*/
int nmg_SurfaceRegion::build_nulldata_polygon (
    const nmb_PlaneSelection &planes, nmg_SurfaceMask *mask,
    nmg_State * state,
    int strips_in_x, Vertex_Struct **surface)
{
    v_gl_set_context_to_vlib_window(); 
    
    if (d_num_lists == 0) {
        // Generate a new set of display list indices
        if ( (d_list_base = glGenLists(1)) == 0) {
            fprintf(stderr,
                    "build_nulldata_polygon(): Couldn't get indices\n");
            return(-1);
        }
        d_num_lists = 1;
    }
    short minValid, maxValid;
    short width, height;
    mask->getValidRange(&minValid, &maxValid);
    if (minValid == -1) {
        minValid = maxValid = 0;
    }
    if (strips_in_x) {
        width = planes.height->numX() -1;
        height = planes.height->numY() -1;
    } else {
        width = planes.height->numY() -1;
        height = planes.height->numX() -1;
    }
    GLfloat Normal [3];
    Normal[0] = 0;
    Normal[1] = 0;
    Normal[2] = 1;

    GLfloat Vertex [3];
    // Plane at 0 in Z coordinate, but put just below
    // to avoid co-planar polygons during region mode.  
    Vertex[2]= -1;

    glNewList(d_list_base, GL_COMPILE);

    if (minValid != 0) {
        // Draw a quad from one edge to the minimum valid data row
        glBegin(GL_TRIANGLE_STRIP);
        
        glNormal3fv(Normal);

        Vertex[0]= (float) planes.height->xInWorld(0);
        Vertex[1]= (float) planes.height->yInWorld(0);
        glVertex3fv(Vertex);

        Vertex[0]= (float) planes.height->xInWorld(strips_in_x ? 0:minValid);
        Vertex[1]= (float) planes.height->yInWorld(strips_in_x ? minValid:0);
        glVertex3fv(Vertex);

        Vertex[0]= (float) planes.height->xInWorld(strips_in_x ? width:0);
        Vertex[1]= (float) planes.height->yInWorld(strips_in_x ? 0:width);
        glVertex3fv(Vertex);
        
        Vertex[0]= (float) planes.height->xInWorld(strips_in_x ? width:minValid);
        Vertex[1]= (float) planes.height->yInWorld(strips_in_x ? minValid:width);
        glVertex3fv(Vertex);
        
        glEnd();
    } 
    if (maxValid != height) {
        // Draw a quad from the opposite edge to the maximum valid data row
        glBegin(GL_TRIANGLE_STRIP);
        
        glNormal3fv(Normal);

        Vertex[0]= (float) planes.height->xInWorld(strips_in_x ? 0:maxValid);
        Vertex[1]= (float) planes.height->yInWorld(strips_in_x ? maxValid:0);
        glVertex3fv(Vertex);

        Vertex[0]= (float) planes.height->xInWorld(strips_in_x ? 0:height);
        Vertex[1]= (float) planes.height->yInWorld(strips_in_x ? height:0);
        glVertex3fv(Vertex);

        Vertex[0]= (float) planes.height->xInWorld(strips_in_x ? width:maxValid);
        Vertex[1]= (float) planes.height->yInWorld(strips_in_x ? maxValid:width);
        glVertex3fv(Vertex);
        
        Vertex[0]= (float) planes.height->xInWorld(strips_in_x ? width:height);
        Vertex[1]= (float) planes.height->yInWorld(strips_in_x ? height:width);
        glVertex3fv(Vertex);
        
        glEnd();
    }
    glEndList();

    return 0;
}


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
nmg_SurfaceRegion(int region_id)
{
    d_regionID = region_id;
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

    d_scanDirection = 0;
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

    d_vertexPtr = new Vertex_Struct*[d_VertexArrayDim];
    
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
 * This region should be completely rebuilt. 
 * Access: Public
 */
void nmg_SurfaceRegion::
forceRebuildCondition()
{
    if (d_regionalMask) d_regionalMask->forceUpdate();
}

void nmg_SurfaceRegion::
setTextureTransform(nmg_State * state, nmb_Dataset *data) {

	glPushAttrib(GL_TRANSFORM_BIT);
	glMatrixMode(GL_TEXTURE);
    double surface_z_scale = 1.0;
    if (data) {
        BCPlane *heightPlane =
            data->inputGrid->getPlaneByName(state->heightPlaneName);
        if (heightPlane) surface_z_scale = heightPlane->scale();
    }
        
    double theta = 0.0;

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
    case nmg_Graphics::PER_QUAD_COORD:
        break;
    case nmg_Graphics::SURFACE_REGISTRATION_COORD:
		glMultMatrixd(state->surfaceModeTextureTransform);
		if (surface_z_scale != 0) {
			glScaled(1.0, 1.0, 1.0/surface_z_scale);
		}
        break;
    case nmg_Graphics::MODEL_REGISTRATION_COORD:
		glMultMatrixd(state->modelModeTextureTransform);
		if (surface_z_scale != 0) {
			glScaled(1.0, 1.0, 1.0/surface_z_scale);
		}
		break;
    default:
        fprintf(stderr, "Error, unknown texture coordinate mode\n");
        break;
    }
	glPopAttrib();
	report_gl_errors();
}

/**
 * Access: Protected, Virtual
 * 
 */
void nmg_SurfaceRegion::
setTexture(nmg_State * state, nmb_Dataset *data)
{
	switch(d_currentState.textureDisplayed) {

	case nmg_Graphics::COLORMAP:
    case nmg_Graphics::RULERGRID:
    case nmg_Graphics::VIDEO:
    case nmg_Graphics::REMOTE_DATA:
    case nmg_Graphics::VISUALIZATION:
		state->currentProjectiveTexture->enable();
        break;
	case nmg_Graphics::CONTOUR:
		glEnable(GL_TEXTURE_1D);
		glDisable(GL_TEXTURE_2D);
#ifndef _WIN32
		glDisable(GL_TEXTURE_3D_EXT);
#endif
		if (state->contourTextureID) {
			glBindTexture(GL_TEXTURE_1D, state->contourTextureID);
		} else {
			buildContourTexture(state);
		}
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, 
                  state->contourTextureBlendFunc);
        break;
    case nmg_Graphics::ALPHA:
#ifndef _WIN32
		glDisable(GL_TEXTURE_1D);
		glDisable(GL_TEXTURE_2D);
		glEnable(GL_TEXTURE_3D_EXT);
		if (state->alphaTextureID) {
			glBindTexture(GL_TEXTURE_3D, state->alphaTextureID);
		} else {
			buildAlphaTexture(state);
		}
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, 
                  state->alphaTextureBlendFunc);
#endif
	default:
		break;
	}
	
	if (d_currentState.textureMode == GL_TEXTURE_2D) {
		setTextureTransform(state, data);
	}
	
	return;
}

/**
 * Unset or disable whatever needs to be after 
 *              rendering is complete
 * Access: Protected
 */
void nmg_SurfaceRegion::
cleanUp(nmg_State * state)
{
    if (d_currentState.textureMode == GL_TEXTURE_1D) {
        glDisable(GL_TEXTURE_1D);
    }
#ifndef _WIN32
	else if (d_currentState.textureMode == GL_TEXTURE_3D_EXT) {
		glDisable(GL_TEXTURE_3D_EXT);
	}
#endif
    
#ifdef PROJECTIVE_TEXTURE
	   
	if (d_currentState.textureMode == GL_TEXTURE_2D){
		state->currentProjectiveTexture->disable();
	}    
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
    forceRebuildCondition();
}

/**
 * Create a masking plane, using a range of
 *              height values
 * Access: Public
 */
int nmg_SurfaceRegion::
deriveMaskPlane(float min_height, float max_height)
{   
    return (d_regionalMask->deriveMask(min_height, max_height));
}

/**
 * Create a masking plane, using an oriented box
 * Access: Public
 */
int nmg_SurfaceRegion::
deriveMaskPlane(float center_x, float center_y, float width,float height, 
                float angle)
{
    return(d_regionalMask->deriveMask(center_x, center_y, width, height, angle));
}

/**
 * Create a masking plane, using invalid data range
 * Access: Public
 */
int nmg_SurfaceRegion::
deriveMaskPlane()
{   
    return(d_regionalMask->deriveMask());
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
 * Set the alpha value to use, if we're associated with the default region, or
 * we are forced.
 * Access: Public */
void nmg_SurfaceRegion::
setAlpha(float alpha, vrpn_bool force_set)
{
    if (d_currentAssociations.alpha || force_set) {
        if (d_currentState.alpha != alpha) {
            d_currentState.alpha = alpha;
            forceRebuildCondition();
        }
    }
}

/**
 * Turn on/off wire frame for this region, if we're associated with the
 * default region, or we are forced.
 * Access: Public */
void nmg_SurfaceRegion::
setFilledPolygons(int enable, vrpn_bool force_set)
{
    if (d_currentAssociations.filledPolygonsEnabled || force_set) {
        if (d_currentState.filledPolygonsEnabled != enable) {
            d_currentState.filledPolygonsEnabled = enable;
            forceRebuildCondition();
        }
    }
}

/**
 * Set the tesselation stride to use for this
 * region, if we're associated with the default region, or
 * we are forced.
 * Access: Public
 */
void nmg_SurfaceRegion::
setStride(int stride, vrpn_bool force_set)
{
    if (d_currentAssociations.stride || force_set) {
        if (d_currentState.stride != stride) {
            d_currentState.stride = stride;
            forceRebuildCondition();
        }
    }
}

/**
 * Set which texture is to be displayed in this
 * region, if we're associated with the default region, or
 * we are forced.
 * Access: Public
 */
void nmg_SurfaceRegion::
setTextureDisplayed(int display, vrpn_bool force_set)
{
    if (d_currentAssociations.textureDisplayed || force_set) {
        if (d_currentState.textureDisplayed != display) {
            d_currentState.textureDisplayed = display;
            forceRebuildCondition();
        }
    }
}

/**
 * Set texture mode, if we're associated with the default region, or
 * we are forced.
 * Access: Public
 * 
 */
void nmg_SurfaceRegion::
setTextureMode(int mode, vrpn_bool force_set)
{
    if (d_currentAssociations.textureMode || force_set) {
        if (d_currentState.textureMode != mode) {
            d_currentState.textureMode = mode;
            forceRebuildCondition();
        }
    }
}

/**
 * Set texture transform mode, if we're associated with the default region, or
 * we are forced.  
 * Access: Public
 */
void nmg_SurfaceRegion::
setTextureTransformMode(int mode, vrpn_bool force_set)
{
    if (d_currentAssociations.textureTransformMode || force_set) {
        if (d_currentState.textureTransformMode != mode) {
            d_currentState.textureTransformMode = mode;
            forceRebuildCondition();
        }
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


/**
 * Access: Public
 * Determine what strips we are going to work on.
 * It recomputes only those lists which are not likely to have to be
 * recomputed in the near future as more data comes in from the scope.
 * @return true if there are any strips to work on, i.e. either 
 * d_update or d_color_update is not empty. 
 */
int nmg_SurfaceRegion::
determineInterval(nmb_Dataset *dataset, 
                  int low_row, int high_row, int strips_in_x, 
                  bool do_update_mask) 
{
    bool d_needsFullRebuild = false;
    // Update our mask, and maybe rebuild entire region. 
    if (do_update_mask) {
      if (updateMaskPlane(dataset, low_row, high_row, strips_in_x)) {
        // Yup, the whole region needs to be rebuilt. 
        d_needsFullRebuild = true;
        low_row = 0;
        // for 100x100 grid, strips are numbered 0-98
        if (strips_in_x) {
            high_row = dataset->inputGrid->numY() -2;
        } else {
            high_row = dataset->inputGrid->numX() -2;
        }
      } 
    }

    // for 100x100 grid, strips are numbered 0-98
    int max_strip = (strips_in_x ? 
                     dataset->inputGrid->numY() -1 :
                     dataset->inputGrid->numX() -1 )/ d_currentState.stride -1;
    int low_strip =  low_row / d_currentState.stride;
    int high_strip = min(max_strip, high_row / d_currentState.stride);

  // Let's make it a run-time decision, based on grid resolution.  Using this
  // method, some vertical "tears" will _temporarily_ be left in the rendered
  // image.

  if (dataset->inputGrid->numX() > 99) {
    nmb_Interval mark;
    bool emptyUpdate = false;

    // Assume no strips need to be cleared this time. 
    d_mark_clear.clear();

    // Check for empty or full range of change, don't change direction if so. 
    if (high_row >= low_row && !d_needsFullRebuild) {
        // Avoid changing direction if we have nothing to compare. 
        if(!d_last_nonempty_update.empty()) {
            // Figure out what direction the scan has apparently progressed
            // since the last time.  Heuristic. d_scanDirection will grow
            // big over time, and that's OK, it won't get big enough to wrap. 
            if (low_row >= d_last_nonempty_update.low()) d_scanDirection++;
            if (high_row <= d_last_nonempty_update.high()) d_scanDirection--;
        } 
        d_last_nonempty_update = nmb_Interval (low_row, high_row);
        if (spm_graphics_verbosity >= 9 && d_regionID==0) 
            fprintf(stderr, "Dir %d Data %d, %d ", d_scanDirection, low_row, high_row);
    } else {
        emptyUpdate = true;
    }

    if (d_needsFullRebuild) {
        // If we have a full rebuild, we will only create one strip at a time
        // each time through this function, to maintain interactivity in the
        // program while the surface is being rebuilt.
        if (d_scanDirection > 0) {
            d_update = nmb_Interval(low_strip, low_strip);
            d_todo = nmb_Interval(low_strip+1, high_strip);
        } else {
            d_update = nmb_Interval(high_strip, high_strip);
            d_todo = nmb_Interval(low_strip, high_strip-1);
        }
        // Leave mark empty. 
        d_last_marked.clear();
    } else if (!emptyUpdate) {
        // Recompute as few display lists as necessary. Delay any til the next
        // screen refresh, if more data coming in from the scope will cause
        // them to be recomputed, or maybe no data will come in and we'll have
        // extra time to recompute strips. But render all strips possible from
        // new data, rather than just one strip, to avoid leaving gaps in the
        // surface.
        // See comment for rebuildInterval for explanation of range of strips!
      if ((high_strip-low_strip)*max_strip < 3000) {
        if (d_scanDirection > 0) {
            // Avoid d_last_marked from sucking up the whole surface
            // when the scan restarts. 
            if (high_strip == max_strip) {
                d_update = nmb_Interval (max(0, low_strip - 2), high_strip);
                mark.clear();
            } else {
                d_update = nmb_Interval (max(0, low_strip - 2), 
                                         max(0, high_strip - 2));
                mark = nmb_Interval (d_update.high()+1, 
                                     min(max_strip, high_strip + 1));
            }
        } else {
            // Avoid d_last_marked from sucking up the whole surface
            // when the scan restarts. 
            if (low_strip == 0) {
                d_update = nmb_Interval (low_strip, min(max_strip, high_strip + 1));
                mark.clear();
            } else {
                d_update = nmb_Interval (min(max_strip, low_strip + 1), 
                                         min(max_strip, high_strip + 1));
                mark = nmb_Interval (max(0, low_strip - 2), d_update.low()-1);
            }
        }
      } else {
        // We have a large number of strips to render, >10 strips at 300x300
        // resolution, let's not do them all at once, possibly leaving some
        // gaps in the surface until we get enough idle time to catch up.
        nmb_Interval skipped;
        if (d_scanDirection > 0) {
            d_update = nmb_Interval (max(0, low_strip - 2), 
                                     max(0, low_strip - 2));
            skipped = nmb_Interval (d_update.low()+1, 
                                 min(max_strip, high_strip + 1));
        } else {
            d_update = nmb_Interval (min(max_strip, high_strip + 1), 
                                     min(max_strip, high_strip + 1));
            skipped = nmb_Interval (max(0, low_strip - 2), d_update.low()-1);
        }
        // We need to clear the strips we are skipping,
        // so we don't confuse old data with new data. 
        d_mark_clear = skipped;
        // Add missed strips to "todo" so they don't interfere with
        // d_last_marked after we jump times in a stream file.
        d_todo += skipped;
      }
        //Debug
        if (spm_graphics_verbosity >= 9 && !d_last_marked.empty() && d_regionID==0) 
            fprintf(stderr, "LM %d - %d, ",
                    d_last_marked.low(), d_last_marked.high());
        if (d_last_marked.empty()) {
            d_last_marked = mark;
        } else {
            d_last_marked += mark;
            // If update is handling something from last time, remove it. 
            if (d_last_marked.low() == d_update.low() ||
                d_last_marked.high() == d_update.high()) {
                d_last_marked -= d_update;
            }
        }
    } else {
        // Nothing new to do this frame! 
        // Check and see if there are any strips left from last time, and
        // update one on the trailing edge. d_todo takes precedence over
        // d_last_marked, because new data will likely overwrite
        if (!d_todo.empty()) {
            // Avoid duplicating work
            if (d_todo.includes(d_last_marked)) d_last_marked.clear();

            if (d_scanDirection > 0) {
                d_update = nmb_Interval(d_todo.low(), 
                                        min(max_strip, d_todo.low()+1));
            } else {
                d_update = nmb_Interval(max(0, d_todo.high()-1), d_todo.high());
            }
            d_todo -= d_update;

        } else if (!d_last_marked.empty()) {
            if (d_scanDirection > 0) {
                d_update = nmb_Interval(d_last_marked.low(), 
                                        d_last_marked.low());
            } else {
                d_update = nmb_Interval(d_last_marked.high(), 
                                        d_last_marked.high());
            }
            d_last_marked -= d_update;
            //Debug
            if (spm_graphics_verbosity >= 9 && !d_last_marked.empty() && d_regionID==0) 
                fprintf(stderr, "LM %d - %d, ",
                        d_last_marked.low(), d_last_marked.high());
        } else {
            // Nothing to do at all this frame. 
            d_update.clear();
        }
    }
    //Debug
    if (spm_graphics_verbosity >= 9 && !d_update.empty() && d_regionID==0)
        fprintf(stderr, "update %d - %d, todo %d - %d, "
		"mark %d - %d.\n",
		d_update.low(), d_update.high(), d_todo.low(),
		d_todo.high(), mark.low(), mark.high());

    // Look for parts of the surface to recolor. 
    int color_update_size = 5; // Must be less than (min res)/(max stride).
    if (d_currentState.justColor) {
        // Recolor from the top. 
        if (d_scanDirection > 0) {
            d_color_update = nmb_Interval(0,color_update_size);
            d_color_todo = nmb_Interval(color_update_size+1, max_strip);
        } else {
            d_color_update = nmb_Interval(max_strip-color_update_size, 
                                          max_strip);
            d_color_todo = nmb_Interval(0, max_strip -(color_update_size+1));
        }
        d_currentState.justColor = vrpn_FALSE;
    } else if (!d_color_todo.empty()) {
        // We haven't finished recoloring from last time. 
        if (d_scanDirection > 0) {
            d_color_update = nmb_Interval(d_color_todo.low(),
                      min(max_strip, d_color_todo.low()+color_update_size));
        } else {
            d_color_update = nmb_Interval(
                max(0, d_color_todo.high()-color_update_size),
                d_color_todo.high());
        }
        d_color_todo -= d_color_update;
    } else {
        d_color_update.clear();
    }
    if (spm_graphics_verbosity >= 9 && !d_color_update.empty() && d_regionID==0)
        fprintf(stderr, "Cupdate %d - %d, Ctodo %d - %d \n",
            d_color_update.low(), d_color_update.high(), d_color_todo.low(),
    		d_color_todo.high());
  } else {
    // See comment for rebuildInterval for explanation of range of strips!

    // Recompute every display list that needs to be modified
    // to reflect this change.  If we're in the middle of the
    // scan many of these may have to be recomputed after the
    // next set of data is received.
    if (do_update_mask) {
        d_update = nmb_Interval ( max(0,low_strip-2), 
                              min(max_strip, high_strip+1));
    } else {
        // Fixes a bug: infinite loop caused by call from build_list_set
        // when stride is 5 on a 12x12 surface. 
        d_update.clear();
        d_color_update.clear();
    }
    // Clamp now.

    // leave mark empty!

    // Recolor the whole surface at once. 
    if (d_currentState.justColor) {
        d_color_update = nmb_Interval(0,max_strip);
        d_currentState.justColor = vrpn_FALSE;
    } 
  }

    return (!d_update.empty() || !d_color_update.empty());
}

/**
 *  Replace the display lists that have had points changed.  This includes
 * those that are in the region of changed points and also those that are past
 * the edge of this region, as these points will have had their normals
 * adjusted (in the picture below, '+' represents a point that is on the
 * stride, '.' indicates a point off stride, and the lists that need to be
 * redrawn because of a point change is shown).  The normal of the two
 * on-stride points around the changed point is changed, requiring the redraw
 * of all strips that include these points. However, determineInterval may
 * tell us to delay the building of some of these strips until the next
 * iteration.
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
 * Access: Public */
int nmg_SurfaceRegion::
rebuildInterval(nmb_Dataset *dataset, nmg_State * state, 
                int low_row, int high_row, int strips_in_x)
{
//      timeval start, end;
//      gettimeofday(&start, NULL);
    // First determine if we need rebuild anything. 
    if (!determineInterval(dataset, low_row, high_row, strips_in_x) ) {
        // Nothing to do. 
        return 0;
    }

    nmb_PlaneSelection planes;
    planes.lookup(dataset);
    
    SaveBuildState(state);

    state->stride = d_currentState.stride;
    state->surface_alpha = d_currentState.alpha;

    if (d_regionalMask->nullDataType()) {
        // For a null data region, only render one polygon. 
        if (build_nulldata_polygon(planes, d_regionalMask, state,
                               strips_in_x, d_vertexPtr)) return -1;
    } else {
        if (build_list_set(dataset, planes, 
                           d_regionalMask, state, 
                           strips_in_x, d_vertexPtr)) return -1;
    }
    VERBOSECHECK(6);
    
    RestoreBuildState(state);
//      gettimeofday(&end, NULL);
//      end = vrpn_TimevalDiff(end, start);
//      printf("Range %d, time %ld.%ld\n", 
//             d_update.high() - d_update.low(), end.tv_sec, end.tv_usec);
    return 0;
}

/**
 * Renders the region. Called once each frame. 
 * Access: Public
 */
void nmg_SurfaceRegion::
renderRegion(nmg_State * state, nmb_Dataset *dataset)
{
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
    VERBOSECHECK(6);
 
    cleanUp(state);
    RestoreRenderState(state);
}

/**
* Called by rebuildInterval, calls the other build_list_set.
* This routine creates display lists for the grid.
* It relies on an external routine to determine the set of lists to
* make, the direction (whether x or y is faster), and other important
* variables.
* @return -1 on error, 0 on success. 
*/
int nmg_SurfaceRegion::build_list_set (
    nmb_Dataset *dataset, 
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
    if (d_VertexArrayDim < (unsigned)d_num_lists) {
        // I got this problem when the height plane changed resolutions
        // without calling graphics->causeGridRebuild. I fixed it there,
        // but this will catch anyplace else...
        fprintf(stderr, "Internal: Insufficient vertex array size");
        return -1;
    }
    
    // This clamping should have been done correctly in determineInterval
//      nmb_Interval subset (MAX(0, d_update.low()),
//          MIN(d_num_lists - 1, d_update.high()));
    
    GLdouble * surfaceColor = state->surfaceColor;
    
    int i;
    
    if (d_update.empty() && d_color_update.empty() ) return 0;
    
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
        fprintf(stderr, "  updating %d - %d", d_update.low(), d_update.high());
    } 

    bool all_strips_masked = true;
    int j=0;
    while (all_strips_masked ) {
      // Clear any strips that may need it. Should be disjoint
      // with d_update
      if (!d_mark_clear.empty()) {
          glDeleteLists(d_list_base + d_mark_clear.low(), 
                        d_mark_clear.high() - d_mark_clear.low() + 1);
          //I believe there is no need to call glNewList, because calling
          // glCallList or glDeleteLists on a list that has been deleted is
          // a no-op. 
      }
      for (i = d_update.low(); i <= d_update.high(); i++) {
        
        if (spm_graphics_verbosity >= 10) {
            fprintf(stderr, "    newing list %d for strip %d.\n", d_list_base + i, i);
        }
        
        // Delete lists we are going to create. Frees memory for new lists. 
        glDeleteLists(d_list_base + i, 1);
        // Create or replace existing list with new strip. 
        glNewList(d_list_base + i, GL_COMPILE);

        
        VERBOSECHECK(10);
        if (mask->stripMasked(i, state->stride, strips_in_x)<=0) {
            // We found something to draw, mark it. 
            all_strips_masked = false;
            if ((*stripfn)(state, planes, mask, surfaceColor, 
                           i*state->stride, surface[i])) {
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
        }
        if (spm_graphics_verbosity >= 10) {
            fprintf(stderr, "    updated %d.\n", i);
        }
        VERBOSECHECK(10);
        
        glEndList();
      }

      // Recolor those strips we need to. 
      for (i = d_color_update.low(); i <= d_color_update.high(); i++) {
        // Don't redo a strip which has just been done above. 
        if (d_update.includes(i)) continue;

        if (spm_graphics_verbosity >= 10) {
            fprintf(stderr, "    newing list %d for strip %d.\n", d_list_base + i, i);
        }
        
        // Delete lists we are going to create. Frees memory for new lists. 
        glDeleteLists(d_list_base + i, 1);
        glNewList(d_list_base + i, GL_COMPILE);

        VERBOSECHECK(10);

        state->just_color = 1;
        if (mask->stripMasked(i, state->stride, strips_in_x)<=0) {
            // We found something to draw, mark it. 
            all_strips_masked = false;
            if ((*stripfn)(state, planes, mask, surfaceColor, 
                           i*state->stride, surface[i])) {
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
        }
        state->just_color = 0;        

        if (spm_graphics_verbosity >= 10) {
            fprintf(stderr, "    updated %d.\n", i);
        }
        VERBOSECHECK(10);
        
        glEndList();
      }
      //Check to see whether we drew anything at all
      // If not, we want to skip ahead to where we can draw something.
      if (all_strips_masked) {
          //We didn't. So let's find something to draw, using
          // an empty interval. Pass in a "false" to indicate
          // we don't need to rebuild the region's masks, since we did
          // that above. 
          if (!determineInterval(dataset, 100, 0, strips_in_x, false) ) {
              // Really, there's nothing to do. 
              return 0;
          }
          // Hey, we found something, now saved in d_update and 
          // d_color_update, handled on next loop iteration
      }
    } // while 

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
    // Polygon just below valid values.  
    Vertex[2]= planes.height->minNonZeroValue();

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


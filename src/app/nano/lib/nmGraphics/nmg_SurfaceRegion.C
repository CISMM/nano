#include "nmg_SurfaceRegion.h"

#include <nmb_Dataset.h>
#include <nmb_PlaneSelection.h>
#include <PPM.h>
#include <BCGrid.h>
#include <BCPlane.h>

#include "graphics_globals.h"
#include "globjects.h"
#include "graphics.h"
#include "spm_gl.h"  
#include "openGL.h"  // for check_extension(), display_lists_in_x
#include "nmg_Graphics.h"
#include "nmg_SurfaceMask.h"

// M_PI not defined for VC++, for some reason. 
#ifndef M_PI
#define M_PI		3.14159265358979323846
#endif

#ifdef FLOW
#define VERBOSECHECK(level)	if (spm_graphics_verbosity >= level) ;
#else
#define VERBOSECHECK(level)	if (spm_graphics_verbosity >= level) report_gl_errors();
#endif

////////////////////////////////////////////////////////////
//    Function: nmg_SurfaceRegion::Constructor
//      Access: Public
// Description: 
////////////////////////////////////////////////////////////
nmg_SurfaceRegion::
nmg_SurfaceRegion()
{
    d_needsFullRebuild = VRPN_FALSE;
    d_regionalMask = new nmg_SurfaceMask;
    d_vertexPtr = (Vertex_Struct **)NULL;

    d_currentState.stride = 1;
    d_currentState.justColor = VRPN_FALSE;
    d_currentState.alpha = 1.0f;
    d_currentState.filledPolygonsEnabled = 1;
    d_currentState.textureDisplayed = nmg_Graphics::NO_TEXTURES;
    d_currentState.textureTransformMode = nmg_Graphics::RULERGRID_COORD;;
    d_currentState.textureMode = GL_FALSE;

    d_currentLocks.stride = false;
    d_currentLocks.justColor = false;
    d_currentLocks.alpha = false;
    d_currentLocks.filledPolygonsEnabled = false;
    d_currentLocks.textureDisplayed = false;
    d_currentLocks.textureTransformMode = false;
    d_currentLocks.textureMode = false;
}

////////////////////////////////////////////////////////////
//    Function: nmg_SurfaceRegion::Destructor
//      Access: Public
// Description: 
////////////////////////////////////////////////////////////
nmg_SurfaceRegion::
~nmg_SurfaceRegion()
{
    for(unsigned int i=0;i < d_VertexArrayDim; i++) {
        delete [] d_vertexPtr[i];
    }
    free(d_vertexPtr);

    if (d_regionalMask) {
        delete d_regionalMask;
    }
}

////////////////////////////////////////////////////////////
//    Function: nmg_SurfaceRegion::init
//      Access: Public
// Description: Allocates memory for each "vertex" array that
//              the class needs
////////////////////////////////////////////////////////////
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
        return 1;
    }
    else {
        d_VertexArrayDim = dim;
    }

    d_regionalMask->init(width, height);
    
    if (d_vertexPtr) {
        free(d_vertexPtr);
    }
    
    d_vertexPtr = (Vertex_Struct **)malloc(d_VertexArrayDim * sizeof(Vertex_Struct **));
    //d_vertexPtr = (Vertex_Struct **)new Vertex_Struct[d_VertexArrayDim];
    
    if (d_vertexPtr == NULL) {
        return 0;
    }
    
    for(unsigned int i=0;i < d_VertexArrayDim; i++) {
        d_vertexPtr[i] = new Vertex_Struct[d_VertexArrayDim * 2];
        
        if(d_vertexPtr[i] == NULL )
            return 0;
    }
    
    return 1;
}

////////////////////////////////////////////////////////////
//    Function: nmg_SurfaceRegion::copy
//      Access: Public
// Description: 
////////////////////////////////////////////////////////////
void nmg_SurfaceRegion::
copy(nmg_SurfaceRegion *other)
{
    d_currentState = other->d_currentState;
}

////////////////////////////////////////////////////////////
//    Function: nmg_SurfaceRegion::forceRebuildCondition
//      Access: Public
// Description: 
////////////////////////////////////////////////////////////
void nmg_SurfaceRegion::
forceRebuildCondition()
{
    d_needsFullRebuild = VRPN_TRUE;
}

////////////////////////////////////////////////////////////
//    Function: nmg_SurfaceRegion::setUpdateAndMark
//      Access: Protected, Virtual
// Description: 
////////////////////////////////////////////////////////////
void nmg_SurfaceRegion::
setUpdateAndTodo(int low_row, int high_row) 
{
    int low_strip = MAX(0, low_row / d_currentState.stride);
    int high_strip = MIN(d_num_lists, high_row / d_currentState.stride);
    nmb_Interval mark;
    
    // Figure out what direction the scan has apparently progressed
    // since the last time.  Heuristic, error-prone, but safe.  XXX
    
    int direction = 0;
    if (low_strip >= last_marked.low()) direction++;
    if (high_strip <= last_marked.high()) direction--;
    
    if (spm_graphics_verbosity >= 6)
        fprintf(stderr, "  Drawing in direction %d (from %d to %d).\n",
		direction, low_strip, high_strip);
    VERBOSECHECK(6)
        
#ifdef EXPENSIVE_DISPLAY_LISTS
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
#else
    // Recompute every display list that needs to be modified
    // to reflect this change.  If we're in the middle of the
    // scan many of these may have to be recomputed after the
    // next set of data is received.
    update = nmb_Interval (low_strip - 2, high_strip + 2);
    // leave mark empty!
#endif
    
    if (spm_graphics_verbosity >= 6)
        fprintf(stderr, "   Update set is %d - %d, last_marked is %d - %d, "
		"mark is %d - %d.\n",
		update.low(), update.high(), last_marked.low(),
		last_marked.high(), mark.low(), mark.high());
    VERBOSECHECK(6)
        
        // Draw this time what we have to update due to the most recent
        // changes, plus what we delayed updating last time expecting them
        // to overlap with the most recent changes, minus what we're delaying
        // another frame expecting it to overlap with the next set of changes.
        // NOTE:  this is not a correct implementation of interval subtraction.
        
        todo = last_marked - mark;
    
    // If update and todo are contiguous, combine them and do them
    // as a single interval;  otherwise, call Ablist_set() twice
    
    if (spm_graphics_verbosity >= 15)
        fprintf(stderr, "  ");
    
    last_marked = mark;
}

////////////////////////////////////////////////////////////
//    Function: nmg_SurfaceRegion::setTexture
//      Access: Protected, Virtual
// Description: 
////////////////////////////////////////////////////////////
void nmg_SurfaceRegion::
setTexture()
{
    switch (d_currentState.textureDisplayed) {
    case nmg_Graphics::NO_TEXTURES:
        // nothing to do here
        break;
    case nmg_Graphics::CONTOUR:
        glBindTexture(GL_TEXTURE_1D, tex_ids[CONTOUR_1D_TEX_ID]);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, 
                  g_tex_blend_func[CONTOUR_1D_TEX_ID]);
        break;
    case nmg_Graphics::ALPHA:
#if !(defined(__CYGWIN__) || defined(hpux) || defined(_WIN32))
        glBindTexture(GL_TEXTURE_3D, tex_ids[ALPHA_3D_TEX_ID]);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, 
                  g_tex_blend_func[ALPHA_3D_TEX_ID]);
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
        glBindTexture(GL_TEXTURE_2D, tex_ids[RULERGRID_TEX_ID]);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, 
                  g_tex_blend_func[RULERGRID_TEX_ID]);
        break;
    case nmg_Graphics::COLORMAP:
        glBindTexture(GL_TEXTURE_2D, tex_ids[COLORMAP_TEX_ID]);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, 
                  g_tex_blend_func[COLORMAP_TEX_ID]);
        glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR,
                   g_tex_env_color[COLORMAP_TEX_ID]);
        break;
    case nmg_Graphics::SEM_DATA:
        glBindTexture(GL_TEXTURE_2D, tex_ids[SEM_DATA_TEX_ID]);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, 
                  g_tex_blend_func[SEM_DATA_TEX_ID]);
        glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR,
                   g_tex_env_color[SEM_DATA_TEX_ID]);
        break;
    case nmg_Graphics::REMOTE_DATA:
        glBindTexture(GL_TEXTURE_2D, tex_ids[REMOTE_DATA_TEX_ID]);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, 
                  g_tex_blend_func[REMOTE_DATA_TEX_ID]);
        break;
    case nmg_Graphics::VISUALIZATION:
        glBindTexture(GL_TEXTURE_2D, tex_ids[VISUALIZATION_TEX_ID]);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, 
                  g_tex_blend_func[VISUALIZATION_TEX_ID]);
        break;
    default:
        fprintf(stderr, "Error, unknown texture set for display\n");
        break;
    }
    
#if !(defined(__CYGWIN__) || defined(_WIN32))
	//#ifndef __CYGWIN__
    if ((g_texture_mode == GL_TEXTURE_1D) ||
        (g_texture_mode == GL_TEXTURE_3D_EXT)) {
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
            glScalef(1.0/g_rulergrid_scale,1.0/g_rulergrid_scale,1.0); // 1.0/SCALE
            theta = asin(g_rulergrid_sin);
            if (g_rulergrid_cos < 0)
                theta = M_PI - theta;
            glRotated(theta*180.0/M_PI, 0.0, 0.0, 1.0);              // -ROTATION
            glTranslatef(-g_rulergrid_xoffset, -g_rulergrid_yoffset, 0.0);// -TRANS.
            break;
        case nmg_Graphics::VIZTEX_COORD:
            glScalef(1.0/g_viztex_scale,1.0/g_viztex_scale,1.0); // 1.0/SCALE
            break;
        case nmg_Graphics::MANUAL_REALIGN_COORD:
            compute_texture_matrix(g_translate_tex_x, g_translate_tex_y,
                                   g_tex_theta_cumulative, g_scale_tex_x,
                                   g_scale_tex_y, g_shear_tex_x, g_shear_tex_y,
                                   g_tex_coord_center_x, g_tex_coord_center_y,
                                   texture_matrix);
            glLoadMatrixd(texture_matrix);
            break;
        case nmg_Graphics::PER_QUAD_COORD:
            break;
        case nmg_Graphics::REGISTRATION_COORD:
            glLoadIdentity();
            // scale by actual texture image given divided by texture image used
            // (i.e. the one actually in texture memory is a power of 2 but the
            // one we were given had some smaller size)
            switch (g_texture_displayed) {
            case nmg_Graphics::NO_TEXTURES:
            case nmg_Graphics::BUMPMAP:
            case nmg_Graphics::HATCHMAP:
            case nmg_Graphics::PATTERNMAP:
				// nothing to do here
                break;
            case nmg_Graphics::RULERGRID:
                x_scale_factor = (double)g_tex_image_width[RULERGRID_TEX_ID]/
                    (double)g_tex_installed_width[RULERGRID_TEX_ID];
                y_scale_factor = (double)g_tex_image_height[RULERGRID_TEX_ID]/
                    (double)g_tex_installed_height[RULERGRID_TEX_ID];
                x_translate = (double)g_tex_image_offsetx[RULERGRID_TEX_ID]/
                    (double)g_tex_installed_width[RULERGRID_TEX_ID];
                y_translate = (double)g_tex_image_offsety[RULERGRID_TEX_ID]/
                    (double)g_tex_installed_height[RULERGRID_TEX_ID];
                break;
            case nmg_Graphics::COLORMAP:
                x_scale_factor = (double)g_tex_image_width[COLORMAP_TEX_ID]/
                    (double)g_tex_installed_width[COLORMAP_TEX_ID];
                y_scale_factor = (double)g_tex_image_height[COLORMAP_TEX_ID]/
                    (double)g_tex_installed_height[COLORMAP_TEX_ID];
                x_translate = (double)g_tex_image_offsetx[COLORMAP_TEX_ID]/
                    (double)g_tex_installed_width[COLORMAP_TEX_ID];
                y_translate = (double)g_tex_image_offsety[COLORMAP_TEX_ID]/
                    (double)g_tex_installed_height[COLORMAP_TEX_ID];
                break;
            case nmg_Graphics::SEM_DATA:
                x_scale_factor = (double)g_tex_image_width[SEM_DATA_TEX_ID]/
                    (double)g_tex_installed_width[SEM_DATA_TEX_ID];
                y_scale_factor = (double)g_tex_image_height[SEM_DATA_TEX_ID]/
                    (double)g_tex_installed_height[SEM_DATA_TEX_ID];
                x_translate = (double)g_tex_image_offsetx[SEM_DATA_TEX_ID]/
                    (double)g_tex_installed_width[SEM_DATA_TEX_ID];
                y_translate = (double)g_tex_image_offsety[SEM_DATA_TEX_ID]/
                    (double)g_tex_installed_height[SEM_DATA_TEX_ID];
                break;
            case nmg_Graphics::REMOTE_DATA:
                x_scale_factor = (double)g_tex_image_width[REMOTE_DATA_TEX_ID]/
                    (double)g_tex_installed_width[REMOTE_DATA_TEX_ID];
                y_scale_factor = (double)g_tex_image_height[REMOTE_DATA_TEX_ID]/
                    (double)g_tex_installed_height[REMOTE_DATA_TEX_ID];
                x_translate = (double)g_tex_image_offsetx[REMOTE_DATA_TEX_ID]/
                    (double)g_tex_installed_width[REMOTE_DATA_TEX_ID];
                y_translate = (double)g_tex_image_offsety[REMOTE_DATA_TEX_ID]/
                    (double)g_tex_installed_height[REMOTE_DATA_TEX_ID];
                break;
            case nmg_Graphics::VISUALIZATION:
                x_scale_factor = (double)g_tex_image_width[VISUALIZATION_TEX_ID]/
                    (double)g_tex_installed_width[VISUALIZATION_TEX_ID];
                y_scale_factor = (double)g_tex_image_height[VISUALIZATION_TEX_ID]/
                    (double)g_tex_installed_height[VISUALIZATION_TEX_ID];
                x_translate = (double)g_tex_image_offsetx[VISUALIZATION_TEX_ID]/
                    (double)g_tex_installed_width[VISUALIZATION_TEX_ID];
                y_translate = (double)g_tex_image_offsety[VISUALIZATION_TEX_ID]/
                    (double)g_tex_installed_height[VISUALIZATION_TEX_ID];
                break;
            default:
                fprintf(stderr, "Error, unknown texture set for display\n");
                break;
            }
            glTranslated(x_translate, y_translate, 0.0);
            glScaled(x_scale_factor, y_scale_factor, 1.0);
            glMultMatrixd(g_texture_transform);
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

////////////////////////////////////////////////////////////
//    Function: nmg_SurfaceRegion::cleanUp
//      Access: Protected
// Description: Unset or disable whatever needs to be after 
//              rendering is complete
////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////
//    Function: nmg_SurfaceRegion::setRegionControl
//      Access: Public
// Description: Set the plane that controls the functions that
//              automatically derive the masking plane
////////////////////////////////////////////////////////////
void nmg_SurfaceRegion::
setRegionControl(BCPlane *control)
{
    d_regionalMask->setControlPlane(control);
    d_needsFullRebuild = VRPN_TRUE;
}

////////////////////////////////////////////////////////////
//    Function: nmg_SurfaceRegion::setMaskPlane
//      Access: Public
// Description: Manually set the mask plane
////////////////////////////////////////////////////////////
void nmg_SurfaceRegion::
setMaskPlane(nmg_SurfaceMask *mask)
{
    if (d_regionalMask) {
        delete d_regionalMask;
    }
    d_regionalMask = mask;
    d_needsFullRebuild = VRPN_TRUE;
}

////////////////////////////////////////////////////////////
//    Function: nmg_SurfaceRegion::deriveMaskPlane
//      Access: Public
// Description: Create a masking plane, using a range of
//              height values
////////////////////////////////////////////////////////////
void nmg_SurfaceRegion::
deriveMaskPlane(float min_height, float max_height)
{
    d_regionalMask->deriveMask(min_height, max_height);
    d_needsFullRebuild = VRPN_TRUE;
}

////////////////////////////////////////////////////////////
//    Function: nmg_SurfaceRegion::setAlpha
//      Access: Public
// Description: Set the alpha value to use
////////////////////////////////////////////////////////////
void nmg_SurfaceRegion::
setAlpha(float alpha, vrpn_bool respect_lock)
{
    if (!d_currentLocks.alpha || !respect_lock) {
        d_currentState.alpha = alpha;
        d_needsFullRebuild = VRPN_TRUE;
    }
}

////////////////////////////////////////////////////////////
//    Function: nmg_SurfaceRegion::enableFilledPolygons
//      Access: Public
// Description: Determine whether wire frame is enabled for
//				this region
////////////////////////////////////////////////////////////
void nmg_SurfaceRegion::
enableFilledPolygons(int enable, vrpn_bool respect_lock)
{
    if (!d_currentLocks.filledPolygonsEnabled || !respect_lock) {
        d_currentState.filledPolygonsEnabled = enable;
    }
}

////////////////////////////////////////////////////////////
//    Function: nmg_SurfaceRegion::setStride
//      Access: Public
// Description: Set the tesselation stride to use for this
//				region
////////////////////////////////////////////////////////////
void nmg_SurfaceRegion::
setStride(int stride, vrpn_bool respect_lock)
{
    if (!d_currentLocks.stride || !respect_lock) {
        d_currentState.stride = stride;
        d_needsFullRebuild = VRPN_TRUE;
    }
}

////////////////////////////////////////////////////////////
//    Function: nmg_SurfaceRegion::setTextureDisplayed
//      Access: Public
// Description: Set which texture is to be displayed in this
//              region
////////////////////////////////////////////////////////////
void nmg_SurfaceRegion::
setTextureDisplayed(int display, vrpn_bool respect_lock)
{
    if (!d_currentLocks.textureDisplayed || !respect_lock) {
        d_currentState.textureDisplayed = display;
        d_needsFullRebuild = VRPN_TRUE;
    }
}

////////////////////////////////////////////////////////////
//    Function: nmg_SurfaceRegion::setTextureMode
//      Access: Public
// Description: 
////////////////////////////////////////////////////////////
void nmg_SurfaceRegion::
setTextureMode(int mode, vrpn_bool respect_lock)
{
    if (!d_currentLocks.textureMode || !respect_lock) {
        d_currentState.textureMode = mode;
        d_needsFullRebuild = VRPN_TRUE;
    }
}

////////////////////////////////////////////////////////////
//    Function: nmg_SurfaceRegion::setTextureTransformMode
//      Access: Public
// Description: 
////////////////////////////////////////////////////////////
void nmg_SurfaceRegion::
setTextureTransformMode(int mode, vrpn_bool respect_lock)
{
    if (!d_currentLocks.textureTransformMode || !respect_lock) {
        d_currentState.textureTransformMode = mode;
        d_needsFullRebuild = VRPN_TRUE;
    }
}


////////////////////////////////////////////////////////////
//    Function: nmg_SurfaceRegion::getRegionData
//      Access: Public
// Description: The registration code needs explicit access 
//              to the data
////////////////////////////////////////////////////////////
Vertex_Struct ** nmg_SurfaceRegion::
getRegionData()
{
    return d_vertexPtr;
}

////////////////////////////////////////////////////////////
//    Function: nmg_SurfaceRegion::SaveBuildState
//      Access: Protected
// Description: Saves the state of global variables
//        Note: This function wil be unnecessary once we get
//              rid of all the Global variables!
////////////////////////////////////////////////////////////
void nmg_SurfaceRegion::
SaveBuildState()
{
    d_savedState.justColor = g_just_color;  //This one gets
                                     //automatically changed
    d_savedState.stride = g_stride;
    d_savedState.alpha = g_surface_alpha;
}

////////////////////////////////////////////////////////////
//    Function: nmg_SurfaceRegion::RestoreBuildState
//      Access: Protected
// Description: 
//        Note: This function wil be unnecessary once we get
//              rid of all the Global variables!
////////////////////////////////////////////////////////////
void nmg_SurfaceRegion::
RestoreBuildState()
{
    g_just_color = d_savedState.justColor;
    g_stride = d_savedState.stride;
    g_surface_alpha = d_savedState.alpha;
}

////////////////////////////////////////////////////////////
//    Function: nmg_SurfaceRegion::SaveRenderState
//      Access: Protected
// Description: Saves the state of global variables
//        Note: This function wil be unnecessary once we get
//              rid of all the Global variables!
////////////////////////////////////////////////////////////
void nmg_SurfaceRegion::
SaveRenderState()
{
    d_savedState.filledPolygonsEnabled = g_config_filled_polygons;
    d_savedState.textureDisplayed = g_texture_displayed;
    d_savedState.textureMode = g_texture_mode;
    d_savedState.textureTransformMode = g_texture_transform_mode;
}

////////////////////////////////////////////////////////////
//    Function: nmg_SurfaceRegion::RestoreRenderState
//      Access: Protected
// Description: 
//        Note: This function wil be unnecessary once we get
//              rid of all the Global variables!
////////////////////////////////////////////////////////////
void nmg_SurfaceRegion::
RestoreRenderState()
{
    g_config_filled_polygons = d_savedState.filledPolygonsEnabled;
    g_texture_displayed = d_savedState.textureDisplayed;
    g_texture_mode = d_savedState.textureMode;
    g_texture_transform_mode = d_savedState.textureTransformMode;
}

////////////////////////////////////////////////////////////
//    Function: nmg_SurfaceRegion::rebuildGrid
//      Access: Public
// Description: Rebuilds the display lists that correspond
//              to the region
////////////////////////////////////////////////////////////
int nmg_SurfaceRegion::
rebuildRegion(nmb_Dataset *dataset, vrpn_bool force)
{  
    if (!d_needsFullRebuild && !force) {
        return 1;
    }

    nmb_PlaneSelection planes;  // Rebuilds the texture coordinate array:
    planes.lookup(dataset);
    
    VERBOSE(6, "nmg_SurfaceRegion::rebuildRegion()");

    SaveBuildState();

    g_stride = d_currentState.stride;
    g_surface_alpha = d_currentState.alpha;
    
    //Make sure we have a valid mask before we rebuild the display lists
    d_regionalMask->rederive();
    if (build_grid_display_lists(planes, d_regionalMask, display_lists_in_x, &d_list_base, 
                                 &d_num_lists, d_num_lists, g_minColor,
                                 g_maxColor, d_vertexPtr)) {
        return 0;
    }

    RestoreBuildState();

    d_needsFullRebuild = VRPN_FALSE;
    
    return 1;
}

////////////////////////////////////////////////////////////
//    Function: nmg_SurfaceRegion::rebuildInterval
//      Access: Public
// Description: 
////////////////////////////////////////////////////////////
int nmg_SurfaceRegion::
rebuildInterval(nmb_Dataset *dataset, int low_row, int high_row, int strips_in_x)
{
    setUpdateAndTodo(low_row, high_row);
    
    nmb_PlaneSelection planes;
    planes.lookup(dataset);
    
    SaveBuildState();

    g_stride = d_currentState.stride;
    g_surface_alpha = d_currentState.alpha;

    if (!update.empty() || !todo.empty()) {
        //Make sure we have a valid mask before we rebuild the display lists
        d_regionalMask->rederive();
    }
    if (update.overlaps(todo) || update.adjacent(todo)) {
        if (build_list_set(update + todo, planes, d_regionalMask, d_list_base, 
                           d_num_lists, strips_in_x, d_vertexPtr)) return 0;
    } 
    else {
        if (build_list_set(update, planes, d_regionalMask, d_list_base, 
                           d_num_lists, strips_in_x, d_vertexPtr)) return 0;
        if (build_list_set(todo, planes, d_regionalMask, d_list_base, 
                           d_num_lists, strips_in_x, d_vertexPtr)) return 0;
    }
    
    RestoreBuildState();

    return 1;
}

////////////////////////////////////////////////////////////
//    Function: nmg_SurfaceRegion::renderRegion
//      Access: Public
// Description: Renders the region.
////////////////////////////////////////////////////////////
void nmg_SurfaceRegion::
renderRegion()
{
	static bool set = false;
    int i;
    
    SaveRenderState();
    setTexture();    

    g_config_filled_polygons = d_currentState.filledPolygonsEnabled;
    g_texture_mode = d_currentState.textureMode;
    g_texture_displayed = d_currentState.textureDisplayed;
    g_texture_transform_mode = d_currentState.textureTransformMode;

    spm_set_surface_materials();
    setFilled();

    for (i = 0; i < d_num_lists; i++) {
        glCallList(d_list_base + i);
    }
    
    cleanUp();
    RestoreRenderState();
}

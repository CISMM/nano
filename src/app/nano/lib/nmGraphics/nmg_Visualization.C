#include <nmb_Dataset.h>
#include <nmb_PlaneSelection.h>
#include <PPM.h>
#include <BCGrid.h>
#include <BCPlane.h>
#include "graphics_globals.h"
#include "globjects.h"
#include "graphics.h"
#include "spm_gl.h"  // for init_vertexArray()
#include "openGL.h"  // for check_extension(), display_lists_in_x
#include "nmg_Graphics.h"
#include "nmg_Visualization.h"

// M_PI not defined for VC++, for some reason. 
#ifndef M_PI
  #define M_PI		3.14159265358979323846
#endif

#ifdef FLOW
  #define VERBOSECHECK(level)	if (spm_graphics_verbosity >= level) ;
#else
  #define VERBOSECHECK(level)	if (spm_graphics_verbosity >= level) report_gl_errors();
#endif


nmg_Visualization* create_new_visualization(int choice, nmb_Dataset *dataset)
{
	nmg_Visualization *viz;
	switch(choice) {
	case 0:
		viz = new nmg_Viz_Opaque(dataset);
		break;
	case 1:
		viz = new nmg_Viz_Transparent(dataset);
		break;
	case 2:
		viz = new nmg_Viz_WireFrame(dataset);
		break;
	case 3:
		viz = new nmg_Viz_OpaqueTexture(dataset);
		break;
	default:
		printf("Request to make unknown visualization!\n");
		printf("Defaulting to Opaque visualization.\n");
		viz = new nmg_Viz_Opaque(dataset);
		break;
	}

	return viz;
}

////////////////////////////////////////////////////////////
//    Function: nmg_Visualization::Constructor
//      Access: Public
// Description:
////////////////////////////////////////////////////////////
nmg_Visualization::
nmg_Visualization(nmb_Dataset *dataset)
  : d_maxHeight(0.5), d_minHeight(0.5), 
    d_alpha(0.5), d_vertexPtr(NULL),
	d_texture(NULL), d_tex_width(0), d_tex_height(0),
	d_control(NULL), d_VertexArrayDim(0)
{
    changeDataset(dataset);
}

////////////////////////////////////////////////////////////
//    Function: nmg_Visualization::Destructor
//      Access: Public
// Description:
////////////////////////////////////////////////////////////
nmg_Visualization::
~nmg_Visualization()
{
    delete [] d_list_base;
    delete [] d_num_lists;
    delete [] last_marked;
    delete [] update;
    delete [] todo;
}

////////////////////////////////////////////////////////////
//    Function: nmg_Visualization::getBaseSurface
//      Access: Public
// Description: 
////////////////////////////////////////////////////////////
Vertex_Struct ** nmg_Visualization::
getBaseSurface()
{
    return d_vertexPtr[0];
}

////////////////////////////////////////////////////////////
//    Function: nmg_Visualization::setMinHeight
//      Access: Public
// Description: Set the bottom level height to use the 
//              visualization method at
////////////////////////////////////////////////////////////
void nmg_Visualization::
setMinHeight(float min_height) {
    d_minHeight = min_height;
}

////////////////////////////////////////////////////////////
//    Function: nmg_Visualization::getMinHeight
//      Access: Public
// Description: 
////////////////////////////////////////////////////////////
float nmg_Visualization::
getMinHeight() {
    return d_minHeight;
}

////////////////////////////////////////////////////////////
//    Function: nmg_Visualization::setMaxHeight
//      Access: Public
// Description: Set the top level height to use the 
//              visualization method to
////////////////////////////////////////////////////////////
void nmg_Visualization::
setMaxHeight(float max_height) {
    d_maxHeight = max_height;
}

////////////////////////////////////////////////////////////
//    Function: nmg_Visualization::getMaxHeight
//      Access: Public
// Description: 
////////////////////////////////////////////////////////////
float nmg_Visualization::
getMaxHeight() {
    return d_maxHeight;
}

////////////////////////////////////////////////////////////
//    Function: nmg_Visualization::setControlPlane
//      Access: Public
// Description: Set the BCPlane to use to control where the
//              visualization is used and where the plane
//              is displayed normally.
////////////////////////////////////////////////////////////
void nmg_Visualization::
setControlPlane(BCPlane *control) {
    d_control = control;
}

////////////////////////////////////////////////////////////
//    Function: nmg_Visualization::changeDataset
//      Access: Public
// Description: Change the current dataset to a new one
////////////////////////////////////////////////////////////
void nmg_Visualization::
changeDataset(nmb_Dataset *dataset) {
	d_dataset = dataset;
}

////////////////////////////////////////////////////////////
//    Function: nmg_Visualization::ensureMaskPlane
//      Access: Protected
// Description: 
////////////////////////////////////////////////////////////
void nmg_Visualization::
ensureMaskPlane(nmb_PlaneSelection &planes) {
    if (planes.mask == (BCPlane*)NULL) {
        BCPlane *plane = d_dataset->inputGrid->addNewPlane("Mask-Plane", "", NOT_TIMED);		
        d_dataset->maskPlaneName->Set(plane->name()->Characters());
        strcpy(g_maskPlaneName, plane->name()->Characters());
		planes.mask = plane;
    }
}    
////////////////////////////////////////////////////////////
//    Function: nmg_Visualization::ensureTransparentPlane
//      Access: Protected
// Description: 
////////////////////////////////////////////////////////////
void nmg_Visualization::
ensureTransparentPlane(nmb_PlaneSelection &planes) {
    if (planes.transparent == (BCPlane*)NULL) {
        BCPlane *plane = d_dataset->inputGrid->addNewPlane("Transparent-Plane", "", NOT_TIMED);		
        d_dataset->transparentPlaneName->Set(plane->name()->Characters());
        strcpy(g_transparentPlaneName, plane->name()->Characters());
		planes.transparent = plane;
    }
}

////////////////////////////////////////////////////////////
//    Function: nmg_Visualization::setAlpha
//      Access: Public
// Description: Set the alpha value to use at the mask areas
////////////////////////////////////////////////////////////
void nmg_Visualization::
setAlpha(float alpha) {
    d_alpha = alpha;
}

////////////////////////////////////////////////////////////
//    Function: nmg_Visualization::getAlpha
//      Access: Public
// Description: 
////////////////////////////////////////////////////////////
float nmg_Visualization::
getAlpha() {
    return d_alpha;
}

////////////////////////////////////////////////////////////
//    Function: nmg_Visualization::determineVertexDim
//      Access: Protected, Virtual
// Description: 
////////////////////////////////////////////////////////////
int nmg_Visualization::
determineVertexDim(int x, int y)
{
	int dim;
    
    if(x<=y) {
        dim=y;
    }
    else {
        dim=x;
    }       
    
	if (dim == d_VertexArrayDim) {
		return 0;
	}
	else {
		d_VertexArrayDim = dim;
	}

	return 1;
}

////////////////////////////////////////////////////////////
//    Function: nmg_Visualization::setUpdateAndMark
//      Access: Protected, Virtual
// Description: 
////////////////////////////////////////////////////////////
void nmg_Visualization::
setUpdateAndTodo(int low_row, int high_row, int stride, 
                 int num, nmb_Interval &last_marked,
                 nmb_Interval &update, nmb_Interval &todo) 
{
    int low_strip = MAX(0, low_row / stride);
    int high_strip = MIN(num, high_row / stride);
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
//    Function: nmg_Visualization::setTexture
//      Access: Protected, Virtual
// Description: 
////////////////////////////////////////////////////////////
void nmg_Visualization::
setTexture()
{
	switch (g_texture_displayed) {
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

  if (g_texture_mode == GL_TEXTURE_2D) {
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

    switch (g_texture_transform_mode) {
      case nmg_Graphics::RULERGRID_COORD:
        // use values from the older rulergrid adjustment interface
        glScalef(1.0/g_rulergrid_scale,1.0/g_rulergrid_scale,1.0); // 1.0/SCALE
        theta = asin(g_rulergrid_sin);
        if (g_rulergrid_cos < 0)
          theta = M_PI - theta;
        glRotated(theta*180.0/M_PI, 0.0, 0.0, 1.0);              // -ROTATION
        glTranslatef(-g_rulergrid_xoffset, -g_rulergrid_yoffset, 0.0);// -TRANS.
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
            break;
	    /*
          case nmg_Graphics::GENETIC:
            x_scale_factor = (double)g_tex_image_width[GENETIC_TEX_ID]/
                             (double)g_tex_installed_width[GENETIC_TEX_ID];
            y_scale_factor = (double)g_tex_image_height[GENETIC_TEX_ID]/
                             (double)g_tex_installed_height[GENETIC_TEX_ID];
            break;
	    */
          case nmg_Graphics::COLORMAP:
            x_scale_factor = (double)g_tex_image_width[COLORMAP_TEX_ID]/
                             (double)g_tex_installed_width[COLORMAP_TEX_ID];
            y_scale_factor = (double)g_tex_image_height[COLORMAP_TEX_ID]/
                             (double)g_tex_installed_height[COLORMAP_TEX_ID];
            break;
          case nmg_Graphics::SEM_DATA:
            x_scale_factor = (double)g_tex_image_width[SEM_DATA_TEX_ID]/
                             (double)g_tex_installed_width[SEM_DATA_TEX_ID];
            y_scale_factor = (double)g_tex_image_height[SEM_DATA_TEX_ID]/
                             (double)g_tex_installed_height[SEM_DATA_TEX_ID];
            break;
          case nmg_Graphics::REMOTE_DATA:
            x_scale_factor = (double)g_tex_image_width[REMOTE_DATA_TEX_ID]/
                             (double)g_tex_installed_width[REMOTE_DATA_TEX_ID];
            y_scale_factor = (double)g_tex_image_height[REMOTE_DATA_TEX_ID]/
                             (double)g_tex_installed_height[REMOTE_DATA_TEX_ID];
            break;
		  case nmg_Graphics::VISUALIZATION:
            x_scale_factor = (double)g_tex_image_width[VISUALIZATION_TEX_ID]/
                             (double)g_tex_installed_width[VISUALIZATION_TEX_ID];
            y_scale_factor = (double)g_tex_image_height[VISUALIZATION_TEX_ID]/
                             (double)g_tex_installed_height[VISUALIZATION_TEX_ID];
            break;
          default:
            fprintf(stderr, "Error, unknown texture set for display\n");
            break;
        }
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
  
  if (g_texture_mode == GL_TEXTURE_1D) {
     glEnable(GL_TEXTURE_1D);
  }
}


////////////////////////////////////////////////////////////
//    Function: nmg_Visualization::drawLists
//      Access: Protected
// Description: Draw a set of display lists
////////////////////////////////////////////////////////////
void nmg_Visualization::
drawLists(int base, int num) 
{
	int i;

	setTexture();

    for (i = 0; i < num; i++) {
        glCallList(base + i);
    }

	cleanUp();
}

////////////////////////////////////////////////////////////
//    Function: nmg_Visualization::buildMaskPlane
//      Access: Protected, Virtual
// Description: Uses the current mode settings and height or
//              control plane data to build a plane used to
//              mask between normal display and the particular
//              visualization method
////////////////////////////////////////////////////////////
void nmg_Visualization::
buildMaskPlane(nmb_PlaneSelection &planes) {

	ensureMaskPlane(planes);

	if (d_control == (BCPlane*)NULL) {
		//If there is no control plane, then just fill the
		//mask plane with 1's.
		for(int y = 0; y < planes.mask->numY(); y++) {
			for(int x = 0; x < planes.mask->numX(); x++) {
				planes.mask->setValue(x, y, 1);
			}
		}
		return;
	}

    float maskVal, z;
	for(int y = 0; y < d_control->numY(); y++) {
        for(int x = 0; x < d_control->numX(); x++) {
            z = d_control->value(x,y);
            maskVal = ((z < d_minHeight) || (z > d_maxHeight));
            planes.mask->setValue(x, y, maskVal);
        }
    }
}

////////////////////////////////////////////////////////////
//    Function: nmg_Visualization::buildTransparentPlane
//      Access: Protected, Virtual
// Description: Uses the current mode settings and height or
//              control plane data to build a plane used to
//              control the alpha values at each point
////////////////////////////////////////////////////////////
void nmg_Visualization::
buildTransparentPlane(nmb_PlaneSelection &planes) {

	ensureTransparentPlane(planes);

	if (d_control == (BCPlane*)NULL) {
		//If there is no control plane, then just fill the
		//transparency plane with 1's.
		for(int y = 0; y < planes.transparent->numY(); y++) {
			for(int x = 0; x < planes.transparent->numX(); x++) {
				planes.transparent->setValue(x, y, 1);
			}
		}
		return;
	}
    
    float alphaVal, z;
    for(int y = 0; y < d_control->numY(); y++) {
        for(int x = 0; x < d_control->numX(); x++) {
            z = d_control->value(x,y);
            alphaVal = g_surface_alpha;

			if ((z >= d_minHeight) && (z <= d_maxHeight)) {
				alphaVal = d_alpha;
			}
            
            planes.transparent->setValue(x, y, alphaVal);
        }
    }
}

////////////////////////////////////////////////////////////
//    Function: nmg_Visualization::cleanUp
//      Access: Protected, Virtual
// Description: Unset or disable whatever needs to be after 
//              rendering is complete
////////////////////////////////////////////////////////////
void nmg_Visualization::
cleanUp()
{
  if (g_texture_mode == GL_TEXTURE_1D) {
     glDisable(GL_TEXTURE_1D);
  }


#ifdef PROJECTIVE_TEXTURE

  if (g_texture_mode == GL_TEXTURE_2D){
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
//    Function: nmg_Viz_Opaque::Constructor
//      Access: Public
// Description:
////////////////////////////////////////////////////////////
nmg_Viz_Opaque::
nmg_Viz_Opaque(nmb_Dataset *dataset)
  : nmg_Visualization(dataset)
{
    d_list_base = new unsigned int[1];
    d_num_lists = new int[1];
    last_marked = new nmb_Interval[1];
    update = new nmb_Interval[1];
    todo = new nmb_Interval[1];
    
    d_list_base[0] = 0;
    d_num_lists[0] = 0;
}

////////////////////////////////////////////////////////////
//    Function: nmg_Viz_Opaque::Destructor
//      Access: Public
// Description:
////////////////////////////////////////////////////////////
nmg_Viz_Opaque::
~nmg_Viz_Opaque()
{
	if (d_vertexPtr) {
		for(int i=0;i < d_VertexArrayDim; i++) {
			delete [] d_vertexPtr[0][i];
		}

		delete [] d_vertexPtr[0];
		delete [] d_vertexPtr;
	}
}

////////////////////////////////////////////////////////////
//    Function: nmg_Viz_Opaque::rebuildGrid
//      Access: Public
// Description: Rebuilds the display lists that correspond
//              to the surface
////////////////////////////////////////////////////////////
int nmg_Viz_Opaque::
rebuildGrid()
{  
    nmb_PlaneSelection planes;  // Rebuilds the texture coordinate array:
    planes.lookup(d_dataset);

	//ensureMaskPlane(planes);
	//ensureTransparentPlane(planes);
    
    VERBOSE(6, "nmg_Viz_Opaque::rebuildGrid()");

    if (build_grid_display_lists(planes,display_lists_in_x, &d_list_base[0], 
                                 &d_num_lists[0], d_num_lists[0], g_minColor,
                                 g_maxColor, d_vertexPtr[0])) {
        return 0;
    }

    return 1;
}

////////////////////////////////////////////////////////////
//    Function: nmg_Viz_Opaque::renderSurface
//      Access: Protected, Virtual
// Description: Renders the world.
////////////////////////////////////////////////////////////
void nmg_Viz_Opaque::
renderSurface()
{
	drawLists(d_list_base[0], d_num_lists[0]);
}

////////////////////////////////////////////////////////////
//    Function: nmg_Viz_Opaque::rebuildInterval
//      Access: Public
// Description: 
////////////////////////////////////////////////////////////
int nmg_Viz_Opaque::
rebuildInterval(int low_row, int high_row, int strips_in_x)
{
    setUpdateAndTodo(low_row, high_row, g_stride, d_num_lists[0], 
                     last_marked[0], update[0], todo[0]);
    
    nmb_PlaneSelection planes;
    planes.lookup(d_dataset);
    
    if (update[0].overlaps(todo[0]) || update[0].adjacent(todo[0])) {
        if (build_list_set(update[0] + todo[0], planes, d_list_base[0], 
                           d_num_lists[0], strips_in_x, d_vertexPtr[0])) return 0;
    } 
    else {
        if (build_list_set(update[0], planes, d_list_base[0], 
                           d_num_lists[0], strips_in_x, d_vertexPtr[0])) return 0;
        if (build_list_set(todo[0], planes, d_list_base[0], 
                           d_num_lists[0], strips_in_x, d_vertexPtr[0])) return 0;
    }
    
    return 1;
}


////////////////////////////////////////////////////////////
//    Function: nmg_Viz_Opaque::initVertexArrays
//      Access: Public
// Description: Allocates memory for each "vertex" array that
//              the class needs
////////////////////////////////////////////////////////////
int nmg_Viz_Opaque::
initVertexArrays(int x, int y)
{
    if (!determineVertexDim(x,y)) {
		return 1;
	}

    if (d_vertexPtr) {
        free(d_vertexPtr);
    }
    
    //d_vertexPtr = (Vertex_Struct ***)malloc(sizeof(Vertex_Struct **));
	d_vertexPtr = (Vertex_Struct***)new Vertex_Struct[1];
    
    //d_vertexPtr[0] = (Vertex_Struct **)malloc(
    //    sizeof(Vertex_Struct *) * d_VertexArrayDim);
    d_vertexPtr[0] = (Vertex_Struct **)new Vertex_Struct[d_VertexArrayDim];

    if (d_vertexPtr[0] == NULL) {
        return 0;
    }
    
    for(int i=0;i < d_VertexArrayDim; i++) {
        //d_vertexPtr[0][i]= (Vertex_Struct *)malloc(sizeof(Vertex_Struct)* d_VertexArrayDim * 2);
		d_vertexPtr[0][i] = new Vertex_Struct[d_VertexArrayDim * 2];
        
        if(d_vertexPtr[0][i] == NULL )
            return 0;
    }
    
    return 1;
}

////////////////////////////////////////////////////////////
//    Function: nmg_Viz_Transparent::Constructor
//      Access: Public
// Description:
////////////////////////////////////////////////////////////
nmg_Viz_Transparent::
nmg_Viz_Transparent(nmb_Dataset *dataset)
	: nmg_Visualization(dataset)
{
    d_list_base = new unsigned int[2];
    d_num_lists = new int[2];
    last_marked = new nmb_Interval[2];
    update = new nmb_Interval[2];
    todo = new nmb_Interval[2];
        
    d_list_base[0] = 0; d_list_base[1] = 0;
    d_num_lists[0] = 0; d_num_lists[1] = 0;
}

////////////////////////////////////////////////////////////
//    Function: nmg_Viz_Transparent::Destructor
//      Access: Public
// Description:
////////////////////////////////////////////////////////////
nmg_Viz_Transparent::
~nmg_Viz_Transparent()
{
	if (d_vertexPtr) {
		for(int i = 0; i < 2; i++) {
			for(int j=0;j < d_VertexArrayDim; j++) {
				delete [] d_vertexPtr[i][j];
			}
		}

		delete [] d_vertexPtr[0];
		delete [] d_vertexPtr[1];
		delete [] d_vertexPtr;
	}
}
////////////////////////////////////////////////////////////
//    Function: nmg_Viz_Transparent::rebuildGrid
//      Access: Public
// Description: Rebuilds the display lists that correspond
//              to the surface
////////////////////////////////////////////////////////////
int nmg_Viz_Transparent::
rebuildGrid()
{
    nmb_PlaneSelection planes;
    planes.lookup(d_dataset);

    buildMaskPlane(planes);  
    buildTransparentPlane(planes);
    g_mask = ENABLE_MASK;
        
    VERBOSE(6, "nmg_Viz_Transparent::rebuildGrid()");

    if (build_grid_display_lists(planes, display_lists_in_x, &d_list_base[0],
                                 &d_num_lists[0], d_num_lists[0], g_minColor,
                                 g_maxColor, d_vertexPtr[0])) {
        return 0;
    }
    
    g_mask = INVERT_MASK;
    g_transparent = 1;
    
    if (build_grid_display_lists(planes, display_lists_in_x, &d_list_base[1],
                                 &d_num_lists[1], d_num_lists[1], g_minColor,
                                 g_maxColor, d_vertexPtr[1])) {
        return 0;
    }
    
    g_mask = DISABLE_MASK;
    g_transparent = 0;
    
    return 1;
}

////////////////////////////////////////////////////////////
//    Function: nmg_Viz_Transparent::renderSurface
//      Access: Protected, Virtual
// Description: Renders the world.
////////////////////////////////////////////////////////////
void nmg_Viz_Transparent::
renderSurface()
{
	drawLists(d_list_base[0], d_num_lists[0]);
	drawLists(d_list_base[1], d_num_lists[1]);
}

////////////////////////////////////////////////////////////
//    Function: nmg_Viz_Transparent::rebuildInterval
//      Access: Public
// Description: 
////////////////////////////////////////////////////////////
int nmg_Viz_Transparent::
rebuildInterval(int low_row, int high_row, int strips_in_x)
{
    nmb_PlaneSelection planes;
    planes.lookup(d_dataset);

    //Pass 1
    setUpdateAndTodo(low_row, high_row, g_stride, d_num_lists[0], 
                     last_marked[0], update[0], todo[0]);
    
	buildMaskPlane(planes);  
    buildTransparentPlane(planes);
    g_mask = ENABLE_MASK;

    if (update[0].overlaps(todo[0]) || update[0].adjacent(todo[0])) {
        if (build_list_set(update[0] + todo[0], planes, d_list_base[0], 
                           d_num_lists[0], strips_in_x, d_vertexPtr[0])) return 0;
    } 
    else {
        if (build_list_set(update[0], planes, d_list_base[0], 
                           d_num_lists[0], strips_in_x, d_vertexPtr[0])) return 0;
        if (build_list_set(todo[0], planes, d_list_base[0], 
                           d_num_lists[0], strips_in_x, d_vertexPtr[0])) return 0;
    }
    
    //Pass 2
    setUpdateAndTodo(low_row, high_row, 5, d_num_lists[1], 
                     last_marked[1], update[1], todo[1]);

    g_mask = INVERT_MASK;
    g_transparent = 1;

    if (update[1].overlaps(todo[1]) || update[1].adjacent(todo[1])) {
        if (build_list_set(update[1] + todo[1], planes, d_list_base[1],
                           d_num_lists[1], strips_in_x, d_vertexPtr[1])) return 0;
    } 
    else {
        if (build_list_set(update[1], planes, d_list_base[1],
                           d_num_lists[1], strips_in_x, d_vertexPtr[1])) return 0;
        if (build_list_set(todo[1], planes, d_list_base[1],
                           d_num_lists[1], strips_in_x, d_vertexPtr[1])) return 0;
    }
    
	g_mask = DISABLE_MASK;
    g_transparent = 0;

    return 1;
}

////////////////////////////////////////////////////////////
//    Function: nmg_Viz_Transparent::initVertexArrays
//      Access: Public
// Description: Allocates memory for each "vertex" array that
//              the class needs
////////////////////////////////////////////////////////////
int nmg_Viz_Transparent::
initVertexArrays(int x, int y)
{
    if (!determineVertexDim(x,y)) {
		return 1;
	}
    
    d_vertexPtr = (Vertex_Struct***)new Vertex_Struct[2];
    
    for(int i = 0; i < 2; i++) {
        d_vertexPtr[i] = (Vertex_Struct **)new Vertex_Struct[d_VertexArrayDim];
        
        if (d_vertexPtr[i] == NULL)
            return 0;
        for(int j=0;j < d_VertexArrayDim; j++) {
            d_vertexPtr[i][j] = new Vertex_Struct[d_VertexArrayDim * 2];
            
            if(d_vertexPtr[i][j] == NULL )
                return 0;
        }
    }
    
    return 1;
}

////////////////////////////////////////////////////////////
//    Function: nmg_Viz_WireFrame::Constructor
//      Access: Public
// Description:
////////////////////////////////////////////////////////////
nmg_Viz_WireFrame::
nmg_Viz_WireFrame(nmb_Dataset *dataset)
	: nmg_Visualization(dataset)
{
    d_list_base = new unsigned int[2];
    d_num_lists = new int[2];
    last_marked = new nmb_Interval[2];
    update = new nmb_Interval[2];
    todo = new nmb_Interval[2];
    
    d_list_base[0] = 0; d_list_base[1] = 0;
    d_num_lists[0] = 0; d_num_lists[1] = 0;
}

////////////////////////////////////////////////////////////
//    Function: nmg_Viz_WireFrame::Destructor
//      Access: Public
// Description:
////////////////////////////////////////////////////////////
nmg_Viz_WireFrame::
~nmg_Viz_WireFrame()
{
	if (d_vertexPtr) {
		for(int i = 0; i < 2; i++) {
			for(int j=0;j < d_VertexArrayDim; j++) {
				delete [] d_vertexPtr[i][j];
			}
		}

		delete [] d_vertexPtr[0];
		delete [] d_vertexPtr[1];
		delete [] d_vertexPtr;
	}
}

////////////////////////////////////////////////////////////
//    Function: nmg_Viz_WireFrame::rebuildGrid
//      Access: Public
// Description: Rebuilds the display lists that correspond
//              to the surface
////////////////////////////////////////////////////////////
int nmg_Viz_WireFrame::
rebuildGrid()
{
    nmb_PlaneSelection planes;  
    planes.lookup(d_dataset);

	//ensureMaskPlane(planes);
	//ensureTransparentPlane(planes);
    
    buildMaskPlane(planes);   
    g_mask = ENABLE_MASK;
    
    VERBOSE(6, "nmg_Viz_WireFrame::rebuildGrid()");

    if (build_grid_display_lists(planes, display_lists_in_x, &d_list_base[0],
                                 &d_num_lists[0], d_num_lists[0], g_minColor,
                                 g_maxColor, d_vertexPtr[0])) 
    {
        return 0;
    }

    g_mask = INVERT_MASK;
    
    int stride = g_stride;
    g_stride = 5;
    
    if (build_grid_display_lists(planes, display_lists_in_x, &d_list_base[1],
                                 &d_num_lists[1], d_num_lists[1], g_minColor,
                                 g_maxColor, d_vertexPtr[1])) 
    {
        return 0;
    }  
    
    g_stride = stride;    
    g_mask = DISABLE_MASK;
    
    return 1;
}

////////////////////////////////////////////////////////////
//    Function: nmg_Viz_WireFrame::renderSurface
//      Access: Protected, Virtual
// Description: Renders the world.
////////////////////////////////////////////////////////////
void nmg_Viz_WireFrame::
renderSurface()
{
    drawLists(d_list_base[0], d_num_lists[0]);

	int old_texture_displayed = g_texture_displayed;
	g_texture_displayed = nmg_Graphics::NO_TEXTURES;

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    
	drawLists(d_list_base[1], d_num_lists[1]);
	
	g_texture_displayed = old_texture_displayed;
}

////////////////////////////////////////////////////////////
//    Function: nmg_Viz_WireFrame::rebuildInterval
//      Access: Public
// Description: 
////////////////////////////////////////////////////////////
int nmg_Viz_WireFrame::
rebuildInterval(int low_row, int high_row, int strips_in_x)
{
    nmb_PlaneSelection planes;
    planes.lookup(d_dataset);
    
    //Pass 1
    setUpdateAndTodo(low_row, high_row, g_stride, d_num_lists[0], 
                     last_marked[0], update[0], todo[0]);
    
	buildMaskPlane(planes);   
    g_mask = ENABLE_MASK;

    if (update[0].overlaps(todo[0]) || update[0].adjacent(todo[0])) {
        if (build_list_set(update[0] + todo[0], planes, d_list_base[0], 
                           d_num_lists[0], strips_in_x, d_vertexPtr[0])) return 0;
    } 
    else {
        if (build_list_set(update[0], planes, d_list_base[0], 
                           d_num_lists[0], strips_in_x, d_vertexPtr[0])) return 0;
        if (build_list_set(todo[0], planes, d_list_base[0], 
                           d_num_lists[0], strips_in_x, d_vertexPtr[0])) return 0;
    }
    
    //Pass 2
    setUpdateAndTodo(low_row, high_row, 5, d_num_lists[1], 
                     last_marked[1], update[1], todo[1]);
    
	g_mask = INVERT_MASK;
    
    int stride = g_stride;
    g_stride = 5;

    if (update[1].overlaps(todo[1]) || update[1].adjacent(todo[1])) {
        if (build_list_set(update[1] + todo[1], planes, d_list_base[1],
                           d_num_lists[1], strips_in_x, d_vertexPtr[1])) return 0;
    } 
    else {
        if (build_list_set(update[1], planes, d_list_base[1],
                           d_num_lists[1], strips_in_x, d_vertexPtr[1])) return 0;
        if (build_list_set(todo[1], planes, d_list_base[1],
                           d_num_lists[1], strips_in_x, d_vertexPtr[1])) return 0;
    }
    
	g_stride = stride;    
    g_mask = DISABLE_MASK;

    return 1;
}

////////////////////////////////////////////////////////////
//    Function: nmg_Viz_WireFrame::initVertexArrays
//      Access: Public
// Description: Allocates memory for each "vertex" array that
//              the class needs
////////////////////////////////////////////////////////////
int nmg_Viz_WireFrame::
initVertexArrays(int x, int y)
{
    if (!determineVertexDim(x,y)) {
		return 1;
	}
    
    d_vertexPtr = (Vertex_Struct***)new Vertex_Struct[2];
    
    for(int i = 0; i < 2; i++) {
        d_vertexPtr[i] = (Vertex_Struct **)new Vertex_Struct[d_VertexArrayDim];
        
        if (d_vertexPtr[i] == NULL)
            return 0;
        for(int j=0;j < d_VertexArrayDim; j++) {
            d_vertexPtr[i][j] = new Vertex_Struct[d_VertexArrayDim * 2];
            
            if(d_vertexPtr[i][j] == NULL )
                return 0;
        }
    }
    
    return 1;
}

////////////////////////////////////////////////////////////
//    Function: nmg_Viz_OpaqueTexture::Constructor
//      Access: Public
// Description:
////////////////////////////////////////////////////////////
nmg_Viz_OpaqueTexture::
nmg_Viz_OpaqueTexture(nmb_Dataset *dataset)
	: nmg_Visualization(dataset)
{
	d_list_base = new unsigned int[2];
	d_num_lists = new int[2];
	last_marked = new nmb_Interval[2];
	update = new nmb_Interval[2];
	todo = new nmb_Interval[2];

	d_list_base[0] = 0; d_list_base[1] = 0;
	d_num_lists[0] = 0; d_num_lists[1] = 0;

	/*
	PPM image("/nsrc/nano/src/app/nano/lib/nmGraphics/spots.ppm");

	d_texture = new unsigned char[image.nx * image.ny * 4];
	int tex_ind = 0;
	for(int i = 0; i < image.nx; i++) {
		for(int j = 0; j < image.ny; j++) {
			int r, g, b;
			image.Tellppm(i, j, &r, &g, &b);
			if (r == 255 && g == 255 &&	b == 255)
			{
				d_texture[tex_ind] = 255;
				d_texture[tex_ind+1] = 255;
				d_texture[tex_ind+2] = 255;
				d_texture[tex_ind+3] = 255;
			}
			else {
				d_texture[tex_ind] = r;
				d_texture[tex_ind+1] = g;
				d_texture[tex_ind+2] = b;
				d_texture[tex_ind+3] = 255;
			}
			tex_ind += 4;
		}
	}


	//build_spots_at_vertices();
	buildVisualizationTexture(image.nx, image.ny, (unsigned char*)image.pixels);

	g_tex_image_width[VISUALIZATION_TEX_ID] = d_tex_size;
    g_tex_installed_width[VISUALIZATION_TEX_ID] = d_tex_size;
    g_tex_image_height[VISUALIZATION_TEX_ID] = d_tex_size;
    g_tex_installed_height[VISUALIZATION_TEX_ID] = d_tex_size;
	*/
}

////////////////////////////////////////////////////////////
//    Function: nmg_Viz_OpaqueTexture::Destructor
//      Access: Public
// Description:
////////////////////////////////////////////////////////////
nmg_Viz_OpaqueTexture::
~nmg_Viz_OpaqueTexture()
{
	if (d_vertexPtr) {
		for(int i = 0; i < 2; i++) {
			for(int j=0;j < d_VertexArrayDim; j++) {
				delete [] d_vertexPtr[i][j];
			}
		}

		delete [] d_vertexPtr[0];
		delete [] d_vertexPtr[1];
		delete [] d_vertexPtr;
	}
}

////////////////////////////////////////////////////////////
//    Function: nmg_Viz_OpaqueTexture::rebuildGrid
//      Access: Public
// Description: Rebuilds the display lists that correspond
//              to the surface
////////////////////////////////////////////////////////////
int nmg_Viz_OpaqueTexture::
rebuildGrid()
{
    nmb_PlaneSelection planes;  
    planes.lookup(d_dataset);
    
	//ensureMaskPlane(planes);
	//ensureTransparentPlane(planes);

    buildMaskPlane(planes);   
    g_mask = ENABLE_MASK;
    
    VERBOSE(6, "nmg_Viz_OpaqueTexture::rebuildGrid()");

    if (build_grid_display_lists(planes, display_lists_in_x, &d_list_base[0],
                                 &d_num_lists[0], d_num_lists[0], g_minColor,
                                 g_maxColor, d_vertexPtr[0])) 
    {
        return 0;
    }
    
	g_mask = INVERT_MASK;
    
    if (build_grid_display_lists(planes, display_lists_in_x, &d_list_base[1],
                                 &d_num_lists[1], d_num_lists[1], g_minColor,
                                 g_maxColor, d_vertexPtr[1])) 
    {
        return 0;
    }  
    
    g_mask = DISABLE_MASK;
    
    return 1;
}

////////////////////////////////////////////////////////////
//    Function: nmg_Viz_OpaqueTexture::renderSurface
//      Access: Protected, Virtual
// Description: Renders the world.
////////////////////////////////////////////////////////////
void nmg_Viz_OpaqueTexture::
renderSurface()
{
    drawLists(d_list_base[0], d_num_lists[0]);

	int old_transform_mode = g_texture_transform_mode;
	int old_texture_displayed = g_texture_displayed;
	int old_texture_mode = g_texture_mode;

	g_texture_transform_mode = nmg_Graphics::RULERGRID_COORD;
	g_texture_displayed = nmg_Graphics::VISUALIZATION;
	g_texture_mode = GL_TEXTURE_2D;

	drawLists(d_list_base[1], d_num_lists[1]);

	g_texture_transform_mode = old_transform_mode;
	g_texture_displayed = old_texture_displayed;
	g_texture_mode = old_texture_mode;
}

////////////////////////////////////////////////////////////
//    Function: nmg_Viz_OpaqueTexture::rebuildInterval
//      Access: Public
// Description: 
////////////////////////////////////////////////////////////
int nmg_Viz_OpaqueTexture::
rebuildInterval(int low_row, int high_row, int strips_in_x)
{
    nmb_PlaneSelection planes;
    planes.lookup(d_dataset);
    
    //Pass 1
    setUpdateAndTodo(low_row, high_row, g_stride, d_num_lists[0], 
                     last_marked[0], update[0], todo[0]);

	buildMaskPlane(planes);   
    g_mask = ENABLE_MASK;
    
    if (update[0].overlaps(todo[0]) || update[0].adjacent(todo[0])) {
        if (build_list_set(update[0] + todo[0], planes, d_list_base[0], 
                           d_num_lists[0], strips_in_x, d_vertexPtr[0])) return 0;
    } 
    else {
        if (build_list_set(update[0], planes, d_list_base[0], 
                           d_num_lists[0], strips_in_x, d_vertexPtr[0])) return 0;
        if (build_list_set(todo[0], planes, d_list_base[0], 
                           d_num_lists[0], strips_in_x, d_vertexPtr[0])) return 0;
    }
    
    //Pass 2
    setUpdateAndTodo(low_row, high_row, 5, d_num_lists[1], 
                     last_marked[1], update[1], todo[1]);

	g_mask = INVERT_MASK;
    
    if (update[1].overlaps(todo[1]) || update[1].adjacent(todo[1])) {
        if (build_list_set(update[1] + todo[1], planes, d_list_base[1],
                           d_num_lists[1], strips_in_x, d_vertexPtr[1])) return 0;
    } 
    else {
        if (build_list_set(update[1], planes, d_list_base[1],
                           d_num_lists[1], strips_in_x, d_vertexPtr[1])) return 0;
        if (build_list_set(todo[1], planes, d_list_base[1],
                           d_num_lists[1], strips_in_x, d_vertexPtr[1])) return 0;
    }
    
    g_mask = DISABLE_MASK;

    return 1;
}

////////////////////////////////////////////////////////////
//    Function: nmg_Viz_OpaqueTexture::initVertexArrays
//      Access: Public
// Description: Allocates memory for each "vertex" array that
//              the class needs
////////////////////////////////////////////////////////////
int nmg_Viz_OpaqueTexture::
initVertexArrays(int x, int y)
{
    if (!determineVertexDim(x,y)) {
		return 1;
	}
    
    d_vertexPtr = (Vertex_Struct***)new Vertex_Struct[2];
    
    for(int i = 0; i < 2; i++) {
        d_vertexPtr[i] = (Vertex_Struct **)new Vertex_Struct[d_VertexArrayDim];
        
        if (d_vertexPtr[i] == NULL)
            return 0;
        for(int j=0;j < d_VertexArrayDim; j++) {
            d_vertexPtr[i][j] = new Vertex_Struct[d_VertexArrayDim * 2];
            
            if(d_vertexPtr[i][j] == NULL )
                return 0;
        }
    }
    
    return 1;
}

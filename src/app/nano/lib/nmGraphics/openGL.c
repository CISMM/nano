/*===3rdtech===
Copyright (c) 2000 by 3rdTech, Inc.
All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
===3rdtech===*/
/****************************************************************************
*				openGL.c
*      Qiang Liu 07/28/95
*	This program is part of microscape. It primarily deals with openGL
* related stuff. This code was based on nanoGL.c written by Russell Taylor.
*	The marker XXX indicates an area of code that needs more attention.
****************************************************************************/

/*XXX In raster scan mode, we end up refreshing the whole surface on line wrap*/
/*XXX Add an option that does not use Z buffering? */

#include	<math.h>	/* System includes */
#include	<stdio.h>
#include	<fcntl.h>
#include	<string.h>
//#include	<sys/time.h>
#include	<sys/types.h>
#include	<errno.h>
#include        <v.h>

#include	<GL/gl.h>	/* graphics-specific includes */

#include <BCPlane.h>
#include <nmb_Dataset.h>
#include <nmb_Decoration.h>
#include <nmb_PlaneSelection.h>
#include <nmb_Globals.h>
#include <nmb_Interval.h>
#include <nmb_Line.h>

// UGRAPHICS GLOBAL DEFINED IN MICROSCAPE.C
#include <UTree.h>
#include <URender.h>
extern UTree World;

#include	"nmg_Graphics.h" // for enums
#include    "nmg_Surface.h"
#include	"openGL.h"
#include	"globjects.h"  // for myworld()
#include	"graphics_globals.h"
#include	"graphics.h" // for compute_texture_matrix()
#include	"spm_gl.h"

#include	"Timer.h"

/*Globals*/
extern int	do_y_fastest;		/* Tells which direction to scan in */
extern int	do_raster;	      /* Raster (vs. Boustrophedonic) scan? */
extern int setup_lighting(int);

#ifdef MIN
#undef MIN
#endif
#define MIN(a,b) ((a)<(b)?(a):(b))
#ifdef MAX
#undef MAX
#endif
#define MAX(a,b) ((a)<(b)?(b):(a))

int	display_lists_in_x = 1;	/* Are display lists strips in X? */

#define VERBOSECHECK(level)	if (spm_graphics_verbosity >= level) report_gl_errors();

// Define EXPENSIVE_DISPLAY_LISTS if recomputing display lists is very
// expensive on the current architecture (but you still want to use them).
// It recomputes only those lists which are not likely to have to be
// recomputed in the near future as more data comes in from the scope.

// #define EXPENSIVE_DISPLAY_LISTS

int report_gl_errors(void)
{
    v_gl_set_context_to_vlib_window(); 
    // Calling glGetError() on PixelFlow has horrible performance
    GLenum errcode;
    int n_errors = 0;
    while ( (errcode = glGetError()) != GL_NO_ERROR) {
        n_errors++;
        switch (errcode) {
        case GL_INVALID_ENUM:
            fprintf(stderr,"Warning: GL error GL_INVALID_ENUM occurred\n");
            break;
        case GL_INVALID_VALUE:
            fprintf(stderr,"Warning: GL error GL_INVALID_VALUE occurred\n");
            break;
        case GL_INVALID_OPERATION:
            fprintf(stderr,"Warning: GL error GL_INVALID_OPERATION occurred\n");
            break;
        case GL_STACK_OVERFLOW:
            fprintf(stderr,"Warning: GL error GL_STACK_OVERFLOW occurred\n");
            break;
        case GL_STACK_UNDERFLOW:
            fprintf(stderr,"Warning: GL error GL_STACK_UNDERFLOW occurred\n");
            break;
        case GL_OUT_OF_MEMORY:
            fprintf(stderr,"Warning: GL error GL_OUT_OF_MEMORY occurred\n");
            break;
        default:
            fprintf(stderr,"Warning: GL error (code 0x%x) occurred\n", errcode);
        }
    }
    return n_errors;
}
int check_extension (const char *exten) {
    static const GLubyte * extensions = glGetString(GL_EXTENSIONS);
    char *work_str;
    char *token;
    
    if (!extensions)
        return 0;
    
    work_str = new char [strlen((const char *)extensions) + 1];
    strcpy(work_str,(const char *) extensions);
    token = strtok(work_str, " ");
    
    if (strcmp(token, exten) == 0) {
        delete [] work_str;
        return 1;
    } 
    while( (token=strtok(NULL, " ")) != NULL) {
        if (strcmp(token, exten) == 0) {
            delete [] work_str;
            return 1;
        }
    }
    delete [] work_str;
    return 0;
}

void determine_GL_capabilities() {
    
    int gl_1_1 = 0;
    
#ifdef GL_VERSION_1_1
    gl_1_1 = 1;
#endif
    
#if ( defined(linux) || defined(hpux) )
    // RMT The accelerated X server seems to be telling us that we have
    // vertex arrays, but then they are not drawn; ditto for MesaGL on
    // HPUX
    g_VERTEX_ARRAY = 0;
#else
    g_VERTEX_ARRAY = gl_1_1; //Vertex arrays are standard as of GL 1.1
    //g_VERTEX_ARRAY = 0;
#endif
}

//ADDED BY DANIEL ROHRER
#define MAXFONTS 2
int	font_array[MAXFONTS];

//GLdouble minColor[3];		/* Color areas of lowest colorparams */
//GLdouble maxColor[3];		/* Color areas of highest colorparams */
//GLuint	grid_list_base;		/* Base for grid display lists */
//GLsizei	num_grid_lists;		/* Number of display lists for grid */
char	message[1000];		/* Message to display on screen */

//int (* stripfn)
//    (nmb_PlaneSelection, GLdouble [3], GLdouble [3], int, Vertex_Struct *);

#ifdef RENDERMAN
GLfloat cur_projection_matrix[16];
GLfloat cur_modelview_matrix[16];
#endif


/**********************************************************************
* This routine creates display lists for the grid.
* It relies on an external routine to determine the set of lists to
* make, the direction (whether x or y is faster), and other important
* variables.
* Returns -1 on failure, 0 on success.
**********************************************************************/

int build_list_set
(nmb_Interval subset,
 nmb_PlaneSelection planes, nmg_SurfaceMask *mask,
 GLuint base,
 GLsizei num_lists,
 GLdouble * minColor,
 GLdouble * maxColor,
 int (* stripfn)
 (nmb_PlaneSelection, nmg_SurfaceMask *, GLdouble [3], GLdouble [3], int, Vertex_Struct *),
 Vertex_Struct **surface)
{
    
    v_gl_set_context_to_vlib_window(); 
    // globals:
    // surface
    // min/maxColor
    
    int i;
    
    if (!g_just_color && subset.empty()) return 0;
    
#if defined(sgi) || defined(_WIN32)
    //#if defined(sgi)
    if (g_VERTEX_ARRAY) { // same extension is for COLOR_ARRAY
        
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
        //if (planes.color || g_PRERENDERED_COLORS || g_PRERENDERED_TEXTURE ||
        //    g_null_data_alpha_toggle || g_transparent) {
            //glEnable(GL_COLOR_ARRAY_EXT);
            glEnableClientState(GL_COLOR_ARRAY);
        //} else {
            //glDisable(GL_COLOR_ARRAY_EXT);
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
    
    if (g_VERTEX_ARRAY){// same extension is for TEXTURE_COORD_ARRAY
#ifndef PROJECTIVE_TEXTURE
        if (g_texture_displayed != nmg_Graphics::NO_TEXTURES) {
            //glEnable(GL_TEXTURE_COORD_ARRAY_EXT);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        } else {
            //glDisable(GL_TEXTURE_COORD_ARRAY_EXT);
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        }
        if (glGetError() != GL_NO_ERROR) {
            printf(" Error setting GL_TEXTURE_COORD_ARRAY_EXT.\n");
        } 
#else
        if ( planes.contour || planes.alpha) {
            //glEnable(GL_TEXTURE_COORD_ARRAY_EXT);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        } else {
            //glDisable(GL_TEXTURE_COORD_ARRAY_EXT);
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        }
        if (glGetError() != GL_NO_ERROR) {
            printf(" Error setting GL_TEXTURE_COORD_ARRAY_EXT.\n");
        } 
#endif // PROJECTIVE_TEXTURE
    }
    
#endif // sgi or win32
    //#endif // sgi 
    
    if (spm_graphics_verbosity >= 15) { 
        fprintf(stderr, "  updating %d - %d", subset.low(), subset.high());
    } 
    // Store g_just_color
    vrpn_bool g_just_color_was_on = g_just_color;
    // If we are re-doing the whole surface, we don't need to then
    // re-do the color, so turn flag off.
    if ( (subset.low() == 0) && (subset.high() == num_lists -1) ) {
        g_just_color_was_on = 0;
    }
    
    // turn g_just_color off so only geometry gets re-generated
    g_just_color = 0;
    for (i = subset.low(); i <= subset.high(); i++) {
        
        if (spm_graphics_verbosity >= 10) {
            fprintf(stderr, "    newing list %d for strip %d.\n", base + i, i);
        }
        
        glNewList(base + i, GL_COMPILE);
        
        VERBOSECHECK(10);
        
        if ((*stripfn)(planes, mask, minColor, maxColor, i*g_stride, surface[i])) {
            if (g_VERTEX_ARRAY) {
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
    if ( g_just_color_was_on ) {
        // Flag tells stripfn to only regenerate color, and use cached normals 
        // and vertices. 
        g_just_color = 1;
        // re-color the whole surface
        for (i = 0; i < num_lists; i++) {
            
            if (spm_graphics_verbosity >= 10) {
                fprintf(stderr, "    newing list %d for strip %d.\n", base + i, i);
            }
            
            glNewList(base + i, GL_COMPILE);
            
            VERBOSECHECK(10);
            
            if ((*stripfn)(planes, mask, minColor, maxColor, i*g_stride, surface[i])) {
                if (g_VERTEX_ARRAY) {
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
    g_just_color = 0;
    return 0;
}

/**********************************************************************
* Modified by Jason 11/19/00
*
* It's useful to not have to pass in the strip function to this
* procedure.  It makes the Visualization classes simpler
*
*********************************************************************/
int build_list_set (
                    nmb_Interval insubset,
                    nmb_PlaneSelection planes, nmg_SurfaceMask *mask,
                    GLuint base, GLsizei num,
                    int strips_in_x, Vertex_Struct **surface)
{
    
    v_gl_set_context_to_vlib_window(); 
    
    // TCH 27 Jan 00
    // Quick hacking to fix bug?
    //int maxStrip;
    //if (strips_in_x) {
    //maxStrip = (planes.height->numY() - 1) / g_stride;
    //} else {
    //maxStrip = (planes.height->numX() - 1) / g_stride;
    //}
    //fprintf(stderr, "Max strip is %d;  input max was %d.\n", maxStrip, insubset.high());
    //nmb_Interval subset (MAX(0, insubset.low()),
    //MIN(maxStrip - 1, insubset.high()));
    
    int (* stripfn)
        (nmb_PlaneSelection, nmg_SurfaceMask *, GLdouble [3], GLdouble [3], int, Vertex_Struct *);
    
    if (strips_in_x) {
        stripfn = spm_x_strip_masked;
    } else {
        stripfn = spm_y_strip_masked;
    }
    
    nmb_Interval subset (MAX(0, insubset.low()),
        MIN(num - 1, insubset.high()));
    
    if (!subset.empty()) {      
        if (spm_graphics_verbosity >= 8) {
            fprintf(stderr, "Deleting display lists from %d for %d.\n",
                base + subset.low(), subset.high() - subset.low() + 1);
        }
        glDeleteLists(base + subset.low(), subset.high() - subset.low() + 1);
        VERBOSECHECK(8);
    } 
    
    return build_list_set(subset, planes, mask, base, num,
        g_minColor, g_maxColor, stripfn, surface);
}


/*************************************************************************
*	This routine creates display lists for the grid.  It will make
* the lists in either X or Y fastest, and will delete the old lists (if
* this is not the first time around) before making the new ones.
* 	It returns the base name of the display lists (which are
* contiguous) and the number of lists that it generates.
*	This routine returns -1 on failure and 0 on success.
*
*  Modified by JMC on 11/19/00
*  build_grid_display_lists now assumes that old_num specifies the old
*  number of lists allocated for the current base.  If old_num is less than
*  1, then it assumes that it is being called for the first time for
*  a particular base. 
*************************************************************************/

int	build_grid_display_lists(nmb_PlaneSelection planes,  nmg_SurfaceMask *mask, 
                             int strips_in_x, GLuint *base, GLsizei *num, 
                             GLsizei old_num, GLdouble *minColor, GLdouble *maxColor,
                             Vertex_Struct **surface)
{
    
    int (* stripfn)
        (nmb_PlaneSelection, nmg_SurfaceMask *, GLdouble [3], GLdouble [3], int, Vertex_Struct *);
    
    VERBOSE(4,"     build_grid_display_lists in openGL.c");
    VERBOSECHECK(4);
    
    v_gl_set_context_to_vlib_window(); 
    
    /* If this is not the first time around, free the old display lists */
    if (old_num > 0) {
        glDeleteLists(*base, old_num);
        if (spm_graphics_verbosity >= 6)
            fprintf(stderr, "      deleted display lists "
            "from %d for %d.\n", *base, old_num);
    }
    
    VERBOSE(4,"     build_grid_display_lists in openGL.c");
    TIMERVERBOSE(5, mytimer, "begin build_grid_display_lists");
    
    /* set material parameters */
    spm_set_surface_materials();
    if (report_gl_errors()) {
        printf("spm_set_surface_materials: generated gl error\n");
    }
    
    // If we have a very small grid size, make sure g_stride doesn't tell us
    // to skip any.
    if ((planes.height->numY() < 10) || (planes.height->numX() < 10)) {
        g_stride = 1;
    }
    // Figure out how many strips we will need.  Recall that we are
    // skipping along by stride gridpoints each time.
    if (strips_in_x) {
        *num = (planes.height->numY() - 1) / g_stride;
        stripfn = spm_x_strip_masked;
    } else {
        *num = (planes.height->numX() - 1) / g_stride;
        stripfn = spm_y_strip_masked;
    }
    
    //fprintf(stderr, "Generating %d lists.\n", *num);
    
    // Generate a new set of display list indices
    if ( (*base = glGenLists(*num)) == 0) {
        fprintf(stderr,
            "build_grid_display_lists(): Couldn't get indices\n");
        return(-1);
    }
    if (spm_graphics_verbosity >= 6)
        fprintf(stderr, "    allocated display lists from %d for %d.\n",
        *base, *num);
    
#if defined(sgi) || defined(_WIN32)
    //#if defined(sgi)
    // use vertex array extension
    if (g_VERTEX_ARRAY) {
        //glEnable(GL_VERTEX_ARRAY_EXT);
        glEnableClientState(GL_VERTEX_ARRAY);
        // Color arrays are enabled/disabled dynamically depending
        // on whether or not planes.color or g_PRERENDERED_COLORS are
        // valid.
        if (!g_PRERENDERED_COLORS && !g_PRERENDERED_TEXTURE) {
            //glEnable(GL_NORMAL_ARRAY_EXT);
            glEnableClientState(GL_NORMAL_ARRAY);
        }
    }
#endif
    
    build_list_set(nmb_Interval (0, *num - 1), planes, mask, *base, *num,
        minColor, maxColor, stripfn, surface);
    
    VERBOSE(4,"     done build_grid_display_lists in openGL.c");
    VERBOSECHECK(4);
    
    TIMERVERBOSE(5, mytimer, "end build_grid_display_lists");
    
    return(0);
}

int draw_world (int) {
    
    v_gl_set_context_to_vlib_window(); 
    /********************************************************************/
    // Draw Ubergraphics
    /********************************************************************/
      
    //UGRAPHICS CALL TO DRAW THE WORLD  -- ASSUMING THAT VLIB HAS IT IN WORLD SPACE
    //WHEN I HIT THIS POINT
    if (g_config_enableUber) {
        World.Do(&URender::Render);
    }
    
    /********************************************************************/
    // End Ubergraphics
    /********************************************************************/
    
    TIMERVERBOSE(3, mytimer, "begin draw_world");
    
    // Look up the planes we're mapping to various data sets this time.
    // If we don't have a height plane, no sense trying to draw anything.
    VERBOSECHECK(4);
    VERBOSE(4,"    Looking up planes in openGL.c");
    TIMERVERBOSE(5, mytimer, "draw_world:Looking up planes");
    
    nmb_PlaneSelection planes;
    /*
    planes.lookup(g_inputGrid, g_heightPlaneName,
    g_colorPlaneName, g_contourPlaneName, g_alphaPlaneName);
    */
    
    planes.lookup(g_inputGrid, g_heightPlaneName, g_colorPlaneName,
        g_contourPlaneName, g_opacityPlaneName,
        g_alphaPlaneName, g_maskPlaneName, g_transparentPlaneName,
        g_vizPlaneName);
    
    if (g_PRERENDERED_COLORS) {
        planes.lookupPrerenderedColors(g_prerendered_grid);
    }
    if (g_PRERENDERED_DEPTH) {
        planes.lookupPrerenderedDepth(g_prerendered_grid);
    }
    
    if (planes.height == NULL) {	
        return -1;
    }
    
    
    //fprintf(stderr, "Corners of grid:  (%.2f, %.2f), (%.2f, %.2f).\n",
    //planes.height->xInWorld(0), planes.height->yInWorld(0),
    //planes.height->xInWorld(planes.height->numX() - 1),
    //planes.height->yInWorld(planes.height->numX() - 1));
    
    
    /********************************************************************/
    // Set up surface materials parameters for the surface, then draw it
    /********************************************************************/
    
    VERBOSECHECK(4);
    VERBOSE(4,"    Setting surface materials");
    TIMERVERBOSE(5, mytimer, "draw_world: spm_set_surface_materials");    
    TIMERVERBOSE(5, mytimer, "draw_world: spm_set_surface_materials");
    
    /************************************************************
    * If the region has changed, we need to rebuild the display
    * lists for the surface.
    ************************************************************/
    
    // TODO  - add special cases for g_PRERENDERED_COLORS
    
    if (decoration->selectedRegion_changed) {
        
        VERBOSECHECK(4);
        VERBOSE(4,"    Rebuilding display lists (for new selected region)");
        if (!g_surface->rebuildSurface(VRPN_TRUE)) {
            fprintf(stderr,
                "ERROR: Could not build grid display lists\n");
            dataset->done = V_TRUE;
        }
        
        /* Clear the range of change */
        dataset->range_of_change.Clear();
        
        decoration->selectedRegion_changed = 0;
    }
    
    /************************************************************
    * If the scan direction is inefficiently using the display 
    * lists, switch to using display lists in the other direction.
    ************************************************************/
    
    if (dataset->range_of_change.Changed()) {
        float	ratio;
        
        /* See which way more changes occurred */
        ratio = dataset->range_of_change.RatioOfChange();
        
        /* If the ratio is very skewed, make sure we are
        * scanning in the correct direction. */
        if (ratio > 4) {		/* 4x as much in y */
            if (display_lists_in_x) {	/* Going wrong way */
                
                display_lists_in_x = 0;
                VERBOSE(4,"    Rebuilding display lists (in y).");
                if (!g_surface->rebuildSurface(VRPN_TRUE)) {
                    fprintf(stderr,
                        "ERROR: Could not build grid display lists\n");
                    dataset->done = V_TRUE;
                }
                
                /* Clear the range of change */
                dataset->range_of_change.Clear();
            }
        } else if (ratio < 0.25) {	/* 4x as much in x */
            if (!display_lists_in_x) {	/* Going wrong way */
                
                display_lists_in_x = 1;
                VERBOSE(4,"    Rebuilding display lists (in x).");
                if (!g_surface->rebuildSurface(VRPN_TRUE)) {
                    fprintf(stderr,
                        "ERROR: Could not build grid display lists\n");
                    dataset->done = V_TRUE;
                }
                
                /* Clear the range of change */
                dataset->range_of_change.Clear();
            }
        }
    }
    

    //See if there are any regions that need rebuilding for
    //some reason
    if (!g_surface->rebuildSurface()) {
        fprintf(stderr,
            "ERROR: Could not build grid display lists\n");
        dataset->done = V_TRUE;
    }

    /**********************************************************
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
    *          norm   changed  norm                        
    * 	+ . . .	+ . . .	+ . . .	+ . . .	+ . . .	+
    * 	|_______|_______|_______|_______|_______|
    *     list-2  list-1  list    list+1
    * 
    **********************************************************/
    
    VERBOSECHECK(4);
    VERBOSE(4,"    Replacing changed display lists");
    TIMERVERBOSE(5, mytimer, "draw_world:Replacing changed display lists");
    
    // Semi-optimized, thread-safe version of display list redrawing code.
    // TCH 17 June 98
    
    // Trys to figure out which direction the scanning tip is moving
    // to reduce the amount of redrawing done.  Delays by one graphics
    // loop drawing strips that it expects to have to redraw after
    // receiving the next update from the microscope.
    
    // (Nearly) starvation-free:  if we ever stop receiving updates from
    // the microscope, all old cached lines (stored in last_marked)
    // are updated on the next pass through this routine.  There is one
    // starvation case:  if the microscope ever continuously scans and
    // rescans the same line of the sample more often than the graphics
    // process runs, that new data will never be drawn.
    
    //  int (* stripfn)
    //    (nmb_PlaneSelection, GLdouble [3], GLdouble [3], int, Vertex_Struct *);
    
    //  static nmb_Interval last_marked;
    //  nmb_Interval mark;
    //  nmb_Interval update;
    
    //  int direction;
    
    int low_row;
    int high_row;
    //  int low_strip;
    //  int high_strip;
    
    // Get the data (low and high X, Y vales changed) atomically
    // so we have bulletproof synchronization.
    
    dataset->range_of_change.GetBoundsAndClear
        (&g_minChangedX, &g_maxChangedX, &g_minChangedY, &g_maxChangedY);
    if (g_PRERENDERED_COLORS || g_PRERENDERED_DEPTH) {
        //fprintf(stderr, "Using prerendered grid for bounds of change.\n");
        g_prerenderedChange->GetBoundsAndClear
            (&g_minChangedX, &g_maxChangedX, &g_minChangedY, &g_maxChangedY);
    }
    
    if (display_lists_in_x) {
        
        low_row = g_minChangedY;
        high_row = g_maxChangedY;
        //stripfn = spm_x_strip;
        
        //fprintf(stderr, "X row from %d to %d.\n", low_row, high_row);
        
    } else {
        
        low_row = g_minChangedX;
        high_row = g_maxChangedX;
        //stripfn = spm_y_strip;
        
        //fprintf(stderr, "Y row from %d to %d.\n", low_row, high_row);
        
    }
    
    // Convert from rows to strips:  divide through by the tesselation stride
    
    
    if (!g_surface->rebuildInterval(low_row, high_row, display_lists_in_x)) {
        return -1;
    }

    /*
    if (update.overlaps(todo) || update.adjacent(todo)) {
    if (build_list_set(update + todo, planes, stripfn, display_lists_in_x)) return -1;
    } 
    else {
    if (build_list_set(update, planes, stripfn, display_lists_in_x)) return -1;
    if (build_list_set(todo, planes, stripfn, display_lists_in_x)) return -1;
    }
    */
    
    if (spm_graphics_verbosity >= 15)
        fprintf(stderr, "\n");
    VERBOSECHECK(15)
        
        //last_marked = mark;
        
        
    /* Draw grid using current viewing/modeling matrix */
    VERBOSECHECK(4);
    VERBOSE(4,"    Drawing the grid");
    TIMERVERBOSE(5, mytimer, "draw_world:Drawing the grid");
    
    /*
    for (i = 0; i < num_grid_lists; i++) {
    glCallList(grid_list_base + i);
    }
    */
    
    /* Draw the light */       
    setup_lighting(0);

    g_surface->renderSurface();
    
    setFilled();
    /*******************************************************/
    // Draw the parts of the scene other than the surface.
    /*******************************************************/
    
    // draw the microscope's current scanline as a visual indicator to the user
    // Not related to "scanline" mode, which controls the AFM tip. 
    if (decoration->drawScanLine && g_config_chartjunk) {
        float oldColor[4];
        float oldLineWidth[1];
        glGetFloatv(GL_CURRENT_COLOR, oldColor);
        glGetFloatv(GL_LINE_WIDTH, oldLineWidth);
        glColor4f(0.1, 1.0, 0.1, 1.0);
        glLineWidth(1.0);
        glBegin(GL_LINE_STRIP);
        for (int p = 0; p < decoration->scanLineCount; p++) {
            glVertex3f(decoration->scan_line[p][0], decoration->scan_line[p][1],
                decoration->scan_line[p][2] + 5.0);
        }
        glEnd();
        glLineWidth(oldLineWidth[0]);
        glColor4fv(oldColor);
    }
    
    if ((decoration->num_slow_line_3d_markers > 0) && g_config_chartjunk) {
        for (int i=0; i < decoration->num_slow_line_3d_markers; i++) {
            position_sphere( decoration->slowLine3dMarkers[i][0],
                decoration->slowLine3dMarkers[i][1],
                decoration->slowLine3dMarkers[i][2] );
            mysphere(NULL);
        }
    }
    
    // TCH 8 April 98 don't know where these go best
    if (decoration->red.changed()) {
        //fprintf(stderr, "Making red line.\n");
        make_red_line(decoration->red.top(), decoration->red.bottom());
        decoration->red.clearChanged();
    }
    if (decoration->green.changed()) {
        //fprintf(stderr, "Making green line.\n");
        make_green_line(decoration->green.top(), decoration->green.bottom());
        decoration->green.clearChanged();
    }
    if (decoration->blue.changed()) {
        //fprintf(stderr, "Making blue line.\n");
        make_blue_line(decoration->blue.top(), decoration->blue.bottom());
        decoration->blue.clearChanged();
    }
    /* OBSOLETE, never called. 
    if (decoration->selectedRegion_changed) {
        //fprintf(stderr, "Making selected region marker.\n");
        make_selected_region_marker
            (decoration->selectedRegionMinX,
            decoration->selectedRegionMinY,
            decoration->selectedRegionMaxX,
            decoration->selectedRegionMaxY);
        decoration->selectedRegion_changed = 0;
        // XXX won't this be cleared in draw_world()?
    }
    */
    if (decoration->aimLine.changed()) {
        make_aim(decoration->aimLine.top(), decoration->aimLine.bottom());
        decoration->aimLine.clearChanged();
    }
    
    if (decoration->trueTipLocation_changed) {
        g_trueTipLocation[0] = decoration->trueTipLocation[0];
        g_trueTipLocation[1] = decoration->trueTipLocation[1];
        g_trueTipLocation[2] = decoration->trueTipLocation[2];
        if (spm_graphics_verbosity >= 12)
            fprintf(stderr, "Setting true tip location to (%.2f %.2f %.2f).\n",
            g_trueTipLocation[0], g_trueTipLocation[1],
            g_trueTipLocation[2]);
        decoration->trueTipLocation_changed = 0;
    }
    
    if (g_position_collab_hand) {
        make_collab_hand_icon(g_collabHandPos, g_collabHandQuat,
            g_collabMode);
        g_position_collab_hand = 0;
    }	
    
    
    
    // Set the lighting model for the measurement things
    VERBOSECHECK(4);
    VERBOSE(4,"    Setting measurement materials");
    spm_set_measure_materials();
    
    /* Draw the pulse indicators */
    glColor3f(1.0f, 0.3f, 0.3f);
    decoration->traverseVisiblePulses(spm_render_mark, NULL);
    /* Draw the scrape indicators */
    glColor3f(1.0f, 1.0f, 1.0f);
    decoration->traverseVisibleScrapes(spm_render_mark, NULL);
    
    // Set the lighting model for the icons in the world, then draw it
    VERBOSECHECK(4);
    VERBOSE(4,"    Setting icon materials");
    spm_set_icon_materials();
    VERBOSECHECK(4);
    VERBOSE(4,"    Drawing the world");
    TIMERVERBOSE(5, mytimer, "draw_world:Drawing the world");
    
    myworld();
    
    /***************************/
    /* Check for any GL errors */
    /***************************/
    report_gl_errors();
    
    TIMERVERBOSE(3, mytimer, "end draw_world");
    
    return(0);
}





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

#include	"nmg_Graphics.h" // for enums
#include	"openGL.h"
#include	"globjects.h"  // for myworld()
#include	"graphics_globals.h"
#include	"graphics.h" // for compute_texture_matrix()
#include	"spm_gl.h"

#include	"Timer.h"


/*Globals*/
extern int	do_y_fastest;		/* Tells which direction to scan in */
extern int	do_raster;	      /* Raster (vs. Boustrophedonic) scan? */
#ifndef FLOW
extern int setup_lighting(int);
#endif

#ifdef MIN
  #undef MIN
#endif
#define MIN(a,b) ((a)<(b)?(a):(b))
#ifdef MAX
  #undef MAX
#endif
#define MAX(a,b) ((a)<(b)?(b):(a))

int	display_lists_in_x = 1;	/* Are display lists strips in X? */

#ifdef FLOW
  #define VERBOSECHECK(level)	if (spm_graphics_verbosity >= level) ;
#else
  #define VERBOSECHECK(level)	if (spm_graphics_verbosity >= level) report_gl_errors();
#endif

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


int check_extension (const GLubyte * exten_str) {
  char work_str[1000];
  char *token;

  if (!exten_str)
    return 0;

  strcpy(work_str,(const char *) exten_str);
  token = strtok(work_str, " ");
  
  if (strcmp(token, "GL_EXT_vertex_array") == 0) {
     return 1;
  } 
  while( (token=strtok(NULL, " ")) != NULL) {
       if (strcmp(token, "GL_EXT_vertex_array") == 0) {
           return 1;
       }
  }
  return 0;
}


//ADDED BY DANIEL ROHRER
#define MAXFONTS 2
int	font_array[MAXFONTS];

//GLdouble minColor[3];		/* Color areas of lowest colorparams */
//GLdouble maxColor[3];		/* Color areas of highest colorparams */
GLuint	grid_list_base;		/* Base for grid display lists */
GLsizei	num_grid_lists;		/* Number of display lists for grid */
char	message[1000];		/* Message to display on screen */

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
     nmb_PlaneSelection planes,
     GLuint base,
     GLdouble * minColor,
     GLdouble * maxColor,
     int (* stripfn)
       (nmb_PlaneSelection, GLdouble [3], GLdouble [3], int, Vertex_Struct *))
{

  v_gl_set_context_to_vlib_window(); 
  // globals:
  // vertexptr
  // min/maxColor

  int i;
  int count;

  if (subset.empty()) return 0;

#if defined(sgi) || defined(__CYGWIN__)
  if (g_VERTEX_ARRAY) { // same extension is for COLOR_ARRAY
    if (planes.color || g_PRERENDERED_COLORS || g_PRERENDERED_TEXTURE) {
      glEnable(GL_COLOR_ARRAY_EXT);
    } else {
      glDisable(GL_COLOR_ARRAY_EXT);
    }
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
      glEnable(GL_TEXTURE_COORD_ARRAY_EXT);
    } else {
      glDisable(GL_TEXTURE_COORD_ARRAY_EXT);
    }
    if (glGetError() != GL_NO_ERROR) {
      printf(" Error setting GL_TEXTURE_COORD_ARRAY_EXT.\n");
    } 
#else
    if ( planes.contour || planes.alpha) {
      glEnable(GL_TEXTURE_COORD_ARRAY_EXT);
    } else {
      glDisable(GL_TEXTURE_COORD_ARRAY_EXT);
    }
    if (glGetError() != GL_NO_ERROR) {
       printf(" Error setting GL_TEXTURE_COORD_ARRAY_EXT.\n");
    } 
#endif // PROJECTIVE_TEXTURE
  }

#endif // sgi or __CYGWIN__

  if (spm_graphics_verbosity >= 15) {
    fprintf(stderr, "  updating %d - %d", subset.low(), subset.high());
  }

  for (i = subset.low(); i <= subset.high(); i++) {

    if (spm_graphics_verbosity >= 10) {
      fprintf(stderr, "    newing list %d for strip %d.\n", base + i, i);
    }

    glNewList(base + i, GL_COMPILE);

    VERBOSECHECK(10);

    if (g_VERTEX_ARRAY) {
      count = (*stripfn)(planes, minColor, maxColor, i * g_stride,
                         vertexptr[i]);
      if (count == -1) {
        fprintf(stderr, "build_list_set():  "
                        "Internal error (with arrays) - bad strip\n");
        return -1;
      }

      specify_vertexArray(planes, i, count);
    } else {
      if ((*stripfn)(planes, minColor, maxColor, i * g_stride, NULL)) {
        fprintf(stderr, "build_list_set():  "
                        "Internal error - bad strip\n");
        return -1;
      }
    }

    if (spm_graphics_verbosity >= 10) {
      fprintf(stderr, "    updated %d.\n", i);
    }
    VERBOSECHECK(10);

    glEndList();
  }

  return 0;
}

int build_list_set
    (nmb_Interval insubset,
     nmb_PlaneSelection planes,
     int (* stripfn)
       (nmb_PlaneSelection, GLdouble [3], GLdouble [3], int, Vertex_Struct *))
{

  v_gl_set_context_to_vlib_window(); 
  nmb_Interval subset (MAX(0, insubset.low()),
                       MIN(num_grid_lists - 1, insubset.high()));

  if (!subset.empty()) {

    if (spm_graphics_verbosity >= 8)
      fprintf(stderr, "Deleting display lists from %d for %d.\n",
              grid_list_base + subset.low(), subset.high() - subset.low() + 1);
    glDeleteLists(grid_list_base + subset.low(),
                  subset.high() - subset.low() + 1);
    VERBOSECHECK(8);

    return build_list_set(subset, planes, grid_list_base,
                          g_minColor, g_maxColor, stripfn);
  } else
    return 0;
}


/*************************************************************************
 *	This routine creates display lists for the grid.  It will make
 * the lists in either X or Y fastest, and will delete the old lists (if
 * this is not the first time around) before making the new ones.
 * 	It returns the base name of the display lists (which are
 * contiguous) and the number of lists that it generates.
 *	This routine returns -1 on failure and 0 on success.
 *************************************************************************/

int	build_grid_display_lists(nmb_PlaneSelection planes, int strips_in_x,
				 GLuint *base, GLsizei *num,
				 GLdouble *minColor, GLdouble *maxColor)
{
       
  int (* stripfn)
    (nmb_PlaneSelection, GLdouble [3], GLdouble [3], int, Vertex_Struct *);

	static	int	first_call = 1;	/* First time we were called? */
	static	int	last_num_lists = 0;	// Number of lists before

	VERBOSE(4,"     build_grid_display_lists in openGL.c");
        VERBOSECHECK(4);
 
	v_gl_set_context_to_vlib_window(); 

	/* If this is not the first time around, free the old display lists */
	if (first_call) {
		first_call = 0;		/* Won't be next time */
	} else {
		glDeleteLists(*base, last_num_lists);
		if (spm_graphics_verbosity >= 6)
			fprintf(stderr, "      deleted display lists "
                                "from %d for %d.\n", *base, last_num_lists);
	}

	VERBOSE(4,"     build_grid_display_lists in openGL.c");
	TIMERVERBOSE(5, mytimer, "begin build_grid_display_lists");
 
        /* set material parameters */
        spm_set_surface_materials();
        if (report_gl_errors()) {
	   printf("spm_set_surface_materials: generated gl error\n");
	}

	// Figure out how many strips we will need.  Recall that we are
	// skipping along by stride gridpoints each time.
	if (strips_in_x) {
		*num = (planes.height->numY() - 1) / g_stride;
		stripfn = spm_x_strip;
	} else {
		*num = (planes.height->numX() - 1) / g_stride;
		stripfn = spm_y_strip;
	}

	// Generate a new set of display list indices
	if ( (*base = glGenLists(*num)) == 0) {
		fprintf(stderr,
			"build_grid_display_lists(): Couldn't get indices\n");
		return(-1);
	}
        if (spm_graphics_verbosity >= 6)
          fprintf(stderr, "    allocated display lists from %d for %d.\n",
                  *base, *num);

#if defined(sgi) || defined(__CYGWIN__)
	// use vertex array extension
	if (g_VERTEX_ARRAY) {
	  glEnable(GL_VERTEX_ARRAY_EXT);
          // Color arrays are enabled/disabled dynamically depending
          // on whether or not planes.color or g_PRERENDERED_COLORS are
          // valid.
          if (!g_PRERENDERED_COLORS && !g_PRERENDERED_TEXTURE) {
	    glEnable(GL_NORMAL_ARRAY_EXT);
          }
	}
#endif

	build_list_set(nmb_Interval (0, *num - 1), planes, *base,
                       minColor, maxColor, stripfn);

	last_num_lists = *num;	// Remember how may done this time

	VERBOSE(4,"     done build_grid_display_lists in openGL.c");
        VERBOSECHECK(4);
 
	TIMERVERBOSE(5, mytimer, "end build_grid_display_lists");

	return(0);
}

//UGRAPHICS GLOBAL DEFINED IN MICROSCAPE.C
#include "UTree.h"
#include "URender.h"
extern UTree World;
int draw_world (int) {

  int i;

  v_gl_set_context_to_vlib_window(); 
  /********************************************************************/
  // Draw Ubergraphics
  /********************************************************************/

  //UGRAPHICS CALL TO DRAW THE WORLD  -- ASSUMING THAT VLIB HAS IT IN WORLD SPACE
  //WHEN I HIT THIS POINT
  World.Do(&URender::Render);

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
  planes.lookup(g_inputGrid, g_heightPlaneName,
                g_colorPlaneName, g_contourPlaneName, g_alphaPlaneName);
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

  spm_set_surface_materials();

  TIMERVERBOSE(5, mytimer, "draw_world: spm_set_surface_materials");

  /************************************************************
   * If the region has changed, we need to rebuild the display
   * lists for the surface.
   ************************************************************/

  // TODO  - add special cases for g_PRERENDERED_COLORS

  if (decoration->selectedRegion_changed) {
    
    VERBOSECHECK(4);
    VERBOSE(4,"    Rebuilding display lists (for new selected region)");
    if (build_grid_display_lists(planes,
				 display_lists_in_x, &grid_list_base,
				 &num_grid_lists, g_minColor,g_maxColor)) {
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
	if (build_grid_display_lists(planes,
				     display_lists_in_x, &grid_list_base,
				     &num_grid_lists, g_minColor,g_maxColor)) {
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
	if (build_grid_display_lists(planes,
				     display_lists_in_x, &grid_list_base,
				     &num_grid_lists, g_minColor,g_maxColor)) {
	  fprintf(stderr,
		  "ERROR: Could not build grid display lists\n");
	  dataset->done = V_TRUE;
	}
	
	/* Clear the range of change */
	dataset->range_of_change.Clear();
      }
    }
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

  int (* stripfn)
    (nmb_PlaneSelection, GLdouble [3], GLdouble [3], int, Vertex_Struct *);

  static nmb_Interval last_marked;
  nmb_Interval mark;
  nmb_Interval update;

  int direction;

  int low_row;
  int high_row;
  int low_strip;
  int high_strip;

  dataset->range_of_change.GetBoundsAndClear
    (&g_minChangedX, &g_maxChangedX, &g_minChangedY, &g_maxChangedY);
  if (g_PRERENDERED_COLORS || g_PRERENDERED_DEPTH) {
    g_prerenderedChange->GetBoundsAndClear
      (&g_minChangedX, &g_maxChangedX, &g_minChangedY, &g_maxChangedY);
  }

  if (display_lists_in_x) {

    // Get the data (low and high Y vales changed) atomically
    // so we have bulletproof synchronization.
    //dataset->range_of_change.GetBoundsAndClear
      //(NULL, NULL, &low_row, &high_row);
    low_row = g_minChangedY;
    high_row = g_maxChangedY;
    stripfn = spm_x_strip;

  } else {

    // Get the data (low and high X vales changed) atomically
    // so we have bulletproof synchronization.
    //dataset->range_of_change.GetBoundsAndClear
      //(&low_row, &high_row, NULL, NULL);
    low_row = g_minChangedX;
    high_row = g_maxChangedX;
    stripfn = spm_y_strip;

  }

  // Convert from rows to strips:  divide through by the tesselation stride

  low_strip = MAX(0, low_row / g_stride);
  high_strip = MIN(num_grid_lists, high_row / g_stride);

  // Figure out what direction the scan has apparently progressed
  // since the last time.  Heuristic, error-prone, but safe.  XXX

  direction = 0;
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
      //update = nmb_Interval (low_strip + 1, high_strip + 2);
      //mark = nmb_Interval (low_strip - 2, low_strip);
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

  nmb_Interval todo = last_marked - mark;

  // If update and todo are contiguous, combine them and do them
  // as a single interval;  otherwise, call build_list_set() twice

  if (spm_graphics_verbosity >= 15)
    fprintf(stderr, "  ");

  if (update.overlaps(todo) ||
      update.adjacent(todo)) {
    if (build_list_set(update + todo, planes, stripfn)) return -1;
  } else {
    if (build_list_set(update, planes, stripfn)) return -1;
    if (build_list_set(todo, planes, stripfn)) return -1;
  }

  if (spm_graphics_verbosity >= 15)
    fprintf(stderr, "\n");
  VERBOSECHECK(15)

  last_marked = mark;


  /* Draw grid using current viewing/modeling matrix */
  VERBOSECHECK(4);
  VERBOSE(4,"    Drawing the grid");
  TIMERVERBOSE(5, mytimer, "draw_world:Drawing the grid");

  if (g_texture_mode == GL_TEXTURE_2D) {
#ifdef __CYGWIN__
      glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, CYGWIN_TEXTURE_FUNCTION);
#else
      if ((g_texture_displayed == nmg_Graphics::RULERGRID) ||
          (g_texture_displayed == nmg_Graphics::REMOTE_DATA)) {
          glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
      } else {
	  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      }
#endif
  }
#ifndef __CYGWIN__
  if ((g_texture_mode == GL_TEXTURE_1D) ||
      (g_texture_mode == GL_TEXTURE_3D_EXT)) {
      glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
  }
#endif


  switch (g_texture_displayed) {
    case nmg_Graphics::NO_TEXTURES:
      // nothing to do here
      break;
    case nmg_Graphics::CONTOUR:
      glBindTexture(GL_TEXTURE_1D, tex_ids[CONTOUR_1D_TEX_ID]);
      break;
    case nmg_Graphics::ALPHA:
#ifndef __CYGWIN__
      glBindTexture(GL_TEXTURE_3D, tex_ids[ALPHA_3D_TEX_ID]);
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
      break;
    case nmg_Graphics::GENETIC:
      glBindTexture(GL_TEXTURE_2D, tex_ids[GENETIC_TEX_ID]);
      break;
    case nmg_Graphics::COLORMAP:
      glBindTexture(GL_TEXTURE_2D, tex_ids[COLORMAP_TEX_ID]);
      break;
    case nmg_Graphics::SEM_DATA:
      glBindTexture(GL_TEXTURE_2D, tex_ids[SEM_DATA_TEX_ID]);
      break;
    case nmg_Graphics::REMOTE_DATA:
      glBindTexture(GL_TEXTURE_2D, tex_ids[REMOTE_DATA_TEX_ID]);
      break;
    default:
      fprintf(stderr, "Error, unknown texture set for display\n");
      break;
  }

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
        GLdouble texture_matrix[16];
        compute_texture_matrix(g_translate_tex_x, g_translate_tex_y,
		g_tex_theta_cumulative, g_scale_tex_x,
		g_scale_tex_y, g_shear_tex_x, g_shear_tex_y,
		g_tex_coord_center_x, g_tex_coord_center_y,
		texture_matrix);
        glLoadMatrixd(texture_matrix);
        break;
      case nmg_Graphics::REGISTRATION_COORD:
        glLoadIdentity();
        // scale by actual texture image given divided by texture image used
        // (i.e. the one actually in texture memory is a power of 2 but the
        // one we were given had some smaller size)
        glScaled((double)g_tex_image_width/(double)g_tex_installed_width,
                (double)g_tex_image_height/(double)g_tex_installed_height,
                1.0);
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

  for (i = 0; i < num_grid_lists; i++) {
    glCallList(grid_list_base + i);
  }

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

  /*******************************************************/
  // Draw the parts of the scene other than the surface.
  /*******************************************************/

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

  if (g_draw_collab_hand) {
	make_collab_hand_icon(g_collabHandPos, g_collabHandQuat,
		g_collabMode);
	g_draw_collab_hand = 0;
  }	

#ifndef FLOW  
  /* Draw the light */
  setup_lighting(0);
#endif

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

#ifndef FLOW
  report_gl_errors();
#endif

  TIMERVERBOSE(3, mytimer, "end draw_world");

  return(0);
}





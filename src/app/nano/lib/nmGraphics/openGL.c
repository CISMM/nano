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
*
* 10/2001 Reorganized to include utility functions related to openGL state.
****************************************************************************/


#include	<math.h>	/* System includes */
#include	<stdio.h>
#include	<fcntl.h>
#include	<string.h>
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
#include	"nmg_State.h"

#include	"Timer.h"

/*Globals*/
extern int	do_y_fastest;		/* Tells which direction to scan in */
extern int	do_raster;	      /* Raster (vs. Boustrophedonic) scan? */

// default position of the light:  overhead, at infinity
static GLfloat l0_position [4] = { 0.0, 1.0, 0.1, 0.0 };

#ifdef MIN
#undef MIN
#endif
#define MIN(a,b) ((a)<(b)?(a):(b))
#ifdef MAX
#undef MAX
#endif
#define MAX(a,b) ((a)<(b)?(b):(a))

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

void determine_GL_capabilities(nmg_State * state) {
    
    int gl_1_1 = 0;
    
#ifdef GL_VERSION_1_1
    gl_1_1 = 1;
#endif
    
#if ( defined(linux) || defined(hpux) )
    // RMT The accelerated X server seems to be telling us that we have
    // vertex arrays, but then they are not drawn; ditto for MesaGL on
    // HPUX
    state->VERTEX_ARRAY = 0;
#else
    state->VERTEX_ARRAY = gl_1_1; //Vertex arrays are standard as of GL 1.1
    //state->VERTEX_ARRAY = 0;
#endif
}

//ADDED BY DANIEL ROHRER
#define MAXFONTS 2
int	font_array[MAXFONTS];

char	message[1000];		/* Message to display on screen */


#ifdef RENDERMAN
GLfloat cur_projection_matrix[16];
GLfloat cur_modelview_matrix[16];
#endif


int draw_world (int, void * data)
{
  nmg_State * state = (nmg_State *) data;
    
    v_gl_set_context_to_vlib_window(); 
    /********************************************************************/
    // Draw Ubergraphics
    /********************************************************************/
      
    //UGRAPHICS CALL TO DRAW THE WORLD  -- ASSUMING THAT VLIB HAS IT IN WORLD SPACE
    //WHEN I HIT THIS POINT

	// This is now called in renderSurface so that projective textures can be displayed
	// on the imported objects

//    if (state->config_enableUber) {
//        World.Do(&URender::Render);
//    }

    
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

    
    planes.lookup(state->inputGrid, state->heightPlaneName, state->colorPlaneName,
        state->contourPlaneName, state->opacityPlaneName,
        state->alphaPlaneName, state->maskPlaneName, state->transparentPlaneName,
        state->vizPlaneName);
    
    if (state->PRERENDERED_COLORS) {
        planes.lookupPrerenderedColors(state->prerendered_grid);
    }
    if (state->PRERENDERED_DEPTH) {
        planes.lookupPrerenderedDepth(state->prerendered_grid);
    }
    
    if (planes.height == NULL) {	
        return -1;
    }
    
    
    /********************************************************************/
    // Set up surface materials parameters for the surface, then draw it
    /********************************************************************/
    
    VERBOSECHECK(4);
    VERBOSE(4,"    Setting surface materials");
    
    /************************************************************
    * If the region has changed, we need to rebuild the display
    * lists for the surface.
    ************************************************************/
    
    // TODO  - add special cases for state->PRERENDERED_COLORS
    
    if (decoration->selectedRegion_changed) {
        
        VERBOSECHECK(4);
        VERBOSE(4,"    Rebuilding display lists (for new selected region)");
        if (!state->surface->rebuildSurface(state, VRPN_TRUE)) {
            fprintf(stderr,
                "ERROR: Could not build grid display lists\n");
            dataset->done = V_TRUE;
        }
        
        decoration->selectedRegion_changed = 0;
    }
    
    //See if there are any regions that need full rebuilding for
    //some reason
    if (!state->surface->rebuildSurface(state)) {
        fprintf(stderr,
            "ERROR: Could not build grid display lists\n");
        dataset->done = V_TRUE;
    }

    VERBOSECHECK(4);
    VERBOSE(4,"    Replacing changed display lists");
    TIMERVERBOSE(5, mytimer, "draw_world:Replacing changed display lists");
    
    // Convert from rows to strips:  divide through by the tesselation stride 
    if (!state->surface->rebuildInterval(state)) {
        return -1;
    }

    if (spm_graphics_verbosity >= 15)
        fprintf(stderr, "\n");
    VERBOSECHECK(15)
        
    /* Draw grid using current viewing/modeling matrix */
    VERBOSECHECK(4);
    VERBOSE(4,"    Drawing the grid");
    TIMERVERBOSE(5, mytimer, "draw_world:Drawing the grid");
    
    /* Draw the light */       
    setup_lighting(0, state);

    state->surface->renderSurface(state);

	setFilled(state);
    /*******************************************************/
    // Draw the parts of the scene other than the surface.
    /*******************************************************/
    
    // draw the microscope's current scanline as a visual indicator to the user
    // Not related to "scanline" mode, which controls the AFM tip. 
    if (decoration->drawScanLine && state->config_chartjunk) {
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
    
    if ((decoration->num_slow_line_3d_markers > 0) && state->config_chartjunk) {
        for (int i=0; i < decoration->num_slow_line_3d_markers; i++) {
            position_sphere( state, decoration->slowLine3dMarkers[i][0],
                decoration->slowLine3dMarkers[i][1],
                decoration->slowLine3dMarkers[i][2] );
            mysphere(NULL);
        }
    }
    
    // TCH 8 April 98 don't know where these go best
    if (decoration->red.changed()) {
        make_red_line(state,decoration->red.top(), decoration->red.bottom());
        decoration->red.clearChanged();
    }
    if (decoration->green.changed()) {
        make_green_line(state,decoration->green.top(), decoration->green.bottom());
        decoration->green.clearChanged();
    }
    if (decoration->blue.changed()) {
        make_blue_line(state,decoration->blue.top(), decoration->blue.bottom());
        decoration->blue.clearChanged();
    }

    if (decoration->aimLine.changed()) {
        make_aim(decoration->aimLine.top(), decoration->aimLine.bottom());
        decoration->aimLine.clearChanged();
    }
    
    if (decoration->trueTipLocation_changed) {
        state->trueTipLocation[0] = decoration->trueTipLocation[0];
        state->trueTipLocation[1] = decoration->trueTipLocation[1];
        state->trueTipLocation[2] = decoration->trueTipLocation[2];
        if (spm_graphics_verbosity >= 12)
            fprintf(stderr, "Setting true tip location to (%.2f %.2f %.2f).\n",
            state->trueTipLocation[0], state->trueTipLocation[1],
            state->trueTipLocation[2]);
        decoration->trueTipLocation_changed = 0;
    }
    
    if (state->position_collab_hand) {
        make_collab_hand_icon(state->collabHandPos, state->collabHandQuat,
            state->collabMode);
        state->position_collab_hand = 0;
    }    
    
    // Set the lighting model for the measurement things
    VERBOSECHECK(4);
    VERBOSE(4,"    Setting measurement materials");
    set_gl_measure_materials(state);
    
    /* Draw the pulse indicators */
    glColor3f(1.0f, 0.3f, 0.3f);
    decoration->traverseVisiblePulses(spm_render_mark, NULL);
    /* Draw the scrape indicators */
    decoration->traverseVisibleScrapes(spm_render_mark, NULL);
    
    VERBOSE(4,"    Drawing the world");
    TIMERVERBOSE(5, mytimer, "draw_world:Drawing the world");
    
    myworld(state);
    
    /***************************/
    /* Check for any GL errors */
    /***************************/
    report_gl_errors();
    
    TIMERVERBOSE(3, mytimer, "end draw_world");
    
    return(0);
}





/** Sets material properties that never change. */
void setupMaterials (void) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_COLOR_MATERIAL);
    /* Use local vertex color for ambient and diffuse */
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    /* Counter-clockwise is forward for the surface and all objects. */
    glFrontFace(GL_CCW);
    
}


/**	This routine will set up the material properties so that the
* surface will appear to be made of shiny plastic. */

void set_gl_surface_materials(nmg_State * state)
{	
    GLfloat	specular[4] = { (float)state->specular_color,
        (float)state->specular_color,
        (float)state->specular_color, 1.0 };
    GLfloat	dark[4] = { 0.0, 0.0, 0.0, 1.0 };
    
    
    //fprintf(stderr, "In set_gl_surface_materials with texture mode %d.\n",
    //state->texture_mode);
    TIMERVERBOSE(5, mytimer, "begin set_gl_surface_materials");
    
    // Set up the specular characteristics.
    // Note that the ambient and diffuse colors are from the vertices.
    // NOTE: It is important that back is set first because front/back
    //       is ignored in an early implementation of FLOW, and it always
    //       set both.
    glMaterialfv(GL_BACK, GL_SPECULAR, dark);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT, GL_SHININESS, state->shiny);
    
    // Set the light model to have completely ambient-off.  There is
    // ambient specified in light 0.
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, dark);
    
    /* Set a color, in case the color is not being adjusted per vertex. */
    /* Use the surface color for this. */
    
    state->surfaceColor[3] = state->surface_alpha; //make sure alpha value is updated
    
    glColor4dv(state->surfaceColor);
    
    // Turn texture mapping to the appropriate state
    // Note that the Enable has to be the last one, after all the
    // Disable calls.
    switch (state->texture_mode) {
    case GL_FALSE:
        glDisable(GL_TEXTURE_1D);
        glDisable(GL_TEXTURE_2D);
#ifdef	sgi
        glDisable(GL_TEXTURE_3D_EXT);
#endif
        break;
        
    case GL_TEXTURE_1D:
        glDisable(GL_TEXTURE_2D);
#ifdef	sgi
        glDisable(GL_TEXTURE_3D_EXT);
#endif
        glEnable(GL_TEXTURE_1D);
        break;
        
    case GL_TEXTURE_2D:
        glDisable(GL_TEXTURE_1D);
#ifdef	sgi
        glDisable(GL_TEXTURE_3D_EXT);
#endif
        glEnable(GL_TEXTURE_2D);
        break;
        
#ifdef	sgi
    case GL_TEXTURE_3D_EXT:
        glDisable(GL_TEXTURE_1D);
        glDisable(GL_TEXTURE_2D);
        glEnable(GL_TEXTURE_3D_EXT);
        break;
#endif
        
    default:
        if (spm_graphics_verbosity > 3)
            fprintf(stderr, "set_gl_surface_materials:  "
            "texture_mode %i\n", state->texture_mode);
        break;
    }
    if (spm_graphics_verbosity > 3)
        fprintf(stderr, "set_gl_surface_materials:  "
        "texture_mode %i\n", state->texture_mode);
    
    TIMERVERBOSE(5, mytimer, "end set_gl_surface_materials");
}

/**	This routine will set up the material properties so that the
* icons and such will appear to be made of shiny plastic, and will react
* to specular and diffuse lighting, but will never be textured. */

void    set_gl_icon_materials(nmg_State * state)
{	
    GLfloat	specular[4] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat	dark[4] = { 0.0, 0.0, 0.0, 1.0 };
    
    VERBOSE(5, "    Entering set_gl_icon_materials()");
    TIMERVERBOSE(5, mytimer, "begin set_gl_icon_materials");
    
    // Set up the specular characteristics.
    // Note that the ambient and diffuse colors are from the vertices.
    // NOTE: It is important that back is set first because front/back
    //       is ignored in an early implementation of FLOW, and it always
    //       set both.
    glMaterialfv(GL_BACK, GL_SPECULAR, dark);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT, GL_SHININESS, state->shiny);
    
    // Set the light model to have completely ambient-off.  There is
    // ambient specified in light 0.
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, dark);
    
    /* Set a color, in case the color is not being adjusted per vertex. */
    /* Use the surface color for this. */
    state->surfaceColor[3] = state->surface_alpha; //make sure alpha value is updated
    glColor4dv(state->surfaceColor);
    
    // Disable texture-mapping.
    glDisable(GL_TEXTURE_1D);
    glDisable(GL_TEXTURE_2D);
#ifdef	sgi
    glDisable(GL_TEXTURE_3D_EXT);
#endif
    
    TIMERVERBOSE(5, mytimer, "end set_gl_icon_materials");
}

/**	This routine will set up the material properties so that the
* measurement tools (lines and text) will not depend on the lighting
* model or be texture-mapped.  This is done by setting the ambient
* coefficients to 1 and the diffuse/specular ones to 0.  */

void    set_gl_measure_materials(nmg_State * state)
{	
    GLfloat	bright[4] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat	dark[4] = { 0.0, 0.0, 0.0, 1.0 };
    
    VERBOSE(5, "    Entering set_gl_measure_materials()");
    TIMERVERBOSE(5, mytimer, "begin set_gl_measure_materials");
    
    // Set up the specular characteristics.
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, dark);
    
    TIMERVERBOSE(7, mytimer, "set_gl_measure_materials: end glMaterialfv");
    
    // Set the light model to have completely ambient-on.
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, bright);
    
    TIMERVERBOSE(7, mytimer, "set_gl_measure_materials: end glLightModelfv");
    
    /* Set a color, in case the color is not being adjusted per vertex. */
    /* Use the surface color for this. */
    state->surfaceColor[3] = state->surface_alpha; //Make sure alpha value is updated
    glColor4dv(state->surfaceColor);
    
    TIMERVERBOSE(7, mytimer, "set_gl_measure_materials: end glColor3dv");
    
    // Disable texture-mapping.
    glDisable(GL_TEXTURE_1D);
    glDisable(GL_TEXTURE_2D);
#ifdef	sgi
    glDisable(GL_TEXTURE_3D_EXT);
#endif
    
    TIMERVERBOSE(5, mytimer, "end set_gl_measure_materials");
}

//---------------------------------------------------------------------------
// This routine sets up the lighting and some of the surface material
// properties.  It should be the one used in both openGL and PixelFlow.
// Some effort should be made to bring all of the material parameters
// together here.

int setup_lighting (int, void * data)
{
    static	int	was_smooth_shading = -1;
  nmg_State * state = (nmg_State *) data;

    GLfloat l0_ambient[4] = { 0.2, 0.2, 0.2, 1.0 };
//    GLfloat l0_diffuse[4] = { 0.4, 0.4, 0.4, 1.0 };
    GLfloat l0_diffuse[4] = { state->diffuse, state->diffuse, state->diffuse, 1.0 };
/*     GLfloat l0_specular[4] = { 0.4, 0.4, 0.4, 1.0 }; */
    GLfloat l0_specular[4] = { 0.2, 0.2, 0.2, 1.0 };
    // l0_position defined at the top of this file as a global variable

    // make sure gl calls are directed to the right context
    v_gl_set_context_to_vlib_window();

    glLightfv(GL_LIGHT0, GL_AMBIENT, l0_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, l0_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, l0_specular);
    glPushMatrix();
    glLoadIdentity();
    glLightfv(GL_LIGHT0, GL_POSITION, l0_position);
    glPopMatrix();
    glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 0.0);
    glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 180.0);
    glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 1.0);
    glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.0);
    glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.0);

    if (state->config_smooth_shading != was_smooth_shading) {
	was_smooth_shading = state->config_smooth_shading;
	if (state->config_smooth_shading) {
	    glShadeModel(GL_SMOOTH);	/* Gouraud shading */
	} else {
	    glShadeModel(GL_FLAT);	/* Flat shading */
	}
    }

  if (!state->PRERENDERED_COLORS && !state->PRERENDERED_TEXTURE) {
    // With prerendered colors we don't have any normals;  this REALLY
    // slows us down, since GL_NORMALIZE special-cases that.
    glEnable(GL_NORMALIZE);                 /* Re-Normalize normals */
  }

  // No default ambient lighting other than specified in the light
  {       
	GLfloat global_ambient[4] = { 0.0, 0.0, 0.0, 1.0 };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);
  }

  // Local viewer is slower, and creates a highlight It's more realistic, but
  // can hide features outside the highlight and possibly cause
  // mis-interpretation of bumps.
  if (state->local_viewer) {
      glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
  } else {
      glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_FALSE);
  }

  // 2 sided lighting causes black lines to show through the surface on Nvidia
  // Quadro2Pro, and it's probably slower, anyway.
  //glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
  glEnable(GL_LIGHT0);

  // Is this reasonable?  TCH 10 Jan 99
  if (state->PRERENDERED_COLORS || state->PRERENDERED_TEXTURE) {
    glDisable(GL_LIGHTING);
  } else {
    glEnable(GL_LIGHTING);
  }

    return 0;
}

void setLightDirection (const q_vec_type & newValue) {
  l0_position[0] = newValue[0];
  l0_position[1] = newValue[1];
  l0_position[2] = newValue[2];
}

void getLightDirection (q_vec_type * v) {
  (*v)[0] = l0_position[0];
  (*v)[1] = l0_position[1];
  (*v)[2] = l0_position[2];
}

// Put the light back where it was when the program started.

void resetLightDirection (void) {
  l0_position[0] = 0.0;
  l0_position[1] = 1.0;
  l0_position[2] = 0.1;
  l0_position[3] = 0.0;
}

void getViewportSize (nmg_State * state, int * width, int * height) {
  *width  = v_display_table[state->displayIndexList[0]].viewports[0].fbExtents[0];
  *height = v_display_table[state->displayIndexList[0]].viewports[0].fbExtents[1];
}


void setFilled(nmg_State * state)
{
    if (state->config_filled_polygons) {
	    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    } else {
	    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
}


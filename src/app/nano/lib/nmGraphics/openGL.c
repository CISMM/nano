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

#include "nmg_haptic_graphics.h"


/*Globals*/
extern int	do_y_fastest;		/* Tells which direction to scan in */
extern int	do_raster;	      /* Raster (vs. Boustrophedonic) scan? */

//JM TEMP
extern int config_feelPlane_temp;
extern int config_feelGrid_temp;

// default position of the light:  overhead, at infinity
static GLfloat l0_position [4] = { 0.0, 1.0, 0.1, 0.0 };

// extra data for video capture functionality
extern unsigned char * read_buffer;
static unsigned int read_frame = 0;
extern int save_frame = 0;
extern PAVISTREAM pStream;
extern unsigned int save_width, save_height;

extern void handle_videoCaptureEnd_change(int, void *);


#ifdef MIN
#undef MIN
#endif
#define MIN(a,b) ((a)<(b)?(a):(b))
#ifdef MAX
#undef MAX
#endif
#define MAX(a,b) ((a)<(b)?(b):(a))

#define VERBOSECHECK(level)	if (spm_graphics_verbosity >= level) report_gl_errors();

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


int draw_world (int, void * data)
{
  nmg_State * state = (nmg_State *) data;
  nmg_haptic_graphics * haptic_graphics = (nmg_haptic_graphics *) data;
    
    v_gl_set_context_to_vlib_window(); 
    // Draw Ubergraphics
	if (state->texture_transform_mode == nmg_Graphics::MODEL_REGISTRATION_COORD) {
		// we need to compute this transform for every rendering loop because
		// there is no callback for the action that modifies the state on which
		// it depends (the position of a special texture-positioning 
		// Ubergraphics object)
		computeModelRegistrationTextureTransform(state, 
			state->modelModeTextureTransform);
		World.Do(&URender::SetTextureTransformAll, 
			(void *)state->modelModeTextureTransform);
	} else {
		// otherwise we use the transformation set by the registration
		// code
		World.Do(&URender::SetTextureTransformAll, 
			(void *)state->surfaceModeTextureTransform);
	}
	// we need to set this each rendering loop because otherwise when a
	// new ubergraphics object is added then it won't get the current
	// texture state until that state changes
	World.Do(&URender::SetProjTextureAll, state->currentProjectiveTexture);

    if (state->config_enableUber) {
        World.Do(&URender::Render);
    }

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
        if (state->surface->rebuildSurface(state)) {
            fprintf(stderr,
                "ERROR: Could not build grid display lists\n");
            dataset->done = V_TRUE;
        }
        decoration->red.normalize(planes.height);
        decoration->green.normalize(planes.height);
        decoration->blue.normalize(planes.height);
        decoration->aimLine.normalize(planes.height);
        
        decoration->selectedRegion_changed = 0;
    }
    
    VERBOSECHECK(4);
    VERBOSE(4,"    Replacing changed display lists");
    TIMERVERBOSE(5, mytimer, "draw_world:Replacing changed display lists");
    
    //See if there are any regions that need rebuilding for
    //any reason
    if (state->surface->rebuildInterval(state)) {
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
    if (decoration->drawScanLine && state->config_chartjunk && decoration->scanLineCount) {
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
	//if we are drawing haptic graphics

    //if(haptic_graphics->get_show_feel_plane() ) {
    if(config_feelPlane_temp) {
           make_feelPlane(state);
    }

    if(config_feelGrid_temp) {
        make_feelGrid(state);
    }



	//if we are drawing the axis on the sphere for direct step...
	if(decoration-> ds_sphere_axis) {

		UTree *node;
		node = World.TGetNodeByName(*World.current_object);
		URender &obj = node->TGetContents();
		
		make_ds_sphere_axis(state, obj.GetLocalXform().GetRot() );
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

	/* Extra code to read back the window for writing to a video file. */
	read_frame = (read_frame + 1) % 4;
	if (save_frame && read_frame == 0) {
		GLint viewport[4];
		glFlush();
		//glReadBuffer(GL_BACK);
		glGetIntegerv(GL_VIEWPORT, viewport);
		//printf("debug %u x %u + %u x %u\n", viewport[2], viewport[3], viewport[0], viewport[1]);
		int width = viewport[2] & 0xfffffffc;
		int height = viewport[3] & 0xfffffffc;
		//if (width & 3) {
		//	width >>= 2;
		//	width++;
		//	width <<= 2;
		//}
		if (read_buffer == NULL)
			printf("NULL BUFFER!!\n");
		//if (viewport[0] != 0 || viewport[1] != 0)
		//	printf("starting at %i x %i\n", viewport[0], viewport[1]);
		if (width == save_width && height == save_height) {
			glReadPixels(viewport[0], viewport[1], width, height, /*GL_RGB*/ GL_BGR_EXT, GL_UNSIGNED_BYTE, read_buffer);
			//printf("width: %u\n", width);
			//imdebug("bgr w=%d h=%d %p", width, viewport[3] - viewport[1], read_buffer);
			AppendFrameToAVI(width, height, read_buffer);
		} else {
			printf("Window resized. Ending video capture.\n");
			handle_videoCaptureEnd_change(0, NULL);
		}
	}

    /***************************/
    /* Check for any GL errors */
    /***************************/
    report_gl_errors();
    
    TIMERVERBOSE(3, mytimer, "end draw_world");
    
    return(0);
}


// Link against vfw32.lib
#pragma comment( lib, "vfw32" )


int AppendFrameToAVI(unsigned int a_nWidth, unsigned int a_nHeight, unsigned char *a_pImage)
{
    //BITMAPFILEHEADER bmpFileHeader;
    //FILE *filep;
    //unsigned int row, column;
    //unsigned char *paddedImage = NULL, *paddedImagePtr, *imagePtr;
	//unsigned int a_nWidth = 320, a_nHeight = 240;
	int a_nFrameRate = 15;

    /* The .bmp format requires that the image data is aligned on a 4 byte boundary.  For 24 - bit bitmaps,
       this means that the width of the bitmap  * 3 must be a multiple of 4. This code determines
       the extra padding needed to meet this requirement. */

	/// CD: This is for writing BMP files (commented out for AVI usage)
    // Fill the bitmap file header structure
    //bmpFileHeader.bfType = 'MB';   // Bitmap header
    //bmpFileHeader.bfSize = 0;      // This can be 0 for BI_RGB bitmaps
    //bmpFileHeader.bfReserved1 = 0;
    //bmpFileHeader.bfReserved2 = 0;
    //bmpFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);



	/* kent: removed the copy for this proof-of-concept portion
	   because OpenGL will do the alignment and BGR thing for us
	
	   Also, using calloc() in a video-capture setting?  UGH!!! */
    // Allocate memory for some temporary storage
    //paddedImage = (unsigned char *)calloc(sizeof(unsigned char), bytesize);
    //if (paddedImage == NULL) {
    //        printf("Error allocating memory \n");
	//      //fclose(filep);
    //        return FALSE;
    //}


    /* This code does three things.  First, it flips the image data upside down, as the .bmp
       format requires an upside down image.  Second, it pads the image data with extrabytes 
            number of bytes so that the width in bytes of the image data that is written to the
            file is a multiple of 4.  Finally, it swaps (r, g, b) for (b, g, r).  This is another
            quirk of the .bmp file format. */

    //for (row = 0; row < a_nHeight; row++) {
            
	//		/// CD: It turns out that I didn't need this line.  I replaced it with the next.
	//		//imagePtr = image + (height - 1 - row) * width * 3;				
	//		imagePtr = a_pImage + (row * a_nWidth * 3);

    //        paddedImagePtr = paddedImage + row * (a_nWidth * 3 + extrabytes);
    //        for (column = 0; column < a_nWidth; column++) {
    //                *paddedImagePtr = *(imagePtr + 2);
    //                *(paddedImagePtr + 1) = *(imagePtr + 1);
    //                *(paddedImagePtr + 2) = *imagePtr;
    //                imagePtr += 3;
    //                paddedImagePtr += 3;
    //        }
    //}

	// Append the bitmap to the stream...
	AVIStreamWrite(pStream, max(0, AVIStreamEnd(pStream)), 1, a_pImage/*paddedImage*/, a_nWidth * 3 * a_nHeight, AVIIF_KEYFRAME, NULL, NULL);

    //free(paddedImage);
    return TRUE;
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

void computeModelRegistrationTextureTransform(nmg_State * state, double *matrix)
{
	UTree *node;
	node = World.TGetNodeByName("projtextobj.ptx");
	if (node != NULL) {

		nmb_TransformMatrix44 transform;

		URender &obj = node->TGetContents();

		// here, we use the associated object's transform.  However, we must invert everything
		// for the texture's transform...

		q_vec_type v;
		q_type q;
		qogl_matrix_type mat;

		q_vec_copy(v, obj.GetLocalXform().GetTrans());
		q_vec_scale(v, -1.0, v);

		q_copy(q, obj.GetLocalXform().GetRot());
		q_invert(q, q);

        transform.translate(0.5, 0.5, 0.0);

        double plane_width = state->inputGrid->maxX() - state->inputGrid->minX();
        double plane_height = state->inputGrid->maxY() - state->inputGrid->minY();
	    transform.scale(1 / plane_width, 1 / plane_height, 1.0);

        // scale to correct ratio in x and y
        int image_width = state->currentProjectiveTexture->width();
        int image_height = state->currentProjectiveTexture->height();
        if (image_width != 0 && image_height != 0) {
            double scale;
            if (image_width > image_height) {
                scale = (double)image_width / (double)image_height;
                transform.scale(1.0, scale, 1.0);
            }
            else if (image_height > image_width) {
                scale = (double)image_height / (double)image_width;
                transform.scale(scale, 1.0, 1.0);
            }
        }

		q_to_ogl_matrix(mat, q);
		transform.compose(mat);

		transform.translate(v[0], v[1], v[2]);
		
		transform.getMatrix(matrix);
	}
}

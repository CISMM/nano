#ifndef	OPENGL_H
#define	OPENGL_H

class nmg_State;
class nmg_haptic_graphics;
void determine_GL_capabilities(nmg_State * state);

extern int report_gl_errors(void);

extern void post_handle_exportFileName_change(void);

#if defined (sgi) || defined(linux)
	extern int draw_world (int, void *);
#else
	extern "C" int draw_world(int, void *);
#endif

extern void setupMaterials (void);  // for initialization

extern	void	set_gl_surface_materials (nmg_State * state);
extern	void	set_gl_icon_materials (nmg_State * state);
extern	void	set_gl_measure_materials (nmg_State * state);


int setup_lighting (int nothing, void * data);
void setLightDirection (const q_vec_type &);
void getLightDirection (q_vec_type *);
void resetLightDirection (void);

void getViewportSize (nmg_State * state, int * width, int * height);

void setFilled(nmg_State * state);

void computeModelRegistrationTextureTransform(nmg_State * state, double *matrix);


// int AppendFrameToAVI(char *a_sFilename, unsigned int a_nWidth, unsigned int a_nHeight, unsigned char *a_pImage, int a_nFrameRate)
//
// Credits:
// The BMP file portion of this code was taken from somewhere, and I've forgotten where -  <deepest apologies to the author>
//
// The remainder of the code appends to the AVI... it's pretty straightforward...

// USAGE:	a_sFilename is a pointer to the AVI file
//			a_nWidth and a_nHeight are the dimensions of an RGB (24 bpp) image
//			a_pImage is the image pointer
//			a_nFrameRate is the AVI frame rate (the added frame will be displayed for 1/a_nFrameRate seconds... or something like that)

#include <vfw.h>


int AppendFrameToAVI(unsigned int a_nWidth, unsigned int a_nHeight, unsigned char *a_pImage);

#endif	/* OPENGL_H */

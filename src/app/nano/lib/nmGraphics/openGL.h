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

#endif	/* OPENGL_H */

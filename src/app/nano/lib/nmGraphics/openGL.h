#ifndef	OPENGL_H
#define	OPENGL_H


void determine_GL_capabilities();

extern int report_gl_errors(void);

extern void post_handle_exportFileName_change(void);

#if defined (sgi) || defined(linux)
	extern int draw_world (int);
#else
	extern "C" int draw_world(int);
#endif

extern void setupMaterials (void);  // for initialization

extern	void	set_gl_surface_materials (void);
extern	void	set_gl_icon_materials (void);
extern	void	set_gl_measure_materials (void);

#endif	/* OPENGL_H */

#ifndef	OPENGL_H
#define	OPENGL_H

#ifndef NMB_PLANE_SELECTION_H
#include <nmb_PlaneSelection.h>
#endif

extern int build_grid_display_lists (nmb_PlaneSelection planes,
                                     int strips_in_x,
                                     GLuint * base, GLsizei * num,
                                     GLdouble * minColor,
                                     GLdouble * maxColor);

extern int check_extension (const GLubyte * exten_str);

extern int display_lists_in_x;
extern GLuint grid_list_base;
extern GLsizei num_grid_lists;

extern int report_gl_errors(void);

#if defined(FLOW) || defined (sgi) || defined(linux)
	extern int draw_world (int);
#else
	extern "C" int draw_world(int);
#endif

#endif	/* OPENGL_H */

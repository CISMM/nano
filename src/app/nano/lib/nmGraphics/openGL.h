#ifndef	OPENGL_H
#define	OPENGL_H

#ifndef NMB_PLANE_SELECTION_H
#include <nmb_PlaneSelection.h>
#endif

class nmb_Interval;
class nmg_SurfaceMask;
struct Vertex_Struct;

extern int build_grid_display_lists (const nmb_PlaneSelection &planes, nmg_SurfaceMask *mask,
                                     int strips_in_x,
                                     GLuint * base, GLsizei * num, GLsizei old_num,
                                     GLdouble * minColor,
                                     GLdouble * maxColor,
                                     Vertex_Struct **surface);

extern int build_list_set (const nmb_Interval &insubset,
                           const nmb_PlaneSelection &planes, nmg_SurfaceMask *mask,
                           GLuint base, GLsizei num,
                           int strips_in_x,
                           Vertex_Struct **surface);

void determine_GL_capabilities();

extern int display_lists_in_x;
//extern GLuint grid_list_base;
//extern GLsizei num_grid_lists;

extern int report_gl_errors(void);

extern void post_handle_exportFileName_change(void);

#if defined (sgi) || defined(linux)
	extern int draw_world (int);
#else
	extern "C" int draw_world(int);
#endif

#endif	/* OPENGL_H */

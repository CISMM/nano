/*      This structure holds information about a pulse that is used to
 * display a marker at the pulse location. */

#ifndef	SURFACE_STRIP_CREATE_H
#define	SURFACE_STRIP_CREATE_H

#ifndef NMB_PLANE_SELECTION_H
#include <nmb_PlaneSelection.h>
#endif

class BCGrid;  // from BCGrid.h
class nmg_SurfaceMask;

// structure for vertex array
struct Vertex_Struct;


extern int spm_x_strip (const nmb_PlaneSelection &planes,
                        GLdouble surfaceColor [3], int which,
                        Vertex_Struct * vertexArray);
extern int spm_x_strip_masked (const nmb_PlaneSelection &planes, 
                               nmg_SurfaceMask *mask,
                               GLdouble surfaceColor [3], int which,
                               Vertex_Struct * vertexArray);
extern int spm_y_strip (const nmb_PlaneSelection &planes,
                        GLdouble surfaceColor [3], int which,
                        Vertex_Struct * vertexArray);
extern int spm_y_strip_masked (const nmb_PlaneSelection &planes, 
                               nmg_SurfaceMask *mask,
                               GLdouble surfaceColor [3], int which,
                               Vertex_Struct * vertexArray);
extern	int	spm_grid_to_tris (BCGrid *, GLdouble *, GLdouble *, int);
//extern	void	spm_show_location_indicators (int total_num,
//                     nmb_LocationInfo [], int num_to_show, int last_one);
#endif  /* SPM_GL_H */
















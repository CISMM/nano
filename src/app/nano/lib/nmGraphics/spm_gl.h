/*      This structure holds information about a pulse that is used to
 * display a marker at the pulse location. */

#ifndef	SPM_GL_H
#define	SPM_GL_H

//#include	"microscape.h"

#ifndef NMB_PLANE_SELECTION_H
#include <nmb_PlaneSelection.h>
#endif
#ifndef NMB_DECORATION_H
#include <nmb_Decoration.h>  // for nmb_LocationInfo
#endif

class BCGrid;  // from BCGrid.h
class nmg_SurfaceMask;

// MOVED to nmb_Decoration
//#define MAX_LOCATION_INFOS (2000)
//struct LOCATION_INFO {

// structure for vertex array
struct Vertex_Struct {
        GLfloat Texcoord [3];
        GLubyte Color [4];
        GLshort Normal [3];
        GLfloat Vertex [3];
};



// MOVED to nmb_PlaneSelection.[Ch]
// class PLANE_SELECTION

// MOVED to microscape.h
// Why was it here in the first place?
//extern int read_mode;

//Moved these to the visualization classes
//extern Vertex_Struct **vertexptr;
//extern  int     init_vertexArray(int x, int y);


//No longer called outside of spm_gl
//extern  void    specify_vertexArray(int i, int start, int count);

// pcfl shader ids
#ifdef FLOW
extern GLuint nM_diffuse;
extern GLuint nM_shader;
#endif

extern	int	spm_x_strip (nmb_PlaneSelection planes,
						 GLdouble minColor [3], GLdouble maxColor [3], int which,
						 Vertex_Struct * vertexArray);
extern	int	spm_x_strip_masked (nmb_PlaneSelection planes, nmg_SurfaceMask *mask,
								GLdouble minColor [3], GLdouble maxColor [3], int which,
								Vertex_Struct * vertexArray);
extern	int	spm_y_strip (nmb_PlaneSelection planes,
						 GLdouble minColor [3], GLdouble maxColor [3], int which,
						 Vertex_Struct * vertexArray);
extern	int	spm_y_strip_masked (nmb_PlaneSelection planes, nmg_SurfaceMask *mask,
								GLdouble minColor [3], GLdouble maxColor [3], int which,
								Vertex_Struct * vertexArray);
extern	int	spm_grid_to_tris (BCGrid *, GLdouble *, GLdouble *, int);
extern	void	spm_set_surface_materials (void);
extern	void	spm_set_icon_materials (void);
extern	void	spm_set_measure_materials (void);
//extern	void	spm_show_location_indicators (int total_num,
//                     nmb_LocationInfo [], int num_to_show, int last_one);
extern int spm_render_mark (const nmb_LocationInfo &, void *);

#endif  /* SPM_GL_H */
















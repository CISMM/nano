/*
 *				spm_gl.c
 *
 *	This file contains routines to make openGL structures out of
 * information obtained from the Scanning Probe Microscope.
 *	The routines here take grid files and create a structure whose
 * x,y,z values match those specified in the grid.  There is no scaling,
 * either uniform or of Z relative to X and Y, to distort the coordinates
 * present in the grid file.  All scaling or other transformations should
 * be set up before calling these routines.
 */
#ifdef _WIN32
#include        <windows.h>  // This must be included before <GL/gl.h>
#endif
#include	<GL/gl.h>
#include	<stdio.h>
#include	<string.h>
#include	<stdlib.h>  // malloc
#include	<math.h>

#include <colormap.h>
#include <nmb_PlaneSelection.h>

#include "spm_gl.h"
//#include "Tcl_Linkvar.h"
#include "graphics_globals.h"
#include "BCPlane.h"
#include "Timer.h"

#if (!defined(X) || !defined(Y) || !defined(Z))
#define	X	(0)
#define	Y	(1)
#define	Z	(2)
#endif

#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef max
#define max(a,b) ((a)<(b)?(b):(a))
#endif

Vertex_Struct ** vertexptr = NULL;



//---------------------------------------------------------------------------
// Vector utility routines.

#if 0
inline	void	vector_cross(GLfloat a[3], GLfloat b[3], GLfloat c[3])
{
    c[0] = a[1]*b[2] - b[1]*a[2];
    c[1] = -(a[0]*b[2] - b[0]*a[2]);
    c[2] = a[0]*b[1] - b[0]*a[1];
}
#endif  // 0

// TCH 15 May 98
// Speed up stm_compute_plane_normal() by specilizing cross-product routines,
// knowing that we're multiplying by axis-aligned unit vectors.
// Gives us about a 10% speedup.
inline void vector_cross_X (GLfloat a[3], GLfloat c[3]) {
  c[0] = 0.0f;
  c[1] = a[2];
  c[2] = -a[1];
}

inline void vector_cross_Y (GLfloat a[3], GLfloat c[3]) {
  c[0] = -a[2];
  c[1] = 0.0f;
  c[2] = a[0];
}

inline void vector_cross_NX (GLfloat a[3], GLfloat c[3]) {
  c[0] = 0.0f;
  c[1] = -a[2];
  c[2] = a[1];
}

inline void vector_cross_NY (GLfloat a[3], GLfloat c[3]) {
  c[0] = a[2];
  c[1] = 0.0f;
  c[2] = -a[0];
}


inline	void	vector_add(GLfloat a[3], GLfloat b[3], GLfloat c[3])
{
    c[0] = a[0] + b[0];     
    c[1] = a[1] + b[1];     
    c[2] = a[2] + b[2];
}

inline	void	vector_normalize(GLfloat a[3])
{
    double mag;   

    mag = sqrt(a[0] * a[0] + a[1]*a[1] + a[2]*a[2]); 

    if (mag == 0.0) {
        fprintf(stderr,"pg_vector_normalize:  vector has zero magnitude\n");
        a[0] = a[1] = a[2] = 1.0/sqrt(3.0);
    } else {
        mag = 1.0/mag;
        a[0] *= mag;
        a[1] *= mag;
        a[2] *= mag;
    }
}


/*	This routine finds the normal to the surface at the given grid
 * point.  It does this by averaging the normals with the four 4-connected
 * points in the grid (one away in x or y.)
 *	This routine takes into account the current stride in x and y
 * between tesselated points and grid points.
 *	This routine returns 0 on success and -1 on failure. */

int	stm_compute_plane_normal(BCPlane *plane, int x,int y,
			     double dx,double dy,double dz, GLfloat Normal[3])
{
	GLfloat		diff_vec[3];
	GLfloat		local_norm[3];
	int	i;

	// Valid points must be within the grid and lie exactly on a stride
	// point.
	if ( (x < 0) || (x > plane->numX()-1) ||
	     (y < 0) || (y > plane->numY()-1) ||
	     (x % g_stride) || (y % g_stride) ){
		return(-1);
	}

	/* Initially, clear the normal */
	for (i = 0; i < 3; i++) {
		Normal[i] = 0.0;
	}

	// Find the normal with stride more in x, if it is within the grid

	if ( (x+g_stride) < plane->numX()) {
		diff_vec[X] = dx * g_stride;
		diff_vec[Y] = 0;
		diff_vec[Z] = dz * (float) (plane->valueInWorld(x+g_stride,y) -
					    plane->valueInWorld(x, y));
		vector_cross_Y(diff_vec, local_norm);
		vector_add(local_norm,Normal, Normal);
	}

	// Find the normal with stride more in y, if it is within the grid

	if ( (y+g_stride) < plane->numY()) {
		diff_vec[X] = 0;
		diff_vec[Y] = dy * g_stride;
		diff_vec[Z] = (float) (dz * (plane->valueInWorld(x,y+g_stride) -
					     plane->valueInWorld(x,y)));
		vector_cross_NX(diff_vec, local_norm);
		vector_add(local_norm,Normal, Normal);
	}
 
	// Find the normal with stride less in x, if it is within the grid

	if ( (x-g_stride) >= 0) {
		diff_vec[X] = -dx * g_stride;
		diff_vec[Y] = 0;
		diff_vec[Z] = (float) (dz * (plane->valueInWorld(x-g_stride,y) -
					     plane->valueInWorld(x, y)));
		vector_cross_NY(diff_vec, local_norm);
		vector_add(local_norm,Normal, Normal);
	}

	// Find the normal with stride less in y, if it is within the grid

	if ( (y-g_stride) >= 0) {
		diff_vec[X] = 0;
		diff_vec[Y] = -dy * g_stride;
		diff_vec[Z] = (float) (dz * (plane->valueInWorld(x,y-g_stride) -
					     plane->valueInWorld(x, y)));
		vector_cross_X(diff_vec, local_norm);
		vector_add(local_norm,Normal, Normal);
	}

	/* Normalize the normal */

	vector_normalize(Normal);
	return(0);
}




/*	This routine will create the openGL commands needed to display a
 * triangle strip for one of the strips needed to show a grid.  This routine
 * displays a strip along the X axis;  the "which" parameter selects from the
 * strips to be drawn.  The first strip is 0 and the last is grid->num_y - 1,
 * when the stride is 1.  The stride determines the number of points to move
 * in both X and Y to get to the next grid point to use.  If the stride is
 * not an integer factor of the length, the last partial square is ignored.
 *	This routine should only be called to generate a strip for an integer
 * multiple of stride.
 *	The triangle strips are mapped from the grid into the graphics space
 * as follows:
 *                                                                         
 *         0     2     4     6     8                                       
 *          +-----+-----+-----+-----+                               
 *          |    /|    /|    /|    /|                   
 *    ^     |   / |   / |   / |   / |   (Counter-clockwise face forward)
 *    |     |  /  |  /  |  /  |  /  |                         
 *    |     | /   | /   | /   | /   |                            
 *    |     |/    |/    |/    |/    |                               
 *    |     +-----+-----+-----+-----+                               
 *    Y    1     3     5     7     9 
 *     X-------->                           
 *                                                                         
 *	This routine returns 0 on success and -1 on failure. */

int init_vertexArray(int x, int y)
{  int dim;

  if(x<=y) {
     dim=y;
  }
  else {
     dim=x;
  }       

  // with RenderClient may be called more than once;  why waste memory?
  if (vertexptr) {
    free(vertexptr);
  }

   vertexptr= (Vertex_Struct **)malloc(
                               sizeof(Vertex_Struct *) * dim);

   if (vertexptr == NULL)
      return 0;
   for(int i=0;i< dim;i++) {
     vertexptr[i]= (Vertex_Struct *)malloc(sizeof(Vertex_Struct)* dim * 2);

     if(vertexptr[i] == NULL )
       return 0;
   }
   return 1;
}

void specify_vertexArray(nmb_PlaneSelection /*planes*/, int i, int count)
{
#if defined(sgi)  // These functions aren't available in CYGWIN:
  if (!g_PRERENDERED_COLORS && !g_PRERENDERED_TEXTURE) {
    glNormalPointerEXT(GL_SHORT, sizeof(Vertex_Struct),
	                        count, vertexptr[i][0].Normal );
  }
    glColorPointerEXT(4,GL_UNSIGNED_BYTE,sizeof(Vertex_Struct),
	                        count, vertexptr[i][0].Color);
    glVertexPointerEXT(3,GL_FLOAT,sizeof(Vertex_Struct),
				count,vertexptr[i][0].Vertex);

    if (g_texture_mode == GL_TEXTURE_1D) // (planes.contour)
 	glTexCoordPointerEXT(1,GL_FLOAT,sizeof(Vertex_Struct),
	                   count,&(vertexptr[i][0].Texcoord[2]));
    else if (g_texture_mode == GL_TEXTURE_3D) // (planes.alpha)
        glTexCoordPointerEXT(3,GL_FLOAT,sizeof(Vertex_Struct),
                                   count,vertexptr[i][0].Texcoord);

// if using projective texture then we still do 1D and 3D textures as above
// but 2D texture coordinates should not be specified here
#ifndef PROJECTIVE_TEXTURE
    else if (g_texture_mode == GL_TEXTURE_2D)
        glTexCoordPointerEXT(2,GL_FLOAT,sizeof(Vertex_Struct),
                                  count,&(vertexptr[i][0].Texcoord[1]));

    else if ( g_genetic_textures_enabled || g_realign_textures_enabled )
      glTexCoordPointerEXT( 2, GL_FLOAT, sizeof(Vertex_Struct), count,
			    &(vertexptr[i][0].Texcoord[1]));
#endif  // PROJECTIVE_TEXTURE

    glDrawArraysEXT( GL_TRIANGLE_STRIP,0,count);
#else
    i = i;           // Keep the compiler happy
    count = count;


    int vert;
    glBegin(GL_TRIANGLE_STRIP);
    for (vert = 0; vert < count; vert++) {
        if (!g_PRERENDERED_COLORS && !g_PRERENDERED_TEXTURE) {
	    glNormal3s(	vertexptr[i][vert].Normal[0],
			vertexptr[i][vert].Normal[1],
                        vertexptr[i][vert].Normal[2]);
        }
        glColor4b( 	vertexptr[i][vert].Color[0],
			vertexptr[i][vert].Color[1],
			vertexptr[i][vert].Color[2],
			vertexptr[i][vert].Color[3]);
	if (g_texture_mode == GL_TEXTURE_1D) {// (planes.contour)
	    glTexCoord1f(vertexptr[i][vert].Texcoord[0]);
        }
        glVertex3f(vertexptr[i][vert].Vertex[0],
		   vertexptr[i][vert].Vertex[1],
		   vertexptr[i][vert].Vertex[2]);
    }
    glEnd();

#endif

}




/*      This routine will describe the vertex from the grid specified
 * by the (x,y) coordinates.
 *      This routine returns 0 on success and -1 on failure. */
int describe_gl_vertex(nmb_PlaneSelection planes, GLdouble minColor[4],
		       GLdouble maxColor[4], int x, int y, 
		       Vertex_Struct *vertexArrayPtr)
{
  VERBOSE(10, "    *************Entering describe_gl_vertex"); 

  double  min_x = planes.height->minX(), max_x = planes.height->maxX();
  double  min_y = planes.height->minY(), max_y = planes.height->maxY();
  
  int i;
  GLfloat Normal [3];
  GLfloat Vertex [3];
  
  /* Make sure it is a legal vertex */
  if ( (x < 0) || (x > planes.height->numX() - 1) ||
       (y < 0) || (y > planes.height->numY() - 1) ) {
    fprintf(stderr, "describe_gl_vertex:  %d, %d outside grid.\n", x, y);
    return -1;
  }
  
  /* Find the normal for the vertex */
  // Using prerendered colors (and no lighting), we don't need normals
  if (!g_PRERENDERED_COLORS && !g_PRERENDERED_TEXTURE) {
    if (stm_compute_plane_normal(planes.height, x,y,
         (float) ((max_x - min_x) / planes.height->numX()),
         (float) ((max_y - min_y) / planes.height->numY()),
			         1.0f,
			         Normal)) {
      fprintf(stderr,"describe_gl_vertex(): Can't find normal!\n");
      return -1;
    }
  
    if (g_VERTEX_ARRAY) {
      vertexArrayPtr->Normal[0] = (GLshort) (Normal[0] * 32767);
      vertexArrayPtr->Normal[1] = (GLshort) (Normal[1] * 32767);
      vertexArrayPtr->Normal[2] = (GLshort) (Normal[2] * 32767);
    }
    else {
      glNormal3fv(Normal);
    }
  }

  /* Color the vertex according to its color parameter, if we have
   * a color plane.  Clip the value mapped to color from 0 to 1.  */
  // XXX At some point, we may want to set alpha.
  GLfloat Color [4];
  if (g_PRERENDERED_TEXTURE) {
    // Do we need a flat white background to modulate the texture?
    Color[0] = 1.0f;
    Color[1] = 1.0f;
    Color[2] = 1.0f;
    Color[3] = g_surface_alpha * 255;
  } else if (g_PRERENDERED_COLORS) {
    Color[0] = planes.red->value(x, y);
    Color[1] = planes.green->value(x, y);
    Color[2] = planes.blue->value(x, y);
    Color[3] = g_surface_alpha * 255;
      // XXX why do the other implementations cast this to a GLubyte
      // before writing it into a float?
  } else if (planes.color) {
    double scale = (planes.color->value(x, y) - g_color_slider_min) /
      (g_color_slider_max - g_color_slider_min);
    scale = min(1.0, scale);
    scale = max(0.0, scale);
    if (g_curColorMap) {    // Use the color map loaded from file
      float r, g, b, a;
      g_curColorMap->lookup(scale, &r, &g, &b, &a);
      Color[0] = r; Color[1] = g; Color[2] = b; Color[3] = (GLubyte) (g_surface_alpha * 255);
    } else {      // Use the CUSTOM color mapping tool
      for (i = 0; i < 3; i++) {
	double  color_diff = (maxColor[i] - minColor[i]);
	Color[i] = minColor[i] + (color_diff * scale);
        Color[3] = (GLubyte) (g_surface_alpha * 255);
      
      }
    }
  }
  if (g_PRERENDERED_COLORS || g_PRERENDERED_TEXTURE || planes.color) {
    if (g_VERTEX_ARRAY) {
      vertexArrayPtr->Color[0] = (GLubyte) (Color[0] * 255);
      vertexArrayPtr->Color[1] = (GLubyte) (Color[1] * 255); 
      vertexArrayPtr->Color[2] = (GLubyte) (Color[2] * 255); 
      vertexArrayPtr->Color[3] = (GLubyte) (g_surface_alpha * 255);
    }
    else {
      glColor4fv(Color);
    }
  }

#ifndef PROJECTIVE_TEXTURE
  // Realigning Textures, no need to recalculate  
  int num_x = planes.height->numX();
  int num_y = planes.height->numY();
  if ( g_realign_textures_enabled ) {
    if ( g_translate_textures ) {
      if ( (x == (num_x -1)) && (y == (num_y -1))) {
	g_translate_textures = 0;
	g_tex_coord_center_x += g_translate_tex_dx;
	g_tex_coord_center_y += g_translate_tex_dy;
      }
      GLfloat translated_coord[2];
      
      if ( g_VERTEX_ARRAY ) {
	translated_coord[0]=vertexArrayPtr->Texcoord[1]+g_translate_tex_dx;
	translated_coord[1]=vertexArrayPtr->Texcoord[2]+g_translate_tex_dy;
	vertexArrayPtr->Texcoord[1]=  translated_coord[0];
	vertexArrayPtr->Texcoord[2]=  translated_coord[1];
      }
      return 0;
    }
    else if ( g_scale_textures ) {
      if ( (x == (num_x -1)) && (y == (num_y -1))) {
	g_scale_textures = 0;
      }
      GLfloat scale_coord[2];
      
      if ( g_VERTEX_ARRAY ) {
	scale_coord[0]=
	  g_tex_coord_center_x +
	  (( vertexArrayPtr->Texcoord[1] - g_tex_coord_center_x ) *
	   g_scale_tex_dx );
	scale_coord[1]=
	  g_tex_coord_center_y +
	  (( vertexArrayPtr->Texcoord[2] - g_tex_coord_center_y ) *
	   g_scale_tex_dy);
	vertexArrayPtr->Texcoord[1]=  scale_coord[0];
 	vertexArrayPtr->Texcoord[2]=  scale_coord[1];
      }
      return 0;
    }
    
    else if ( g_shear_textures ) {
      if ( (x == (num_x -1)) && (y == (num_y -1))) {
	g_shear_textures = 0;
      }
      GLfloat shear_coord[2];
      
      if ( g_VERTEX_ARRAY ) {
	shear_coord[0] = g_tex_coord_center_x +
	  ( ( vertexArrayPtr->Texcoord[1] - g_tex_coord_center_x) +
	    ( g_shear_tex_dx *
	      ( vertexArrayPtr->Texcoord[2] - g_tex_coord_center_y ) ) );
	shear_coord[1] = g_tex_coord_center_y +
	  ( ( vertexArrayPtr->Texcoord[2] - g_tex_coord_center_y ) +
	    ( g_shear_tex_dy *
	      ( vertexArrayPtr->Texcoord[1] - g_tex_coord_center_x ) ) );
	vertexArrayPtr->Texcoord[1]=  shear_coord[0];
 	vertexArrayPtr->Texcoord[2]=  shear_coord[1];
      }		
      return 0;
    }

    else if ( g_rotate_textures ) {
      if ( (x == (num_x -1)) && (y == (num_y -1))) {
	g_rotate_textures = 0;
      }
      GLfloat rotate_coord[2];
      
      if ( g_VERTEX_ARRAY ) {
	rotate_coord[0] = g_tex_coord_center_x +
	  ( ( vertexArrayPtr->Texcoord[1] - g_tex_coord_center_x ) *
	    cos( g_rotate_tex_theta ) + 
	    ( vertexArrayPtr->Texcoord[2] - g_tex_coord_center_y ) *
	    sin( g_rotate_tex_theta ) );
	rotate_coord[1] = g_tex_coord_center_y +
	  ( ( vertexArrayPtr->Texcoord[1] - g_tex_coord_center_x ) *
	    -sin( g_rotate_tex_theta ) +
	    ( vertexArrayPtr->Texcoord[2] - g_tex_coord_center_y ) *
	    cos( g_rotate_tex_theta ) );
	
	vertexArrayPtr->Texcoord[1]=  rotate_coord[0];
	vertexArrayPtr->Texcoord[2]=  rotate_coord[1];
      }		

      return 0;
    }
  }
#endif
  
  // Texture scale is multipled by 10 because there are 10 steps of red
  // between each step of white (10 gradiations drawn in the texture
  // map).
  
  if (planes.contour) {
    GLfloat Scoord =
      (float) (planes.contour->value(x, y) /
	       (g_texture_scale * 10.0f));
    if(g_VERTEX_ARRAY) {
      vertexArrayPtr->Texcoord[2] = Scoord;
    }
    else {
      glTexCoord1f(Scoord);
    }
  }

  
#ifdef  FLOW
  // Define the texture coordinate for pxfl
  {
    GLfloat texcoord[2];
    texcoord[0] = (GLfloat) x / g_data_tex_size; 
    texcoord[1] = (GLfloat) y / g_data_tex_size;
    glTexCoord2fv(texcoord);
  }
#endif
  
  
#if defined(sgi)
  if (planes.alpha) {
    double value;
    GLfloat checktexcoord[3];
    
    value = (planes.alpha->value(x, y) - g_alpha_slider_min) /
      (g_alpha_slider_max - g_alpha_slider_min);
    value = min(1.0, value);
    value = max(0.0, value);
    
    // integer coordinates are used as texture coord for
    // checker board pattern
    checktexcoord[0] = x * 6.0f / planes.height->numX();
    checktexcoord[1] = y * 6.0f / planes.height->numY();
    checktexcoord[2] = value;
    
    if(g_VERTEX_ARRAY) {
      vertexArrayPtr->Texcoord[0] = checktexcoord[0];
      vertexArrayPtr->Texcoord[1] = checktexcoord[1];
      vertexArrayPtr->Texcoord[2] = checktexcoord[2]; 
    }
    else {
      glTexCoord3fv(checktexcoord);
    }	
  }
#endif

#ifndef PROJECTIVE_TEXTURE
  if (g_texture_transform_mode == RULERGRID_COORD) {
    GLfloat rulercoord [2];

    /*
      texture_coord = T*plane_coord
      plane_coord has components (x,y,w)
      texture_coord has components(s,t)

      where T*g_rulergrid_scale = 
/								    \
| (g_rulergrid_cos) (-g_rulergrid_sin) (-g_rulergrid_xoffset*rulergrid_cos +
				     g_rulergrid_yoffset*rulergrid_sin) 
| (g_rulergrid_sin) (g_rulergrid_cos)  (-g_rulergrid_yoffset*rulergrid_cos -
				     g_rulergrid_xoffset*rulergrid_sin
\								    /
    */

    rulercoord[0] = (float) (
			     ((planes.height->xInWorld(x) -
			       g_rulergrid_xoffset) * g_rulergrid_cos -
			      (planes.height->yInWorld(y) -
			       g_rulergrid_yoffset) * g_rulergrid_sin) /
			     g_rulergrid_scale);
    rulercoord[1] = (float) (
			     ((planes.height->yInWorld(y) -
			       g_rulergrid_yoffset) * g_rulergrid_cos +
			      (planes.height->xInWorld(x) -
			       g_rulergrid_xoffset) * g_rulergrid_sin) /
			     g_rulergrid_scale);
    
    if(g_VERTEX_ARRAY) {
      vertexArrayPtr->Texcoord[1] = rulercoord[0];
      vertexArrayPtr->Texcoord[2] = rulercoord[1];
    }
    else {
      glTexCoord2fv(rulercoord);
    }
  } 
  else if (g_texture_transform_mode == MANUAL_REALIGN_COORD) {
    GLfloat genetic_coord[2];
    
    if ( (x == (num_x -1)) && (y == (num_y -1))) {
      g_tex_coord_center_x = (num_x / 512.0) / 2.0;
      g_tex_coord_center_y = (num_y / 512.0) / 2.0;
      g_tex_range_x = 1000.0;
      g_tex_range_y = 1000.0;
      g_tex_theta_cumulative = 0.0;
    }
    genetic_coord[0]= (float) (x/512.0);
    genetic_coord[1]= (float) (y/512.0);         
    if(g_VERTEX_ARRAY) {
      vertexArrayPtr->Texcoord[1]=  genetic_coord[0];
      vertexArrayPtr->Texcoord[2]=  genetic_coord[1];
    }
    else {
      glTexCoord2fv(genetic_coord);
    }		
  }
#endif


#if defined(sgi) || defined(_WIN32) || defined(FLOW)

  if (g_PRERENDERED_TEXTURE) {
    GLfloat tc [2];

    tc[0] = x / 512.0f;
    tc[1] = y / 512.0f;

    if (g_VERTEX_ARRAY) {
      vertexArrayPtr->Texcoord[0] = tc[0];
      vertexArrayPtr->Texcoord[1] = tc[1];
    } else {
      glTexCoord2fv(tc);
    }
  }

#endif
  
  /* Put the vertex at the correct location */
  
  Vertex[0]= (float) planes.height->xInWorld(x);
  Vertex[1]= (float) planes.height->yInWorld(y);
  Vertex[2]= (float) planes.height->valueInWorld(x, y);
  
  if(g_VERTEX_ARRAY) {
    vertexArrayPtr->Vertex[0] = Vertex[0];
    vertexArrayPtr->Vertex[1] = Vertex[1];
    vertexArrayPtr->Vertex[2] = Vertex[2];
  }
  else {
    glVertex3fv(Vertex);
  }
  
  return(0);
}

        

/*	This routine will create the openGL commands needed to display a
 * triangle strip for one of the strips needed to show a grid.  This routine
 * displays a strip along the Y axis;  the "which" parameter selects from the
 * strips to be drawn.  The first strip is 0 and the last is grid->num_x - 1,
 * when the stride is 1.  The stride determines the number of points to move
 * in both X and Y to get to the next grid point to use.  If the stride is
 * not an integer factor of the length, the last partial square is ignored.
 *	This routine should only be called to generate a strip for an integer
 * multiple of stride.
 *	The triangle strips are mapped from the grid into the graphics space
 * as follows:
 *                                                                         
 *          +-----+                                    
 *         7|    /|6                                                     
 *          |   / |                                                     
 *          |  /  |                                                        
 *          | /   |                                                         
 *          |/    |                                                      
 *          +-----+           (Clockwise face is forwards)
 *         5|    /|4                                                     
 *          |   / |                                                     
 *          |  /  |                                                        
 *          | /   |                                                         
 *          |/    |                                                      
 *          +-----+                                                     
 *    ^    3|    /|2                                                     
 *    |     |   / |                                                     
 *    |     |  /  |                                                        
 *    |     | /   |                                                         
 *    |     |/    |                                                      
 *    |     +-----+                                                     
 *    Y    1       0       
*     X-------->                                                              
 *	This routine returns 0 on success and -1 on failure. */

int spm_y_strip( nmb_PlaneSelection planes,
                 GLdouble minColor[4], GLdouble maxColor[4], int which,
                 Vertex_Struct * vertexArray)
{       int     x,y;
        int    i=0;

        // Make sure we found a height plane
        if (planes.height == NULL) {
            fprintf(stderr, "spm_y_strip: could not get grid!\n");
            return -1;
        }

        // Make sure they asked for a legal strip (in range and on stride)
        if ((which < 0) || (which >= planes.height->numX())) {
          fprintf(stderr, "Strip %d is outside the plane.\n", which);
                return(-1);
        }
        if (which % g_stride) {
          fprintf(stderr, "Strip %d is off stride.\n", which);
          return -1;
        }

        /* Fill in the vertices for the triangle strip */
        glFrontFace(GL_CCW);            /* Counter-clockwise is forward */

      if (g_VERTEX_ARRAY){
        for (y = 0; y < planes.height->numY(); y += g_stride) {   // Left->right
         for (x = which+g_stride; x >= which; x -= g_stride) {     // top->bottom
            if (describe_gl_vertex(planes, minColor,maxColor,x,y,
                                   &(vertexArray[i]))) {
              fprintf(stderr, "spm_y_strip:  describe_gl_vertex() failed.\n");
                return(-1);
            }
            i++;
          }
         }

         return(i);  // i is the total number of points in the strip

      } else {
	glBegin(GL_TRIANGLE_STRIP);
  VERBOSE(20, "          glBegin(GL_TRIANGLE_STRIP)");
	for (y = 0; y < planes.height->numY(); y += g_stride) {	// bottom->top
	  for (x = which+g_stride; x >= which; x -= g_stride) {// right->left
 if (describe_gl_vertex(planes, minColor,maxColor, x,y, NULL)) {
		return(-1);
	    }
	  }
	}
  VERBOSE(20, "          glEnd()");
	glEnd();

	return(0);
     }
}


int spm_x_strip( nmb_PlaneSelection planes,
                 GLdouble minColor[4], GLdouble maxColor[4], int which,
                 Vertex_Struct * vertexArray)
{       int     x,y;
        int    i=0;	

        // Make sure we found a height plane
        if (planes.height == NULL) {
            fprintf(stderr, "spm_x_strip: could not get grid!\n");
            return -1;
        }

        // Make sure they asked for a legal strip (in range and on stride)
        if ((which < 0) || (which >= planes.height->numY() - 1)) {
          fprintf(stderr, "Strip %d is outside plane.\n", which);
                return(-1);
        }
        if (which % g_stride) {
          fprintf(stderr, "Strip %d is off stride.\n", which);
                return(-1);
        }

        /* Fill in the vertices for the triangle strip */
        glFrontFace(GL_CCW);            /* Counter-clockwise is forward */

      if(g_VERTEX_ARRAY) {
        for (x = 0; x < planes.height->numX(); x += g_stride) {   // Left->right
         for (y = which+g_stride; y >= which; y -= g_stride) {     // top->bottom
            if (describe_gl_vertex(planes, minColor,maxColor,x,y,
                                   &(vertexArray[i]))) {
              fprintf(stderr, "spm_x_strip:  describe_gl_vertex() failed.\n");
                return(-1);
            }
            i++;
          }
        }
        
        return(i);  // i is the total number of points in the strip

       } else {
	glBegin(GL_TRIANGLE_STRIP);
  VERBOSE(20, "          glBegin(GL_TRIANGLE_STRIP)");
        for (x = 0; x < planes.height->numX(); x += g_stride) {   // Left->right
         for (y = which+g_stride; y >= which; y -= g_stride) {     // top->bottom
           if(describe_gl_vertex(planes, minColor,maxColor, x,y, NULL)) {
                     return(-1);
                 }
               }
          }
  VERBOSE(20, "          glEnd()");
         glEnd();

         return(0);
      }
}

	


/*	This routine will set up the material properties so that the
 * surface will appear to be made of shiny plastic. */

void    spm_set_surface_materials(void)
{	GLfloat	specular[4] = { (float)g_specular_color,
				(float)g_specular_color,
				(float)g_specular_color, 1.0 };
	GLfloat	dark[4] = { 0.0, 0.0, 0.0, 1.0 };


//fprintf(stderr, "In spm_set_surface_materials with texture mode %d.\n",
//g_texture_mode);
	TIMERVERBOSE(5, mytimer, "begin spm_set_surface_materials");

	/* Use local vertex color for ambient and diffuse */
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

#ifdef FLOW
    glShaderEXT(nM_shader);
#endif

	// Set up the specular characteristics.
	// Note that the ambient and diffuse colors are from the vertices.
	// NOTE: It is important that back is set first because front/back
	//       is ignored in an early implementation of FLOW, and it always
	//       set both.
	glMaterialfv(GL_BACK, GL_SPECULAR, dark);
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
	glMaterialf(GL_FRONT, GL_SHININESS, g_shiny);

	// Set the light model to have completely ambient-off.  There is
	// ambient specified in light 0.
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, dark);

	/* Set a color, in case the color is not being adjusted per vertex. */
	/* Use the min color for this. */
        g_minColor[3] = g_surface_alpha; //make sure alpha value is updated
	glColor4dv(g_minColor);

	// Turn texture mapping to the appropriate state
	// Note that the Enable has to be the last one, after all the
	// Disable calls.
	switch (g_texture_mode) {
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
                  fprintf(stderr, "spm_set_surface_materials:  "
                                  "texture_mode %i\n", g_texture_mode);
		break;
	}
	if (spm_graphics_verbosity > 3)
          fprintf(stderr, "spm_set_surface_materials:  "
                          "texture_mode %i\n", g_texture_mode);

	TIMERVERBOSE(5, mytimer, "end spm_set_surface_materials");
}

/*	This routine will set up the material properties so that the
 * icons and such will appear to be made of shiny plastic, and will react
 * to specular and diffuse lighting, but will never be textured. */

void    spm_set_icon_materials(void)
{	GLfloat	specular[4] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat	dark[4] = { 0.0, 0.0, 0.0, 1.0 };

	VERBOSE(5, "    Entering spm_set_icon_materials()");
	TIMERVERBOSE(5, mytimer, "begin spm_set_icon_materials");

	/* Use local vertex color for ambient and diffuse */
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);
#ifdef FLOW
    glShaderEXT(nM_diffuse);
#endif

	// Set up the specular characteristics.
	// Note that the ambient and diffuse colors are from the vertices.
	// NOTE: It is important that back is set first because front/back
	//       is ignored in an early implementation of FLOW, and it always
	//       set both.
	glMaterialfv(GL_BACK, GL_SPECULAR, dark);
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
	glMaterialf(GL_FRONT, GL_SHININESS, g_shiny);
        glDisable(GL_BLEND);

	// Set the light model to have completely ambient-off.  There is
	// ambient specified in light 0.
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, dark);

	/* Set a color, in case the color is not being adjusted per vertex. */
	/* Use the min color for this. */
        g_minColor[3] = g_surface_alpha; //make sure alpha value is updated
	glColor4dv(g_minColor);

	// Disable texture-mapping.
	glDisable(GL_TEXTURE_1D);
	glDisable(GL_TEXTURE_2D);
#ifdef	sgi
	glDisable(GL_TEXTURE_3D_EXT);
#endif

	TIMERVERBOSE(5, mytimer, "end spm_set_icon_materials");
}

/*	This routine will set up the material properties so that the
 * measurement tools (lines and text) will not depend on the lighting
 * model or be texture-mapped.  This is done by setting the ambient
 * coefficients to 1 and the diffuse/specular ones to 0.  */

void    spm_set_measure_materials(void)
{	GLfloat	bright[4] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat	dark[4] = { 0.0, 0.0, 0.0, 1.0 };

	VERBOSE(5, "    Entering spm_set_measure_materials()");
	TIMERVERBOSE(5, mytimer, "begin spm_set_measure_materials");

	/* Use local vertex color for ambient and diffuse */
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	
	TIMERVERBOSE(7, mytimer, "spm_set_measure_materials:end glColorMaterial");
	glEnable(GL_COLOR_MATERIAL);
        glDisable(GL_BLEND);
     
	TIMERVERBOSE(7, mytimer, "spm_set_measure_materials:end glEnable(GL_COLOR_MATERIAL)");

#ifdef FLOW
        glShaderEXT(nM_diffuse);
#endif

	// Set up the specular characteristics.
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, dark);

        TIMERVERBOSE(7, mytimer, "spm_set_measure_materials: end glMaterialfv");

	// Set the light model to have completely ambient-on.
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, bright);

        TIMERVERBOSE(7, mytimer, "spm_set_measure_materials: end glLightModelfv");

	/* Set a color, in case the color is not being adjusted per vertex. */
	/* Use the min color for this. */
        g_minColor[3] = g_surface_alpha; //Make sure alpha value is updated
	glColor4dv(g_minColor);

        TIMERVERBOSE(7, mytimer, "spm_set_measure_materials: end glColor3dv");

	// Disable texture-mapping.
	glDisable(GL_TEXTURE_1D);
	glDisable(GL_TEXTURE_2D);
#ifdef	sgi
	glDisable(GL_TEXTURE_3D_EXT);
#endif

	TIMERVERBOSE(5, mytimer, "end spm_set_measure_materials");
}

 
// WARNING
// This may be inefficient.
// If it is too slow, write spm_draw_scrapes() and spm_draw_pulses()
// that do the glLineWidth/glDisable/glBegin/glEnd and just send the
// vertices in spm_render_mark

int spm_render_mark (const nmb_LocationInfo & p, void *) {
  GLfloat Bottom [3], Top [3];

  Bottom[0] = Top[0] = p.x;
  Bottom[1] = Top[1] = p.y;
  Bottom[2] = p.bottom;
  Top[2] = p.top;

  glLineWidth(1.0);
  glDisable(GL_LINE_STIPPLE);
  glBegin(GL_LINES);
  VERBOSE(20, "          glBegin(GL_TRIANGLE_STRIP)");
  glVertex3fv(Bottom);
  glVertex3fv(Top);
  VERBOSE(20, "          glEnd()");
  glEnd();

  return 0;
}

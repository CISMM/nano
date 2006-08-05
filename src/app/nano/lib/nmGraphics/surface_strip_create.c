/*===3rdtech===
Copyright (c) 2000 by 3rdTech, Inc.
All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
===3rdtech===*/
/*
* @file surface_strip_create.c
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
#include <assert.h>

#include <nmb_ColorMap.h>
#include <nmb_PlaneSelection.h>
#include <nmb_Decoration.h>
#include <nmb_Globals.h>
#include "nmg_SurfaceMask.h"
#include "nmg_SurfaceRegion.h"

#include "surface_strip_create.h"
// #include "Tcl_Linkvar.h"
#include "nmg_State.h"
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
/// Speed up stm_compute_plane_normal() by specilizing cross-product routines,
/// knowing that we're multiplying by axis-aligned unit vectors.
/// Gives us about a 10% speedup.
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
        fprintf(stderr,"vector_normalize:  vector has zero magnitude\n");
        a[0] = a[1] = a[2] = 1.0/sqrt(3.0);
    } else {
        mag = 1.0/mag;
        a[0] *= mag;
        a[1] *= mag;
        a[2] *= mag;
    }
}


/**	This routine finds the normal to the surface at the given grid
* point.  It does this by averaging the normals with the four 4-connected
* points in the grid (one away in x or y.)
*	This routine takes into account the current stride in x and y
* between tesselated points and grid points.
*	This routine returns 0 on success and -1 on failure. */

int	stm_compute_plane_normal(BCPlane *plane, int x,int y,
                                 double dx,double dy,double dz, 
                                 int stride, GLfloat Normal[3])
{
    GLfloat		diff_vec[3];
    GLfloat		local_norm[3];
    int	i;
    
    // Valid points must be within the grid and lie exactly on a stride
    // point.
    if ( (x < 0) || (x > plane->numX()-1) ||
        (y < 0) || (y > plane->numY()-1) ||
        (x % stride) || (y % stride) ){
        return(-1);
    }
    
    /* Initially, clear the normal */
    for (i = 0; i < 3; i++) {
        Normal[i] = 0.0;
    }
    
    // Find the normal with stride more in x, if it is within the grid
    
    if ( (x+stride) < plane->numX()) {
        diff_vec[X] = dx * stride;
        diff_vec[Y] = 0;
        diff_vec[Z] = dz * (float) (plane->valueInWorld(x+stride,y) -
            plane->valueInWorld(x, y));
        vector_cross_Y(diff_vec, local_norm);
        vector_add(local_norm,Normal, Normal);
    }
    
    // Find the normal with stride more in y, if it is within the grid
    
    if ( (y+stride) < plane->numY()) {
        diff_vec[X] = 0;
        diff_vec[Y] = dy * stride;
        diff_vec[Z] = (float) (dz * (plane->valueInWorld(x,y+stride) -
            plane->valueInWorld(x,y)));
        vector_cross_NX(diff_vec, local_norm);
        vector_add(local_norm,Normal, Normal);
    }
    
    // Find the normal with stride less in x, if it is within the grid
    
    if ( (x-stride) >= 0) {
        diff_vec[X] = -dx * stride;
        diff_vec[Y] = 0;
        diff_vec[Z] = (float) (dz * (plane->valueInWorld(x-stride,y) -
            plane->valueInWorld(x, y)));
        vector_cross_NY(diff_vec, local_norm);
        vector_add(local_norm,Normal, Normal);
    }
    
    // Find the normal with stride less in y, if it is within the grid
    
    if ( (y-stride) >= 0) {
        diff_vec[X] = 0;
        diff_vec[Y] = -dy * stride;
        diff_vec[Z] = (float) (dz * (plane->valueInWorld(x,y-stride) -
            plane->valueInWorld(x, y)));
        vector_cross_X(diff_vec, local_norm);
        vector_add(local_norm,Normal, Normal);
    }
    
    /* Normalize the normal */
    
    vector_normalize(Normal);
    return(0);
}

void specify_vertexArray(nmg_State * state,Vertex_Struct * vertexArray, int *vert_counts, int num_strips)
{
    if (!state->PRERENDERED_COLORS && !state->PRERENDERED_TEXTURE) {
        glNormalPointer(GL_SHORT, sizeof(Vertex_Struct),
            vertexArray[0].Normal );
    }
    glColorPointer(4,GL_UNSIGNED_BYTE,sizeof(Vertex_Struct),
        vertexArray[0].Color);
    glVertexPointer(3,GL_FLOAT,sizeof(Vertex_Struct),
        vertexArray[0].Vertex);
    
    if (state->texture_mode == GL_TEXTURE_1D) // (planes.contour)
        glTexCoordPointer(1,GL_FLOAT,sizeof(Vertex_Struct),
        &(vertexArray[0].Texcoord[2]));
#if defined(sgi)
    else if (state->texture_mode == GL_TEXTURE_3D) // (planes.alpha)
        glTexCoordPointer(3,GL_FLOAT,sizeof(Vertex_Struct),
        vertexArray[0].Texcoord);
#endif
    
    // if using projective texture then we still do 1D and 3D textures as above
    // but 2D texture coordinates should not be specified here
#ifndef PROJECTIVE_TEXTURE
    else if (state->texture_mode == GL_TEXTURE_2D)
        glTexCoordPointer(2,GL_FLOAT,sizeof(Vertex_Struct),
        &(vertexArray[0].Texcoord[1]));
#endif  // PROJECTIVE_TEXTURE
    
    int start = 0;
    for(int i = 0; i < num_strips; i++) {
        glDrawArrays( GL_TRIANGLE_STRIP,start,vert_counts[i]);
        start += vert_counts[i];
    }
}


/**      This routine will describe the vertex from the grid specified
* by the (x,y) coordinates.
*      @return 0 on success and -1 on failure. 
*/
static int
describe_gl_vertex(nmg_State * state, const nmb_PlaneSelection & planes,
                   GLdouble surfaceColor[3],
                   int x, int y, 
                   Vertex_Struct *vertexArrayPtr)
{
    VERBOSE(10, "    *************Entering describe_gl_vertex"); 
    
    // dereference once
    BCPlane & height_plane = *(planes.height);
    BCPlane * transparent_plane = planes.transparent;
    Vertex_Struct & vertexArray = *vertexArrayPtr;
    
    double  min_x = height_plane.minX(), max_x = height_plane.maxX();
    double  min_y = height_plane.minY(), max_y = height_plane.maxY();
    
    int i;
    GLfloat Normal [3];
    GLfloat Vertex [3];
    GLfloat Color [4];
    
    /* Make sure it is a legal vertex */
    if ( (x < 0) || (x > height_plane.numX() - 1) ||
        (y < 0) || (y > height_plane.numY() - 1) ) {
        fprintf(stderr, "describe_gl_vertex:  %d, %d outside grid.\n", x, y);
        return -1;
    }
    
    // Vertices used in following calculations. Compute here.
    Vertex[0]= (float) height_plane.xInWorld(x);
    Vertex[1]= (float) height_plane.yInWorld(y);
    Vertex[2]= (float) height_plane.valueInWorld(x, y);
    
    vertexArray.Vertex[0] = Vertex[0];
    vertexArray.Vertex[1] = Vertex[1];
    vertexArray.Vertex[2] = Vertex[2];
    
    
    // Phase 1: Color the vertex 
    if (state->PRERENDERED_TEXTURE) {
        // Do we need a flat white background to modulate the texture?
        Color[0] = 1.0f;
        Color[1] = 1.0f;
        Color[2] = 1.0f;
    }
    else if (state->PRERENDERED_COLORS) {
        Color[0] = planes.red->value(x, y);
        Color[1] = planes.green->value(x, y);
        Color[2] = planes.blue->value(x, y);
    }
    else if (planes.color) {
        // Get the data value
        float data_value = planes.color->value(x, y);
        // Drift-compensate the data value, based on 
        // the stored average of the first scan line.
        if ((decoration->first_line_avg !=0) && 
            (decoration->first_line_avg_prev !=0)) {
            data_value += decoration->first_line_avg_prev 
                - decoration->first_line_avg;
        }
        // stretch/shrink data based on data_min/max colors:
        // normalize the data to a zero to one scale - but data doesn't
        // have to fall in this range.
        data_value = (data_value - state->data_min)/(state->data_max - state->data_min);
        
        // Scale data again based on color map widget controls
        data_value = data_value * (state->data_max_norm - state->data_min_norm) 
            + state->data_min_norm;
        
        // clamp data based on the stretched/shrunk colormap:
        if ( data_value <  state->color_min ) data_value = 0;
        else if ( data_value > state->color_max ) data_value = 1.0;
        else data_value = (data_value - state->color_min)/(state->color_max - state->color_min);
        
        if (state->curColorMap) {    // Use the color map loaded from file
            // a = alpha is ignored. 
            float r, g, b, a;
            state->curColorMap->lookup(data_value, &r, &g, &b, &a);
            Color[0] = r;
            Color[1] = g;
            Color[2] = b;
        } else {      // Use the "none" color mapping tool
            for (i = 0; i < 3; i++) {
                Color[i] = surfaceColor[i] * data_value;
            }
        }
    }
    else {
        // No special color cases exists, so set the surface to a solid color.
        Color[0] = state->surfaceColor[0];
        Color[1] = state->surfaceColor[1];
        Color[2] = state->surfaceColor[2];
    }
    Color[3] = state->surface_alpha;
    
    // Set the alpha value for this vertex. 
    if (planes.opacity) {
        // A data plane determines the opacity value.
        float opacity_value = (planes.opacity->value(x, y) 
            - state->opacity_slider_min) /
            (state->opacity_slider_max - state->opacity_slider_min);
        Color[3] = (GLubyte) min(255.0, opacity_value);
    }
    
    if (state->transparent) {
        Color[3] = transparent_plane->value(x, y);
    }
    
    // Always specify a vertex color (right?!?)
    //    if (state->PRERENDERED_COLORS || state->PRERENDERED_TEXTURE || planes.color ||
    //        state->null_data_alpha_toggle || state->transparent) {
    vertexArray.Color[0] = (GLubyte) (Color[0] * 255);
    vertexArray.Color[1] = (GLubyte) (Color[1] * 255);
    vertexArray.Color[2] = (GLubyte) (Color[2] * 255);
    
    if ( (state->null_data_alpha_toggle) && (Vertex[2] == 0.0) ) {
        vertexArray.Color[3] = 0;
    } 
    else {
        vertexArray.Color[3] = (GLubyte) (Color[3] * 255);
    }
    
    if (!state->VERTEX_ARRAY) {
        glColor4fv(Color);
    }
    
    // Phase 2: Find the texture coordinates for the vertex. 
    vertexArray.Texcoord[0] = vertexArray.Texcoord[1] = 
        vertexArray.Texcoord[2] = 0;
    
    // Texture scale is multipled by 10 because there are 10 steps of red
    // between each step of white (10 gradiations drawn in the texture
    // map).
    
    if (planes.contour) {
        GLfloat Scoord =
            (float) (planes.contour->value(x, y) /
            (state->texture_scale * 10.0f));
        vertexArray.Texcoord[2] = Scoord;
        if(!state->VERTEX_ARRAY) {
            glTexCoord1f(Scoord);
        }
    }
    
#if defined(sgi)
    if (planes.alpha) {
        double value;
        GLfloat checktexcoord[3];
        
        value = (planes.alpha->value(x, y) - state->alpha_slider_min) /
            (state->alpha_slider_max - state->alpha_slider_min);
        value = min(1.0, value);
        value = max(0.0, value);
        
        // integer coordinates are used as texture coord for
        // checker board pattern
        checktexcoord[0] = x * 6.0f / height_plane.numX();
        checktexcoord[1] = y * 6.0f / height_plane.numY();
        checktexcoord[2] = value;
        
        vertexArray.Texcoord[0] = checktexcoord[0];
        vertexArray.Texcoord[1] = checktexcoord[1];
        vertexArray.Texcoord[2] = checktexcoord[2]; 
        if(!state->VERTEX_ARRAY) {
            glTexCoord3fv(checktexcoord);
        }	
    }
#endif
    
#ifndef PROJECTIVE_TEXTURE
    if (state->texture_transform_mode == RULERGRID_COORD) {
        GLfloat rulercoord [2];
        
        /*
        texture_coord = T*plane_coord
        plane_coord has components (x,y,w)
        texture_coord has components(s,t)
        
          where T*state->rulergrid_scale = 
          /								    \
          | (state->rulergrid_cos) (-state->rulergrid_sin) (-state->rulergrid_xoffset*rulergrid_cos +
          state->rulergrid_yoffset*rulergrid_sin) 
          | (state->rulergrid_sin) (state->rulergrid_cos)  (-state->rulergrid_yoffset*rulergrid_cos -
          state->rulergrid_xoffset*rulergrid_sin
          \								    /
        */
        
        rulercoord[0] = (float) (
            ((height_plane.xInWorld(x) -
            state->rulergrid_xoffset) * state->rulergrid_cos -
            (height_plane.yInWorld(y) -
            state->rulergrid_yoffset) * state->rulergrid_sin) /
            state->rulergrid_scale);
        rulercoord[1] = (float) (
            ((height_plane.yInWorld(y) -
            state->rulergrid_yoffset) * state->rulergrid_cos +
            (height_plane.xInWorld(x) -
            state->rulergrid_xoffset) * state->rulergrid_sin) /
            state->rulergrid_scale);
        
        vertexArray.Texcoord[1] = rulercoord[0];
        vertexArray.Texcoord[2] = rulercoord[1];
        if(!state->VERTEX_ARRAY) {
            glTexCoord2fv(rulercoord);
        }
    } 
    else if (state->texture_transform_mode == PER_QUAD_COORD) {
        if(state->VERTEX_ARRAY) {
            vertexArrayPtr->Texcoord[1] =  x;
            vertexArrayPtr->Texcoord[2] =  y;
        }
        else {
            GLfloat texcoord[2];
            texcoord[0] = x;
            texcoord[1] = y;
            glTexCoord2fv(texcoord);
        }
    }
#endif
    
    
#if defined(sgi) || defined(_WIN32)
    
    if (state->PRERENDERED_TEXTURE) {
        GLfloat tc [2];
        
        tc[0] = x / 512.0f;
        tc[1] = y / 512.0f;
        
        vertexArray.Texcoord[0] = tc[0];
        vertexArray.Texcoord[1] = tc[1];
        if(!state->VERTEX_ARRAY) {
            glTexCoord2fv(tc);
        }
    }
    
#endif
    
    // "Just" color the surface - means specify color and textures, but
    // use the normals cached in the vertex array, and return.
    if (state->just_color) {
        Normal[0] = float( vertexArray.Normal[0] )/32767;
        Normal[1] = float( vertexArray.Normal[1] )/32767;
        Normal[2] = float( vertexArray.Normal[2] )/32767;
        
        if (!state->VERTEX_ARRAY) {
            glNormal3fv(Normal);
            // Use already-computed vertex.
            glVertex3fv(Vertex);
        }
        
        return 0;
    }
    
    // Phase 3: Find the normal for the vertex 
    // Using prerendered colors (and no lighting), we don't need normals
    if (!state->PRERENDERED_COLORS && !state->PRERENDERED_TEXTURE) {
        if (stm_compute_plane_normal(planes.height, x,y,
            (float) ((max_x - min_x) / height_plane.numX()),
            (float) ((max_y - min_y) / height_plane.numY()),
                     1.0f, state->stride,
                     Normal)) {
            fprintf(stderr,"describe_gl_vertex(): Can't find normal!\n");
            return -1;
        }
        
        vertexArray.Normal[0] = (GLshort) (Normal[0] * 32767);
        vertexArray.Normal[1] = (GLshort) (Normal[1] * 32767);
        vertexArray.Normal[2] = (GLshort) (Normal[2] * 32767);
        if (!state->VERTEX_ARRAY) {
            glNormal3fv(Normal);
        }
    }
    
    /* Specify the already-computed vertex */
    if(!state->VERTEX_ARRAY) {
        glVertex3fv(Vertex);
    }
    
    return(0);
}



/**	This routine will create the openGL commands needed to display a
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
/* Probably obsolete
int spm_y_strip(nmg_State * state, const  nmb_PlaneSelection &planes,
                GLdouble surfaceColor[3], int which,
                Vertex_Struct * vertexArray)
{
    int     x,y;
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
    if (which % state->stride) {
        fprintf(stderr, "Strip %d is off stride.\n", which);
        return -1;
    }
    
    if (!state->VERTEX_ARRAY) {
        glBegin(GL_TRIANGLE_STRIP);
        VERBOSE(20, "          glBegin(GL_TRIANGLE_STRIP)");
    }
    
    for (y = 0; y < planes.height->numY(); y += state->stride) {   
        for (x = which+state->stride; x >= which; x -= state->stride) { 
            if (describe_gl_vertex(state, planes, 
                                   surfaceColor,x,y,&(vertexArray[i]))) {
                fprintf(stderr, "spm_y_strip:  describe_gl_vertex() failed.\n");
                return(-1);
            }
            i++;
        }
    }
    
    if (state->VERTEX_ARRAY) {
        specify_vertexArray(state, vertexArray, &i, 1);
    }
    else {
        glEnd();
        VERBOSE(20, "          glEnd()");
    }
    return 0;
}
*/
/**
 This version of spm_y_strip uses an assumed masking plane to decide
 what to draw and what not to draw.  Since there could be situations
 where every 2 points alternated on whether they were set to be drawn
 or not, and since we need 3 points for a triangle, a quad based decision
 metric is used to determine whether to draw or not.  What this means is
 that the decision whether to draw is made at the quad level, so that if
 1 point in the current quad should not be drawn, then the entire quad is
 not drawn.   Since there is a variable that tells this function to invert
 the meaning of the masking plane, in that case if any point should not be
 drawn, then the entire quad is draw. This ensures that 2 passes that use
 the masking plane and the inverted masking plane, will match exactly at
 the "seams".   

 The final result of this procedure will be an unspecified number of tristrips,
 instead of 1 tripstrip like in the above procedure.
 */

int spm_y_strip_masked(nmg_State * state, 
                       const  nmb_PlaneSelection &planes, nmg_SurfaceMask *mask,
                       GLdouble surfaceColor[3], int which,
                       Vertex_Struct * vertexArray)
{
    int     x,y;
    int    i;	
    
    // Make sure we found a height plane
    if (planes.height == NULL) {
        fprintf(stderr, "spm_y_strip_masked: could not get grid!\n");
        return -1;
    }
    
    // Make sure they asked for a legal strip (in range and on stride)
    if ((which < 0) || (which >= planes.height->numX() - 1)) {
        fprintf(stderr, "Strip %d is outside plane.\n", which);
        return(-1);
    }
    
    if (which % state->stride) {
        fprintf(stderr, "Strip %d is off stride.\n", which);
        return(-1);
    }
    
    int number_of_strips = 0;
    bool skipping = false;
    int *x_array = new int[planes.height->numY() * 3];
    int *y_array = new int[planes.height->numY() * 3];
    i = 0;
    
    x = which + state->stride;
    y = 0;
    
    //Check the first quad to see if it is partially masked or not and
    //set up the state variables so that the following loop will run
    //correctly.
    if (mask->quadMasked(x,y,state->stride))
    {
        //If the first quad shouldn't be drawn, telling the loop
        //that it should be currently skipping quads.
        skipping = true;
    }
    else {
        //If the first quad should be draw, then drop the first
        //2 points of the quad into the x and y point arrays.
        x_array[i] = x; x_array[i+1] = x - state->stride;
        y_array[i] = y; y_array[i+1] = y;
        i+=2;
    }
    //We make the decision whether to draw points on a quad basis, but we 
    //store only the "first" two points of that quad.  This is because we
    //are really wanting to output a tristrip.  A tristrip can equivalently
    //be thought of as a quad strip, so we know that the "second" set of 
    //points will be the first set of the next quad on the next pass.
    for (y = state->stride; y < planes.height->numX() - state->stride; y += state->stride) {   // Left->right
        bool quad_masked = mask->quadMasked(x,y,state->stride);
        if (!skipping) {
            //If we get in here, then we know that on the previous run
            //of the loop we decided that the entire quad should be 
            //displayed, so it will always be safe to drop the first
            //two points of the current quad, since they were the second
            //points of the last quad, which we know to be good.
            x_array[i] = x; x_array[i+1] = x - state->stride;
            y_array[i] = y; y_array[i+1] = y;
            i+=2;
            if (quad_masked)
            {
                //If we had some good values before, and we have decided
                //that the current quad shouldn't be drawn, then start
                //skipping and throw some bogus values into
                //the x and y arrays, so that in the next stage we will
                //know when 1 tristrip has ended and we need to start up
                //a new one.
                skipping = true;
                x_array[i] = -1;
                y_array[i] = -1;
                i++;
                number_of_strips++;
            }
        }
        else if (skipping) {
            if (!quad_masked)
            {
                //We had some unknown amount of quads that we decided
                //shouldn't be displayed and have found 1 that should.
                //So don't skip anymore and store the
                //first two points.
                skipping = false;
                x_array[i] = x; x_array[i+1] = x - state->stride;
                y_array[i] = y; y_array[i+1] = y;
                i+=2;
            }
        }
    }
    
    //Since we only drop the first two points of good quads
    //per loop, if we exit and we weren't skipping then we
    //need to go ahead and put in the last 2 points of the
    //last quad
    if (!skipping) {
        x_array[i] = x; x_array[i+1] = x;
        y_array[i] = y; y_array[i+1] = y;
        i+=2;
        number_of_strips++;
    }
    else {
        i--;
    }
    
    int max = i;
    int count = 0;
    int *vert_counts;
    int strip_i = 0;
    int vert = 0;
    
    //Now draw the valid strips that we have determined should be
    //formed from the information in the masking plane.

    if (!state->VERTEX_ARRAY) {
        glBegin(GL_TRIANGLE_STRIP);
        VERBOSE(20, "          glBegin(GL_TRIANGLE_STRIP)");
    }
    
    
    if (number_of_strips > 0) {
        vert_counts = new int[number_of_strips];
        
        for (i = 0; i < max; i++) {
            x = x_array[i];
            y = y_array[i];
            if (x == -1 && y == -1) {
                if (state->VERTEX_ARRAY) {
                    vert_counts[strip_i] = count;
                    count = 0;
                    strip_i++;
                }
                else {
                    glEnd();
                    glBegin(GL_TRIANGLE_STRIP);
                }
            }
            else {
                if (describe_gl_vertex(state, planes, surfaceColor,x,y,
                                       &(vertexArray[vert]))) {
                    fprintf(stderr, "spm_y_strip_masked:  describe_gl_vertex() failed.\n");
                    return(-1);
                }
                vert++;
                count++;
            }
        }
        
        if (state->VERTEX_ARRAY) {
            vert_counts[strip_i] = count;
            specify_vertexArray(state, vertexArray, 
                                vert_counts, number_of_strips);
        }
        else {
            glEnd();
            VERBOSE(20, "          glEnd()");
        }
        
        delete [] vert_counts;
    }
    
    delete [] x_array;
    delete [] y_array;
    
    return 0;
}

/**
 *	This routine will create the openGL commands needed to display a
 * triangle strip for one of the strips needed to show a grid.  This routine
 * displays a strip along the X axis;  the "which" parameter selects from the
 * strips to be drawn.  The first strip is 0 and the last is grid->num_y - 1,
 * when the stride is 1.  The stride determines the number of points to move
 * in both X and Y to get to the next grid point to use.  If the stride is
 * not an integer factor of the length, the last partial square is ignored.
 *	This routine should only be called to generate a strip for an integer
 * multiple of stride.
 * 	The triangle strips are mapped from the grid into the graphics space
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
/* Probably obsolete
int spm_x_strip(nmg_State * state, const  nmb_PlaneSelection &planes,
                GLdouble surfaceColor[3], int which,
                Vertex_Struct * vertexArray)
{
    int     x,y;
    int    i=0;
    
    // Make sure we found a height plane
    if (planes.height == NULL) {
        fprintf(stderr, "spm_x_strip: could not get grid!\n");
        return -1;
    }
    
    // Make sure they asked for a legal strip (in range and on stride)
    if ((which < 0) || (which >= planes.height->numX())) {
        fprintf(stderr, "Strip %d is outside the plane.\n", which);
        return(-1);
    }
    if (which % state->stride) {
        fprintf(stderr, "Strip %d is off stride.\n", which);
        return -1;
    }
    
    if (!state->VERTEX_ARRAY) {
        glBegin(GL_TRIANGLE_STRIP);
        VERBOSE(20, "          glBegin(GL_TRIANGLE_STRIP)");
    }
    
    for (x = 0; x < planes.height->numX(); x += state->stride) {   
        for (y = which+state->stride; y >= which; y -= state->stride) { 
            if (describe_gl_vertex(state, planes, surfaceColor,x,y,
                                   &(vertexArray[i]))) {
                fprintf(stderr, "spm_x_strip:  describe_gl_vertex() failed.\n");
                return(-1);
            }
            i++;
        }
    }
    
    if (state->VERTEX_ARRAY) {
        specify_vertexArray(state, vertexArray, &i, 1);
    }
    else {
        glEnd();
        VERBOSE(20, "          glEnd()");
    }
    return 0;
}
*/
/**
 This version of spm_x_strip uses an assumed masking plane to decide
 what to draw and what not to draw.  Since there could be situations
 where every 2 points alternated on whether they were set to be drawn
 or not, and since we need 3 points for a triangle, a quad based decision
 metric is used to determine whether to draw or not.  What this means is
 that the decision whether to draw is made at the quad level, so that if
 1 point in the current quad should not be drawn, then the entire quad is
 not drawn.   Since there is a variable that tells this function to invert
 the meaning of the masking plane, in that case if any point should not be
 drawn, then the entire quad is draw. This ensures that 2 passes that use
 the masking plane and the inverted masking plane, will match exactly at
 the "seams".   

 The final result of this procedure will be an unspecified number of tristrips,
 instead of 1 tripstrip like in the above procedure.
 */

int spm_x_strip_masked(nmg_State * state,
                       const nmb_PlaneSelection &planes, nmg_SurfaceMask *mask,
                       GLdouble surfaceColor[3], int which,
                       Vertex_Struct * vertexArray)
{      
    int     x,y;
    int    i;	
    
    // Make sure we found a height plane
    if (planes.height == NULL) {
        fprintf(stderr, "spm_x_strip_masked: could not get grid!\n");
        return -1;
    }
    
    // Make sure they asked for a legal strip (in range and on stride)
    if ((which < 0) || (which >= planes.height->numY() - 1)) {
        fprintf(stderr, "Strip %d is outside plane.\n", which);
        return(-1);
    }
    
    if (which % state->stride) {
        fprintf(stderr, "Strip %d is off stride.\n", which);
        return(-1);
    }
    
    int number_of_strips = 0;
    bool skipping = false;
    //Multiplying by 3 is more space than needed
    //but it's simple and we can need some
    //unknown amount more than 2 if state->stride is 1
    int *x_array = new int[planes.height->numX() * 3];  
    int *y_array = new int[planes.height->numX() * 3];  
    i = 0;
    
    x = 0;
    y = which + state->stride;

    //Check the first quad to see if it is partially masked or not and
    //set up the state variables so that the following loop will run
    //correctly.
    if (mask->quadMasked(x,y,state->stride))
    {
        //If the first quad shouldn't be drawn, tell the loop
        //that it should be currently skipping quads.
        skipping = true;
    }
    else {
        //If the first quad should be draw, then drop the first
        //2 points of the quad into the x and y point arrays.
        x_array[i] = x; x_array[i+1] = x;
        y_array[i] = y; y_array[i+1] = y - state->stride;
        i+=2;
    }
    //We make the decision whether to draw points on a quad basis, but we 
    //store only the "first" two points of that quad.  This is because we
    //are really wanting to output a tristrip.  A tristrip can equivalently
    //be thought of as a quad strip, so we know that the "second" set of 
    //points will be the first set of the next quad on the next pass.
    for (x = state->stride; x < planes.height->numX() - state->stride; x += state->stride) {   // Left->right
        bool quad_masked = mask->quadMasked(x,y,state->stride);
        if (!skipping) {
            //If we get in here, then we know that on the previous run
            //of the loop we decided that the entire quad should be 
            //displayed, so it will always be safe to drop the first
            //two points of the current quad, since they were the second
            //points of the last quad, which we know to be good.
            x_array[i] = x; x_array[i+1] = x;
            y_array[i] = y; y_array[i+1] = y - state->stride;
            i+=2;
            if (quad_masked)
            {
                //If we had some good values before, and we have decided
                //that the current quad shouldn't be drawn, then set the
                //skipping bool to true and throw some bogus values into
                //the x and y arrays, so that in the next stage we will
                //know when 1 tristrip has ended and we need to start up
                //a new one.
                skipping = true;
                x_array[i] = -1;
                y_array[i] = -1;
                i++;
                number_of_strips++;
            }
        }
        else if (skipping) {
            if (!quad_masked)
            {
                //We had some unknown amount of quads that we decided
                //shouldn't be displayed and have found 1 that should.
                //So set the skipping variable to true and store the
                //first two points.
                skipping = false;
                x_array[i] = x; x_array[i+1] = x;
                y_array[i] = y; y_array[i+1] = y - state->stride;
                i+=2;
            }
        }
    }
    
    //Since we only drop the first two points of good quads
    //per loop, if we exit and we weren't skipping then we
    //need to go ahead and put in the last 2 points of the
    //last quad
    if (!skipping) {
        x_array[i] = x; x_array[i+1] = x;
        y_array[i] = y; y_array[i+1] = y - state->stride;
        i+=2;
        number_of_strips++;
    }
    else {
        //Otherwise decrement the index into the arrays of
        //points
        i--;
    }
    
    int max = i;
    int count = 0;
    int *vert_counts;
    int strip_i = 0;
    int vert = 0;
    

    //Now draw the valid strips that we have determined should be
    //formed from the information in the masking plane.
    if (!state->VERTEX_ARRAY) {
        glBegin(GL_TRIANGLE_STRIP);
        VERBOSE(20, "          glBegin(GL_TRIANGLE_STRIP)");
    }
    
    if (number_of_strips > 0) {
        vert_counts = new int[number_of_strips];
        
        for (i = 0; i < max; i++) {
            x = x_array[i];
            y = y_array[i];
            if (x == -1 && y == -1) {
                if (state->VERTEX_ARRAY) {
                    vert_counts[strip_i] = count;
                    count = 0;
                    strip_i++;
                }
                else {
                    glEnd();
                    glBegin(GL_TRIANGLE_STRIP);
                }
            }
            else {
                if (describe_gl_vertex(state, planes, 
                                       surfaceColor,x,y,&(vertexArray[vert]))) {
                    fprintf(stderr, "spm_x_strip_masked: "
                            " describe_gl_vertex() failed.\n");
                    return(-1);
                }
                vert++;
                count++;
            }
        }
        
        if (state->VERTEX_ARRAY) {
            vert_counts[strip_i] = count;
            specify_vertexArray(state, vertexArray, 
                                vert_counts, number_of_strips);
        }
        else {
            glEnd();
            VERBOSE(20, "          glEnd()");
        }
        
        
        delete [] vert_counts;
    }
    
    delete [] y_array;
    delete [] x_array;
    
    return 0;
}




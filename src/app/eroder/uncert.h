#ifndef _MYGLU_H_
#define _MYGLU_H_

#include <stdio.h>
#include <GL/glut_UNC.h>
#include "Tips.h"
#include "defns.h"

struct myGLUquadric
{
  GLenum DrawStyle;            /* GLU_FILL, LINE, SILHOUETTE, or POINT */
  GLenum Orientation;          /* GLU_INSIDE or GLU_OUTSIDE */
  GLboolean TextureFlag;       /* Generate texture coords? */
  GLenum Normals;              /* GLU_NONE, GLU_FLAT, or GLU_SMOOTH */
  //  void (GLCALLBACK  *ErrorFunc) (GLenum err);  /* Error handler callback function */
  void (*ErrorFunc) (GLenum err);  /* Error handler callback function */
};

typedef struct myGLUquadric myGLUquadricObj;

myGLUquadricObj *mygluNewQuadric(void);
void mygluDeleteQuadric(myGLUquadricObj * state);
void mygluQuadricDrawStyle(myGLUquadricObj * quadObject, GLenum drawStyle);
void mygluQuadricOrientation(myGLUquadricObj * quadObject, GLenum orientation);
void mygluQuadricNormals(myGLUquadricObj * quadObject, GLenum normals);

void uncert_sphere(myGLUquadricObj * qobj, GLdouble radius, GLint slices, GLint stacks);
void uncert_frustum(myGLUquadricObj * qobj,
		   GLdouble baseRadius, GLdouble topRadius,
		   GLdouble height, GLint slices, GLint stacks,
		    GLfloat grad_l, GLfloat grad_r);

#if DISP_LIST
void make_uncert_sphere();
void make_uncert_cylinder();
void make_uncert_cone_sphere(InvConeSphereTip ics);
#endif

void draw_uncert_sphere( double diameter);
void draw_uncert_cylinder(double diameter, double height);
void draw_uncert_frustum(double bottomdiameter, double topdiameter, double height);
GLfloat get_sphere_color_rho(GLfloat rho);
void set_sphere_color_z(GLfloat z);
GLfloat get_sphere_color_z(GLfloat z);
#endif

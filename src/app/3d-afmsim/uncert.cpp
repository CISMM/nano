/* Gokul Varadhan
 * varadhan@cs.unc.edu
 * May 2001
 */
#include <stdlib.h>
#include <stdio.h>	
#include <math.h>	
#include <GL/glut.h>
#include <string.h>
#include "ConeSphere.h"
#include "defns.h"
#include "uncert.h"

#define SPHERE_LIST 1
#define CYLINDER_LIST 2

/* Picked most of the following code directly from quadric.c */

#ifndef M_PI
#  define M_PI (3.1415926)
#endif

extern int tesselation;


#define DEBUG 1

/*
 * Convert degrees to radians:
 */
#define MY_DEG_TO_RAD(A)   ((A)*(M_PI/180.0))


/*
 * Sin and Cos for degree angles:
 */
#define SIND( A )   sin( (A)*(M_PI/180.0) )
#define COSD( A)    cos( (A)*(M_PI/180.0) )


/*
 * Texture coordinates if texture flag is set
 */
#define TXTR_COORD(x,y)    if (qobj->TextureFlag) glTexCoord2f(x,y);


/*
 * Process a GLU error.
 */
static void myquadric_error(myGLUquadricObj * qobj, GLenum error, const char *msg)
{
  /* Call the error call back function if any */
  if (qobj->ErrorFunc) {
    (*qobj->ErrorFunc) (error);
  }
  /* Print a message to stdout if MESA_DEBUG variable is defined */
  if (getenv("MESA_DEBUG")) {
    fprintf(stderr, "GLUError: %s: %s\n", (char *) gluErrorString(error),
	    msg);
  }
}




myGLUquadricObj *mygluNewQuadric(void)
{
  myGLUquadricObj *q;

  q = (myGLUquadricObj *) malloc(sizeof(struct myGLUquadric));
  if (q) {
    q->DrawStyle = GLU_FILL;
    q->Orientation = GLU_OUTSIDE;
    q->TextureFlag = GL_FALSE;
    q->Normals = GLU_SMOOTH;
    q->ErrorFunc = NULL;
  }
  return q;
}



void mygluDeleteQuadric(myGLUquadricObj * state)
{
  if (state) {
    free((void *) state);
  }
}

/*
 * Set the drawing style to be GLU_FILL, GLU_LINE, GLU_SILHOUETTE,
 * or GLU_POINT.
 */
void mygluQuadricDrawStyle(myGLUquadricObj * quadObject, GLenum drawStyle)
{
  if (quadObject && (drawStyle == GLU_FILL || drawStyle == GLU_LINE
		     || drawStyle == GLU_SILHOUETTE
		     || drawStyle == GLU_POINT)) {
    quadObject->DrawStyle = drawStyle;
  }
  else {
    myquadric_error(quadObject, GLU_INVALID_ENUM, "qluQuadricDrawStyle");
  }
}

void mygluQuadricNormals(myGLUquadricObj * quadObject, GLenum normals)
{
  if (quadObject
      && (normals == GLU_NONE || normals == GLU_FLAT
	  || normals == GLU_SMOOTH)) {
    quadObject->Normals = normals;
  }
}


/*
 * Call glNormal3f after scaling normal to unit length.
 */
static void
normal3f(GLfloat x, GLfloat y, GLfloat z)
{
  GLdouble mag;

  mag = sqrt(x * x + y * y + z * z);
  if (mag > 0.00001F) {
    x /= mag;
    y /= mag;
    z /= mag;
  }
  glNormal3f(x, y, z);
}

/* We want to color the upper half of the sphere (z=0 to z=1) as per a gradient
 *
 * We want the top z=1 to have the color grad_r * current color
 *
 * And we want z=0 to have color grad_l * current color
 *
 * We choose a linear gradient
 */
void set_frustum_color_z(GLfloat z, GLdouble height, GLfloat grad_l, GLfloat grad_r) {
  GLfloat col[4];

  GLfloat t = (grad_l + (grad_r-grad_l)*(z/height));

  col[0] = col[1] = col[2] = t;
  col[3] = 1;

  glColor4fv(col);
}

// uncertainty map for the conical part of the AFM of a sphere
void uncert_frustum(myGLUquadricObj * qobj,
		   GLdouble baseRadius, GLdouble topRadius,
		   GLdouble height, GLint slices, GLint stacks,
		   GLfloat grad_l, GLfloat grad_r)
{
  GLdouble da, r, dr, dz;
  GLfloat z, nz, nsign;
  GLint i, j;

  GLfloat color[4];

  glGetFloatv(GL_CURRENT_COLOR, color);


  if (qobj->Orientation == GLU_INSIDE) {
    nsign = -1.0;
  }
  else {
    nsign = 1.0;
  }

  da = 2.0 * M_PI / slices;
  dr = (topRadius - baseRadius) / stacks;
  dz = height / stacks;
  nz = (baseRadius - topRadius) / height;      /* Z component of normal vectors */

  if (qobj->DrawStyle == GLU_FILL) {
    GLfloat ds = 1.0 / slices;
    GLfloat dt = 1.0 / stacks;
    GLfloat t = 0.0;
    z = 0.0;
    r = baseRadius;

    for (j = 0; j < stacks; j++) {
      GLfloat s = 0.0;
      glBegin(GL_QUAD_STRIP);

      for (i = 0; i <= slices; i++) {
	GLfloat x, y;
	if (i == slices) {
	  x = sin(0.0);
	  y = cos(0.0);
	}
	else {
	  x = sin(i * da);
	  y = cos(i * da);
	}

	set_frustum_color_z(z,height,grad_l, grad_r);


	if (nsign == 1.0) {
	  normal3f(x * nsign, y * nsign, nz * nsign);
	  TXTR_COORD(s, t);
	  glVertex3f(x * r, y * r, z);
	  normal3f(x * nsign, y * nsign, nz * nsign);
	  TXTR_COORD(s, t + dt);
	  glVertex3f(x * (r + dr), y * (r + dr), z + dz);
	}
	else {
	  normal3f(x * nsign, y * nsign, nz * nsign);
	  TXTR_COORD(s, t);
	  glVertex3f(x * r, y * r, z);
	  normal3f(x * nsign, y * nsign, nz * nsign);
	  TXTR_COORD(s, t + dt);
	  glVertex3f(x * (r + dr), y * (r + dr), z + dz);
	}
	s += ds;
      }                        /* for slices */
      glEnd();
      r += dr;
      t += dt;
      z += dz;
    }                         /* for stacks */
  }
}

/* basically color left part of the unit cylinder (aligned along z-axis) 
 * according to a gradient
 */
void set_cylinder_color_rho(GLfloat rho) {
  GLfloat col[4];

  if (rho >= M_PI) {
    // offset rho by M_PI/2 (because for a slice rho=0 for pt x=0. y=1
    // whereas we want to color starting at pt x=-1, y=0

    //    GLfloat t = (1 - fabs(rho-1.5*M_PI)/(M_PI/2.));
    GLfloat newrho = fabs(rho-1.5*M_PI);
    GLfloat t = get_sphere_color_rho(newrho);

    col[0] = col[1] = col[2] = t;
  }
  else {
    col[0] = col[1] = col[2] = 0;
  }

  col[3] = 1.;
  glColor4fv(col);

  //  glColor3f(0,1,0);
}


// uncertainty map for the cylindrical part of the SSL
void uncert_cylinder(myGLUquadricObj * qobj,
		     GLdouble baseRadius, GLdouble topRadius,
		     GLdouble height, GLint slices, GLint stacks)
{
  GLdouble da, r, dr, dz;
  GLfloat z, nz, nsign;
  GLint i, j;

  GLfloat color[4];

  glGetFloatv(GL_CURRENT_COLOR, color);


  if (qobj->Orientation == GLU_INSIDE) {
    nsign = -1.0;
  }
  else {
    nsign = 1.0;
  }

  da = 2.0 * M_PI / slices;
  dr = (topRadius - baseRadius) / stacks;
  dz = height / stacks;
  nz = (baseRadius - topRadius) / height;      /* Z component of normal vectors */

  if (qobj->DrawStyle == GLU_FILL) {
    GLfloat ds = 1.0 / slices;
    GLfloat dt = 1.0 / stacks;
    GLfloat t = 0.0;
    z = 0.0;
    r = baseRadius;
    for (j = 0; j < stacks; j++) {
      GLfloat s = 0.0;
      glBegin(GL_QUAD_STRIP);
      for (i = 0; i <= slices; i++) {
	GLfloat x, y;
	if (i == slices) {
	  x = sin(0.0);
	  y = cos(0.0);
	}
	else {
	  x = sin(i * da);
	  y = cos(i * da);
	}

	set_cylinder_color_rho(i*da);

	if (nsign == 1.0) {
	  normal3f(x * nsign, y * nsign, nz * nsign);
	  TXTR_COORD(s, t);
	  glVertex3f(x * r, y * r, z);
	  normal3f(x * nsign, y * nsign, nz * nsign);
	  TXTR_COORD(s, t + dt);
	  glVertex3f(x * (r + dr), y * (r + dr), z + dz);
	}
	else {
	  normal3f(x * nsign, y * nsign, nz * nsign);
	  TXTR_COORD(s, t);
	  glVertex3f(x * r, y * r, z);
	  normal3f(x * nsign, y * nsign, nz * nsign);
	  TXTR_COORD(s, t + dt);
	  glVertex3f(x * (r + dr), y * (r + dr), z + dz);
	}
	s += ds;
      }                        /* for slices */
      glEnd();
      r += dr;
      t += dt;
      z += dz;
    }                         /* for stacks */
  }
}




/* Uncertainty map for the sphere 
 *
 * We want to color the upper half of the sphere (z=0 to z=1) as per a gradient
 *
 * We want the top z=1 to have the color grad_r * current color
 *
 * And we want z=0 to have color grad_l * current color
 *
 * We choose a linear gradient
 */
void set_sphere_color_rho(GLfloat rho) {
  GLfloat col[4];
  GLfloat t;

  if (rho < M_PI/2) {

    t = (1 - rho/(M_PI/2.));

    col[0] = col[1] = col[2] = t;
  }
  else {
    col[0] = col[1] = col[2] = 0.;
  }
  col[3] = 1.;

  glColor4fv(col);
}

// gray color
GLfloat get_sphere_color_rho(GLfloat rho) {

  if (rho < M_PI/2.) {

    GLfloat t = (1 - rho/(M_PI/2.));

    return t;
  }
  return 0.;
}

/* Uncertainty map for the sphere 
 * sphere is colored according to a gradient in the range 
 * current_color *[grad_l, grad_r]. 
 */
void uncert_sphere(myGLUquadricObj * qobj, GLdouble radius, GLint slices, GLint stacks)
{
  GLfloat rho, drho, theta, dtheta;
  GLfloat x, y, z;
  GLfloat s, t, ds, dt;
  GLint i, j, imin, imax;
  GLboolean normals;
  GLfloat nsign;

  GLfloat color[4];

  glGetFloatv(GL_CURRENT_COLOR, color);


  if (qobj->Normals == GLU_NONE) {
    normals = GL_FALSE;
  }
  else {
    normals = GL_TRUE;
  }
  if (qobj->Orientation == GLU_INSIDE) {
    nsign = -1.0;
  }
  else {
    nsign = 1.0;
  }

  drho = M_PI / (GLfloat) stacks;
  dtheta = 2.0 * M_PI / (GLfloat) slices;

  /* texturing: s goes from 0.0/0.25/0.5/0.75/1.0 at +y/+x/-y/-x/+y axis */
  /* t goes from -1.0/+1.0 at z = -radius/+radius (linear along longitudes) */
  /* cannot use triangle fan on texturing (s coord. at top/bottom tip varies) */

  if (qobj->DrawStyle == GLU_FILL) {
    if (!qobj->TextureFlag) {
      /* draw +Z end as a triangle fan */
      glBegin(GL_TRIANGLE_FAN);
      glNormal3f(0.0, 0.0, 1.0);

      set_sphere_color_rho(0);

      glVertex3f(0.0, 0.0, nsign * radius);

      for (j = 0; j <= slices; j++) {
	theta = (j == slices) ? 0.0 : j * dtheta;
	x = -sin(theta) * sin(drho);
	y = cos(theta) * sin(drho);
	z = nsign * cos(drho);

	if (normals)
	  glNormal3f(x * nsign, y * nsign, z * nsign);

	set_sphere_color_rho(drho);


	glVertex3f(x * radius, y * radius, z * radius);
      }
      glEnd();
    }

    ds = 1.0 / slices;
    dt = 1.0 / stacks;
    t = 1.0;                  /* because loop now runs from 0 */
    if (qobj->TextureFlag) {
      imin = 0;
      imax = stacks;
    }
    else {
      imin = 1;
      imax = stacks - 1;
    }

    /* draw intermediate stacks as quad strips */
    for (i = imin; i < imax; i++) {
      rho = i * drho;
      glBegin(GL_QUAD_STRIP);
      s = 0.0;
      for (j = 0; j <= slices; j++) {
	theta = (j == slices) ? 0.0 : j * dtheta;
	x = -sin(theta) * sin(rho);
	y = cos(theta) * sin(rho);
	z = nsign * cos(rho);
	if (normals)
	  glNormal3f(x * nsign, y * nsign, z * nsign);
	TXTR_COORD(s, t);

	set_sphere_color_rho(rho);

	glVertex3f(x * radius, y * radius, z * radius);
	x = -sin(theta) * sin(rho + drho);
	y = cos(theta) * sin(rho + drho);
	z = nsign * cos(rho + drho);
	if (normals)
	  glNormal3f(x * nsign, y * nsign, z * nsign);
	TXTR_COORD(s, t - dt);
	s += ds;

	set_sphere_color_rho(rho+drho);
	glVertex3f(x * radius, y * radius, z * radius);
      }
      glEnd();
      t -= dt;
    }

    if (!qobj->TextureFlag) {
      /* draw -Z end as a triangle fan */
      glBegin(GL_TRIANGLE_FAN);
      glNormal3f(0.0, 0.0, -1.0);
      glVertex3f(0.0, 0.0, -radius * nsign);
      rho = M_PI - drho;
      s = 1.0;
      t = dt;
      for (j = slices; j >= 0; j--) {
	theta = (j == slices) ? 0.0 : j * dtheta;
	x = -sin(theta) * sin(rho);
	y = cos(theta) * sin(rho);
	z = nsign * cos(rho);
	if (normals)
	  glNormal3f(x * nsign, y * nsign, z * nsign);
	s -= ds;

	set_sphere_color_rho(rho);
	
	glVertex3f(x * radius, y * radius, z * radius);
      }
      glEnd();
    }
  }

  // restore original color
  glColor4fv(color);
}

#if DISP_LIST
void make_uncert_sphere() {
  static int firstTime = 1;
  static myGLUquadricObj* qobj;
  if( firstTime ) { 
    qobj = mygluNewQuadric();
    mygluQuadricDrawStyle( qobj, GLU_FILL);
    mygluQuadricNormals( qobj, GLU_FLAT );
    firstTime=0;
  }
  glNewList(UNCERT_SPHERE_LIST, GL_COMPILE);
  uncert_sphere( qobj, 1., tesselation, tesselation);
  glEndList();
}

void make_uncert_cylinder() {
  static int firstTime = 1;
  static myGLUquadricObj* qobj;
  if( firstTime ) { 
    qobj = mygluNewQuadric();
    mygluQuadricDrawStyle( qobj, GLU_FILL);
    mygluQuadricNormals( qobj, GLU_FLAT );
    firstTime=0;
  }

  glNewList(UNCERT_CYLINDER_LIST, GL_COMPILE);
  uncert_cylinder( qobj, 1., 1., 1., tesselation, tesselation);
  glEndList();
}

void make_uncert_cone_sphere(InvConeSphereTip ics) {
  static myGLUquadricObj* qobj;

  //  printf("Building ConeSphere display list (tesselation %d)\n",tesselation);

  static int first_time=1;
  if (first_time) {
    first_time=0;
    qobj = mygluNewQuadric();
    mygluQuadricDrawStyle( qobj, GLU_FILL);
    mygluQuadricNormals( qobj, GLU_FLAT );
  }

  // here we consider the AFM of a sphere of 0 radius with a tip of radius 1.
  double theta = ics.theta; // theta for the tip
  
  double bignum = 200; // some big no for now
  
  double cone_height = bignum + (1.)/sin(theta);
  ConeSphere c = ConeSphere(1., cone_height, theta);
  
  // Create a display list for a sphere
  glNewList(UNCERT_CONE_SPHERE_LIST, GL_COMPILE);
  uncert_sphere( qobj, 1., tesselation, tesselation);

  GLfloat gcol = get_sphere_color_rho(PI/2.-theta);
  
  glPushMatrix();
  glTranslatef(0,0,-bignum);
  uncert_frustum( qobj, c.cr, c.topRadius, c.topHeight, tesselation, tesselation,0,gcol);
  glPopMatrix();
  glEndList();

  glFlush();
  glFinish();

}

#endif


void draw_uncert_sphere( double diameter)
{
#if (DISP_LIST == 0)
  static int firstTime = 1;
  static myGLUquadricObj* qobj;
  if( firstTime ) { 
    qobj = mygluNewQuadric();
    mygluQuadricDrawStyle( qobj, GLU_FILL);
    mygluQuadricNormals( qobj, GLU_FLAT );
    firstTime=0;
  }
  uncert_sphere( qobj, diameter/2., tesselation, tesselation);
#else
  glPushMatrix();
  glScalef(diameter/2., diameter/2., diameter/2);
  glCallList(UNCERT_SPHERE_LIST);
  glPopMatrix();
#endif
}

void draw_uncert_cylinder(double diameter, double height) {
#if (DISP_LIST == 0)
  static int firstTime = 1;
  static myGLUquadricObj* qobj;
  if( firstTime ) { 
    qobj = mygluNewQuadric();
    mygluQuadricDrawStyle( qobj, GLU_FILL);
    mygluQuadricNormals( qobj, GLU_FLAT );
    firstTime=0;
  }
  uncert_cylinder( qobj, diameter/2., diameter/2., height, tesselation, tesselation);
#else
  glPushMatrix();
  glScalef(diameter/2.,diameter/2.,height);
  glCallList(UNCERT_CYLINDER_LIST);
  glPopMatrix();
#endif
}

void draw_uncert_frustum(double bottomdiameter, double topdiameter, double height) {
  static int firstTime = 1;
  static myGLUquadricObj* qobj;
  if( firstTime ) { 
    qobj = mygluNewQuadric();
    mygluQuadricDrawStyle( qobj, GLU_FILL);
    mygluQuadricNormals( qobj, GLU_FLAT );
    firstTime=0;
  }
  uncert_frustum( qobj, bottomdiameter/2., topdiameter/2., height, tesselation, tesselation, 0, 1);
}



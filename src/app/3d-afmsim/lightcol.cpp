#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>
#include "defns.h"
#include "lightcol.h"
#include "sim.h"

//GLenum shadingModel = GL_FLAT;   // GL_FLAT or GL_SMOOTH
GLenum shadingModel = GL_SMOOTH;   // GL_FLAT or GL_SMOOTH
Bool lightOn[8] = { 1, 1, 0, 0, 0, 0, 0, 0 };

void lighting( void )
{
  if (uncertainty_mode) {
    glDisable( GL_LIGHTING );
    if( shadingModel == GL_FLAT ) {
      glShadeModel(GL_FLAT);
    }
    else {
      glShadeModel(GL_SMOOTH);
    }
    return;
  }

  if( shadingModel == GL_FLAT ) {
    glShadeModel(GL_FLAT);
    glDisable( GL_LIGHTING );
  }
  else {
    GLfloat light_ambient[]  = { 0.5, 0.5, 0.5, 1.0 };
    GLfloat light_diffuse[]  = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat light_position0[] = {  1.0, 1.0, 1.0, 0.0 };
    GLfloat light_position1[] = { -1.0, 1.0, 0.0, 0.0 };
		

    glShadeModel(GL_SMOOTH);

    glLightfv( GL_LIGHT0,  GL_AMBIENT,  light_ambient );
    glLightfv( GL_LIGHT0,  GL_DIFFUSE,  light_diffuse );
    glLightfv( GL_LIGHT0,  GL_SPECULAR, light_specular );
    glLightfv( GL_LIGHT0,  GL_POSITION, light_position0 );

    glLightfv( GL_LIGHT1,  GL_AMBIENT,  light_ambient );
    glLightfv( GL_LIGHT1,  GL_DIFFUSE,  light_diffuse );
    glLightfv( GL_LIGHT1,  GL_SPECULAR, light_specular );
    glLightfv( GL_LIGHT1,  GL_POSITION, light_position1 );

    if( lightOn[0] )  glEnable(  GL_LIGHT0 );
    else              glDisable( GL_LIGHT0 );
    if( lightOn[1] )  glEnable(  GL_LIGHT1 );
    else              glDisable( GL_LIGHT1 );

    glLightModeli( GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE );
    glEnable( GL_LIGHTING );
  }
}

void setColor( int colorIndex ) {
  switch( colorIndex ) {
  case 0: setMaterialColor(1.0, 1.0, 1.0); break;  /* white */
  case 1: setMaterialColor(1.0, 0.0, 0.0); break;  /* red */
  case 2: setMaterialColor(0.0, 1.0, 0.0); break;  /* green */
  case 3: setMaterialColor(0.0, 0.0, 1.0); break;  /* blue */
  case 4: setMaterialColor(1.0, 0.0, 1.0); break;  /* magenta */
  case 5: setMaterialColor(1.0, 1.0, 0.0); break;  /* yellow */
  case 6: setMaterialColor(0.0, 1.0, 1.0); break;  /* cyan */
  case 7: setMaterialColor(0.0, 0.0, 0.0); break;  /* black */
  }
}

void setMaterialColor( GLfloat r, GLfloat g, GLfloat b ) {

  if (uncertainty_mode) {
    glColor3f( r, g, b );
    return;
  }

  if( shadingModel == GL_FLAT ) {
    glColor3f( r, g, b );
  }
  else {
    GLfloat mat_ambient[]    = { 0.2*r, 0.2*g, 0.2*b, 1.0 };
    GLfloat mat_diffuse[]    = { 0.8*r, 0.8*g, 0.8*b, 1.0 };
    GLfloat mat_specular[]   = { 1.0*r, 1.0*g, 1.0*b, 1.0 };
    GLfloat mat_shininess[]  = { 30.0 };

    glMaterialfv( GL_FRONT_AND_BACK,  GL_AMBIENT,  mat_ambient );
    glMaterialfv( GL_FRONT_AND_BACK,  GL_DIFFUSE,  mat_diffuse );
    glMaterialfv( GL_FRONT_AND_BACK,  GL_SPECULAR,  mat_specular );
    glMaterialfv( GL_FRONT_AND_BACK,  GL_SHININESS, mat_shininess );
  }
}





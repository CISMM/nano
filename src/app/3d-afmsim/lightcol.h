#ifndef _LIGHTCOL_H_
#define _LIGHTCOL_H_

extern GLenum shadingModel;

void lighting( void );
void setColor( int colorIndex );
void setMaterialColor( GLfloat r, GLfloat g, GLfloat b );

#endif

#include "URVector.h"
#include "quat.h"
#include <GL/glut_UNC.h>

URVector::URVector(double start_x, double start_y, double start_z,
	  double dir_x, double dir_y, double dir_z):URender()
{
  set(start_x, start_y, start_z, dir_x, dir_y, dir_z);
  d_quadric = gluNewQuadric();
  gluQuadricDrawStyle(d_quadric, GLU_FILL);
  gluQuadricNormals(d_quadric, GLU_FLAT);
}

URVector::~URVector(){
  if (d_quadric) {
    gluDeleteQuadric(d_quadric);
  }
  return;
}

void URVector::set(double start_x, double start_y, double start_z,
	  double dir_x, double dir_y, double dir_z)
{
	d_start[0] = start_x; 
	d_start[1] = start_y; 
	d_start[2] = start_z;
	d_end[0] = start_x+dir_x; 
	d_end[1] = start_y+dir_y; 
	d_end[2] = start_z+dir_z;
}

int URVector::Render(void * /*userdata*/ )
{
  bool simpleModel = false;

  if(visible){
	glColor3f(1.0, 1.0, 1.0);
	q_type quat;
	q_vec_type v1 = {0.0, 0.0, 1.0};
	q_vec_type v2;
	v2[0] = d_end[0] - d_start[0];
	v2[1] = d_end[1] - d_start[1];
	v2[2] = d_end[2] - d_start[2];
	double vecLength = q_vec_magnitude(v2);
	q_vec_normalize(v2, v2);
	q_from_two_vecs(quat, v1, v2);
	qogl_matrix_type matrix;
	q_to_ogl_matrix(matrix, quat);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslated(d_start[0], d_start[1], d_start[2]); 
	glMultMatrixd(matrix);
	if (simpleModel) {
		glBegin(GL_LINES);
		glVertex3d(0,0,0);
		glVertex3d(0,0,vecLength);
		glEnd();
	} else {
		double cylinderRadius = 0.05*vecLength;
		double cylinderLength = 0.6*vecLength;
		double coneRadius = 0.1*vecLength;
		double coneLength = 0.4*vecLength;
		gluCylinder(d_quadric, cylinderRadius, cylinderRadius, 
			cylinderLength, 30, 30);
		glTranslated(0.0,0.0,cylinderLength);
		gluDisk(d_quadric, 0.0, coneRadius, 30, 1);
		gluCylinder(d_quadric, coneRadius, 0.0, coneLength, 30,30);
	}
	glPopMatrix();
  }

  if(recursion) return  ITER_CONTINUE;
  else return ITER_STOP;
  
}
#include "URAxis.h"

URAxis::URAxis():URender()
{
  obj_type=URAXIS;
}

URAxis::~URAxis(){
	return;
}

#define ASIZE 100
int URAxis::Render(void * /*userdata*/ )
{
  if(visible){
	glBegin(GL_LINES);

	glColor3f(ASIZE,0,0);
	glVertex3f(0,0,0);
	glVertex3f(ASIZE,0,0);

  	glColor3f(0,ASIZE,0);
  	glVertex3f(0,0,0);
	glVertex3f(0,ASIZE,0);
	
	glColor3f(0,0,ASIZE);
	glVertex3f(0,0,0);
	glVertex3f(0,0,ASIZE);
	
	glEnd();
	glColor3f(ASIZE,0,0);
  }

  if(recursion) return  ITER_CONTINUE;
  else return ITER_STOP;
  
}

int URAxis::Scale(void *) 
{
	// Do nothing
	if(recursion) return  ITER_CONTINUE;
	else return ITER_STOP;
}

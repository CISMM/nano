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

int URAxis::SetVisibilityAll(void* /*userdata*/) {
	// do nothing
	if(recursion) return  ITER_CONTINUE;
	else return ITER_STOP;
}

int URAxis::SetProjTextAll(void* /*userdata*/) {
	// do nothing
	if(recursion) return  ITER_CONTINUE;
	else return ITER_STOP;
}

int URAxis::ScaleAll(void* /*userdata*/) {
	// do nothing
	if(recursion) return  ITER_CONTINUE;
	else return ITER_STOP;
}

int URAxis::SetTransxAll(void* /*userdata*/) {
	// do nothing
	if(recursion) return  ITER_CONTINUE;
	else return ITER_STOP;
}

int URAxis::SetTransyAll(void* /*userdata*/) {
	// do nothing
	if(recursion) return  ITER_CONTINUE;
	else return ITER_STOP;
}

int URAxis::SetTranszAll(void* /*userdata*/) {
	// do nothing
	if(recursion) return  ITER_CONTINUE;
	else return ITER_STOP;
}

int URAxis::SetRotxAll(void* /*userdata*/) {
	// do nothing
	if(recursion) return  ITER_CONTINUE;
	else return ITER_STOP;
}

int URAxis::SetRotyAll(void* /*userdata*/) {
	// do nothing
	if(recursion) return  ITER_CONTINUE;
	else return ITER_STOP;
}

int URAxis::SetRotzAll(void* /*userdata*/) {
	// do nothing
	if(recursion) return  ITER_CONTINUE;
	else return ITER_STOP;
}

int URAxis::SetColorAll(void* /*userdata*/) {
	// do nothing
	if(recursion) return  ITER_CONTINUE;
	else return ITER_STOP;
}

int URAxis::SetAlphaAll(void* /*userdata*/) {
	// do nothing
	if(recursion) return  ITER_CONTINUE;
	else return ITER_STOP;
}





int URAxis::ChangeStaticFile(void* /*userdata*/) {
	// do nothing
	if(recursion) return  ITER_CONTINUE;
	else return ITER_STOP;
}


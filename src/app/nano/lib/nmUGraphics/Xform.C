#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include "Xform.h"

Xform::Xform(q_vec_type t, q_type r, double s, int lt, int lr, int ls)
{
  q_copy(rot,r);
  q_vec_copy(trans,t);
  scale=s;
  lock_rot=lr;
  lock_scale=ls;
  lock_trans=lt;
}

Xform::Xform(q_vec_type t, q_type r, double s)
{
  q_copy(rot,r);
  q_vec_copy(trans,t);
  scale=s;
  lock_rot=lock_scale=lock_trans=0;
}


Xform::Xform()
{
  	rot[Q_X]=0;
	rot[Q_Y]=0;
	rot[Q_Z]=0;
	rot[Q_W]=1;
        lock_rot=lock_scale=lock_trans=0;
	trans[0]=trans[1]=trans[2]=0;
	scale=1;
}

void Xform::SetTransLock(int i){	
	if(i==0) lock_trans=0;
	else lock_trans=1;
}

void Xform::SetRotLock(int i){	
	if(i==0) lock_rot=0;
	else lock_rot=1;
}

void Xform::SetScaleLock(int i){
	if(i==0) lock_scale=0;
	else lock_scale=1;	
}

void Xform::AddRotate(q_type q){
	q_mult(rot,rot,q);
}

void Xform::SetRotate(q_type q){
	rot[Q_X]=q[Q_X]; rot[Q_Y]=q[Q_Y]; rot[Q_Z]=q[Q_Z]; rot[Q_W]=q[Q_W];
}

void Xform::SetRotate(double rx, double ry, double rz, double rw){
	rot[Q_X]=rx; rot[Q_Y]=ry; rot[Q_Z]=rz; rot[Q_W]=rw;
}

void Xform::SetTranslate(q_vec_type t){
	trans[0]=t[0];trans[1]=t[1];trans[2]=t[2];
        cerr<<"Entering SetTranslate\n";
}

void Xform::SetTranslate(double tx, double ty, double tz){
	trans[0]=tx;trans[1]=ty;trans[2]=tz;
}

void Xform::AddTranslate(q_vec_type t){
	trans[0]+=t[0];trans[1]+=t[1];trans[2]+=t[2];
}

void Xform::SetScale(double s){
	scale=s;
}

void Xform::Push_As_OGL(){
  GLdouble m[4][4];

  //I THINK this should be a column vector format but GL seems to do it strangley
  //with quat lib rotations ... so I'm using row vector instead
  //this is a HACK to fix it -- DON'T do this for actually printing
  //because the matrix will come out funny... but for actually pushing on 
  //the stack the m[4][4] arrangement from quatlib is weird when OGL is 
  //expecting an m[16].  I think the order of references is what causes
  //the problem

  //scale the point, then rotate it, then translate
  glTranslated(trans[0],trans[1],trans[2]); 
  q_to_row_matrix(m,rot);
  glMultMatrixd(&m[0][0]);
  glScaled(scale,scale,scale);
 
  /*q_to_col_matrix(m,rot);
  int i,j;
  for(i=0; i < 3; i++){
	for(j=0; j<3; j++){
	  m[i][j]=m[i][j]*scale;
	}
  }
  m[0][3]=trans[0];
  m[1][3]=trans[1];
  m[2][3]=trans[2];
  glMultMatrixd(&m[0][0]);*/

  return;
}

ostream& operator<< (ostream& co,const Xform& x){
	int i;
	co << "Trans: " << ends;
	for(i=0; i<3; i++){ co << x.trans[i] << " ";} 
	co << " Quat: " << ends;
	for(i=0; i<4; i++){ co << x.rot[i] << " ";} 
	co << " Scale: " << x.scale << endl;
	return co;
}

void Xform::print(int val){
  
   int i,j;
   if(val==XP_4BY4){
  	q_matrix_type m;
	q_to_col_matrix(m,rot);	//rotation
	for(i=0; i < 3; i++){	//scale
		for(j=0; j<3; j++){
		  m[i][j]=m[i][j]*scale;
		}
	}
	m[0][3]=trans[0];	//translate
	m[1][3]=trans[1];
	m[2][3]=trans[2];
  
	for(i=0; i< 4;i++){
		for(j=0; j <4; j++){
			cerr << m[i][j] <<"\t\t";
		}
		cerr <<"\n";
  	}
   }

   else if(val==XP_RAW){
	cerr << "T: " << ends;
	for(i=0; i<3; i++){ cerr << trans[i] << " ";} 
	cerr << " R: " << ends;
	for(i=0; i<4; i++){ cerr << rot[i] << " ";} 
	cerr << " S: " << scale << endl;
   }

}

	
void Xform::invert()
{
  int i;
  q_vec_type temp;
  
  q_invert(rot,rot);
  for(i=0;i<3;i++) temp[i]=-trans[i];
  q_xform(trans,rot,temp);
  if(scale==0){
	cerr << "unexpected Zero scale ... cannot invert\n";
	return;
  }
  for(i=0;i<3;i++) trans[i]/=scale;
  scale=1/scale;
  
}

Xform& Xform::operator=(const Xform& src)
{
  if(this!=&src){
	//const casts are for quatlib which didn't declare const
	double *temprot = (double *)(void*)(src.rot);
	double *temptrans = (double *)(void*)(src.trans);
	q_copy(rot, temprot);
	q_vec_copy(trans, temptrans);
	scale=src.scale;
	lock_rot=src.lock_rot;
	lock_scale=src.lock_scale;
	lock_trans=src.lock_trans;
  }
  return (*this);
}

void Xform::apply(q_vec_type p, q_vec_type temp)
{
  int i;
  //scale first
  for(i=0;i < 3;i++){ temp[i]=p[i]*scale;}
  //then rotate
  q_xform(temp,rot,temp);
  //then translate
  for(i=0;i < 3;i++){ temp[i]+=trans[i];}
  return;
}

Xform Xform::compose(Xform& src)
{
  q_vec_type temp;
  q_type qtemp;
  apply(src.trans,temp);
  q_copy(temp,temp);
  q_mult(qtemp,rot,src.rot);
  return Xform(temp,qtemp,scale*src.scale);
}

Xform4x4 Xform::operator*(const Xform4x4& x){

        int i,j;
        q_matrix_type mtemp;
        q_to_col_matrix(mtemp,rot); //rotation
        for(i=0; i < 3; i++){   //scale
                for(j=0; j<3; j++){
                  mtemp[i][j]=mtemp[i][j]*scale;
                }
        }
        mtemp[0][3]=trans[0];       //translate
        mtemp[1][3]=trans[1];
        mtemp[2][3]=trans[2];

        return Xform4x4(mtemp)*x;
}





Xform Xform::operator*(const Xform& src)
{
  
  Xform src2;

  q_vec_type temp;
  q_type qtemp;
  double s;
  
  src2=src;
  if(lock_trans){
	src2.trans[0]=src2.trans[1]=src2.trans[2]=0;
  }
  if(lock_rot){
	src2.rot[0]=src2.rot[1]=src2.rot[2]=0;
	src2.rot[3]=1;
  }
  if(lock_scale) src2.scale=1;

  apply(src2.trans,temp);
  q_vec_copy(temp,temp);
  q_mult(qtemp,rot,src2.rot);
  s=scale*src2.scale;

  return Xform(temp,qtemp,s,lock_trans,lock_rot,lock_scale);

}

 
void PrintGLStack()
{
  GLdouble m[4][4];
  int i,j;
  
  glGetDoublev(GL_MODELVIEW_MATRIX,&m[0][0]);
  for(i=0; i < 4; i++){
	for(j=0; j < 4; j++){
	  cerr << m[j][i] << "\t";
	}
	cerr << "\n";
  }
  cerr << "\n";

  return;
}  
  

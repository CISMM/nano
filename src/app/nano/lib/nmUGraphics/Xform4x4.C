#include <stdlib.h>
#include "Xform4x4.h"


Xform4x4::Xform4x4(){
	int i,j;
	for(i=0; i < 4; i++)
	  for(j=0; j < 4; j++)
		m[i][j]=0;
	m[0][0]=m[1][1]=m[2][2]=m[3][3]=1;
}

Xform4x4::~Xform4x4(){
	return;
}

void Xform4x4::operator()(int i, int j, double val){
	if(i<4 && i>=0 && j<4 && j>=0){ m[i][j]=val; return;}
	cerr << "Illegal xform index acces\n"; exit(-1);
}
double Xform4x4::operator()(int i, int j){
	if(i<4 && i>=0 && j<4 && j>=0){ return m[i][j];}
	cerr << "Illegal xform index acces\n"; exit(-1);
	return -1;
}


Xform4x4 Xform4x4::operator*(const Xform4x4& x){
	int i,j,k;

	Xform4x4 result;
	for(i=0; i < 4; i++){
		for(j=0; j < 4; j++){
			result.m[i][j]=0;
			for(k=0; k<4; k++){
				result.m[i][j]+=m[i][k]*x.m[k][j];
			}
		}
	}	
	return result;
}

Xform4x4 Xform4x4::operator*(const Xform& x){

	int i,j;
        q_matrix_type mtemp;
	double *temprot = (double *)(void*)(x.rot); // const to non for quat

        q_to_col_matrix(mtemp,temprot); //rotation
        for(i=0; i < 3; i++){   //scale
                for(j=0; j<3; j++){
                  mtemp[i][j]=mtemp[i][j]*x.scale;
                }
        }
        mtemp[0][3]=x.trans[0];       //translate
        mtemp[1][3]=x.trans[1];
        mtemp[2][3]=x.trans[2];
	
	return (*this)*Xform4x4(mtemp);

}

Xform4x4::Xform4x4(double msrc[4][4]){
	int i,j;
	for(i=0; i < 4; i++)
		for(j=0; j < 4; j++)
			m[i][j]=msrc[i][j];
}

Xform4x4& Xform4x4::operator=(const Xform4x4& x){
	int i,j;
	for(i=0; i < 4; i++)
		for(j=0; j < 4; j++)
			m[i][j]=x.m[i][j];
	return (*this);
}

void Xform4x4::apply(q_vec_type src, q_vec_type result){

	q_type a,b; //promote 3 vectors to homogenous 4 vectors;

	a[0]=src[0]; a[1]=src[1]; a[2]=src[2]; a[3]=1;

	int i,j;
	for(i=0; i < 4; i++){
		b[i]=0;	
		for(j=0; j< 4; j++){
			b[i]+=m[i][j]*a[j];
		}
	}
	result[0]=b[0]/b[3];	//convert back from homogenous coords
	result[1]=b[1]/b[3];
	result[2]=b[2]/b[3];

	return;
}

ostream& operator<< (ostream& co,const Xform4x4& x){
	int i,j;
        for(i=0; i< 4;i++){
                for(j=0; j <4; j++){
                        co << x.m[i][j] <<"\t\t";
                }
                co <<"\n";
        }
	return co;
}


void Xform4x4::transpose(){
	int i,j;
	double temp;
	for(i=0; i< 4 ;i++){
		for(j=i; j < 4; j++){
			temp=m[i][j];
			m[i][j]=m[j][i];
			m[j][i]=temp;
		}
	}
}
		
void Xform4x4::invert(){
	//simple gaussian elimination -- no pivoting... so watch
	//for numerical errors if you aren't using well conditioned
	//matrices... in general we are for ugraphics so I haven't 
	//tried to be fancy here

	int i,j,k;
	double result[4][4];

	for(i=0; i<4; i++)		//zero out result array
		for(j=0; j<4; j++)
			result[i][j]=0;
	result[0][0]=result[1][1]=result[2][2]=result[3][3]=1;

	double scaling;
	for(i=0; i<4; i++){
		for(j=i+1; j<4; j++){
			scaling=m[j][i]/m[i][i];
			for(k=0; k < 4; k++){		//subtract out result
				m[j][k]-=scaling*m[i][k];
				result[j][k]-=scaling*result[i][k];
			}
		}
	}
	//back substitution
	for(i=3; i>=0; i--){
		scaling=m[i][i];
		for(k=0; k<4; k++){
			m[i][k]/=scaling;
			result[i][k]/=scaling;
		}
		for(j=i-1; j>=0; j--){
			scaling=m[j][i];
			for(k=0; k < 4; k++){
				m[j][k]-=scaling*m[i][k];
				result[j][k]-=scaling*result[i][k];
			}
		}
	}

	for(i=0; i < 4; i++)			//copy result
		for(j=0; j<4; j++)
			m[i][j]=result[i][j];

	return;
}

#ifndef MF_NORMAL
#define MF_NORMAL

/* normal.h - supporting data definitions for client side normal calculations
**/

#define VectorSet(v,s)  ((v)[0] = (v)[1] = (v)[2] = (s))

#define VectorDot(va,vb) \
		(	(va)[0]*(vb)[0] \
		+ 	(va)[1]*(vb)[1] \
		+ 	(va)[2]*(vb)[2])

#define VectorLength(v) (sqrt(VectorDot((v),(v))))

#define VectorScale(v,s) { \
			(v)[0] *= (s); \
			(v)[1] *= (s); \
			(v)[2] *= (s);}

#define	VectorNormalize(v)	{ 				\
				double s=VectorLength(v);	\
				if(s < 1.0e-10)	{		\
					(v)[0] = 0.0;		\
					(v)[1] = 0.0;		\
					(v)[2] = 1.0;		\
					}			\
				else				\
					VectorScale((v),1.0/s);	\
				}

#define	VectorNegate(d)	{ \
			(d)[0] = -(d)[0]; \
			(d)[1] = -(d)[1]; \
			(d)[2] = -(d)[2];}

#define	VectorCopy(d,s)	{ \
			(d)[0] = (s)[0]; \
			(d)[1] = (s)[1]; \
			(d)[2] = (s)[2];}

#define	VectorAdd(s1,s2,d)	{ \
			(d)[0] = (s1)[0] + (s2)[0]; \
			(d)[1] = (s1)[1] + (s2)[1]; \
			(d)[2] = (s1)[2] + (s2)[2];}

#define	VectorSubtract(s1,s2,d)	{ \
			(d)[0] = (s1)[0] - (s2)[0]; \
			(d)[1] = (s1)[1] - (s2)[1]; \
			(d)[2] = (s1)[2] - (s2)[2];}

#define	VectorCross(sa,sb,d)  { \
		VectorType	tmp;\
		tmp[0] = (sa)[1]*(sb)[2] - (sa)[2]*(sb)[1]; \
		tmp[1] = (sa)[2]*(sb)[0] - (sa)[0]*(sb)[2]; \
		tmp[2] = (sa)[0]*(sb)[1] - (sa)[1]*(sb)[0]; \
		VectorCopy((d),tmp); \
		}

int Compute_Norm(int x, int y, VectorType Normal, BCPlane* plane);
int Compute_Norm(float x, float y, VectorType Normal, BCPlane* plane);
int Update_Normals(int x, int y);

#endif /* indef MF_NORMAL */

/************************************************************************/
/* core_stimulate_at_point.C    -- code by Dan Fritsch                  */
/* Modified slightly by Greg Clary from                                 */
/* CIMAGE/src/core/ops/core_stimulate_at_point.c                        */
/* Comments were later added to that CIMAGE version, so it is what you  */
/* should read if you want to understand the algorithm.                 */
/************************************************************************/


#include <math.h>
#include <stdio.h>
#include <string.h>

#include "cimage.h"
#include "cimage_core.h"
#include "core_ops.h"
#include "stimulate_core_at_pointP.h"

#ifdef SGI_PARALLEL
#include "task.h"
#endif


/* Indicates whether to traverse to the left only, the right only,
   or both ways.  See core_ops.h.  */
int direction_flag = BOTH;

/* User supplied function called when computing core points */
CoreDrawFunction CoreDrawFunc;

/* User supplied data to function above */
void *core_appData;

FILE *fp;

static int medialness_type;
Core Core1, Core2;
static int polarity;

extern double xConvFactor, yConvFactor;
extern char tmpDir[512];

static MedialnessFunctionDirect myMedialness; 
static MedialnessFunctionDirect1 myMedialness1;
static GetMderivsFunction myDerivative;

/* Function to bracket medialness maximum */
void mnbrak(cimage im,  // Image on which medialness is tested.
	    float x, float y,  
	    float *ax, float *bx, float *cx, 
	    float *fa, float *fb, float *fc, 
	    float (*func)(cimage, float, float, float)) 
{
    float ulim, u, r, q, fu, dum;

    *fa = (*func)(im, x, y, *ax);
    *fb = (*func)(im, x, y, *bx);
    if (*fb > *fa) {
	SHFT(dum, *ax, *bx, dum)
	SHFT(dum, *fb, *fa, dum)
    }
    *cx=(*bx)+GOLD*(*bx-*ax);
    *fc=(*func)(im, x,y,*cx);
    while(*fb > *fc ) {
	r=(*bx-*ax)*(*fb-*fc);
	q=(*bx-*cx)*(*fb-*fa);
	u=(*bx)-((*bx-*cx)*q-(*bx-*ax)*r)/
	  (2.0*SIGN(FLTMAX(fabs(q-r), TINY), q-r));
	ulim=(*bx)+GLIMIT*(*cx-*bx);
	if ((*bx-u)*(u-*cx) > 0.0) {
	    fu = (*func)(im, x,y,u);
	    if (fu < *fc) {
		*ax = (*bx);
		*bx=u;
		*fa=(*fb);
		*fb=fu;
		return;
	    }
	    else if (fu > *fb) {
		*cx=u;
		*fc=fu;
		return;
	    }
	    u=(*cx)+GOLD*(*cx-*bx);
	    fu=(*func)(im, x,y,u);
	}
	else if ((*cx-u)*(u-ulim) > 0.0) {
	    fu=(*func)(im, x,y,u);
	    if (fu < *fc) {
		SHFT(*bx, *cx, u, *cx+GOLD*(*cx-*bx))
		SHFT(*fb, *fc, fu, (*func)(im, x,y,u))
	    }
	}
	else if ((u-ulim)*(ulim-*cx) >= 0.0) {
	    u=ulim;
	    fu=(*func)(im, x,y,u);
	}
	else {
	    u=(*cx)+GOLD*(*cx-*bx);
	    fu=(*func)(im, x,y,u);
	}
	SHFT(*ax, *bx, *cx, u)
	    SHFT(*fa, *fb, *fc, fu);
    }
}	

/* Function to maximize medialness over scale */
float dbrent(cimage im, float x0, float x1, float ax, float bx, float cx, 
	     float (*f)(cimage, float, float, float), float tol, float *xmin)
{
    int iter;
    float a, b, d, etemp, fu, fv, fw, fx, p, q, r, tol1, tol2, u, v,w,x,xm;
    float e=0.0;
    static float count = 0;
    static float n_called = 0;


    n_called += 1;
    a=(ax < cx ? ax : cx);
    b=(ax > cx ? ax : cx);
    x=w=v=bx;
    fw=fv=fx=(*f)(im, x0, x1, x);
    for (iter=1; iter<=ITMAX; iter++) {
	xm=0.5*(a+b);
	tol2=2.0*(tol1=tol*fabs(x)+ZEPS);
	if (fabs(x-xm) <= (tol2-0.5*(b-a))) {
	    *xmin = x;
	    return fx;
	}
	if (fabs(e) > tol1) {
	    r=(x-w)*(fx-fv);
	    q=(x-v)*(fx-fw);
	    p=(x-v)*q-(x-w)*r;
	    if (q > 0.0) p = -p;
	    q=fabs(q);
	    etemp=e;
	    e=d;
	    if (fabs(p) >= fabs(0.5*q*etemp) || p<=q*(a-x)
		|| p>= q*(b-x))
		d=CGOLD*(e=(x >= xm ? a-x : b-x));
	    else {
		d=p/q;
		u=x+d;
		if (u-a < tol2 || b-u < tol2)
		    d=SIGN(tol1, xm-x);
	    }
	}
	else {
	    d = CGOLD*(e=(x >= xm ? a-x : b-x));	
	}
	u=(fabs(d) >= tol1 ? x+d : x+SIGN(tol1, d));
	count +=1.0;
	fu=(*f)(im, x0, x1, u);

	if (fu <= fx) {
	    if (u >= x) a=x; else b=x;
	    SHFT(v,w,x,u)
		SHFT(fv, fw, fx, fu)
		}
	else {
	    if (u < x) a=u; else b=u;
	    if (fu <= fw || w == x) {
		v=w;
		w=u;
		fv=fw;
		fw=fu;
	    }
	    else if (fu <= fv || v == x || v == w) {
		v=u;
		fv=fu;
	    } 
	}
    }
    printf("Error: too many iterations in brent");
    *xmin = x;
    return fx;
}

static float rstep;

static int weak_or_strong;

int find_ridge_point (cimage im, CorePoint *P);
int traverse_one_way (cimage im, Core C, CorePoint Pinit, int which_way);
int traverse_ridge (cimage im,Cores C,CorePoint Pinit);
int compute_direction ();

static float f;
static float fx, fy;
static float fxx, fxy, fyy;
static float fxxx, fxxy, fxyy, fyyy;
static float MM, Ms, Mss, ds_dx, ds_dy;
static float alph, u0,  u1, P, PX, PY;
static float beta, v0,  v1, Q;
static float d0, d1, d0_old, d1_old;
static float scale;
static float gradP;
static float gradPthresh, Mssthresh, alphathresh;

int Scale(cimage im, float x, float y, float *s, int flag)
{
    float tol = 1e-6;
    float as, bs, cs, fa, fb, fc, f, smin;
    as = *s; 
    if (flag == 0) {
	bs = *s + 1;
    }
    else {
	bs = *s+(ds_dx*d0+ds_dy*d1)*rstep/sqrt(d0*d0+d1*d1);
    }

    mnbrak(im, x, y, &as, &bs, &cs, &fa, &fb, &fc, myMedialness1);
    f=dbrent(im, x,y,as, bs, cs, myMedialness1, tol, &smin);
	
    if (smin < 0.8 && flag ==1) {
	printf("Scale too small\n");
	return 0;
    }

/*        if (fabs(*s - smin)/rstep > 1.1 && flag ==1) {
	  printf("Jumped surfaces (S(n)=%f S(n+1)=%f)\n", *s, smin);
	  return 0;
	  } */
    if (fabs(*s - smin)/(*s) > 0.75 && flag == 0) {
	printf("Scale of core too far from seed\n");
	return 0;
    }
    *s = smin;
    return 1;
}


/*-----------------------------------------------------------------------*/
int Fjet (cimage im, CorePoint P)
{
    Mderivs M;
    float MXS2, MYS2, MSS2, MSS3;

    if (P.x<1 || P.x>cimage_xdim(im)-1 || P.y<1 || P.y>cimage_ydim(im)-1) {
	printf("Image dimensions are: %dx%d\n", 
	       cimage_xdim(im), cimage_ydim(im));
	printf("outside of image domain\n");
	return 0;
    }

    myDerivative(im, P, &M); /* Medialness and its derivatives */

    MXS2 = M.MXS*M.MXS; MYS2 = M.MYS*M.MYS;
    MSS2 = M.MSS*M.MSS; MSS3 = MSS2*M.MSS;

    /* first derivatives of OSM surface function */
    f =  M.M;
    fx = M.MX;
    fy = M.MY;
    Ms = M.MS;
    ds_dx = -M.MXS/M.MSS; ds_dy = -M.MYS/M.MSS;

    /* second derivatives of OSM surface function */
    fxx = (M.MXX + M.MXS*ds_dx);
    fxy = (M.MXY + ds_dx*M.MYS);
    fyy = (M.MYY + M.MYS*ds_dy);

    /* third derivatives of OSM surface function */
    fxxx = (M.MXXX
	    - 3.0*(M.MXXS*M.MXS/M.MSS-M.MXSS*ds_dx*ds_dx)
	    - M.MSSS*MXS2*M.MXS/(MSS3));
    fxxy = (M.MXXY
	    - (M.MXXS*M.MYS+2*M.MXYS*M.MXS)/M.MSS
	    + (M.MYSS*MXS2+2*M.MXSS*M.MXS*M.MYS)/(MSS2)
	    - (M.MSSS*MXS2*M.MYS)/(MSS3));
    fxyy = (M.MXYY
	    - (M.MYYS*M.MXS+2*M.MXYS*M.MYS)/M.MSS
	    + (M.MXSS*MYS2+2*M.MYSS*M.MYS*M.MXS)/(MSS2)
	    - (M.MSSS*MYS2*M.MXS)/(MSS3));
    fyyy = (M.MYYY
	    - 3.0*(M.MYYS*M.MYS/M.MSS - M.MYSS*ds_dy*ds_dy)
	    - M.MSSS*MYS2*M.MYS/(MSS3));

    scale = P.s;

    Mss = fabs(M.MSS);
    MM = fabs(M.M);

    return 1;
}

int eigenstuff()
{
    float arg1, arg2, norm1, norm2;
    float Mssold;
    /* Solve Hessian eigensystem */
    arg1 = (fxx-fyy)*(fxx-fyy);
    arg2 = sqrt(arg1+4.0*fxy*fxy);
    alph = (fxx+fyy - arg2)/2.0;
    beta = (fxx+fyy + arg2)/2.0;

    if (alph < beta) {
	u0 = -(-fxx+fyy+arg2)/(2*fxy);
	u1 = 1;
	v0 = -(-fxx+fyy-arg2)/(2*fxy);
	v1 = 1;
    }
    else {
	float temp;
	temp = alph;
	alph = beta;
	beta = temp;
	u0 = -(-fxx+fyy-arg2)/(2*fxy);
	u1 = 1;
	v0 = -(-fxx+fyy+arg2)/(2*fxy);
	v1 = 1;
    }
    norm1 = sqrt(u0*u0+1);
    norm2 = sqrt(v0*v0+1);
    u0/=norm1; u1/=norm1;
    v0/=norm2; v1/=norm2;
    Mssold = Mss;
    Mss = (Mss)/(MM)/4.0;
/*
  if (Mss > Mssthresh) return 1;
  else {
  printf("Ridge too weak in scale (%f)\n", Mss);
  return 0;
  }
  */
    if (weak_or_strong == 0 && alph < 0) return 1;
    else if (weak_or_strong == 1 && alph < 0 && (alph+beta) < 0) return 1;
    printf("outside valid region\n");
	
    return 0;
}

int PQjet()
{
    float Q_div_diff_ab;
    float D3fv00 = fxxx*v0+fxxy*v1;
    float D3fv01 = fxxy*v0+fxyy*v1;
    float D3fv11 = fxyy*v0+fyyy*v1;

    float len = (fx*fx+fy*fy);

    /* ridges are solutions to P(x,y) = 0 */
    P = u0*fx+u1*fy;  /* P = u * grad(f) */
    Q = v0*fx+v1*fy;  /* Q = v * grad(f) */

    /* gradient of P */
    Q_div_diff_ab = Q/(alph-beta);
    PX = alph*u0+Q_div_diff_ab*(u0*D3fv00+u1*D3fv01);
    PY = alph*u1+Q_div_diff_ab*(u0*D3fv01+u1*D3fv11);
    gradP = sqrt(PX*PX+PY*PY)/(2.0*MM);
/*	if (gradP > gradPthresh) return 1;
	else {
	printf("gradP is too small (%f))\n", gradP);
	return 0;
	}
	*/
    return 1;
}

/*-------------------------------------------------------------------------*/
int find_ridge_point (cimage im, CorePoint *Pinit)
{
    int iter;
    int MAXITER = 50;
    float TOLERANCE = 1e-6;
    float Pold = MAXCREAL, test;
    float w=1.0;
    CorePoint Pnew;
    Pnew.x = Pinit->x;
    Pnew.y = Pinit->y;
    Pnew.s = Pinit->s;
    d0 = 0; d1 = 0;

    printf("find_ridge_point:\n");
    printf("Seed at %f, %f, %f\n", Pinit->x, Pinit->y, Pinit->s);

    if (!Scale(im, Pinit->x, Pinit->y, &Pinit->s, 0)) return 0;

    printf("find_ridge_point iteration ");          /* nano */
    for (iter = 0; iter < MAXITER; iter++) {
	printf("%d ", iter);                        /* nano */
	float P2temp, len, gradmag2;

	Pnew.x = Pinit->x + w*d0;
	Pnew.y = Pinit->y - w*d1;
	if (!Scale(im, Pnew.x, Pnew.y, &Pnew.s, 0)) return 0;
	if ( !Fjet(im, Pnew) || !eigenstuff() ) {
	    /*        if ( (w/2) ==0.0) */
	    return 0;
	    continue;
	}
	if (!PQjet()) return 0;
	P2temp = P*P;
	if (P2temp >= Pold) {
	    if ( (w/=2) == 0.0) return 0;
	    continue;
	}
	gradmag2 = (fx*fx+fy*fy);
	test = P2temp/gradmag2;
	if ( test <= TOLERANCE ) {  /* ridge found */
	    Pinit->x = Pnew.x; 
	    Pinit->y = Pnew.y;
	    Pinit->s = Pnew.s;
	    return 1;
	}
	Pold = P2temp;
	Pinit->x = Pnew.x;
	Pinit->y = Pnew.y;
	d0 = P*PX; d1 = P*PY;
	len = sqrt(d0*d0+d1*d1);
	if (len) {
	    d0 /= len; d1 /= len;
	}
    }
    return 0;

}

/*-------------------------------------------------------------------------*/
int compute_direction ()
{
    static float root_half = 0.70710678;
    static float mroot_half = -0.70710678;

    float len, dotD;
    d0 = PY; d1 = PX;

    len = sqrt(d0*d0+d1*d1);

    if ( len > 0 ) {
	d0 /= len; d1 /= len;
    }
    else {
	printf("zero length direction\n");
	return 0;
    }

    dotD = d0*d0_old+d1*d1_old;
    if ( dotD < root_half ) {
	if ( dotD <= mroot_half ) {
	    d0 = -d0; d1 = -d1;
	}
	else {
	    printf("direction vector changed too much\n");
	    if ( dotD <= 0 ) {
		d0 = -d0; d1 = -d1;
		d0_old = d0; d1_old = d1;
	    }
	    printf("(P*P)/(P*P+Q*Q) = %f\n", (P*P)/(P*P+Q*Q));
	    return (P*P)/(P*P+Q*Q) < .02 ? 1 : 0;
	}
    }

    d0_old = d0; d1_old = d1;
    return 1;
}

/*-------------------------------------------------------------------------*/
int traverse_one_way(cimage im, Core C, Core Cnano, CorePoint Pinit, int which_way)
{
    double med_tol = 1e-2;
    int MAXITER = 20000;
    int iter;
    float h = rstep*which_way;
    float len;
    float x0, x1, s;
    float x0_old, x1_old, s_old;
    int oldx=0, oldy=0;
    int exit_code = 1;
    CorePoint Pnew;
    int xb1,xb0,yb1,yb0;
    int xdim=cimage_xdim(im);
    int ydim=cimage_ydim(im);
    float fbx1,fbx0,fbxy;
    CREALTYPE *pixels = (CREALTYPE *)cimage_pixels(im);

    printf("traversing direction %d\n",which_way);

    /* save last eigenvector for continuity testing */
    Fjet(im, Pinit);
    eigenstuff();
    if (!PQjet()) return 0;
    d0 = PY; d1 = PX;
    len = sqrt(d0*d0+d1*d1);
    if (len > 0) {d0/=len; d1/=len;}
    d0_old = d0; d1_old = d1;

    Pnew.x = Pinit.x;  Pnew.y = Pinit.y; Pnew.s = Pinit.s;

    for (iter = 0; iter < MAXITER; iter++) {
	x0_old = Pnew.x; x1_old = Pnew.y; s_old = Pnew.s;
	Pnew.x += h*d0;
	Pnew.y += h*d1;

	if (!Scale(im, Pnew.x, Pnew.y, &Pnew.s,1)) {
	    printf("Failure in Scale\n");
	    exit_code = 0;
	}
	if (!Fjet(im, Pnew) || !eigenstuff() ) {
	    printf("Failure in Fjet of eigenstuff\n");
	    exit_code = 0;
	}
	if (!PQjet()) {
	    printf("Failure in PQjet\n");
	    exit_code = 0;
	}
	if (!compute_direction() ) {
	    printf("Failure in compute_direction\n");
	    exit_code = 0;
	}
	Pnew.alpha = (float) (alph/(MM*2.0));

	if (Pnew.alpha > alphathresh) {
	    printf("Alpha threshold exceeded\n");
	    exit_code = 0;
	}
	if (exit_code == 0) return 0;

	/* Do bilinear interpolation to get image value */
	/* note that casting truncates, so the following correctly *
	 * selects the indices */
	xb1 = (int)(Pnew.x + 1.0);
	xb0 = xb1 - 1;
	yb1 = (int)(Pnew.y + 1.0);
	yb0 = yb1 - 1;

	if (xb1 > xdim) xb1 = xdim-1;
	if (xb0 > xdim) xb0 = xdim-1;
	if (yb1 > ydim) yb1 = ydim-1;
	if (yb0 > ydim) yb0 = ydim-1;

	fbx0 = (float)pixels[yb0*xdim+xb0] + (float)(pixels[yb0*xdim+xb1] - pixels[yb0*xdim+xb0])*(Pnew.x-(float)xb0);
	fbx1 = (float)pixels[yb1*xdim+xb0] + (float)(pixels[yb1*xdim+xb1] - pixels[yb1*xdim+xb0])*(Pnew.x-(float)xb0);
	fbxy = fbx0 + (fbx1 - fbx0)*(Pnew.y-(float)yb0);

/************ for debugging --
  if (fbxy > 400.0)
  {
  printf("xb1=%d, xb0=%d, yb1=%d, yb0=%d, pix00=%f, pix01=%f\npix10=%f, pix11=%f, fbx0=%f, fbx1=%f, fbxy=%f\n",xb1,xb0,yb1,yb0,pixels[yb0*xdim+xb0],pixels[yb1*xdim+xb0],pixels[yb0*xdim+xb1],pixels[yb1*xdim+xb1],fbx0,fbx1,fbxy);
  }
  ************/

/*
  fprintf(fp,"%f %f %f\n",xConvFactor*Pnew.x,yConvFactor*Pnew.y,fbxy);
  */
	core_point_add(Cnano,xConvFactor*Pnew.x,yConvFactor*Pnew.y,fbxy);


/************* for comparing versions of cimage - also hardwired
  stimulation point in core.c
  numpoints++;
  if (numpoints==20)
  {
  fclose(fp);
  exit(1);
  }
  **************/
	if ((oldx != (int)(Pnew.x+0.5) || oldy != (int)(Pnew.y+0.5))) {
	    Pnew.gradP = (float) gradP;
	    Pnew.Mss = Mss;
	    Pnew.dx = d0; Pnew.dy = d1; 
	    Pnew.ds = (ds_dx*d0+ds_dy*d1)*rstep;
	    if (Pnew.ds/rstep > 1.0) return 0;
	    x0 = Pnew.x; x1 = Pnew.y; s = Pnew.s;
	    oldx = (int) (x0+0.5); oldy = (int) (x1+0.5);
	    if (CoreDrawFunc != NULL) {
		if ((*CoreDrawFunc)(Pnew, core_appData) == -1) {
		    printf("Core ends due to user interrupt\n");
// fclose(fp);
// exit(1);
		    exit_code = 0;
		}
	    }
	    if (exit_code != 0) {
		core_point_add(C, Pnew.x, Pnew.y, Pnew.s);
		(C->cpoints[core_length(C)-1]).dx = Pnew.dx;
		(C->cpoints[core_length(C)-1]).dy = Pnew.dy;
		(C->cpoints[core_length(C)-1]).ds = Pnew.ds;
		(C->cpoints[core_length(C)-1]).alpha = 
						       (float) (-alph/(MM*2.0));
		(C->cpoints[core_length(C)-1]).gradP = 
						       (float) gradP;
		(C->cpoints[core_length(C)-1]).Mss = Mss;
	    }
	}
    }
    return 1;
}

/*------------------------------------------------------------------------*/
int traverse_ridge (cimage im, Cores C, CorePoint P)
{
    int i;
    Core thisCore = core_init(500);
    CorePoint Pnull; Pnull.x = -1; Pnull.y = -1; Pnull.s = -1;

    Core Corenano1 = core_init(400);
    Core Corenano2 = core_init(400);

    char *outputfilename;

    Core1 = core_init(400);
    Core2 = core_init(400);

    thisCore->med_type = C->med_type;
    thisCore->polarity = polarity;
    printf("Traversing ridge\n");

    (*CoreDrawFunc)(Pnull, core_appData);
    if (direction_flag == 1 || direction_flag == 0)
	traverse_one_way(im, Core1, Corenano1, P, 1);
    (*CoreDrawFunc)(Pnull, core_appData);
    if (direction_flag == 2 || direction_flag == 0)
	traverse_one_way(im, Core2, Corenano2, P, -1);

    if (tmpDir[0]!='\0')
    {
	int len = strlen(tmpDir) + 1 + strlen("points.out") + 1;
	outputfilename = new char[len];

	if (outputfilename==NULL)
	    cimage_error("Can't allocate space for core output file name\n");

	strcpy(outputfilename,tmpDir);
	strcat(outputfilename,"points.out");
    }
    else
    {
	outputfilename = new char[strlen("./points.out")+1];

	if (outputfilename==NULL)
	    cimage_error("Can't allocate space for core output file name\n");

	strcpy(outputfilename,"./points.out");
    }

    fp=fopen(outputfilename,"w");
    if (fp==NULL)
	cimage_error("Can't open core output file\n");

    for (i=1; i<=core_length(Corenano1); i++) {
	// note that s is height for Corenano1 and Corenano2
	P = core_point_get(Corenano1, core_length(Corenano1)-i);
	fprintf(fp,"%f %f %f\n",P.x,P.y,P.s);
    }

    for (i=1; i<core_length(Corenano2); i++) {
	// note that s is height for Corenano1 and Corenano2
	P = core_point_get(Corenano2, i-1);
	fprintf(fp,"%f %f %f\n",P.x,P.y,P.s);
    }

    fclose(fp);

    for (i=1; i<=core_length(Core1); i++) {
	CorePoint P = core_point_get(Core1, core_length(Core1)-i);
	core_point_add(thisCore, P.x, P.y, P.s);
	(thisCore->cpoints[core_length(thisCore)-1]).dx = P.dx;
	(thisCore->cpoints[core_length(thisCore)-1]).dy = P.dy;
	(thisCore->cpoints[core_length(thisCore)-1]).ds = P.ds;
	(thisCore->cpoints[core_length(thisCore)-1]).alpha =  P.alpha;
	(thisCore->cpoints[core_length(thisCore)-1]).Mss = P.Mss;
	(thisCore->cpoints[core_length(thisCore)-1]).gradP = P.gradP;
    }
    for (i=1; i<core_length(Core2); i++) {
	CorePoint P = core_point_get(Core2, i);
	core_point_add(thisCore, P.x, P.y, P.s);
	(thisCore->cpoints[core_length(thisCore)-1]).dx = P.dx;
	(thisCore->cpoints[core_length(thisCore)-1]).dy = P.dy;
	(thisCore->cpoints[core_length(thisCore)-1]).ds = P.ds;
	(thisCore->cpoints[core_length(thisCore)-1]).alpha = P.alpha;
	(thisCore->cpoints[core_length(thisCore)-1]).Mss = P.Mss;
	(thisCore->cpoints[core_length(thisCore)-1]).gradP = P.gradP;
    }
       
    cores_core_add(C, thisCore);

    core_destroy(Core1);
    core_destroy(Core2);

    core_destroy(Corenano1);
    core_destroy(Corenano2);

    return 1;

}

// core_stimulate_at_point()
// Starts off the process of tracking a core.  Sets some global
// variables, calls find_ridge_point() to find a nearby point on a
// core, and then calls traverse_ridge() to start the process of
// tracking the core.
int core_stimulate_at_point(
    cimage input, 		    // image for core finding
    Cores C, 			    // list on which to output cores
    CorePoint P,		    // scale space point at which to
				    //    begin tracking
    float step_size,		    // size of increment when tracking
    int ridge_weak_or_strong, 
    int black_or_white,		    // black on white or vice versa;
				    //    simply stored in core point
    int med_type,		    // the types are defined in
				    //    stimulate_core_at_pointP.h
    int (*func)(CorePoint, void *), // Callback function used to plot
				    //    points on the core.
    void *AppData)
{
    int found; // Will be set if a core point is found.

    if (C == NULL)
	C = cores_init(10);
    medialness_type = med_type;

    //---------------------------------------------------------------------
    // Depending on the kind of medialness, assign appropriate
    // function pointers to myMedialness, etc.  

    if (med_type == LAP_MEDIALNESS) {
#ifdef SGI_PARALLEL
	printf("Using parallel version of medialness server\n");
	printf("This SGI has %d processors", m_get_numprocs());

	myMedialness = Lap_Medialness2proc;
	myMedialness1 = Lap_Medialness12proc;
	myDerivative = Lap_get_mderivs2proc;
	cores_medtype(C) = LAP_MEDIALNESS;
#else
	myMedialness = Lap_Medialness;
	myMedialness1 = Lap_Medialness1;
	myDerivative = Lap_get_mderivs;
	cores_medtype(C) = LAP_MEDIALNESS;
#endif
    }
    else if (med_type == LPP_MEDIALNESS) {
	myMedialness = Lpp_Medialness;
	myMedialness1 = Lpp_Medialness1;
	myDerivative = Lpp_get_mderivs;
	cores_medtype(C) = LPP_MEDIALNESS;
    }
    else if (med_type == EDGE_MEDIALNESS) {
	printf("Using edge medialness\n");
	myMedialness = Edge_Medialness;
	myMedialness1 = Edge_Medialness1;
	myDerivative = Edge_get_mderivs;
	cores_medtype(C) = EDGE_MEDIALNESS;
    }

    weak_or_strong =  ridge_weak_or_strong;
    rstep = step_size;
    CoreDrawFunc = func;
    core_appData = AppData;
    polarity = black_or_white;
    gradPthresh = P.gradP;
    Mssthresh = P.Mss;
    alphathresh = P.alpha;
    if (gradPthresh < 0.0 || gradPthresh > 1.0) gradPthresh = 0.45;
    if (Mssthresh < 0.0 || Mssthresh > 1.0) Mssthresh = 0.3;
    if (alphathresh < 0.0 || alphathresh > 1.0) alphathresh = 0.4;

    found = find_ridge_point(input, &P);
    printf("\n");  /* Solely to make debugging printfs look nice. */

    if ( found )
	found = traverse_ridge(input, C, P);
    else
	cimage_warning("ridge not found");

    return found;
}

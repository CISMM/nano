/* 
This file contains functions for solving for a particular type of 
transformation that minimizes (least squares) the distance between
a set of transformed points and a set of corresponding points in the
space to which they were transformed (commonly known as the Procrustes
method).


some of the types of 2d transformations that we might want to solve for
and whether or not they can be solved using a linear solver:

*general (rotation, non-uniform scale, shear, translate) - yes
        a b c
        d e f

(rotation, non-uniform scale, no shear, translate) - no - d must be
	found by non-linear technique
		a   b  c
		-bd ad e

(rotation, uniform scale, shear, translate) - yes
		a b c
        d a e

(rotation, uniform scale, no shear, translate) - yes
        a -b  c
        b  a  d

(no rotation, non-uniform scale, no shear, translate) - yes
        a 0 b
        0 c d

(no rotation, uniform scale, no shear, translate) - yes
        a 0 b
        0 a c

**(no rotation, uniform scale, shear in x xor y, translate) - yes
        a b c
        0 a d
	or (we would try both and pick which ever one is optimal)
        a 0 b
        c a d

**(no rotation, non-uniform scale, shear in x xor y, translate) - yes
        a b c        
        0 d e
        or (we would try both and pick which ever one is optimal)
        a 0 b        
        c d e


[notes: *this would probably be the best option for 2D-->2D registration
	problems as with the combination confocal microscope/afm
	**these would probably be best in combination with 3D rotation for
	the 3D-->2D registration as in combination afm/sem

For 3d rotation we will probably want to solve for
the azimuth angles but constrain altitude and roll angle (imagine an sem
looking down at an afm surface from a 45 degree altitude
now if we change afm scan angle, it is like the afm surface is rotating
about the z axis (changing the azimuthal angle but keeping roll and
altitude constant) - really the surface is staying in the same place but
since our afm coordinate system changes it will seem like the surface
is rotated and we should probably realign using the hint from the
known change in scan angle. Also, some component of afm shear caused by 
both x and y piezo drift may be indistinguishable from a small rotation about z.

Since we don't know the altitude is precisely 45 degrees we would like to
include this in our search space although perhaps it is okay to leave
it as a constant since it not supposed to change.

We don't know the roll angle precisely either but this can be
included if it is known to change or determined once and then left as
a constant - 
roll angle determines which direction in the image gets forlengthened so
We could determine this by some kind of calibration procedure
 (e.g. We could look at a known flat circular object and the angle that
 the major axis of its ellipse makes with the image axis gives you the
 roll angle - barring shear and other distortions of course.
 Alternatively, you could adjust roll angle to be zero, if such an
 adjustment (sem scan angle) is available, by looking at a circular object
 and adjusting until its elliptical projection in the image has the long
 axis in the direction of the vertical image axis.

*/

#include "transformSolve.h"
#include "linLeastSqr.h"
#include <math.h>
#include "quat.h"
#if ((defined __CYGWIN__) || (defined _WIN32))
#include <float.h> // for DBL_MAX
#else
#include <values.h> // for MAXDOUBLE
#endif

// M_PI not defined for VC++, for some reason. 
#ifndef M_PI
#define M_PI		3.14159265358979323846
#endif

/* This function solves for the optimal transformation 
   
   By default it solves for a general 2D->2D transformation including
   rotation, translation, shear, and non-uniform scaling.

   if rotation3D is TRUE then the function solves the non-linear least
     squares problem of determining the optimal rotation (parametrized by
     2 angles theta and phi) and uses a linear least squares solver for
     determining the optimal 2D transformation given each 3D rotation.
     However, this 2D transformation is constrained to have no rotation -
     see (**) above.

*/

int transformSolver(double *xform_matrix, double *error,
        Correspondence &c, int im0, int im1,
        vrpn_bool rotation3D)
{
	// for now we only rotate about two angles, leaving the third angle
	// constant, phi, theta determine the direction of the projection 
	// hence the name of the third angle
    double phi, theta; //rot_about_projection_vector;
    double r_x = 0.0, r_y = 0.0, r_z = 0.0, angle = 0;
    double scale_ratio_x_to_y = 1.0;
	// - we do a uniform scaling since we want
	// the projection to account for the different scaling in x and y and
	// adding this additional degree of freedom makes the result a little
	// too underconstrained (we should be able to make the assumption that
	// the SEM is calibrated to have equal scaling in x and y)
    double shear_x = 0.0, shear_y = 0.0;
	// we also leave shear as a constant for similar reasons to those for
	// leaving the scale_ratio constant
#if ((defined __CYGWIN__) || (defined _WIN32))
    double error_min = DBL_MAX;
#else
    double error_min = MAXDOUBLE;
#endif
    //double phi_error_min, theta_error_min;
    double r_x_error_min, r_y_error_min, r_z_error_min, angle_error_min;

    if (rotation3D) {
	// I know this is not the way to do it but I just want
	// to see if it gets the right answer
	double theta_min = 0;
	double theta_max = 2.0*M_PI;
	double phi_min = 0;
	double phi_max = M_PI/2.0;
        double theta_incr = theta_max/10.0;
	double phi_incr = phi_max/5.0;
	q_type q0, q1;

	for (theta = theta_min; theta <= theta_max; theta += theta_incr){
	    for (phi = phi_min; phi <= phi_max; phi += phi_incr){
		q_make(q0, 0, 0, 1.0, theta);

		// NOTE: this rotation determines which axis in the
		// image plane is parallel to the surface; by using
		// the x-axis here, we constrain the solution to
		// be such that the projection of the horizontal axis of 
		// the image is parallel to the horizontal axis of the
		// image - this will not be the case in general for the
		// vertical axis of the image; as a result, you can only
		// get lengthening of the image in the vertical direction
		// (the direction along which the image can be stretched
		//  is perpendicular to this rotation axis and by
		// stretching I am referring to the relative scaling of
		// the horizontal and vertical - in other words:
		// this constrains scale_y/scale_x >= 1.0
		q_make(q1, -1.0, 0.0, 0.0, phi);

		q_mult(q0, q0, q1);
		q_normalize(q0, q0);

		// we could add a 3rd rotation here about z to control roll
                // angle

		// x = x0*sin(angle/2)
    		// y = y0*sin(angle/2)
		// z = z0*sin(angle/2)
		// w = cos(angle/2)
		double sinA = q0[Q_X]*q0[Q_X] + q0[Q_Y]*q0[Q_Y] +
                  q0[Q_Z]*q0[Q_Z];
		double cosA = q0[Q_W];
		sinA = sqrt(sinA);
		if (sinA == 0){
			r_x = q0[Q_X];
                        r_y = q0[Q_Y];
                        r_z = q0[Q_Z];
		}
		else {
			r_x = q0[Q_X]/sinA;
			r_y = q0[Q_Y]/sinA;
			r_z = q0[Q_Z]/sinA;
		}
		angle = asin(sinA);
		if (cosA < 0)
        		angle = M_PI - angle;
		angle *= 2.0;

/*
		printf("(theta,phi)=(%g,%g) --> (%g,%g,%g,(%g))\n",
			theta*180.0/M_PI, phi*180.0/M_PI,
			r_x, r_y, r_z, angle*180.0/M_PI);
*/

		if (transformSolver(xform_matrix, error,
		    c, im0, im1, r_x, r_y, r_z, angle,
		    ZERO_ROTATION_2D2D,
		    shear_x, shear_y, scale_ratio_x_to_y)) {
		    fprintf(stderr, "Error in transformSolver: rotate\n");
		    return -1;
		}
		if (*error < error_min) {
		    error_min = *error;
		    //phi_error_min = phi;
		    //theta_error_min = theta;
		    r_x_error_min = r_x;
		    r_y_error_min = r_y;
		    r_z_error_min = r_z;
		    angle_error_min = angle;
		}
	    }
	}
/*
	if (transformSolver(xform_matrix, error,
            c, im0, im1, 1.0, 0.0, 0.0, 0.0,
	    ZERO_ROTATION_2D2D,
            0.0, 0.0, 1.0)) {
            fprintf(stderr, "Error in transformSolver\n");
            return -1;
        }
	printf("Error for 0 rotation is :%g\n", *error);
*/

//	printf("optimal theta, phi is %g, %g\n",
//		theta_error_min, phi_error_min);
	if (transformSolver(xform_matrix, error,
            c, im0, im1, r_x_error_min, r_y_error_min, r_z_error_min, 
	    angle_error_min,
            ZERO_ROTATION_2D2D, shear_x, shear_y, scale_ratio_x_to_y)) {
            fprintf(stderr, "Error in transformSolver using rotation\n");
	    return -1;
        }

	return 0;
    } else {
	if (transformSolver(xform_matrix, error,
                 c, im0, im1, r_x, r_y, 0.0, angle, 
		 		GENERAL_2D2D,
                 shear_x, shear_y, scale_ratio_x_to_y)) {
	    fprintf(stderr, "Error in transformSolver using no rotation\n");
	    return -1;
	}
	return 0;
    }
}

/* This function solves for the optimal 2D-->2D transformation that
   takes points from im0 to im1. However, if the angle given is non-zero
   then im0 is treated as a set of 3D points that are first rotated by the
   inverse of the angle and then the 2D transformation is found which if
   applied to the resulting x,y values optimally transforms into im1
   The composition of the rotation and 2D transformation is returned as
   well as an error.
 
   This function is intended to be used as a subroutine for an iterative
   solver that determines the optimal 3D rotation and possibly shear and
   scaling. It may also be used by itself typically with type ==
   GENERAL to solve for a general 2D-->2D affine transformation.

   The shear and scaling arguments are used if type == ZERO_SHEAR_UNIFORM_SCALE
   so that the user may give specific values for shear and the scale ratio.
   These values will be considered in determining the optimum transformation
   and included in the returned transformation but the function will not
   look at optima for different values of shear and scale ratio.

    XXX - I wasn't thinking clearly enough when I wrote this to realize
   that giving two values for shear is not necessary since one value for
   shear plus rotation gives you the same thing and we are searching over
   rotations so you really just need to give one value for shear (say shear_x)
   You can always just pass 0 for the other shear direction if you want to.

 */

int transformSolver(double *xform_matrix, double *error,
        Correspondence &c, int im0, int im1,
        double r_x, double r_y, double r_z, double angle, 
		xform_solution_t type,
        double shear_x, double shear_y,
        double scale_ratio_x_to_y)
{
    double scale_x_shear[16] = {scale_ratio_x_to_y,shear_y,0.0,0.0,
			scale_ratio_x_to_y*shear_x, 1.0, 0.0, 0.0,
			0.0,     0.0, 1.0, 0.0,
			0.0,     0.0, 0.0, 1.0};
    double pre_xform_matrix[16];
    double xform_matrix_sol[16];
//      double sinA = sin(angle);
//      double cosA = cos(angle);
    q_type quat;

    if (scale_ratio_x_to_y == 0){
	fprintf(stderr, "Warning: x scaling will be set to 0.0!\n");
    }

    int i;
    /* we apply the inverse rotation to the points in im0 and solve for
       the 2D transformation of the result that takes us to the points
       in im1
       then the total solution is the 2D transformation applied to the
       rotation
    */
    q_make(quat, r_x, r_y, r_z, -angle);
    q_to_ogl_matrix(pre_xform_matrix, quat);

    q_vec_type im0pt;

	if (type == GENERAL_3D2D) {
/*
		a*x_grid0 + b*y_grid0 + c*z_grid0 + d = x_image0
		a*x_grid1 + b*y_grid1 + c*z_grid1 + d = x_image1
		a*x_grid2 + b*y_grid2 + c*z_grid2 + d = x_image2
		a*x_grid3 + b*y_grid3 + c*z_grid3 + d = x_image3
		...
		
		--> A = [x_grid0, x_grid1, x_grid2, x_grid3, ...
				 y_grid0, y_grid1, y_grid2, y_grid3, ...
				 z_grid0, z_grid1, z_grid2, z_grid3, ...
				 1.0,     1.0,     1.0,     1.0,     ...]
			B = [x_image0, x_image1, x_image2, x_image3, ...]
			m = 4+, n = 4
		
		--> solve for X = [a b c d]

		e*x_grid0 + f*y_grid0 + g*z_grid0 + h = y_image0
        e*x_grid1 + f*y_grid1 + g*z_grid1 + h = y_image1
        e*x_grid2 + f*y_grid2 + g*z_grid2 + h = y_image2
		e*x_grid3 + f*y_grid3 + g*z_grid3 + h = y_image3
        ...

        --> A = [x_grid0, x_grid1, x_grid2, x_grid3, ...
                 y_grid0, y_grid1, y_grid2, y_grid3, ...
				 z_grid0, z_grid1, z_grid2, z_grid3, ...
                 1.0,     1.0,     1.0,     1.0,     ...]
            B = [y_image0, y_image1, y_image2, y_image3, ...]
            m = 4+, n = 4

        --> solve for X = [e f g h]

        xform_matrix = [a b c d]
                       [e f g h]
                       [0 0 1 0]
                       [0 0 0 1]
*/
		if (c.numPoints() < 4) {
			fprintf(stderr, "Error: need at least 4 points\n");
			return -1;
		}
		int m,n;
		double *A = NULL;
		double *B = NULL;
		n = 4;
		m = c.numPoints();
		A = new double[m*n];
		B = new double[m];
		corr_point_t p;
		for (i = 0; i < c.numPoints(); i++){
        	c.getPoint(im0, i, &p);
//      	printf("(%g,%g) -->", p.x, p.y);
        	im0pt[0] = p.x;	// 3D surface position (im0pt = image 0 point)
        	im0pt[1] = p.y;
        	im0pt[2] = p.z;
        	q_xform(im0pt, quat, im0pt);
        	A[i] = im0pt[0];
        	A[m+i] = im0pt[1];
			A[2*m+i] = im0pt[2];
        	A[3*m+i] = 1.0;
        	c.getPoint(im1, i, &p);
//      	printf(" (%g,%g)\n", p.x, p.y);
        	B[i] = p.x;		// 2D image position x component (image 1 point)
    	}
    	if (linearLeastSquaresSolve(m,n,A,B)){
        	fprintf(stderr, "Error: linleastsqr solver failed\n");
        	return -1;
    	}

    	xform_matrix_sol[0*4] = B[0];    //a
    	xform_matrix_sol[1*4] = B[1];    //b
    	xform_matrix_sol[2*4] = B[2];	//c
    	xform_matrix_sol[3*4] = B[3];    //d

    	*error = 0;				// (note: error = 0 if n==m)
    	for (i = n; i < m; i++)
        	*error += B[i]*B[i];

    	for (i = 0; i < c.numPoints(); i++){
        	c.getPoint(im0, i, &p);
        	im0pt[0] = p.x;
        	im0pt[1] = p.y;
        	im0pt[2] = p.z;
        	q_xform(im0pt, quat, im0pt);
        	A[i] = im0pt[0];
        	A[m+i] = im0pt[1];
			A[2*m+i] = im0pt[2];
        	A[3*m+i] = 1.0;
        	c.getPoint(im1, i, &p);
        	B[i] = p.y;
    	}
    	if (linearLeastSquaresSolve(m,n,A,B)){
        	fprintf(stderr, "Error: linleastsqr solver failed\n");
        	return -1;
    	}

    	for (i = 0; i < 4; i++){
        	xform_matrix_sol[2+i*4] = 0.0;
        	xform_matrix_sol[3+i*4] = 0.0;
    	}
    	xform_matrix_sol[1+0*4] = B[0];  //e
    	xform_matrix_sol[1+1*4] = B[1];  //f
    	xform_matrix_sol[1+2*4] = B[2];	//g
    	xform_matrix_sol[1+3*4] = B[3];  //h
    	xform_matrix_sol[2+2*4] = 1.0;
    	xform_matrix_sol[3+3*4] = 1.0;

    	for (i = n; i < m; i++)
        	*error += B[i]*B[i];
	}
    else if (type == GENERAL_2D2D) {
/*
        a*x_grid0 + b*y_grid0 + d = x_image0
        a*x_grid1 + b*y_grid1 + d = x_image1
        a*x_grid2 + b*y_grid2 + d = x_image2
        ...

        --> A = [x_grid0, x_grid1, x_grid2, ...
                 y_grid0, y_grid1, y_grid2, ...
                 1.0,     1.0,     1.0,     ...]
            B = [x_image0, x_image1, x_image2, ...]
            m = 3+, n = 3

        --> solve for X = [a b d]

        e*x_grid0 + f*y_grid0 + h = y_image0
        e*x_grid1 + f*y_grid1 + h = y_image1
        e*x_grid2 + f*y_grid2 + h = y_image2
        ...

        --> A = [x_grid0, x_grid1, x_grid2, ...
                 y_grid0, y_grid1, y_grid2, ...
                 1.0,     1.0,     1.0,     ...]
            B = [y_image0, y_image1, y_image2, ...]
            m = 3+, n = 3

        --> solve for X = [e f h]

        xform_matrix = [a b 0 d]
                       [e f 0 h]
                       [0 0 1 0]
                       [0 0 0 1]

*/

        if (c.numPoints() < 3) {
            fprintf(stderr, "Error: need at least 3 points\n");
            return -1;
        }
	int m,n;
	double *A = NULL;
	double *B = NULL;
	n = 3;
	m = c.numPoints();
	A = new double[m*n];
	B = new double[m];
	corr_point_t p;
	for (i = 0; i < c.numPoints(); i++){
	    c.getPoint(im0, i, &p);
//	    printf("(%g,%g) -->", p.x, p.y);
	    im0pt[0] = p.x; 
	    im0pt[1] = p.y; 
	    im0pt[2] = p.z;
	    q_xform(im0pt, quat, im0pt);
	    A[i] = im0pt[0];
	    A[m+i] = im0pt[1];
	    A[2*m+i] = 1.0;
	    c.getPoint(im1, i, &p);
//	    printf(" (%g,%g)\n", p.x, p.y);
	    B[i] = p.x;
	}
	if (linearLeastSquaresSolve(m,n,A,B)){
	    fprintf(stderr, "Error: linleastsqr solver failed\n");
	    return -1;
	}

	xform_matrix_sol[0*4] = B[0];	//a
	xform_matrix_sol[1*4] = B[1];	//b	
	xform_matrix_sol[2*4] = 0.0;
	xform_matrix_sol[3*4] = B[2];	//d

	*error = 0;
	for (i = 3; i < m; i++)
	    *error += B[i]*B[i];

	for (i = 0; i < c.numPoints(); i++){
	    c.getPoint(im0, i, &p);
	    im0pt[0] = p.x; 
	    im0pt[1] = p.y; 
	    im0pt[2] = p.z;
	    q_xform(im0pt, quat, im0pt);
	    A[i] = im0pt[0];
	    A[m+i] = im0pt[1];
	    A[2*m+i] = 1.0;
	    c.getPoint(im1, i, &p);
	    B[i] = p.y;
	}
	if (linearLeastSquaresSolve(m,n,A,B)){
	    fprintf(stderr, "Error: linleastsqr solver failed\n");
	    return -1;
	}

	for (i = 0; i < 4; i++){
	    xform_matrix_sol[2+i*4] = 0.0;
	    xform_matrix_sol[3+i*4] = 0.0;
	}
	xform_matrix_sol[1+0*4] = B[0];	//e
	xform_matrix_sol[1+1*4] = B[1];	//f
	xform_matrix_sol[1+2*4] = 0.0;
	xform_matrix_sol[1+3*4] = B[2];	//h
	xform_matrix_sol[2+2*4] = 1.0;
	xform_matrix_sol[3+3*4] = 1.0;

	for (i = 3; i < m; i++)
	    *error += B[i]*B[i];

    } 
    else if (type == ZERO_SHEAR_UNIFORM_SCALE_2D2D)
    { // we don't want the solver to find an arbitrary shear and scale
	     // ratio, instead we constrain shear to be zero and scaling to be
	     // uniform for the solver, but the given shear and scale ratio is 
	     // applied to the solver's input points and to the resulting
	     // transform output so that the final result includes the
	     // requested values for shear and scaling
/*
   while less efficient we can put all this together in a single problem
   of minimizing || C - A*X ||_2
   which is the form required by linearEqualityConstrainedLeastSquaresSolve()

   A = [x_grid0, x_grid1, x_grid2, ..., 0.0, 0.0, 0.0, ...
        y_grid0, y_grid1, y_grid2, ..., 0.0, 0.0, 0.0, ...
        1.0,     1.0,     1.0,     ..., 0.0, 0.0, 0.0, ...
        0.0, 0.0, 0.0,        ..., x_grid0, x_grid1, x_grid2, ...
        0.0, 0.0, 0.0,        ..., y_grid0, y_grid1, y_grid2, ...
        0.0, 0.0, 0.0,        ..., 1.0,     1.0,     1.0,     ...]

   C = [x_image0, x_image1, x_image2, ..., y_image0, y_image1, y_image2, ...]

   m = 6+, n = 6

   solve for X = [a b d e f h]

   so our transform matrix is:
        xform_matrix = [a b 0 d]
                       [e f 0 h]
                       [0 0 1 0]
                       [0 0 0 1]

   we can constrain to have only rotation, uniform scaling, and translation by
   requiring a = f and e = -b. I believe this is the only constraint we
   can express as a linear expression in the elements of the transform matrix
   that has any usefulness for the registration problem. There are a bunch of
   them that would be nice that I don't think can be written in this way so
   that is left to the non-linear solver which would call this function as 
   a subroutine.
   as a matrix expression this constraint would be:

   BX = D where
   B = [1.0, 0.0,
        0.0, 1.0,
        0.0, 0.0,
        0.0, 1.0,
        -1.0, 0.0,
        0.0, 0.0]
   D = [0.0, 0.0]

*/

	if (c.numPoints() < 3) {
            fprintf(stderr, "Error: need at least 3 points\n");
            return -1;
        }
        int m,n,p;
        double *A = NULL;
	double *C = NULL;

	p = 2;	// we need 2 constraints here
        n = 6;
        m = 2*c.numPoints();
        A = new double[m*n];
        double B[12] = { 1.0, 0.0,
			 0.0, 1.0,
			 0.0, 0.0,
			 0.0, 1.0,
			-1.0, 0.0,
			 0.0, 0.0};

	C = new double[m];
	double D[2] = {0.0, 0.0};
	double Xarr[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
        corr_point_t pt;
	qogl_matrix_mult (pre_xform_matrix, scale_x_shear, pre_xform_matrix);

/*
   reminder of what is in the comment above since this is what gets
   initialized in the following loop:
   A = [x_grid0, x_grid1, x_grid2, ..., 0.0, 0.0, 0.0, ...
        y_grid0, y_grid1, y_grid2, ..., 0.0, 0.0, 0.0, ...
        1.0,     1.0,     1.0,     ..., 0.0, 0.0, 0.0, ...
        0.0, 0.0, 0.0,        ..., x_grid0, x_grid1, x_grid2, ...
        0.0, 0.0, 0.0,        ..., y_grid0, y_grid1, y_grid2, ...
        0.0, 0.0, 0.0,        ..., 1.0,     1.0,     1.0,     ...]

   C = [x_image0, x_image1, x_image2, ..., y_image0, y_image1, y_image2, ...]
*/
	int npt = c.numPoints();

        for (i = 0; i < npt; i++){
            c.getPoint(im0, i, &pt);
//            printf("(%g,%g) -->", pt.x, pt.y);
            im0pt[0] = scale_ratio_x_to_y*pt.x + shear_x*pt.y;
            im0pt[1] = pt.y + shear_y*pt.x;
            im0pt[2] = pt.z;
            q_xform(im0pt, quat, im0pt);
            A[i] = im0pt[0];	A[npt+i] = 0.0;
            A[m+i] = im0pt[1];	A[m+npt+i] = 0.0;
            A[2*m+i] = 1.0;	A[2*m+npt+i] = 0.0;
	    A[3*m+i] = 0.0;	A[3*m+npt+i] = A[i];
	    A[4*m+i] = 0.0;	A[4*m+npt+i] = A[m+i];
	    A[5*m+i] = 0.0;	A[5*m+npt+i] = A[2*m+i];
            c.getPoint(im1, i, &pt);
 //           printf(" (%g,%g)\n", pt.x, pt.y);
            C[i] = pt.x;
	    C[i+npt] = pt.y;
        }


        if (linearEqualityConstrainedLeastSquaresSolve(m,n,p,A,B,C,D,Xarr)){
            fprintf(stderr, "Error: lineqconstrleastsqrsolve failed\n");
            return -1;
        }
	// initialize rows 2 and 3 to 0
        for (i = 0; i < 4; i++){
            xform_matrix_sol[2+i*4] = 0.0;
            xform_matrix_sol[3+i*4] = 0.0;
        }
	// set row 0
        xform_matrix_sol[0+0*4] = Xarr[0];
        xform_matrix_sol[0+1*4] = Xarr[1];
        xform_matrix_sol[0+2*4] = 0.0;
        xform_matrix_sol[0+3*4] = Xarr[2];

	// set row 1
        xform_matrix_sol[1+0*4] = Xarr[3];
        xform_matrix_sol[1+1*4] = Xarr[4];
        xform_matrix_sol[1+2*4] = 0.0;
        xform_matrix_sol[1+3*4] = Xarr[5];

	// set nonzero elements of rows 2 and 3
        xform_matrix_sol[2+2*4] = 1.0;
        xform_matrix_sol[3+3*4] = 1.0;
        *error = 0;
        for (i = n-p; i < m; i++)
            *error += C[i]*C[i];

    } else if (type == ZERO_ROTATION_2D2D) {
	/* this is just like the GENERAL_2D2D case except that we
	   constrain either b or e to be 0.0, this is pretty similar
	   to the previous case except that B is different and we do
	   the minimization twice (for the case of shear in x and then
	   again for shear in y)
	*/
        if (c.numPoints() < 3) {
            fprintf(stderr, "Error: need at least 3 points\n");
            return -1;
        }
        int m,n,p;
        double *A = NULL;
        double *C = NULL;

        p = 1;  // we have 1 constraint here (1 at a time anyway)
        n = 6;
        m = 2*c.numPoints();
        A = new double[m*n];
        double B_shearX[6] = { 	0.0,
                         	0.0,
                         	0.0,
                         	1.0,
                         	0.0,
                         	0.0};
	double error_shearX = 0.0;

	double B_shearY[6] = {	0.0,
			      	1.0,
				0.0,
				0.0,
				0.0,
				0.0};
	double error_shearY = 0.0;

        C = new double[m];
        double D[1];
        double X_shearX[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
	double X_shearY[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
        corr_point_t pt;

        int npt = c.numPoints();

	D[0] = 0.0;

        for (i = 0; i < npt; i++){
            c.getPoint(im0, i, &pt);
            im0pt[0] = scale_ratio_x_to_y*pt.x + shear_x*pt.y;
            im0pt[1] = pt.y + shear_y*pt.x;
            im0pt[2] = pt.z;
            q_xform(im0pt, quat, im0pt);
            A[i] = im0pt[0];    A[npt+i] = 0.0;
            A[m+i] = im0pt[1];  A[m+npt+i] = 0.0;
            A[2*m+i] = 1.0;     A[2*m+npt+i] = 0.0;
            A[3*m+i] = 0.0;     A[3*m+npt+i] = A[i];
            A[4*m+i] = 0.0;     A[4*m+npt+i] = A[m+i];
            A[5*m+i] = 0.0;     A[5*m+npt+i] = A[2*m+i];
            c.getPoint(im1, i, &pt);
            C[i] = pt.x;
            C[i+npt] = pt.y;
        }


        if (linearEqualityConstrainedLeastSquaresSolve(m,n,p,A,B_shearX,
		C,D,X_shearX)){
            fprintf(stderr, "Error: lineqconstrleastsqrsolve failed\n");
            return -1;
        }
        error_shearX = 0;
        for (i = n-p; i < m; i++)
            error_shearX += C[i]*C[i];

	// now do it again for shear in y:

        if (linearEqualityConstrainedLeastSquaresSolve(m,n,p,A,B_shearY,
                C,D,X_shearY)){
            fprintf(stderr, "Error: lineqconstrleastsqrsolve failed\n");
            return -1;
        }
	error_shearY = 0.0;
	for (i = n-p; i < m; i++)
            error_shearY += C[i]*C[i];

	// set xform_matrix_sol from the best solution:

	// first set things that are the same in both solutions:
        // initialize rows 2 and 3 to 0
        for (i = 0; i < 4; i++){
            xform_matrix_sol[2+i*4] = 0.0;
            xform_matrix_sol[3+i*4] = 0.0;
        }

	// set zeros in column 2
	xform_matrix_sol[0+2*4] = 0.0;
	xform_matrix_sol[1+2*4] = 0.0;

	// set nonzero elements of rows 2 and 3
	xform_matrix_sol[2+2*4] = 1.0;
	xform_matrix_sol[3+3*4] = 1.0;

	// now we fill in solution values
	if (error_shearY < error_shearX){
            // set elements in row 0
            xform_matrix_sol[0+0*4] = X_shearY[0];
            xform_matrix_sol[0+1*4] = X_shearY[1];
            xform_matrix_sol[0+3*4] = X_shearY[2];

            // set elements in row 1
            xform_matrix_sol[1+0*4] = X_shearY[3];
            xform_matrix_sol[1+1*4] = X_shearY[4];
            xform_matrix_sol[1+3*4] = X_shearY[5];

	    *error = error_shearY;
	}
	else {
            // set elements in row 0
            xform_matrix_sol[0+0*4] = X_shearX[0];
            xform_matrix_sol[0+1*4] = X_shearX[1];
            xform_matrix_sol[0+3*4] = X_shearX[2];

            // set elements in row 1
            xform_matrix_sol[1+0*4] = X_shearX[3];
            xform_matrix_sol[1+1*4] = X_shearX[4];
            xform_matrix_sol[1+3*4] = X_shearX[5];

	    *error = error_shearX;
	}

    } else if (type == ZERO_ROTATION_ZERO_SHEAR_UNIFORM_SCALE_2D2D) 
 	/* here we only do uniform scaling and translation
		so our transformation matrix looks like:
		[  a   0.0   0.0     b
		 0.0     a   0.0     c
                 0.0   0.0   1.0   0.0
                 0.0   0.0   0.0   1.0]


		a*x_grid0 + b*1.0 + c*0.0 = x_image0
		a*y_grid0 + b*0.0 + c*1.0 = y_image0
		a*x_grid1 + b*1.0 + c*0.0 = x_image1
		a*y_grid1 + b*0.0 + c*1.0 = y_image1

		m = # points (2+)
		n = 3

		A = [	x_grid0, y_grid0, x_grid1, y_grid1, ...
			1.0,     0.0,     1.0,     0.0, ...
			0.0,     1.0,     0.0,     1.0, ...]

		B = [   x_image0, y_image0, x_image1, y_image1, ...]
	*/

    {
        if (c.numPoints() < 2) {
            fprintf(stderr, "Error: need at least 2 points\n");
            return -1;
        }
        int m,n;
        double *A = NULL;
        double *B = NULL;
        n = 3;
        m = c.numPoints()*2;
        A = new double[m*n];
        B = new double[m];
        corr_point_t p;
	int npt = c.numPoints();

        for (i = 0; i < npt; i++){
            c.getPoint(im0, i, &p);
 //           printf("(%g,%g) -->", p.x, p.y);
            im0pt[0] = p.x;
            im0pt[1] = p.y;
            im0pt[2] = p.z;
            q_xform(im0pt, quat, im0pt);
            A[2*i] = im0pt[0];	A[2*i+1] = im0pt[1];
            A[m+2*i] = 1.0;	A[m+2*i+1] = 0.0;
	    A[2*m+2*i] = 0.0;	A[2*m+2*i+1] = 1.0;
            c.getPoint(im1, i, &p);
  //          printf(" (%g,%g)\n", p.x, p.y);
            B[2*i] = p.x;
	    B[2*i+1] = p.y;
        }
        if (linearLeastSquaresSolve(m,n,A,B)){
            fprintf(stderr, "Error: linleastsqr solver failed\n");
            return -1;
        }
        xform_matrix_sol[0+0*4] = B[0];
	xform_matrix_sol[0+1*4] = 0.0;
	xform_matrix_sol[0+2*4] = 0.0;
	xform_matrix_sol[0+3*4] = B[1];
	xform_matrix_sol[1+0*4] = 0.0;
	xform_matrix_sol[1+1*4] = B[0];
	xform_matrix_sol[1+2*4] = 0.0;
	xform_matrix_sol[1+3*4] = B[2];
	xform_matrix_sol[2+0*4] = 0.0;
	xform_matrix_sol[2+1*4] = 0.0;
	xform_matrix_sol[2+2*4] = 1.0;
	xform_matrix_sol[2+3*4] = 0.0;
	xform_matrix_sol[3+0*4] = 0.0;
	xform_matrix_sol[3+1*4] = 0.0;
	xform_matrix_sol[3+2*4] = 0.0;
	xform_matrix_sol[3+3*4] = 1.0;

	*error = 0.0;
	for (i = 3; i < m; i++)
	   *error += B[i]*B[i];

    }

    qogl_matrix_mult (xform_matrix, pre_xform_matrix,xform_matrix_sol);

    return 0;
}

// this is just a little debugging hack that I made (AAS)
// This function takes the output from transformSolver and gives you a
// direction of projection assuming the input matrix is an orthogonal projection

// essentially, it solves for the point (using a probably numerical-error-
// prone algorithm) with z = 1.0 that projects to the same point as the origin
// (as an example of where this could go wrong, consider any projection in
// a direction that lies in the x-y plane). It does seem to give the right
// answer most of the time;
int computeProjectionVector(double *xform_matrix, double *vec) {

    double row0[4];
    double row1[4];
    double diff[4];
    double x,y,z;
    int i;

    for (i = 0; i < 3; i++){
        row0[i] = xform_matrix[i*4];
        row1[i] = xform_matrix[i*4+1];
    }

    row0[3] = 0.0;
    row1[3] = 0.0;

    double fact0;

    if (row1[0] == 0.0){
        fact0 = row0[0]/row1[0];
        for (i = 1; i < 3; i++)
            row1[i] *= fact0;
        row1[0] = row0[0];

    } else {
        fact0 = row1[0]/row0[0];
        for (i = 1; i < 3; i++)
            row0[i] *= fact0;
        row0[0] = row1[0];
    }

    diff[0] = 0.0;
    diff[1] = row0[1] - row1[1];
    diff[2] = row0[2] - row1[2];
    diff[3] = 0.0;
    if (diff[1] == 0 && diff[2] == 0){
	if (xform_matrix[0] == 0 && xform_matrix[1] == 0){
	    vec[0] = 1.0; vec[1] = 0.0; vec[2] = 0.0;
	    return 0;
	}
	else{ 
	  fprintf(stderr, "Error: computeProjectionVector: bad input matrix\n");
	  return -1;
	}
    }
    if (diff[1] == 0){
	y = 1.0;
	z = -y*diff[1]/diff[2];
    }
    else {
	z = 1.0;
    	y = -z*diff[2]/diff[1];
    }
    if (xform_matrix[0] != 0.0)
        x = -(xform_matrix[4]*y + xform_matrix[8]*z)/xform_matrix[0];
    else
        x = -(xform_matrix[5]*y + xform_matrix[9]*z)/xform_matrix[1];
    double mag = sqrt(x*x + y*y + z*z);
    vec[0] = x/mag;
    vec[1] = y/mag;
    vec[2] = z/mag;
    return 0;
}


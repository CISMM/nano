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
#define M_PI        3.14159265358979323846
#endif

/* the transformation is returned in xform_matrix which given in this order:
 
   [ m0 m4 m8  m12 ] [srcX]   [destX]
   [ m1 m5 m9  m13 ] [srcY] = [destY]
   [ m2 m6 m10 m14 ] [srcZ]   [destZ]
   [ m3 m7 m11 m15 ] [srcW]   [destW]
*/


int transformSolver(double *xform_matrix, double *error,
        Correspondence &c, int im0, int im1, nmr_TransformationType type)
{
  if (c.numPoints() < numPointConstraintsToSpecify(type)) {
    fprintf(stderr, "transformSolver: Error, not enough points: "
            "got %d, needed %d\n", c.numPoints(),
            numPointConstraintsToSpecify(type));
    return -1;
  }

  corr_point_t p0, p1;
  int i;

  switch (type) {
// ********************
    case NMR_TRANSLATION:
// ********************
        /* 
            The resulting transformation matrix has this form:

            1 0 0 a
            0 1 0 b
        */

        // we find a and b by taking the average of translations over 
        //    all the points
        {
        double a = 0;
        double b = 0;
        for (i = 0; i < c.numPoints(); i++){
          c.getPoint(im0, i, &p0);
          c.getPoint(im1, i, &p1);
          a += (p1.x - p0.x);
          b += (p1.y - p0.y);
        }
        a /= (double)c.numPoints();
        b /= (double)c.numPoints();

        // now we compute the error:
        *error = 0;
        double errX, errY;
        for (i = 0; i < c.numPoints(); i++){
          c.getPoint(im0, i, &p0);
          c.getPoint(im1, i, &p1);
          errX = p1.x - p0.x - a;
          errY = p1.y - p0.y - b;
          *error += errX*errX + errY*errY;
        }

        // fill in the entries in the transform matrix for translation
        for (i = 0; i < 16; i++){
          xform_matrix[i] = 0;
        }
        xform_matrix[0] = 1;
        xform_matrix[5] = 1;
        xform_matrix[10] = 1;
        xform_matrix[15] = 1;
        xform_matrix[12] = a;
        xform_matrix[13] = b;
        }
        break;

// **********************************
    case NMR_TRANSLATION_ROTATION_SCALE:
// **********************************
        /*
            The resulting transform matrix has this form:

            a -b 0 c
            b  a 0 d

            or alternatively:

            a b 0 d
            e f 0 h

            with the constraints: b = -e, a = f

        */

/* **************************************************************************
   we can put all this together in a single problem of 
   minimizing || C - A*X ||_2
   which is the form required by linearEqualityConstrainedLeastSquaresSolve()

   A = [im0.pt0.x, im0.pt1.x, im0.pt2.x,...,0.0, 0.0, 0.0, ...
        im0.pt0.y, im0.pt1.y, im0.pt2.y,...,0.0, 0.0, 0.0, ...
        1.0,     1.0,     1.0,          ...,0.0, 0.0, 0.0, ...
        0.0, 0.0, 0.0,                  ...,im0.pt0.x, im0.pt1.x, im0.pt2.x, ...
        0.0, 0.0, 0.0,                  ...,im0.pt0.y, im0.pt1.y, im0.pt2.y, ...
        0.0, 0.0, 0.0,                  ...,1.0,     1.0,     1.0,     ...]

   C = [im1.pt0.x, im1.pt1.x, im1.pt2.x,...,im1.pt0.y, im1.pt1.y, im1.pt2.y,...]

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
   that has any usefulness for the registration problem.
   as a matrix expression this constraint would be:

   BX = D where
   B = [1.0, 0.0,
        0.0, 1.0,
        0.0, 0.0,
        0.0, 1.0,
        -1.0, 0.0,
        0.0, 0.0]
   D = [0.0, 0.0]

*************************************************************************  */
        {
        int numPts = c.numPoints();
        int numVars = 6;
        int numEq = 2*c.numPoints(); // number of equations (rows in A)
        int numConstraints = 2;
        double *A, *C;
        double B[12] = {1.0, 0.0,
                        0.0, 1.0,
                        0.0, 0.0,
                        0.0, 1.0,
                        -1.0, 0.0,
                        0.0, 0.0};
        double D[2] = {0.0, 0.0};
        double Xarr[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
        C = new double[numEq];
        A = new double[numEq*numVars];

        // setup for solving for first row of matrix
        for (i = 0; i < c.numPoints(); i++){
          c.getPoint(im0, i, &p0);
          c.getPoint(im1, i, &p1);
          A[i] = p0.x;              A[numPts + i] = 0.0;
          A[numEq+i] = p0.y;        A[numEq + numPts + i] = 0.0;
          A[2*numEq+i] = 1.0;       A[numEq*2 + numPts + i] = 0.0;

          A[3*numEq+i] = 0.0;       A[3*numEq+numPts+i] = A[i];
          A[4*numEq+i] = 0.0;       A[4*numEq+numPts+i] = A[numEq+i];
          A[5*numEq+i] = 0.0;       A[5*numEq+numPts+i] = A[2*numEq+i];

          C[i] = p1.x;
          C[numPts + i] = p1.y;
        }

        if (linearEqualityConstrainedLeastSquaresSolve(numEq, numVars, 
             numConstraints, A, B, C, D, Xarr)) {
          fprintf(stderr, "Error: lineqconstrleastsqrsolve failed\n");
          return -1;
        }

        // initialize rows 2 and 3 to 0
        for (i = 0; i < 4; i++){
            xform_matrix[2+i*4] = 0.0;
            xform_matrix[3+i*4] = 0.0;
        }

        // set row 0
        xform_matrix[0+0*4] = Xarr[0];
        xform_matrix[0+1*4] = Xarr[1];
        xform_matrix[0+2*4] = 0.0;
        xform_matrix[0+3*4] = Xarr[2];

        // set row 1
        xform_matrix[1+0*4] = Xarr[3];
        xform_matrix[1+1*4] = Xarr[4];
        xform_matrix[1+2*4] = 0.0;
        xform_matrix[1+3*4] = Xarr[5];

        // set nonzero elements of rows 2 and 3
        xform_matrix[2+2*4] = 1.0;
        xform_matrix[3+3*4] = 1.0;
        *error = 0;
        for (i = numVars-numConstraints; i < numEq; i++)
            *error += C[i]*C[i];

        delete [] A;
        delete [] C;
        }
        break;

// *******************
    case NMR_2D2D_AFFINE:
// *******************
        /*
            The resulting transform matrix has this form:

            a b 0 d
            e f 0 h
        */

/* ********************************************************************
        a*im0.pt0.x + b*im0.pt0.y + d = im1.pt0.x
        a*im0.pt1.x + b*im0.pt1.y + d = im1.pt1.x
        a*im0.pt2.x + b*im0.pt2.y + d = im1.pt2.x
        ...

        --> A = [im0.pt0.x, im0.pt1.x, im0.pt2.x, ...
                 im0.pt0.y, im0.pt1.y, im0.pt2.y, ...
                 1.0,     1.0,     1.0,     ...]
            B = [im1.pt0.x, im1.pt1.x, im1.pt2.x, ...]
            m = 3+, n = 3

        --> solve for X = [a b d]

        e*x_grid0 + f*y_grid0 + h = y_image0
        e*x_grid1 + f*y_grid1 + h = y_image1
        e*x_grid2 + f*y_grid2 + h = y_image2
        ...

        --> A = [im0.pt0.x, im0.pt1.x, im0.pt2.x, ...
                 im0.pt0.y, im0.pt1.y, im0.pt2.y, ...
                 1.0,     1.0,     1.0,     ...]
            B = [im1.pt0.y, im1.pt1.y, im1.pt2.y, ...]
            m = 3+, n = 3

        --> solve for X = [e f h]

        xform_matrix = [a b 0 d]
                       [e f 0 h]
                       [0 0 1 0]
                       [0 0 0 1]

  ******************************************************************** */
        {
        int numPts = c.numPoints();
        // we find the solution in two stages and these are the settings
        // for one of them
        int numVars = 3;
        int numEq = c.numPoints(); // number of equations (rows in A)
        double *A = new double[numEq*numVars];
        double *B = new double[numEq];

        for (i = 0; i < c.numPoints(); i++){
          c.getPoint(im0, i, &p0);
          c.getPoint(im1, i, &p1);
        
          A[i] = p0.x;
          A[numEq+i] = p0.y;
          A[2*numEq+i] = 1.0;
          B[i] = p1.x;
        }
        if (linearLeastSquaresSolve(numEq,numVars,A,B)){
          fprintf(stderr, "Error: linleastsqr solver failed\n");
          return -1;
        }

        xform_matrix[0*4] = B[0];   //a
        xform_matrix[1*4] = B[1];   //b
        xform_matrix[2*4] = 0.0;
        xform_matrix[3*4] = B[2];   //d

        *error = 0;
        for (i = 3; i < numEq; i++)
          *error += B[i]*B[i];

        for (i = 0; i < c.numPoints(); i++){
          c.getPoint(im0, i, &p0);
          c.getPoint(im1, i, &p1);

          A[i] = p0.x;
          A[numEq+i] = p0.y;
          A[2*numEq+i] = 1.0;
          B[i] = p1.y;
        }
        if (linearLeastSquaresSolve(numEq,numVars,A,B)){
          fprintf(stderr, "Error: linleastsqr solver failed\n");
          return -1;
        }

        for (i = 0; i < 4; i++){
          xform_matrix[2+i*4] = 0.0;
          xform_matrix[3+i*4] = 0.0;
        }
        xform_matrix[1+0*4] = B[0]; //e
        xform_matrix[1+1*4] = B[1]; //f
        xform_matrix[1+2*4] = 0.0;
        xform_matrix[1+3*4] = B[2]; //h
        xform_matrix[2+2*4] = 1.0;
        xform_matrix[3+3*4] = 1.0;

        for (i = 3; i < numEq; i++)
           *error += B[i]*B[i];
        }
        break;
    default:
        fprintf(stderr, "transformSolve: This shouldn't happen\n");
        return -1;
        break;
  }
  return 0;
}

/* returns how many correspondence points are needed to fully constrain
   the solution for the given type of transformation
*/
int numPointConstraintsToSpecify(nmr_TransformationType type)
{
  int result = 0;
  switch (type) {
    case NMR_TRANSLATION:
      result = 1;
      break;
    case NMR_TRANSLATION_ROTATION_SCALE:
      result = 2;
      break;
    case NMR_2D2D_AFFINE:
      result = 3;
      break;
    default:
      fprintf(stderr, "numPointConstraintsToSpecify: this shouldn't happen\n");
      break;
  }
  return result;
}

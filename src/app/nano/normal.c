#include	<stdio.h>
#include	<vrpn_Shared.h>
#include	<sys/types.h>
#include	<errno.h>
#include 	<math.h>

#include	<v.h>  // for VectorType

#include	"mf.h"
#include	"BCPlane.h"

#include	"normal.h"

#if (!defined(X) || !defined(Y) || !defined(Z))
#define	X	(0)
#define	Y	(1)
#define	Z	(2)
#endif

/******************************************************************************
 *	This routine will compute the normal for the point with the given     *
 * coordinates in the grid.  It offsets the value that is passed so that it   *
 * refers to the part of the grid for which space has been allocated.	      *
 **mmm adapted from version on GP's.  could be better, but fine for now	      *
 ******************************************************************************/
int Compute_Norm(int x, int y,VectorType Normal, BCPlane* plane)
{
	double	dx = (plane->maxX() - plane->minX()) / (plane->numX()-1);
	double	dy = (plane->maxY() - plane->minY()) / (plane->numY()-1);
        VectorType              xdiff_vec, ydiff_vec;
        VectorType              local_norm;
        int     i;

        /* Initially, clear the normal */
        for (i = 0; i < 3; i++) {
                Normal[i] = 0.0;
        }

DOCUMENT2( "Enter Compute_Normal(%d,%d)\n", x, y );
        /* Find the normal with 1 more in x */

        if ( (x < plane->numX()-1)&&(y < plane->numY()-1) ) {
                xdiff_vec[X] = dx;
                xdiff_vec[Y] = 0;
                xdiff_vec[Z] = plane->value(x+1,y) - plane->value(x, y);
		xdiff_vec[Z] *= plane->scale();
                ydiff_vec[X] = 0;
                ydiff_vec[Y] = dy;
                ydiff_vec[Z] = plane->value(x, y+1) - plane->value(x, y);
		ydiff_vec[Z] *= plane->scale();

                VectorCross(xdiff_vec,ydiff_vec, local_norm);
                VectorAdd(local_norm,Normal, Normal);

DOCUMENT3( "NE (%g,%g,%g)\n", local_norm[0], local_norm[1], local_norm[2] );
        }

        if ( (x > 0)&&(y < plane->numY()-1) ) {
                xdiff_vec[X] = dx;
                xdiff_vec[Y] = 0;
                xdiff_vec[Z] = plane->value(x, y) - plane->value(x-1, y);
		xdiff_vec[Z] *= plane->scale();
                ydiff_vec[X] = 0;
                ydiff_vec[Y] = dy;
                ydiff_vec[Z] = plane->value(x, y+1)- plane->value(x, y);
		ydiff_vec[Z] *= plane->scale();

                VectorCross(xdiff_vec,ydiff_vec, local_norm);
                VectorAdd(local_norm,Normal, Normal);

DOCUMENT3( "NW (%g,%g,%g)\n", local_norm[0], local_norm[1], local_norm[2] );
        }

        if ( (x < plane->numX()-1)&&(y > 0) ) {
                xdiff_vec[X] = dx;
                xdiff_vec[Y] = 0;
                xdiff_vec[Z] = plane->value(x+1, y) - plane->value(x, y);
		xdiff_vec[Z] *= plane->scale();
                ydiff_vec[X] = 0;
                ydiff_vec[Y] = dy;
                ydiff_vec[Z] = plane->value(x, y) - plane->value(x, y-1);
		ydiff_vec[Z] *= plane->scale();

                VectorCross(xdiff_vec,ydiff_vec, local_norm);
                VectorAdd(local_norm,Normal, Normal);

DOCUMENT3( "SE (%g,%g,%g)\n", local_norm[0], local_norm[1], local_norm[2] );
        }

        if ( (x > 0)&&(y > 0) ) {
                xdiff_vec[X] = dx;
                xdiff_vec[Y] = 0;
                xdiff_vec[Z] = plane->value(x, y) - plane->value(x-1, y);
		xdiff_vec[Z] *= plane->scale();
                ydiff_vec[X] = 0;
                ydiff_vec[Y] = dy;
                ydiff_vec[Z] = plane->value(x, y) - plane->value(x, y-1);
		ydiff_vec[Z] *= plane->scale();

                VectorCross(xdiff_vec,ydiff_vec, local_norm);
                VectorAdd(local_norm,Normal, Normal);

DOCUMENT3( "SW (%g,%g,%g)\n", local_norm[0], local_norm[1], local_norm[2] );
        }

        /* Normalize the normal */

	VectorNormalize(Normal);

	/* Copy the normal to the answer */

DOCUMENT3( "Final (%g,%g,%g)\n", Normal[0], Normal[1], Normal[2] );

        return(0);

}

/******************************************************************************
 *	This routine will compute the normal for the point with the given     *
 * coordinates in the grid.  It offsets the value that is passed so that it   *
 * refers to the part of the grid for which space has been allocated.	      *
 **mmm adapted from version on GP's.  could be better, but fine for now	      *
 ******************************************************************************/
int Compute_Norm(float x, float y,VectorType Normal, BCPlane* plane)
{
	double	dx = (plane->maxX() - plane->minX()) / (plane->numX()-1);
	double	dy = (plane->maxY() - plane->minY()) / (plane->numY()-1);
        VectorType              xdiff_vec, ydiff_vec;
        VectorType              local_norm;
        int     i;

        /* Initially, clear the normal */
        for (i = 0; i < 3; i++) {
                Normal[i] = 0.0;
        }

DOCUMENT2( "Enter Compute_Normal(%d,%d)\n", x, y );
        /* Find the normal with 1 more in x */

        if ( (x < plane->numX()-1)&&(y < plane->numY()-1) ) {
                xdiff_vec[X] = dx;
                xdiff_vec[Y] = 0;
                xdiff_vec[Z] = plane->interpolatedValue(x+1,y) - plane->interpolatedValue(x, y);
		xdiff_vec[Z] *= plane->scale();
                ydiff_vec[X] = 0;
                ydiff_vec[Y] = dy;
                ydiff_vec[Z] = plane->interpolatedValue(x, y+1) - plane->interpolatedValue(x, y);
		ydiff_vec[Z] *= plane->scale();

                VectorCross(xdiff_vec,ydiff_vec, local_norm);
                VectorAdd(local_norm,Normal, Normal);

DOCUMENT3( "NE (%g,%g,%g)\n", local_norm[0], local_norm[1], local_norm[2] );
        }

        if ( (x > 0)&&(y < plane->numY()-1) ) {
                xdiff_vec[X] = dx;
                xdiff_vec[Y] = 0;
                xdiff_vec[Z] = plane->interpolatedValue(x, y) - plane->interpolatedValue(x-1, y);
		xdiff_vec[Z] *= plane->scale();
                ydiff_vec[X] = 0;
                ydiff_vec[Y] = dy;
                ydiff_vec[Z] = plane->interpolatedValue(x, y+1)- plane->interpolatedValue(x, y);
		ydiff_vec[Z] *= plane->scale();

                VectorCross(xdiff_vec,ydiff_vec, local_norm);
                VectorAdd(local_norm,Normal, Normal);

DOCUMENT3( "NW (%g,%g,%g)\n", local_norm[0], local_norm[1], local_norm[2] );
        }

        if ( (x < plane->numX()-1)&&(y > 0) ) {
                xdiff_vec[X] = dx;
                xdiff_vec[Y] = 0;
                xdiff_vec[Z] = plane->interpolatedValue(x+1, y) - plane->interpolatedValue(x, y);
		xdiff_vec[Z] *= plane->scale();
                ydiff_vec[X] = 0;
                ydiff_vec[Y] = dy;
                ydiff_vec[Z] = plane->interpolatedValue(x, y) - plane->interpolatedValue(x, y-1);
		ydiff_vec[Z] *= plane->scale();

                VectorCross(xdiff_vec,ydiff_vec, local_norm);
                VectorAdd(local_norm,Normal, Normal);

DOCUMENT3( "SE (%g,%g,%g)\n", local_norm[0], local_norm[1], local_norm[2] );
        }

        if ( (x > 0)&&(y > 0) ) {
                xdiff_vec[X] = dx;
                xdiff_vec[Y] = 0;
                xdiff_vec[Z] = plane->interpolatedValue(x, y) - plane->interpolatedValue(x-1, y);
		xdiff_vec[Z] *= plane->scale();
                ydiff_vec[X] = 0;
                ydiff_vec[Y] = dy;
                ydiff_vec[Z] = plane->interpolatedValue(x, y) - plane->interpolatedValue(x, y-1);
		ydiff_vec[Z] *= plane->scale();

                VectorCross(xdiff_vec,ydiff_vec, local_norm);
                VectorAdd(local_norm,Normal, Normal);

DOCUMENT3( "SW (%g,%g,%g)\n", local_norm[0], local_norm[1], local_norm[2] );
        }

        /* Normalize the normal */

	VectorNormalize(Normal);

	/* Copy the normal to the answer */

DOCUMENT3( "Final (%g,%g,%g)\n", Normal[0], Normal[1], Normal[2] );

        return(0);

}

#ifdef _GRID_NORMALS_	/* update normals not in use */
/* Modify the normal for this point. 
 * Then replace the normal at this point.  For every
 * neighbor that we draw, recompute the normal.
 */

int Update_Normals(int x, int y)
{
/*
       if ( (y >= 0) && (y < plane->num_y) ) {
	       Compute_Norm(x,y);
       }
       if ( (y >= 0) && (y < plane->num_y) &&
	    (x > 0) ) {
	       Compute_Norm(x-1,y);
       }
       if ( (y >= 0) && (y < plane->num_y) &&
	    ( x < (stm_grid.num_x - 1) ) ) {
	       Compute_Norm(x+1,y);
       }
       if (y > 0) {
	       Compute_Norm(x,y-1);
       }
       if (y < (stm_grid.num_y-1)) {
	       Compute_Norm(x,y+1);
       }
*/
       return 0;
}
#endif /* _GRID_NORMALS_ */






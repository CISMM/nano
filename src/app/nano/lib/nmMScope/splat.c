#include "splat.h"

#include <math.h>
#include <stdio.h>
#include <malloc.h>

#include <BCGrid.h>
#include <BCPlane.h>
#include <Point.h>

#include <nmb_Dataset.h>

#define	N_SIGS		(5)
#define MAX_EXT		(25)
#define INIT_OVER	(8)

#define	TIP_WIDTH	(10.0)		/* half width of tip in nm */

static	double	OVER = INIT_OVER;  /* oversampling of filter */
static	int	X_Ext = 0;
static	int	Y_Ext = 0;
static	float	**SplatFilter = NULL;

/* mkSplat - determine the appropriate size for a gaussian multiplication
** kernel with width about the width of the tip.  TIP_WIDTH should be
** a variable, not a define.  care taken not to let the kernel get too
** big.  the kernel is just an oversampled gaussian, we find the closest
** matching point in doing the multiply.
**/
float ** mkSplat (BCGrid * grid)
{
  double	dx= (grid->maxX() - grid->minX()) / (grid->numX() - 1);
  double	dy= (grid->maxY() - grid->minY()) / (grid->numY() - 1);
  double	dmx = dx/OVER;
  double	dmy = dy/OVER;
  double	denom = 1.0/(2*TIP_WIDTH*TIP_WIDTH);
  int		mx, my;

  /* if we had to drop the oversampling rate before, see if we're good
  ** to bring it back up
  **/
  if( OVER < INIT_OVER ) {
    OVER = INIT_OVER;
    dmx = dx/OVER;
    dmy = dy/OVER;
  }

  if (!dx || !dy || !dmx || !dmy) {
    fprintf(stderr, "mkSplat:  fatal error!\n");
    return;
  }

  /* if we've already made one, deallocate and make up a fresh one 
     To free the memory correctly, we have to subtract an offset from 
     SplatFilter because it was added at allocation (see below)
  */
  if( SplatFilter ) {
    for( mx = -X_Ext; mx <= X_Ext; mx++ )
	free( (char*)(SplatFilter[mx]-Y_Ext) );
    free( (char*)(SplatFilter-X_Ext) );
  }
    
  /* establish how big a filter we need 
  */
  X_Ext = (int)(N_SIGS*TIP_WIDTH/dmx + 1);
  Y_Ext = (int)(N_SIGS*TIP_WIDTH/dmy + 1);

  /* if the filter is too big, lower the oversampling rate to fit
  ** as big as we can
  **/
  if( X_Ext > MAX_EXT ) {
    X_Ext = MAX_EXT;
    OVER = (dx*X_Ext)/(N_SIGS*TIP_WIDTH);
    dmx = dx/OVER;
    dmy = dy/OVER;
    Y_Ext = (int)(N_SIGS*TIP_WIDTH/dmy + 1);
  }
  if( Y_Ext > MAX_EXT ) {
    Y_Ext = MAX_EXT;
    OVER = (dy*Y_Ext)/(N_SIGS*TIP_WIDTH);
    dmx = dx/OVER;
    dmy = dy/OVER;
    X_Ext = (int)(N_SIGS*TIP_WIDTH/dmx + 1);
  }

  /* allocate and fill the new filter 
     An offset is added so that indexing is symmetric.
  */
  SplatFilter = ( float ** )calloc( 2*X_Ext+1, sizeof( float * ) ) + X_Ext;
  for( mx = -X_Ext; mx <= X_Ext; mx++ ) {
    SplatFilter[mx] = ( float * )calloc( 2*Y_Ext+1, sizeof( float ) ) + Y_Ext;
    for( my = -Y_Ext; my <= Y_Ext; my++ ) {
      SplatFilter[mx][my] = exp( -(mx*dmx*mx*dmx+my*dmy*my*dmy)*denom );
    }
  }

  return SplatFilter;
}

/* just multiply the oversampled kernel we've already made with the
** grid.
**/
int ptSplat (int * lost_changes, BCGrid * grid, Point_results * inputPoint)
{
//XXX This should do all of the planes in the grid, not just height and color
	BCPlane* heightPlane = grid->getPlaneByName("height");
	if (heightPlane == NULL) {
	    fprintf(stderr, "Error in ptSplat: could not get height grid!\n");
	    return -1;
	}     

	BCPlane* colorPlane = grid->getPlaneByName("color");
	if (colorPlane == NULL) {
	    fprintf(stderr, "Error in ptSplat: could not get color grid!\n");
	    return -1;
	}       

	Point_value * value = inputPoint->getValueByName("Topography");
	if (value == NULL) {
		fprintf(stderr, "ptSplat(): could not get value!\n");
		return -1;
	}

	float 		x = (value->results()->x() - heightPlane->minX()) *
			    heightPlane->derangeX();
	float 		y = (value->results()->y() - heightPlane->minY()) *
			    heightPlane->derangeY();
	int		ix = (int)x+1;
	int		iy = (int)y+1;
	int		xoffset = (int)((ix-x) * OVER);
	int		yoffset = (int)((iy-y) * OVER);
	int		mx, my;
	unsigned	gx, gy;

	lost_changes = lost_changes;

	for( mx = (int)(-X_Ext+xoffset); mx <= X_Ext; mx+=(int)OVER ) {
	  for( my = (int)(-Y_Ext+yoffset); my <= Y_Ext; my+=(int)OVER ) {
	    
	    gx = (int)(ix +( mx - xoffset )/OVER);
	    gy = (int)(iy +( my - yoffset )/OVER);

	    //XXX Needs to be changed up update all current planes that
	    //XXX match incoming point values.
	    if( ( gx < (unsigned)grid->numX())
		&&
		( gy < (unsigned)grid->numY()) ) {
	       heightPlane->setValue(gx, gy, heightPlane->value(gx, gy) * ( 1 - SplatFilter[mx][my] ));
	       heightPlane->setValue(gx, gy, heightPlane->value(gx, gy) +
			SplatFilter[mx][my] * value->value());
	       } /* end if inside the grid */
	    } /* end for y */
	  } /* end for x */

      return 0;
}
	

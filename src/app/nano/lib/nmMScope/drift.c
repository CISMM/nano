/* drift compensation
**/

#include <time.h>
#ifndef _WIN32
#include <sys/time.h>  // struct timeval  time before types!
#else
#include <vrpn_Shared.h>  // gets us struct timeval one way or another
#endif
#include <sys/types.h>

struct	timeval	driftT0 = { 0L, 0L };
int		driftZClean = 0;
int		prevZClean = 0;
double		driftZRate = 0.0;
double		driftZOffset = 0.0;
double		driftZAccum = 0.0;
double		driftZAvg = 0.0;
long		driftZNum = 0;

/* driftZDirty - flag current average as dirty
** Z averages get dirty by 
**	a) force constants changing
**	b) modes changing
**	d) scan regions changing
**/
int driftZDirty()
  {
  driftZClean = 0;

  return 0;
  }

/* drift_init - if we have a clean average Z going calculate 
** the drift rate.  if no clean average Z, stay with previous rate.
** then initialize for collecting the next average.
** Note that we need two consecutive clean sweeps to calculate the drift.
**/
int drift_init(long sec, long usec)
{
  double		driftDT; 
  double		oldZAvg;
  long			dsec, dusec;

  dsec = sec - driftT0.tv_sec;
  dusec = usec - driftT0.tv_usec;
  if( dusec < 0 ){
    dsec--;
    dusec += 1000000;
    }

  driftDT = dsec*1000.0 + dusec/1000.0;

  /* add this scan's drift into our offset
  **/
  driftZOffset += driftZRate * driftDT;

  if( !(driftZClean && driftZNum && prevZClean ) ) {
    driftZRate /= 2.0;
    }

  /* if we have a good set of data, figure current rate
  **/
  if( driftZClean && driftZNum ) {
       oldZAvg = driftZAvg;
       driftZAvg = driftZAccum/driftZNum;
       if( prevZClean ) {
	  driftZRate = ( driftZAvg - oldZAvg )/driftDT;
	  }
       prevZClean = 1;
       }

  /* now initialize for next set
  **/
  driftZClean = 1;
  driftZAccum = 0.0;
  driftZNum = 0;
  driftT0.tv_sec = sec;
  driftT0.tv_usec = usec;

  return 0;
  }

/* drift_comp - correct this Z value by adding an offset based on
** total time till the beginning of this scan at T0, and a fractional
** offset which is the current drift rate times the time since the
** scan began.
** initialization is performed BEFORE correcting this point 
** every other time we hit the first pixel (0,0) (remember, raster
** mode returns backscan data, and boustro hits it twice, coming
** and going).
**/
double drift_comp (int x, int y, double val, long sec, long usec,
                   int do_y_fastest )
{
  double	DT;
  //static	int	odd_scan = 0;
  static	int	last_x = -1;
  static	int	last_y = -1;

  /* initialize on beginning of forward scan line.  this marked by
  ** x++, y (do_y_fastest) or x, y++ (!do_y_fastest).
  ** note that first row/col never contributes (not worth a x%num_x).
  ** initialize again on start of backscan, where we repeat last point.
  **/
  if (do_y_fastest && (x == (last_x + 1)) && (y == last_y)) {
    drift_init(sec, usec);
  }
  else if (!do_y_fastest && (x == last_x) && (y == (last_y + 1))) {
    drift_init(sec, usec);
  }
  else if ((x == last_x) && (y == last_y)) {
    drift_init(sec, usec);
    prevZClean = 0;
  }

  last_x = x;
  last_y = y;
    
  /*
  if( !( x || y )&&(odd_scan = !odd_scan) ) {
    drift_init( sec, usec );
    }
  **/

  sec -= driftT0.tv_sec;
  usec -= driftT0.tv_usec;
  if( usec < 0 ){
    sec--;
    usec += 1000000;
    }

  driftZAccum += val;
  driftZNum++;

  DT = sec*1000.0 + usec/1000.0;

  val -= driftZOffset + DT*driftZRate;

  return val;
}



#include <math.h>


/* Relaxation compensation routines.
** Basic idea:
**   1)Constants tmin and tsep set with relax_set_???() routines.
**     These initialize the how long to ignore data as microscope
**     stabalizes, how long to average data, and how much separation
**     between data points is necessary to get accurate model.
**     Averaging is only important if the microscope is in single
**     sample mode.
**   2)Type of compensation, initial time, and nominal (assumed correct)
**     Z value set with one of the init routines (bias_init, relax_init).
**   3)relax_comp() can be called any time.  It's return value is stored
**     in variable RelaxComp.  RelaxComp is 0 if no compensation is to
**     be performed (relax_comp() is not called), -1 if the compensation
**     is in the data acquisition phase (so output of relax_comp() should
**     be ignored, and 1 if the output of relax_comp() is compensated and
**     ready to be used.
**
**   The system cycles through the following states:
**     IDLE: Initialized and waiting for the microscope to stabalize.
**     WAIT: Has the first data point (may be average of several samples)
**       and waiting for time to take 2nd data point (1/t only).
**     READY: Has compensation equation and is actively compensating samples.
**
**   BIAS is zeroth order compensation (offset only).  RELAX works on
**   equation z' = z + Z0 - k/t.
**
**   Microscape currently only uses the RELAX capabilities.  Note that
**   ideally, RELAX performs same function as BIAS, with k computed to
**   be zero, but measurement error will introduce drift.  If the biases
**   introduced into the measurements are known to be constant, the BIAS
**   version will perform better and with less latency introduced.
**   RELAX automatically converts to BIAS when abs(k/t) gets less than
**   REAL_SMALL (1.0e-3).
**
**   All times in milliseconds, all length in nanometers.
**/

/* states of the compensation */
#define RELAX_NULL	(0)
#define	RELAX_IDLE	(1)
#define	RELAX_WAIT	(2)
#define	RELAX_READY	(3)
#define	RELAX_OVER	(4)
#define	BIAS_IDLE	(5)
#define	BIAS_READY	(6)
#define REAL_SMALL	(1.0e-3)

/* holders of our current state */
static	long	relax_sec0 = 0;
static	long	relax_usec0 = 0;
static	int	relax_state = RELAX_NULL;
static	double	TMin = 1000.0, TSep = 100.0, TAvg = 10.0;
static 	double	TAvg_2 = 5.0; /* TAvg/2.0 	*/
static	double	TMax;
static	double	Z0 = 0.0;
static	double	t1, z1;
static	double	t2, z2;
static	int	n_avg = 0;

/* These are extern to allow display devices to report them.  They
** aren't actually used here.  Changes of their values must occur
** through the relax_set_???() routines to have any real effect.
**/
//extern	int	stmRxTmin;
//extern	int	stmRxTsep;

/* Set the time till the microscope has stabalized.  If the microscope
** takes longer than this to start sending data after a transition, the
** first data sent will be accepted as good.
**/
int relax_set_min (int tmin)
{
   TMin = tmin;

   return 0;
   }

/* Set the time between the first data point accepted and the second.
** Also set the length of time to average points on each end of the
** sampling interval.  Currently set to 1/20th of the separation interval
** because we have 10 fingers.
**/
int relax_set_sep (int tsep)
{
   TSep = tsep;

   TAvg = TSep/20+1;
   TAvg_2 = TAvg*0.5;
   TSep -= TAvg_2;

   return 0;
}

/* Identical interface as relax_init() so they can be swapped easily,
** even though bias isn't a function of time.  Will find an offset
** which will bring the average of the samples around tmin to zflat.
**/
int bias_init(long sec, long usec, double zflat)
{
   relax_sec0 = sec;
   relax_usec0 = usec;
   relax_state = BIAS_IDLE;
   Z0 = zflat;
   t1 = 0.0;
   z1 = 0.0;
   n_avg = 0;

   return 1;
   }

/* Initialize to do z' = z + zflat - k/( t - (sec/1000+usec*1000) )
** compensation.
**/
int relax_init(long sec, long usec, double zflat )
{
   relax_sec0 = sec;
   relax_usec0 = usec;
   relax_state = RELAX_IDLE;
   Z0 = zflat;
   t1 = 0.0;
   t2 = 0.0;
   z1 = 0.0;
   z2 = 0.0;
   n_avg = 0;

   return 1;
}


/* Acquire compensation equation and perform compensation as requested
** by init routines.  See top for discussion of algorithm.
**/
int relax_comp(long sec, long usec, float* z)
{
   static	double	k = 0.0;
   		double t;

   /* Subtract off initial time from this time, and convert to ms
   **/
   sec -= relax_sec0;
   usec -= relax_usec0;
   if( usec < 0 ){
     sec--;
     usec += 1000000;
     }
   t = (sec*1000.0 + usec/1000.0);

   /* what we do depends on what state we're in **/
   switch( relax_state ) {

     /* someone screwed up and didn't initialize properly.  don't do
     ** anything.  warn only if debugging on.
     **/
     case RELAX_NULL:
	return 0;

     /* Look for samples from TMin to TMin+TAvg to get current reported z
     **/
     case BIAS_IDLE:
	/* has microscope stabilized */
	if( t >= TMin ) {
	    t1 += t;
	    z1 += *z;
	    n_avg++;
	    /* have we averaged long enough */
	    if( t > (TMin+TAvg) ) {
	      t1 /= n_avg;
	      z1 /= n_avg;
	      relax_state = BIAS_READY;
	      Z0 -= z1;
	      *z += Z0;
	      n_avg = 0;
	      }  /* end averaged enough, gone READY */
	   return 1;
	   }  /* end microscope stabil */
	return -1;

     /* Just add the computed offset to this z value
     **/
     case BIAS_READY:
	*z += Z0;
	return 1;

     /* Look for samples for first data point 
     **/
     case RELAX_IDLE:
	/* microscope stable? */
	if( t >= TMin ) {
	    t1 += t;
	    z1 += *z;
	    n_avg++;
	    /* Averaged long enough ? */
	    if( t > (TMin+TAvg) ) {
	      t1 /= n_avg;
	      z1 /= n_avg;
	      relax_state = RELAX_WAIT;
	      n_avg = 0;
	      }  /* end averaged long enough, gone WAIT for 2nd */
	    }  /* end microscope stable */
	return -1;

     /* Wait for samples to compute 2nd data point.  When we get them,
     ** go ahead and compute compensation coefficients.
     **/
     case RELAX_WAIT:
	/* Have we waited through separation interval? */
	if( t >= (t1 + TSep) ) {
	   t2 += t;
	   z2 += *z;
	   n_avg++;
	   /* Have we averaged enough of them? */
	   if( t >= (t1+TSep+TAvg_2) ) {
	      t2 /= n_avg;
	      z2 /= n_avg;
	      relax_state = RELAX_READY;

	      /* okay, we have two points as a function of time, so
	      ** we can calculate coefficients to z0 - k/t eqn.
	      **/
	      k = ( z1 - z2 )/( 1.0/t1 - 1.0/t2 );

	      TMax = fabs(k/REAL_SMALL);

	      Z0 -= z1 - k/t1;

	      /* go ahead an compensate this one */
	      *z += Z0 - k/t;
	      return 1;
	      }  /* end averaged enough, gone READY */
	   } /* end past separation interval */
	return -1;

	/* Have equation, just do compensation.  Switch over to BIAS only
	** when k/t gets insignificant.
	**/
	case RELAX_READY:
	   if( (t > TMax)||(t < 0.0) ) {
	     relax_state = BIAS_READY;
	     k = 0.0;
	     }
	*z += Z0 - k/t;
	return 1;

	/* anything else, try to stop system from calling us anymore, and
	** set to known state.
	**/
	default:
	   relax_state = RELAX_NULL;
	   return 0;

    } /* end switch */

  //return 1;

} /* end relax_comp */


/*===3rdtech===
  Copyright (c) 2000 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
/*********
 * nmm_RelaxComp
 * Purpose: encapsulate relaxation compensation
 * 
 * Please see "nmm_RelaxComp.h" for general useage instructions.
 * Technical details:
**   1)Constants tignore and tsep set with relax_set_???() routines.
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
**   equation z' = z + z_offset - k/t.
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

 *************/
#include <math.h>
#include "nmm_RelaxComp.h"

#include <nmm_MicroscopeRemote.h>

#define REAL_SMALL	(1.0e-3)

nmm_RelaxComp::nmm_RelaxComp(nmm_Microscope_Remote* m):
               microscope(m),
	       current_state(DISABLED),
	       type_of_compensation(nmm_RelaxComp::DECAY),
	       relax_sec0( 0),
	       relax_usec0(0),
	       TIgnore(500.0), 
	       TSep(500.0), 
	       TAvg(10.0),
	       TMax(0),
	       z_offset(0.0),
	       n_avg(0), 
	       decay_const(0.0)
{

}

void nmm_RelaxComp::updateMicroscope()
{
    if (microscope) {
        if (current_state == DISABLED) {
	    microscope->SetRelax((long)0, (long)0);
	} else {
	    microscope->SetRelax((long)TIgnore, (long)TSep);
	}
    }
}

void nmm_RelaxComp::enable(RelaxType relax_type)
{
    current_state = IDLE;
    type_of_compensation = relax_type;
/*
    if(microscope) {
	microscope->SetRelax((long)TIgnore, (long)TSep);
    }
*/
}    

void nmm_RelaxComp::disable()
{
    current_state = DISABLED;
/*
    if(microscope) {
	microscope->SetRelax((long)0, (long)0);
    }
*/
}    

vrpn_bool nmm_RelaxComp::is_enabled()
{
    return (vrpn_bool)!(current_state == DISABLED);
}

vrpn_bool nmm_RelaxComp::is_ignoring_points()
{
    return (vrpn_bool)((current_state == CALC_MODEL_POINT1)||
			(current_state == CALC_MODEL_POINT1));
}

/* Set the time till the microscope has stabilized.  If the microscope
** takes longer than this to start sending data after a transition, the
** first data sent will be accepted as good.
**/
int nmm_RelaxComp::set_ignore_time_ms (int tignore)
{
    // Only accept new ignore time if we are enabled.
    // If we are disabled, AFM will send us 0, we want
    // to ignore that update.
//    if (is_enabled()) {
      //printf("Relax ignore time set at %d\n", tignore);
	TIgnore = tignore;
 //   }

   return 0;
}

/* Set the time between the first data point accepted and the second.
** Also set the length of time to average points on each end of the
** separation interval.  Currently set to 1/20th of the separation interval.
**/
int nmm_RelaxComp::set_separation_time_ms (int tsep)
{
    // Only accept new ignore time if we are enabled.
    // If we are disabled, AFM will send us 0, we want
    // to ignore that update.
//    if (is_enabled()) {
      //printf("Relax separation time set at %d\n", tsep);
	TSep = tsep;

	// I have no idea why we add one ms to TAvg. Make sure it's not zero?
	TAvg = TSep/20+1;
//    }
   return 0;
}


/* Initialize to do z' = z + zflat - decay_const/( t - (sec/1000+usec*1000) )
** compensation.
**/
int nmm_RelaxComp::start_fix (long sec, long usec, double zflat )
{
   relax_sec0 = sec;
   relax_usec0 = usec;
   def_z = zflat;
   z_offset = 0.0;
   t1 = 0.0;
   t2 = 0.0;
   z1 = 0.0;
   z2 = 0.0;
   n_avg = 0;

   if (current_state == DISABLED) return 0;
   else current_state = CALC_MODEL_POINT1;
   return 0;
}


/* Acquire compensation equation and perform compensation as requested
** by init routines.  See top for discussion of algorithm.
**/
float nmm_RelaxComp::fix_height (long sec, long usec, float z)
{
    // Relaxation is disabled. Do nothing.
    if (current_state == DISABLED) return z;

    double t;  // time in msec since we started relaxing.
    
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
   switch( current_state ) {
   case IDLE:
       return z;

   case CALC_MODEL_POINT1:
       //See if has microscope stabilized 
       if( t < TIgnore ) {
	   return def_z;
       }
       if (type_of_compensation == CONST_OFFSET) {
	   //Average samples from TIgnore to TIgnore+TAvg to get current reported z
	   t1 += t;
	   z1 += z;
	   n_avg++;
	   /* have we averaged long enough ?*/
	   if( t > (TIgnore+TAvg) ) {
	       t1 /= n_avg;
	       z1 /= n_avg;
	       current_state = CONST_COMP;
	       z_offset = def_z - z1;
	       n_avg = 0;
	   }  // Ready to do constant compensation.
	   return ( z + z_offset);
       } else if (type_of_compensation == DECAY) {
	   t1 += t;
	   z1 += z;
	   n_avg++;
	   /* Averaged long enough ? */
	   if( t > (TIgnore+TAvg) ) {
	       t1 /= n_avg;
	       z1 /= n_avg;
	       current_state = CALC_MODEL_POINT2;
	       n_avg = 0;
	   }  /* end averaged long enough, gone WAIT for 2nd */
       }  /* end microscope stable */
       return def_z;
       
       
       /* Wait for samples to compute 2nd data point.  When we get them,
       ** go ahead and compute compensation coefficients.
       **/
   case CALC_MODEL_POINT2:
       // We ignore points until we get through the separation interval
       // Are we still in the separation interval? 
       // We expect the final point we get from averaging to be 1/2 way
       // through the averaging time, so subtract that from separation time
       if( t < (t1 + TSep - TAvg*0.5) ) {
	   return def_z;
       }
       // Start averaging points
       t2 += t;
       z2 += z;
       n_avg++;
       /* Have we averaged enough of them? */
       if( t >= (t1+TSep+TAvg*0.5) ) {
	   t2 /= n_avg;
	   z2 /= n_avg;
	   current_state = DECAY_COMP;

	   /* okay, we have two points as a function of time, so
	   ** we can calculate coefficients to z_offset - k/t eqn.
	   **/
	   decay_const = ( z1 - z2 )/( 1.0/t1 - 1.0/t2 );

	   // Calc how long before we switch to a constant offset. 
	   TMax = fabs(decay_const/REAL_SMALL);
	   
	   z_offset = def_z - (z1 - decay_const/t1);
	   
	   /* go ahead an compensate this one */
	   return (z + z_offset - decay_const/t);
       }  /* end averaged enough, gone READY */
       
       return def_z;
       
       /* Just add the computed offset to this z value
       **/
   case CONST_COMP:
       return(z + z_offset);
       
       /* Have equation, just do compensation.  Switch over to CONST_COMP only
       ** when decay_const/t gets insignificant.
       **/
   case  DECAY_COMP:
       if( (t > TMax)||(t < 0.0) ) {
	   // Why would t ever be less than 0?
	   current_state = CONST_COMP;
	   decay_const = 0.0;
       }
       return( z + z_offset - decay_const/t);
       
       /* anything else, try to stop system from calling us anymore, and
       ** set to known state.
       **/
   default:
       current_state = DISABLED;
       return z;
       
   } /* end switch */
   
} // end fix_height

int nmm_RelaxComp::stop_fix ()
{
    // Relaxation is disabled. Do nothing.
    if (current_state == DISABLED) return 0;
    // Otherwise, go into the IDLE state, where we don't fix heights. 
    else current_state = IDLE;

    return 0;
}


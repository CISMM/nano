#ifndef _TIMER_
#define _TIMER_

#include	<vrpn_Shared.h>
#include	<sys/types.h>

#include <stdio.h>

class Timer {
  public:
	inline Timer() {
	  initialized = 0; enabled = 1; 
	  max_framelog = 0; frame_time = new long int[1];};
	inline ~Timer() { delete frame_time; }
	inline void	enable(void) {enabled = 1;};
	inline void	disable(void) {enabled = 0; initialized = 0;};
	inline void	read(char *msg = "") {
	  struct timeval	now;
	  struct timezone	zone;

	  if (!enabled) {
	    return;
	  }
	  gettimeofday(&now, &zone);
	  if (!initialized) {
	    initialized = 1;
	    fprintf(stderr,"      Timer (%s): Initializing\n",msg);
	  } else {
	    long int duration = (now.tv_usec - last_time.tv_usec) +
	      1000000L * (now.tv_sec - last_time.tv_sec);
	    fprintf(stderr, "      Timer (%s): %ld microseconds since last\n",
		    msg, duration);
	  }
	  last_time = now;
	};
	inline void startlog(int max) {
	  initialized = 0; enabled = 1;  max_framelog = max; 
	  delete frame_time; frame_time = new long int[max_framelog];
	};
	inline void     log() {
	  struct	timeval	now;
	  struct	timezone	zone;
	  if (!enabled) {
	    return;
	  }
	  gettimeofday(&now, &zone);
	  if (!initialized) {
	    initialized = 1;
	  } else {
	    // do not record the first time after init. since it is time
	    // set up
	    if (framelog == 0)  framelog++;      
	    else if ( (framelog>0) && (framelog<max_framelog) ) {
	      frame_time[framelog-1] = (now.tv_usec - last_time.tv_usec)
		+  1000000L * (now.tv_sec - last_time.tv_sec);
	      framelog++;
	    }
	  }
	  last_time = now;
	}
        inline void print() {
	  int i = 0;
	  double ave_time = 0.0;
	  framelog--;
	  while(i<framelog) {
	    if (i%2 == 0)
	      printf(" Time for Frame %3i: %ld msec.      ", i, frame_time[i]);
	    else 
	      printf("     Time for Frame %3i: %ld msec.\n", i, frame_time[i]);

	    ave_time += frame_time[i];
	    i++;
	  }
	  if (i%2 == 0)   printf("\n");
	  printf("\nThe average for %i frames: %f msecs.\n", 
		 framelog, ave_time/framelog); 
	}
  
  protected:
	int	 initialized;
	int	 enabled;
	struct timeval	last_time;
	long int *frame_time;
	int      framelog; 
	int      max_framelog;
};

extern Timer mytimer;
extern int   timer_verbosity; ///< How much information about timer to print
//extern int   time_frame;      // How much frames to time
extern int   framelog;        ///< Flag to see if frame time log is on

#define TIMERVERBOSE(level, timer, msg) if (timer_verbosity>=level) timer.read(msg);
#define TIMERLOG(timer) if (framelog != 0) timer.log();

#endif

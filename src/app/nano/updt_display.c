
#include	<stdlib.h>
#include	<stdio.h>
#include	<sys/time.h>
//#include	"microscape.h"
#include        "updt_display.h"

/* externs */
//extern	int	new_epoch;	/* animate.c */

/**

generic function to update display


dependence on globals, and other creative hacking, removed 19 Aug 97 by TCH
******************************************************************************/
vrpn_bool updt_display (const long _displayPeriod, struct timeval & _dTime,
                        vrpn_bool & _stmNewFrame) {
  struct timeval	now;
  struct timezone	t_zone;

  gettimeofday(&now, &t_zone);

  if( 
	( ( (now.tv_sec-_dTime.tv_sec)*1000000L
	+ now.tv_usec-_dTime.tv_usec) < _displayPeriod ) 
	) return VRPN_FALSE;

    _dTime.tv_sec = now.tv_sec;
    _dTime.tv_usec = now.tv_usec;

    return (_stmNewFrame = VRPN_TRUE);
}


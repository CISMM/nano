#ifndef _UPDT_DISPLAY_
#define _UPDT_DISPLAY_

#include <v.h>  // for v_index
#include <nmb_Types.h>  // for vrpn_bool

extern int pg_updt_display(v_index *);
extern /*int*/ vrpn_bool updt_display (const long, struct timeval &, vrpn_bool &);

#endif

#ifndef NMB_TIME_H
#define NMB_TIME_H

#if !defined(_WIN32) || defined(__CYGWIN__)
#include <sys/time.h>
#else
#include <vrpn_Shared.h>  // get timeval some other way
#endif

int time_compare (const struct timeval &, const struct timeval &);
int time_subtract (const struct timeval &, const struct timeval &,
                   struct timeval *);
void time_multiply (const struct timeval &, double, struct timeval *);
void time_add (const struct timeval &, const struct timeval &,
               struct timeval *);

#endif  // NMB_TIME_H

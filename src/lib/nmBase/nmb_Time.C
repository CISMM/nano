#include "nmb_Time.h"



/****************
 *      This routine returns -1 if t1 < t2, 0 if t1 = t2, 1 if t1 > t2
 ****************/

int time_compare (const struct timeval & t1, const struct timeval & t2)
{
        if (t1.tv_sec < t2.tv_sec) return(-1);
        if (t1.tv_sec > t2.tv_sec) return(1);

        /* If it gets here, seconds are equal */
        if (t1.tv_usec < t2.tv_usec) return(-1);
        if (t1.tv_usec > t2.tv_usec) return(1);

        /* If it gets here, everything is equal */
        return(0);
}

/****************
 *      This routine subtracts the times t1 and t2 and puts the result into
 * result.  It returns 0 if things go okay.  It returns 1 if t2 > t1.
 ****************/

int  time_subtract (const struct timeval & t1, const struct timeval & t2,
                    struct timeval * res)
{
        /* Error if t2 > t1 */
        if (time_compare(t1,t2) == -1) return(1);

        res->tv_sec = t1.tv_sec - t2.tv_sec;
        if (t1.tv_usec >= t2.tv_usec) {
                res->tv_usec = t1.tv_usec - t2.tv_usec;
        } else {
                res->tv_usec = (t1.tv_usec + 1000000l) - t2.tv_usec;
                res->tv_sec--;
        }

        return(0);
}

/****************
 *	This routine multiplies the time by the scale factor and returns
 * the result.
 ****************/

void time_multiply (const struct timeval & t1, double scale,
                    struct timeval * res)
{
        res->tv_sec = (int)(t1.tv_sec * scale);
        res->tv_usec = (int)(t1.tv_usec * scale);
	while (res->tv_usec > 1000000l) {
		res->tv_sec++;
		res->tv_usec -= 1000000l;
	}
}

/****************
 *      This routine adds the times t1 and t2 and puts the result into
 * result.
 ****************/

void	time_add (const struct timeval & t1, const struct timeval & t2,
                  struct timeval * res)
{
        res->tv_usec = t1.tv_usec + t2.tv_usec;
        res->tv_sec = t1.tv_sec + t2.tv_sec;
        if (res->tv_usec >= 1000000l) {
                res->tv_sec++;
                res->tv_usec -= 1000000l;
        }
}


/** \file termio.c 
    raw terminal i/o routines
    
    
    Overview:
    	- this file has all routines for getting raw terminal input so
	    that user code can check for keyboard input without waiting
    
    Notes:
    	- I doubt that this is real portable.  In particular, I doubt 
	this works on System V-ish machines.


    Revision History:

    Author	    	Date	  Comments
    ------	    	--------  ----------------------------
    Rich Holloway	06/26/91  Initial version


   Developed at the University of North Carolina at Chapel Hill, supported
   by the following grants/contracts:
   
     DARPA #DAEA18-90-C-0044
     ONR #N00014-86-K-0680
     NIH #5-R24-RR-02170
   
 *
 *****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#if !defined (_WIN32) || defined (__CYGWIN__)
#include <unistd.h>
#include <sys/file.h>	/* for O_RDWR in open_raw_term()	    	*/
#endif
#include <fcntl.h>
#include <errno.h>  	/* for errno on ioctl call  */

#if defined(sparc) || defined(linux)
#include <sgtty.h>  	/* for sgttyb */
#endif
#include "termio.h"

#if defined(sparc) || defined(linux)
long	getAvaBytes(int ttyFD);
#endif

/*****************************************************************************
 *
   open_raw_term - open terminal device for unbuffered i/o
 
    input:
    	- boolean flag indicating whether to enable echo
    
    output:
    	- file descriptor for terminal
	- -1 on error
 *
 *****************************************************************************/

int
open_raw_term(int echoEnabled)
{
	int	    	    ttyFD;
#if defined(sparc) || defined(linux)
	static char	    termName[] = "/dev/tty";

	/* open /dev	*/
	if ( (ttyFD = open(termName, O_RDWR)) < 0 )
	    {
	    fprintf(stderr, "vlib: Unable to open '%s'.\n", termName);
	    return(-1);
	    }
#else
	ttyFD = 0;
#endif
	set_raw_term(ttyFD, echoEnabled);
	return(ttyFD);
}	/* open_raw_term */



/*****************************************************************************
 *
   close_raw_term - close raw terminal
 
    input:
    	- terminal's file descriptor
    
    output:
    	- returns what close(2) returns
 *
 *****************************************************************************/

int
close_raw_term(int ttyFD)
{
	/* undo all of the 'raw' stuff	*/
	reset_raw_term(ttyFD);

#if (defined(sparc)||defined(linux))
	return( close(ttyFD) );
#else
	return(0);
#endif

}	/* close_raw_term */



/*****************************************************************************
 *
   read_raw_term - read any keyboard input and return number of chars read
 
    input:
    	- tty file descriptor
	- buffer to read into
    
    output:
    	- number of bytes read
    
    notes:
    	- assumes buffer is at least 80 bytes long
 *
 *****************************************************************************/

int
read_raw_term(int ttyFD, char buffer[])
{
#if (defined(sparc)||defined(linux))
	int	    bytesRead;


	/* is there anything to read at the keyboard?	*/
	if ( getAvaBytes(ttyFD) == 0 )
	    return(0);

	if ( (bytesRead = read(ttyFD, buffer, 80-1)) < 1 )
	    /* if nothing read, quit	*/
	    return(bytesRead);

	/* null terminate input string	*/
	buffer[bytesRead] = '\0';

	return(bytesRead);
#else
	int	key;
	ttyFD = ttyFD;	/* Keep the compiler happy */

	key = getc(stdin);
	if (key == -1) {
		return(0);
	} else {
		buffer[0] = key;
		return(1);
	}
#endif
}	/* read_raw_term */


/*****************************************************************************
 *
   set_raw_term - put term in cbreak (half-cooked) mode & turn echo on/off
 
    input:
    	- terminal's file descriptor
    
    output:
    	- returns 0 on success, -1 otherwise    	
 *
 *****************************************************************************/

int set_raw_term(int ttyFD, int) // don't use echoEnabled parameter 
{
#if (defined(sparc))
    struct sgttyb   sttyArgs;


    /* get current settings */
    if ( ioctl(ttyFD, TIOCGETP, &sttyArgs) == -1 )
    {
	fprintf(stderr, "vlib: ioctl TIOCGETP on /dev/tty failed.\n");
	return(-1);
    }

    /* set new values	*/

    /* set term into cbreak (half-cooked) mode  */
    sttyArgs.sg_flags |= CBREAK;

//XXX We may need this RMT    if ( ! echoEnabled )
	/* turn off echo	*/
//	sttyArgs.sg_flags &= ~ECHO;

    /* install new values	*/
    if ( ioctl(ttyFD, TIOCSETP, &sttyArgs) == -1 )
    {
	fprintf(stderr, "vlib: ioctl TIOCSETP on /dev/tty failed.\n");
	return(-1);
    }

#else
    ttyFD = ttyFD;	/* Keep the compiler happy */
    system("/bin/stty raw opost onlcr -echo");
    fcntl(0,F_SETFL, FNDELAY );
#endif

    return(0);

}	/* set_raw_term */


/*****************************************************************************
 *
   reset_raw_term - turn on ech for this terminal
 
    input:
    	- tty file descriptor
    
    output:
    	- terminal is set back to normal
	- returns 0 on success, -1 otherwise
 *
 *****************************************************************************/

int
reset_raw_term(int ttyFD)
{
#if (defined(sparc))
    struct sgttyb   sttyArgs;


/* get current settings */
if ( ioctl(ttyFD, TIOCGETP, &sttyArgs) == -1 )
    {
    fprintf(stderr, "vlib: ioctl TIOCGETP on /dev/tty failed.\n");
    return(-1);
    }

/* set new values	*/
/* turn off cbreak mode	*/
sttyArgs.sg_flags &= ~CBREAK;

/* make sure echo is on	*/
sttyArgs.sg_flags |= ECHO;

/* install new values	*/
if ( ioctl(ttyFD, TIOCSETP, &sttyArgs) == -1 )
    {
    fprintf(stderr, "vlib: ioctl TIOCSETP on /dev/tty failed.\n");
    return(-1);
    }

#else
	ttyFD = ttyFD;	/* Keep the compiler happy */
	system("/bin/stty cooked echo");
	fcntl(0,F_SETFL,0);
#endif

return(0);

}	/* reset_raw_term */



/*****************************************************************************
 *
   getAvaBytes - returns number of bytes available for reading
 
    input:
    	- tracker table pointer
    
    output:
    	- number of bytes available for reading, -1 < 0 on error
 *
 *****************************************************************************/
#if defined(sparc) || defined(linux)

long
getAvaBytes(int ttyFD)
{
    long    charsToRead;

    if ( ioctl(ttyFD, FIONREAD, &charsToRead) < 0 )
    {
	fprintf(stderr,"vlib: ioctl error on FIONREAD, errno = %d\n", errno);
	return(-1);
    }

    /* otherwise, return number of bytes available to read	*/
    return(charsToRead);

}	/* getAvaBytes */

#endif  /* sparc || linux */





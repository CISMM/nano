/** /file filter.C

        This file describes the functions that are needed to derive
 a new data set from an old one by passing it through one of the unix filter
 programs (scripts) found in ~stm/etc/filters.  It creates a new plane in the
 inputGrid if and only if the filter succeeds.
        XXX This should be changed into a class, perhaps.  In any case, the
 function should return as soon as the remote command has started, and there
 should be a polling command in the main loop that will handle reading in the
 data set later.  This will allow another processor on the SGI to go off and
 do the filtering while the program goes on.  It would also allow multiple
 filters to run at once.
*/

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef __CYGWIN__
#include <sys/types.h>	// pipe()
#include <signal.h>	// kill()
#endif

#if (!defined(_WIN32) && !defined(__CYGWIN__))
#include <unistd.h>	// fork()
#include <sys/wait.h>	// wait()
#endif

/*
// This is a horrible hack.  It should be fixed.  (WTL-20/Apr/1999)
#ifdef __CYGWIN__
#ifndef _GNU_H_WINDOWS32_SOCKETS
typedef u_int SOCKET;
// juliano 9/18/99
//   had to add this to compile on my PC under cygwin
//   looks like implicit declarations are not allowed in gcc-2.95
//     (I thought they were supposed to be by default, but guess not)
//   tanner removed the include of "sdi.h" because of classes, but it
//   looks like that causes other problems if implicit decl not allowed
int sdi_noint_block_write( SOCKET, char[], int );
#endif
#else
#ifndef _WIN32
#include "sdi.h"
#endif
#endif  // not __CYGWIN__
*/

// instead of the above using sdi we have this:
//#include "vrpn_Connection.h"

#include "BCGrid.h"
#include "BCPlane.h"

#include "filter.h"

//#define	DEBUG

// eliminate "unused function" warning in cygwin.
#if (!defined(_WIN32) || defined(__CYGWIN))

/** Write the data set name, units, size and X and Y dimension, and width and
 length for the 
 plane that is passed in, into the file descriptor that is passed in.
 @return 0 on success, -1 on failure.
*/
static	int	filter_write(const char *newset_name, BCPlane *plane, int fd)
{
	char	header[500];
	float	*xlist = NULL;
	int	xsize = 0;
	int	x,y;

	
	// Form the header and write it to the file
	sprintf(header,"%s\n%s\n%d\n%d\n%f\n%f\n",
		newset_name,
		plane->units()->c_str(),
		plane->numX(), plane->numY(), //number of grid points in x and y
		plane->maxX() - plane->minX(), //actual size (in nm) in x
		plane->maxY() - plane->minY()); //actual size in y

#if (!defined(_WIN32) || defined(__CYGWIN))
	if (vrpn_noint_block_write(fd,header,strlen(header)) != strlen(header)) {
		perror("filter_write(): Can't write header");
		return -1;
	}
#endif

	fprintf(stderr,"filter_write(): Wrote numX=%d, numY=%d, sizeX=%f, sizeY=%f\n", plane->numX(), plane->numY(), plane->maxX()-plane->minX(), plane->maxY()-plane->minY());

#ifdef	DEBUG
	fprintf(stderr,"filter_write(): Wrote header\n");
#endif

	// Write the data values to the file, one line at a time
	xsize = sizeof(float) * plane->numX();
	xlist = new float[plane->numX()];
	if (xlist == NULL) {
		fprintf(stderr,"filter_write(): Out of memory\n");
		delete [] xlist;
		return -1;
	}
	for (y = 0; y < plane->numY(); y++) {
		for (x = 0; x < plane->numX(); x++) {
			xlist[x] = plane->value(x,y);
		}

#if (!defined(_WIN32) || defined(__CYGWIN))
		if (vrpn_noint_block_write(fd, (char*)xlist, xsize) != xsize) {
			perror("filter_write(): Can't write line");
			delete [] xlist;
			return -1;
		}
#endif

	}

#ifdef	DEBUG
	fprintf(stderr,"filter_write(): Wrote body\n");
#endif

	delete [] xlist;
	return 0;
}

/** Read the data set name, units, size and values for the plane whose name is
read in from the file descriptor that is passed in.
Make a plane with the new values in inputGrid if all goes well.
@return 0 on success, -1 on failure.
*/
static	int	filter_read (int fd, BCGrid * grid)
{
	FILE	*infile;		// stream to attach to descriptor
	char	planeName[100];		// Name read from file
	char	planeUnits[100];	// Units read from file
	char	lenString[100];		// String representation of length
	int	planeX, planeY;		// Size read from file
	float	*datalist;
	int	datacount;
	BCPlane *plane;			// New plane
	int	x,y;

	// Get a file handle so we can use formatted reads
	if ( (infile = fdopen(fd, "r")) == NULL) {
		perror("filter_read(): can't reopen file for read");
		return -1;
	}

	// Read the header information from the file (name, units, size)
	// Each time, remove the newline character from the string
	if (fgets(planeName, sizeof(planeName)-1, infile) == NULL) {
		perror("filter_read(): can't read name");
		return -1;
	} else {
		planeName[strlen(planeName)-1] = '\0';
	}
	if (fgets(planeUnits, sizeof(planeUnits)-1, infile) == NULL) {
		perror("filter_read(): can't read units");
		return -1;
	} else {
		planeUnits[strlen(planeUnits)-1] = '\0';
	}
	if (fgets(lenString, sizeof(lenString)-1, infile) == NULL) {
		perror("filter_read(): can't read sizes");
		return -1;
	}
	if (sscanf(lenString, "%d%d", &planeX, &planeY) != 2) {
		fprintf(stderr,"filter_read(): can't parse sizes\n");
		return -1;
	}

fprintf(stderr,"filter_read(): read %s, %s, numX=%d, numY=%d\n", planeName, planeUnits, planeX, planeY);
#ifdef	DEBUG
	fprintf(stderr,"filter_read(): Read header\n");
#endif

	// Read the data values from the file in a huge batch.
	datacount = planeX * planeY;
	datalist = new float [planeX * planeY];
	if (datalist == NULL) {
		fprintf(stderr,"filter_read(): Out of memory\n");
		//delete [] datalist;  //it's already null...
		return -1;
	}
	if (fread(datalist, sizeof(float), datacount, infile) != (unsigned)datacount) {
		perror("filter_read(): Can't read data");
		delete [] datalist;
		return -1;
	}

#ifdef	DEBUG
	fprintf(stderr,"filter_read(): Read body\n");
#endif

	// Make sure that the plane has the right number of entries.
	if ( (planeX != grid->numX()) || (planeY != grid->numY()) ) {
		fprintf(stderr,"filter_read(): Bad grid size\n");
		delete [] datalist;
		return -1;
	}

	// Create a new plane for the data if there is not already
	// one with that name and put the data in there.
	if ( (plane = grid->getPlaneByName(planeName)) == NULL) {
	  if ( (plane = grid->addNewPlane(planeName, planeUnits,
	     NOT_TIMED)) == NULL) {
		fprintf(stderr,"filter_read(): Can't make new plane\n");
		delete [] datalist;
		return -1;
	  }
	}
	for (y = 0; y < planeY; y++) { for (x = 0; x < planeX; x++) {
		plane->setValue(x,y, datalist[x + y*planeX]);
	} }
	plane->setMinAttainableValue(plane->minValue());
	plane->setMaxAttainableValue(plane->maxValue());
	plane->setScale(1.0);

	delete [] datalist;
	return 0;
}
#endif // cygwin unused function fix.

/**
 Run the given plane through the given filter with the given arguments.
 If all goes well, put the dataset into a plane with the given name.
 @return 0 on success and -1 on failure.
*/
int     filter_plane(const char * filtername,	///< Full path name
                     BCPlane * oldset,		///< Plane to filter
                     const char * newset_name,	///< Where to put it
                     double scale,		///< Scale of the filter
                     double angle,		///< Angle of filter
                     const char * otherargs,	///< Other arguments (or "")
                     BCGrid * grid)		///< Grid to put it in
{

#if (!defined(_WIN32) || defined(__CYGWIN))
  // Don't have the same pipe support in NT, or if we do
  // can't find the right magic header file.

	int	to_child[2], from_child[2];
	pid_t	childpid;

float sizeX, sizeY;  //actual size in nm in x and y

sizeX = (grid->maxX() - grid->minX());
sizeY = (grid->maxY() - grid->minY());

	// Make two pipes - one to talk in each direction
	if (pipe(to_child)) {
		perror("filter_plane(): pipe() failed");
		return -1;
	}
	if (pipe(from_child)) {
		perror("filter_plane(): pipe() failed");
		return -1;
	}

	// Fork!
	childpid = fork();
	if (childpid == -1) {
		perror("filter_plane(): fork() failed");
		return -1;
	}

	if (childpid == 0) {	// Child
		int	ret;
		char	command[5000];

		// Redirect stdin/stdout through the pipe
		if(dup2(to_child[0], 0) < 0) {	// Stdin
		   perror("filter_plane(): dup2() failed");
		}
		if (dup2(from_child[1], 1) < 0) {	// Stdout
		   perror("filter_plane(): dup2() failed");
		}
		close(to_child[1]);	// Parent owns
		close(from_child[0]);	// Parent owns

		// Call the program.
		sprintf(command,"%s %f %f %f %f %s",
			filtername, scale, angle, sizeX, sizeY, otherargs);
		//fprintf(stderr, "Filter command:  %s\n", command);
		ret = system(command);
		if (ret < 0) {
			perror("child of filter_plane(): system() failed");
		}

		// Close the remaining pipes and wait for the parent to kill
		// us with SIGKILL.  If we exit here, that calls the
		// destructors for all of the tcl variables, which blows up
		// the parent.
		close(0);	// They were mapped to stdin and stdout
		close(1);
		wait(0);

		// wait() isn't supposed to make this process wait (unless
		// it still has a child process around from the system() call)
		// What we want to do is wait until the parent kills us.
		// 1000 seconds seems like a reasonable time to wait.
		// If we don't sleep here, it's possible that we'll leave
		// this function, go into Tcl, and get killed while something
		// isn't kosher with Tcl, throwing the whole program (and the
		// X server) into chaos.

	//	fprintf(stderr, "Child past wait.\n");
                sleep(1000);
		
	} else {		// Parent
		// Close the pipe parts that aren't ours
		close(to_child[0]);	// Child owns
		close(from_child[1]);	// Child owns

		// Write the data set to the pipe
		if (filter_write(newset_name, oldset, to_child[1])) {
			fprintf(stderr,"filter_plane(): Can't write dataset\n");
			close(to_child[1]);
			goto DONE;
		}

		// Close the pipe that writes to the child.
		// This should help flush everything through the
		// system if there is a job waiting on EOF.
		close(to_child[1]);

		// Read the dataset and build a plane if there was no error
		if (filter_read(from_child[0], grid)) {
			fprintf(stderr,"filter_plane(): Can't make plane\n");
			goto DONE;
		}

	DONE:
		// Make sure the child has terminated
		close(from_child[0]);
		kill(childpid, SIGKILL);
		wait(0);

		return 0;
	}

#endif

	return 0;
}

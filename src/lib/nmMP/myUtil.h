/****************************************************************************\

  Copyright 1999 The University of North Carolina at Chapel Hill.
  All Rights Reserved.

  Permission to use, copy, modify and distribute this software and its
  documentation for educational, research and non-profit purposes,
  without fee, and without a written agreement is hereby granted,
  provided that the above copyright notice and the following three
  paragraphs appear in all copies.

  IN NO EVENT SHALL THE UNIVERSITY OF NORTH CAROLINA AT CHAPEL HILL BE
  LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
  CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF THE
  USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY
  OF NORTH CAROLINA HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH
  DAMAGES.

  THE UNIVERSITY OF NORTH CAROLINA SPECIFICALLY DISCLAIM ANY
  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE
  PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND THE UNIVERSITY OF
  NORTH CAROLINA HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT,
  UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

  The author may be contacted via:

  US Mail:             Hans Weber
                       Department of Computer Science
                       Sitterson Hall, CB #3175
                       University of N. Carolina
                       Chapel Hill, NC 27599-3175

  Phone:               (919) 962 - 1928

  EMail:               weberh@cs.unc.edu

\****************************************************************************/

/*****************************************************************************\
  myUtil.h
  --
  Description : A collection of common utilties macros and typedefs
                This library also sets up a new new_handler for when
		dynamic allocation fails.
		
		It also helps to define some pc equivalents for unix calls
		so that code can be compiled on both (ie, unistd.h,
		gettimeofday, sockets, and sleep). NOTE: pc version of
		gettimeofday has ms, not us granularity.

		Debugging is turned off by defining NDEBUG, while some
		extra debugging might be turned on by defining _DEBUG.
		These both conform with the MSVC++ compiler defaults.
		
		This file also includes windows.h (needed for glut.h
		GL/gl.h).

		This file should be free of my classes so that there are
		no include difficulties.

		If this code is being linked with vrpn, then you need
		to define USE_VRPN_CLOCK when compiling myUtil.cpp if
		you want to have all of the code use the same local
		clock time frame (and avoid multiply defined function
		complaint for gettimeofday).

  ----------------------------------------------------------------------------
  Author: weberh
  Created: Tue Sep  9 02:20:27 1997
  Revised: Thu May 14 10:10:08 1998 by weberh
  $Source$
  $Locker$
  $Revision$
\*****************************************************************************/
#ifndef _MYUTIL_H_
#define _MYUTIL_H_

#include <iostream.h>
#include <iomanip.h>
#include <fstream.h>
#include <new.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
// for mem debugging
// To use it, you have to :
// 0) define MEM_DBG_THIS_FILE in the file you want debugged
// 1) try to have everything be deleted properly
// 1) set _CrtSetDbgFlag as in testViewer
// 2) recompile and run

#ifdef MEM_DBG_THIS_FILE
#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
// the second macro is to prevent problems with functions being declared
// multiple times when malloc.h is included (ie, by flex/bison)
// alloca is the only call they need
#define _INC_MALLOC
#define alloca _alloca
extern "C" { void *          __cdecl _alloca(size_t);}
#endif
#endif
#endif

/*-----------------------------*/
/*****  Constants/#defines *****/
/*-----------------------------*/

// for ease in writing print statements
#define NL << "\n"
#define TAB << "\t"

#ifdef _WIN32
#define IOS_BINARY ios::binary
#else
#define IOS_BINARY 0x0
#endif


#ifdef R
#undef R
#endif
#ifdef G
#undef G
#endif
#ifdef B
#undef B
#endif
#ifdef A
#undef A
#endif

enum { R=0, G=1, B=2, A=3 };
// get rid of annoying windows macros -- i use min/max as member functions
#ifdef _WIN32
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif
#endif

/*-----------------------------*/
/*****       Macros        *****/
/*-----------------------------*/
// for those that need to be tested at run time regardless of debugging status
// do_exit is defined to allow user to set breakpoint there to find failure
// and have stack trace
#define ALL_ASSERT(exp, msg) if(!(exp)){ cerr NL << "Assertion failed! " << \
endl << msg << " (" << __FILE__ << ", " << __LINE__ << ")" NL; do_exit(-1);}

// if you define NDEBUG, then regular asserts are removed
#ifndef NDEBUG
#define ASSERT(exp, msg) ALL_ASSERT(exp, msg)
#else
#define ASSERT(exp, msg) ((void) 0)
#endif

#define DEPS 1e-6
// a delta-epsilon equals
#define DEPS_EQ(a, b) ( ((a) > (b)) ? ((a)-(b) < DEPS) : ((b)-(a) < DEPS) )

/*-----------------------------*/
/*****      Typedefs       *****/
/*-----------------------------*/
#ifdef FALSE
#undef FALSE
#endif
#ifdef TRUE
#undef TRUE
#endif
enum { FALSE=0, TRUE=1 };
// want bool to be an int so we can
// return comparisons as bools
typedef int Boolean;

/*-----------------------------*/
/***** Function Prototypes *****/
/*-----------------------------*/
// all assertions go thru this, so you can set a breakpoint here
// when an assertion happens and then look at the stack
void do_exit( int iExitCode );

/************************************************************
  Config Info Utilties
************************************************************/
// returns -1 on failure, otherwise# of processors on current machine
int numProcessors();

/************************************************************
  Endianness utilities
************************************************************/

enum Endianness { UNDEF, LITTLE_END, BIG_END };

#undef ENDIANNESS
#if defined(_WIN32) | defined(linux)
#define ENDIANNESS LITTLE_END
#endif
#ifdef hpux
#define ENDIANNESS BIG_END
#endif
#ifdef sparc
#define ENDIANNESS BIG_END
#endif
#ifdef sgi
#define ENDIANNESS BIG_END
#endif

// do proper conversion of the arg with eIn endianness to current endianness
unsigned short usEndian( unsigned short us, Endianness eIn );
unsigned long  ulEndian( unsigned long ul, Endianness eIn );
float fEndian( float f, Endianness eIn );
double dEndian( double d, Endianness eIn );

/************************************************************
  Binary I/O utilities
************************************************************/

/// These are binary writing functions and reading functions
/// Use them in class reads and writes. 
/// NEED to open the file for read or write with IOS_BINARY in mode.
/// To write a ptr, just call read/write -- template will do it
/// When reading an writing ptrs, you will need to keep
/// track of some base from which they have meaning (ie,
/// write that base out, and adjust the ptrs appropriately
/// after they are read back in).
/// To read a ptr, you must specifically define a read for 
/// that ptr type (casts to another type won't work -- casts
/// create a copy of the var, so you have a ref to a a copy,
/// which is an rvalue (does not persist), and you won't get the
/// data that is read into it).(that is why we have a template func)

// for read/write of all types of ptrs
// (since it is a template function, it needs to be declared
//  here so that it is included in the appropriate files)
template <class Type> 
ostream& write( ostream& os, Type * const & pt ) {
  return os.write( (char *)&pt, sizeof(pt) );
}
template <class Type> 
istream& read( istream& is, Type *& pt ) {
  return is.read( (char *)&pt, sizeof(pt) );
}

ostream& write( ostream& os, const char& ch );
ostream& write( ostream& os, const unsigned char& uch );
ostream& write( ostream& os, const int& i );
ostream& write( ostream& os, const unsigned int& ui );
ostream& write( ostream& os, const double& d );
ostream& write( ostream& os, const float& f );

istream& read( istream& is, char& ch );
istream& read( istream& is, unsigned char& uch );
istream& read( istream& is, int& i );
istream& read( istream& is, unsigned int& ui );
istream& read( istream& is, double& d );
istream& read( istream& is, float& f );

/************************************************************
  GL debugging utilities
************************************************************/

// routines for glDebugging
extern Boolean gGlDebug;

// turn on/off debugging printouts of gl calls
// all gl calls need to be compiled with -DGL_DEBUG,
// and then you have to call glDebugOn(); (def is off)

void glDebugOn();
void glDebugOff();

/************************************************************
  Time and Timeval utilities
************************************************************/

// these were grabbed from vrpn_Shared.h
// for sockets and gettimeofday

#if defined(_WIN32)
#include <sys/timeb.h>
#include <winsock.h>   // timeval is defined here
  /* from HP-UX */
#if !defined(__CYGWIN__)
#ifndef _STRUCT_TIMEVAL
#define _STRUCT_TIMEVAL
struct timezone {
    int     tz_minuteswest; /* minutes west of Greenwich */
    int     tz_dsttime;     /* type of dst correction */
};
#endif
// name set up so that i have to explicitly call my_gettimeofday
// to use mine -- otherwise i use vrpn's version (since i don't
// want to change vrpn code and i need all stamps in the same
// time frame).  Any calls to gettimeofday will use vrpn,
// any to my_gettimeofday will use my util lib version.
//#define my_gettimeofday win95_gettimeofday
extern int gettimeofday(struct timeval *tp, struct timezone *tzp);
// CYGWIN defines gettimeofday, but it uses C style linkage.
// If we define it here, the compiler thinks it has C++ style 
// linkage and can't find it later.
#endif // CYGWIN
// #define close closesocket
#else
// unix time headers
#include <sys/time.h>
//#define my_gettimeofday gettimeofday
#endif


// timeval calculation routines
struct timeval operator+( const struct timeval& tv1, 
			  const struct timeval& tv2 );
struct timeval operator-( const struct timeval& tv1, 
			  const struct timeval& tv2 );
extern struct timeval timevalSum( const struct timeval& tv1, 
				  const struct timeval& tv2 );
extern struct timeval timevalDiff( const struct timeval& tv1, 
				   const struct timeval& tv2 );
extern double timevalToMsecs( const struct timeval& tv1 );
extern struct timeval timevalFromMsecs( const double dMsecs );

// header needed for unix sleep
#ifdef _WIN32
#include <time.h>
#include <winbase.h>
// For some reason cygwin doesn't like this.
#ifndef __CYGWIN__
// VC++ compile problem
//unsigned int sleep( unsigned int cSeconds );
#endif
#endif

// short-time-period sleep
void sleepMsecs( double dMsecs );

/************************************************************
  Windows sockets utilities
************************************************************/

// TO USE SOCKETS for win and non-win apps, be sure to include
// an object of type UseSockets in (at least) one of your .cpp files.

class UseSockets {
public:
  UseSockets();
  ~UseSockets();
};

/************************************************************
 Unbuffered, nonblocking input
************************************************************/
// returns 0 if no char, 1 if char (and fills it in pch), -1 on error
// fEcho says whether or not to print the char when it is read 
int nonblockingGetch(char *, int fEcho=1);

#endif  //_UTIL_H_



/*****************************************************************************\
  $Log$
  Revision 1.1.1.1  1999/12/14 20:40:08  weigle
  This is the new directory structure.  More documentation will be forth
  coming, but there is a README.1ST which should get you compiling. 

  NOTE: This does not currently go make libraries for you if they can not
  be found, only complains that they are not there.  There are two places
  where you must do a gmake to get libraries:
     ./nano/src/lib  and  ./nano/src/app/nano/lib

  Revision 1.5  1999/05/28 21:44:22  lovelace
  One small change that was left out of the previous NT commit.  These
  files had been changed in the include directory, but not in the
  multip directory.

  Revision 1.3  1999/03/23 15:22:55  hudson
  Added a new control panel (for latency compensation techniques).
  Added (an early version of) the first compensation technique -
    showing both the Phantom tip and true tip positions.
  Migrated a few small pieces of data onto nmb_Decoration.

  Revision 1.2  1998/06/01 20:56:26  kumsu
  put in code to compile with aCC for pxfl

  Revision 1.1  1998/05/27 19:30:41  hudson
  * Added multip/Makefile
                 README
                 myUtil.h
                 util.C
                 threads.[Ch]
  * Wrote sharedGraphicsServer() and spawnSharedGraphics() in microscape.c

  This is Hans Weber's multithreading library, used for a shared-memory
  multiprocessing version of microscape.

  Revision 1.2  1998/05/01 05:10:39  weberh
  *** empty log message ***

  Revision 1.1  1998/04/23 04:14:33  weberh
  Initial revision

  Revision 4.1  1998/03/31 03:03:17  weberh
   pc threading compiles
  >

  Revision 4.0  1998/03/29 22:40:41  weberh
   sgi threading works. now on to pc threading.

  Revision 3.4  1998/03/25 23:06:26  weberh
  *** empty log message ***

  Revision 3.3  1998/03/16 16:04:15  weberh
  *** empty log message ***

  Revision 3.2  1998/03/15 22:18:38  weberh
  *** empty log message ***

  Revision 3.1  1998/03/05 15:18:48  weberh
  added scale func

  Revision 3.0  1998/02/25 20:13:18  weberh
  all in good shape for viewer

  Revision 1.19  1998/02/19 22:22:50  weberh
  *** empty log message ***

  Revision 1.18  1998/02/10 21:26:47  weberh
   got rid of all_assert

  Revision 1.17  1998/02/09 19:34:13  weberh
  *** empty log message ***

  Revision 1.16  1997/12/21 10:34:57  weberh
  better gettimeofday, etc. for pc

  Revision 1.15  1997/12/14 08:15:30  weberh
  timing calls added -- tvDiff, tvSum tvElapsedMsecs
  also windwos gettimeofday implemented.

  Revision 1.14  1997/12/11 06:38:08  weberh
  added pc sleep and gettimeofday

  Revision 1.13  1997/12/08 21:46:34  weberh
  *** empty log message ***

  Revision 1.12  1997/12/01 22:35:53  weberh
  *** empty log message ***

  Revision 1.11  1997/11/17 23:04:08  weberh
  added binary read/write

  Revision 1.10  1997/11/15 00:27:33  weberh
  lots of read and write.

  Revision 1.9  1997/11/12 18:10:06  weberh
  added binary write and read for basic types.

  Revision 1.8  1997/11/04 06:51:46  weberh
  utility routines for adjusting endianess

  Revision 1.7  1997/10/30 16:26:22  weberh
  added proper new handler call for windows

  Revision 1.6  1997/10/28 21:24:44  weberh
  compiles with proper enums

  Revision 1.5  1997/10/10 20:06:37  weberh
  *** empty log message ***

  Revision 1.4  1997/10/10 17:10:26  weberh
  fixed for windows

  Revision 1.3  1997/09/18 20:00:10  weberh
  fixed up assert msg

  Revision 1.2  1997/09/17 07:44:35  weberh
  *** empty log message ***

  Revision 1.1  1997/09/17 07:39:51  weberh
  Initial revision

  Revision 1.5  1997/09/16 20:28:20  weberh
  fixed up enum so true=1 is confirmed
  /

  Revision 1.4  1997/09/16 20:22:00  weberh
  added NL, SP, TAB, and new_handler

  Revision 1.3  1997/09/09 21:53:07  weberh
  small changes for windows

  Revision 1.2  1997/09/09 08:54:08  weberh
  got rid of assert all together -- just my own c++ macro now.

  Revision 1.1  1997/09/09 06:23:12  weberh
  Initial revision

\*****************************************************************************/


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

#if defined(_WIN32) && !defined(__CYGWIN__)
#include <sys/timeb.h>
#include <winsock.h>   // timeval is defined here
  /* from HP-UX */
//#if !defined(__CYGWIN__)
#ifndef _STRUCT_TIMEZONE
#define _STRUCT_TIMEZONE
struct timezone {
    int     tz_minuteswest; /* minutes west of Greenwich */
    int     tz_dsttime;     /* type of dst correction */
};
#endif
// Always use VRPN version
//extern int gettimeofday(struct timeval *tp, struct timezone *tzp);

// CYGWIN defines gettimeofday, but it uses C style linkage.
// If we define it here, the compiler thinks it has C++ style 
// linkage and can't find it later.
//#endif // CYGWIN
// #define close closesocket
#else
// unix time headers
#include <sys/time.h>

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

\*****************************************************************************/



#ifndef __MF__
#define __MF__

#include <stdio.h>

/** \file mf.h 
**    personal little nothing's - mf
**
** These provide inline documentation with optional printed traces
** in the code.  They progress in detail of debugging downward.
** That is, DOCUMENT_?s should be entry and exit level traces, which
** wouldn't severely inconvenience the user if left on, though they
** might be more confusing than helpful to anyone but the interested
** programmer.  DEBUG0_? are somewhat more detailed, and might annoy
** the user but not especially affect performance otherwise.  This
** progresses to DEBUG3_?'s which might be inside loops or functions
** which are called 1000 times per second, ie. which must be turned
** off for the program to run at a reasonable rate, which in interactive
** systems means for the program to function at all.  In other words,
** we progress downward from program documentation through the levels
** of debugging which demand more and more detail of internals to the
** code.  These don't affect performance unless turned on (good), but
** the code must be recompiled to change whether on or off (bad).
** They are best left in the code for future modification and testing,
** as well as to point out what's going on.
*/

#ifdef DOCUMENT
#define DOCUMENT0(s)		fprintf(stderr,s)
#define DOCUMENT1(s,a)		fprintf(stderr,s,a)
#define DOCUMENT2(s,a,b)	fprintf(stderr,s,a,b)
#define DOCUMENT3(s,a,b,c)	fprintf(stderr,s,a,b,c)
#else
#define DOCUMENT0(s)	
#define DOCUMENT1(s,a)
#define DOCUMENT2(s,a,b)
#define DOCUMENT3(s,a,b,c)
#endif /* DOCUMENT */

#ifdef DEBUG0_
#define DEBUG0_0(s)		fprintf(stderr,s)
#define DEBUG0_1(s,a)		fprintf(stderr,s,a)
#define DEBUG0_2(s,a,b)		fprintf(stderr,s,a,b)
#define DEBUG0_3(s,a,b,c)	fprintf(stderr,s,a,b,c)
#else
#define DEBUG0_0(s)	
#define DEBUG0_1(s,a)
#define DEBUG0_2(s,a,b)
#define DEBUG0_3(s,a,b,c)
#endif /* DEBUG0_ */

#ifdef DEBUG1_
#define DEBUG1_0(s)		fprintf(stderr,s)
#define DEBUG1_1(s,a)		fprintf(stderr,s,a)
#define DEBUG1_2(s,a,b)		fprintf(stderr,s,a,b)
#define DEBUG1_3(s,a,b,c)	fprintf(stderr,s,a,b,c)
#else
#define DEBUG1_0(s)	
#define DEBUG1_1(s,a)
#define DEBUG1_2(s,a,b)
#define DEBUG1_3(s,a,b,c)
#endif /* DEBUG1_ */

#ifdef DEBUG2_
#define DEBUG2_0(s)		fprintf(stderr,s)
#define DEBUG2_1(s,a)		fprintf(stderr,s,a)
#define DEBUG2_2(s,a,b)		fprintf(stderr,s,a,b)
#define DEBUG2_3(s,a,b,c)	fprintf(stderr,s,a,b,c)
#else
#define DEBUG2_0(s)	
#define DEBUG2_1(s,a)
#define DEBUG2_2(s,a,b)
#define DEBUG2_3(s,a,b,c)
#endif /* DEBUG2_ */

#ifdef DEBUG3_
#define DEBUG3_0(s)		fprintf(stderr,s)
#define DEBUG3_1(s,a)		fprintf(stderr,s,a)
#define DEBUG3_2(s,a,b)		fprintf(stderr,s,a,b)
#define DEBUG3_3(s,a,b,c)	fprintf(stderr,s,a,b,c)
#else
#define DEBUG3_0(s)	
#define DEBUG3_1(s,a)
#define DEBUG3_2(s,a,b)
#define DEBUG3_3(s,a,b,c)
#endif /* DEBUG3_ */

#endif

// nmb_Util
// 
// Tom Hudson, March 1998

//   Contains utility functions to buffer and unbuffer data for network
// transmission.

//   Each Buffer() routine takes a pointer to a character pointer for
// the insertion point of the buffer and a pointer to the end of the buffer.
// If the argument fits between the two, it is copied in and the insertion
// point is advanced appropriately.

//   Each Unbuffer() routine takes a pointer to a character pointer for
// the buffer to extract from.  After unbuffering into the output space
// specified in the parameters it advances the pointer into the buffer
// past the unbuffered data item.

#include <vrpn_Types.h>

struct nmb_Util {

  static int Buffer (char ** insertPt, long * buflen,
                     const long value);	// Tiger implemented with long type
  static int Buffer (char ** insertPt, int * buflen,
                     const int value);
  static int Buffer (char ** insertPt, long * buflen,
                     const float value);
  static int Buffer (char ** insertPt, int * buflen,
                     const float value);
  static int Buffer (char ** insertPt, long * buflen,
                     const double value);
  static int Buffer (char ** insertPt, int * buflen,
                     const double value);
  static int Buffer (char ** insertPt, long * buflen,
                     const char * string, unsigned long length);
  static int Buffer (char ** insertPt, int * buflen,
                     const char * string, unsigned int length);
  static int Buffer (char ** insertPt, long * buflen,
                     const struct timeval & time);
  static int Buffer (char ** insertPt, int * buflen,
                     const struct timeval & time);
  static int Buffer (char ** insertPt, long * buflen,
                     const vrpn_bool value);
  static int Buffer (char ** insertPt, int * buflen,
                     const vrpn_bool value);

  static int Unbuffer (const char ** buffer, long *);	// Tiger implemented with long type
  static int Unbuffer (const char ** buffer, int *);
  static int Unbuffer (const char ** buffer, float *);
  static int Unbuffer (const char ** buffer, double *);
  static int Unbuffer (const char ** buffer, char *,
                       unsigned long len);
  static int Unbuffer (const char ** buffer, struct timeval *);
  static int Unbuffer (const char ** buffer, vrpn_bool *);

};


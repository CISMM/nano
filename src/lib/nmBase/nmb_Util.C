#include "nmb_Util.h"
#include <stdio.h>
#include <string.h>  // for memcpy

#ifdef _WIN32
#include <winsock.h>    // only for VC++ compiler, it can replace 
                        //netinet/in.h and sys/time.h
#else
#include <netinet/in.h>  // for htonl()
#include <sys/time.h>  // for timeval

#endif

/* Tiger	added several similar Buffer and Unbuffer functions that
		takes long and long * arguments for compatibility with
		Windows NT. ( In NT, int and long are not the same length;
		but in UNIX, they are. So we have to differentiate them
		in NT version.
*/

#define CHECK(a) if ((a) == -1) return -1

//static
int nmb_Util::Buffer (char ** insertPt, int * buflen,
                      const int value) {
/* NANO BEGIN
  fprintf(stderr, "nmb_Util::Buffer(int)\n");
*/
  int netValue = htonl(value);
  int length = sizeof(netValue);

  CHECK(length < *buflen);  // XXX bug

  memcpy(*insertPt, &netValue, length);
  *insertPt += length;
  *buflen -= length;

  return 0;
}

//static
int nmb_Util::Buffer (char ** insertPt, long * buflen,
                      const long value) {	// Tiger implemented with long
  long netValue = htonl(value);
  long length = sizeof(netValue);
/* NANO BEGIN
  fprintf(stderr, "nmb_Util::Buffer(long): value = %lX\t sizeof(value) = %d\n", value, sizeof(value));
  fprintf(stderr, "\t\tnetValue = %lX\t sizeof(netValue) = %d\n", netValue, length);
  fprintf(stderr, "\t\tbuflen = %ld\n", *buflen);
*/

  CHECK(length < *buflen); // XXX bug

  memcpy(*insertPt, &netValue, length);
  *insertPt += length;
  *buflen -= length;

  return 0;
}

//static
int nmb_Util::Buffer (char ** insertPt, long * buflen,
                      const float value) {

#if (defined(sgi) || defined(hpux) || defined(sparc))
  float netValue = value;
#else
  float temp = value;
  long netValue = htonl(*(long *) &temp);
#endif

  long length = sizeof(netValue);

  CHECK(length < *buflen); // XXX bug

  memcpy(*insertPt, &netValue, length);
  *insertPt += length;
  *buflen -= length;

  return 0;
}

//static
int nmb_Util::Buffer (char ** insertPt, int * buflen,
                      const float value) {

#if (defined(sgi) || defined(hpux) || defined(sparc))
  float netValue = value;
#else
  float temp = value;
  int netValue = htonl(*(long *) &temp);
#endif

  int length = sizeof(netValue);

  CHECK(length < *buflen);  // XXX bug

  memcpy(*insertPt, &netValue, length);
  *insertPt += length;
  *buflen -= length;

  return 0;
}


//static
int nmb_Util::Buffer (char ** insertPt, long * buflen,
                      const double value) {

#if (defined(sgi) || defined(hpux) || defined(sparc))
  double netValue = value;
#else
  double temp = value;
  long netValue [2];
  netValue[0] = htonl(((long *) &temp)[0]);
  netValue[1] = htonl(((long *) &temp)[1]);
#endif

  long length = sizeof(netValue);

  CHECK(length < *buflen);  // XXX bug

  memcpy(*insertPt, &netValue, length);
  *insertPt += length;
  *buflen -= length;

  return 0;
}

//static
int nmb_Util::Buffer (char ** insertPt, int * buflen,
                      const double value) {

#if (defined(sgi) || defined(hpux) || defined(sparc))
  double netValue = value;
#else
  double temp = value;
  int netValue [2];
  netValue[0] = htonl(((long *) &temp)[0]);
  netValue[1] = htonl(((long *) &temp)[1]);
#endif

  int length = sizeof(netValue);

  CHECK(length < *buflen);  // XXX bug

  memcpy(*insertPt, &netValue, length);
  *insertPt += length;
  *buflen -= length;

  return 0;
}

//static
int nmb_Util::Buffer (char ** insertPt, long * buflen,
                      const char * string, unsigned long length) {
  CHECK(length < (unsigned)*buflen);  // XXX bug
  memcpy(*insertPt, string, length);
  *insertPt += length;
  *buflen -= length;

  return 0;
}

//static
int nmb_Util::Buffer (char ** insertPt, int * buflen,
                      const char * string, unsigned int length) {
  CHECK(length < (unsigned)*buflen);  // XXX bug

  memcpy(*insertPt, string, length);
  *insertPt += length;
  *buflen -= length;

  return 0;
}

//static
int nmb_Util::Buffer (char ** insertPt, long * buflen,
                      const struct timeval & time) {
  struct timeval netValue;
  long length = sizeof(struct timeval);

  CHECK(length < *buflen);  // XXX bug

  netValue.tv_sec = htonl(time.tv_sec);
  netValue.tv_usec = htonl(time.tv_usec);

  memcpy(*insertPt, &netValue, length);
  *insertPt += length;
  *buflen -= length;

  return 0;
}

//static
int nmb_Util::Buffer (char ** insertPt, int * buflen,
                      const struct timeval & time) {
  struct timeval netValue;
  int length = sizeof(struct timeval);

  CHECK(length < *buflen);  // XXX bug

  netValue.tv_sec = htonl(time.tv_sec);
  netValue.tv_usec = htonl(time.tv_usec);

  memcpy(*insertPt, &netValue, length);
  *insertPt += length;
  *buflen -= length;

  return 0;
}

//static
int nmb_Util::Buffer (char ** insertPt, long * buflen,
                      const vrpn_bool value) {
  vrpn_int32 netValue;
  long length = sizeof(vrpn_int32);

  CHECK(length < *buflen);  // XXX bug

  netValue = htonl(value);

  memcpy(*insertPt, &netValue, length);
  *insertPt += length;
  *buflen -= length;

  return 0;
}

//static
int nmb_Util::Buffer (char ** insertPt, int * buflen,
                      const vrpn_bool value) {
  vrpn_int32 netValue;
  int length = sizeof(vrpn_int32);

  CHECK(length < *buflen);  // XXX bug

  netValue = htonl(value);

  memcpy(*insertPt, &netValue, length);
  *insertPt += length;
  *buflen -= length;

  return 0;
}

//static
int nmb_Util::Unbuffer (const char ** buffer, long * i) { // Tiger change int* to long*
/* NANO BEGIN
  fprintf(stderr, "nmb_Util::Unbuffer(long): netValue = %lX\t sizeof(netValue) = %d\n", *(long *)(*buffer), sizeof(*(long *)(*buffer)));
*/
  *i = ntohl(*(long *)(*buffer));
  *buffer += sizeof(long);
/* NANO BEGIN
  fprintf(stderr, "nmb_Util::Unbuffer(long): after ntohl(), i = %lX\t sizeof(i) = %d\n", *i, sizeof(*i));
*/
  return 0;
}

//static
int nmb_Util::Unbuffer (const char ** buffer, int * i) {
/* NANO BEGIN
  fprintf(stderr, "nmb_Util::Unbuffer(int)\n");
*/
  *i = ntohl(*(long *)(*buffer));
  *buffer += sizeof(int);
  return 0;
}

//static
int nmb_Util::Unbuffer (const char ** buffer, float * f) {

#if (defined(sgi) || defined(hpux) || defined(sparc))
  *f = *((float *) (*buffer));
#else
  long value = ntohl(*(long *)(*buffer));
  *f = *((float *) &value);
#endif

  *buffer += sizeof(float);
  return 0;
}

//static
int nmb_Util::Unbuffer (const char ** buffer, double * f) {

#if (defined(sgi) || defined(hpux) || defined(sparc))
  *f = *((double *)(*buffer));
#else
  long value [2];
  value[0] = ntohl(((long *)(*buffer))[0]);
  value[1] = ntohl(((long *)(*buffer))[1]);
  *f = *((double *) value);
#endif

  *buffer += sizeof(double);
  return 0;
}

//static
int nmb_Util::Unbuffer (const char ** buffer, char * c,
                        unsigned long len)  {
  if (!c) return -1;

  memcpy(c, *buffer, len);
  *buffer += len;
  return 0;
}

//static
int nmb_Util::Unbuffer (const char ** buffer, struct timeval * tv) {
  tv->tv_sec = ntohl(((struct timeval *) (*buffer))->tv_sec);
  tv->tv_usec = ntohl(((struct timeval *) (*buffer))->tv_usec);

  *buffer += sizeof(struct timeval);
  return 0;
}


//static
int nmb_Util::Unbuffer (const char ** buffer, vrpn_bool * v) {
  *v = ntohl(((vrpn_int32 *)(*buffer))[0]);

  *buffer += sizeof(vrpn_int32);
  return 0;
}



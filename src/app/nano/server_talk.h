#ifndef _SERVER_TALK_
#define _SERVER_TALK_

#ifndef __CYGWIN__
#include	<netinet/in.h>
#include        <time.h>
#endif

extern int stm_enable_send();
extern int stm_clear_outgoing_buffer();
extern int stm_buffer_int(int);
extern int stm_buffer_float(double);
extern int stm_buffer_chars(char *c, int len);
extern int stm_send_buffer_to(int);
extern int stm_recv_buffer_from(int file, char *buffer, int maxlen);

typedef	char	*BUFPTR;

inline void stm_unbuffer_int(BUFPTR &bufptr, int &value) {
	value = ntohl(*(int*)(bufptr));
	bufptr += sizeof(int);
}

inline void stm_unbuffer_timeval(BUFPTR &bufptr, struct timeval &value) {
	value = (*(struct timeval*)(bufptr));
	value.tv_sec = htonl (value.tv_sec);
	value.tv_usec = htonl (value.tv_usec);
	bufptr += sizeof(struct timeval);
}

inline void stm_unbuffer_long (BUFPTR &bufptr, long &value) {
	value = ntohl(*(long*)(bufptr));
	bufptr += sizeof(long);
}

inline void stm_unbuffer_float(BUFPTR &bufptr, float &value) {
#if (defined(sgi) || defined(hpux) || defined(sparc))
	value = *(float*)(bufptr);
#else
	int	localvalue;
	localvalue = ntohl(*(int*)(bufptr));
	value = *(float*)(&localvalue);
#endif
	bufptr += sizeof(float);
}

inline	void stm_unbuffer_chars(BUFPTR &bufptr, char *c, int len) {
	memcpy(c, bufptr, len);
	bufptr += len;
}

#endif 

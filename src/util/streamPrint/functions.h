// functions.h

#include        <stdio.h>
#include        <strings.h>
#include        <stdlib.h>
#include <sys/time.h>

// connectsock.c
int connectsock (char *, int, int, int);

// error.c
void err_quit ();
void err_sys ();

// nano_utils.c
typedef char* BUFPTR;

void stm_unbuffer_short (BUFPTR* bufptr, short* value);
void stm_unbuffer_double (BUFPTR* bufptr, double* value);
void stm_unbuffer_int (BUFPTR* bufptr, int* value);
void stm_unbuffer_long(BUFPTR* bufptr, long* value);
void stm_unbuffer_timeval(BUFPTR* bufptr, struct timeval* value);
void stm_unbuffer_float(BUFPTR* bufptr, float* value);
void stm_unbuffer_chars(BUFPTR* bufptr, char *c, int len);
int time_compare(struct timeval t1, struct timeval t2);
int  time_subtract(struct timeval t1, struct timeval t2, struct timeval *res);
void time_multiply(struct timeval t1, double scale, struct timeval *res);
void    time_add(struct timeval t1, struct timeval t2, struct timeval *res);
int     stm_read_block_from_stream(stm_stream* s, char* buf);
int     noint_block_write(int outfile, char buffer[], int length);
int     noint_block_read(int infile, char buffer[], int length);
int stm_retry_save(char* name);
int     stm_close_stream(stm_stream *s);
stm_stream      *stm_open_datastream_for_read(char* filename);

// stevens.c
int readn (register int fd, register char *ptr, register int nbytes);
int writen(register int fd, register char *ptr, register int nbytes);
int readline(register int fd, register char* ptr, register int maxlen);






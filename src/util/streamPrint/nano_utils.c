// nano_utils.c
// Utility functions taken from the nano code 
// -- most come from stm_file.c

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>

//#include "inet.h"

#include "stm_file.h"

typedef char* BUFPTR;

// Borrowed inline functions from server_talk.h
void stm_unbuffer_int (BUFPTR* bufptr, int* value) {
  *value = ntohl(*(int *)(*bufptr));
  *bufptr += sizeof(int);
}

void stm_unbuffer_long(BUFPTR* bufptr, long* value) {
  *value = ntohl(*(long *)(*bufptr));
  *bufptr += sizeof(long);
}

void stm_unbuffer_timeval(BUFPTR* bufptr, struct timeval* value) {
  *value = (*(struct timeval *)(*bufptr));
  value->tv_sec = ntohl (value->tv_sec);
  value->tv_usec = ntohl (value->tv_usec);
  *bufptr += sizeof(struct timeval);
}

void stm_unbuffer_short (BUFPTR* bufptr, short* value) {
  *value = ntohl (*(short *) (*bufptr));
  *bufptr += sizeof (short);
}

void stm_unbuffer_double (BUFPTR* bufptr, double* value) {
  *value = ntohl (*(double *) (*bufptr));
  *bufptr += sizeof (double);
}

void stm_unbuffer_float(BUFPTR* bufptr, float* value) {
#if (defined(sgi) || defined(hpux) || defined(sparc))
  *value = *(float*)(*bufptr);
#else
  int	localvalue;
  localvalue = ntohl(*(int*)(*bufptr));
  *value = *(float*)(&localvalue);
#endif
  *bufptr += sizeof(float);
}

void stm_unbuffer_chars(BUFPTR* bufptr, char *c, int len) {
  memcpy(c, *bufptr, len);
  *bufptr += len;
}


/****************
 *      This routine returns -1 if t1 < t2, 0 if t1 = t2, 1 if t1 > t2
 ****************/

int time_compare(struct timeval t1, struct timeval t2)
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

int  time_subtract(struct timeval t1, struct timeval t2, struct timeval *res)
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

void time_multiply(struct timeval t1, double scale, struct timeval *res)
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

void	time_add(struct timeval t1, struct timeval t2, struct timeval *res)
{
  res->tv_usec = t1.tv_usec + t2.tv_usec;
  res->tv_sec = t1.tv_sec + t2.tv_sec;
  if (res->tv_usec >= 1000000l) {
    res->tv_sec++;
    res->tv_usec -= 1000000l;
  }
}


/*      This routine will write a block to a file descriptor.  It acts just
 * like the write() system call does on files, but it will keep sending to
 * a socket until an error or all of the data has gone.
 *      This will also take care of problems caused by interrupted system
 * calls, retrying the write when they occur. */

int     noint_block_write(int outfile, char buffer[], int length)
{
        register int    sofar;          /* How many characters sent so far */
        register int    ret;            /* Return value from write() */

        sofar = 0;
        do {
                /* Try to write the remaining data */
                ret = write(outfile, buffer+sofar, length-sofar);
                sofar += ret;

                /* Ignore interrupted system calls - retry */
                if ( (ret == -1) && (errno == EINTR) ) {
                        ret = 0;
                        sofar += 1;
                }

        } while ( (ret >= 0) && (sofar < length) );

        if (ret == -1) return(-1);
        else return(sofar);
}

/*      This routine will read in a block from the file descriptor.
 * It acts just like the read() routine does on normal files, so that
 * it hides the fact that the descriptor may point to a socket.  This
 * routine is used by the others in this file to get things from the
 * input file.
 *      This routine ignores interrupts while reading. */

int     noint_block_read(int infile, char buffer[], int length)
{
        register int    sofar;          /* How many we read so far */
        register int    ret;            /* Return value from the read() */

        sofar = 0;
        do {
                ret = read(infile, buffer+sofar, length-sofar);
                sofar += ret;

                /* Ignore interrupted system calls - retry */
                if ( (ret == -1) && (errno == EINTR) ) {
                        ret = 0;
                        sofar += 1;
                }
        } while ((ret >= 0) && (sofar < length));

        if (ret == -1) return(-1);
        else return(sofar);
}


/*      This routine creates a stream for writing and returns a pointer
 * to it.  Don't allow overwriting of an existing streamfile.
 * It returns NULL on failure. */
stm_stream *stm_open_datastream_for_write(char* filename)
{
  stm_stream *s;

  /* Allocate the stream structure. */
  if ( (s=(stm_stream*)malloc(sizeof(stm_stream))) == NULL) {
    fprintf(stderr,
	    "stm_open_datastream_for_write(): can't malloc stream\n");
    return(NULL);
  }
  
  /* Attempt to open the file for writing. */
  /* Do not allow overwriting of an existing file. */
  if ( (s->descriptor = open(filename, O_WRONLY | (O_CREAT | O_EXCL),
			     0644))<0) {
    free((char*)s);
    perror("stm_open_datastream_for_write(): can't open");
    fprintf(stderr,"  (file %s)\n",filename);
    return(NULL);
  }

  /* Attempt to malloc() the buffer area for the stream. */
  if ( (s->buffer = (char*)malloc(STM_STREAM_BUFSIZE)) == NULL) {
    close(s->descriptor);
    free((char*)s);
    fprintf(stderr, "stm_open_datastream_for_write(): can't malloc %d bytes",
	    STM_STREAM_BUFSIZE);
    fprintf(stderr, " for buffer\n");
    return(NULL);
  }
  
  /* Initialize the stream. */
  strcpy(s->filename, filename);
  s->cur = s->buffer;
  s->in_len = -1;                         /* output, not input */
  s->fullcheck = 1;
  s->multibuf = 0;                        /* No multiple buffers yet */
  //  s->dup_state = STREAM_DUP_STATE_NONE;   /* Not allowing dup yet */
  
  /* Return the stream to the user */
  return(s);
}



/*      This routine will add a block to the buffer area of the given
 * stream.  If the data will not fit into the buffer area, the buffer is
 * flushed to disk and then reused.  A warning is printed in this case,
 * because it will not be possible to write a new file if the close on
 * this stream file fails.  Warnings are also printed as each block gets
 * near to filling up.
 *      An integer that specifies the length of the block is inserted
 * into the stream before the actual block is.  This allows the data
 * to be retrieved in block chunks. */

int     stm_write_block_to_stream(stm_stream *s, /* Stream to write to */
                                  char* buf, /* Buffer to send */
                                  int len) /* Number of bytes to send */
{
  unsigned long local_len = len;
  int bytes_used = s->cur-s->buffer;

  /* Make sure the block will fit in the buffer ever */
  if ( (len + 2*sizeof(int)) > STM_STREAM_BUFSIZE) {
    fprintf(stderr,"stm_write_to_stream(): block too large! (%d -> %d)\n",
	    len + 2* sizeof (int), STM_STREAM_BUFSIZE);
    return(-1);
  }
  
  /* Check to make sure it will all fit */
  if( ((s->cur - s->buffer) + len + 2*sizeof(int)) > STM_STREAM_BUFSIZE) {
    fprintf(stderr,"stm_write_to_stream(): Stream buffer full\n");
    fprintf(stderr,"  (Purging to disk to get more space)\n");
    fprintf(stderr,"  Warning - will not be able to recover if ");
    fprintf(stderr,"file too large for partition!\n");
    
    /* Attempt to write the buffer to the file. */
    if (noint_block_write(s->descriptor, s->buffer,
			      (s->cur - s->buffer)) != (s->cur - s->buffer)) {
      perror("stm_write_block_to_stream(): Can't write buffer");
      close(s->dup_descriptor);
      return -1;
    }

    /* It worked... free the buffer for a new load */
    s->multibuf = 1;
    s->fullcheck = 1;
    s->cur = s->buffer;
    bytes_used = 0;
  }
  
  /* Copy the length into the buffer.  Get the byte order right */
  local_len = htonl(local_len);
  memcpy(s->cur, &local_len, sizeof(local_len));
  
  /* Advance the pointer */
  s->cur += sizeof(local_len);
  
  /* Copy the buffer */
  memcpy(s->cur, buf, len);
  
  /* Advance the pointer */
  s->cur += len;
  
  /* now check to see if we're filling up */
  if((STM_STREAM_BUFSIZE-bytes_used) < (STM_STREAM_BUFSIZE >> s->fullcheck)) {
    fprintf( stderr, 
	     "WARNING: Stream file past %d%% full:WARNING\n",
	     (int)((1.0-1.0/(1<<s->fullcheck))*100) );
    s->fullcheck++;
  }
  return(0);
}


/*	This routine will read one block from the data stream and return
 * it in the given buffer.  The length of the block is returned by the
 * function.  A value of -1 is returned when the end of the file has been
 * reached and no data is filled into the buffer.
 *	No error checking is done on the data in the file. */

int	stm_read_block_from_stream(stm_stream* s, /* Stream to write to */
				   char* buf) /* Buffer to send */
{
  unsigned long	len;

  /* See if we are at the end */
  if ( (s->cur - s->buffer) >= s->in_len) {
    return(-1);
  }

  /* Copy the length from the buffer.  Get the byte order right */
  memcpy(&len, s->cur, sizeof(len));
  len = htonl(len);
  
  /* Advance the pointer */
  s->cur += sizeof(len);
  
  /* Copy the buffer */
  memcpy(buf, s->cur, len);
  
  /* Advance the pointer */
  s->cur += len;
  
  return(len);
}



int stm_retry_save(char* name)
{
        char    input_line[255];

        /* See if they want to retry */
        printf("Retry (y/n) [y] ? ");
//        gets(input_line);
        scanf ("%s", input_line);
        if ( (input_line[0] == 'n') || (input_line[0] == 'N') ) {
                return(0);
        }

        /* Find the name of the file to use (default unchanged) */
        printf("Filename [%s]: ",name);
//        gets(input_line);
        scanf ("%s", input_line);
        if (strlen(input_line) > 0) {
                strcpy(name, input_line);
        }
        return(1);
}





/*	This routine will flush the contents of the stream buffer to the
 * disk file and then close the file.
 *	If there is a failure during the close for a write stream, the routine
 * will query the user about whether it should retry.  It will allow the user
 * to enter a new file name to try to save the data if they want.  This is
 * to allow the data to be recovered in almost any circumstance.
 *	It returns 0 on success and -1 on failure.  Note that this
 * could take a long time to finish for write streams. */

int	stm_close_stream(stm_stream *s)
{
	int	failure = 0;

	/* If this is a writing stream, send the buffer */
	if (s->in_len == -1) {

		/* Attempt to write the stream to the file. */
		if (noint_block_write(s->descriptor, s->buffer,
		   (s->cur - s->buffer)) != (s->cur - s->buffer)) {
			perror("stm_close_stream(): Can't write buffer");
			fprintf(stderr,"   (file %s)\n",s->filename);
			failure = 1;
		}
	}

	/* Close the file */
	if (close(s->descriptor)) {
		perror("stm_close_stream(): Error closing file");
		fprintf(stderr,"   (file %s)\n",s->filename);
		failure = 1;
	}

	/* If this was a reading stream and there was a failure, return -1 */
	if (failure && (s->in_len != -1) ) {
		return(-1);
	}

	/* If there was a failure during write, allow retries */
	while (failure && (s->in_len == -1) ) {

		/* Can't allow retries if multiple buffers. */
		if (s->multibuf) {
			fprintf(stderr,"stm_close_stream(): Can't recover from save failure on multi-buffer file.\n");
			failure = 1;
			return(-1);
		}

		/* Ask if should retry and with what filename */
		if (!stm_retry_save(s->filename)) {	/* No retry */
			return(-1);
		}
		failure = 0;		/* This time might work */

		/* Attempt to open the file for writing. */
		if ( (s->descriptor = open(s->filename, O_WRONLY | O_CREAT |
		      O_TRUNC, 0644))<0) {
			perror("stm_close_stream(): can't open");
			fprintf(stderr,"   (file %s)\n",s->filename);
			failure = 1;
		}

		/* Attempt to write the stream to the file. */
		if (noint_block_write(s->descriptor, s->buffer,
		   (s->cur - s->buffer)) != (s->cur - s->buffer)) {
			perror("stm_close_stream(): Can't write buffer");
			fprintf(stderr,"   (file %s)\n",s->filename);
			failure = 1;
		}

		/* Close the file */
		if (close(s->descriptor)) {
			perror("stm_close_stream(): Error closing file");
			fprintf(stderr,"   (file %s)\n",s->filename);
			failure = 1;
		}
	}

	return(0);
}


/*	This routine creates a stream for reading, reads the data into the
 * buffer from the file, and returns a pointer to the stream.  It returns
 * NULL on failure.
 *	Note that this routine can take a long time to complete. */

stm_stream	*stm_open_datastream_for_read(char* filename)
{
	stm_stream	*s;
	struct	stat	statbuf;
	int		length;

	/* Allocate the stream structure. */
	if ( (s=(stm_stream*)malloc(sizeof(stm_stream))) == NULL) {
		fprintf(stderr,
		    "stm_open_datastream_for_read(): can't malloc stream\n");
		return(NULL);
	}

	/* Attempt to open the file for reading. */
	if ( (s->descriptor = open(filename, O_RDONLY)) < 0) {
		free((char*)s);
		perror("stm_open_datastream_for_read(): can't open");
		fprintf(stderr,"   (file %s)\n",filename);
		return(NULL);
	}

	/* Find out how big the file is */
	if (fstat(s->descriptor, &statbuf)) {
		perror("stm_open_datastream_for_read(): Can't stat file");
		return(NULL);
	}
	length = (int)statbuf.st_size;

	/* Attempt to malloc() the buffer area for the stream. */
	if ( (s->buffer = (char*)malloc(length)) == NULL) {
		close(s->descriptor);
		free((char*)s);
		fprintf(stderr,
		    "stm_open_datastream_for_read(): can't malloc buffer\n");
		return(NULL);
	}

	/* Initialize the stream. */
	strcpy(s->filename, filename);
	s->cur = s->buffer;

	/* Read in as much data as the file has */
	s->in_len = noint_block_read(s->descriptor, s->cur, length);
	if (s->in_len < 0) {
		perror("stm_open_datastream_for_read(): Read failed");
		fprintf(stderr,"   (file %s, %d characters)\n",filename,length);
		close(s->descriptor);
		free((char*)s);
		return(NULL);
	}

	/* Return the stream to the user */
	return(s);
}

/************************************************************************
 *			stm_file.c
 *
 *	This file contains routines that read/write the files that we
 * get from the Scanning Tunnelling Microscope.
 *
 ************************************************************************/

#include 	"mf.h"
#include	<stdio.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<string.h>
#include	<malloc.h>
#include	<errno.h>
#include	<sys/types.h>
#include	<time.h>
#if (defined(sparc) || defined(linux) || defined(__CYGWIN__))
#include	<sys/time.h>
#endif
#include	<sys/socket.h>
#include	<sys/stat.h>
#include	<netinet/in.h>
#ifndef __CYGWIN__
#include	<netinet/tcp.h>
#endif
#include	<netdb.h>
#include	"stm_file.h"

#define	STREAM_DUP_STATE_BROKEN		(-1)
#define	STREAM_DUP_STATE_NONE		(0)
#define	STREAM_DUP_STATE_LISTENING	(1)
#define	STREAM_DUP_STATE_CONNECTED	(2)

#ifdef FLOW
  extern int sdi_noint_select(int nfds, fd_set *readfds,
		 fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
  extern int sdi_noint_block_write(int outfile, char buffer[], int length);
  extern int sdi_noint_block_read(int infile, char buffer[], int length);
#else
  #ifdef __cplusplus
     extern "C" int sdi_noint_select(int nfds, fd_set *readfds,
		 fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
     extern "C" int sdi_noint_block_write(int outfile, char buffer[], int length);
     extern "C" int sdi_noint_block_read(int infile, char buffer[], int length);
     #ifdef _WIN32
        extern "C" int sdi_noint_block_file_read(int infile, char buffer[], int length);
     #endif
  #endif
#endif

/*	This routine creates a stream for writing and returns a pointer
 * to it.  Don't allow overwriting of an existing streamfile.
 * It returns NULL on failure. */

stm_stream	*stm_open_datastream_for_write (const char * filename)
{
	stm_stream	*s;

	/* Allocate the stream structure. */
	if ( (s=(stm_stream*)malloc(sizeof(stm_stream))) == NULL) {
		fprintf(stderr,
		    "stm_open_datastream_for_write(): can't malloc stream\n");
		return(NULL);
	}

	/* Attempt to open the file for writing. */
	/* Do not allow overwriting of an existing file. */
#ifndef _WIN32
	if ( (s->descriptor = open(filename, O_WRONLY | (O_CREAT | O_EXCL),
	     0644))<0) {
#else
	if ( (s->descriptor = open(filename, O_WRONLY | (O_CREAT | O_EXCL) | O_BINARY,
	     0644))<0) {
#endif

		free((char*)s);
		perror("stm_open_datastream_for_write(): can't open");
		fprintf(stderr,"  (file %s)\n",filename);
		return(NULL);
	}

	/* Attempt to malloc() the buffer area for the stream. */
	if ( (s->buffer = (char*)malloc(STM_STREAM_BUFSIZE)) == NULL) {
		close(s->descriptor);
		free((char*)s);
		fprintf(stderr,
		    "stm_open_datastream_for_write(): can't malloc buffer\n");
		return(NULL);
	}

	/* Initialize the stream. */
	strcpy(s->filename, filename);
	s->cur = s->buffer;
	s->in_len = -1;				/* output, not input */
	s->fullcheck = 1;
	s->multibuf = 0;			/* No multiple buffers yet */
	s->dup_state = STREAM_DUP_STATE_NONE;	/* Not allowing dup yet */

	/* Return the stream to the user */
	return(s);
}

/*	This routine creates a stream for reading, reads the data into the
 * buffer from the file, and returns a pointer to the stream.  It returns
 * NULL on failure.
 *	Note that this routine can take a long time to complete. */

stm_stream	* stm_open_datastream_for_read (const char * filename)
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
#ifndef _WIN32
	if ( (s->descriptor = open(filename, O_RDONLY)) < 0) {
#else
	if ( (s->descriptor = open(filename, O_RDONLY | O_BINARY)) < 0) {
#endif 
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
#ifdef _WIN32
	s->in_len = sdi_noint_block_file_read(s->descriptor, s->cur, length);
#else
	s->in_len = sdi_noint_block_read(s->descriptor, s->cur, length);
#endif
	if (s->in_len < 0) {
		perror("stm_open_datastream_for_read(): Read failed");
		fprintf(stderr,"   (file %s, %d characters; retval: %d)\n",filename,length,s->in_len);
		close(s->descriptor);
		free((char*)s);
		return(NULL);
	}

	/* Return the stream to the user */
	s->dup_state = STREAM_DUP_STATE_NONE;	/* Not allowing dup yet */
	return(s);
}


/*	This routine will add a block to the buffer area of the given
 * stream.  If the data will not fit into the buffer area, the buffer is
 * flushed to disk and then reused.  A warning is printed in this case,
 * because it will not be possible to write a new file if the close on
 * this stream file fails.  Warnings are also printed as each block gets
 * near to filling up.
 *	An integer that specifies the length of the block is inserted
 * into the stream before the actual block is.  This allows the data
 * to be retrieved in block chunks. */

int	stm_write_block_to_stream(stm_stream *s, /* Stream to write to */
				  char* buf, /* Buffer to send */
				  int len) /* Number of bytes to send */
{
	unsigned long	local_len = len;
	int	bytes_used = s->cur-s->buffer;


    /* This section will check to see if a remote process wants to have
     * this streamfile duplicated to it.  This check is only done when the
     * dup_state is LISTEN.  If it is listen, and there is a connection
     * request, then the connection is accepted and all of the data in the
     * current stream is sent over.
     * This is not done if we are past the first buffer, since we can't
     * get the beginning of the stream in that case. */

    if ( (!s->multibuf) && (s->dup_state == STREAM_DUP_STATE_LISTENING) ) {
	static	struct	timeval notime = { 0,0 };
	fd_set	inmask;		/* Used in select() */

	/* See if there is a connection request on the socket.
	 * The select() will return immediately on success or failure. */
	FD_ZERO(&inmask);
	FD_SET(s->listen_descriptor,&inmask);
	if (sdi_noint_select(32,&inmask,NULL,NULL,&notime) == 1) {
		printf("Accepting remote stream duplication request...\n");

		/* Connection requested - accept it */
                if ((s->dup_descriptor=accept(s->listen_descriptor,0,0)) == -1){
                        perror("stm_write_block_to_stream(): accept() failed");
                        close(s->listen_descriptor);
			s->dup_state = STREAM_DUP_STATE_BROKEN;
                }

                /* Set the socket for TCP_NODELAY */

                if (s->dup_state != STREAM_DUP_STATE_BROKEN) {
			struct  protoent        *p_entry;
                        static  int     nonzero = 1;

                        if ( (p_entry = getprotobyname("TCP")) == NULL ) {
                                fprintf(stderr,
                   "stm_write_block_to_stream(): getprotobyname() failed.\n");
                                close(s->dup_descriptor);
				s->dup_state = STREAM_DUP_STATE_BROKEN;
                        }

                        if (setsockopt(s->dup_descriptor, p_entry->p_proto,
                            TCP_NODELAY, (const char *)&nonzero,
			    sizeof(nonzero))==-1) {
                                perror(
			  "stm_write_block_to_stream(): setsockopt() failed");
                                close(s->dup_descriptor);
				s->dup_state = STREAM_DUP_STATE_BROKEN;
                        }
                }

		/* Send all of the data that is already in the stream to
		 * the connecting client */
		/* Be sure to send each packet separately, and translate
		 * the length fields.  In a streamfile, the length of the
		 * packet does NOT include the length of the field that
		 * tells the length, while from a microscope it DOES. */
		printf("  ...sending existing stream data...\n");
		{ char *nextchar = s->buffer;
		  int blocklen;
		  int netblocklen;

		  s->dup_state = STREAM_DUP_STATE_CONNECTED;
		  do {
		    blocklen = ntohl(*(int*)nextchar);
		    netblocklen = htonl(blocklen + sizeof(blocklen));
		    nextchar += sizeof(blocklen);
		    if (sdi_noint_block_write(s->dup_descriptor,
			    (char*)&netblocklen,
			    sizeof(netblocklen)) != sizeof(netblocklen)) {
		        perror("stm_write_block_to_stream(): Dup write failed");
		        close(s->dup_descriptor);
		        s->dup_state = STREAM_DUP_STATE_LISTENING;
			break;
		    }
		    if (sdi_noint_block_write(s->dup_descriptor,
					  nextchar, blocklen) != blocklen) {
		        perror("stm_write_block_to_stream(): Dup write failed");
		        close(s->dup_descriptor);
		        s->dup_state = STREAM_DUP_STATE_LISTENING;
			break;
		    }
		    nextchar += blocklen;
		  } while ( nextchar < s->cur );
		  if (s->dup_state == STREAM_DUP_STATE_CONNECTED) {
		        /* We're done accepting the connection. */
		        printf("...duplication stream now open\n");
		  }
		}
	}
    }

	/* Make sure the block will fit in the buffer ever */
	if ( (len + 2*sizeof(int)) > STM_STREAM_BUFSIZE) {
		fprintf(stderr,"stm_write_to_stream(): block too large!\n");
		return(-1);
	}

	/* Check to make sure it will all fit */
	if( ((s->cur - s->buffer) + len + 2*sizeof(int)) > STM_STREAM_BUFSIZE) {
		fprintf(stderr,"stm_write_to_stream(): Stream buffer full\n");
		fprintf(stderr,"  (Purging to disk to get more space)\n");
		fprintf(stderr,"  Warning - will not be able to recover if file too large for partition!\n");

		/* Attempt to write the buffer to the file. */
		if (sdi_noint_block_write(s->descriptor, s->buffer,
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

	/* Write the block to the duplicate stream if we have one */
	/* Be sure to adjust the block length field to include the length
	 * of the field itself in the outgoing stream, since this is
	 * what the microscope streams do. */
	if (s->dup_state == STREAM_DUP_STATE_CONNECTED) {
	  int netblocklen = htonl(len + sizeof(netblocklen));
	  if (sdi_noint_block_write(s->dup_descriptor, (char*)&netblocklen,
		   sizeof(netblocklen)) != sizeof(netblocklen)) {
	    perror("stm_write_block_to_stream(): Can't write block length");
	    close(s->dup_descriptor);
	    s->dup_state = STREAM_DUP_STATE_BROKEN;
	  }
	  if (sdi_noint_block_write(s->dup_descriptor, s->cur-len, len) != len){
	    perror("stm_write_block_to_stream(): Can't write block");
	    close(s->dup_descriptor);
	    s->dup_state = STREAM_DUP_STATE_BROKEN;
	  }
	}

	/* Read anything from the duplicate stream and discard it */
	if (s->dup_state == STREAM_DUP_STATE_CONNECTED) {
		static	char	inbuf[2048];	/* Pick size > max packet */
		static  struct  timeval notime = { 0,0 };
		fd_set  readfds, exceptfds;
		int     ret;

		/* Check to see if there are any incoming characters.
		 * If so, read at most max_char of them into buffer. */

		FD_ZERO(&readfds);              /* Clear the descriptor sets */
		FD_ZERO(&exceptfds);
		FD_SET(s->dup_descriptor, &readfds);     /* Check for read */
		FD_SET(s->dup_descriptor, &exceptfds);   /* Check for except */
		ret = sdi_noint_select(32,&readfds,NULL,&exceptfds, &notime);

		/* See if the select failed */
		if (ret == -1) {
			perror("stm_write_block_to_stream(): select failed()");
			close(s->dup_descriptor);
			s->dup_state = STREAM_DUP_STATE_BROKEN;
		}

		/* If there is an exception on the socket, close it */
		if (FD_ISSET(s->dup_descriptor, &exceptfds)) {
			close(s->dup_descriptor);
			s->dup_state = STREAM_DUP_STATE_BROKEN;
		}

		/* If there is data on the socket, read it */
		if (FD_ISSET(s->dup_descriptor, &readfds)) {
			ret = read(s->dup_descriptor, inbuf, sizeof(inbuf));
			if (ret == 0) { /* End of file, socket closed */
				close(s->dup_descriptor);
				s->dup_state = STREAM_DUP_STATE_BROKEN;
			}
			if (ret == -1) {
			    perror("stm_write_blovk_to_stream(): read failed");
			    close(s->dup_descriptor);
			    s->dup_state = STREAM_DUP_STATE_BROKEN;
			}
		}
	}

	/* now check to see if we're filling up */
	if( (STM_STREAM_BUFSIZE-bytes_used) 
	    < 
	    (STM_STREAM_BUFSIZE >> s->fullcheck) ) {
		fprintf( stderr, 
		   "WARNING: Stream file past %d%% full:WARNING\n",
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

int	stm_read_block_from_stream(stm_stream* s, /* Stream to read from */
				   char* buf) /* Buffer to return */
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


/*	This routine will ask the user if they want to retry and ask them
 * the name of a file to retry with.  It is used by the stm_close_stream()
 * routine if the write/close fails on a write stream.
 *	This routine returns 1 if it should retry, 0 if not.  It fills the
 * new name into the passed parameter unless the name is not changed by
 * the user. */

int	stm_retry_save(char* name)
{
	char	input_line[255];

	/* See if they want to retry */
	printf("Retry (y/n) [y] ? ");
	fgets(input_line, 255, stdin);
	if ( (input_line[0] == 'n') || (input_line[0] == 'N') ) {
		return(0);
	}

	/* Find the name of the file to use (default unchanged) */
	printf("Filename [%s]: ",name);
	fgets(input_line, 255, stdin);
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
		if (sdi_noint_block_write(s->descriptor, s->buffer,
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
#ifndef _WIN32
		if ( (s->descriptor = open(s->filename, O_WRONLY | O_CREAT |
		      O_TRUNC, 0644))<0) {
#else
		if ( (s->descriptor = open(s->filename, O_WRONLY | O_CREAT | O_BINARY |
		      O_TRUNC, 0644))<0) {
#endif
			perror("stm_close_stream(): can't open");
			fprintf(stderr,"   (file %s)\n",s->filename);
			failure = 1;
		}

		/* Attempt to write the stream to the file. */
		if (sdi_noint_block_write(s->descriptor, s->buffer,
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

/*	This routine will toss the contents of the stream buffer and then
 * delete the file on the disk.  This is used to abort the saving of a
 * stream file.
 *	It returns 0 on success and -1 on failure. */

int	stm_abort_stream(stm_stream *s)
{
	int	failure = 0;

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

	/* If this is a writing stream, delete the file */
	if (s->in_len == -1) {
		return (unlink(s->filename));
	}

	return(0);
}

int	stm_allow_tcp_duplication_on(stm_stream *s, unsigned short port_number)
{
	struct sockaddr_in name;	/* The socket to listen to */

	/* Make sure that this is a writing stream.  We can't duplicate
	 * reading streams. */
	if (s->in_len != -1) {
		fprintf(stderr,"Error: Attempt to duplicate read stream\n");
		return -1;
	}

	/* Open the socket that we're going to accept connections on and
	 * listen() on it. */

	name.sin_family = AF_INET;
	name.sin_addr.s_addr = INADDR_ANY;
	name.sin_port = htons(port_number);
	s->listen_descriptor = socket(AF_INET,SOCK_STREAM,0);
	if (s->listen_descriptor < 0) {
		perror("stm_allow_tcp_duplication_on(): can't open socket");
		return(-1);
	}
	if ( bind(s->listen_descriptor,(struct sockaddr*)&name,sizeof(name)) ) {
		close(s->listen_descriptor);
		perror("stm_allow_tcp_duplication_on(): binding socket");
		return(-1);
	}
	if ( listen(s->listen_descriptor,1) ) {
		close(s->listen_descriptor);
		perror("stm_allow_tcp_duplication_on(): failed listen");
		return(-1);
	}

	printf("stm_allow_tcp_duplication_on(): Listening on port %d\n",
		port_number);

	/* Set the dup state to listening */
	s->dup_state = STREAM_DUP_STATE_LISTENING;
	return 0;
}

/* reset the stream to it's start. 
   Return 0 for success, -1 for failure
*/
int	stm_restart_stream(stm_stream *s)
{
  if (s->in_len == -1) {
    fprintf(stderr, "stm_restart_stream:: Can't restart write stream.\n");
    return -1;
  }
  s->cur = s->buffer;
  return 0;
}

#ifndef	STM_FILE_H
#define STM_FILE_H

/*******
 * Datastream information.
 *******/

#define STM_STREAM_BUFSIZE      (20000000)
#define STM_NEAR_FULL_PERCENT   (90)
#define STM_STREAM_NEAR_FULL    ((STM_STREAM_BUFSIZE*STM_NEAR_FULL_PERCENT)/100)

struct stm_stream {
  char    filename[255];          /* Name of file */
  int     descriptor;             /* File descriptor */
  char    *buffer;                /* Holds the data */
  char    *cur;                   /* Where in the buffer */
  int     in_len;                 /* Length for read streams */
				  /*   -1 for write streams */
  int     fullcheck;              /* Used to monitor fullness */
  int     multibuf;               /* Written multiple buffers? */

  int	dup_state;		/* What state is duplication? */
  int	listen_descriptor;	/* TCP link to listen() on*/
  int	dup_descriptor;		/* TCP link to duplicate to */
} ;

extern stm_stream * stm_open_datastream_for_write (const char *);
extern stm_stream * stm_open_datastream_for_read (const char *);

extern int stm_write_block_to_stream (stm_stream *, char *, int);
extern int stm_read_block_from_stream (stm_stream *, char *);

extern int stm_close_stream (stm_stream * s);
extern int stm_abort_stream (stm_stream * s);

extern int stm_allow_tcp_duplication_on (stm_stream * s,
		unsigned short port_number);

extern int stm_restart_stream(stm_stream *s);

#endif

#include "vrpn_Ohmmeter.h"	// for ohmmeter status types
#include	<stdio.h>
#include	<sys/time.h>
#include	<sys/types.h>
#include	<errno.h>
#include	<math.h>

#include "stm_cmd.h"
#include "stm_file.h"
#include "functions.h"

/* Edited by Michele Clark 06/05/97 and 6/19/97 and 7/3/97
    o include messages found in stm_cmd.h that weren't here
    o changed all sec, usec to long and printed as %u
    o separate packets by a blank line, add msg # to output, so 
      that we can tell which packets were missed, if any
*/

long seqNum = 0;
long first_time_sec = 0;

extern  "C" int sdi_noint_block_write(int outfile, char buffer[], int length);
extern  "C" int sdi_noint_block_read(int infile, char buffer[], int length);
extern	"C" int	sdi_noint_select(int nfds, fd_set *readfds, fd_set *writefds,
				 fd_set *exceptfds, struct timeval *timeout);

/* Required extern variables */
float	response_x, response_y, pulse_x_res, pulse_y_res;
int	std_dev_samples, pulse_enabled, fmods_enabled;
float	pulse_peak, fmods_baseforce, pulse_bias, response_height, pulse_width;
float	std_dev_frequency, response_std_dev;

void Usage(char *s)
{
  fprintf(stderr,"Usage: %s instream\n",s);
  fprintf(stderr,"       instream: Input stream file\n");
  exit(-1);
}

/*****************************************************************************
 *
   translate_packet
	- Read one packet from file if there is one to read
	- Save the packet to the output stream, translating as needed
	- return 1 if something found, 0 if end of file, -1 if error
 *
 *****************************************************************************/

int	translate_packet(stm_stream *instream)
{
  int	x,y;		/* Position of current read */
  int dx, dy;
  float	fx,fy;		/* Position as a float */
  long	sec,usec;	/* When the value was obtained */
  float	value;		/* Value at this point */
  float	std_dev;	/* Standard deviation of value */
  float	xmin,xmax, ymin,ymax, zmin,zmax;
  float	fmin, fmax, fcur;
  int	enabled;
  float baseforce, modforce, base, mode, current;

  static	char    buffer[MAXBUF];   /* Holds data from the client */
  int     bufsize;        /* Number of characters read */
  char    *bufptr;        /* Current location in buffer */
  static	char	outbuffer[2*MAXBUF];	/* Holds output data */
  char	*outbufptr;

  char header[5000];
  int iscrap;
  long lscrap;
  float	fscrap;		/* Scrap variable */
  int reports, fields;
  int i, j;
  char string[100];
  float P, I, D;
  // ohmmeter resistance variables;
  float resist, voltage, range, filter;
  long status;
  // force curve
  long num_samples, num_halfcycles;

  printf ("\n** msg #%u:\n", ++seqNum);

  /* Read a block from the input file, if there is one */
  bufsize = stm_read_block_from_stream(instream, buffer);
  if (bufsize == -1) {	/* Indicates end of stream */
    return(0);
  }

  /* While the buffer has not been used up, read more reports */
  bufptr = buffer;        /* Point bufptr to buffer start */
  outbufptr = outbuffer;	/* Point output buffer to start */

  while ( (bufptr-buffer) < bufsize ) {
    int	data_type;

    /* Scan the current location */
    stm_unbuffer_int (&bufptr, &data_type);

    switch (data_type) {

    case STM_APPROACH_COMPLETE: 
      printf("STM_APPROACH_COMPLETE ()\n");
      break;

    case STM_TUNNELLING_FAILURE: 
      printf("STM_TUNNELLING_FAILURE ()\n");
      break;

    case STM_PULSE_PARAMETERS: 
      /* Read the parameters */
      stm_unbuffer_int (&bufptr, &pulse_enabled);
      stm_unbuffer_float (&bufptr, &pulse_bias);
      stm_unbuffer_float (&bufptr, &pulse_peak);
      stm_unbuffer_float (&bufptr, &pulse_width);
      printf("STM_PULSE_PARAMETERS (%d, %g,%g,%g)\n",
	     pulse_enabled,
	     pulse_bias,pulse_peak,pulse_width);
      break;

    case STM_STD_DEV_PARAMETERS:
      stm_unbuffer_int (&bufptr, &std_dev_samples);
      stm_unbuffer_float (&bufptr, &std_dev_frequency);
      printf("STM_STD_DEV_PARAMETERS (%d, %g)\n",
	     std_dev_samples, std_dev_frequency);
      break;

    case AFM_FORCE_SET:
      stm_unbuffer_float (&bufptr, &fscrap);   
      printf("AFM_FORCE_SET (%g)\n", fscrap);   
      break;

    case AFM_FORCE_SET_FAILURE:
      stm_unbuffer_float (&bufptr, &fscrap); 
      printf("AFM_FORCE_SET_FAILURE (%g)\n", &fscrap);
      break;

    case AFM_FORCE_PARAMETERS: 
      stm_unbuffer_int (&bufptr, &fmods_enabled);
      stm_unbuffer_float (&bufptr, &fmods_baseforce);
      printf("AFM_FORCE_PARAMETERS (%d, %g)\n",
	     fmods_enabled, fmods_baseforce);
      break;

    case STM_WINDOW_SCAN_NM: 
      stm_unbuffer_int (&bufptr, &x);     
      stm_unbuffer_int (&bufptr, &y);
      stm_unbuffer_long (&bufptr, &sec);
      stm_unbuffer_long (&bufptr, &usec);
      stm_unbuffer_float (&bufptr, &value);
      stm_unbuffer_float (&bufptr, &std_dev);
      printf("STM_WINDOW_SCAN_NM (%d,%d, %u:%u, %g,%g)\n",
	     x,y, sec-first_time_sec,usec, value,std_dev);
      break;

    case STM_WINDOW_BACKSCAN_NM: 
      //      printf ("STM_WINDOW_BACKSCAN_NM --> timestamped\n");
      stm_unbuffer_int (&bufptr, &x);     
      stm_unbuffer_int (&bufptr, &y);
      stm_unbuffer_long (&bufptr, &sec);
      stm_unbuffer_long (&bufptr, &usec);
      stm_unbuffer_float (&bufptr, &value);
      stm_unbuffer_float (&bufptr, &std_dev);
      printf("STM_WINDOW_BACKSCAN_NM (%d,%d, %u:%u, %g,%g)\n",
	     x,y, sec-first_time_sec,usec, value,std_dev);
      break;

    case STM_POINT_RESULT_NM: 
      //      printf ("STM_POINT_RESULT_NM --> timestamped\n");
      stm_unbuffer_float (&bufptr, &fx);     
      stm_unbuffer_float (&bufptr, &fy);
      stm_unbuffer_long (&bufptr, &sec);
      stm_unbuffer_long (&bufptr, &usec);
      stm_unbuffer_float (&bufptr, &value);
      stm_unbuffer_float (&bufptr, &std_dev);
      printf("STM_POINT_RESULT_NM (%d,%d, %u:%u, %g,%g)\n",
	     x,y, sec-first_time_sec,usec, value,std_dev);      
      break;

    case STM_PULSE_COMPLETED_NM: 
      stm_unbuffer_float (&bufptr, &fx);
      stm_unbuffer_float (&bufptr, &fy);
      printf("STM_PULSE_COMPLETED_NM (%g,%g)\n",
	     fx,fy);
      break;

    case STM_PULSE_FAILURE_NM: 
      stm_unbuffer_float (&bufptr, &fx);
      stm_unbuffer_float (&bufptr, &fy);
      printf("STM_PULSE_FAILURE_NM (%g,%g)\n",
	     fx,fy);
      break;

    case STM_SET_REGION_COMPLETED: 
      stm_unbuffer_float (&bufptr, &xmin);
      stm_unbuffer_float (&bufptr, &ymin);
      stm_unbuffer_float (&bufptr, &xmax);
      stm_unbuffer_float (&bufptr, &ymax);
      printf("STM_SET_REGION_COMPLETED (%g,%g, %g,%g)\n",
	     xmin,ymin, xmax,ymax);
      break;

    case STM_SET_REGION_CLIPPED: 
      stm_unbuffer_float (&bufptr, &xmin);
      stm_unbuffer_float (&bufptr, &ymin);
      stm_unbuffer_float (&bufptr, &xmax);
      stm_unbuffer_float (&bufptr, &ymax);
      printf("STM_SET_REGION_CLIPPED (%g,%g, %g,%g)\n",
	     xmin,ymin, xmax,ymax);
      break;

    case STM_TUNNELLING_ATTAINED_NM: 
      stm_unbuffer_float (&bufptr, &value);
      printf("STM_TUNNELLING_ATTAINED_NM (%g)\n",
	     value);
      break;

    case SPM_SCAN_RANGE: 
      stm_unbuffer_float (&bufptr, &xmin);
      stm_unbuffer_float (&bufptr, &xmax);
      stm_unbuffer_float (&bufptr, &ymin);
      stm_unbuffer_float (&bufptr, &ymax);
      stm_unbuffer_float (&bufptr, &zmin);
      stm_unbuffer_float (&bufptr, &zmax);
      printf("SPM_SCAN_RANGE (%g,%g, %g,%g, %g,%g)\n",
	     xmin,xmax, ymin,ymax, zmin, zmax);
      break;

    case AFM_MOD_FORCE_SET: 
      stm_unbuffer_float (&bufptr, &value);
      printf("AFM_MOD_FORCE_SET (%g)\n", value);
      break;

    case AFM_MOD_FORCE_SET_FAILURE: 
      stm_unbuffer_float (&bufptr, &value);
      printf("AFM_MOD_FORCE_SET_FAILURE (%g)\n", value);
      break;

    case AFM_IMG_FORCE_SET: 
      stm_unbuffer_float (&bufptr, &value);
      printf("AFM_IMG_FORCE_SET (%g)\n", value);
      break;

    case AFM_IMG_FORCE_SET_FAILURE: 
      stm_unbuffer_float (&bufptr, &value);
      printf("AFM_IMG_FORCE_SET_FAILURE (%g)\n", value);
      break;

    case AFM_IMG_SET: 
      stm_unbuffer_int (&bufptr, &enabled);
      stm_unbuffer_float (&bufptr, &fmax); 
      stm_unbuffer_float (&bufptr, &fmin); 
      stm_unbuffer_float (&bufptr, &fcur); 
      printf("AFM_IMG_SET (%d, %g,%g, %g)\n",
	     enabled, fmax,fmin, fcur);
      break;

    case AFM_MOD_SET: 
      stm_unbuffer_int (&bufptr, &enabled);
      stm_unbuffer_float (&bufptr, &fmax); 
      stm_unbuffer_float (&bufptr, &fmin); 
      stm_unbuffer_float (&bufptr, &fcur);
      printf("AFM_MOD_SET (%d, %g,%g, %g)\n",
	     enabled, fmax,fmin, fcur);
      break;

    case AFM_IN_MOD_MODE: 
      printf("AFM_IN_MOD_MODE ()\n");
      break;

    case AFM_IN_IMG_MODE: 
      printf("AFM_IN_IMG_MODE ()\n");
      break;

    case SPM_RELAX_SET: 
      stm_unbuffer_int (&bufptr, &x); 
      stm_unbuffer_int (&bufptr, &y); 
      printf("SPM_RELAX_SET (%d, %d)\n", x, y);
      break;

    case AFM_IN_MOD_MODE_T: 
      stm_unbuffer_long (&bufptr, &sec);
      stm_unbuffer_long (&bufptr, &usec);
      printf("AFM_IN_MOD_MODE_T (%u:%u)\n", sec-first_time_sec,usec);
      break;

    case AFM_IN_IMG_MODE_T: 
      stm_unbuffer_long (&bufptr, &sec);
      stm_unbuffer_long (&bufptr, &usec);
      printf("AFM_IN_IMG_MODE_T (%u:%u)\n", sec-first_time_sec,usec);
      break;

    case SPM_RESISTANCE:
      stm_unbuffer_long (&bufptr, &lscrap);
      stm_unbuffer_long (&bufptr, &sec);
      stm_unbuffer_long (&bufptr, &usec);
      stm_unbuffer_float (&bufptr, &fscrap);
      printf ("SPM_RESISTANCE (%u, %u:%u, %g)\n", 
              lscrap, sec-first_time_sec, usec, fscrap);
      break;

    case SPM_RESISTANCE_FAILURE:
      stm_unbuffer_long (&bufptr, &lscrap);
      printf ("SPM_RESISTANCE_FAILURE (%u)\n", lscrap);
      break;
      
    case STM_ZIG_RESULT_NM:
      printf ("STM_ZIG_RESULT_NM (");
      stm_unbuffer_float (&bufptr, &fx);
      stm_unbuffer_float (&bufptr, &fy);
      stm_unbuffer_long (&bufptr, &sec);
      stm_unbuffer_long (&bufptr, &usec);
      printf ("%g, %g, %u:%u, ", fx, fy, sec-first_time_sec, usec);
      stm_unbuffer_float (&bufptr, &fscrap);
      printf ("%g, ", fscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      printf ("%g, ", fscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      printf ("%g, ", fscrap);
      stm_unbuffer_float (&bufptr, &fscrap);      
      printf ("%g)\n", fscrap);
      break;

    case SPM_SNAP_SHOT_BEGIN:
      stm_unbuffer_int (&bufptr, &x);
      stm_unbuffer_int (&bufptr, &y);
      printf ("SPM_SNAP_SHOT_BEGIN (%d, %d)\n", x, y);
      break;

    case SPM_SNAP_SHOT_END:
      printf ("SPM_SNAP_SHOT_END ()\n");
      break;

    case STM_SCAN_PARAMETERS:
      stm_unbuffer_int (&bufptr, &iscrap);
      printf ("STM_SCAN_PARAMETERS (%d)\n", iscrap);
      break;

    case SPM_BLUNT_RESULT_NM:
      stm_unbuffer_float (&bufptr, &fx);
      stm_unbuffer_float (&bufptr, &fy);
      stm_unbuffer_long (&bufptr, &sec);
      stm_unbuffer_long (&bufptr, &usec);
      printf ("SPM_BLUNT_RESULT_NM (%g, %g, %u:%u, ", 
	      fx, fy, sec-first_time_sec, usec);
      stm_unbuffer_float (&bufptr, &fscrap);
      printf ("%g, ", fscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      printf ("%g, ", fscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      printf ("%g, ", fscrap);
      stm_unbuffer_float (&bufptr, &fscrap);  
      printf ("%g)\n", fscrap);
      break;
      
    case SPM_WINDOW_LINE_DATA:
      stm_unbuffer_int (&bufptr, &x);
      stm_unbuffer_int (&bufptr, &y);
      stm_unbuffer_int (&bufptr, &dx);
      stm_unbuffer_int (&bufptr, &dy);
      stm_unbuffer_int (&bufptr, &reports);
      stm_unbuffer_int (&bufptr, &fields);
      stm_unbuffer_long (&bufptr, &sec);
      stm_unbuffer_long (&bufptr, &usec);
      printf ("SPM_WINDOW_LINE_DATA (%d, %d, %d, %d, %d, %d, %d:%d)\n",
	      x, y, dx, dy, reports, fields, sec-first_time_sec, usec);
      for (i=0; i<reports; i++) {
	printf ("  (");
        for (j=0; j<fields; j++) {
	  stm_unbuffer_float (&bufptr, &fscrap);
	  printf ("%g ", fscrap);
        }
	printf (")");
      }
      printf ("\n");
      break;

    case SPM_HELLO_MESSAGE:
      stm_unbuffer_chars (&bufptr, string, 4);
      printf ("SPM_HELLO_MESSAGE (%s, ", string);
      stm_unbuffer_chars (&bufptr, string, 64);
      printf ("%s, ", string);
      stm_unbuffer_int (&bufptr, &iscrap);
      printf ("%d, ", iscrap);
      stm_unbuffer_int (&bufptr, &iscrap);
      printf ("%d)\n", iscrap);
      break;
      
    case SPM_SCAN_DATASETS:
      stm_unbuffer_int (&bufptr, &iscrap);
      printf ("SPM_SCAN_DATASETS (%d)\n", iscrap);
      for (i=0; i<iscrap; i++) {
	stm_unbuffer_chars (&bufptr, string, 64);
	printf ("  (%s, ", string);
	stm_unbuffer_chars (&bufptr, string, 64);
	printf ("%s, ", string);
	stm_unbuffer_float (&bufptr, &fscrap);
	printf ("%g, ", fscrap);
	stm_unbuffer_float (&bufptr, &fscrap);
	printf ("%g)\n", fscrap);
      }
      break;

    case SPM_REPORT_SLOW_SCAN:
      stm_unbuffer_int (&bufptr, &iscrap);
      printf ("SPM_REPORT_SLOW_SCAN (%d)\n", iscrap);
      break;

    case SPM_CLIENT_HELLO:
      stm_unbuffer_chars (&bufptr, string, 4);
      printf ("SPM_CLIENT_HELLO (%s, ", string);
      stm_unbuffer_chars (&bufptr, string, 64);
      printf ("%s, ", string);
      stm_unbuffer_int (&bufptr, &iscrap);
      printf ("%d, ", iscrap);
      stm_unbuffer_int (&bufptr, &iscrap);
      printf ("%d)\n", iscrap);
      break;      
      
    case SPM_POINT_DATASETS:
      stm_unbuffer_int (&bufptr, &reports);
      printf ("SPM_POINT_DATASETS (%d)\n", reports);
      for (i=0; i<reports; i++) {
	stm_unbuffer_chars (&bufptr, string, 64);
	printf ("  (%s, ", string);
	stm_unbuffer_chars (&bufptr, string, 64);
	printf ("%s, ", string);
	stm_unbuffer_int (&bufptr, &iscrap);
	printf ("%d, ", iscrap);
	stm_unbuffer_float (&bufptr, &fscrap);
	printf ("%g, ", fscrap);
	stm_unbuffer_float (&bufptr, &fscrap);
	printf ("%g)\n", fscrap);
      }
      break;      

    case SPM_POINT_RESULT_DATA:
      stm_unbuffer_float (&bufptr, &fx);
      stm_unbuffer_float (&bufptr, &fy);
      stm_unbuffer_long (&bufptr, &sec);
      stm_unbuffer_long (&bufptr, &usec);
      stm_unbuffer_int (&bufptr, &reports);
      printf ("SPM_POINT_RESULT_DATA (%g, %g, %u:%u, %d)\n", 
	      fx, fy, sec-first_time_sec, usec, reports);
      printf ("  (");
      for (i=0; i<reports; i++) {
	stm_unbuffer_float (&bufptr, &fscrap);
	printf ("%g ", fscrap);
      }
      printf (")\n");
      break;

    case SPM_PID_PARAMETERS:
      stm_unbuffer_float (&bufptr, &P);
      stm_unbuffer_float (&bufptr, &I);
      stm_unbuffer_float (&bufptr, &D);
      printf ("SPM_PID_PARAMETERS (%g, %g, %g)\n", P, I, D);
      break;

    case SPM_SCANRATE_PARAMETER:
      stm_unbuffer_float (&bufptr, &fscrap);
      printf ("SPM_SCANRATE_PARAMETER (%g)\n", fscrap);
      break;

    case SPM_REPORT_GRID_SIZE:
      stm_unbuffer_int (&bufptr, &x);
      stm_unbuffer_int (&bufptr, &y);
      printf ("SPM_REPORT_GRID_SIZE (%d, %d)\n", x, y);
      break;
      
    case AFM_IN_SEWING_MODE:
      stm_unbuffer_float (&bufptr, &fscrap);
      printf ("AFM_IN_SEWING_MODE (%g, ", fscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      printf ("%g, ", fscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      printf ("%g, ", fscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      printf ("%g, ", fscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      printf ("%g, ", fscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      printf ("%g, ", fscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      printf ("%g)\n", fscrap);
      break;

    case AFM_IN_CONTACT_MODE:
      stm_unbuffer_float (&bufptr, &P);
      stm_unbuffer_float (&bufptr, &I);
      stm_unbuffer_float (&bufptr, &D);
      stm_unbuffer_float (&bufptr, &fscrap);
      printf("AFM_IN_CONTACT_MODE (%g, %g, %g, %g)\n", P, I, D, fscrap);
      break;      

    case AFM_IN_TAPPING_MODE:
      stm_unbuffer_float (&bufptr, &P);
      stm_unbuffer_float (&bufptr, &I);
      stm_unbuffer_float (&bufptr, &D);
      stm_unbuffer_float (&bufptr, &fscrap);
      printf ("AFM_IN_TAPPING_MODE (%g, %g, %g, %g, ", P, I, D, fscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      printf ("%g)\n", fscrap);
      break;            

    case SPM_BOTTOM_PUNCH_RESULT_DATA:
      stm_unbuffer_float (&bufptr, &fx);
      stm_unbuffer_float (&bufptr, &fy);
      stm_unbuffer_long (&bufptr, &sec);
      stm_unbuffer_long (&bufptr, &usec);
      stm_unbuffer_int (&bufptr, &reports);
      printf ("SPM_BOTTOM_PUNCH_RESULT_DATA (%g, %g, %u:%u, %d)\n", 
	      fx, fy, sec-first_time_sec, usec, reports);
      printf ("  (");
      for (i=0; i<reports; i++) {
	stm_unbuffer_float (&bufptr, &fscrap);
	printf ("%g ", fscrap);
      }
      printf (")\n");
      break;

    case SPM_TOP_PUNCH_RESULT_DATA:
      stm_unbuffer_float (&bufptr, &fx);
      stm_unbuffer_float (&bufptr, &fy);
      stm_unbuffer_long (&bufptr, &sec);
      stm_unbuffer_long (&bufptr, &usec);
      stm_unbuffer_int (&bufptr, &reports);
      printf ("SPM_TOP_PUNCH_RESULT_DATA (%g, %g, %u:%u, %d)\n", 
	      fx, fy, sec-first_time_sec, usec, reports);
      printf ("  (");
      for (i=0; i<reports; i++) {
	stm_unbuffer_float (&bufptr, &fscrap);
	printf ("%g ", fscrap);
      }
      printf (")\n");
      break;

    case SPM_VOLTSOURCE_ENABLED:
      stm_unbuffer_int (&bufptr, &iscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      printf ("SPM_VOLTSOURCE_ENABLED (%d, %g)\n", iscrap, fscrap);
      break;

    case SPM_VOLTSOURCE_DISABLED:
      stm_unbuffer_int (&bufptr, &iscrap);
      printf ("SPM_VOLTSOURCE_DISABLED (%d)\n", iscrap);
      break;

    case SPM_AMP_ENABLED:
      stm_unbuffer_int (&bufptr, &iscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      printf ("SPM_AMP_ENABLED (%d, %g, ", iscrap, fscrap);      
      stm_unbuffer_float (&bufptr, &fscrap);
      stm_unbuffer_int (&bufptr, &iscrap);
      printf ("%g, %d)\n", fscrap, iscrap);
      break;

    case SPM_AMP_DISABLED:
      stm_unbuffer_int (&bufptr, &iscrap);
      printf ("SPM_AMP_DISABLED (%d)\n", iscrap);
      break;

    case SPM_STARTING_TO_RELAX:
      stm_unbuffer_long (&bufptr, &sec);
      stm_unbuffer_long (&bufptr, &usec);
      printf ("SPM_STARTING_TO_RELAX (%u:%u)\n", sec-first_time_sec, usec);
      break;

    case SPM_REGION_ANGLE_SET:
      stm_unbuffer_float (&bufptr, &xmin);
      stm_unbuffer_float (&bufptr, &ymin);
      stm_unbuffer_float (&bufptr, &xmax);
      stm_unbuffer_float (&bufptr, &ymax);
      stm_unbuffer_float (&bufptr, &fscrap);
      printf ("SPM_REGION_ANGLE_SET (%g, %g, %g, %g, %g)\n",
	      xmin, ymin, xmax, ymax, fscrap);
      break;
      
    case SPM_REGION_ANGLE_CLIPPED:
      stm_unbuffer_float (&bufptr, &xmin);
      stm_unbuffer_float (&bufptr, &ymin);
      stm_unbuffer_float (&bufptr, &xmax);
      stm_unbuffer_float (&bufptr, &ymax);
      stm_unbuffer_float (&bufptr, &fscrap);
      printf ("SPM_REGION_ANGLE_CLIPPED (%g, %g, %g, %g, %g)\n",
	      xmin, ymin, xmax, ymax, fscrap);
      break;

    case SPM_TOPO_FILE_HEADER:
      stm_unbuffer_long (&bufptr, &lscrap);
      if (lscrap >= 5000) {
	fprintf (stderr, "header is too long -- %u bytes\n", lscrap);
	exit (-1);
      }
	
      stm_unbuffer_chars (&bufptr, header, lscrap);
      printf ("SPM_TOPO_FILE_HEADER (%u, %s)\n", lscrap, header);
      break;

    case SPM_SERVER_PACKET_TIMESTAMP:
      stm_unbuffer_long (&bufptr, &sec);
      stm_unbuffer_long (&bufptr, &usec);
      if (first_time_sec == 0) {
	 first_time_sec = sec;
      }
      printf ("SPM_SERVER_PACKET_TIMESTAMP (%u:%u)\n", sec-first_time_sec, usec);
      break;

    case OHM_RESISTANCE:
      stm_unbuffer_int (&bufptr, &iscrap);
      stm_unbuffer_long (&bufptr, &sec);
      stm_unbuffer_long (&bufptr, &usec);
      stm_unbuffer_float (&bufptr, &resist);
      stm_unbuffer_float (&bufptr, &voltage);
      stm_unbuffer_float (&bufptr, &range);
      stm_unbuffer_float (&bufptr, &filter);
      printf ("OHM_RESISTANCE (%d, %u:%u, %g, %g, %g, %g)\n", 
	      iscrap, sec-first_time_sec, usec, resist, voltage, range, filter);
      break;

    case SPM_FORCE_CURVE_DATA:
      stm_unbuffer_float (&bufptr, &fx);     
      stm_unbuffer_float (&bufptr, &fy);
      stm_unbuffer_long (&bufptr, &num_samples);
      stm_unbuffer_long (&bufptr, &num_halfcycles);
      stm_unbuffer_long (&bufptr, &sec);
      stm_unbuffer_long (&bufptr, &usec);
      printf("SPM_FORCE_CURVE_DATA (%d samples, %d halfcycles, %u:%u at %g,%g)\n",
	     num_samples, num_halfcycles, sec-first_time_sec,usec, fx, fy);
      for(long k = 0; k < num_samples; k++) { 
	 stm_unbuffer_float (&bufptr, &fscrap);
	 // z value
	 printf("%f -->", fscrap);
	 for (long m = 0; m < num_halfcycles; m++) {
	    stm_unbuffer_float (&bufptr, &fscrap);
	    printf(" %f", fscrap);
	 }
	 printf("\n");
      }
      break;

    case SPM_IN_SPECTROSCOPY_MODE:
      stm_unbuffer_float (&bufptr, &fscrap);
      printf("SPM_FORCE_CURVE_DATA (%f, bunch-o-parameters)\n", fscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      stm_unbuffer_long (&bufptr, &lscrap);
      stm_unbuffer_long (&bufptr, &lscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      stm_unbuffer_long (&bufptr, &lscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      
      break;

    case OHM_RESISTANCE_WSTATUS:
      stm_unbuffer_int (&bufptr, &iscrap);
      stm_unbuffer_long (&bufptr, &sec);
      stm_unbuffer_long (&bufptr, &usec);
      stm_unbuffer_float (&bufptr, &resist);
      stm_unbuffer_float (&bufptr, &voltage);
      stm_unbuffer_float (&bufptr, &range);
      stm_unbuffer_float (&bufptr, &filter);
	  stm_unbuffer_long (&bufptr, &status);
      printf ("OHM_RESISTANCE (%d, %u:%u, %g, %g, %g, %g, ",
		iscrap, sec-first_time_sec, usec, resist, voltage, range, filter);
	  switch(status) {
		case MEASURING: printf("VALID MEASUREMENT");
			break;
		case ITF_ERROR: printf("DEVICE IS BROKEN");
			break;
		case M_OVERFLO: printf("METER SATURATION");
			break;
		case M_UNDERFLO: printf("WARNING: RANGE LARGER THAN IDEAL");
			break;
		case R_OVERFLO: printf("WARNING: RANGE SMALLER THAN IDEAL");
			break;
		default:
			printf("THIS SHOULDN'T HAPPEN");
	  }
	  printf(")\n");
      break;

    /* Parameters echoed back by the server to record info in the stream */
    case AFM_BASE_MOD_PARAMETERS:
      stm_unbuffer_float (&bufptr, &baseforce); 
      stm_unbuffer_float (&bufptr, &modforce); 
      printf ("AFM_BASE_MOD_PARAMETERS (%g, %g)\n", 
	      baseforce, modforce);
      break;

    case SPM_VISIBLE_TRAIL:
      printf ("SPM_VISIBLE_TRAIL\n");
      break;

    case SPM_INVISIBLE_TRAIL:
      printf ("SPM_INVISIBLE_TRAIL\n");
      break;

    case AFM_FORCE_SETTINGS:
      stm_unbuffer_float (&bufptr, &base); 
      stm_unbuffer_float (&bufptr, &mode); 
      stm_unbuffer_float (&bufptr, &current);
      printf ("AFM_FORCE_SETTINGS (%g, %g, %g)\n", base, mode, 
	      current);
      break;

    case SPM_REFRESH_GRID:
      printf ("SPM_REFRESH_GRID\n");
      break;

    case NANO_RECV_TIMESTAMP:
      stm_unbuffer_long (&bufptr, &sec);
      stm_unbuffer_long (&bufptr, &usec);
      printf ("NANO_RECV_TIMESTAMP (%u:%u)\n", sec, usec);
      break;


    case FAKE_SPM_SEND_TIMESTAMP:
      stm_unbuffer_long (&bufptr, &sec);
      stm_unbuffer_long (&bufptr, &usec);
      printf ("FAKE_SPM_SEND_TIMESTAMP (%u:%u)\n", sec, usec);
      break;

    case SPM_UDP_SEQ_NUM:
      stm_unbuffer_long (&bufptr, &lscrap);
      printf ("SPM_UDP_SEQ_NUM (%u)\n", lscrap);
      break;

    default:
      fprintf(stderr, "=> Unknown data type returned (%d)\n", 
	      data_type);
      exit(-1);
    }
  }

  return(1);

}	/* translate_packet */


void	main(unsigned argc, char *argv[])
{
  int	ret;
  char	instream_name[255];
  stm_stream	*instream;

  /* Parse the command line */
  switch (argc) {
  case 2:
    strcpy(instream_name, argv[1]);
    break;
  default:
    Usage(argv[0]);
  }

  /* Open the input stream files */
  fprintf(stderr, "Reading the input stream file...\n");
  if ((instream = stm_open_datastream_for_read(instream_name)) == NULL){
    fprintf(stderr, "Couldn't open the input stream file\n");
    exit(-1);
  }

  /* Scan in the input file and translate to the output file until
   * the end of the input file is reached. */
  fprintf(stderr, "Converting the stream file...\n");
  while ( (ret = translate_packet(instream)) == 1) {};
  if (ret == -1) {
    fprintf(stderr,"Error during translation!\n");
  }

  /* Close the stream file */
  if (stm_close_stream(instream)) {
    fprintf(stderr, "Couldn't close the input stream file\n");
    exit(-1);
  }
}


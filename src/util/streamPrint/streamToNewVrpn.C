#include "vrpn_Ohmmeter.h"	// for ohmmeter status types
#include	<stdio.h>
#include	<sys/time.h>
#include	<sys/types.h>
#include	<errno.h>
#include	<math.h>

#include "stm_cmd.h"
#include "stm_file.h"
#include "functions.h"

// streamToNewVrpn
//
// Tom Hudson, May 1999
// Program takes a stm_cmd stream and translates it to the NEW
// VRPN message spec by Aron Helser.  (As of June 2000, we had
// not yet implemented this new type; this program is not the
// one to use to convert to the Nano version 9.0; use the program
// streamToVrpn instead.

// This is not very efficient;  we copy buffers over and over
// even when the format is unchanged because while copying we
// determine the length of the buffer.  Ugh.

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

// connection we're writing the output file on
vrpn_Connection * connection;

// now:  most recent time read in original stream
static struct timeval now;

vrpn_int32 myId;

vrpn_int32 InModifyMode_type;
vrpn_int32 InImageMode_type;
vrpn_int32 ReportRelaxTimes_type;
vrpn_int32 WindowLineData_type;
vrpn_int32 HelloMessage_type;
vrpn_int32 ReportScanDatasets_type;
vrpn_int32 ReportPointDatasets_type;
vrpn_int32 PointData_type;
vrpn_int32 ReportPID_type;
vrpn_int32 ReportScanrateNM_type;
vrpn_int32 ReportGridSize_type;
vrpn_int32 InSewingStyle_type;
vrpn_int32 InContactMode_type;
vrpn_int32 InOscillatingMode_type;
vrpn_int32 StartingToRelax_type;
vrpn_int32 TopoFileHeader_type;
vrpn_int32 ForceCurveData_type;
vrpn_int32 InForceCurveStyle_type;


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
  vrpn_int32	x,y;		/* Position of current read */
  vrpn_int32 dx, dy;
  vrpn_float32	fx,fy;		/* Position as a float */
  vrpn_int32	sec,usec;	/* When the value was obtained */
  vrpn_float32	value;		/* Value at this point */
  vrpn_float32	std_dev;	/* Standard deviation of value */
  vrpn_float32	xmin,xmax, ymin,ymax, zmin,zmax;
  vrpn_float32	fmin, fmax, fcur;
  vrpn_int32	enabled;
  vrpn_float32 baseforce, modforce, base, mode, current;

  static	char    buffer[MAXBUF];   /* Holds data from the client */
  int     bufsize;        /* Number of characters read */
  char    *bufptr;        /* Current location in buffer */
  static	char	outbuffer[2*MAXBUF];	/* Holds output data */
  char	*outbufptr;

  char header[5000];
  int iscrap;
  vrpn_int32 lscrap;
  float	fscrap;		/* Scrap variable */
  int reports, fields;
  int i, j;
  char string[100];
  float P, I, D;
  // ohmmeter resistance variables;
  float resist, voltage, range, filter;
  long status;
  // force curve
  vrpn_int32 num_samples, num_halfcycles;

  // output buffer to VRPN log
  char vrpnbuffer [10000];
  char * vbp;
  vrpn_int32 vbuflen;

  //printf ("\n** msg #%u:\n", ++seqNum);

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

    vbp = vrpnbuffer;
    vbuflen = 10000;

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
      //stm_unbuffer_long (&bufptr, &sec);
      //stm_unbuffer_long (&bufptr, &usec);
      stm_unbuffer_int (&bufptr, &sec);
      stm_unbuffer_int (&bufptr, &usec);
      stm_unbuffer_float (&bufptr, &value);
      stm_unbuffer_float (&bufptr, &std_dev);
      printf("STM_WINDOW_SCAN_NM (%d,%d, %u:%u, %g,%g)\n",
	     x,y, sec-first_time_sec,usec, value,std_dev);
      break;

    case STM_WINDOW_BACKSCAN_NM: 
      //      printf ("STM_WINDOW_BACKSCAN_NM --> timestamped\n");
      stm_unbuffer_int (&bufptr, &x);     
      stm_unbuffer_int (&bufptr, &y);
      //stm_unbuffer_long (&bufptr, &sec);
      //stm_unbuffer_long (&bufptr, &usec);
      stm_unbuffer_int (&bufptr, &sec);
      stm_unbuffer_int (&bufptr, &usec);
      stm_unbuffer_float (&bufptr, &value);
      stm_unbuffer_float (&bufptr, &std_dev);
      printf("STM_WINDOW_BACKSCAN_NM (%d,%d, %u:%u, %g,%g)\n",
	     x,y, sec-first_time_sec,usec, value,std_dev);
      break;

    case STM_POINT_RESULT_NM: 
      stm_unbuffer_float (&bufptr, &fx);     
      stm_unbuffer_float (&bufptr, &fy);
      //stm_unbuffer_long (&bufptr, &sec);
      //stm_unbuffer_long (&bufptr, &usec);
      stm_unbuffer_int (&bufptr, &sec);
      stm_unbuffer_int (&bufptr, &usec);
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
      vrpn_buffer(&vbp, &vbuflen, xmin);
      stm_unbuffer_float (&bufptr, &xmax);
      vrpn_buffer(&vbp, &vbuflen, xmax);
      stm_unbuffer_float (&bufptr, &ymin);
      vrpn_buffer(&vbp, &vbuflen, ymin);
      stm_unbuffer_float (&bufptr, &ymax);
      vrpn_buffer(&vbp, &vbuflen, ymax);
      stm_unbuffer_float (&bufptr, &zmin);
      vrpn_buffer(&vbp, &vbuflen, zmin);
      stm_unbuffer_float (&bufptr, &zmax);
      vrpn_buffer(&vbp, &vbuflen, zmax);
      connection->pack_message(10000 - vbuflen, now, ReportMaxScanRangeNM_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);
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
      connection->pack_message(10000 - vbuflen, now, InModifyMode_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);
      break;

    case AFM_IN_IMG_MODE: 
      connection->pack_message(10000 - vbuflen, now, InImageMode_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);
      break;

    case SPM_RELAX_SET: 
      stm_unbuffer_int (&bufptr, &x); 
      vrpn_buffer(&vbp, &vbuflen, x);
      stm_unbuffer_int (&bufptr, &y); 
      vrpn_buffer(&vbp, &vbuflen, y);
      connection->pack_message(10000 - vbuflen, now, ReportRelaxTimes_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);
      break;

    case AFM_IN_MOD_MODE_T: 
      //stm_unbuffer_long (&bufptr, &sec);
      stm_unbuffer_int (&bufptr, &sec);
      vrpn_buffer(&vbp, &vbuflen, sec);
      //stm_unbuffer_long (&bufptr, &usec);
      stm_unbuffer_int (&bufptr, &usec);
      vrpn_buffer(&vbp, &vbuflen, usec);
      connection->pack_message(10000 - vbuflen, now, InModifyMode_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);
      break;

    case AFM_IN_IMG_MODE_T: 
      //stm_unbuffer_long (&bufptr, &sec);
      stm_unbuffer_int (&bufptr, &sec);
      vrpn_buffer(&vbp, &vbuflen, sec);
      //stm_unbuffer_long (&bufptr, &usec);
      stm_unbuffer_int (&bufptr, &usec);
      vrpn_buffer(&vbp, &vbuflen, usec);
      connection->pack_message(10000 - vbuflen, now, InImageMode_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);
      break;

    case SPM_RESISTANCE:
      //stm_unbuffer_long (&bufptr, &lscrap);
      stm_unbuffer_int (&bufptr, &sec);
      //stm_unbuffer_long (&bufptr, &sec);
      stm_unbuffer_int (&bufptr, &sec);
      //stm_unbuffer_long (&bufptr, &usec);
      stm_unbuffer_int (&bufptr, &usec);
      stm_unbuffer_float (&bufptr, &fscrap);
      printf ("SPM_RESISTANCE (%u, %u:%u, %g)\n", 
              lscrap, sec-first_time_sec, usec, fscrap);
      break;

    case SPM_RESISTANCE_FAILURE:
      //stm_unbuffer_long (&bufptr, &lscrap);
      stm_unbuffer_int (&bufptr, &lscrap);
      printf ("SPM_RESISTANCE_FAILURE (%u)\n", lscrap);
      break;
      
    case STM_ZIG_RESULT_NM:
      printf ("STM_ZIG_RESULT_NM (");
      stm_unbuffer_float (&bufptr, &fx);
      stm_unbuffer_float (&bufptr, &fy);
      //stm_unbuffer_long (&bufptr, &sec);
      stm_unbuffer_int (&bufptr, &sec);
      //stm_unbuffer_long (&bufptr, &usec);
      stm_unbuffer_int (&bufptr, &usec);
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
      //stm_unbuffer_long (&bufptr, &sec);
      stm_unbuffer_int (&bufptr, &sec);
      //stm_unbuffer_long (&bufptr, &usec);
      stm_unbuffer_int (&bufptr, &usec);
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
      vrpn_buffer(&vbp, &vbuflen, x);
      stm_unbuffer_int (&bufptr, &y);
      vrpn_buffer(&vbp, &vbuflen, y);
      stm_unbuffer_int (&bufptr, &dx);
      vrpn_buffer(&vbp, &vbuflen, dx);
      stm_unbuffer_int (&bufptr, &dy);
      vrpn_buffer(&vbp, &vbuflen, dy);
      stm_unbuffer_int (&bufptr, &reports);
      vrpn_buffer(&vbp, &vbuflen, reports);
      stm_unbuffer_int (&bufptr, &fields);
      vrpn_buffer(&vbp, &vbuflen, fields);
      //stm_unbuffer_long (&bufptr, &sec);
      stm_unbuffer_int (&bufptr, &sec);
      vrpn_buffer(&vbp, &vbuflen, sec);
      //stm_unbuffer_long (&bufptr, &usec);
      stm_unbuffer_int (&bufptr, &usec);
      vrpn_buffer(&vbp, &vbuflen, usec);
      for (i = 0; i < reports; i++) {
        for (j = 0; j < fields; j++) {
	  stm_unbuffer_float (&bufptr, &fscrap);
          vrpn_buffer(&vbp, &vbuflen, fscrap);
        }
      }
      connection->pack_message(10000 - vbuflen, now, WindowLineData_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);
      break;

    case SPM_HELLO_MESSAGE:
      stm_unbuffer_chars (&bufptr, string, 4);
      vrpn_buffer(&vbp, &vbuflen, string, 4);
      stm_unbuffer_chars (&bufptr, string, 64);
      vrpn_buffer(&vbp, &vbuflen, string, 64);
      stm_unbuffer_int (&bufptr, &iscrap);
      vrpn_buffer(&vbp, &vbuflen, iscrap);
      stm_unbuffer_int (&bufptr, &iscrap);
      vrpn_buffer(&vbp, &vbuflen, iscrap);

      connection->pack_message(10000 - vbuflen, now, HelloMessage_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);
      break;
      
    case SPM_SCAN_DATASETS:
      stm_unbuffer_int (&bufptr, &iscrap);
      vrpn_buffer(&vbp, &vbuflen, iscrap);
      for (i = 0; i < iscrap; i++) {
	stm_unbuffer_chars (&bufptr, string, 64);
        vrpn_buffer(&vbp, &vbuflen, string, 64);
	stm_unbuffer_chars (&bufptr, string, 64);
        vrpn_buffer(&vbp, &vbuflen, string, 64);
	stm_unbuffer_float (&bufptr, &fscrap);
        vrpn_buffer(&vbp, &vbuflen, fscrap);
	stm_unbuffer_float (&bufptr, &fscrap);
        vrpn_buffer(&vbp, &vbuflen, fscrap);
      }
      connection->pack_message(10000 - vbuflen, now, ReportScanDatasets_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);
      break;

    case SPM_REPORT_SLOW_SCAN:
      stm_unbuffer_int (&bufptr, &iscrap);
      printf("SPM_REPORT_SLOW_SCAN (%d) =>", iscrap);
      if (iscrap) {
        connection->pack_message(10000 - vbuflen, now, SlowScanPause_type,
                                 myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);
        printf("SlowScanPause ()\n");
      } else {
        connection->pack_message(10000 - vbuflen, now, SlowScanResume_type,
                                 myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);
        printf("SlowScanResume ()\n");
      }
      break;

    case SPM_CLIENT_HELLO:
      printf ("Found SPM_CLIENT_HELLO;  issuing HelloMessage.\n");
      stm_unbuffer_chars (&bufptr, string, 4);
      vrpn_buffer(&vbp, &vbuflen, string, 4);
      stm_unbuffer_chars (&bufptr, string, 64);
      vrpn_buffer(&vbp, &vbuflen, string, 64);
      stm_unbuffer_int (&bufptr, &iscrap);
      vrpn_buffer(&vbp, &vbuflen, iscrap);
      stm_unbuffer_int (&bufptr, &iscrap);
      vrpn_buffer(&vbp, &vbuflen, iscrap);

      connection->pack_message(10000 - vbuflen, now, HelloMessage_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);
      break;      
      
    case SPM_POINT_DATASETS:
      stm_unbuffer_int (&bufptr, &reports);
      vrpn_buffer(&vbp, &vbuflen, reports);
      for (i = 0; i < reports; i++) {
	stm_unbuffer_chars (&bufptr, string, 64);
        vrpn_buffer(&vbp, &vbuflen, string, 64);
	stm_unbuffer_chars (&bufptr, string, 64);
        vrpn_buffer(&vbp, &vbuflen, string, 64);
	stm_unbuffer_int (&bufptr, &iscrap);
        vrpn_buffer(&vbp, &vbuflen, iscrap);
	stm_unbuffer_float (&bufptr, &fscrap);
        vrpn_buffer(&vbp, &vbuflen, fscrap);
	stm_unbuffer_float (&bufptr, &fscrap);
        vrpn_buffer(&vbp, &vbuflen, fscrap);
      }
      connection->pack_message(10000 - vbuflen, now, ReportPointDatasets_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);
      break;      

    case SPM_POINT_RESULT_DATA:
      stm_unbuffer_float (&bufptr, &fx);
      vrpn_buffer(&vbp, &vbuflen, fx);
      stm_unbuffer_float (&bufptr, &fy);
      vrpn_buffer(&vbp, &vbuflen, fy);
      //stm_unbuffer_long (&bufptr, &sec);
      stm_unbuffer_int (&bufptr, &sec);
      vrpn_buffer(&vbp, &vbuflen, sec);
      //stm_unbuffer_long (&bufptr, &usec);
      stm_unbuffer_int (&bufptr, &usec);
      vrpn_buffer(&vbp, &vbuflen, usec);
      stm_unbuffer_int (&bufptr, &reports);
      vrpn_buffer(&vbp, &vbuflen, reports);
      for (i = 0; i < reports; i++) {
	stm_unbuffer_float (&bufptr, &fscrap);
        vrpn_buffer(&vbp, &vbuflen, fscrap);
      }
      connection->pack_message(10000 - vbuflen, now, PointData_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);
      break;

    case SPM_PID_PARAMETERS:
      stm_unbuffer_float (&bufptr, &P);
      vrpn_buffer(&vbp, &vbuflen, P);
      stm_unbuffer_float (&bufptr, &I);
      vrpn_buffer(&vbp, &vbuflen, I);
      stm_unbuffer_float (&bufptr, &D);
      vrpn_buffer(&vbp, &vbuflen, D);

      connection->pack_message(10000 - vbuflen, now, ReportPID_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);
      break;

    case SPM_SCANRATE_PARAMETER:
      stm_unbuffer_float (&bufptr, &fscrap);
      vrpn_buffer(&vbp, &vbuflen, fscrap);
      connection->pack_message(10000 - vbuflen, now, ReportScanrateNM_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);
      break;

    case SPM_REPORT_GRID_SIZE:
      stm_unbuffer_int (&bufptr, &x);
      vrpn_buffer(&vbp, &vbuflen, x);
      stm_unbuffer_int (&bufptr, &y);
      vrpn_buffer(&vbp, &vbuflen, y);
      connection->pack_message(10000 - vbuflen, now, ReportGridSize_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);
      break;
      
    case AFM_IN_SEWING_MODE:
      stm_unbuffer_float (&bufptr, &fscrap);
      vrpn_buffer(&vbp, &vbuflen, fscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      vrpn_buffer(&vbp, &vbuflen, fscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      vrpn_buffer(&vbp, &vbuflen, fscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      vrpn_buffer(&vbp, &vbuflen, fscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      vrpn_buffer(&vbp, &vbuflen, fscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      vrpn_buffer(&vbp, &vbuflen, fscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      vrpn_buffer(&vbp, &vbuflen, fscrap);
      connection->pack_message(10000 - vbuflen, now, InSewingStyle_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);
      break;

    case AFM_IN_CONTACT_MODE:
      stm_unbuffer_float (&bufptr, &P);
      vrpn_buffer(&vbp, &vbuflen, P);
      stm_unbuffer_float (&bufptr, &I);
      vrpn_buffer(&vbp, &vbuflen, I);
      stm_unbuffer_float (&bufptr, &D);
      vrpn_buffer(&vbp, &vbuflen, D);
      stm_unbuffer_float (&bufptr, &fscrap);
      vrpn_buffer(&vbp, &vbuflen, fscrap);
      connection->pack_message(10000 - vbuflen, now, InContactMode_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);
      break;      

    case AFM_IN_TAPPING_MODE:
      stm_unbuffer_float (&bufptr, &P);
      vrpn_buffer(&vbp, &vbuflen, P);
      stm_unbuffer_float (&bufptr, &I);
      vrpn_buffer(&vbp, &vbuflen, I);
      stm_unbuffer_float (&bufptr, &D);
      vrpn_buffer(&vbp, &vbuflen, D);
      stm_unbuffer_float (&bufptr, &fscrap);
      vrpn_buffer(&vbp, &vbuflen, fscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      vrpn_buffer(&vbp, &vbuflen, fscrap);
      connection->pack_message(10000 - vbuflen, now, InOscillatingMode_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);
      break;            

    case SPM_BOTTOM_PUNCH_RESULT_DATA:
      stm_unbuffer_float (&bufptr, &fx);
      stm_unbuffer_float (&bufptr, &fy);
      //stm_unbuffer_long (&bufptr, &sec);
      //stm_unbuffer_long (&bufptr, &usec);
      stm_unbuffer_int (&bufptr, &sec);
      stm_unbuffer_int (&bufptr, &usec);
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
      //stm_unbuffer_long (&bufptr, &sec);
      //stm_unbuffer_long (&bufptr, &usec);
      stm_unbuffer_int (&bufptr, &sec);
      stm_unbuffer_int (&bufptr, &usec);
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
      //stm_unbuffer_long (&bufptr, &sec);
      stm_unbuffer_int (&bufptr, &sec);
      vrpn_buffer(&vbp, &vbuflen, sec);
      //stm_unbuffer_long (&bufptr, &usec);
      stm_unbuffer_int (&bufptr, &usec);
      vrpn_buffer(&vbp, &vbuflen, usec);
      connection->pack_message(10000 - vbuflen, now, StartingToRelax_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);
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
      //stm_unbuffer_long (&bufptr, &lscrap);
      stm_unbuffer_int (&bufptr, &lscrap);
      if (lscrap >= 5000) {
	fprintf (stderr, "header is too long -- %u bytes\n", lscrap);
	exit (-1);
      }
	
      vrpn_buffer(&vbp, &vbuflen, lscrap);
      stm_unbuffer_chars (&bufptr, header, lscrap);
      vrpn_buffer(&vbp, &vbuflen, header, lscrap);
      connection->pack_message(10000 - vbuflen, now, TopoFileHeader_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);
      break;

    case SPM_SERVER_PACKET_TIMESTAMP:
      //stm_unbuffer_long (&bufptr, &sec);
      //stm_unbuffer_long (&bufptr, &usec);
      if (first_time_sec == 0) {
	 first_time_sec = sec;
      }
      printf ("SPM_SERVER_PACKET_TIMESTAMP (%u:%u)\n", sec-first_time_sec, usec);
      break;

    case OHM_RESISTANCE:
      stm_unbuffer_int (&bufptr, &iscrap);
      //stm_unbuffer_long (&bufptr, &sec);
      //stm_unbuffer_long (&bufptr, &usec);
      stm_unbuffer_float (&bufptr, &resist);
      stm_unbuffer_float (&bufptr, &voltage);
      stm_unbuffer_float (&bufptr, &range);
      stm_unbuffer_float (&bufptr, &filter);
      printf ("OHM_RESISTANCE (%d, %u:%u, %g, %g, %g, %g)\n", 
	      iscrap, sec-first_time_sec, usec, resist, voltage, range, filter);
      break;

    case SPM_FORCE_CURVE_DATA:
      stm_unbuffer_float (&bufptr, &fx);     
      vrpn_buffer(&vbp, &vbuflen, fx);
      stm_unbuffer_float (&bufptr, &fy);
      vrpn_buffer(&vbp, &vbuflen, fy);
      //stm_unbuffer_long (&bufptr, &num_samples);
      stm_unbuffer_int (&bufptr, &num_samples);
      vrpn_buffer(&vbp, &vbuflen, num_samples);
      //stm_unbuffer_long (&bufptr, &num_halfcycles);
      stm_unbuffer_int (&bufptr, &num_halfcycles);
      vrpn_buffer(&vbp, &vbuflen, num_halfcycles);
      //stm_unbuffer_long (&bufptr, &sec);
      stm_unbuffer_int (&bufptr, &sec);
      vrpn_buffer(&vbp, &vbuflen, sec);
      //stm_unbuffer_long (&bufptr, &usec);
      stm_unbuffer_int (&bufptr, &usec);
      vrpn_buffer(&vbp, &vbuflen, usec);
      for (i = 0; i < num_samples; i++) { 
	 stm_unbuffer_float (&bufptr, &fscrap);
         vrpn_buffer(&vbp, &vbuflen, fscrap);
	 for (j = 0; j < num_halfcycles; j++) {
	    stm_unbuffer_float (&bufptr, &fscrap);
            vrpn_buffer(&vbp, &vbuflen, fscrap);
	 }
      }
      connection->pack_message(10000 - vbuflen, now, ForceCurveData_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);
      break;

    case SPM_IN_SPECTROSCOPY_MODE:
      stm_unbuffer_float (&bufptr, &fscrap);
      vrpn_buffer(&vbp, &vbuflen, fscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      vrpn_buffer(&vbp, &vbuflen, fscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      vrpn_buffer(&vbp, &vbuflen, fscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      vrpn_buffer(&vbp, &vbuflen, fscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      vrpn_buffer(&vbp, &vbuflen, fscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      vrpn_buffer(&vbp, &vbuflen, fscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      vrpn_buffer(&vbp, &vbuflen, fscrap);
      //stm_unbuffer_long (&bufptr, &lscrap);
      stm_unbuffer_int (&bufptr, &lscrap);
      vrpn_buffer(&vbp, &vbuflen, lscrap);
      //stm_unbuffer_long (&bufptr, &lscrap);
      stm_unbuffer_int (&bufptr, &lscrap);
      vrpn_buffer(&vbp, &vbuflen, lscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      vrpn_buffer(&vbp, &vbuflen, fscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      vrpn_buffer(&vbp, &vbuflen, fscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      vrpn_buffer(&vbp, &vbuflen, fscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      vrpn_buffer(&vbp, &vbuflen, fscrap);
      //stm_unbuffer_long (&bufptr, &lscrap);
      stm_unbuffer_int (&bufptr, &lscrap);
      vrpn_buffer(&vbp, &vbuflen, lscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      vrpn_buffer(&vbp, &vbuflen, fscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      vrpn_buffer(&vbp, &vbuflen, fscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      vrpn_buffer(&vbp, &vbuflen, fscrap);
      connection->pack_message(10000 - vbuflen, now, InForceCurveStyle_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);
      
      break;

    case OHM_RESISTANCE_WSTATUS:
      //stm_unbuffer_int (&bufptr, &iscrap);
      //stm_unbuffer_long (&bufptr, &sec);
      //stm_unbuffer_long (&bufptr, &usec);
      //stm_unbuffer_float (&bufptr, &resist);
      //stm_unbuffer_float (&bufptr, &voltage);
      //stm_unbuffer_float (&bufptr, &range);
      //stm_unbuffer_float (&bufptr, &filter);
	  //stm_unbuffer_long (&bufptr, &status);
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
      //stm_unbuffer_float (&bufptr, &base); 
      //stm_unbuffer_float (&bufptr, &mode); 
      //stm_unbuffer_float (&bufptr, &current);
      printf ("AFM_FORCE_SETTINGS (%g, %g, %g)\n", base, mode, 
	      current);
      break;

    case SPM_REFRESH_GRID:
      printf ("SPM_REFRESH_GRID\n");
      break;

    case NANO_RECV_TIMESTAMP:
      //stm_unbuffer_long (&bufptr, &sec);
      //stm_unbuffer_long (&bufptr, &usec);
      printf ("NANO_RECV_TIMESTAMP (%u:%u)\n", sec, usec);
      break;


    case FAKE_SPM_SEND_TIMESTAMP:
      //stm_unbuffer_long (&bufptr, &sec);
      //stm_unbuffer_long (&bufptr, &usec);
      printf ("FAKE_SPM_SEND_TIMESTAMP (%u:%u)\n", sec, usec);
      break;

    case SPM_UDP_SEQ_NUM:
      //stm_unbuffer_long (&bufptr, &lscrap);
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
  char	outstream_name[255];
  stm_stream	*instream;

  /* Parse the command line */
  switch (argc) {
  case 3:
    strcpy(instream_name, argv[1]);
    strcpy(outstream_name, argv[2]);
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

  // don't connect to anything, just log
  connection = new vrpn_Synchronized_Connection
    ("localhost", 4500, outstream_name, vrpn_LOG_OUTGOING);

  myId = connection->register_sender("nmm_Microscope");

  // register types
  InModifyMode_type =
      connection->register_message_type("InModifyMode");
  InImageMode_type =
      connection->register_message_type("InImageMode");
  ReportRelaxTimes_type =
      connection->register_message_type("ReportRelaxTimes");
  WindowLineData_type =
      connection->register_message_type("WindowLineData");
  HelloMessage_type =
      connection->register_message_type("HelloMessage");
  ReportScanDatasets_type =
      connection->register_message_type("ReportScanDatasets");
  ReportPointDatasets_type =
      connection->register_message_type("ReportPointDatasets");
  PointData_type =
      connection->register_message_type("PointData");
  ReportPID_type =
      connection->register_message_type("ReportPID");
  ReportScanrateNM_type =
      connection->register_message_type("ReportScanrateNM");
  ReportGridSize_type =
      connection->register_message_type("ReportGridSize");
  InSewingStyle_type =
      connection->register_message_type("InSewingStyle");
  InContactMode_type =
      connection->register_message_type("InContactMode");
  InOscillatingMode_type =
      connection->register_message_type("InOscillatingMode");
  StartingToRelax_type =
      connection->register_message_type("StartingToRelax");
  TopoFileHeader_type =
      connection->register_message_type("TopoFileHeader");
  ForceCurveData_type =
      connection->register_message_type("ForceCurveData");
  InForceCurveStyle_type =
      connection->register_message_type("InForceCurveStyle");


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

  delete connection;
}


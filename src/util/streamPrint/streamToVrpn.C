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
// VRPN message spec by Aron Helser.

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

vrpn_int32 RelaxSet_type;
vrpn_int32 EnableVoltsource_type;
vrpn_int32 DisableVoltsource_type;
vrpn_int32 EnableAmp_type;
vrpn_int32 DisableAmp_type;
vrpn_int32 StartingToRelax_type;
vrpn_int32 RecordResistance_type;

vrpn_int32 WindowLineData_type;
vrpn_int32 WindowScanNM_type;
vrpn_int32 WindowBackscanNM_type;
vrpn_int32 PointResultNM_type;
vrpn_int32 PointResultData_type;
vrpn_int32 BottomPunchResultData_type;
vrpn_int32 TopPunchResultData_type;
vrpn_int32 ZigResultNM_type;
vrpn_int32 BluntResultNM_type;
vrpn_int32 ScanRange_type;
vrpn_int32 SetRegionCompleted_type;
vrpn_int32 SetRegionClipped_type;
vrpn_int32 ResistanceFailure_type;
vrpn_int32 Resistance_type;
vrpn_int32 Resistance2_type;
vrpn_int32 ReportSlowScan_type;
vrpn_int32 ScanParameters_type;
vrpn_int32 HelloMessage_type;
vrpn_int32 ClientHello_type;
vrpn_int32 ScanDataset_type;
vrpn_int32 PointDataset_type;
vrpn_int32 PidParameters_type;
vrpn_int32 ScanrateParameter_type;
vrpn_int32 ReportGridSize_type;
vrpn_int32 ServerPacketTimestamp_type;
vrpn_int32 TopoFileHeader_type;
vrpn_int32 ForceCurveData_type;

vrpn_int32 MaxSetpointExceeded_type;

vrpn_int32 RecvTimestamp_type;
vrpn_int32 FakeSendTimestamp_type;
vrpn_int32 UpSeqNum_type;

vrpn_int32 InTappingMode_type;
vrpn_int32 InOscillatingMode_type;
vrpn_int32 InContactMode_type;
vrpn_int32 InDirectZControl_type;
vrpn_int32 InSewingStyle_type;
vrpn_int32 InSpectroscopyMode_type;
vrpn_int32 ForceParameters_type;
vrpn_int32 BaseModParameters_type;
vrpn_int32 ForceSettings_type;
vrpn_int32 InModModeT_type;
vrpn_int32 InImgModeT_type;
vrpn_int32 InModeMode_type;
vrpn_int32 InImgMode_type;
vrpn_int32 ModForceSet_type;
vrpn_int32 ImgForceSet_type;
vrpn_int32 ModForceSetFailure_type;
vrpn_int32 ImgForceSetFailure_type;
vrpn_int32 ModSet_type;
vrpn_int32 ImgSet_type;
vrpn_int32 ForceSet_type;
vrpn_int32 ForceSetFailure_type;
vrpn_int32 SmapleApproach_type;
vrpn_int32 SetBias_type;
vrpn_int32 SampleApproachNM_type;
vrpn_int32 SetPulsePeak_type;
vrpn_int32 SetPulseDuration_type;
vrpn_int32 PulsePoint_type;
vrpn_int32 PulsePointNM_type;

vrpn_int32 PulseParameters_type;
vrpn_int32 PulseCompletedNM_type;
vrpn_int32 PulseFailureNM_type;
vrpn_int32 PulseCompleted_type;
vrpn_int32 PulseFailure_type;
vrpn_int32 TunnellingAttained_type;
vrpn_int32 TunnellingAttainedNM_type;
vrpn_int32 TunnellingFailure_type;
vrpn_int32 ApproachComplete_type;

void Usage(char *s)
{
  fprintf(stderr,"Usage: %s instream outputfile\n",s);
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

  char header[5000];
  vrpn_int32 iscrap;
  vrpn_int32 lscrap;
  vrpn_float32	fscrap;		/* Scrap variable */
  vrpn_int32 reports, fields;
  vrpn_int32 i, j;
  char string[100];
  vrpn_float32 P, I, D;
  // ohmmeter resistance variables;
  vrpn_float32 resist, voltage, range, filter;
  vrpn_int32 status;
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
      vrpn_buffer (&vbp, &vbuflen, pulse_enabled);
      stm_unbuffer_float (&bufptr, &pulse_bias);
      vrpn_buffer (&vbp, &vbuflen, pulse_bias);
      stm_unbuffer_float (&bufptr, &pulse_peak);
      vrpn_buffer (&vbp, &vbuflen, pulse_peak);
      stm_unbuffer_float (&bufptr, &pulse_width);
      vrpn_buffer (&vbp, &vbuflen, pulse_width);
      connection->pack_message(10000 - vbuflen, now, PulseParameters_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);

      break;

    case STM_STD_DEV_PARAMETERS:
      stm_unbuffer_int (&bufptr, &std_dev_samples);
      stm_unbuffer_float (&bufptr, &std_dev_frequency);
      printf ("STM_STD_DEV_PARAMETERS (%u, %g)\n",
	      std_dev_samples, std_dev_frequency);	 
     
      break;

    case AFM_FORCE_SET:
      stm_unbuffer_float (&bufptr, &fscrap);
      vrpn_buffer(&vbp, &vbuflen, fscrap);
      connection->pack_message(10000 - vbuflen, now, ForceSet_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);
   
      break;

    case AFM_FORCE_SET_FAILURE:
      stm_unbuffer_float (&bufptr, &fscrap);
      vrpn_buffer(&vbp, &vbuflen, fscrap);
      connection->pack_message(10000 - vbuflen, now, ForceSetFailure_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);

      break;

    case AFM_FORCE_PARAMETERS: 
      stm_unbuffer_int (&bufptr, &fmods_enabled);
      vrpn_buffer (&vbp, &vbuflen, fmods_enabled);
      stm_unbuffer_float (&bufptr, &fmods_baseforce);
      vrpn_buffer (&vbp, &vbuflen, fmods_baseforce);
      connection->pack_message(10000 - vbuflen, now, ForceParameters_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);

      break;

    case STM_WINDOW_SCAN_NM: 
      stm_unbuffer_int (&bufptr, &x);
      vrpn_buffer (&vbp, &vbuflen, x); 
      stm_unbuffer_int (&bufptr, &y);
      vrpn_buffer (&vbp, &vbuflen, y);
      //stm_unbuffer_long (&bufptr, &sec);
      //stm_unbuffer_long (&bufptr, &usec);
      stm_unbuffer_int (&bufptr, &sec);
      vrpn_buffer (&vbp, &vbuflen, sec);
      stm_unbuffer_int (&bufptr, &usec);
      vrpn_buffer (&vbp, &vbuflen, usec);
      stm_unbuffer_float (&bufptr, &value);
      vrpn_buffer (&vbp, &vbuflen, value);
      stm_unbuffer_float (&bufptr, &std_dev);
      vrpn_buffer (&vbp, &vbuflen, std_dev);
      if (first_time_sec == 0) {
	 first_time_sec = sec;
      }
      now.tv_sec = sec - first_time_sec;
      now.tv_usec = usec;
      connection->pack_message(10000 - vbuflen, now, WindowScanNM_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);

      break;

    case STM_WINDOW_BACKSCAN_NM: 
      //      printf ("STM_WINDOW_BACKSCAN_NM --> timestamped\n");
      stm_unbuffer_int (&bufptr, &x);
      vrpn_buffer (&vbp, &vbuflen, x);
      stm_unbuffer_int (&bufptr, &y);
      vrpn_buffer (&vbp, &vbuflen, y);
      //stm_unbuffer_long (&bufptr, &sec);
      //stm_unbuffer_long (&bufptr, &usec);
      stm_unbuffer_int (&bufptr, &sec);
      vrpn_buffer (&vbp, &vbuflen, sec);
      stm_unbuffer_int (&bufptr, &usec);
      vrpn_buffer (&vbp, &vbuflen, usec);
      stm_unbuffer_float (&bufptr, &value);
      vrpn_buffer (&vbp, &vbuflen, value);
      stm_unbuffer_float (&bufptr, &std_dev);
      vrpn_buffer (&vbp, &vbuflen, std_dev);
      if (first_time_sec == 0) {
	 first_time_sec = sec;
      }
      now.tv_sec = sec - first_time_sec;
      now.tv_usec = usec;
      connection->pack_message(10000 - vbuflen, now, WindowBackscanNM_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);

      break;

    case STM_POINT_RESULT_NM: 
      stm_unbuffer_float (&bufptr, &fx);
      vrpn_buffer (&vbp, &vbuflen, fx);
      stm_unbuffer_float (&bufptr, &fy);
      vrpn_buffer (&vbp, &vbuflen, fy);
      //stm_unbuffer_long (&bufptr, &sec);
      //stm_unbuffer_long (&bufptr, &usec);
      stm_unbuffer_int (&bufptr, &sec);
      vrpn_buffer (&vbp, &vbuflen, sec);
      stm_unbuffer_int (&bufptr, &usec);
      vrpn_buffer (&vbp, &vbuflen, usec);
      stm_unbuffer_float (&bufptr, &value);
      vrpn_buffer (&vbp, &vbuflen, value);
      stm_unbuffer_float (&bufptr, &std_dev);
      vrpn_buffer (&vbp, &vbuflen, std_dev);
      if (first_time_sec == 0) {
	 first_time_sec = sec;
      }
      now.tv_sec = sec - first_time_sec;
      now.tv_usec = usec;
      connection->pack_message(10000 - vbuflen, now, PointResultNM_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);
      
      break;

    case STM_PULSE_COMPLETED_NM: 
      stm_unbuffer_float (&bufptr, &fx);
      vrpn_buffer (&vbp, &vbuflen, fx);
      stm_unbuffer_float (&bufptr, &fy);
      vrpn_buffer (&vbp, &vbuflen, fy);
      connection->pack_message(10000 - vbuflen, now, PulseCompletedNM_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);

      break;

    case STM_PULSE_FAILURE_NM: 
      stm_unbuffer_float (&bufptr, &fx);
      vrpn_buffer (&vbp, &vbuflen, fx);
      stm_unbuffer_float (&bufptr, &fy);
      vrpn_buffer (&vbp, &vbuflen, fy);
      connection->pack_message(10000 - vbuflen, now, PulseFailureNM_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);

      break;

    case STM_SET_REGION_COMPLETED: 
      stm_unbuffer_float (&bufptr, &xmin);
      vrpn_buffer (&vbp, &vbuflen, xmin);
      stm_unbuffer_float (&bufptr, &ymin);
      vrpn_buffer (&vbp, &vbuflen, ymin);
      stm_unbuffer_float (&bufptr, &xmax);
      vrpn_buffer (&vbp, &vbuflen, xmax);
      stm_unbuffer_float (&bufptr, &ymax);
      vrpn_buffer (&vbp, &vbuflen, ymax);
      connection->pack_message(10000 - vbuflen, now, SetRegionCompleted_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);

      break;

    case STM_SET_REGION_CLIPPED: 
      stm_unbuffer_float (&bufptr, &xmin);
      vrpn_buffer (&vbp, &vbuflen, xmin);
      stm_unbuffer_float (&bufptr, &ymin);
      vrpn_buffer (&vbp, &vbuflen, ymin);
      stm_unbuffer_float (&bufptr, &xmax);
      vrpn_buffer (&vbp, &vbuflen, xmax);
      stm_unbuffer_float (&bufptr, &ymax);
      vrpn_buffer (&vbp, &vbuflen, ymax);
      connection->pack_message(10000 - vbuflen, now, SetRegionClipped_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);

      break;

    case STM_TUNNELLING_ATTAINED_NM: 
      stm_unbuffer_float (&bufptr, &value);
      vrpn_buffer (&vbp, &vbuflen, value);
      connection->pack_message(10000 - vbuflen, now, TunnellingAttained_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);

      break;

    case SPM_SCAN_RANGE: 
      stm_unbuffer_float (&bufptr, &xmin);
      vrpn_buffer (&vbp, &vbuflen, xmin);
      stm_unbuffer_float (&bufptr, &xmax);
      vrpn_buffer (&vbp, &vbuflen, xmax);
      stm_unbuffer_float (&bufptr, &ymin);
      vrpn_buffer (&vbp, &vbuflen, ymin);
      stm_unbuffer_float (&bufptr, &ymax);
      vrpn_buffer (&vbp, &vbuflen, ymax);
      stm_unbuffer_float (&bufptr, &zmin);
      vrpn_buffer (&vbp, &vbuflen, zmin);
      stm_unbuffer_float (&bufptr, &zmax);
      vrpn_buffer (&vbp, &vbuflen, zmax);
      connection->pack_message(10000 - vbuflen, now, ScanRange_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);
      break;

    case AFM_MOD_FORCE_SET: 
      stm_unbuffer_float (&bufptr, &value);
      vrpn_buffer (&vbp, &vbuflen, value);
      connection->pack_message(10000 - vbuflen, now, ModForceSet_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);

      break;

    case AFM_MOD_FORCE_SET_FAILURE: 
      stm_unbuffer_float (&bufptr, &value);
      vrpn_buffer (&vbp, &vbuflen, value);
      connection->pack_message(10000 - vbuflen, now, ModForceSetFailure_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);

      break;

    case AFM_IMG_FORCE_SET: 
      stm_unbuffer_float (&bufptr, &value);
      vrpn_buffer  (&vbp, &vbuflen, value);
      connection->pack_message(10000 - vbuflen, now, ImgForceSet_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);

      break;

    case AFM_IMG_FORCE_SET_FAILURE: 
      stm_unbuffer_float (&bufptr, &value);
      vrpn_buffer (&vbp, &vbuflen, value);
      connection->pack_message(10000 - vbuflen, now, ImgForceSetFailure_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);

      break;

    case AFM_IMG_SET: 
      stm_unbuffer_int (&bufptr, &enabled);
      vrpn_buffer (&vbp, &vbuflen, enabled);
      stm_unbuffer_float (&bufptr, &fmin); 
      vrpn_buffer (&vbp, &vbuflen, fmin);
      stm_unbuffer_float (&bufptr, &fmax); 
      vrpn_buffer (&vbp, &vbuflen, fmax);
      stm_unbuffer_float (&bufptr, &fcur); 
      vrpn_buffer (&vbp, &vbuflen, fcur);
      connection->pack_message(10000 - vbuflen, now, ImgSet_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);

      break;

    case AFM_MOD_SET: 
      stm_unbuffer_int (&bufptr, &enabled);
      vrpn_buffer (&vbp, &vbuflen, enabled);
      stm_unbuffer_float (&bufptr, &fmin); 
      vrpn_buffer (&vbp, &vbuflen, fmin); 
      stm_unbuffer_float (&bufptr, &fmax); 
      vrpn_buffer (&vbp, &vbuflen, fmax);
      stm_unbuffer_float (&bufptr, &fcur);
      vrpn_buffer (&vbp, &vbuflen, fcur);
      connection->pack_message(10000 - vbuflen, now, ModSet_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);

      break;

    case AFM_IN_MOD_MODE: 
      printf ("AFM_IN_MOD_MODE ()\n");
      break;

    case AFM_IN_IMG_MODE:
      printf ("AFM_IN_IMG_MODE ()\n");
      break;

    case SPM_RELAX_SET: 
      stm_unbuffer_int (&bufptr, &x); 
      vrpn_buffer (&vbp, &vbuflen, x);
      stm_unbuffer_int (&bufptr, &y); 
      vrpn_buffer (&vbp, &vbuflen, y);
      connection->pack_message(10000 - vbuflen, now, RelaxSet_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);
      break;

    case AFM_IN_MOD_MODE_T: 
      //stm_unbuffer_long (&bufptr, &sec);
      stm_unbuffer_int (&bufptr, &sec);
      vrpn_buffer (&vbp, &vbuflen, sec);
      //stm_unbuffer_long (&bufptr, &usec);
      stm_unbuffer_int (&bufptr, &usec);
      vrpn_buffer (&vbp, &vbuflen, usec);
      if (first_time_sec == 0) {
	 first_time_sec = sec;
      }
      now.tv_sec = sec - first_time_sec;
      now.tv_usec = usec;
      connection->pack_message(10000 - vbuflen, now, InModModeT_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);
      break;

    case AFM_IN_IMG_MODE_T: 
      //stm_unbuffer_long (&bufptr, &sec);
      stm_unbuffer_int (&bufptr, &sec);
      vrpn_buffer (&vbp, &vbuflen, sec);
      //stm_unbuffer_long (&bufptr, &usec);
      stm_unbuffer_int (&bufptr, &usec);
      vrpn_buffer (&vbp, &vbuflen, usec);
      if (first_time_sec == 0) {
	 first_time_sec = sec;
      }
      now.tv_sec = sec - first_time_sec;
      now.tv_usec = usec;
      connection->pack_message(10000 - vbuflen, now, InImgModeT_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);
      break;

    case SPM_RESISTANCE:
      //stm_unbuffer_long (&bufptr, &lscrap);
      stm_unbuffer_int (&bufptr, &lscrap);
      vrpn_buffer (&vbp, &vbuflen, lscrap);
      //stm_unbuffer_long (&bufptr, &sec);
      stm_unbuffer_int (&bufptr, &sec);
      vrpn_buffer (&vbp, &vbuflen, sec);
      //stm_unbuffer_long (&bufptr, &usec);
      stm_unbuffer_int (&bufptr, &usec);
      vrpn_buffer (&vbp, &vbuflen, usec); 
      stm_unbuffer_float (&bufptr, &fscrap);
      vrpn_buffer (&vbp, &vbuflen, fscrap);
      if (first_time_sec == 0) {
	 first_time_sec = sec;
      }
      now.tv_sec = sec - first_time_sec;
      now.tv_usec = usec;
      connection->pack_message(10000 - vbuflen, now, Resistance_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);

      break;

    case SPM_RESISTANCE_FAILURE:
      //stm_unbuffer_long (&bufptr, &lscrap);
      stm_unbuffer_int (&bufptr, &lscrap);
      vrpn_buffer (&vbp, &vbuflen, iscrap); 
      connection->pack_message(10000 - vbuflen, now, ResistanceFailure_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);

      break;
      
    case STM_ZIG_RESULT_NM:
      printf ("STM_ZIG_RESULT_NM (");
      stm_unbuffer_float (&bufptr, &fx);
      vrpn_buffer (&vbp, &vbuflen, fx);
      stm_unbuffer_float (&bufptr, &fy);
      vrpn_buffer (&vbp, &vbuflen, fy);
      //stm_unbuffer_long (&bufptr, &sec);
      stm_unbuffer_int (&bufptr, &sec);
      vrpn_buffer (&vbp, &vbuflen, sec);
      //stm_unbuffer_long (&bufptr, &usec);
      stm_unbuffer_int (&bufptr, &usec);
      vrpn_buffer (&vbp, &vbuflen, usec);
      stm_unbuffer_float (&bufptr, &fscrap);
      vrpn_buffer (&vbp, &vbuflen, fscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      vrpn_buffer (&vbp, &vbuflen, fscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      vrpn_buffer (&vbp, &vbuflen, fscrap);
      stm_unbuffer_float (&bufptr, &fscrap);     
      vrpn_buffer (&vbp, &vbuflen, fscrap);
      if (first_time_sec == 0) {
	 first_time_sec = sec;
      }
      now.tv_sec = sec - first_time_sec;
      now.tv_usec = usec;
      connection->pack_message(10000 - vbuflen, now, ZigResultNM_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);

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
      vrpn_buffer (&vbp, &vbuflen, iscrap);
      connection->pack_message(10000 - vbuflen, now, ScanParameters_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);

      break;

    case SPM_BLUNT_RESULT_NM:
      stm_unbuffer_float (&bufptr, &fx);
      vrpn_buffer (&vbp, &vbuflen, fx);
      stm_unbuffer_float (&bufptr, &fy);
      vrpn_buffer (&vbp, &vbuflen, fy);
      //stm_unbuffer_long (&bufptr, &sec);
      stm_unbuffer_int (&bufptr, &sec);
      vrpn_buffer (&vbp, &vbuflen, sec);
      //stm_unbuffer_long (&bufptr, &usec);
      stm_unbuffer_int (&bufptr, &usec);
      vrpn_buffer (&vbp, &vbuflen, usec);
      stm_unbuffer_float (&bufptr, &fscrap);
      vrpn_buffer (&vbp, &vbuflen, fscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      vrpn_buffer (&vbp, &vbuflen, fscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      vrpn_buffer (&vbp, &vbuflen, fscrap);
      stm_unbuffer_float (&bufptr, &fscrap);  
      vrpn_buffer (&vbp, &vbuflen, fscrap);
      if (first_time_sec == 0) {
	 first_time_sec = sec;
      }
      now.tv_sec = sec - first_time_sec;
      now.tv_usec = usec;
      connection->pack_message(10000 - vbuflen, now, BluntResultNM_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);

      break;
      
    case SPM_WINDOW_LINE_DATA:
      stm_unbuffer_int (&bufptr, &x);
      vrpn_buffer (&vbp, &vbuflen, x);
      stm_unbuffer_int (&bufptr, &y);
      vrpn_buffer (&vbp, &vbuflen, y);
      stm_unbuffer_int (&bufptr, &dx);
      vrpn_buffer (&vbp, &vbuflen, dx);
      stm_unbuffer_int (&bufptr, &dy);
      vrpn_buffer (&vbp, &vbuflen, dy);
      stm_unbuffer_int (&bufptr, &reports);
      vrpn_buffer (&vbp, &vbuflen, reports);
      stm_unbuffer_int (&bufptr, &fields);
      vrpn_buffer (&vbp, &vbuflen, fields);
      //stm_unbuffer_long (&bufptr, &sec);
      stm_unbuffer_int (&bufptr, &sec);
      vrpn_buffer (&vbp, &vbuflen, sec);
      //stm_unbuffer_long (&bufptr, &usec);
      stm_unbuffer_int (&bufptr, &usec);
      vrpn_buffer(&vbp, &vbuflen, usec);
      for (i = 0; i < reports; i++) {
        for (j = 0; j < fields; j++) {
	  stm_unbuffer_float (&bufptr, &fscrap);
          vrpn_buffer (&vbp, &vbuflen, fscrap);
        }
      }
      if (first_time_sec == 0) {
	 first_time_sec = sec;
      }
      now.tv_sec = sec - first_time_sec;
      now.tv_usec = usec;
      connection->pack_message(10000 - vbuflen, now, WindowLineData_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);
      break;

    case SPM_HELLO_MESSAGE:
      stm_unbuffer_chars (&bufptr, string, 4);
      vrpn_buffer(&vbp, &vbuflen, string, 4);
      stm_unbuffer_chars (&bufptr, string, 64);
      vrpn_buffer(&vbp, &vbuflen, string, 64);
      stm_unbuffer_int (&bufptr, &iscrap);
      vrpn_buffer (&vbp, &vbuflen, iscrap);
      stm_unbuffer_int (&bufptr, &iscrap);
      vrpn_buffer (&vbp, &vbuflen, iscrap);

      connection->pack_message(10000 - vbuflen, now, HelloMessage_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);
      break;
      
    case SPM_SCAN_DATASETS:
      stm_unbuffer_int (&bufptr, &iscrap);
      vrpn_buffer (&vbp, &vbuflen, iscrap);
      for (i = 0; i < iscrap; i++) {
	stm_unbuffer_chars (&bufptr, string, 64);
        vrpn_buffer (&vbp, &vbuflen, string, 64);
	stm_unbuffer_chars (&bufptr, string, 64);
        vrpn_buffer (&vbp, &vbuflen, string, 64);
	stm_unbuffer_float (&bufptr, &fscrap);
        vrpn_buffer (&vbp, &vbuflen, fscrap);
	stm_unbuffer_float (&bufptr, &fscrap);
        vrpn_buffer (&vbp, &vbuflen, fscrap);
      }
      printf ("SPM_SCAN_DATASETS %d\n", iscrap);
      connection->pack_message(10000 - vbuflen, now, ScanDataset_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);
      break;

    case SPM_REPORT_SLOW_SCAN:
      stm_unbuffer_int (&bufptr, &iscrap);
      vrpn_buffer (&vbp, &vbuflen, iscrap);
      connection->pack_message(10000 - vbuflen, now, ReportSlowScan_type,
				myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);
      break;

    case SPM_CLIENT_HELLO:
      printf ("Found SPM_CLIENT_HELLO;  issuing HelloMessage.\n");
      stm_unbuffer_chars (&bufptr, string, 4);
      vrpn_buffer (&vbp, &vbuflen, string, 4);
      stm_unbuffer_chars (&bufptr, string, 64);
      vrpn_buffer (&vbp, &vbuflen, string, 64);
      stm_unbuffer_int (&bufptr, &iscrap);
      vrpn_buffer (&vbp, &vbuflen, iscrap);
      stm_unbuffer_int (&bufptr, &iscrap);
      vrpn_buffer (&vbp, &vbuflen, iscrap);

      connection->pack_message(10000 - vbuflen, now, HelloMessage_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);
      break;      
      
    case SPM_POINT_DATASETS:
      stm_unbuffer_int (&bufptr, &reports);
      vrpn_buffer (&vbp, &vbuflen, reports);
      for (i = 0; i < reports; i++) {
	stm_unbuffer_chars (&bufptr, string, 64);
        vrpn_buffer (&vbp, &vbuflen, string, 64);
	stm_unbuffer_chars (&bufptr, string, 64);
        vrpn_buffer (&vbp, &vbuflen, string, 64);
	stm_unbuffer_int (&bufptr, &iscrap);
        vrpn_buffer (&vbp, &vbuflen, iscrap);
	stm_unbuffer_float (&bufptr, &fscrap);
        vrpn_buffer (&vbp, &vbuflen, fscrap);
	stm_unbuffer_float (&bufptr, &fscrap);
        vrpn_buffer (&vbp, &vbuflen, fscrap);
      }
      connection->pack_message(10000 - vbuflen, now, PointDataset_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);
      break;      

    case SPM_POINT_RESULT_DATA:
      stm_unbuffer_float (&bufptr, &fx);
      vrpn_buffer (&vbp, &vbuflen, fx);
      stm_unbuffer_float (&bufptr, &fy);
      vrpn_buffer (&vbp, &vbuflen, fy);
      //stm_unbuffer_long (&bufptr, &sec);
      stm_unbuffer_int (&bufptr, &sec);
      vrpn_buffer (&vbp, &vbuflen, sec);
      //stm_unbuffer_long (&bufptr, &usec);
      stm_unbuffer_int (&bufptr, &usec);
      vrpn_buffer (&vbp, &vbuflen, usec);
      stm_unbuffer_int (&bufptr, &reports);
      vrpn_buffer (&vbp, &vbuflen, reports);
      for (i = 0; i < reports; i++) {
	stm_unbuffer_float (&bufptr, &fscrap);
        vrpn_buffer (&vbp, &vbuflen, fscrap);
      }
      if (first_time_sec == 0) {
	 first_time_sec = sec;
      }
      now.tv_sec = sec - first_time_sec;
      now.tv_usec = usec;
      connection->pack_message(10000 - vbuflen, now, PointResultData_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);
      break;

    case SPM_PID_PARAMETERS:
      stm_unbuffer_float (&bufptr, &P);
      vrpn_buffer (&vbp, &vbuflen, P);
      stm_unbuffer_float (&bufptr, &I);
      vrpn_buffer (&vbp, &vbuflen, I);
      stm_unbuffer_float (&bufptr, &D);
      vrpn_buffer(&vbp, &vbuflen, D);

      connection->pack_message(10000 - vbuflen, now, PidParameters_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);
      break;

    case SPM_SCANRATE_PARAMETER:
      stm_unbuffer_float (&bufptr, &fscrap);
      vrpn_buffer (&vbp, &vbuflen, fscrap);
      connection->pack_message(10000 - vbuflen, now, ScanrateParameter_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);
      break;

    case SPM_REPORT_GRID_SIZE:
      stm_unbuffer_int (&bufptr, &x);
      vrpn_buffer (&vbp, &vbuflen, x);
      stm_unbuffer_int (&bufptr, &y);
      vrpn_buffer (&vbp, &vbuflen, y);
      connection->pack_message(10000 - vbuflen, now, ReportGridSize_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);
      break;
      
    case AFM_IN_SEWING_MODE:
      stm_unbuffer_float (&bufptr, &fscrap);
      vrpn_buffer (&vbp, &vbuflen, fscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      vrpn_buffer (&vbp, &vbuflen, fscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      vrpn_buffer (&vbp, &vbuflen, fscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      vrpn_buffer (&vbp, &vbuflen, fscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      vrpn_buffer (&vbp, &vbuflen, fscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      vrpn_buffer (&vbp, &vbuflen, fscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      vrpn_buffer (&vbp, &vbuflen, fscrap);
      connection->pack_message(10000 - vbuflen, now, InSewingStyle_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);
      break;

    case AFM_IN_CONTACT_MODE:
      stm_unbuffer_float (&bufptr, &P);
      vrpn_buffer (&vbp, &vbuflen, P);
      stm_unbuffer_float (&bufptr, &I);
      vrpn_buffer (&vbp, &vbuflen, I);
      stm_unbuffer_float (&bufptr, &D);
      vrpn_buffer (&vbp, &vbuflen, D);
      stm_unbuffer_float (&bufptr, &fscrap);
      vrpn_buffer (&vbp, &vbuflen, fscrap);
      connection->pack_message(10000 - vbuflen, now, InContactMode_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);
      break;      

    case AFM_IN_TAPPING_MODE:
      stm_unbuffer_float (&bufptr, &P);
      vrpn_buffer(&vbp, &vbuflen, P);
      stm_unbuffer_float (&bufptr, &I);
      vrpn_buffer (&vbp, &vbuflen, I);
      stm_unbuffer_float (&bufptr, &D);
      vrpn_buffer (&vbp, &vbuflen, D);
      stm_unbuffer_float (&bufptr, &fscrap);
      vrpn_buffer (&vbp, &vbuflen, fscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      vrpn_buffer (&vbp, &vbuflen, fscrap);
      connection->pack_message(10000 - vbuflen, now, InTappingMode_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);
      break;            

    case SPM_BOTTOM_PUNCH_RESULT_DATA:
      stm_unbuffer_float (&bufptr, &fx);
      vrpn_buffer (&vbp, &vbuflen, fx);
      stm_unbuffer_float (&bufptr, &fy);
      vrpn_buffer (&vbp, &vbuflen, fy);
      //stm_unbuffer_long (&bufptr, &sec);
      //stm_unbuffer_long (&bufptr, &usec);
      stm_unbuffer_int (&bufptr, &sec);
      vrpn_buffer (&vbp, &vbuflen, sec);
      stm_unbuffer_int (&bufptr, &usec);
      vrpn_buffer (&vbp, &vbuflen, usec);
      stm_unbuffer_int (&bufptr, &reports);
      vrpn_buffer (&vbp, &vbuflen, reports);      
      for (i=0; i<reports; i++) {
	stm_unbuffer_float (&bufptr, &fscrap);
	vrpn_buffer (&vbp, &vbuflen, fscrap);
	
      }
      if (first_time_sec == 0) {
	 first_time_sec = sec;
      }
      now.tv_sec = sec - first_time_sec;
      now.tv_usec = usec;
      connection->pack_message(10000 - vbuflen, now, BottomPunchResultData_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);
      break;

    case SPM_TOP_PUNCH_RESULT_DATA:
      stm_unbuffer_float (&bufptr, &fx);
      vrpn_buffer (&vbp, &vbuflen, fx);
      stm_unbuffer_float (&bufptr, &fy);
      vrpn_buffer (&vbp, &vbuflen, fy);
      //stm_unbuffer_long (&bufptr, &sec);
      //stm_unbuffer_long (&bufptr, &usec);
      stm_unbuffer_int (&bufptr, &sec);
      vrpn_buffer (&vbp, &vbuflen, sec);
      stm_unbuffer_int (&bufptr, &usec);
      vrpn_buffer (&vbp, &vbuflen, usec);
      stm_unbuffer_int (&bufptr, &reports);
      vrpn_buffer (&vbp, &vbuflen, reports);
      for (i=0; i<reports; i++) {
	stm_unbuffer_float (&bufptr, &fscrap);
	vrpn_buffer (&vbp, &vbuflen, fscrap);
	
      }
      if (first_time_sec == 0) {
	 first_time_sec = sec;
      }
      now.tv_sec = sec - first_time_sec;
      now.tv_usec = usec;
      connection->pack_message(10000 - vbuflen, now, TopPunchResultData_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);
      break;

    case SPM_VOLTSOURCE_ENABLED:
      stm_unbuffer_int (&bufptr, &iscrap);
      vrpn_buffer (&vbp, &vbuflen, iscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      vrpn_buffer (&vbp, &vbuflen, fscrap);
      connection->pack_message(10000 - vbuflen, now, EnableVoltsource_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);
      break;

    case SPM_VOLTSOURCE_DISABLED:
      stm_unbuffer_int (&bufptr, &iscrap);
      vrpn_buffer (&vbp, &vbuflen, iscrap);
      connection->pack_message(10000 - vbuflen, now, DisableVoltsource_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);
      break;

    case SPM_AMP_ENABLED:
      stm_unbuffer_int (&bufptr, &iscrap);
      vrpn_buffer (&vbp, &vbuflen, iscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      vrpn_buffer (&vbp, &vbuflen, fscrap);
      printf ("SPM_AMP_ENABLED (%d, %g, ", iscrap, fscrap);      
      stm_unbuffer_float (&bufptr, &fscrap);
      vrpn_buffer (&vbp, &vbuflen, fscrap);
      stm_unbuffer_int (&bufptr, &iscrap);
      vrpn_buffer (&vbp, &vbuflen, iscrap);
      connection->pack_message(10000 - vbuflen, now, EnableAmp_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);
      break;

    case SPM_AMP_DISABLED:
      stm_unbuffer_int (&bufptr, &iscrap);
      vrpn_buffer (&vbp, &vbuflen, iscrap);
      connection->pack_message(10000 - vbuflen, now, DisableAmp_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);
      break;

    case SPM_STARTING_TO_RELAX:
      //stm_unbuffer_long (&bufptr, &sec);
      stm_unbuffer_int (&bufptr, &sec);
      vrpn_buffer (&vbp, &vbuflen, sec);
      //stm_unbuffer_long (&bufptr, &usec);
      stm_unbuffer_int (&bufptr, &usec);
      vrpn_buffer (&vbp, &vbuflen, usec);
      if (first_time_sec == 0) {
	 first_time_sec = sec;
      }
      now.tv_sec = sec - first_time_sec;
      now.tv_usec = usec;
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
      vrpn_buffer (&vbp, &vbuflen, lscrap);
      if (lscrap >= 5000) {
	fprintf (stderr, "header is too long -- %u bytes\n", lscrap);
	exit (-1);
      }
	
      stm_unbuffer_chars (&bufptr, header, lscrap);
      vrpn_buffer (&vbp, &vbuflen, header, lscrap);
      connection->pack_message(10000 - vbuflen, now, TopoFileHeader_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);
      break;

    case SPM_SERVER_PACKET_TIMESTAMP:
      stm_unbuffer_int (&bufptr, &sec);
      vrpn_buffer (&vbp, &vbuflen, sec);
      stm_unbuffer_int (&bufptr, &usec);
      vrpn_buffer (&vbp, &vbuflen, usec);
      if (first_time_sec == 0) {
	 first_time_sec = sec;
      }
      now.tv_sec = sec - first_time_sec;
      now.tv_usec = usec;
      connection->pack_message(10000 - vbuflen, now, ServerPacketTimestamp_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);
      break;

    case OHM_RESISTANCE:
      stm_unbuffer_int (&bufptr, &iscrap); 
      vrpn_buffer (&vbp, &vbuflen, iscrap);
      //stm_unbuffer_long (&bufptr, &sec);
      //stm_unbuffer_long (&bufptr, &usec);
      stm_unbuffer_float (&bufptr, &resist);
      vrpn_buffer (&vbp, &vbuflen, resist);
      stm_unbuffer_float (&bufptr, &voltage);
      vrpn_buffer (&vbp, &vbuflen, voltage);
      stm_unbuffer_float (&bufptr, &range);
      vrpn_buffer (&vbp, &vbuflen, range);
      stm_unbuffer_float (&bufptr, &filter);
      vrpn_buffer (&vbp, &vbuflen, filter);
      if (first_time_sec == 0) {
	 first_time_sec = sec;
      }
      now.tv_sec = sec - first_time_sec;
      now.tv_usec = usec;
      connection->pack_message(10000 - vbuflen, now, RecordResistance_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);
      break;

    case SPM_FORCE_CURVE_DATA:
      stm_unbuffer_float (&bufptr, &fx);     
      vrpn_buffer (&vbp, &vbuflen, fx);
      stm_unbuffer_float (&bufptr, &fy);
      vrpn_buffer (&vbp, &vbuflen, fy);
      //stm_unbuffer_long (&bufptr, &num_samples);
      stm_unbuffer_int (&bufptr, &num_samples);
      vrpn_buffer (&vbp, &vbuflen, num_samples);
      //stm_unbuffer_long (&bufptr, &num_halfcycles);
      stm_unbuffer_int (&bufptr, &num_halfcycles);
      vrpn_buffer (&vbp, &vbuflen, num_halfcycles);
      //stm_unbuffer_long (&bufptr, &sec);
      stm_unbuffer_int (&bufptr, &sec);
      vrpn_buffer (&vbp, &vbuflen, sec);
      //stm_unbuffer_long (&bufptr, &usec);
      stm_unbuffer_int (&bufptr, &usec);
      vrpn_buffer (&vbp, &vbuflen, usec);
      for (i = 0; i < num_samples; i++) { 
	 stm_unbuffer_float (&bufptr, &fscrap);
         vrpn_buffer (&vbp, &vbuflen, fscrap);
	 for (j = 0; j < num_halfcycles; j++) {
	    stm_unbuffer_float (&bufptr, &fscrap);
            vrpn_buffer (&vbp, &vbuflen, fscrap);
	 }
      }
      if (first_time_sec == 0) {
	 first_time_sec = sec;
      }
      now.tv_sec = sec  - first_time_sec;
      now.tv_usec = usec;
      connection->pack_message(10000 - vbuflen, now, ForceCurveData_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);
      break;

    case SPM_IN_SPECTROSCOPY_MODE:
      stm_unbuffer_float (&bufptr, &fscrap);
      vrpn_buffer (&vbp, &vbuflen, fscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      vrpn_buffer (&vbp, &vbuflen, fscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      vrpn_buffer (&vbp, &vbuflen, fscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      vrpn_buffer (&vbp, &vbuflen, fscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      vrpn_buffer (&vbp, &vbuflen, fscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      vrpn_buffer (&vbp, &vbuflen, fscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      vrpn_buffer (&vbp, &vbuflen, fscrap);
      //stm_unbuffer_long (&bufptr, &lscrap);
      stm_unbuffer_int (&bufptr, &lscrap);
      vrpn_buffer (&vbp, &vbuflen, lscrap);
      //stm_unbuffer_long (&bufptr, &lscrap);
      stm_unbuffer_int (&bufptr, &lscrap);
      vrpn_buffer (&vbp, &vbuflen, lscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      vrpn_buffer (&vbp, &vbuflen, fscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      vrpn_buffer (&vbp, &vbuflen, fscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      vrpn_buffer (&vbp, &vbuflen, fscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      vrpn_buffer (&vbp, &vbuflen, fscrap);
      //stm_unbuffer_long (&bufptr, &lscrap);
      stm_unbuffer_int (&bufptr, &lscrap);
      vrpn_buffer (&vbp, &vbuflen, lscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      vrpn_buffer (&vbp, &vbuflen, fscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      vrpn_buffer (&vbp, &vbuflen, fscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      vrpn_buffer (&vbp, &vbuflen, fscrap);
      connection->pack_message(10000 - vbuflen, now, InSpectroscopyMode_type,
      myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);
      
      break;

    case OHM_RESISTANCE_WSTATUS:
      stm_unbuffer_int (&bufptr, &iscrap);
      vrpn_buffer (&vbp, &vbuflen, iscrap);
      stm_unbuffer_int (&bufptr, &sec);
      vrpn_buffer (&vbp, &vbuflen, sec);
      stm_unbuffer_int (&bufptr, &usec);
      vrpn_buffer (&vbp, &vbuflen, usec);
      stm_unbuffer_float (&bufptr, &resist);
      vrpn_buffer (&vbp, &vbuflen, resist);
      stm_unbuffer_float (&bufptr, &voltage);
      vrpn_buffer (&vbp, &vbuflen, voltage);
      stm_unbuffer_float (&bufptr, &range);
      vrpn_buffer (&vbp, &vbuflen, range);
      stm_unbuffer_float (&bufptr, &filter);
      vrpn_buffer (&vbp, &vbuflen, filter);
      stm_unbuffer_int (&bufptr, &status);
      vrpn_buffer (&vbp, &vbuflen,status);
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
      if (first_time_sec == 0) {
	 first_time_sec = sec;
      }
      now.tv_sec = sec - first_time_sec;
      now.tv_usec = usec;
	  printf(")\n");
      break;

    /* Parameters echoed back by the server to record info in the stream */
    case AFM_BASE_MOD_PARAMETERS:
      stm_unbuffer_float (&bufptr, &baseforce);
      vrpn_buffer (&vbp, &vbuflen, baseforce);
      stm_unbuffer_float (&bufptr, &modforce); 
      vrpn_buffer (&vbp, &vbuflen, modforce);
      connection->pack_message(10000 - vbuflen, now, BaseModParameters_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);
      break;

    case SPM_VISIBLE_TRAIL:
      printf ("SPM_VISIBLE_TRAIL\n");
      break;

    case SPM_INVISIBLE_TRAIL:
      printf ("SPM_INVISIBLE_TRAIL\n");
      break;

    case AFM_FORCE_SETTINGS:
      stm_unbuffer_float (&bufptr, &base);
      vrpn_buffer (&vbp, &vbuflen, base);
      stm_unbuffer_float (&bufptr, &mode); 
      vrpn_buffer (&vbp, &vbuflen, mode);
      stm_unbuffer_float (&bufptr, &current);
      vrpn_buffer (&vbp, &vbuflen, current);
      connection->pack_message(10000 - vbuflen, now, ForceSettings_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);
      break;

    case SPM_REFRESH_GRID:
      printf ("SPM_REFRESH_GRID\n");
      break;

    case NANO_RECV_TIMESTAMP:
      stm_unbuffer_int (&bufptr, &sec);
      stm_unbuffer_int (&bufptr, &usec);
      if (first_time_sec == 0) {
	 first_time_sec = sec;
      }
      now.tv_sec = sec - first_time_sec;
      now.tv_usec = usec;
      connection->pack_message(10000 - vbuflen, now, RecvTimestamp_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);
      break;


    case FAKE_SPM_SEND_TIMESTAMP:
      stm_unbuffer_int (&bufptr, &sec);
      stm_unbuffer_int (&bufptr, &usec);
      if (first_time_sec == 0) {
	 first_time_sec = sec;
      }
      now.tv_sec = sec  - first_time_sec;
      now.tv_usec = usec;
      connection->pack_message(10000 - vbuflen, now, FakeSendTimestamp_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);
      break;

    case SPM_UDP_SEQ_NUM:
      stm_unbuffer_int (&bufptr, &lscrap);
      vrpn_buffer (&vbp, &vbuflen, lscrap);
      connection->pack_message(10000 - vbuflen, now, UpSeqNum_type,
                               myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);
      break;

    default:
      fprintf(stderr, "=> Unknown data type returned (%d)\n", 
	      data_type);
      exit(-1);
    }
    connection->mainloop();
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

    WindowLineData_type = connection->register_message_type  
         ("nmm Microscope WindowLineData");
    WindowScanNM_type = connection->register_message_type
         ("nmm Microscope WindowScanNM");
    WindowBackscanNM_type = connection->register_message_type
         ("nmm Microscope WindowBackscanNM");
    PointResultNM_type = connection->register_message_type
         ("nmm Microscope PointResultNM");
    PointResultData_type = connection->register_message_type
         ("nmm Microscope PointResultData");
    BottomPunchResultData_type = connection->register_message_type
         ("nmm Microscope BottomPunchResultData");
    TopPunchResultData_type = connection->register_message_type
         ("nmm Microscope TopPunchResultData");
    ZigResultNM_type = connection->register_message_type
         ("nmm Microscope ZigResultNM");
    BluntResultNM_type = connection->register_message_type
         ("nmm Microscope BluntResultNM");
    ScanRange_type = connection->register_message_type
         ("nmm Microscope ScanRange");
    SetRegionCompleted_type = connection->register_message_type
         ("nmm Microscope SetRegionCompleted");
    SetRegionClipped_type = connection->register_message_type
         ("nmm Microscope SetRegionClipped");
    ResistanceFailure_type = connection->register_message_type
         ("nmm Microscope ResistanceFailure");
    Resistance_type = connection->register_message_type
         ("nmm Microscope Resistance");
    Resistance2_type = connection->register_message_type
         ("nmm Microscope Resistance2");
    ReportSlowScan_type = connection->register_message_type
         ("nmm Microscope ReportSlowScan");
    ScanParameters_type = connection->register_message_type
         ("nmm Microscope ScanParameters");
    HelloMessage_type = connection->register_message_type
         ("nmm Microscope HelloMessage");
    ClientHello_type = connection->register_message_type
         ("nmm Microscope ClientHello");
    ScanDataset_type = connection->register_message_type
         ("nmm Microscope ScanDataset");
    PointDataset_type = connection->register_message_type
         ("nmm Microscope PointDataset");
    PidParameters_type = connection->register_message_type
         ("nmm Microscope PidParameters");
    ScanrateParameter_type = connection->register_message_type
         ("nmm Microscope ScanrateParameter");
    ReportGridSize_type = connection->register_message_type
         ("nmm Microscope ReportGridSize");
    ServerPacketTimestamp_type = connection->register_message_type
         ("nmm Microscope ServerPacketTimestamp");
    TopoFileHeader_type = connection->register_message_type
         ("nmm Microscope TopoFileHeader");
    ForceCurveData_type = connection->register_message_type
	 ("nmm Microscope ForceCurveData");

    MaxSetpointExceeded_type = connection->register_message_type
	 ("nmm Microscope MaxSetpointExceeded");

    RecvTimestamp_type = connection->register_message_type
         ("nmm Microscope Clark RecvTimestamp");
    FakeSendTimestamp_type = connection->register_message_type
         ("nmm Microscope Clark FakeSendTimestamp");
    UpSeqNum_type = connection->register_message_type
         ("nmm Microscope Clark UdpSeqNum");

    InTappingMode_type = connection->register_message_type
        ("nmm Microscope AFM InTappingMode");
    InOscillatingMode_type = connection->register_message_type
        ("nmm Microscope AFM InOscillatingMode");
    InContactMode_type = connection->register_message_type
        ("nmm Microscope AFM InContactMode");
    InDirectZControl_type = connection->register_message_type
        ("nmm Microscope AFM InDirectZControl");
    InSewingStyle_type = connection->register_message_type
        ("nmm Microscope AFM InSewingStyle");
    InSpectroscopyMode_type = connection->register_message_type
	("nmm Microscope AFM InSpectroscopyMode");
    ForceParameters_type = connection->register_message_type
        ("nmm Microscope AFM ForceParameters");
    BaseModParameters_type = connection->register_message_type
        ("nmm Microscope AFM BaseModParameters");
    ForceSettings_type = connection->register_message_type
        ("nmm Microscope AFM ForceSettings");
    InModModeT_type = connection->register_message_type
        ("nmm Microscope AFM InModModeT");
    InImgModeT_type = connection->register_message_type
        ("nmm Microscope AFM InImgModeT");
    // InModMode_type = connection->register_message_type
    //    ("nmm Microscope AFM InModMode");
    InImgMode_type = connection->register_message_type
        ("nmm Microscope AFM InImgMode");
    ModForceSet_type = connection->register_message_type
        ("nmm Microscope AFM ModForceSet");
    ImgForceSet_type = connection->register_message_type
        ("nmm Microscope AFM ImgForceSet");
    ModForceSetFailure_type = connection->register_message_type
        ("nmm Microscope AFM ModForceSetFailure");
    ImgForceSetFailure_type = connection->register_message_type
        ("nmm Microscope AFM ImgForceSetFailure");
    ModSet_type = connection->register_message_type
        ("nmm Microscope AFM ModSet");
    ImgSet_type = connection->register_message_type
        ("nmm Microscope AFM ImgSet");
    ForceSet_type = connection->register_message_type
        ("nmm Microscope AFM ForceSet");
    ForceSetFailure_type = connection->register_message_type
        ("nmm Microscope AFM ForceSetFailure");

    // SampleApproach_type = connection->register_message_type
    //    ("nmm Microscope STM SampleApproach");
    SetBias_type = connection->register_message_type
        ("nmm Microscope STM SetBias");
    SampleApproachNM_type = connection->register_message_type
        ("nmm Microscope STM SampleApproachNM");
    SetPulsePeak_type = connection->register_message_type
        ("nmm Microscope STM SetPulsePeak");
    SetPulseDuration_type = connection->register_message_type
        ("nmm Microscope STM SetPulseDuration");
    PulsePoint_type = connection->register_message_type
        ("nmm Microscope STM PulsePoint");
    PulsePointNM_type = connection->register_message_type
        ("nmm Microscope STM PulsePointNM");  

    PulseParameters_type = connection->register_message_type
        ("nmm Microscope STM PulseParameters");
    PulseCompletedNM_type = connection->register_message_type
        ("nmm Microscope STM PulseCompletedNM");
    PulseFailureNM_type = connection->register_message_type
        ("nmm Microscope STM PulseFailureNM");
    PulseCompleted_type = connection->register_message_type
        ("nmm Microscope STM PulseCompleted");
    PulseFailure_type = connection->register_message_type
        ("nmm Microscope STM PulseFailure");
    TunnellingAttained_type = connection->register_message_type
        ("nmm Microscope STM TunnellingAttained");
    TunnellingAttainedNM_type = connection->register_message_type
        ("nmm Microscope STM TunnellingAttainedNM");
    TunnellingFailure_type = connection->register_message_type
        ("nmm Microscope STM TunnellingFailure");
    ApproachComplete_type = connection->register_message_type
        ("nmm Microscope STM ApproachComplete"); 

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


#include "nmm_MicroscopeTranslator.h"
#include "vrpn_Ohmmeter.h"      // for ohmmeter status types
#include        <stdio.h>
#include <stdlib.h>
#include        <sys/time.h>
#include        <sys/types.h>
#include        <errno.h>
#include        <math.h>
#include <netinet/in.h>

#include "stm_cmd.h"
#include "stm_file.h"

typedef char* BUFPTR;

void stm_unbuffer_short (BUFPTR* bufptr, short* value);
void stm_unbuffer_double (BUFPTR* bufptr, double* value);
void stm_unbuffer_int (BUFPTR* bufptr, int* value);
void stm_unbuffer_long(BUFPTR* bufptr, long* value);
void stm_unbuffer_timeval(BUFPTR* bufptr, struct timeval* value);
void stm_unbuffer_float(BUFPTR* bufptr, float* value);
void stm_unbuffer_chars(BUFPTR* bufptr, char *c, int len);

extern  "C" int sdi_noint_block_write(int outfile, char buffer[], int length);
extern  "C" int sdi_noint_block_read(int infile, char buffer[], int length);
extern  "C" int sdi_noint_select(int nfds, fd_set *readfds, fd_set *writefds,
                                 fd_set *exceptfds, struct timeval *timeout);

/* Required extern variables */
float   response_x, response_y, pulse_x_res, pulse_y_res;
int     std_dev_samples, pulse_enabled, fmods_enabled;
float   pulse_peak, fmods_baseforce, pulse_bias, response_height, pulse_width;
float   std_dev_frequency, response_std_dev;

nmm_Microscope_Translator::nmm_Microscope_Translator 
           (const char * name,
            vrpn_Connection *c) : 
           nmb_SharedDevice_Server(name, c),
           nmm_Microscope(name, c)
{
  /* When vrpn writes a log file, it uses the current time
   * for system message timestamps and we want to make sure we add this
   * time to the time we read out of the stream file.
   * The alternative would be to alter vrpn in some way to make it not
   * write system messages or to write them with old timestamps but that
   * is probably not worth adding complication to vrpn  */
  gettimeofday(&start_time_for_vrpn, NULL);
  now.tv_sec = start_time_for_vrpn.tv_sec;
  now.tv_usec = start_time_for_vrpn.tv_usec;
  seqNum = 0;

  start_time_for_stream.tv_sec = 0;
  start_time_for_stream.tv_usec = 0;
  start_time_for_stream_initialized = VRPN_FALSE;
}

nmm_Microscope_Translator::~nmm_Microscope_Translator (void)
{}

// converts from stream to vrpn time
void nmm_Microscope_Translator::doTimeCorrection(timeval &msg_time)
{
      if (!start_time_for_stream_initialized) {
         start_time_for_stream.tv_sec = msg_time.tv_sec;
         start_time_for_stream.tv_usec = msg_time.tv_usec;
         start_time_for_stream_initialized = VRPN_TRUE;
         printf("start times:\n");
         printf("  vrpn: (%ld, %ld)\n",
                start_time_for_vrpn.tv_sec, start_time_for_vrpn.tv_usec);
         printf("  stream: (%ld, %ld)\n",
                start_time_for_stream.tv_sec, start_time_for_stream.tv_usec);
      }
      msg_time = vrpn_TimevalDiff(msg_time, start_time_for_stream);
      msg_time = vrpn_TimevalSum(msg_time, start_time_for_vrpn);
}


/*****************************************************************************
 *
   translate_packet
        - Read one packet from file if there is one to read
        - Save the packet to the output stream, translating as needed
        - return 1 if something found, 0 if end of file, -1 if error
 *
 *****************************************************************************/

int nmm_Microscope_Translator::translate_packet(stm_stream *instream)
{
  vrpn_int32    x,y;            /* Position of current read */
  vrpn_int32 dx, dy;
  vrpn_float32  fx,fy;          /* Position as a float */
  vrpn_int32    sec,usec;       /* When the value was obtained */
  vrpn_float32  value;          /* Value at this point */
  vrpn_float32  std_dev;        /* Standard deviation of value */
  vrpn_float32  xmin,xmax, ymin,ymax, zmin,zmax;
  vrpn_float32  fmin, fmax, fcur;
  vrpn_int32    enabled;
  vrpn_float32 baseforce, modforce, base, mode, current;

  static        char    buffer[MAXBUF];   /* Holds data from the client */
  int     bufsize;        /* Number of characters read */
  char    *bufptr;        /* Current location in buffer */

  char header[5000];
  vrpn_float32 **data_arr, *data, *z_data;
  vrpn_int32 iscrap, iscrap0, iscrap1;
  vrpn_int32 lscrap;
  vrpn_float32  fscrap, fscrap0, fscrap1, fscrap2, fscrap3, fscrap4, fscrap5, fscrap6;         /* Scrap variable */
  vrpn_int32 reports, fields;
  vrpn_int32 i, j;
  char string[100], string0[100], string1[100];
  vrpn_float32 P, I, D;
  // ohmmeter resistance variables;
  vrpn_float32 resist, voltage, range, filter;
  vrpn_int32 status;
  // force curve
  vrpn_int32 num_samples, num_halfcycles;

  char *vrpnbuffer = NULL;
  long vbuflen;

  //printf ("\n** msg #%u:\n", ++seqNum);
  /* Read a block from the input file, if there is one */
  bufsize = stm_read_block_from_stream(instream, buffer);
  if (bufsize == -1) {  /* Indicates end of stream */
    return(0);
  }

  /* While the buffer has not been used up, read more reports */
  bufptr = buffer;        /* Point bufptr to buffer start */

  while ( (bufptr-buffer) < bufsize ) {
    int data_type;
    long vrpn_type;
    vrpn_bool needToSend = VRPN_FALSE;
    vrpn_int32 mlen;
    char *mptr;


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
      
      vrpnbuffer = encode_PulseParameters(&vbuflen, pulse_enabled, 
            pulse_bias, pulse_peak, pulse_width);
      vrpn_type = d_PulseParameters_type;
      needToSend = VRPN_TRUE;

      break;

    case STM_STD_DEV_PARAMETERS:
      stm_unbuffer_int (&bufptr, &std_dev_samples);
      stm_unbuffer_float (&bufptr, &std_dev_frequency);
      printf ("STM_STD_DEV_PARAMETERS (%u, %g)\n",
              std_dev_samples, std_dev_frequency);

      break;

    case AFM_FORCE_SET:
      stm_unbuffer_float (&bufptr, &fscrap);

      vrpnbuffer = encode_ForceSet(&vbuflen, fscrap);
      vrpn_type = d_ForceSet_type;
      needToSend = VRPN_TRUE;

      break;

    case AFM_FORCE_SET_FAILURE:
      stm_unbuffer_float (&bufptr, &fscrap);
      vrpnbuffer = encode_ForceSetFailure(&vbuflen, fscrap);
      vrpn_type = d_ForceSetFailure_type;
      needToSend = VRPN_TRUE;
      break;

    case AFM_FORCE_PARAMETERS:
      stm_unbuffer_int (&bufptr, &fmods_enabled);
      stm_unbuffer_float (&bufptr, &fmods_baseforce);
      vrpnbuffer = encode_ForceParameters(&vbuflen, fmods_enabled, 
                 fmods_baseforce);
      vrpn_type = d_ForceParameters_type;
      needToSend = VRPN_TRUE;
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

      now.tv_sec = sec;
      now.tv_usec = usec;
      doTimeCorrection(now);

      vrpnbuffer = encode_WindowScanNM(&vbuflen, x, y, sec, usec, 
                              value, std_dev);
      vrpn_type = d_WindowScanNM_type;
      needToSend = VRPN_TRUE;
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

      now.tv_sec = sec;
      now.tv_usec = usec;
      doTimeCorrection(now);

      vrpnbuffer =  encode_WindowBackscanNM(&vbuflen, x, y, sec, usec,
            value, std_dev);
      vrpn_type = d_WindowBackscanNM_type;
      needToSend = VRPN_TRUE;

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

      now.tv_sec = sec;
      now.tv_usec = usec;
      doTimeCorrection(now);

      vrpnbuffer = encode_PointResultNM(&vbuflen, fx, fy, sec, usec, value, std_dev);
      vrpn_type = d_PointResultNM_type;
      needToSend = VRPN_TRUE;

      break;

    case STM_PULSE_COMPLETED_NM:
      stm_unbuffer_float (&bufptr, &fx);
      stm_unbuffer_float (&bufptr, &fy);
      vrpnbuffer = encode_PulseCompletedNM(&vbuflen, fx, fy);
      vrpn_type = d_PulseCompletedNM_type;
      needToSend = VRPN_TRUE;

      break;

    case STM_PULSE_FAILURE_NM:
      stm_unbuffer_float (&bufptr, &fx);
      stm_unbuffer_float (&bufptr, &fy);
      vrpnbuffer = encode_PulseFailureNM(&vbuflen, fx, fy);
      vrpn_type = d_PulseFailureNM_type;
      needToSend = VRPN_TRUE;

      break;

    case STM_SET_REGION_COMPLETED:
      stm_unbuffer_float (&bufptr, &xmin);
      stm_unbuffer_float (&bufptr, &ymin);
      stm_unbuffer_float (&bufptr, &xmax);
      stm_unbuffer_float (&bufptr, &ymax);
      vrpnbuffer = encode_SetRegionC(&vbuflen, xmin, ymin, xmax, ymax);
      vrpn_type = d_SetRegionCompleted_type;
      needToSend = VRPN_TRUE;

      break;

    case STM_SET_REGION_CLIPPED:
      stm_unbuffer_float (&bufptr, &xmin);
      stm_unbuffer_float (&bufptr, &ymin);
      stm_unbuffer_float (&bufptr, &xmax);
      stm_unbuffer_float (&bufptr, &ymax);
      vrpnbuffer = encode_SetRegionC(&vbuflen, xmin, ymin, xmax, ymax);
      vrpn_type = d_SetRegionClipped_type;
      needToSend = VRPN_TRUE;

      break;

    case STM_TUNNELLING_ATTAINED_NM:
      stm_unbuffer_float (&bufptr, &value);
      printf("STM_TUNNELLING_ATTAINED_NM");
      break;

    case SPM_SCAN_RANGE:
      stm_unbuffer_float (&bufptr, &xmin);
      stm_unbuffer_float (&bufptr, &xmax);
      stm_unbuffer_float (&bufptr, &ymin);
      stm_unbuffer_float (&bufptr, &ymax);
      stm_unbuffer_float (&bufptr, &zmin);
      stm_unbuffer_float (&bufptr, &zmax);
      vrpnbuffer = encode_ScanRange(&vbuflen, xmin, xmax, ymin, ymax, zmin, zmax);
      vrpn_type = d_ScanRange_type;
      needToSend = VRPN_TRUE;

      break;

    case AFM_MOD_FORCE_SET:
      stm_unbuffer_float (&bufptr, &value);
      vrpnbuffer = encode_ModForceSet(&vbuflen, value);
      vrpn_type = d_ModForceSet_type;
      needToSend = VRPN_TRUE;

      break;

    case AFM_MOD_FORCE_SET_FAILURE:
      stm_unbuffer_float (&bufptr, &value);
      vrpnbuffer = encode_ModForceSet(&vbuflen, value);
      vrpn_type = d_ModForceSetFailure_type;
      needToSend = VRPN_TRUE;

      break;

    case AFM_IMG_FORCE_SET:
      stm_unbuffer_float (&bufptr, &value);
      vrpnbuffer = encode_ImgForceSet(&vbuflen, value);
      vrpn_type = d_ImgForceSet_type;
      needToSend = VRPN_TRUE;

      break;

    case AFM_IMG_FORCE_SET_FAILURE:
      stm_unbuffer_float (&bufptr, &value);
      vrpnbuffer = encode_ImgForceSet(&vbuflen, value);
      vrpn_type = d_ImgForceSetFailure_type;
      needToSend = VRPN_TRUE;

      break;

    case AFM_IMG_SET:
      stm_unbuffer_int (&bufptr, &enabled);
      stm_unbuffer_float (&bufptr, &fmin);
      stm_unbuffer_float (&bufptr, &fmax);
      stm_unbuffer_float (&bufptr, &fcur);
      vrpnbuffer = encode_ImgSet(&vbuflen, enabled, fmin, fmax, fcur);
      vrpn_type = d_ImgSet_type;
      needToSend = VRPN_TRUE;

      break;

    case AFM_MOD_SET:
      stm_unbuffer_int (&bufptr, &enabled);
      stm_unbuffer_float (&bufptr, &fmin);
      stm_unbuffer_float (&bufptr, &fmax);
      stm_unbuffer_float (&bufptr, &fcur);
      vrpnbuffer = encode_ModSet(&vbuflen, enabled, fmin, fmax, fcur);
      vrpn_type = d_ModSet_type;
      needToSend = VRPN_TRUE;

      break;

    case AFM_IN_MOD_MODE:
      vrpnbuffer = NULL;
      vbuflen = 0;
      vrpn_type = d_InModMode_type;
      needToSend = VRPN_TRUE;

      break;

    case AFM_IN_IMG_MODE:
      vrpnbuffer = NULL;
      vbuflen = 0;
      vrpn_type = d_InImgMode_type;
      needToSend = VRPN_TRUE;

      break;

    case SPM_RELAX_SET:
      stm_unbuffer_int (&bufptr, &x);
      stm_unbuffer_int (&bufptr, &y);
      vrpnbuffer = encode_RelaxSet(&vbuflen, x, y);
      vrpn_type = d_RelaxSet_type;
      needToSend = VRPN_TRUE;

      break;

    case AFM_IN_MOD_MODE_T:
      // We send out the InModMode message, since the timed one is
      // obsolete (the time is stored in "now" already, so doesn't need
      // to be also in the body of the message.
      stm_unbuffer_int (&bufptr, &sec);
      stm_unbuffer_int (&bufptr, &usec);

      now.tv_sec = sec;
      now.tv_usec = usec;
      doTimeCorrection(now);

      vrpnbuffer = NULL;
      vbuflen = 0;

      vrpn_type = d_InModMode_type;
      needToSend = VRPN_TRUE;

      break;

    case AFM_IN_IMG_MODE_T:
      // We send out the InImgMode message, since the timed one is
      // obsolete (the time is stored in "now" already, so doesn't need
      // to be also in the body of the message.
      stm_unbuffer_int (&bufptr, &sec);
      stm_unbuffer_int (&bufptr, &usec);

      now.tv_sec = sec;
      now.tv_usec = usec;
      doTimeCorrection(now);

      vrpnbuffer = NULL;
      vbuflen = 0;

      vrpn_type = d_InImgMode_type;
      needToSend = VRPN_TRUE;

      break;

    case SPM_RESISTANCE:
      //stm_unbuffer_long (&bufptr, &lscrap);
      stm_unbuffer_int (&bufptr, &lscrap);
      //stm_unbuffer_long (&bufptr, &sec);
      stm_unbuffer_int (&bufptr, &sec);
      //stm_unbuffer_long (&bufptr, &usec);
      stm_unbuffer_int (&bufptr, &usec);
      stm_unbuffer_float (&bufptr, &fscrap);

      now.tv_sec = sec;
      now.tv_usec = usec;
      doTimeCorrection(now);

      vrpnbuffer = encode_Resistance(&vbuflen, lscrap, sec, usec, fscrap);
      vrpn_type = d_Resistance_type;
      needToSend = VRPN_TRUE;

      break;

    case SPM_RESISTANCE_FAILURE:
      //stm_unbuffer_long (&bufptr, &lscrap);
      stm_unbuffer_int (&bufptr, &lscrap);
      vrpnbuffer = encode_ResistanceFailure(&vbuflen, lscrap);
      vrpn_type = d_ResistanceFailure_type;
      needToSend = VRPN_TRUE;

      break;

    case STM_ZIG_RESULT_NM:
      printf ("STM_ZIG_RESULT_NM (");
      stm_unbuffer_float (&bufptr, &fx);
      stm_unbuffer_float (&bufptr, &fy);
      //stm_unbuffer_long (&bufptr, &sec);
      stm_unbuffer_int (&bufptr, &sec);
      //stm_unbuffer_long (&bufptr, &usec);
      stm_unbuffer_int (&bufptr, &usec);
      stm_unbuffer_float (&bufptr, &fscrap0);
      stm_unbuffer_float (&bufptr, &fscrap1);
      stm_unbuffer_float (&bufptr, &fscrap2);
      stm_unbuffer_float (&bufptr, &fscrap3);

      now.tv_sec = sec;
      now.tv_usec = usec;
      doTimeCorrection(now);

      vrpnbuffer = encode_ResultNM(&vbuflen, fx, fy, sec, usec, fscrap0, fscrap1, fscrap2, fscrap3);
      vrpn_type = d_ZigResultNM_type;
      needToSend = VRPN_TRUE;

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
      printf("STM_SCAN_PARAMETERS, %d\n", iscrap);
      break;

    case SPM_BLUNT_RESULT_NM:
      stm_unbuffer_float (&bufptr, &fx);
      stm_unbuffer_float (&bufptr, &fy);
      //stm_unbuffer_long (&bufptr, &sec);
      stm_unbuffer_int (&bufptr, &sec);
      //stm_unbuffer_long (&bufptr, &usec);
      stm_unbuffer_int (&bufptr, &usec);
      stm_unbuffer_float (&bufptr, &fscrap0);
      stm_unbuffer_float (&bufptr, &fscrap1);
      stm_unbuffer_float (&bufptr, &fscrap2);
      stm_unbuffer_float (&bufptr, &fscrap3);

      now.tv_sec = sec;
      now.tv_usec = usec;
      doTimeCorrection(now);

      vrpnbuffer = encode_ResultNM(&vbuflen, fx, fy, sec, usec, fscrap0, fscrap1, fscrap2, fscrap3);
      vrpn_type = d_BluntResultNM_type;
      needToSend = VRPN_TRUE;

      break;

    case SPM_WINDOW_LINE_DATA:
      stm_unbuffer_int (&bufptr, &x);
      stm_unbuffer_int (&bufptr, &y);
      stm_unbuffer_int (&bufptr, &dx);
      stm_unbuffer_int (&bufptr, &dy);
      stm_unbuffer_int (&bufptr, &reports);
      stm_unbuffer_int (&bufptr, &fields);
      //stm_unbuffer_long (&bufptr, &sec);
      stm_unbuffer_int (&bufptr, &sec);
      //stm_unbuffer_long (&bufptr, &usec);
      stm_unbuffer_int (&bufptr, &usec);
      
      data = new float[fields*reports];
      data_arr = new float*[fields];
      for (j = 0; j < fields; j++) {
          data_arr[j] = &(data[j*reports]);
      }

      for (i = 0; i < reports; i++) {
        for (j = 0; j < fields; j++) {
          stm_unbuffer_float (&bufptr, &(data_arr[j][i]));
        }
      }

      now.tv_sec = sec;
      now.tv_usec = usec;
      doTimeCorrection(now);

      vrpnbuffer = encode_WindowLineData(&vbuflen, x, y, dx, dy, reports, fields, sec, usec, data_arr);
      vrpn_type = d_WindowLineData_type;
      needToSend = VRPN_TRUE;

      delete [] data;
      delete [] data_arr;

      break;

    case SPM_HELLO_MESSAGE:
      stm_unbuffer_chars (&bufptr, string0, 4);
      stm_unbuffer_chars (&bufptr, string1, 64);
      stm_unbuffer_int (&bufptr, &iscrap0);
      stm_unbuffer_int (&bufptr, &iscrap1);

      vrpnbuffer = encode_HelloMessage(&vbuflen, string0, string1, iscrap0, iscrap1); 
      vrpn_type = d_HelloMessage_type;
      needToSend = VRPN_TRUE;

      break;

    case SPM_SCAN_DATASETS:

      // encode_ScanDataset is implemented in nmm_Microscope_Topometrix.C

      stm_unbuffer_int (&bufptr, &iscrap);
      vbuflen = sizeof(vrpn_int32) + ( 2*64*sizeof(char) + 2*sizeof(float) ) * iscrap;
      vrpnbuffer = new char[vbuflen];
      mlen = vbuflen;
      mptr = vrpnbuffer;
      
      vrpn_buffer(&mptr, &mlen, iscrap);

      for (i = 0; i < iscrap; i++) {
        stm_unbuffer_chars (&bufptr, string, 64);
        vrpn_buffer(&mptr, &mlen, string, 64);
        stm_unbuffer_chars (&bufptr, string, 64);
        vrpn_buffer(&mptr, &mlen, string, 64);
        stm_unbuffer_float (&bufptr, &fscrap);
        vrpn_buffer(&mptr, &mlen, fscrap);
        stm_unbuffer_float (&bufptr, &fscrap);
        vrpn_buffer(&mptr, &mlen, fscrap);
      }
      vrpn_type = d_ScanDataset_type;
      needToSend = VRPN_TRUE;

      break;

    case SPM_REPORT_SLOW_SCAN:
      stm_unbuffer_int (&bufptr, &iscrap);
      vrpnbuffer = encode_ReportSlowScan(&vbuflen, iscrap);
      vrpn_type = d_ReportSlowScan_type;
      needToSend = VRPN_TRUE;

      break;

    case SPM_CLIENT_HELLO:
      printf ("Found SPM_CLIENT_HELLO;  issuing HelloMessage.\n");
      stm_unbuffer_chars (&bufptr, string0, 4);
      stm_unbuffer_chars (&bufptr, string1, 64);
      stm_unbuffer_int (&bufptr, &iscrap0);
      stm_unbuffer_int (&bufptr, &iscrap1);

      vrpnbuffer = encode_HelloMessage(&vbuflen, string0, string1, iscrap0, iscrap1);
      vrpn_type = d_HelloMessage_type;
      needToSend = VRPN_TRUE;

      break;

    case SPM_POINT_DATASETS:

      // encode_ScanDataset is implemented in nmm_Microscope_Topometrix.C

      stm_unbuffer_int (&bufptr, &reports);
      vbuflen = sizeof(vrpn_int32) + ( 2*64*sizeof(char) + sizeof(vrpn_int32) + 2*sizeof(float) ) * reports;
      vrpnbuffer = new char[vbuflen];
      mlen = vbuflen;
      mptr = vrpnbuffer;

      vrpn_buffer(&mptr, &mlen, reports);
      for (i = 0; i < reports; i++) {
        stm_unbuffer_chars (&bufptr, string, 64);
        vrpn_buffer(&mptr, &mlen, string, 64);
        stm_unbuffer_chars (&bufptr, string, 64);
        vrpn_buffer(&mptr, &mlen, string, 64);
        stm_unbuffer_int (&bufptr, &iscrap);
        vrpn_buffer(&mptr, &mlen, iscrap);
        stm_unbuffer_float (&bufptr, &fscrap);
        vrpn_buffer(&mptr, &mlen, fscrap);
        stm_unbuffer_float (&bufptr, &fscrap);
        vrpn_buffer(&mptr, &mlen, fscrap);
      }
      vrpn_type = d_PointDataset_type;
      needToSend = VRPN_TRUE;

      break;

    case SPM_POINT_RESULT_DATA:
      stm_unbuffer_float (&bufptr, &fx);
      stm_unbuffer_float (&bufptr, &fy);
      //stm_unbuffer_long (&bufptr, &sec);
      stm_unbuffer_int (&bufptr, &sec);
      //stm_unbuffer_long (&bufptr, &usec);
      stm_unbuffer_int (&bufptr, &usec);
      stm_unbuffer_int (&bufptr, &reports);

      data = new float[reports];
      for (i = 0; i < reports; i++) {
        stm_unbuffer_float (&bufptr, &(data[i]));
      }

      now.tv_sec = sec;
      now.tv_usec = usec;
      doTimeCorrection(now);

      vrpnbuffer = encode_ResultData(&vbuflen, fx, fy, sec, usec, reports, data);
      vrpn_type = d_PointResultData_type;
      needToSend = VRPN_TRUE;

      delete [] data;

      break;

    case SPM_PID_PARAMETERS:
      stm_unbuffer_float (&bufptr, &P);
      stm_unbuffer_float (&bufptr, &I);
      stm_unbuffer_float (&bufptr, &D);

      vrpnbuffer = encode_PidParameters(&vbuflen, P, I, D);
      vrpn_type = d_PidParameters_type;
      needToSend = VRPN_TRUE;

      break;

    case SPM_SCANRATE_PARAMETER:
      stm_unbuffer_float (&bufptr, &fscrap);
      vrpnbuffer = encode_ScanrateParameter(&vbuflen, fscrap);
      vrpn_type = d_ScanrateParameter_type;
      needToSend = VRPN_TRUE;

      break;

    case SPM_REPORT_GRID_SIZE:
      stm_unbuffer_int (&bufptr, &x);
      stm_unbuffer_int (&bufptr, &y);
      vrpnbuffer = encode_ReportGridSize(&vbuflen, x, y);
      vrpn_type = d_ReportGridSize_type;
      needToSend = VRPN_TRUE;

      break;

    case AFM_IN_SEWING_MODE:
      stm_unbuffer_float (&bufptr, &fscrap0);
      stm_unbuffer_float (&bufptr, &fscrap1);
      stm_unbuffer_float (&bufptr, &fscrap2);
      stm_unbuffer_float (&bufptr, &fscrap3);
      stm_unbuffer_float (&bufptr, &fscrap4);
      stm_unbuffer_float (&bufptr, &fscrap5);
      stm_unbuffer_float (&bufptr, &fscrap6);
      vrpnbuffer = encode_InSewingStyle(&vbuflen, fscrap0, fscrap1, fscrap2, fscrap3, fscrap4, fscrap5, fscrap6);
      vrpn_type = d_InSewingStyle_type;
      needToSend = VRPN_TRUE;

      break;

    case AFM_IN_CONTACT_MODE:
      stm_unbuffer_float (&bufptr, &P);
      stm_unbuffer_float (&bufptr, &I);
      stm_unbuffer_float (&bufptr, &D);
      stm_unbuffer_float (&bufptr, &fscrap);
      vrpnbuffer = encode_InContactMode(&vbuflen, P, I, D, fscrap);
      vrpn_type = d_InContactMode_type;
      needToSend = VRPN_TRUE;

      break;

    case AFM_IN_TAPPING_MODE:
      stm_unbuffer_float (&bufptr, &P);
      stm_unbuffer_float (&bufptr, &I);
      stm_unbuffer_float (&bufptr, &D);
      stm_unbuffer_float (&bufptr, &fscrap0);
      stm_unbuffer_float (&bufptr, &fscrap1);
      vrpnbuffer = encode_InTappingMode(&vbuflen, P, I, D, fscrap0, fscrap1);
      vrpn_type = d_InTappingMode_type;
      needToSend = VRPN_TRUE;

      break;

    case SPM_BOTTOM_PUNCH_RESULT_DATA:
      stm_unbuffer_float (&bufptr, &fx);
      stm_unbuffer_float (&bufptr, &fy);
      //stm_unbuffer_long (&bufptr, &sec);
      //stm_unbuffer_long (&bufptr, &usec);
      stm_unbuffer_int (&bufptr, &sec);
      stm_unbuffer_int (&bufptr, &usec);
      stm_unbuffer_int (&bufptr, &reports);

      data = new float[reports];

      for (i=0; i<reports; i++) {
        stm_unbuffer_float (&bufptr, &(data[i]));

      }

      now.tv_sec = sec;
      now.tv_usec = usec;
      doTimeCorrection(now);

      vrpnbuffer = encode_ResultData(&vbuflen, fx, fy, sec, usec, reports, data);
      vrpn_type = d_BottomPunchResultData_type;
      needToSend = VRPN_TRUE;

      delete [] data;

      break;

    case SPM_TOP_PUNCH_RESULT_DATA:
      stm_unbuffer_float (&bufptr, &fx);
      stm_unbuffer_float (&bufptr, &fy);
      //stm_unbuffer_long (&bufptr, &sec);
      //stm_unbuffer_long (&bufptr, &usec);
      stm_unbuffer_int (&bufptr, &sec);
      stm_unbuffer_int (&bufptr, &usec);
      stm_unbuffer_int (&bufptr, &reports);

      data = new float[reports];

      for (i=0; i<reports; i++) {
        stm_unbuffer_float (&bufptr, &(data[i]));

      }

      now.tv_sec = sec;
      now.tv_usec = usec;
      doTimeCorrection(now);

      vrpnbuffer = encode_ResultData(&vbuflen, fx, fy, sec, usec, reports, data);
      vrpn_type = d_TopPunchResultData_type;
      needToSend = VRPN_TRUE;

      delete [] data;

      break;

    case SPM_VOLTSOURCE_ENABLED:
      stm_unbuffer_int (&bufptr, &iscrap);
      stm_unbuffer_float (&bufptr, &fscrap);
      vrpnbuffer = encode_EnableVoltsource(&vbuflen, iscrap, fscrap);
      vrpn_type = d_EnableVoltsource_type;
      needToSend = VRPN_TRUE;

      break;

    case SPM_VOLTSOURCE_DISABLED:
      stm_unbuffer_int (&bufptr, &iscrap);
      vrpnbuffer = encode_DisableVoltsource(&vbuflen, iscrap);
      vrpn_type = d_DisableVoltsource_type;
      needToSend = VRPN_TRUE;

      break;

    case SPM_AMP_ENABLED:
      stm_unbuffer_int (&bufptr, &iscrap0);
      stm_unbuffer_float (&bufptr, &fscrap0);
      printf ("SPM_AMP_ENABLED (%d, %g, ", iscrap0, fscrap0);
      stm_unbuffer_float (&bufptr, &fscrap1);
      stm_unbuffer_int (&bufptr, &iscrap1);
      vrpnbuffer = encode_EnableAmp(&vbuflen,iscrap0, fscrap0, fscrap1, iscrap1); 
      vrpn_type = d_EnableAmp_type;
      needToSend = VRPN_TRUE;

      break;

    case SPM_AMP_DISABLED:
      stm_unbuffer_int (&bufptr, &iscrap);
      vrpnbuffer = encode_DisableAmp(&vbuflen,iscrap); 
      vrpn_type = d_DisableAmp_type;
      needToSend = VRPN_TRUE;

      break;

    case SPM_STARTING_TO_RELAX:
      //stm_unbuffer_long (&bufptr, &sec);
      stm_unbuffer_int (&bufptr, &sec);
      //stm_unbuffer_long (&bufptr, &usec);
      stm_unbuffer_int (&bufptr, &usec);

      now.tv_sec = sec;
      now.tv_usec = usec;
      doTimeCorrection(now);

      vrpnbuffer = encode_StartingToRelax(&vbuflen, sec, usec);
      vrpn_type = d_StartingToRelax_type;
      needToSend = VRPN_TRUE;

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

      stm_unbuffer_chars (&bufptr, header, lscrap);
      vrpnbuffer = encode_TopoFileHeader(&vbuflen, header, lscrap);
      vrpn_type = d_TopoFileHeader_type;
      needToSend = VRPN_TRUE;

      break;

    case SPM_SERVER_PACKET_TIMESTAMP:
      stm_unbuffer_int (&bufptr, &sec);
      stm_unbuffer_int (&bufptr, &usec);

      now.tv_sec = sec;
      now.tv_usec = usec;
      doTimeCorrection(now);

      vrpnbuffer = encode_ServerPacketTimestamp(&vbuflen, sec, usec);
      vrpn_type = d_ServerPacketTimestamp_type;
      needToSend = VRPN_TRUE;

      break;

    case OHM_RESISTANCE:
      stm_unbuffer_int (&bufptr, &iscrap);
      //stm_unbuffer_long (&bufptr, &sec);
      //stm_unbuffer_long (&bufptr, &usec);
      stm_unbuffer_float (&bufptr, &resist);
      stm_unbuffer_float (&bufptr, &voltage);
      stm_unbuffer_float (&bufptr, &range);
      stm_unbuffer_float (&bufptr, &filter);

      now.tv_sec = sec;
      now.tv_usec = usec;

      // record the uncorrected time in case we want it later
      vrpnbuffer = encode_RecordResistance(&vbuflen, iscrap, now, resist, voltage, range, filter);

      doTimeCorrection(now);

      vrpn_type = d_RecordResistance_type;
      needToSend = VRPN_TRUE;

      break;

    case SPM_FORCE_CURVE_DATA:
      stm_unbuffer_float (&bufptr, &fx);
      stm_unbuffer_float (&bufptr, &fy);
      //stm_unbuffer_long (&bufptr, &num_samples);
      stm_unbuffer_int (&bufptr, &num_samples);
      //stm_unbuffer_long (&bufptr, &num_halfcycles);
      stm_unbuffer_int (&bufptr, &num_halfcycles);
      //stm_unbuffer_long (&bufptr, &sec);
      stm_unbuffer_int (&bufptr, &sec);
      //stm_unbuffer_long (&bufptr, &usec);
      stm_unbuffer_int (&bufptr, &usec);

      z_data = new float[num_samples];
      data = new float[num_samples*num_halfcycles];
      data_arr = new float*[num_halfcycles];
      for (j = 0; j < num_halfcycles; j++)
          data_arr[j] = &(data[num_samples*j]);


      for (i = 0; i < num_samples; i++) {
         stm_unbuffer_float (&bufptr, &(z_data[i]));
         for (j = 0; j < num_halfcycles; j++) {
            stm_unbuffer_float (&bufptr, &(data_arr[j][i]));
         }
      }

      now.tv_sec = sec;
      now.tv_usec = usec;
      doTimeCorrection(now);

      vrpnbuffer = encode_ForceCurveData(&vbuflen, fx, fy, num_samples, num_halfcycles, sec, usec, z_data, data_arr);
      vrpn_type = d_ForceCurveData_type;
      needToSend = VRPN_TRUE;

      delete [] data;
      delete [] data_arr;
      delete [] z_data;

      break;

    case SPM_IN_SPECTROSCOPY_MODE:

      float setpoint, startDelay, zStart, zEnd, zPullback, forceLimit, distBetweenFC,
            sample_speed, pullback_speed, start_speed, feedback_speed, sample_delay, pullback_delay, feedback_delay;
      vrpn_int32 avg_num;

      stm_unbuffer_float (&bufptr, &setpoint);
      stm_unbuffer_float (&bufptr, &startDelay);
      stm_unbuffer_float (&bufptr, &zStart);
      stm_unbuffer_float (&bufptr, &zEnd);
      stm_unbuffer_float (&bufptr, &zPullback);
      stm_unbuffer_float (&bufptr, &forceLimit);
      stm_unbuffer_float (&bufptr, &distBetweenFC);
      stm_unbuffer_int (&bufptr, &num_samples);
      stm_unbuffer_int (&bufptr, &num_halfcycles);
      stm_unbuffer_float (&bufptr, &sample_speed);
      stm_unbuffer_float (&bufptr, &pullback_speed);
      stm_unbuffer_float (&bufptr, &start_speed);
      stm_unbuffer_float (&bufptr, &feedback_speed);
      stm_unbuffer_int (&bufptr, &avg_num);
      stm_unbuffer_float (&bufptr, &sample_delay);
      stm_unbuffer_float (&bufptr, &pullback_delay);
      stm_unbuffer_float (&bufptr, &feedback_delay);
      vrpnbuffer = encode_InSpectroscopyMode(&vbuflen, setpoint, startDelay, zStart, zEnd, zPullback, forceLimit,
			distBetweenFC, num_samples, num_halfcycles, sample_speed, pullback_speed, start_speed,
			feedback_speed, avg_num, sample_delay, pullback_delay, feedback_delay);
      vrpn_type = d_InSpectroscopyMode_type;
      needToSend = VRPN_TRUE;

      break;

    case OHM_RESISTANCE_WSTATUS:
      stm_unbuffer_int (&bufptr, &iscrap);
      stm_unbuffer_int (&bufptr, &sec);
      stm_unbuffer_int (&bufptr, &usec);
      stm_unbuffer_float (&bufptr, &resist);
      stm_unbuffer_float (&bufptr, &voltage);
      stm_unbuffer_float (&bufptr, &range);
      stm_unbuffer_float (&bufptr, &filter);
      stm_unbuffer_int (&bufptr, &status);
      printf ("OHM_RESISTANCE (%d, %u:%u, %g, %g, %g, %g, ",
                  iscrap, sec-start_time_for_stream.tv_sec, usec,
                  resist, voltage, range, filter);
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

      now.tv_sec = sec;
      now.tv_usec = usec;
      doTimeCorrection(now);

      break;

    /* Parameters echoed back by the server to record info in the stream */
    case AFM_BASE_MOD_PARAMETERS:
      stm_unbuffer_float (&bufptr, &baseforce);
      stm_unbuffer_float (&bufptr, &modforce);
      vrpnbuffer = encode_BaseModParameters(&vbuflen, baseforce, modforce);
      vrpn_type = d_BaseModParameters_type;
      needToSend = VRPN_TRUE;

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
      vrpnbuffer = encode_ForceSettings(&vbuflen,  base, mode, current);
      vrpn_type = d_ForceSettings_type;
      needToSend = VRPN_TRUE;

      break;

    case SPM_REFRESH_GRID:
      printf ("SPM_REFRESH_GRID\n");
      break;

    case NANO_RECV_TIMESTAMP:
      stm_unbuffer_int (&bufptr, &sec);
      stm_unbuffer_int (&bufptr, &usec);

      now.tv_sec = sec;
      now.tv_usec = usec;

      vrpnbuffer = encode_RecvTimestamp(&vbuflen, now);

      doTimeCorrection(now);

      vrpn_type = d_RecvTimestamp_type;
      needToSend = VRPN_TRUE;

      break;


    case FAKE_SPM_SEND_TIMESTAMP:
      stm_unbuffer_int (&bufptr, &sec);
      stm_unbuffer_int (&bufptr, &usec);

      now.tv_sec = sec;
      now.tv_usec = usec;

      vrpnbuffer = encode_FakeSendTimestamp(&vbuflen, now);

      doTimeCorrection(now);

      vrpn_type = d_FakeSendTimestamp_type;
      needToSend = VRPN_TRUE;

      break;

    case SPM_UDP_SEQ_NUM:
      stm_unbuffer_int (&bufptr, &lscrap);
      vrpn_type = d_UdpSeqNum_type;
      vrpnbuffer = encode_UdpSeqNum(&vbuflen, lscrap);
      vrpn_type = d_UdpSeqNum_type;
      needToSend = VRPN_TRUE;

      break;

    default:
      fprintf(stderr, "=> Unknown data type returned (%d)\n",
              data_type);
      exit(-1);
    }

    if (needToSend) {
      d_connection->pack_message(vbuflen, now, vrpn_type, 
	d_myId, vrpnbuffer, vrpn_CONNECTION_RELIABLE);

      needToSend = VRPN_FALSE;
    }
    if (vrpnbuffer) {
      delete [] vrpnbuffer;
      vrpnbuffer = NULL;
    }

    d_connection->mainloop();
  }

  return(1);

} /* translate_packet */


timeval nmm_Microscope_Translator::getTimeElapsed()
{
    timeval result;
    result = now;

    result = vrpn_TimevalDiff(result, start_time_for_vrpn);

    return result;
}

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
  int   localvalue;
  localvalue = ntohl(*(int*)(*bufptr));
  *value = *(float*)(&localvalue);
#endif
  *bufptr += sizeof(float);
}

void stm_unbuffer_chars(BUFPTR* bufptr, char *c, int len) {
  memcpy(c, *bufptr, len);
  *bufptr += len;
}


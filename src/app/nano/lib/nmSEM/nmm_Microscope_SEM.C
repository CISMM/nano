#include <vrpn_Connection.h>
#include <vrpn_Types.h>
#include "nmm_Microscope_SEM.h"
#include "nmb_Image.h"

/* nmm_Microscope_SEM
*/

nmm_Microscope_SEM::nmm_Microscope_SEM (
    const char * /*name*/,
    vrpn_Connection *c)
//  d_connection (c),
//  d_fileController (new vrpn_File_Controller (c))
{
//  char * servicename;
//  servicename = vrpn_copy_service_name(name);

  if (c) {
//    d_myId = d_connection->register_sender(servicename);
    d_SetResolution_type = c->register_message_type
		("nmmMicroscopeSEM SetResolution");
    d_SetPixelIntegrationTime_type = c->register_message_type
		("nmmMicroscopeSEM SetPixelIntegrationTime");
    d_SetInterPixelDelayTime_type = c->register_message_type
                ("nmmMicroscopeSEM SetInterPixelDelayTime");
    d_RequestScan_type = c->register_message_type
                ("nmmMicroscopeSEM RequestScan");
    d_SetPointDwellTime_type = c->register_message_type
                ("nmmMicroscopeSEM SetPointDwellTime");
    d_SetBeamBlankEnable_type = c->register_message_type
                ("nmmMicroscopeSEM SetBeamBlankEnable");
    d_GoToPoint_type = c->register_message_type
                ("nmmMicroscopeSEM GoToPoint");
    d_SetRetraceDelays_type = c->register_message_type
                ("nmmMicroscopeSEM SetRetraceDelays");
    d_SetDACParams_type = c->register_message_type
                ("nmmMicroscopeSEM SetDACParams");
    d_SetExternalScanControlEnable_type = c->register_message_type
                ("nmmMicroscopeSEM SetExternalScanControlEnable");

    d_ClearExposePattern_type = c->register_message_type
                ("nmmMicroscopeSEM ClearExposePattern");
    d_AddPolygon_type = c->register_message_type
                ("nmmMicroscopeSEM AddPolygon");
    d_AddPolyline_type = c->register_message_type
                ("nmmMicroscopeSEM AddPolyline");
    d_AddDumpPoint_type = c->register_message_type
                ("nmmMicroscopeSEM AddDumpPoint");
    d_ExposePattern_type = c->register_message_type
                ("nmmMicroscopeSEM ExposePattern");
    d_ExposureTimingTest_type = c->register_message_type
                ("nmmMicroscopeSEM ExposureTimingTest");

    d_SetBeamCurrent_type = c->register_message_type
                ("nmmMicroscopeSEM SetBeamCurrent");
    d_SetBeamWidth_type = c->register_message_type
                ("nmmMicroscopeSEM SetBeamWidth");

    d_SetPointReportEnable_type = c->register_message_type
                ("nmmMicroscopeSEM SetPointReportEnable");

    d_SetDotSpacing_type = c->register_message_type
                ("nmmMicroscopeSEM SetDotSpacing");
    d_SetLineSpacing_type = c->register_message_type
                ("nmmMicroscopeSEM SetLineSpacing");
    d_SetLinearExposure_type = c->register_message_type
                ("nmmMicroscopeSEM SetLinearExposure");
    d_SetAreaExposure_type = c->register_message_type
                ("nmmMicroscopeSEM SetAreaExposure");

    d_SetMagnification_type = c->register_message_type
                ("nmmMicroscopeSEM SetMagnification");

    d_ReportResolution_type = c->register_message_type
		("nmmMicroscopeSEM ReportResolution");
    d_ReportPixelIntegrationTime_type = c->register_message_type
                ("nmmMicroscopeSEM ReportPixelIntegrationTime");
    d_ReportInterPixelDelayTime_type = c->register_message_type
                ("nmmMicroscopeSEM ReportInterPixelDelayTime");
    d_ScanlineData_type = c->register_message_type
		("nmmMicroscopeSEM ScanlineData");
    d_ReportPointDwellTime_type = c->register_message_type
                ("nmmMicroscopeSEM ReportPointDwellTime");
    d_ReportBeamBlankEnable_type = c->register_message_type
                ("nmmMicroscopeSEM ReportBeamBlankEnable");
    d_ReportMaxScanSpan_type = c->register_message_type
                ("nmmMicroscopeSEM ReportMaxScanSpan");
    d_ReportBeamLocation_type = c->register_message_type
                ("nmmMicroscopeSEM ReportBeamLocation");
    d_ReportRetraceDelays_type = c->register_message_type
                ("nmmMicroscopeSEM ReportRetraceDelays");
    d_ReportDACParams_type = c->register_message_type
                ("nmmMicroscopeSEM ReportDACParams");
    d_ReportExternalScanControlEnable_type = c->register_message_type
                ("nmmMicroscopeSEM ReportExternalScanControlEnable");
    d_ReportMagnification_type = c->register_message_type
                ("nmmMicroscopeSEM ReportMagnification");
    d_ReportBeamCurrent_type = c->register_message_type
                ("nmmMicroscopeSEM ReportBeamCurrent");
    d_ReportBeamWidth_type = c->register_message_type
                ("nmmMicroscopeSEM ReportBeamWidth");
    d_ReportExposureStatus_type = c->register_message_type
                ("nmmMicroscopeSEM ReportExposureStatus");
	d_ReportTimingStatus_type = c->register_message_type
                ("nmmMicroscopeSEM ReportTimingStatus");
	d_ReportPointReportEnable_type = c->register_message_type
                ("nmmMicroscopeSEM ReportPointReportEnable");
	d_ReportDotSpacing_type = c->register_message_type
                ("nmmMicroscopeSEM ReportDotSpacing");
    d_ReportLineSpacing_type = c->register_message_type
                ("nmmMicroscopeSEM ReportLineSpacing");
  }
}


nmm_Microscope_SEM::~nmm_Microscope_SEM (void)
{
}

/*   
vrpn_int32 nmm_Microscope_SEM::mainloop(void)
{
  if (d_connection){
    if ((d_connection->mainloop()) == -1)
       return -1;
  }
  return 0;
}
*/

// message encode, decode functions
// (client-->server)
char * nmm_Microscope_SEM::encode_SetResolution 
		(vrpn_int32 *len, vrpn_int32 x, vrpn_int32 y)
{
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 2 * sizeof(vrpn_int32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope_SEM::encode_SetResolution:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, x);
    vrpn_buffer(&mptr, &mlen, y);
  }

  return msgbuf;

}

vrpn_int32 nmm_Microscope_SEM::decode_SetResolution(const char **buf, 
					vrpn_int32 *x, vrpn_int32 *y)
{
  if ((vrpn_unbuffer(buf, x)) == -1) {
    return -1;
  }
  if ((vrpn_unbuffer(buf, y)) == -1) {
    return -1;
  }

  return 0;
}

char * nmm_Microscope_SEM::encode_SetPixelIntegrationTime (vrpn_int32 *len, 
					vrpn_int32 time_nsec)
{
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 1 * sizeof(vrpn_int32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope_SEM::encode_SetPixelIntegrationTime:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, time_nsec);
  }

  return msgbuf;
}

vrpn_int32 nmm_Microscope_SEM::decode_SetPixelIntegrationTime (const char **buf,
					vrpn_int32 *time_nsec)
{
  if ((vrpn_unbuffer(buf, time_nsec)) == -1) {
    return -1;
  }

  return 0;
}

char * nmm_Microscope_SEM::encode_SetInterPixelDelayTime (vrpn_int32 *len,
					vrpn_int32 time_nsec)
{
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 1 * sizeof(vrpn_int32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope_SEM::encode_SetInterPixelDelayTime:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, time_nsec);
  }

  return msgbuf;
}

vrpn_int32 nmm_Microscope_SEM::decode_SetInterPixelDelayTime (const char **buf,
					vrpn_int32 *time_nsec)
{
  if ((vrpn_unbuffer(buf, time_nsec)) == -1) {
    return -1;
  }

  return 0;
}

char * nmm_Microscope_SEM::encode_RequestScan(vrpn_int32 *len, 
					vrpn_int32 nscans)
{
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 1 * sizeof(vrpn_int32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope_SEM::encode_RequestScan:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, nscans);
  }

  return msgbuf;
}

vrpn_int32 nmm_Microscope_SEM::decode_RequestScan (const char **buf, 
	vrpn_int32 *nscans) 
{ 
  vrpn_int32 val;
  if ((vrpn_unbuffer(buf, &val)) == -1) {
    return -1;
  }
  *nscans = val;

  return 0;
}

char * nmm_Microscope_SEM::encode_SetPointDwellTime (vrpn_int32 *len,
                                            vrpn_int32 time_nsec)
{
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 1 * sizeof(vrpn_int32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope_SEM::encode_SetPointDwellTime:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, time_nsec);
  }

  return msgbuf;
}

vrpn_int32 nmm_Microscope_SEM::decode_SetPointDwellTime (const char **buf,
                                            vrpn_int32 *time_nsec)
{
  if ((vrpn_unbuffer(buf, time_nsec)) == -1) {
    return -1;
  }

  return 0;
}

char * nmm_Microscope_SEM::encode_SetBeamBlankEnable (vrpn_int32 *len,
                                             vrpn_int32 enable)
{
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 1 * sizeof(vrpn_int32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope_SEM::encode_SetBeamBlankEnable:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, enable);
  }

  return msgbuf;
}

vrpn_int32 nmm_Microscope_SEM::decode_SetBeamBlankEnable (const char **buf,
                                            vrpn_int32 *enable)
{
  if ((vrpn_unbuffer(buf, enable)) == -1) {
    return -1;
  }

  return 0;
}

char * nmm_Microscope_SEM::encode_GoToPoint (vrpn_int32 *len,
                              vrpn_int32 x, vrpn_int32 y)
{
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 2 * sizeof(vrpn_int32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope_SEM::encode_GoToPoint:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, x);
    vrpn_buffer(&mptr, &mlen, y);
  }

  return msgbuf;

}

vrpn_int32 nmm_Microscope_SEM::decode_GoToPoint (const char **buf,
                                 vrpn_int32 *x, vrpn_int32 *y)
{
  if ((vrpn_unbuffer(buf, x)) == -1) {
    return -1;
  }
  if ((vrpn_unbuffer(buf, y)) == -1) {
    return -1;
  }

  return 0;
}

char * nmm_Microscope_SEM::encode_SetRetraceDelays (vrpn_int32 *len,
                           vrpn_int32 h_time_nsec, vrpn_int32 v_time_nsec)
{
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 2 * sizeof(vrpn_int32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope_SEM::encode_SetRetraceDelays:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, h_time_nsec);
    vrpn_buffer(&mptr, &mlen, v_time_nsec);
  }

  return msgbuf;

}

vrpn_int32 nmm_Microscope_SEM::decode_SetRetraceDelays (const char **buf,
                           vrpn_int32 *h_time_nsec, vrpn_int32 *v_time_nsec)
{
  if ((vrpn_unbuffer(buf, h_time_nsec)) == -1) {
    return -1;
  }
  if ((vrpn_unbuffer(buf, v_time_nsec)) == -1) {
    return -1;
  }

  return 0;
}

char * nmm_Microscope_SEM::encode_SetDACParams (vrpn_int32 *len,
                 vrpn_int32 x_gain, vrpn_int32 x_offset,
                 vrpn_int32 y_gain, vrpn_int32 y_offset,
                 vrpn_int32 z_gain, vrpn_int32 z_offset)
{
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 6 * sizeof(vrpn_int32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope_SEM::encode_SetDACParams:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, x_gain);
    vrpn_buffer(&mptr, &mlen, x_offset);
    vrpn_buffer(&mptr, &mlen, y_gain);
    vrpn_buffer(&mptr, &mlen, y_offset);
    vrpn_buffer(&mptr, &mlen, z_gain);
    vrpn_buffer(&mptr, &mlen, z_offset);
  }

  return msgbuf;

}

vrpn_int32 nmm_Microscope_SEM::decode_SetDACParams (const char **buf,
                 vrpn_int32 *x_gain, vrpn_int32 *x_offset,
                 vrpn_int32 *y_gain, vrpn_int32 *y_offset,
                 vrpn_int32 *z_gain, vrpn_int32 *z_offset)
{
  if ((vrpn_unbuffer(buf, x_gain)) == -1) {
    return -1;
  }
  if ((vrpn_unbuffer(buf, x_offset)) == -1) {
    return -1;
  }
  if ((vrpn_unbuffer(buf, y_gain)) == -1) {
    return -1;
  }
  if ((vrpn_unbuffer(buf, y_offset)) == -1) {
    return -1;
  }
  if ((vrpn_unbuffer(buf, z_gain)) == -1) {
    return -1;
  }
  if ((vrpn_unbuffer(buf, z_offset)) == -1) {
    return -1;
  }

  return 0;
}

char * nmm_Microscope_SEM::encode_SetExternalScanControlEnable (vrpn_int32 *len,
                                             vrpn_int32 enable)
{
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 1 * sizeof(vrpn_int32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope_SEM::encode_SetExternalScanControlEnable:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, enable);
  }

  return msgbuf;
}

vrpn_int32 nmm_Microscope_SEM::decode_SetExternalScanControlEnable (
                                            const char **buf,
                                            vrpn_int32 *enable)
{
  if ((vrpn_unbuffer(buf, enable)) == -1) {
    return -1;
  }

  return 0;
}


char * nmm_Microscope_SEM::encode_AddPolygon (vrpn_int32 *len,
         vrpn_float32 exposure_uCoul_per_cm2,
         vrpn_int32 numPoints,
         vrpn_float32 *x_nm, vrpn_float32 *y_nm)
{
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = (2*numPoints + 1)*sizeof(vrpn_float32)+sizeof(vrpn_int32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope_SEM::encode_AddPolygon:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, exposure_uCoul_per_cm2);
    vrpn_buffer(&mptr, &mlen, numPoints);
    int i;
    for (i = 0; i < numPoints; i++) {
      vrpn_buffer(&mptr, &mlen, x_nm[i]);
      vrpn_buffer(&mptr, &mlen, y_nm[i]);
    }
  }

  return msgbuf;
}

vrpn_int32 nmm_Microscope_SEM::decode_AddPolygonHeader (
                                            const char **buf,
         vrpn_float32 *exposure_uCoul_per_cm2,
         vrpn_int32 *numPoints)
{
  if ((vrpn_unbuffer(buf, exposure_uCoul_per_cm2) == -1) ||
      (vrpn_unbuffer(buf, numPoints) == -1)) {
    return -1;
  }

  return 0;
}

vrpn_int32 nmm_Microscope_SEM::decode_AddPolygonData (
                                            const char **buf,
         vrpn_int32 numPoints,
         vrpn_float32 *x_nm, vrpn_float32 *y_nm)
{
  int i;
  for (i = 0; i < numPoints; i++) {
    if ((vrpn_unbuffer(buf, &(x_nm[i])) == -1) ||
        (vrpn_unbuffer(buf, &(y_nm[i])) == -1)) {
      return -1;
    }
  }

  return 0;
}

char * nmm_Microscope_SEM::encode_AddPolyline (vrpn_int32 *len,
         vrpn_float32 exposure_pCoul_per_cm,
         vrpn_float32 exposure_uCoul_per_cm2,
         vrpn_float32 lineWidth_nm, 
         vrpn_int32 numPoints,
         vrpn_float32 *x_nm, vrpn_float32 *y_nm)
{
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = (2*numPoints + 3)*sizeof(vrpn_float32)+sizeof(vrpn_int32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope_SEM::encode_AddPolyline:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, exposure_pCoul_per_cm);
    vrpn_buffer(&mptr, &mlen, exposure_uCoul_per_cm2);
    vrpn_buffer(&mptr, &mlen, lineWidth_nm);
    vrpn_buffer(&mptr, &mlen, numPoints);
    int i;
    for (i = 0; i < numPoints; i++) {
      vrpn_buffer(&mptr, &mlen, x_nm[i]);
      vrpn_buffer(&mptr, &mlen, y_nm[i]);
    }
  }

  return msgbuf;
}

vrpn_int32 nmm_Microscope_SEM::decode_AddPolylineHeader (
                                            const char **buf,
         vrpn_float32 *exposure_pCoul_per_cm,
         vrpn_float32 *exposure_uCoul_per_cm2,
         vrpn_float32 *lineWidth_nm,
         vrpn_int32 *numPoints)
{
  if ((vrpn_unbuffer(buf, exposure_pCoul_per_cm) == -1) ||
      (vrpn_unbuffer(buf, exposure_uCoul_per_cm2) == -1) ||
      (vrpn_unbuffer(buf, lineWidth_nm) == -1) ||
      (vrpn_unbuffer(buf, numPoints) == -1)) {
    return -1;
  }

  return 0;
}

vrpn_int32 nmm_Microscope_SEM::decode_AddPolylineData (
                                            const char **buf,
         vrpn_int32 numPoints,
         vrpn_float32 *x_nm, vrpn_float32 *y_nm)
{
  int i;
  for (i = 0; i < numPoints; i++) {
    if ((vrpn_unbuffer(buf, &(x_nm[i])) == -1) ||
        (vrpn_unbuffer(buf, &(y_nm[i])) == -1)) {
      return -1;
    }
  }

  return 0;
}

char * nmm_Microscope_SEM::encode_AddDumpPoint (vrpn_int32 *len,
                           vrpn_float32 x_nm, vrpn_float32 y_nm)
{
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 2 * sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope_SEM::encode_AddDumpPoint:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, x_nm);
    vrpn_buffer(&mptr, &mlen, y_nm);
  }

  return msgbuf;
}

vrpn_int32 nmm_Microscope_SEM::decode_AddDumpPoint (
                                            const char **buf,
        vrpn_float32 *x_nm, vrpn_float32 *y_nm)
{
  if ((vrpn_unbuffer(buf, x_nm) == -1) ||
      (vrpn_unbuffer(buf, y_nm) == -1)) {
    return -1;
  }

  return 0;
}

char * nmm_Microscope_SEM::encode_SetBeamCurrent (vrpn_int32 *len,
                                            vrpn_float32 current_picoAmps)
{
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope_SEM::encode_SetBeamCurrent:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, current_picoAmps);
  }

  return msgbuf;
}

vrpn_int32 nmm_Microscope_SEM::decode_SetBeamCurrent (const char **buf,
                                            vrpn_float32 *current_picoAmps)
{
  if ((vrpn_unbuffer(buf, current_picoAmps) == -1)) {
    return -1;
  }

  return 0;
}

char * nmm_Microscope_SEM::encode_SetBeamWidth (vrpn_int32 *len,
                                            vrpn_float32 beamWidth_nm)
{
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope_SEM::encode_SetBeamWidth:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, beamWidth_nm);
  }

  return msgbuf;
}

vrpn_int32 nmm_Microscope_SEM::decode_SetBeamWidth (const char **buf,
                                            vrpn_float32 *beamWidth_nm)
{
  if ((vrpn_unbuffer(buf, beamWidth_nm) == -1)) {
    return -1;
  }

  return 0;
}

char * nmm_Microscope_SEM::encode_SetPointReportEnable (vrpn_int32 *len,
                                                        vrpn_int32 enable)
{
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 1 * sizeof(vrpn_int32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope_SEM::encode_SetPointReportEnable:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, enable);
  }

  return msgbuf;
}

vrpn_int32 nmm_Microscope_SEM::decode_SetPointReportEnable (const char **buf,
                                        vrpn_int32 *enable)
{
  if ((vrpn_unbuffer(buf, enable)) == -1) {
    return -1;
  }

  return 0;
}

char * nmm_Microscope_SEM::encode_SetDotSpacing (vrpn_int32 *len,
                           vrpn_float32 dotSpacing_nm)
{
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope_SEM::encode_SetDotSpacing:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, dotSpacing_nm);
  }

  return msgbuf;
}

vrpn_int32 nmm_Microscope_SEM::decode_SetDotSpacing (const char **buf,
                           vrpn_float32 *dotSpacing_nm)
{
  return vrpn_unbuffer(buf, dotSpacing_nm);
}

char * nmm_Microscope_SEM::encode_SetLineSpacing (vrpn_int32 *len,
                           vrpn_float32 lineSpacing_nm)
{
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope_SEM::encode_SetLineSpacing:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, lineSpacing_nm);
  }

  return msgbuf;
}

vrpn_int32 nmm_Microscope_SEM::decode_SetLineSpacing (const char **buf,
                           vrpn_float32 *lineSpacing_nm)
{
  return vrpn_unbuffer(buf, lineSpacing_nm);
}

char * nmm_Microscope_SEM::encode_SetLinearExposure (vrpn_int32 *len,
                           vrpn_float32 exposure_pCoul_per_cm)
{
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope_SEM::encode_SetLinExposure:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, exposure_pCoul_per_cm);
  }

  return msgbuf;
}

vrpn_int32 nmm_Microscope_SEM::decode_SetLinearExposure (const char **buf,
                           vrpn_float32 *exposure_pCoul_per_cm)
{
  return vrpn_unbuffer(buf, exposure_pCoul_per_cm);
}

char * nmm_Microscope_SEM::encode_SetAreaExposure (vrpn_int32 *len,
                           vrpn_float32 exposure_uCoul_per_sq_cm)
{
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope_SEM::encode_SetAreaExposure:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, exposure_uCoul_per_sq_cm);
  }

  return msgbuf;

}

vrpn_int32 nmm_Microscope_SEM::decode_SetAreaExposure (const char **buf,
                           vrpn_float32 *exposure_uCoul_per_sq_cm)
{
  return vrpn_unbuffer(buf, exposure_uCoul_per_sq_cm);
}

char * nmm_Microscope_SEM::encode_SetMagnification (
                                            vrpn_int32 *len,
                                            vrpn_float32 mag)
{
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 1 * sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr,
              "nmm_Microscope_SEM::encode_SetMagnification:  "
              "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, mag);
  }

  return msgbuf;
}

vrpn_int32 nmm_Microscope_SEM::decode_SetMagnification (
                                            const char **buf,
                                            vrpn_float32 *mag)
{
  if ((vrpn_unbuffer(buf, mag)) == -1) {
    return -1;
  }

  return 0;
}


// ********************************************************************
// ********************************************************************
// ************************ (server-->client) *************************
// ********************************************************************
// ********************************************************************

char * nmm_Microscope_SEM::encode_ReportResolution (vrpn_int32 *len, 
					vrpn_int32 x, vrpn_int32 y)
{
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 2 * sizeof(vrpn_int32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope_SEM::encode_ReportResolution:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, x);
    vrpn_buffer(&mptr, &mlen, y);
  }

  return msgbuf;

}

vrpn_int32 nmm_Microscope_SEM::decode_ReportResolution(const char **buf, 
                                        vrpn_int32 *x, vrpn_int32 *y)
{
  if ((vrpn_unbuffer(buf, x)) == -1) {
    return -1;
  }
  if ((vrpn_unbuffer(buf, y)) == -1) {
    return -1;
  }

  return 0;

}

char * nmm_Microscope_SEM::encode_ReportPixelIntegrationTime (vrpn_int32 *len, 
                                        vrpn_int32 time_nsec)
{
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 1 * sizeof(vrpn_int32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope_SEM::encode_ReportPixelIntegrationTime:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, time_nsec);
  }

  return msgbuf;
}

vrpn_int32 nmm_Microscope_SEM::decode_ReportPixelIntegrationTime (
	const char **buf, vrpn_int32 *time_nsec)
{
  if ((vrpn_unbuffer(buf, time_nsec)) == -1) {
    return -1;
  }

  return 0;
}

char * nmm_Microscope_SEM::encode_ReportInterPixelDelayTime (vrpn_int32 *len,
                                        vrpn_int32 time_nsec)
{
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 1 * sizeof(vrpn_int32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope_SEM::encode_ReportInterPixelDelayTime:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, time_nsec);
  }

  return msgbuf;
}

vrpn_int32 nmm_Microscope_SEM::decode_ReportInterPixelDelayTime (
			const char **buf, vrpn_int32 *time_nsec)
{
  if ((vrpn_unbuffer(buf, time_nsec)) == -1) {
    return -1;
  }

  return 0;
}

// static
char *nmm_Microscope_SEM::encode_ScanlineData (vrpn_int32 * len,
        vrpn_int32 start_x, vrpn_int32 start_y, vrpn_int32 dx, vrpn_int32 dy,
        vrpn_int32 lineLength, vrpn_int32 numFields, vrpn_int32 numLines,
        vrpn_int32 sec, vrpn_int32 usec, vrpn_int32 pixelType, void **data)
{
  char *msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;
  int i;

  if (!len) return NULL;

  int pixelSize = 0;
  if (pixelType == NMB_UINT8) {
      pixelSize = sizeof(vrpn_uint8);
  } else if (pixelType == NMB_UINT16) {
      pixelSize = sizeof(vrpn_uint16);
  } else if (pixelType == NMB_FLOAT32) {
      pixelSize = sizeof(vrpn_float32);
  }
  *len = 10 * sizeof(vrpn_int32) + 
         lineLength * numFields * numLines * pixelSize;
  msgbuf = new char [*len];
  if (!(msgbuf)) {
    fprintf(stderr, "nmm_Microscope_SEM::encode_ScanlineData:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, start_x);
    vrpn_buffer(&mptr, &mlen, start_y);
    vrpn_buffer(&mptr, &mlen, dx);
    vrpn_buffer(&mptr, &mlen, dy);
    vrpn_buffer(&mptr, &mlen, lineLength);
    vrpn_buffer(&mptr, &mlen, numFields);
    vrpn_buffer(&mptr, &mlen, numLines);
    vrpn_buffer(&mptr, &mlen, sec);
    vrpn_buffer(&mptr, &mlen, usec);
    vrpn_buffer(&mptr, &mlen, (vrpn_int32)pixelType);
    switch (pixelType) {
      case NMB_UINT8:
        vrpn_buffer(&mptr, &mlen, (const char *)(*data), 
              lineLength*numFields*numLines);
        break;
      case NMB_UINT16:
        for (i = 0; i < lineLength*numFields*numLines; i++){
            vrpn_buffer(&mptr, &mlen, ((vrpn_uint16 *)(*data))[i]);
        }
        break;
      case NMB_FLOAT32:
        for (i = 0; i < lineLength*numFields*numLines; i++){
            vrpn_buffer(&mptr, &mlen, ((vrpn_float32 *)(*data))[i]);
        }
        break;
      default:
        fprintf(stderr, "encode_ScanlineData - unknown pixel type\n");
    }
  }

  return msgbuf;
}

vrpn_int32 nmm_Microscope_SEM::decode_ScanlineDataHeader (const char ** buf, 
	vrpn_int32 *start_x, vrpn_int32 *start_y, 
	vrpn_int32 *dx, vrpn_int32 *dy, vrpn_int32 *lineLength, 
	vrpn_int32 *numFields, vrpn_int32 *numLines,
        vrpn_int32 *sec, vrpn_int32 *usec, vrpn_int32 *pixelType)
{
  if (vrpn_unbuffer(buf, start_x) == -1) return -1;
  if (vrpn_unbuffer(buf, start_y) == -1) return -1;
  if (vrpn_unbuffer(buf, dx) == -1) return -1;
  if (vrpn_unbuffer(buf, dy) == -1) return -1;
  if (vrpn_unbuffer(buf, lineLength) == -1) return -1;
  if (vrpn_unbuffer(buf, numFields) == -1) return -1;
  if (vrpn_unbuffer(buf, numLines) == -1) return -1;
  if (vrpn_unbuffer(buf, sec) == -1) return -1;
  if (vrpn_unbuffer(buf, usec) == -1) return -1;
  if (vrpn_unbuffer(buf, pixelType) == -1) return -1;

  return 0;
}

vrpn_int32 nmm_Microscope_SEM::decode_ScanlineDataLine (const char ** buf,
        vrpn_int32 lineLength, vrpn_int32 numFields, vrpn_int32 numLines,
        vrpn_int32 pixelType, void * data)
{
    int i;
    switch (pixelType) {
      case NMB_UINT8:
        if (vrpn_unbuffer (buf, (char *)data, 
                          lineLength*numFields*numLines) == -1)
          return -1;
        break;
      case NMB_UINT16:
        for (i = 0; i < lineLength*numFields*numLines; i++){
            if (vrpn_unbuffer(buf, &(((vrpn_uint16 *)data)[i])) == -1)
                return -1;
        }
        break;
      case NMB_FLOAT32:
        for (i = 0; i < lineLength*numFields*numLines; i++){
            if (vrpn_unbuffer(buf, &(((vrpn_float32 *)data)[i])) == -1)
                return -1;
        }
        break;
    }

  return 0;
}

char * nmm_Microscope_SEM::encode_ReportPointDwellTime (vrpn_int32 *len,
                                            vrpn_int32 time_nsec)
{
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 1 * sizeof(vrpn_int32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope_SEM::encode_ReportPointDwellTime:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, time_nsec);
  }

  return msgbuf;
}

vrpn_int32 nmm_Microscope_SEM::decode_ReportPointDwellTime (const char **buf,
                                            vrpn_int32 *time_nsec)
{
  if ((vrpn_unbuffer(buf, time_nsec)) == -1) {
    return -1;
  }

  return 0;
}

char * nmm_Microscope_SEM::encode_ReportBeamBlankEnable (vrpn_int32 *len,
                                            vrpn_int32 enable)
{
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 1 * sizeof(vrpn_int32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope_SEM::encode_ReportBeamBlankEnable:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, enable);
  }

  return msgbuf;
}

vrpn_int32 nmm_Microscope_SEM::decode_ReportBeamBlankEnable (const char **buf,
                                            vrpn_int32 *enable)
{
  if ((vrpn_unbuffer(buf, enable)) == -1) {
    return -1;
  }

  return 0;
}

char * nmm_Microscope_SEM::encode_ReportMaxScanSpan (vrpn_int32 *len,
                              vrpn_int32 x, vrpn_int32 y)
{
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 2 * sizeof(vrpn_int32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope_SEM::encode_ReportMaxScanSpan:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, x);
    vrpn_buffer(&mptr, &mlen, y);
  }

  return msgbuf;

}

vrpn_int32 nmm_Microscope_SEM::decode_ReportMaxScanSpan (const char **buf,
                              vrpn_int32 *x, vrpn_int32 *y)
{
  if ((vrpn_unbuffer(buf, x)) == -1) {
    return -1;
  }
  if ((vrpn_unbuffer(buf, y)) == -1) {
    return -1;
  }

  return 0;
}

char * nmm_Microscope_SEM::encode_ReportBeamLocation (vrpn_int32 *len,
                              vrpn_int32 x, vrpn_int32 y)
{
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 2 * sizeof(vrpn_int32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope_SEM::encode_ReportBeamLocation:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, x);
    vrpn_buffer(&mptr, &mlen, y);
  }

  return msgbuf;

}

vrpn_int32 nmm_Microscope_SEM::decode_ReportBeamLocation (const char **buf,
                              vrpn_int32 *x, vrpn_int32 *y)
{
  if ((vrpn_unbuffer(buf, x)) == -1) {
    return -1;
  }
  if ((vrpn_unbuffer(buf, y)) == -1) {
    return -1;
  }

  return 0;
}

char * nmm_Microscope_SEM::encode_ReportRetraceDelays (vrpn_int32 *len,
                           vrpn_int32 h_time_nsec, vrpn_int32 v_time_nsec)
{
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 2 * sizeof(vrpn_int32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope_SEM::encode_ReportRetraceDelays:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, h_time_nsec);
    vrpn_buffer(&mptr, &mlen, v_time_nsec);
  }

  return msgbuf;

}

vrpn_int32 nmm_Microscope_SEM::decode_ReportRetraceDelays (const char **buf,
                           vrpn_int32 *h_time_nsec, vrpn_int32 *v_time_nsec)
{
  if ((vrpn_unbuffer(buf, h_time_nsec)) == -1) {
    return -1;
  }
  if ((vrpn_unbuffer(buf, v_time_nsec)) == -1) {
    return -1;
  }

  return 0;
}

char * nmm_Microscope_SEM::encode_ReportDACParams (vrpn_int32 *len,
                 vrpn_int32 x_gain, vrpn_int32 x_offset,
                 vrpn_int32 y_gain, vrpn_int32 y_offset,
                 vrpn_int32 z_gain, vrpn_int32 z_offset)
{
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 6 * sizeof(vrpn_int32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope_SEM::encode_ReportDACParams:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, x_gain);
    vrpn_buffer(&mptr, &mlen, x_offset);
    vrpn_buffer(&mptr, &mlen, y_gain);
    vrpn_buffer(&mptr, &mlen, y_offset);
    vrpn_buffer(&mptr, &mlen, z_gain);
    vrpn_buffer(&mptr, &mlen, z_offset);
  }

  return msgbuf;

}

vrpn_int32 nmm_Microscope_SEM::decode_ReportDACParams (const char **buf,
                 vrpn_int32 *x_gain, vrpn_int32 *x_offset,
                 vrpn_int32 *y_gain, vrpn_int32 *y_offset,
                 vrpn_int32 *z_gain, vrpn_int32 *z_offset)
{
  if ((vrpn_unbuffer(buf, x_gain)) == -1) {
    return -1;
  }
  if ((vrpn_unbuffer(buf, x_offset)) == -1) {
    return -1;
  }
  if ((vrpn_unbuffer(buf, y_gain)) == -1) {
    return -1;
  }
  if ((vrpn_unbuffer(buf, y_offset)) == -1) {
    return -1;
  }
  if ((vrpn_unbuffer(buf, z_gain)) == -1) {
    return -1;
  }
  if ((vrpn_unbuffer(buf, z_offset)) == -1) {
    return -1;
  }

  return 0;
}

char * nmm_Microscope_SEM::encode_ReportExternalScanControlEnable (
                                            vrpn_int32 *len,
                                            vrpn_int32 enable)
{
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 1 * sizeof(vrpn_int32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, 
              "nmm_Microscope_SEM::encode_ReportExternalScanControlEnable:  "
              "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, enable);
  }

  return msgbuf;
}

vrpn_int32 nmm_Microscope_SEM::decode_ReportExternalScanControlEnable (
                                            const char **buf,
                                            vrpn_int32 *enable)
{
  if ((vrpn_unbuffer(buf, enable)) == -1) {
    return -1;
  }

  return 0;
}

char * nmm_Microscope_SEM::encode_ReportMagnification (
                                            vrpn_int32 *len,
                                            vrpn_float32 mag)
{
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 1 * sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr,
              "nmm_Microscope_SEM::encode_ReportMagnification:  "
              "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, mag);
  }

  return msgbuf;
}

vrpn_int32 nmm_Microscope_SEM::decode_ReportMagnification (
                                            const char **buf,
                                            vrpn_float32 *mag)
{
  if ((vrpn_unbuffer(buf, mag)) == -1) {
    return -1;
  }

  return 0;
}

char * nmm_Microscope_SEM::encode_ReportExposureStatus (
                                            vrpn_int32 *len,
                                            vrpn_int32 numPointsTotal,
                                            vrpn_int32 numPointsDone,
                                            vrpn_float32 timeTotal_sec,
                                            vrpn_float32 timeDone_sec)
{
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 2*sizeof(vrpn_int32) + 2*sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr,
              "nmm_Microscope_SEM::encode_ReportExposureStatus:  "
              "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, numPointsTotal);
    vrpn_buffer(&mptr, &mlen, numPointsDone);
    vrpn_buffer(&mptr, &mlen, timeTotal_sec);
    vrpn_buffer(&mptr, &mlen, timeDone_sec);
  }

  return msgbuf;
}

vrpn_int32 nmm_Microscope_SEM::decode_ReportExposureStatus (
                                            const char **buf,
                                            vrpn_int32 *numPointsTotal,
                                            vrpn_int32 *numPointsDone,
                                            vrpn_float32 *timeTotal_sec,
                                            vrpn_float32 *timeDone_sec)
{
  if (((vrpn_unbuffer(buf, numPointsTotal)) == -1) ||
      ((vrpn_unbuffer(buf, numPointsDone)) == -1) ||
      ((vrpn_unbuffer(buf, timeTotal_sec)) == -1) || 
      ((vrpn_unbuffer(buf, timeDone_sec)) == -1)) {
    return -1;
  }

  return 0;
}

char * nmm_Microscope_SEM::encode_ReportTimingStatus (
                                            vrpn_int32 *len,
                                            vrpn_int32 numPointsTotal,
                                            vrpn_int32 numPointsDone,
                                            vrpn_float32 timeTotal_sec,
                                            vrpn_float32 timeDone_sec)
{
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 2*sizeof(vrpn_int32) + 2*sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr,
              "nmm_Microscope_SEM::encode_ReportTimingStatus:  "
              "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, numPointsTotal);
    vrpn_buffer(&mptr, &mlen, numPointsDone);
    vrpn_buffer(&mptr, &mlen, timeTotal_sec);
    vrpn_buffer(&mptr, &mlen, timeDone_sec);
  }

  return msgbuf;
}

vrpn_int32 nmm_Microscope_SEM::decode_ReportTimingStatus (
                                            const char **buf,
                                            vrpn_int32 *numPointsTotal,
                                            vrpn_int32 *numPointsDone,
                                            vrpn_float32 *timeTotal_sec,
                                            vrpn_float32 *timeDone_sec)
{
  if (((vrpn_unbuffer(buf, numPointsTotal)) == -1) ||
      ((vrpn_unbuffer(buf, numPointsDone)) == -1) ||
      ((vrpn_unbuffer(buf, timeTotal_sec)) == -1) || 
      ((vrpn_unbuffer(buf, timeDone_sec)) == -1)) {
    return -1;
  }

  return 0;
}

char * nmm_Microscope_SEM::encode_ReportPointReportEnable (vrpn_int32 *len,
                                                        vrpn_int32 enable)
{
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 1 * sizeof(vrpn_int32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope_SEM::encode_ReportPointReportEnable:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, enable);
  }

  return msgbuf;
}

vrpn_int32 nmm_Microscope_SEM::decode_ReportPointReportEnable (const char **buf,
                                        vrpn_int32 *enable)
{
  if ((vrpn_unbuffer(buf, enable)) == -1) {
    return -1;
  }

  return 0;
}

char * nmm_Microscope_SEM::encode_ReportDotSpacing (vrpn_int32 *len,
                           vrpn_float32 dotSpacing_nm)
{
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope_SEM::encode_ReportDotSpacing:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, dotSpacing_nm);
  }

  return msgbuf;
}

vrpn_int32 nmm_Microscope_SEM::decode_ReportDotSpacing (const char **buf,
                           vrpn_float32 *dotSpacing_nm)
{
  return vrpn_unbuffer(buf, dotSpacing_nm);
}

char * nmm_Microscope_SEM::encode_ReportLineSpacing (vrpn_int32 *len,
                           vrpn_float32 lineSpacing_nm)
{
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope_SEM::encode_ReportLineSpacing:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, lineSpacing_nm);
  }

  return msgbuf;
}

vrpn_int32 nmm_Microscope_SEM::decode_ReportLineSpacing (const char **buf,
                           vrpn_float32 *lineSpacing_nm)
{
  return vrpn_unbuffer(buf, lineSpacing_nm);
}


/*
int nmm_Microscope_SEM::dispatchMessage (vrpn_int32 len, const char * buf, 
                                            vrpn_int32 type) {
  struct timeval now;
  int retval;

  gettimeofday(&now, NULL);
  // If we aren't connected to anything, just pretend we sent the message
  if(d_connection) {
      retval = d_connection->pack_message(len, now, type, d_myId, (char *) buf,
                                      vrpn_CONNECTION_RELIABLE);
  } else {
      retval = 0;
  }
  delete [] (char *) buf;

  return retval;
}
*/

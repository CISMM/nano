//Warning: this file automatically generated using the command line
// ../../../../../../vrpn/util/gen_rpc/gen_vrpn_rpc.pl -c .\nmm_AFMSIMSERVER_Report.vrpndef
//DO NOT EDIT! Edit the source file instead.

/*===3rdtech===
  Copyright (c) 2001 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/

#include <vrpn_Connection.h>
#include "nmm_AFMSIMSERVER_Report.h"

nmm_AFMSIMSERVER_Report::nmm_AFMSIMSERVER_Report (
   vrpn_Connection * connection)
{
  if (connection) {
    d_ReportGridSize_type = connection->register_message_type
       ("nmm SimulatedMicroscope ReportGridSize");
    d_ScanRange_type = connection->register_message_type
       ("nmm SimulatedMicroscope ScanRange");

  }
}

nmm_AFMSIMSERVER_Report::~nmm_AFMSIMSERVER_Report (void) { }

char * nmm_AFMSIMSERVER_Report::encode_ReportGridSize (
      int * len,
      vrpn_int32 x,
      vrpn_int32 y
) {
  char * msgbuf = NULL;
  char *mptr[1];
  int temp; int* mlen = &temp;
  if (!len) return NULL;
  *len =
     (sizeof(vrpn_int32) + sizeof(vrpn_int32));
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "encode_ReportGridSize: Out of memory.\n");
    *len = 0;
  } else {
    *mptr = msgbuf;
    *mlen = *len;
    vrpn_buffer(mptr, mlen, x);
    vrpn_buffer(mptr, mlen, y);
  }
  return msgbuf;
}

int nmm_AFMSIMSERVER_Report::decode_ReportGridSize (
      const char ** buffer,
      vrpn_int32 (*x),
      vrpn_int32 (*y)
) {
  if (!buffer
   || !x
   || !y) return -1;
  if (vrpn_unbuffer(buffer, x)) return -1;
  if (vrpn_unbuffer(buffer, y)) return -1;
  return 0;
}

char * nmm_AFMSIMSERVER_Report::encode_ScanRange (
      int * len,
      vrpn_float32 xmin,
      vrpn_float32 ymin,
      vrpn_float32 zmin,
      vrpn_float32 xmax,
      vrpn_float32 ymax,
      vrpn_float32 zmax
) {
  char * msgbuf = NULL;
  char *mptr[1];
  int temp; int* mlen = &temp;
  if (!len) return NULL;
  *len =
     (sizeof(vrpn_float32) + sizeof(vrpn_float32) + sizeof(vrpn_float32) + sizeof(vrpn_float32) + sizeof(vrpn_float32) + sizeof(vrpn_float32));
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "encode_ScanRange: Out of memory.\n");
    *len = 0;
  } else {
    *mptr = msgbuf;
    *mlen = *len;
    vrpn_buffer(mptr, mlen, xmin);
    vrpn_buffer(mptr, mlen, ymin);
    vrpn_buffer(mptr, mlen, zmin);
    vrpn_buffer(mptr, mlen, xmax);
    vrpn_buffer(mptr, mlen, ymax);
    vrpn_buffer(mptr, mlen, zmax);
  }
  return msgbuf;
}

int nmm_AFMSIMSERVER_Report::decode_ScanRange (
      const char ** buffer,
      vrpn_float32 (*xmin),
      vrpn_float32 (*ymin),
      vrpn_float32 (*zmin),
      vrpn_float32 (*xmax),
      vrpn_float32 (*ymax),
      vrpn_float32 (*zmax)
) {
  if (!buffer
   || !xmin
   || !ymin
   || !zmin
   || !xmax
   || !ymax
   || !zmax) return -1;
  if (vrpn_unbuffer(buffer, xmin)) return -1;
  if (vrpn_unbuffer(buffer, ymin)) return -1;
  if (vrpn_unbuffer(buffer, zmin)) return -1;
  if (vrpn_unbuffer(buffer, xmax)) return -1;
  if (vrpn_unbuffer(buffer, ymax)) return -1;
  if (vrpn_unbuffer(buffer, zmax)) return -1;
  return 0;
}


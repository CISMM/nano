#ifndef NMM_AFMSIMSERVER_REPORT_H
#define NMM_AFMSIMSERVER_REPORT_H
//Warning: this file automatically generated using the command line
// ../../../../../../vrpn/util/gen_rpc/gen_vrpn_rpc.pl -h .\nmm_AFMSIMSERVER_Report.vrpndef
//DO NOT EDIT! Edit the source file instead.

/*===3rdtech===
  Copyright (c) 2001 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/

#include <vrpn_Connection.h>



class nmm_AFMSIMSERVER_Report 
{
  public:

    nmm_AFMSIMSERVER_Report (vrpn_Connection * = NULL);
      // Constructor.
    virtual ~nmm_AFMSIMSERVER_Report (void);
      // Destructor. HP compiler doesn't like it with '= 0;'

    vrpn_int32 d_ReportGridSize_type;
    vrpn_int32 d_ScanRange_type;


  char * encode_ReportGridSize (
      int * len,
      vrpn_int32 x,
      vrpn_int32 y
  );
  int decode_ReportGridSize (
      const char ** buffer,
      vrpn_int32 (*x),
      vrpn_int32 (*y)
  );
  char * encode_ScanRange (
      int * len,
      vrpn_float32 xmin,
      vrpn_float32 ymin,
      vrpn_float32 zmin,
      vrpn_float32 xmax,
      vrpn_float32 ymax,
      vrpn_float32 zmax
  );
  int decode_ScanRange (
      const char ** buffer,
      vrpn_float32 (*xmin),
      vrpn_float32 (*ymin),
      vrpn_float32 (*zmin),
      vrpn_float32 (*xmax),
      vrpn_float32 (*ymax),
      vrpn_float32 (*zmax)
  );
};
#endif // NMM_AFMSIMSERVER_REPORT_H

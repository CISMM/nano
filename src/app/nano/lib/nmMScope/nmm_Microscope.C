	// If you wanna see what changes made by Tiger, just search for 'Tiger'
#include "nmm_Microscope.h"

#include <vrpn_FileController.h>
#include <vrpn_Forwarder.h>
#include <vrpn_Shared.h>

#include <stm_cmd.h>	// Added by Tiger	for STM_NAME_LENGTH

/* Tiger	don't need them be in nmm_Microscope base class, move to remote
#include <Point.h>
#include <BCPlane.h>
#include <nmb_Dataset.h>
#include <nmb_Decoration.h>
#include <Tcl_Linkvar.h>
#include <stm_file.h>
*/

#include <nmb_Util.h>

/* Tiger	seems I don't need these two header files either
#include "MicroscopeIO.h"
#include "ModFile.h"
*/

#define CHECK(a) if (a == -1) return -1

nmm_Microscope::nmm_Microscope
    (const char * name,
     vrpn_Connection * connection) :

  d_connection (connection),
  d_fileController (new vrpn_File_Controller (connection)),
  d_tcl_script_dir (NULL) {

  char * servicename;           // Tiger HACK probably need to do that
  servicename = vrpn_copy_service_name(name);   // to get the right name.

  if (connection) {
    d_myId = connection->register_sender(servicename);	// Tiger changed name
							// to servicename

    d_SetRegionNM_type = connection->register_message_type
         ("nmm Microscope SetRegionNM");
    d_ScanTo_type = connection->register_message_type
         ("nmm Microscope ScanTo");
    d_ScanToZ_type = connection->register_message_type
         ("nmm Microscope ScanToZ");
    d_ZagTo_type = connection->register_message_type
         ("nmm Microscope ZagTo");
    d_SetScanStyle_type = connection->register_message_type
         ("nmm Microscope SetScanStyle");
    d_SetSlowScan_type = connection->register_message_type
         ("nmm Microscope SetSlowScan");
    d_SetStdDelay_type = connection->register_message_type
         ("nmm Microscope SetStdDelay");
    d_SetStPtDelay_type = connection->register_message_type
         ("nmm Microscope SetStPtDelay");
    d_SetRelax_type = connection->register_message_type
         ("nmm Microscope SetRelax");
    d_RecordResistance_type = connection->register_message_type
         ("nmm Microscope RecordResistance");
    d_SetStdDevParams_type = connection->register_message_type
         ("nmm Microscope SetStdDevParams");
    d_SetScanWindow_type = connection->register_message_type
         ("nmm Microscope SetScanWindow");
    d_ResumeWindowScan_type = connection->register_message_type
         ("nmm Microscope ResumeWindowScan");
    d_SetGridSize_type = connection->register_message_type
         ("nmm Microscope SetGridSize");
    d_SetOhmmeterSampleRate_type = connection->register_message_type
         ("nmm Microscope SetOhmmeterSampleRate");
    d_EnableAmp_type = connection->register_message_type
         ("nmm Microscope EnableAmp");
    d_DisableAmp_type = connection->register_message_type
         ("nmm Microscope DisableAmp");
    d_EnableVoltsource_type = connection->register_message_type
         ("nmm Microscope EnableVoltsource");
    d_DisableVoltsource_type = connection->register_message_type
         ("nmm Microscope DisableVoltsource");
    d_SetRateNM_type = connection->register_message_type
         ("nmm Microscope SetRateNM");
    d_SetMaxMove_type = connection->register_message_type
         ("nmm Microscope SetMaxMove");
    d_SetModForce_type = connection->register_message_type
         ("nmm Microscope SetModForce");
    d_DrawSharpLine_type = connection->register_message_type
         ("nmm Microscope DrawSharpLine");
    d_DrawSweepLine_type = connection->register_message_type
         ("nmm Microscope DrawSweepLine");
    d_DrawSweepArc_type = connection->register_message_type
         ("nmm Microscope DrawSweepArc");
    d_GetNewPointDatasets_type = connection->register_message_type
         ("nmm Microscope GetNewPointDatasets");
    d_GetNewScanDatasets_type = connection->register_message_type
         ("nmm Microscope GetNewScanDatasets");
    d_Echo_type = connection->register_message_type
         ("nmm Microscope Echo");
    d_MarkModify_type = connection->register_message_type
	("nmm Microscope MarkModify");
    d_MarkImage_type = connection->register_message_type
        ("nmm Microscope MarkImage");
    d_Shutdown_type = connection->register_message_type
         ("nmm Microscope Shutdown");
    d_QueryScanRange_type = connection->register_message_type
         ("nmm Microscope QueryScanRange");
    d_QueryStdDevParams_type = connection->register_message_type
         ("nmm Microscope QueryStdDevParams");
    d_QueryPulseParams_type = connection->register_message_type
         ("nmm Microscope QueryPulseParams");
    d_VoltsourceEnabled_type = connection->register_message_type
         ("nmm Microscope VoltsourceEnabled");
    d_VoltsourceDisabled_type = connection->register_message_type
         ("nmm Microscope VoltsourceDisabled");
    d_AmpEnabled_type = connection->register_message_type
         ("nmm Microscope AmpEnabled");
    d_AmpDisabled_type = connection->register_message_type
         ("nmm Microscope AmpDisabled");
    d_StartingToRelax_type = connection->register_message_type
         ("nmm Microscope StartingToRelax");
    d_RelaxSet_type = connection->register_message_type
         ("nmm Microscope RelaxSet");
    d_StdDevParameters_type = connection->register_message_type
         ("nmm Microscope StdDevParameters");
    d_WindowLineData_type = connection->register_message_type
         ("nmm Microscope WindowLineData");
    d_WindowScanNM_type = connection->register_message_type
         ("nmm Microscope WindowScanNM");
    d_WindowBackscanNM_type = connection->register_message_type
         ("nmm Microscope WindowBackscanNM");
    d_PointResultNM_type = connection->register_message_type
         ("nmm Microscope PointResultNM");
    d_PointResultData_type = connection->register_message_type
         ("nmm Microscope PointResultData");
    d_BottomPunchResultData_type = connection->register_message_type
         ("nmm Microscope BottomPunchResultData");
    d_TopPunchResultData_type = connection->register_message_type
         ("nmm Microscope TopPunchResultData");
    d_ZigResultNM_type = connection->register_message_type
         ("nmm Microscope ZigResultNM");
    d_BluntResultNM_type = connection->register_message_type
         ("nmm Microscope BluntResultNM");
    d_ScanRange_type = connection->register_message_type
         ("nmm Microscope ScanRange");
    d_SetRegionCompleted_type = connection->register_message_type
         ("nmm Microscope SetRegionCompleted");
    d_SetRegionClipped_type = connection->register_message_type
         ("nmm Microscope SetRegionClipped");
    d_ResistanceFailure_type = connection->register_message_type
         ("nmm Microscope ResistanceFailure");
    d_Resistance_type = connection->register_message_type
         ("nmm Microscope Resistance");
    d_Resistance2_type = connection->register_message_type
         ("nmm Microscope Resistance2");
    d_ReportSlowScan_type = connection->register_message_type
         ("nmm Microscope ReportSlowScan");
    d_ScanParameters_type = connection->register_message_type
         ("nmm Microscope ScanParameters");
    d_HelloMessage_type = connection->register_message_type
         ("nmm Microscope HelloMessage");
    d_ClientHello_type = connection->register_message_type
         ("nmm Microscope ClientHello");
    d_ScanDataset_type = connection->register_message_type
         ("nmm Microscope ScanDataset");
    d_PointDataset_type = connection->register_message_type
         ("nmm Microscope PointDataset");
    d_PidParameters_type = connection->register_message_type
         ("nmm Microscope PidParameters");
    d_ScanrateParameter_type = connection->register_message_type
         ("nmm Microscope ScanrateParameter");
    d_ReportGridSize_type = connection->register_message_type
         ("nmm Microscope ReportGridSize");
    d_ServerPacketTimestamp_type = connection->register_message_type
         ("nmm Microscope ServerPacketTimestamp");
    d_TopoFileHeader_type = connection->register_message_type
         ("nmm Microscope TopoFileHeader");
    d_ForceCurveData_type = connection->register_message_type
	 ("nmm Microscope ForceCurveData");

    d_MaxSetpointExceeded_type = connection->register_message_type
	 ("nmm Microscope MaxSetpointExceeded");

    d_RecvTimestamp_type = connection->register_message_type
         ("nmm Microscope Clark RecvTimestamp");
    d_FakeSendTimestamp_type = connection->register_message_type
         ("nmm Microscope Clark FakeSendTimestamp");
    d_UdpSeqNum_type = connection->register_message_type
         ("nmm Microscope Clark UdpSeqNum");

    d_EnterTappingMode_type = connection->register_message_type
        ("nmm Microscope AFM EnterTappingMode");
    d_EnterContactMode_type = connection->register_message_type
        ("nmm Microscope AFM EnterContactMode");
    d_EnterDirectZControl_type = connection->register_message_type
        ("nmm Microscope AFM EnterDirectZControl");
    d_EnterSewingStyle_type = connection->register_message_type
        ("nmm Microscope AFM EnterSewingStyle");
    d_EnterSpectroscopyMode_type = connection->register_message_type
	("nmm Microscope AFM EnterSpectroscopyMode");
    d_SetContactForce_type = connection->register_message_type
        ("nmm Microscope AFM SetContactForce");
    d_QueryContactForce_type = connection->register_message_type
        ("nmm Microscope AFM QueryContactForce");

    d_InTappingMode_type = connection->register_message_type
        ("nmm Microscope AFM InTappingMode");
    d_InContactMode_type = connection->register_message_type
        ("nmm Microscope AFM InContactMode");
    d_InDirectZControl_type = connection->register_message_type
        ("nmm Microscope AFM InDirectZControl");
    d_InSewingStyle_type = connection->register_message_type
        ("nmm Microscope AFM InSewingStyle");
    d_InSpectroscopyMode_type = connection->register_message_type
	("nmm Microscope AFM InSpectroscopyMode");
    d_ForceParameters_type = connection->register_message_type
        ("nmm Microscope AFM ForceParameters");
    d_BaseModParameters_type = connection->register_message_type
        ("nmm Microscope AFM BaseModParameters");
    d_ForceSettings_type = connection->register_message_type
        ("nmm Microscope AFM ForceSettings");
    d_InModModeT_type = connection->register_message_type
        ("nmm Microscope AFM InModModeT");
    d_InImgModeT_type = connection->register_message_type
        ("nmm Microscope AFM InImgModeT");
    d_InModMode_type = connection->register_message_type
        ("nmm Microscope AFM InModMode");
    d_InImgMode_type = connection->register_message_type
        ("nmm Microscope AFM InImgMode");
    d_ModForceSet_type = connection->register_message_type
        ("nmm Microscope AFM ModForceSet");
    d_ImgForceSet_type = connection->register_message_type
        ("nmm Microscope AFM ImgForceSet");
    d_ModForceSetFailure_type = connection->register_message_type
        ("nmm Microscope AFM ModForceSetFailure");
    d_ImgForceSetFailure_type = connection->register_message_type
        ("nmm Microscope AFM ImgForceSetFailure");
    d_ModSet_type = connection->register_message_type
        ("nmm Microscope AFM ModSet");
    d_ImgSet_type = connection->register_message_type
        ("nmm Microscope AFM ImgSet");
    d_ForceSet_type = connection->register_message_type
        ("nmm Microscope AFM ForceSet");
    d_ForceSetFailure_type = connection->register_message_type
        ("nmm Microscope AFM ForceSetFailure");

    d_SampleApproach_type = connection->register_message_type
        ("nmm Microscope STM SampleApproach");
    d_SetBias_type = connection->register_message_type
        ("nmm Microscope STM SetBias");
    d_SampleApproachNM_type = connection->register_message_type
        ("nmm Microscope STM SampleApproachNM");
    d_SetPulsePeak_type = connection->register_message_type
        ("nmm Microscope STM SetPulsePeak");
    d_SetPulseDuration_type = connection->register_message_type
        ("nmm Microscope STM SetPulseDuration");
    d_PulsePoint_type = connection->register_message_type
        ("nmm Microscope STM PulsePoint");
    d_PulsePointNM_type = connection->register_message_type
        ("nmm Microscope STM PulsePointNM");

    d_PulseParameters_type = connection->register_message_type
        ("nmm Microscope STM PulseParameters");
    d_PulseCompletedNM_type = connection->register_message_type
        ("nmm Microscope STM PulseCompletedNM");
    d_PulseFailureNM_type = connection->register_message_type
        ("nmm Microscope STM PulseFailureNM");
    d_PulseCompleted_type = connection->register_message_type
        ("nmm Microscope STM PulseCompleted");
    d_PulseFailure_type = connection->register_message_type
        ("nmm Microscope STM PulseFailure");
    d_TunnellingAttained_type = connection->register_message_type
        ("nmm Microscope STM TunnellingAttained");
    d_TunnellingAttainedNM_type = connection->register_message_type
        ("nmm Microscope STM TunnellingAttainedNM");
    d_TunnellingFailure_type = connection->register_message_type
        ("nmm Microscope STM TunnellingFailure");
    d_ApproachComplete_type = connection->register_message_type
        ("nmm Microscope STM ApproachComplete");

    // Scanline mode (client-->server)
    d_EnterScanlineMode_type = connection->register_message_type
	("nmm Microscope EnterScanlineMode");
    d_RequestScanLine_type = connection->register_message_type
        ("nmm Microscope RequestScanLine");

    // Scanline mode (server-->client)
    d_InScanlineMode_type = connection->register_message_type
        ("nmm Microscope InScanlineMode");
    d_ScanlineData_type = connection->register_message_type
        ("nmm Microscope ScanlineData");
  }

  if (servicename)
    delete [] servicename;
}


nmm_Microscope::~nmm_Microscope (void) {

  if (d_tcl_script_dir)
    delete [] d_tcl_script_dir;

}

long nmm_Microscope::mainloop (void) {
  if (d_connection)
    CHECK(d_connection->mainloop());

  return 0;
}

// static
void nmm_Microscope::registerMicroscopeMessagesForForwarding
                (vrpn_StreamForwarder * forwarder) {
  registerMicroscopeOutputMessagesForForwarding(forwarder);
  registerMicroscopeInputMessagesForForwarding(forwarder);

}

// static
void nmm_Microscope::registerMicroscopeMessagesForForwarding
                (vrpn_ConnectionForwarder * forwarder,
                 const char * sourceServiceName,
                 const char * destinationServiceName) {
  registerMicroscopeOutputMessagesForForwarding
     (forwarder, sourceServiceName, destinationServiceName);
  registerMicroscopeInputMessagesForForwarding
     (forwarder, sourceServiceName, destinationServiceName);
}

// static
void nmm_Microscope::registerMicroscopeOutputMessagesForForwarding
                (vrpn_StreamForwarder * forwarder) {
  int i;

  for (i = 0; i < d_numOutputMessageNames; i++)
    forwarder->forward(d_outputMessageName[i], d_outputMessageName[i]);

}

// static
void nmm_Microscope::registerMicroscopeOutputMessagesForForwarding
                (vrpn_ConnectionForwarder * forwarder,
                 const char * sourceServiceName,
                 const char * destinationServiceName) {
  int i;

  for (i = 0; i < d_numOutputMessageNames; i++)
    forwarder->forward(d_outputMessageName[i], sourceServiceName,
                       d_outputMessageName[i], destinationServiceName);

}

// static
void nmm_Microscope::registerMicroscopeInputMessagesForForwarding
                (vrpn_StreamForwarder * forwarder) {
  int i;

  for (i = 0; i < d_numInputMessageNames; i++)
    forwarder->forward(d_inputMessageName[i], d_inputMessageName[i]);

}

// static
void nmm_Microscope::registerMicroscopeInputMessagesForForwarding
                (vrpn_ConnectionForwarder * forwarder,
                 const char * sourceServiceName,
                 const char * destinationServiceName) {
  int i;

  for (i = 0; i < d_numInputMessageNames; i++)
    forwarder->forward(d_inputMessageName[i], sourceServiceName,
                       d_inputMessageName[i], destinationServiceName);

}


// static
long nmm_Microscope::d_numOutputMessageNames = 33;
// static
char * nmm_Microscope::d_outputMessageName [] = {
         "nmm Microscope SetRegionNM",
         "nmm Microscope ScanTo",
         "nmm Microscope ZagTo",
         "nmm Microscope SetScanStyle",
         "nmm Microscope SetSlowScan",
         "nmm Microscope SetStdDelay",
         "nmm Microscope SetStPtDelay",
         "nmm Microscope SetRelax",
         "nmm Microscope RecordResistance",
         "nmm Microscope SetStdDevParams",
         "nmm Microscope SetScanWindow",
         "nmm Microscope ResumeWindowScan",
         "nmm Microscope SetGridSize",
         "nmm Microscope SetOhmmeterSampleRate",
         "nmm Microscope EnableAmp",
         "nmm Microscope DisableAmp",
         "nmm Microscope EnableVoltsource",
         "nmm Microscope DisableVoltsource",
         "nmm Microscope SetRateNM",
         "nmm Microscope SetMaxMove",
         "nmm Microscope SetModForce",
         "nmm Microscope DrawSharpLine",
         "nmm Microscope DrawSweepLine",
         "nmm Microscope DrawSweepArc",
         "nmm Microscope GetNewPointDatasets",
         "nmm Microscope GetNewScanDatasets",
         "nmm Microscope Echo",
         "nmm Microscope MarkModify",
         "nmm Microscope MarkImage",
         "nmm Microscope Shutdown",
         "nmm Microscope QueryScanRange",
         "nmm Microscope QueryStdDevParams",
         "nmm Microscope QueryPulseParams",
};
// static
long nmm_Microscope::d_numInputMessageNames = 78;
// static
char * nmm_Microscope::d_inputMessageName [] = {
         "nmm Microscope VoltsourceEnabled",
         "nmm Microscope VoltsourceDisabled",
         "nmm Microscope AmpEnabled",
         "nmm Microscope AmpDisabled",
         "nmm Microscope StartingToRelax",
         "nmm Microscope RelaxSet",
         "nmm Microscope StdDevParameters",
         "nmm Microscope WindowLineData",
         "nmm Microscope WindowScanNM",
         "nmm Microscope WindowBackscanNM",
         "nmm Microscope PointResultNM",
         "nmm Microscope PointResultData",
         "nmm Microscope BottomPunchResultData",
         "nmm Microscope TopPunchResultData",
         "nmm Microscope ZigResultNM",
         "nmm Microscope BluntResultNM",
         "nmm Microscope ScanRange",
         "nmm Microscope SetRegionCompleted",
         "nmm Microscope SetRegionClipped",
         "nmm Microscope ResistanceFailure",
         "nmm Microscope Resistance",
         "nmm Microscope Resistance2",
         "nmm Microscope ReportSlowScan",
         "nmm Microscope ScanParameters",
         "nmm Microscope HelloMessage",
         "nmm Microscope ClientHello",
         "nmm Microscope ScanDataset",
         "nmm Microscope PointDataset",
         "nmm Microscope PidParameters",
         "nmm Microscope ScanrateParameter",
         "nmm Microscope ReportGridSize",
         "nmm Microscope ServerPacketTimestamp",
         "nmm Microscope TopoFileHeader",
	 "nmm Microscope ForceCurveData",
         "nmm Microscope Clark RecvTimestamp",
         "nmm Microscope Clark FakeSendTimestamp",
         "nmm Microscope Clark UdpSeqNum",
         "nmm Microscope AFM EnterTappingMode",
         "nmm Microscope AFM EnterContactMode",
         "nmm Microscope AFM EnterSewingStyle",
         "nmm Microscope AFM EnterSpectroscopyMode",
         "nmm Microscope AFM SetContactForce",
         "nmm Microscope AFM QueryContactForce",
         "nmm Microscope AFM InTappingMode",
         "nmm Microscope AFM InContactMode",
         "nmm Microscope AFM InSewingStyle",
         "nmm Microscope AFM InSpectroscopyMode",
         "nmm Microscope AFM ForceParameters",
         "nmm Microscope AFM BaseModParameters",
         "nmm Microscope AFM ForceSettings",
         "nmm Microscope AFM InModModeT",
         "nmm Microscope AFM InImgModeT",
         "nmm Microscope AFM InModMode",
         "nmm Microscope AFM InImgMode",
         "nmm Microscope AFM ModForceSet",
         "nmm Microscope AFM ImgForceSet",
         "nmm Microscope AFM ModForceSetFailure",
         "nmm Microscope AFM ImgForceSetFailure",
         "nmm Microscope AFM ModSet",
         "nmm Microscope AFM ImgSet",
         "nmm Microscope AFM ForceSet",
         "nmm Microscope AFM ForceSetFailure",
         "nmm Microscope STM SampleApproach",
         "nmm Microscope STM SetBias",
         "nmm Microscope STM SampleApproachNM",
         "nmm Microscope STM SetPulsePeak",
         "nmm Microscope STM SetPulseDuration",
         "nmm Microscope STM PulsePoint",
         "nmm Microscope STM PulsePointNM",
         "nmm Microscope STM PulseParameters",
         "nmm Microscope STM PulseCompletedNM",
         "nmm Microscope STM PulseFailureNM",
         "nmm Microscope STM PulseCompleted",
         "nmm Microscope STM PulseFailure",
         "nmm Microscope STM TunnellingAttained",
         "nmm Microscope STM TunnellingAttainedNM",
         "nmm Microscope STM TunnellingFailure",
         "nmm Microscope STM ApproachComplete",
};

/* Tiger	move it to nmm_MicroscopeRemote.C
long nmm_Microscope::InitializeDataset (nmb_Dataset * ds) {
  BCPlane * plane;

  d_dataset = ds;

  state.data.Initialize(ds);
  plane = ds->ensureHeightPlane();
  plane->setScale(state.stm_z_scale);

  return 0;
}
*/

/* Tiger	move it to nmm_MicroscopeRemote.C
long nmm_Microscope::tInitializeDecoration (nmb_Decoration * dec) {
  d_decoration = dec;

  return 0;
}
*/

/* Tiger	move it to nmm_MicroscopeRemote.C
long nmm_Microscope::InitializeTcl (const char * dir) {
  if (!dir)
    return -1;

  d_tcl_script_dir = new char [1 + strlen(dir)];
  if (!d_tcl_script_dir)
    return -1;

  strcpy(d_tcl_script_dir, dir);
  return 0;
}
*/






char * nmm_Microscope::encode_SetRegionNM (long * len,
                 float minx, float miny, float maxx, float maxy) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = 4 * sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_SetRegionNM:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, minx);
    nmb_Util::Buffer(&mptr, &mlen, miny);
    nmb_Util::Buffer(&mptr, &mlen, maxx);
    nmb_Util::Buffer(&mptr, &mlen, maxy);
  }

  return msgbuf;
}

long nmm_Microscope::decode_SetRegionNM (const char ** buf,
         float * minx, float * miny, float * maxx, float * maxy) {
  CHECK(nmb_Util::Unbuffer(buf, minx));
  CHECK(nmb_Util::Unbuffer(buf, miny));
  CHECK(nmb_Util::Unbuffer(buf, maxx));
  CHECK(nmb_Util::Unbuffer(buf, maxy));

  return 0;
}



char * nmm_Microscope::encode_ScanTo (long * len,
                 float x, float y) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = 2 * sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_ScanTo:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, x);
    nmb_Util::Buffer(&mptr, &mlen, y);
  }

  return msgbuf;
}
long nmm_Microscope::decode_ScanTo (const char ** buf,
         float * x, float * y) {
  CHECK(nmb_Util::Unbuffer(buf, x));
  CHECK(nmb_Util::Unbuffer(buf, y));

  return 0;
}

char * nmm_Microscope::encode_ScanTo (long * len,
                 float x, float y, float z) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = 3 * sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_ScanTo:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, x);
    nmb_Util::Buffer(&mptr, &mlen, y);
    nmb_Util::Buffer(&mptr, &mlen, z);
  }

  return msgbuf;
}
long nmm_Microscope::decode_ScanTo (const char ** buf,
         float * x, float * y, float * z) {
  CHECK(nmb_Util::Unbuffer(buf, x));
  CHECK(nmb_Util::Unbuffer(buf, y));
  CHECK(nmb_Util::Unbuffer(buf, z));

  return 0;
}



char * nmm_Microscope::encode_ZagTo (long * len,
                 float x, float y, float yaw, float sweepWidth,
                 float regionDiag) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = 5 * sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_ZagTo:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, x);
    nmb_Util::Buffer(&mptr, &mlen, y);
    nmb_Util::Buffer(&mptr, &mlen, yaw);
    nmb_Util::Buffer(&mptr, &mlen, sweepWidth);
    nmb_Util::Buffer(&mptr, &mlen, regionDiag);
  }

  return msgbuf;
}
long nmm_Microscope::decode_ZagTo (const char ** buf,
         float * x, float * y, float * yaw, float * sweepWidth,
         float * regionDiag) {
  CHECK(nmb_Util::Unbuffer(buf, x));
  CHECK(nmb_Util::Unbuffer(buf, y));
  CHECK(nmb_Util::Unbuffer(buf, yaw));
  CHECK(nmb_Util::Unbuffer(buf, sweepWidth));
  CHECK(nmb_Util::Unbuffer(buf, regionDiag));

  return 0;
}



char * nmm_Microscope::encode_SetScanStyle (long * len,
                  long style) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = sizeof(long);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_SetScanStyle:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen,  style);
  }

  return msgbuf;
}
long nmm_Microscope::decode_SetScanStyle (const char ** buf,
         long *  style) {
  CHECK(nmb_Util::Unbuffer(buf,  style));

  return 0;
}



char * nmm_Microscope::encode_SetSlowScan (long * len,
                 long value) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = sizeof(long);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_SetSlowScan:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen,  value);
  }

  return msgbuf;
}
long nmm_Microscope::decode_SetSlowScan (const char ** buf,
         long *  value) {
  CHECK(nmb_Util::Unbuffer(buf,  value));

  return 0;
}



char * nmm_Microscope::encode_SetStdDelay (long * len,
                 long delay) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  // NANO BEGIN
  //fprintf(stderr, "nmm_Microscope::encode_SetStdDelay(): Entering...\n");
  // NANO END

  if (!len) return NULL;

  *len = sizeof(long);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_SetStdDelay:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, delay);
  }

  // NANO BEGIN
  //fprintf(stderr, "nmm_Microscope::encode_SetStdDelay(): msg type = %d\n", d_SetStdDelay_type);
  //fprintf(stderr, "nmm_Microscope::encode_SetStdDelay(): delay = %d\n", delay);
  //fprintf(stderr, "nmm_Microscope::encode_SetStdDelay(): Leaving\n");
  // NANO END

  return msgbuf;
}
long nmm_Microscope::decode_SetStdDelay (const char ** buf,
         long * delay) {
  CHECK(nmb_Util::Unbuffer(buf, delay));

  return 0;
}



char * nmm_Microscope::encode_SetStPtDelay (long * len,
                 long delay) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  // NANO BEGIN
  //fprintf(stderr, "nmm_Microscope::encode_SetStPtDelay(): Entering...\n");
  // NANO END

  if (!len) return NULL;

  *len = sizeof(long);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_SetStPtDelay:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, delay);
  }

  // NANO BEGIN
  //fprintf(stderr, "nmm_Microscope::encode_SetStPtDelay(): msg type = %d\n", d_SetStPtDelay_type);
  //fprintf(stderr, "nmm_Microscope::encode_SetStPtDelay(): delay = %d\n", delay);
  //fprintf(stderr, "nmm_Microscope::encode_SetStPtDelay(): Leaving\n");
  // NANO END

  return msgbuf;
}

long nmm_Microscope::decode_SetStPtDelay (const char ** buf,
         long * delay) {
  CHECK(nmb_Util::Unbuffer(buf, delay));

  return 0;
}



char * nmm_Microscope::encode_SetRelax (long * len,
                 long min, long sep) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  // NANO BEGIN
  //fprintf(stderr, "nmm_Microscope::encode_SetRelax(): Entering...\n");
  // NANO END
  if (!len) return NULL;

  *len = 2 * sizeof(long);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_SetRelax:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, min);
    nmb_Util::Buffer(&mptr, &mlen, sep);
  }

  // NANO BEGIN
  //fprintf(stderr, "nmm_Microscope::encode_SetRelax(): msg type = %d\n", d_SetRelax_type);
  fprintf(stderr, "nmm_Microscope::encode_SetRelax(): min = %ld\t sep = %ld\n", (long)min, (long)sep);
  //fprintf(stderr, "nmm_Microscope::encode_SetRelax(): Leaving\n");
  // NANO END

  return msgbuf;
}
long nmm_Microscope::decode_SetRelax (const char ** buf,
         long * min, long * sep) {
  CHECK(nmb_Util::Unbuffer(buf, min));
  CHECK(nmb_Util::Unbuffer(buf, sep));

  return 0;
}




char * nmm_Microscope::encode_RecordResistance (long * len,
                 long meter, struct timeval time, float resistance,
                 float v, float r, float f) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = sizeof(long) + sizeof (struct timeval) + 4 * sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_RecordResistance:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, meter);
    nmb_Util::Buffer(&mptr, &mlen, time);
    nmb_Util::Buffer(&mptr, &mlen, resistance);
    nmb_Util::Buffer(&mptr, &mlen, v);
    nmb_Util::Buffer(&mptr, &mlen, r);
    nmb_Util::Buffer(&mptr, &mlen, f);
  }

  return msgbuf;
}
long nmm_Microscope::decode_RecordResistance (const char ** buf,
         long * meter, struct timeval * time, float * resistance,
         float * v, float * r, float * f) {
  CHECK(nmb_Util::Unbuffer(buf, meter));
  CHECK(nmb_Util::Unbuffer(buf, time));
  CHECK(nmb_Util::Unbuffer(buf, resistance));
  CHECK(nmb_Util::Unbuffer(buf, v));
  CHECK(nmb_Util::Unbuffer(buf, r));
  CHECK(nmb_Util::Unbuffer(buf, f));

  return 0;
}



char * nmm_Microscope::encode_SetStdDevParams (long * len,
                 long samples, float freq) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = sizeof(long) + sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_SetStdDevParams:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, samples);
    nmb_Util::Buffer(&mptr, &mlen, freq);
  }

  return msgbuf;
}
long nmm_Microscope::decode_SetStdDevParams (const char ** buf,
         long * samples, float * freq) {
  CHECK(nmb_Util::Unbuffer(buf, samples));
  CHECK(nmb_Util::Unbuffer(buf, freq));

  return 0;
}



char * nmm_Microscope::encode_SetScanWindow (long * len,
                 long minx, long miny, long maxx, long maxy) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = 4 * sizeof(long);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_SetScanWindow:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, minx);
    nmb_Util::Buffer(&mptr, &mlen, miny);
    nmb_Util::Buffer(&mptr, &mlen, maxx);
    nmb_Util::Buffer(&mptr, &mlen, maxy);
  }

  return msgbuf;
}
long nmm_Microscope::decode_SetScanWindow (const char ** buf,
         long * minx, long * miny, long * maxx, long * maxy) {
  CHECK(nmb_Util::Unbuffer(buf, minx));
  CHECK(nmb_Util::Unbuffer(buf, miny));
  CHECK(nmb_Util::Unbuffer(buf, maxx));
  CHECK(nmb_Util::Unbuffer(buf, maxy));

  return 0;
}



char * nmm_Microscope::encode_SetGridSize (long * len,
                 long x, long y) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = 2 * sizeof(long);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_SetGridSize:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, x);
    nmb_Util::Buffer(&mptr, &mlen, y);
  }

  return msgbuf;
}
long nmm_Microscope::decode_SetGridSize (const char ** buf,
         long * x, long * y) {
  CHECK(nmb_Util::Unbuffer(buf, x));
  CHECK(nmb_Util::Unbuffer(buf, y));

  return 0;
}



char * nmm_Microscope::encode_SetOhmmeterSampleRate (long * len,
                 long which, long rate) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = 2 * sizeof(long);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_SetOhmmeterSampleRate:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, which);
    nmb_Util::Buffer(&mptr, &mlen, rate);
  }

  return msgbuf;
}
long nmm_Microscope::decode_SetOhmmeterSampleRate (const char ** buf,
         long * which, long * rate) {
  CHECK(nmb_Util::Unbuffer(buf, which));
  CHECK(nmb_Util::Unbuffer(buf, rate));

  return 0;
}



char * nmm_Microscope::encode_EnableAmp (long * len,
                 long which, float offset, float percentOffset,
		 long gain) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = 2 * sizeof(long) + 2 * sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_EnableAmp:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, which);
    nmb_Util::Buffer(&mptr, &mlen, offset);
    nmb_Util::Buffer(&mptr, &mlen, percentOffset);
    nmb_Util::Buffer(&mptr, &mlen, gain);
  }

  return msgbuf;
}
long nmm_Microscope::decode_EnableAmp (const char ** buf,
         long * which, float * offset, float * percentOffset,
	 long * gain) {
  CHECK(nmb_Util::Unbuffer(buf, which));
  CHECK(nmb_Util::Unbuffer(buf, offset));
  CHECK(nmb_Util::Unbuffer(buf, percentOffset));
  CHECK(nmb_Util::Unbuffer(buf, gain));

  return 0;
}



char * nmm_Microscope::encode_DisableAmp (long * len,
                 long which) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = sizeof(long);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_DisableAmp:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, which);
  }

  return msgbuf;
}
long nmm_Microscope::decode_DisableAmp (const char ** buf,
         long * which) {
  CHECK(nmb_Util::Unbuffer(buf, which));

  return 0;
}



char * nmm_Microscope::encode_EnableVoltsource (long * len,
                 long which, float voltage) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = sizeof(long) + sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_EnableVoltsource:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, which);
    nmb_Util::Buffer(&mptr, &mlen, voltage);
  }

  return msgbuf;
}
long nmm_Microscope::decode_EnableVoltsource (const char ** buf,
         long * which, float * voltage) {
  CHECK(nmb_Util::Unbuffer(buf, which));
  CHECK(nmb_Util::Unbuffer(buf, voltage));

  return 0;
}



char * nmm_Microscope::encode_DisableVoltsource (long * len,
                 long which) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = sizeof(long);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_DisableVoltsource:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, which);
  }

  return msgbuf;
}
long nmm_Microscope::decode_DisableVoltsource (const char ** buf,
         long * which) {
  CHECK(nmb_Util::Unbuffer(buf, which));

  return 0;
}



char * nmm_Microscope::encode_SetRateNM (long * len,
                 float rate) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_SetRateNM:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, rate);
  }

  return msgbuf;
}
long nmm_Microscope::decode_SetRateNM (const char ** buf,
         float * rate) {
  CHECK(nmb_Util::Unbuffer(buf, rate));

  return 0;
}



char * nmm_Microscope::encode_SetMaxMove (long * len,
                 float distance) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_SetMaxMove:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, distance);
  }

  return msgbuf;
}
long nmm_Microscope::decode_SetMaxMove (const char ** buf,
         float * distance) {
  CHECK(nmb_Util::Unbuffer(buf, distance));

  return 0;
}

char * nmm_Microscope::encode_SetModForce (int * len,
                 float newforce, float min, float max) {
  char * msgbuf = NULL;
  char * mptr;
  int mlen;

  if (!len) return NULL;

  *len = sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_SetModForce:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    // There is an "enable" flag we put in automatically.
    nmb_Util::Buffer(&mptr, &mlen, 1); 
    nmb_Util::Buffer(&mptr, &mlen,min);
    nmb_Util::Buffer(&mptr, &mlen,max);
    nmb_Util::Buffer(&mptr, &mlen,newforce);
  }

  return msgbuf;
}
int nmm_Microscope::decode_SetModForce (const char ** buf,
         float * newforce, float * min, float * max) {
/*	Tiger changed it
   float * enable;
  CHECK(nmb_Util::Unbuffer(buf, enable));
*/
  float enable;
  CHECK(nmb_Util::Unbuffer(buf, &enable));
  CHECK(nmb_Util::Unbuffer(buf,min));
  CHECK(nmb_Util::Unbuffer(buf,max));
  CHECK(nmb_Util::Unbuffer(buf,newforce));
  return 0;
}



char * nmm_Microscope::encode_DrawSharpLine (long * len,
                 float startx, float starty, float endx, float endy,
                 float stepSize) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = 5 * sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_DrawSharpLine:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, startx);
    nmb_Util::Buffer(&mptr, &mlen, starty);
    nmb_Util::Buffer(&mptr, &mlen, endx);
    nmb_Util::Buffer(&mptr, &mlen, endy);
    nmb_Util::Buffer(&mptr, &mlen, stepSize);
  }

  return msgbuf;
}
long nmm_Microscope::decode_DrawSharpLine (const char ** buf,
         float * startx, float * starty, float * endx, float * endy,
         float * stepSize) {
  CHECK(nmb_Util::Unbuffer(buf, startx));
  CHECK(nmb_Util::Unbuffer(buf, starty));
  CHECK(nmb_Util::Unbuffer(buf, endx));
  CHECK(nmb_Util::Unbuffer(buf, endy));
  CHECK(nmb_Util::Unbuffer(buf, stepSize));

  return 0;
}



char * nmm_Microscope::encode_DrawSweepLine (long * len,
                 float startx, float starty, float startYaw,
                 float startSweepWidth, float endx, float endy,
                 float endYaw, float endSweepWidth, float stepSize) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = 9 * sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_DrawSweepLine:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, startx);
    nmb_Util::Buffer(&mptr, &mlen, starty);
    nmb_Util::Buffer(&mptr, &mlen, startYaw);
    nmb_Util::Buffer(&mptr, &mlen, startSweepWidth);
    nmb_Util::Buffer(&mptr, &mlen, endx);
    nmb_Util::Buffer(&mptr, &mlen, endy);
    nmb_Util::Buffer(&mptr, &mlen, endYaw);
    nmb_Util::Buffer(&mptr, &mlen, endSweepWidth);
    nmb_Util::Buffer(&mptr, &mlen, stepSize);
  }

  return msgbuf;
}
long nmm_Microscope::decode_DrawSweepLine (const char ** buf,
         float * startx, float * starty, float * startYaw,
         float * startSweepWidth, float * endx, float * endy,
         float * endYaw, float * endSweepWidth, float * stepSize) {
  CHECK(nmb_Util::Unbuffer(buf, startx));
  CHECK(nmb_Util::Unbuffer(buf, starty));
  CHECK(nmb_Util::Unbuffer(buf, startYaw));
  CHECK(nmb_Util::Unbuffer(buf, startSweepWidth));
  CHECK(nmb_Util::Unbuffer(buf, endx));
  CHECK(nmb_Util::Unbuffer(buf, endy));
  CHECK(nmb_Util::Unbuffer(buf, endYaw));
  CHECK(nmb_Util::Unbuffer(buf, endSweepWidth));
  CHECK(nmb_Util::Unbuffer(buf, stepSize));

  return 0;
}



char * nmm_Microscope::encode_DrawSweepArc (long * len,
                 float x, float y, float startAngle, float startSweepWidth,
                 float endAngle, float endSweepWidth, float stepSize) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = 7 * sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_DrawSweepArc:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, x);
    nmb_Util::Buffer(&mptr, &mlen, y);
    nmb_Util::Buffer(&mptr, &mlen, startAngle);
    nmb_Util::Buffer(&mptr, &mlen, startSweepWidth);
    nmb_Util::Buffer(&mptr, &mlen, endAngle);
    nmb_Util::Buffer(&mptr, &mlen, endSweepWidth);
    nmb_Util::Buffer(&mptr, &mlen, stepSize);
  }

  return msgbuf;
}
long nmm_Microscope::decode_DrawSweepArc (const char ** buf,
         float * x, float * y, float * startAngle, float * startSweepWidth,
         float * endAngle, float * endSweepWidth,
	 float * stepSize) {
  CHECK(nmb_Util::Unbuffer(buf, x));
  CHECK(nmb_Util::Unbuffer(buf, y));
  CHECK(nmb_Util::Unbuffer(buf, startAngle));
  CHECK(nmb_Util::Unbuffer(buf, startSweepWidth));
  CHECK(nmb_Util::Unbuffer(buf, endAngle));
  CHECK(nmb_Util::Unbuffer(buf, endSweepWidth));
  CHECK(nmb_Util::Unbuffer(buf, stepSize));

  return 0;
}



/* Tiger	moved it to nmm_MicroscopeRemote
char * nmm_Microscope::encode_GetNewPointDatasets
                            (long * len,
                             const Tclvar_checklist * list) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;
  long numSets = 0;
  long i;

  if (!len) return NULL;

  for (i = 0; i < list->Num_checkboxes(); i++)
    if (list->Is_set(i)) numSets++;

  *len = sizeof(long) + (STM_NAME_LENGTH + sizeof(long)) * numSets;
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_GetNewPointDatasets:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, numSets);
    for (i = 0; i < list->Num_checkboxes(); i++)
      if (list->Is_set(i)) {
        nmb_Util::Buffer(&mptr, &mlen, list->Checkbox_name(i), STM_NAME_LENGTH);

        // Ask for 10 samples of each except Topography;
        // of that we want 90
        if (!strcmp("Topography", list->Checkbox_name(i)))
          nmb_Util::Buffer(&mptr, &mlen, (long)90);
        else
          nmb_Util::Buffer(&mptr, &mlen, (long)10);
      }
  }

  return msgbuf;
}
*/

long nmm_Microscope::decode_GetNewPointDatasetHeader (const char ** buf,
         long * numSets) {
  CHECK(nmb_Util::Unbuffer(buf, numSets));

  return 0;
}

long nmm_Microscope::decode_GetNewPointDataset (const char ** buf,
         char * name, long * numSamples) {
  CHECK(nmb_Util::Unbuffer(buf, name, STM_NAME_LENGTH));
  CHECK(nmb_Util::Unbuffer(buf, numSamples));

  return 0;
}



/* Tiger	moved it to nmm_MicroscopeRemote
char * nmm_Microscope::encode_GetNewScanDatasets
                            (long * len,
                             const Tclvar_checklist * list) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;
  long numSets = 0;
  long i;

  if (!len) return NULL;

  for (i = 0; i < list->Num_checkboxes(); i++)
    if (list->Is_set(i)) numSets++;

  *len = sizeof(long) + STM_NAME_LENGTH * numSets;
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_GetNewScanDatasets:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, numSets);
    for (i = 0; i < list->Num_checkboxes(); i++)
      if (list->Is_set(i))
        nmb_Util::Buffer(&mptr, &mlen, list->Checkbox_name(i), STM_NAME_LENGTH);
  }

  return msgbuf;
}
*/

long nmm_Microscope::decode_GetNewScanDatasetHeader (const char ** buf,
         long * numSets) {
  CHECK(nmb_Util::Unbuffer(buf, numSets));

  return 0;
}

long nmm_Microscope::decode_GetNewScanDataset (const char ** buf,
         char * name) {
  CHECK(nmb_Util::Unbuffer(buf, name, STM_NAME_LENGTH));

  return 0;
}

char * nmm_Microscope::encode_MarkModify (long * len) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = 2 * sizeof(long);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_MarkModify:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, (long)1);
    nmb_Util::Buffer(&mptr, &mlen, (long)0);
  }

  // NO!  NO!  HACK HACK HACK
  // Can't just use echo, because it needs to be xlated into
  // the server's token and back...

  return msgbuf;
}

// Tiger	HACK HACK HACK
long nmm_Microscope::decode_MarkModify ( const char ** buf )
{
  long i, j;	// dumb variables

  CHECK(nmb_Util::Unbuffer(buf, &i));
  CHECK(nmb_Util::Unbuffer(buf, &j));

  return 0;
}

char * nmm_Microscope::encode_MarkImage (long * len) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = 2 * sizeof(long);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_MarkImage:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, (long)1);
    nmb_Util::Buffer(&mptr, &mlen, (long)0);
  }

  // NO!  NO!  HACK HACK HACK
  // Can't just use echo, because it needs to be xlated longo
  // the server's token and back...

  return msgbuf;
}

// Tiger        HACK HACK HACK
long nmm_Microscope::decode_MarkImage ( const char ** buf ) 
{
  long i, j;    // dumb variables

  CHECK(nmb_Util::Unbuffer(buf, &i));
  CHECK(nmb_Util::Unbuffer(buf, &j));

  return 0;
}


char * nmm_Microscope::encode_VoltsourceEnabled (long * len,
           long which, float voltage) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = sizeof(long) + sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_VoltsourceEnabled:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, which);
    nmb_Util::Buffer(&mptr, &mlen, voltage);
  }

  return msgbuf;
}

long nmm_Microscope::decode_VoltsourceEnabled (const char ** buf,
         long * which, float * voltage) {
  CHECK(nmb_Util::Unbuffer(buf, which));
  CHECK(nmb_Util::Unbuffer(buf, voltage));

  return 0;
}

char * nmm_Microscope::encode_VoltsourceDisabled (long * len,
           long which) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = sizeof(long);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_VoltsourceDisabled:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, which);
  }

  return msgbuf;
}

long nmm_Microscope::decode_VoltsourceDisabled (const char ** buf,
         long * which) {
  CHECK(nmb_Util::Unbuffer(buf, which));

  return 0;
}

char * nmm_Microscope::encode_AmpEnabled (long * len,
           long which, float offset, float percentOffset, long gain) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = 2 * sizeof(long) + 2 * sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_AmpEnabled:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, which);
    nmb_Util::Buffer(&mptr, &mlen, offset);
    nmb_Util::Buffer(&mptr, &mlen, percentOffset);
    nmb_Util::Buffer(&mptr, &mlen, gain);
  }

  return msgbuf;
}

long nmm_Microscope::decode_AmpEnabled (const char ** buf,
         long * which, float * offset, float * percentOffset, long * gain) {
  CHECK(nmb_Util::Unbuffer(buf, which));
  CHECK(nmb_Util::Unbuffer(buf, offset));
  CHECK(nmb_Util::Unbuffer(buf, percentOffset));
  CHECK(nmb_Util::Unbuffer(buf, gain));

  return 0;
}

char * nmm_Microscope::encode_AmpDisabled (long * len,
           long which) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = sizeof(long);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_AmpDisabled:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, which);
  }

  return msgbuf;
}

long nmm_Microscope::decode_AmpDisabled (const char ** buf,
         long * which) {
  CHECK(nmb_Util::Unbuffer(buf, which));

  return 0;
}

char * nmm_Microscope::encode_StartingToRelax (long * len,
           long sec, long usec) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = 2 * sizeof(long);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_StartingToRelax:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, sec);
    nmb_Util::Buffer(&mptr, &mlen, usec);
  }

  return msgbuf;
}

long nmm_Microscope::decode_StartingToRelax (const char ** buf,
         long * sec, long * usec) {
  CHECK(nmb_Util::Unbuffer(buf, sec));
  CHECK(nmb_Util::Unbuffer(buf, usec));

  return 0;
}


char * nmm_Microscope::encode_RelaxSet (long * len,
            long min, long sep) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = 2 * sizeof(long);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_RelaxSet:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, min);
    nmb_Util::Buffer(&mptr, &mlen, sep);
  }

  return msgbuf;
}

long nmm_Microscope::decode_RelaxSet (const char ** buf,
         long * min, long * sep) {
  // NANO BEGIN
   //fprintf(stderr, "nmm_Microscope::decode_RelaxSet(): Entering...\n");
  // NANO END
  CHECK(nmb_Util::Unbuffer(buf, min));
  CHECK(nmb_Util::Unbuffer(buf, sep));

  // NANO BEGIN
  //fprintf(stderr, "nmm_Microscope::decode_RelaxSet(): msg type = %d\n", d_RelaxSet_type);
  //fprintf(stderr, "nmm_Microscope::decode_RelaxSet(): min = %lX\t sep = %lX\n", *min, *sep);
  //fprintf(stderr, "nmm_Microscope::decode_RelaxSet(): Leaving!");
  // NANO END
  return 0;
}


char * nmm_Microscope::encode_StdDevParameters (long * len,
           long samples, float frequency) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = sizeof(long) + sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_StdDevParameters:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, samples);
    nmb_Util::Buffer(&mptr, &mlen, frequency);


  }

  return msgbuf;


}

long nmm_Microscope::decode_StdDevParameters (const char ** buf,
         long * samples, float * frequency) {
  CHECK(nmb_Util::Unbuffer(buf, samples));
  CHECK(nmb_Util::Unbuffer(buf, frequency));

  return 0;
}

char * nmm_Microscope::encode_WindowLineData (long * len,
           long x, long y, long dx, long dy, long lineCount,
           long fieldCount, long * offset, long sec, long usec, 
	   unsigned short ** data) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;
  long i, j;
  float val_hgt;

  if (!len) return NULL;

  *len = 8 * sizeof(long) + lineCount * fieldCount * sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_WindowLineData:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, x);
    nmb_Util::Buffer(&mptr, &mlen, y);
    nmb_Util::Buffer(&mptr, &mlen, dx);
    nmb_Util::Buffer(&mptr, &mlen, dy);
    nmb_Util::Buffer(&mptr, &mlen, lineCount);
    nmb_Util::Buffer(&mptr, &mlen, fieldCount);
    nmb_Util::Buffer(&mptr, &mlen, sec);
    nmb_Util::Buffer(&mptr, &mlen, usec);
    for (i = 0; i < lineCount; i++)
      for (j = 0; j < fieldCount; j++) {
 	val_hgt = (float)(data[j][offset[j]+i]);
        nmb_Util::Buffer(&mptr, &mlen, val_hgt);
      }
  }

  return msgbuf;
}

char * nmm_Microscope::encode_WindowLineData (long * len,
           long x, long y, long dx, long dy, long lineCount,
           long fieldCount, long sec, long usec, 
	   float ** data) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;
  long i, j;

  if (!len) return NULL;

  *len = 8 * sizeof(long) + lineCount * fieldCount * sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_WindowLineData:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, x);
    nmb_Util::Buffer(&mptr, &mlen, y);
    nmb_Util::Buffer(&mptr, &mlen, dx);
    nmb_Util::Buffer(&mptr, &mlen, dy);
    nmb_Util::Buffer(&mptr, &mlen, lineCount);
    nmb_Util::Buffer(&mptr, &mlen, fieldCount);
    nmb_Util::Buffer(&mptr, &mlen, sec);
    nmb_Util::Buffer(&mptr, &mlen, usec);
    for (i = 0; i < lineCount; i++)
      for (j = 0; j < fieldCount; j++) {
        nmb_Util::Buffer(&mptr, &mlen, data[j][i]);
      }
  }

  return msgbuf;
}

long nmm_Microscope::decode_WindowLineDataHeader (const char ** buf,
         long * x, long * y, long * dx, long * dy,
         long * lineCount, long * fieldCount, long * sec, long * usec) {
  CHECK(nmb_Util::Unbuffer(buf, x));
  CHECK(nmb_Util::Unbuffer(buf, y));
  CHECK(nmb_Util::Unbuffer(buf, dx));
  CHECK(nmb_Util::Unbuffer(buf, dy));
  CHECK(nmb_Util::Unbuffer(buf, lineCount));
  CHECK(nmb_Util::Unbuffer(buf, fieldCount));
  CHECK(nmb_Util::Unbuffer(buf, sec));
  CHECK(nmb_Util::Unbuffer(buf, usec));

  return 0;
}

long nmm_Microscope::decode_WindowLineDataField (const char ** buf,
         long fieldCount, float * data) {
  long i;

  for (i = 0; i < fieldCount; i++) {
      CHECK(nmb_Util::Unbuffer(buf, &data[i]));
  }

  return 0;
}

char * nmm_Microscope::encode_WindowScanNM (long * len,
           long x, long y, long sec, long usec, float height,
	   float deviation) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = 4 * sizeof(long) + 2 * sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_WindowScanNM:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, x);
    nmb_Util::Buffer(&mptr, &mlen, y);
    nmb_Util::Buffer(&mptr, &mlen, sec);
    nmb_Util::Buffer(&mptr, &mlen, usec);
    nmb_Util::Buffer(&mptr, &mlen, height);
    nmb_Util::Buffer(&mptr, &mlen, deviation);
  }

  return msgbuf;
}

long nmm_Microscope::decode_WindowScanNM (const char ** buf,
         long * x, long * y, long * sec, long * usec,
         float * value, float * deviation) {
  CHECK(nmb_Util::Unbuffer(buf, x));
  CHECK(nmb_Util::Unbuffer(buf, y));
  CHECK(nmb_Util::Unbuffer(buf, sec));
  CHECK(nmb_Util::Unbuffer(buf, usec));
  CHECK(nmb_Util::Unbuffer(buf, value));
  CHECK(nmb_Util::Unbuffer(buf, deviation));

  return 0;
}

char * nmm_Microscope::encode_WindowBackscanNM (long * len,
           long x, long y, long sec, long usec, float value, float deviation) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = 4 * sizeof(long) + 2 * sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_WindowBackscanNM:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, x);
    nmb_Util::Buffer(&mptr, &mlen, y);
    nmb_Util::Buffer(&mptr, &mlen, sec);
    nmb_Util::Buffer(&mptr, &mlen, usec);
    nmb_Util::Buffer(&mptr, &mlen, value);
    nmb_Util::Buffer(&mptr, &mlen, deviation);
  }

  return msgbuf;
}

long nmm_Microscope::decode_WindowBackscanNM (const char ** buf,
         long * x, long * y, long * sec, long * usec,
         float * value, float * deviation) {
  CHECK(nmb_Util::Unbuffer(buf, x));
  CHECK(nmb_Util::Unbuffer(buf, y));
  CHECK(nmb_Util::Unbuffer(buf, sec));
  CHECK(nmb_Util::Unbuffer(buf, usec));
  CHECK(nmb_Util::Unbuffer(buf, value));
  CHECK(nmb_Util::Unbuffer(buf, deviation));

  return 0;
}

char * nmm_Microscope::encode_PointResultNM (long * len,
           float x, float y, long sec, long usec, float height,
           float deviation) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = 2 * sizeof(long) + 4 * sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_PointResultNM:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, x);
    nmb_Util::Buffer(&mptr, &mlen, y);
    nmb_Util::Buffer(&mptr, &mlen, sec);
    nmb_Util::Buffer(&mptr, &mlen, usec);
    nmb_Util::Buffer(&mptr, &mlen, height);
    nmb_Util::Buffer(&mptr, &mlen, deviation);
  }

  return msgbuf;
}

long nmm_Microscope::decode_PointResultNM (const char ** buf,
         float * x, float * y, long * sec, long * usec,
         float * height, float * deviation) {
  CHECK(nmb_Util::Unbuffer(buf, x));
  CHECK(nmb_Util::Unbuffer(buf, y));
  CHECK(nmb_Util::Unbuffer(buf, sec));
  CHECK(nmb_Util::Unbuffer(buf, usec));
  CHECK(nmb_Util::Unbuffer(buf, height));
  CHECK(nmb_Util::Unbuffer(buf, deviation));

  return 0;
}

char * nmm_Microscope::encode_ResultData (long * len,
           float x, float y, long sec, long usec, long fieldCount,
	   float * data) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;
  long i;

  if (!len) return NULL;

/* Changed by Tiger	x and y are floats
  *len = 5 * sizeof(long) + fieldCount * sizeof(float);
*/
  *len = 3*sizeof(long) + 2*sizeof(float) + fieldCount * sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_ResultData:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, x);
    nmb_Util::Buffer(&mptr, &mlen, y);
    nmb_Util::Buffer(&mptr, &mlen, sec);
    nmb_Util::Buffer(&mptr, &mlen, usec);
    nmb_Util::Buffer(&mptr, &mlen, fieldCount);
    for (i = 0; i < fieldCount; i++)
      nmb_Util::Buffer(&mptr, &mlen, data[i]);
  }

  return msgbuf;
}

long nmm_Microscope::decode_ResultData (const char ** buf,
         float * x, float * y, long * sec, long * usec, long * fieldCount,
         float * data) {
  long i;

  CHECK(nmb_Util::Unbuffer(buf, x));
  CHECK(nmb_Util::Unbuffer(buf, y));
  CHECK(nmb_Util::Unbuffer(buf, sec));
  CHECK(nmb_Util::Unbuffer(buf, usec));
  CHECK(nmb_Util::Unbuffer(buf, fieldCount));

  for (i = 0; i < *fieldCount; i++)
    CHECK(nmb_Util::Unbuffer(buf, &data[i]));

  return 0;
}

char * nmm_Microscope::encode_ResultNM (long * len,
           float x, float y, long sec, long usec, float height,
           float normX, float normY, float normZ) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = 2 * sizeof(long) + 6 * sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_ResultNM:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, x);
    nmb_Util::Buffer(&mptr, &mlen, y);
    nmb_Util::Buffer(&mptr, &mlen, sec);
    nmb_Util::Buffer(&mptr, &mlen, usec);
    nmb_Util::Buffer(&mptr, &mlen, height);
    nmb_Util::Buffer(&mptr, &mlen, normX);
    nmb_Util::Buffer(&mptr, &mlen, normY);
    nmb_Util::Buffer(&mptr, &mlen, normZ);
  }

  return msgbuf;
}

long nmm_Microscope::decode_ResultNM (const char ** buf,
         float * x, float * y, long * sec, long * usec, float * height,
         float * normX, float * normY, float * normZ) {
  CHECK(nmb_Util::Unbuffer(buf, x));
  CHECK(nmb_Util::Unbuffer(buf, y));
  CHECK(nmb_Util::Unbuffer(buf, sec));
  CHECK(nmb_Util::Unbuffer(buf, usec));
  CHECK(nmb_Util::Unbuffer(buf, height));
  CHECK(nmb_Util::Unbuffer(buf, normX));
  CHECK(nmb_Util::Unbuffer(buf, normY));
  CHECK(nmb_Util::Unbuffer(buf, normZ));

  return 0;
}

char * nmm_Microscope::encode_ScanRange (long * len,
           float minX, float maxX, float minY, float maxY,
           float minZ, float maxZ) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = 6 * sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_ScanRange:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, minX);
    nmb_Util::Buffer(&mptr, &mlen, maxX);
    nmb_Util::Buffer(&mptr, &mlen, minY);
    nmb_Util::Buffer(&mptr, &mlen, maxY);
    nmb_Util::Buffer(&mptr, &mlen, minZ);
    nmb_Util::Buffer(&mptr, &mlen, maxZ);


  }

  return msgbuf;


}

long nmm_Microscope::decode_ScanRange (const char ** buf,
         float * minX, float * maxX, float * minY,
         float * maxY, float * minZ, float * maxZ) {
  CHECK(nmb_Util::Unbuffer(buf, minX));
  CHECK(nmb_Util::Unbuffer(buf, maxX));
  CHECK(nmb_Util::Unbuffer(buf, minY));
  CHECK(nmb_Util::Unbuffer(buf, maxY));
  CHECK(nmb_Util::Unbuffer(buf, minZ));
  CHECK(nmb_Util::Unbuffer(buf, maxZ));

  return 0;
}

char * nmm_Microscope::encode_SetRegionC (long * len,
           float minX, float minY, float maxX, float maxY) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

/* Changed by Tiger	there is not long buffered
  *len = sizeof(long) + 4 * sizeof(float);
*/
  *len = 4 * sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_SetRegionC:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, minX);
    nmb_Util::Buffer(&mptr, &mlen, minY);
    nmb_Util::Buffer(&mptr, &mlen, maxX);
    nmb_Util::Buffer(&mptr, &mlen, maxY);
  }

  return msgbuf;
}

long nmm_Microscope::decode_SetRegionC (const char ** buf,
         float * minX, float * minY, float * maxX, float * maxY) {
  CHECK(nmb_Util::Unbuffer(buf, minX));
  CHECK(nmb_Util::Unbuffer(buf, minY));
  CHECK(nmb_Util::Unbuffer(buf, maxX));
  CHECK(nmb_Util::Unbuffer(buf, maxY));

  return 0;
}

char * nmm_Microscope::encode_ResistanceFailure (long * len,
           long which) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = sizeof(long);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_ResistanceFailure:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, which);
  }

  return msgbuf;
}

long nmm_Microscope::decode_ResistanceFailure (const char ** buf,
         long * which) {
  CHECK(nmb_Util::Unbuffer(buf, which));

  return 0;
}

char * nmm_Microscope::encode_Resistance (long * len,
           long which, long sec, long usec, float resistance) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = 3 * sizeof(long) + sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_Resistance:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, which);
    nmb_Util::Buffer(&mptr, &mlen, sec);
    nmb_Util::Buffer(&mptr, &mlen, usec);
    nmb_Util::Buffer(&mptr, &mlen, resistance);
  }

  return msgbuf;
}

long nmm_Microscope::decode_Resistance (const char ** buf,
         long * which, long * sec, long * usec, float * resistance) {
  CHECK(nmb_Util::Unbuffer(buf, which));
  CHECK(nmb_Util::Unbuffer(buf, sec));
  CHECK(nmb_Util::Unbuffer(buf, usec));
  CHECK(nmb_Util::Unbuffer(buf, resistance));

  return 0;
}

char * nmm_Microscope::encode_Resistance2(long * len,
           long which, long sec, long usec, float resistance,
           float voltage, float range, float filter) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = 3 * sizeof(long) + 4 * sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_Resistance2:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, which);
    nmb_Util::Buffer(&mptr, &mlen, sec);
    nmb_Util::Buffer(&mptr, &mlen, usec);
    nmb_Util::Buffer(&mptr, &mlen, resistance);
    nmb_Util::Buffer(&mptr, &mlen, voltage);
    nmb_Util::Buffer(&mptr, &mlen, range);
    nmb_Util::Buffer(&mptr, &mlen, filter);
  }

  return msgbuf;
}

long nmm_Microscope::decode_Resistance2(const char ** buf,
         long * which, long * sec, long * usec, float * resistance,
         float * voltage, float * range, float * filter) {
  CHECK(nmb_Util::Unbuffer(buf, which));
  CHECK(nmb_Util::Unbuffer(buf, sec));
  CHECK(nmb_Util::Unbuffer(buf, usec));
  CHECK(nmb_Util::Unbuffer(buf, resistance));
  CHECK(nmb_Util::Unbuffer(buf, voltage));
  CHECK(nmb_Util::Unbuffer(buf, range));
  CHECK(nmb_Util::Unbuffer(buf, filter));

  return 0;
}

char * nmm_Microscope::encode_ResistanceWithStatus(long * len,
           long which, long sec, long usec, float resistance,
           float voltage, float range, float filter, long status) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = 4 * sizeof(long) + 4 * sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_ResistanceWithStatus:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, which);
    nmb_Util::Buffer(&mptr, &mlen, sec);
    nmb_Util::Buffer(&mptr, &mlen, usec);
    nmb_Util::Buffer(&mptr, &mlen, resistance);
    nmb_Util::Buffer(&mptr, &mlen, voltage);
    nmb_Util::Buffer(&mptr, &mlen, range);
    nmb_Util::Buffer(&mptr, &mlen, filter);
	nmb_Util::Buffer(&mptr, &mlen, status);
  }

  return msgbuf;
}

long nmm_Microscope::decode_ResistanceWithStatus(const char ** buf,
         long * which, long * sec, long * usec, float * resistance,
         float * voltage, float * range, float * filter, long * status) {
  CHECK(nmb_Util::Unbuffer(buf, which));
  CHECK(nmb_Util::Unbuffer(buf, sec));
  CHECK(nmb_Util::Unbuffer(buf, usec));
  CHECK(nmb_Util::Unbuffer(buf, resistance));
  CHECK(nmb_Util::Unbuffer(buf, voltage));
  CHECK(nmb_Util::Unbuffer(buf, range));
  CHECK(nmb_Util::Unbuffer(buf, filter));
  CHECK(nmb_Util::Unbuffer(buf, status));

  return 0;
}

char * nmm_Microscope::encode_ReportSlowScan (long * len,
           long isEnabled) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = sizeof(long);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_ReportSlowScan:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, isEnabled);
  }

  return msgbuf;
}

long nmm_Microscope::decode_ReportSlowScan (const char ** buf,
         long * isEnabled) {
  CHECK(nmb_Util::Unbuffer(buf, isEnabled));

  return 0;
}

#if 0
char * nmm_Microscope::encode_ScanParameters (long * len,
           char * buffer) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;
  long padding;

  if (!len) return NULL;

  *len = sizeof(0);  // TODO

  // align to a 4-byte boundary
  padding = 4 - (*len % 4);
  *len += padding;

  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_ScanParameters:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, , *len);  // TODO
  }

  return msgbuf;
}
#endif  // 0

long nmm_Microscope::decode_ScanParameters (const char ** buf,
         long * length, char ** buffer) {
  long padding;

  CHECK(nmb_Util::Unbuffer(buf, length));

  *buffer = new char [*length];
  if (!*buffer) {
    fprintf(stderr, "nmm_Microscope::decode_ScanParameters:  "
                    "Out of memory!\n");
    return -1;
  }

  CHECK(nmb_Util::Unbuffer(buf, *buffer, *length));

  // align to a 4-byte boundary
  padding = 4 - (*length % 4);
  *buf += padding;

  return 0;
}

char * nmm_Microscope::encode_HelloMessage (long * len,
           char * magic, char * name, long majorVersion, long minorVersion) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = (4 + STM_NAME_LENGTH) * sizeof(char) + 2 * sizeof(long);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_HelloMessage:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, magic, 4);
    nmb_Util::Buffer(&mptr, &mlen, name, STM_NAME_LENGTH);
    nmb_Util::Buffer(&mptr, &mlen, majorVersion);
    nmb_Util::Buffer(&mptr, &mlen, minorVersion);
  }

  return msgbuf;
}

long nmm_Microscope::decode_HelloMessage (const char ** buf,
         char * magic, char * name, long * majorVersion, long * minorVersion) {
  CHECK(nmb_Util::Unbuffer(buf, magic, 4));
  CHECK(nmb_Util::Unbuffer(buf, name, STM_NAME_LENGTH));
  CHECK(nmb_Util::Unbuffer(buf, majorVersion));
  CHECK(nmb_Util::Unbuffer(buf, minorVersion));

  return 0;
}

char * nmm_Microscope::encode_ClientHello (long * len,
           char * magic, char * name, long majorVersion, long minorVersion) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = (4 + STM_NAME_LENGTH) * sizeof(char) + 2 * sizeof(long);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_ClientHello:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, magic, 4);
    nmb_Util::Buffer(&mptr, &mlen, name, STM_NAME_LENGTH);
    nmb_Util::Buffer(&mptr, &mlen, majorVersion);
    nmb_Util::Buffer(&mptr, &mlen, minorVersion);
  }

  return msgbuf;
}

long nmm_Microscope::decode_ClientHello (const char ** buf,
         char * magic, char * name, long * majorVersion, long * minorVersion) {
  CHECK(nmb_Util::Unbuffer(buf, magic, 4));
  CHECK(nmb_Util::Unbuffer(buf, name, STM_NAME_LENGTH));
  CHECK(nmb_Util::Unbuffer(buf, majorVersion));
  CHECK(nmb_Util::Unbuffer(buf, minorVersion));

  return 0;
}


long nmm_Microscope::decode_ScanDatasetHeader (const char ** buf,
         long * count) {
  CHECK(nmb_Util::Unbuffer(buf, count));
// NANO BEGIN
  //  fprintf(stderr, "nmm_Microscope::decode_ScanDatasetHeader(): numDatasets = %ld\n", *count);
// NANO END

  return 0;
}

long nmm_Microscope::decode_ScanDataset (const char ** buf,
         char * name, char * units, float * offset,
                               float * scale) {
  CHECK(nmb_Util::Unbuffer(buf, name, STM_NAME_LENGTH));
  CHECK(nmb_Util::Unbuffer(buf, units, STM_NAME_LENGTH));
  CHECK(nmb_Util::Unbuffer(buf, offset));
  CHECK(nmb_Util::Unbuffer(buf, scale));

  return 0;
}

long nmm_Microscope::decode_PointDatasetHeader (const char ** buf,
         long * count) {
  CHECK(nmb_Util::Unbuffer(buf, count));

  return 0;
}

long nmm_Microscope::decode_PointDataset (const char ** buf,
         char * name, char * units, long * numSamples,
                               float * offset, float * scale) {
  CHECK(nmb_Util::Unbuffer(buf, name, STM_NAME_LENGTH));
  CHECK(nmb_Util::Unbuffer(buf, units, STM_NAME_LENGTH));
  CHECK(nmb_Util::Unbuffer(buf, numSamples));
  CHECK(nmb_Util::Unbuffer(buf, offset));
  CHECK(nmb_Util::Unbuffer(buf, scale));

  return 0;
}

char * nmm_Microscope::encode_PidParameters (long * len,
           float p, float i, float d) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = 3 * sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_PidParameters:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, p);
    nmb_Util::Buffer(&mptr, &mlen, i);
    nmb_Util::Buffer(&mptr, &mlen, d);
  }

  return msgbuf;
}

long nmm_Microscope::decode_PidParameters (const char ** buf,
         float * p, float * i, float * d) {
  CHECK(nmb_Util::Unbuffer(buf, p));
  CHECK(nmb_Util::Unbuffer(buf, i));
  CHECK(nmb_Util::Unbuffer(buf, d));

  return 0;
}

char * nmm_Microscope::encode_ScanrateParameter (long * len,
           float rate) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_ScanrateParameter:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, rate);
  }

  return msgbuf;
}

long nmm_Microscope::decode_ScanrateParameter (const char ** buf,
         float * rate) {
  CHECK(nmb_Util::Unbuffer(buf, rate));

  return 0;
}

char * nmm_Microscope::encode_ReportGridSize (long * len,
           long x, long y) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = 2 * sizeof(long);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_ReportGridSize:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, x);
    nmb_Util::Buffer(&mptr, &mlen, y);
  }

  return msgbuf;
}

long nmm_Microscope::decode_ReportGridSize (const char ** buf,
         long * x, long * y) {
  CHECK(nmb_Util::Unbuffer(buf, x));
  CHECK(nmb_Util::Unbuffer(buf, y));

  return 0;
}

char * nmm_Microscope::encode_ServerPacketTimestamp (long * len,
           long sec, long usec) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = 2 * sizeof(long);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_ServerPacketTimestamp:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, sec);
    nmb_Util::Buffer(&mptr, &mlen, usec);
  }

  return msgbuf;
}

long nmm_Microscope::decode_ServerPacketTimestamp (const char ** buf,
         long * sec, long * usec) {
  CHECK(nmb_Util::Unbuffer(buf, sec));
  CHECK(nmb_Util::Unbuffer(buf, usec));

  return 0;
}

// Tiger implemented it
char * nmm_Microscope::encode_TopoFileHeader (long * len,
           char * buf, long size ) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = sizeof(long)+size*sizeof(char);  // HACK XXX Tiger
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_TopoFileHeader:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, size);  // HACK XXX Tiger
    nmb_Util::Buffer(&mptr, &mlen, buf, (long)size);  // HACK XXX Tiger
  }

  return msgbuf;
}
// Tiger

long nmm_Microscope::decode_TopoFileHeader (const char ** buf,
         long * length, char ** header) {
  CHECK(nmb_Util::Unbuffer(buf, length));

  *header = new char [*length];
  if (!*header) {
    fprintf(stderr, "nmm_Microscope::decode_TopoFileHeader:  "
                    "Out of memory!\n");
    return -1;
  }

  CHECK(nmb_Util::Unbuffer(buf, *header, *length));

  return 0;
}


char * nmm_Microscope::encode_ForceCurveData (long * len, float x, float y, 
    long num_points, long num_halfcycles, long sec, long usec, 
    float *z, float **data){

  char * msgbuf = NULL;
  char * mptr;
  long mlen;
  long i, j;

  if (!len) return NULL;

  *len = 2*sizeof(long) + 2*sizeof(float) + 
		num_points*(num_halfcycles+1)*sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_ForceCurveData:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, x);
    nmb_Util::Buffer(&mptr, &mlen, y);
    nmb_Util::Buffer(&mptr, &mlen, num_points);
    nmb_Util::Buffer(&mptr, &mlen, num_halfcycles);
    nmb_Util::Buffer(&mptr, &mlen, sec);
    nmb_Util::Buffer(&mptr, &mlen, usec);
    for (i = 0; i < num_points; i++){
      nmb_Util::Buffer(&mptr, &mlen, z[i]);
      for (j = 0; j < num_halfcycles; j++){
        nmb_Util::Buffer(&mptr, &mlen, data[j][i]);
      }
    }
  }

  return msgbuf;
  
}

long nmm_Microscope::decode_ForceCurveDataHeader (const char ** buf,
	float *x, float *y, long *num_points, long *num_halfcycles, 
	long *sec, long *usec){
  CHECK(nmb_Util::Unbuffer(buf, x));
  CHECK(nmb_Util::Unbuffer(buf, y));
  CHECK(nmb_Util::Unbuffer(buf, num_points));
  CHECK(nmb_Util::Unbuffer(buf, num_halfcycles));
  CHECK(nmb_Util::Unbuffer(buf, sec));
  CHECK(nmb_Util::Unbuffer(buf, usec));

  return 0;
}

long nmm_Microscope::decode_ForceCurveDataSingleLevel (const char ** buf,
	long num_halfcycles, float *z, float * data){
  long i;

  CHECK(nmb_Util::Unbuffer(buf, z));
  for (i = 0; i < num_halfcycles; i++)
    CHECK(nmb_Util::Unbuffer(buf, &(data[i])));

  return 0;
}


// messages for Michele Clark's experiments
char * nmm_Microscope::encode_RecvTimestamp (long * len,
           struct timeval t) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = sizeof(struct timeval);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_RecvTimestamp:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, t);
  }

  return msgbuf;
}

long nmm_Microscope::decode_RecvTimestamp (const char ** buf,
         struct timeval * time) {
  CHECK(nmb_Util::Unbuffer(buf, time));

  return 0;
}

char * nmm_Microscope::encode_FakeSendTimestamp (long * len,
                                               struct timeval t) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = sizeof(struct timeval);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_FakeSendTimestamp:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, t);
  }

  return msgbuf;
}

long nmm_Microscope::decode_FakeSendTimestamp (const char ** buf,
         struct timeval * time) {
  CHECK(nmb_Util::Unbuffer(buf, time));

  return 0;
}

char * nmm_Microscope::encode_UdpSeqNum (long * len,
           long sn) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = sizeof(long);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_UdpSeqNum:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, sn);
  }

  return msgbuf;
}

long nmm_Microscope::decode_UdpSeqNum (const char ** buf,
         long * number) {
  CHECK(nmb_Util::Unbuffer(buf, number));

  return 0;
}




char * nmm_Microscope::encode_EnterTappingMode (long * len,
           float p, float i, float d, float setpoint, float amplitude) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = 5 * sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_EnterTappingMode:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, p);
    nmb_Util::Buffer(&mptr, &mlen, i);
    nmb_Util::Buffer(&mptr, &mlen, d);
    nmb_Util::Buffer(&mptr, &mlen, setpoint);
    nmb_Util::Buffer(&mptr, &mlen, amplitude);
  }

  return msgbuf;
}

long nmm_Microscope::decode_EnterTappingMode (const char ** buf,
         float * p, float * i, float * d, float * setpoint,
         float * amplitude) {
  CHECK(nmb_Util::Unbuffer(buf, p));
  CHECK(nmb_Util::Unbuffer(buf, i));
  CHECK(nmb_Util::Unbuffer(buf, d));
  CHECK(nmb_Util::Unbuffer(buf, setpoint));
  CHECK(nmb_Util::Unbuffer(buf, amplitude));

  return 0;
}



char * nmm_Microscope::encode_EnterContactMode (long * len,
           float p, float i, float d, float setpoint) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = 4 * sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_EnterContactMode:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, p);
    nmb_Util::Buffer(&mptr, &mlen, i);
    nmb_Util::Buffer(&mptr, &mlen, d);
    nmb_Util::Buffer(&mptr, &mlen, setpoint);
  }

  return msgbuf;
}

long nmm_Microscope::decode_EnterContactMode (const char ** buf,
         float * p, float * i, float * d, float * setpoint) {
  CHECK(nmb_Util::Unbuffer(buf, p));
  CHECK(nmb_Util::Unbuffer(buf, i));
  CHECK(nmb_Util::Unbuffer(buf, d));
  CHECK(nmb_Util::Unbuffer(buf, setpoint));

  return 0;
}

char * nmm_Microscope::encode_EnterDirectZControl (long * len,
           float max_z_step, float max_xy_step, float min_setpoint, 
		 float max_setpoint, float max_lateral_force) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = 5 * sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_EnterDirectZControl:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, max_z_step);
    nmb_Util::Buffer(&mptr, &mlen, max_xy_step);
    nmb_Util::Buffer(&mptr, &mlen, min_setpoint);
    nmb_Util::Buffer(&mptr, &mlen, max_setpoint);
    nmb_Util::Buffer(&mptr, &mlen, max_lateral_force);
  }

  return msgbuf;
}
long nmm_Microscope::decode_EnterDirectZControl (const char ** buf,
           float * max_z_step, float * max_xy_step, float * min_setpoint, 
		 float * max_setpoint, float * max_lateral_force) {
  CHECK(nmb_Util::Unbuffer(buf, max_z_step));
  CHECK(nmb_Util::Unbuffer(buf, max_xy_step));
  CHECK(nmb_Util::Unbuffer(buf, min_setpoint));
  CHECK(nmb_Util::Unbuffer(buf, max_setpoint));
  CHECK(nmb_Util::Unbuffer(buf, max_lateral_force));

  return 0;
}



char * nmm_Microscope::encode_EnterSewingStyle (long * len,
           float setpoint, float bottomDelay, float topDelay,
           float pullBackDistance, float distanceBetweenPunches,
           float speed, float limitOfDescent) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = 7 * sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_EnterSewingStyle:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, setpoint);
    nmb_Util::Buffer(&mptr, &mlen, bottomDelay);
    nmb_Util::Buffer(&mptr, &mlen, topDelay);
    nmb_Util::Buffer(&mptr, &mlen, pullBackDistance);
    nmb_Util::Buffer(&mptr, &mlen, distanceBetweenPunches);
    nmb_Util::Buffer(&mptr, &mlen, speed);
    nmb_Util::Buffer(&mptr, &mlen, limitOfDescent);
  }

  return msgbuf;
}

long nmm_Microscope::decode_EnterSewingStyle (const char ** buf,
         float * setpoint, float * bottomDelay, float * topDelay,
         float * pullBackDistance, float * distanceBetweenPunches,
         float * speed, float * limitOfDescent) {
  CHECK(nmb_Util::Unbuffer(buf, setpoint));
  CHECK(nmb_Util::Unbuffer(buf, bottomDelay));
  CHECK(nmb_Util::Unbuffer(buf, topDelay));
  CHECK(nmb_Util::Unbuffer(buf, pullBackDistance));
  CHECK(nmb_Util::Unbuffer(buf, distanceBetweenPunches));
  CHECK(nmb_Util::Unbuffer(buf, speed));
  CHECK(nmb_Util::Unbuffer(buf, limitOfDescent));

  return 0;
}

// delay at z_start (usec)
// distance at which to start acquiring
// distance at which to stop acquiring (nm)
// initial pull back distance (nm)
// maximum force at which to stop descent (nA)
// distance between force curves (nm)
// number of different z values to sample at
// number of down-up curves to acquire per pnt

char * nmm_Microscope::encode_EnterSpectroscopyMode (long * len, float setpoint,
	float startDelay, float zStart, float zEnd, float zPullback,
	float forceLimit, float distBetweenFC, long numPoints, 
	long numHalfcycles, float sampleSpeed, float pullbackSpeed,
	float startSpeed, float feedbackSpeed, long avgNum,
	float sampleDelay, float pullbackDelay, float feedbackDelay) {

  char *msgbuf = NULL;
  char *mptr;
  long mlen;

  if (!len) return NULL;

  *len = 17 * sizeof(float);
  msgbuf = new char[*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_EnterSpectroscopyMode: "
		    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, setpoint);
    nmb_Util::Buffer(&mptr, &mlen, startDelay);
    nmb_Util::Buffer(&mptr, &mlen, zStart);
    nmb_Util::Buffer(&mptr, &mlen, zEnd);
    nmb_Util::Buffer(&mptr, &mlen, zPullback);
    nmb_Util::Buffer(&mptr, &mlen, forceLimit);
    nmb_Util::Buffer(&mptr, &mlen, distBetweenFC);
    nmb_Util::Buffer(&mptr, &mlen, numPoints);
    nmb_Util::Buffer(&mptr, &mlen, numHalfcycles);
    nmb_Util::Buffer(&mptr, &mlen, sampleSpeed);
    nmb_Util::Buffer(&mptr, &mlen, pullbackSpeed);
    nmb_Util::Buffer(&mptr, &mlen, startSpeed);
    nmb_Util::Buffer(&mptr, &mlen, feedbackSpeed);
    nmb_Util::Buffer(&mptr, &mlen, avgNum);
    nmb_Util::Buffer(&mptr, &mlen, sampleDelay);
    nmb_Util::Buffer(&mptr, &mlen, pullbackDelay);
    nmb_Util::Buffer(&mptr, &mlen, feedbackDelay);
  }
  return msgbuf;
}

long nmm_Microscope::decode_EnterSpectroscopyMode (const char ** buf,
         float * setpoint, float * startDelay, float * zStart, float * zEnd,
         float * zPullback, float * forceLimit,
         float * distBetweenFC, long * numPoints, long * numHalfcycles,
	 float * sampleSpeed, float * pullbackSpeed, float * startSpeed,
	 float * feedbackSpeed, long *avgNum, 
	 float * sampleDelay, float * pullbackDelay, float * feedbackDelay) {
  CHECK(nmb_Util::Unbuffer(buf, setpoint));
  CHECK(nmb_Util::Unbuffer(buf, startDelay));
  CHECK(nmb_Util::Unbuffer(buf, zStart));
  CHECK(nmb_Util::Unbuffer(buf, zEnd));
  CHECK(nmb_Util::Unbuffer(buf, zPullback));
  CHECK(nmb_Util::Unbuffer(buf, forceLimit));
  CHECK(nmb_Util::Unbuffer(buf, distBetweenFC));
  CHECK(nmb_Util::Unbuffer(buf, numPoints));
  CHECK(nmb_Util::Unbuffer(buf, numHalfcycles));
  CHECK(nmb_Util::Unbuffer(buf, sampleSpeed));
  CHECK(nmb_Util::Unbuffer(buf, pullbackSpeed));
  CHECK(nmb_Util::Unbuffer(buf, startSpeed));
  CHECK(nmb_Util::Unbuffer(buf, feedbackSpeed));
  CHECK(nmb_Util::Unbuffer(buf, avgNum));
  CHECK(nmb_Util::Unbuffer(buf, sampleDelay));
  CHECK(nmb_Util::Unbuffer(buf, pullbackDelay));
  CHECK(nmb_Util::Unbuffer(buf, feedbackDelay));

  return 0;
}

char * nmm_Microscope::encode_InTappingMode (long * len,
           float p, float i, float d, float setpoint,
	   float amplitude) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = 5 * sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_InTappingMode:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, p);
    nmb_Util::Buffer(&mptr, &mlen, i);
    nmb_Util::Buffer(&mptr, &mlen, d);
    nmb_Util::Buffer(&mptr, &mlen, setpoint);
    nmb_Util::Buffer(&mptr, &mlen, amplitude);
  }

  return msgbuf;
}

long nmm_Microscope::decode_InTappingMode (const char ** buf,
         float * p, float * i, float * d, float * setpoint,
         float * amplitude) {
  CHECK(nmb_Util::Unbuffer(buf, p));
  CHECK(nmb_Util::Unbuffer(buf, i));
  CHECK(nmb_Util::Unbuffer(buf, d));
  CHECK(nmb_Util::Unbuffer(buf, setpoint));
  CHECK(nmb_Util::Unbuffer(buf, amplitude));

  return 0;
}

char * nmm_Microscope::encode_InContactMode (long * len,
           float p, float i, float d, float setpoint) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = 4 * sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_InContactMode:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, p);
    nmb_Util::Buffer(&mptr, &mlen, i);
    nmb_Util::Buffer(&mptr, &mlen, d);
    nmb_Util::Buffer(&mptr, &mlen, setpoint);
  }

  return msgbuf;
}

long nmm_Microscope::decode_InContactMode (const char ** buf,
         float * p, float * i, float * d, float * setpoint) {
  CHECK(nmb_Util::Unbuffer(buf, p));
  CHECK(nmb_Util::Unbuffer(buf, i));
  CHECK(nmb_Util::Unbuffer(buf, d));
  CHECK(nmb_Util::Unbuffer(buf, setpoint));

  return 0;
}

char * nmm_Microscope::encode_InDirectZControl (long * len,
				float max_z_step, float max_xy_step, 
				float min_setpoint, float max_setpoint, 
				float max_lateral_force,
				float freespace_norm_force,
				float freespace_lat_force) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = 7 * sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_InDirectZControl:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, max_z_step);
    nmb_Util::Buffer(&mptr, &mlen, max_xy_step);
    nmb_Util::Buffer(&mptr, &mlen, min_setpoint);
    nmb_Util::Buffer(&mptr, &mlen, max_setpoint);
    nmb_Util::Buffer(&mptr, &mlen, max_lateral_force);
    nmb_Util::Buffer(&mptr, &mlen, freespace_norm_force);
    nmb_Util::Buffer(&mptr, &mlen, freespace_lat_force);
  }

  return msgbuf;
}

long nmm_Microscope::decode_InDirectZControl (const char ** buf,
    float *max_z_step, float *max_xy_step, float *min_setpoint, 
    float *max_setpoint, float *max_lateral_force, 
    float *freespace_norm_force, float *freespace_lat_force) {
  CHECK(nmb_Util::Unbuffer(buf, max_z_step));
  CHECK(nmb_Util::Unbuffer(buf, max_xy_step));
  CHECK(nmb_Util::Unbuffer(buf, min_setpoint));
  CHECK(nmb_Util::Unbuffer(buf, max_setpoint));
  CHECK(nmb_Util::Unbuffer(buf, max_lateral_force));
  CHECK(nmb_Util::Unbuffer(buf, freespace_norm_force));
  CHECK(nmb_Util::Unbuffer(buf, freespace_lat_force));

  return 0;
}

char * nmm_Microscope::encode_InSewingStyle (long * len,
           float setpoint, float bottomDelay, float topDelay,
           float pullBackDistance, float distanceBetweenPunches,
           float speed, float limitOfDescent) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = 7 * sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_InSewingStyle:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, setpoint);
    nmb_Util::Buffer(&mptr, &mlen, bottomDelay);
    nmb_Util::Buffer(&mptr, &mlen, topDelay);
    nmb_Util::Buffer(&mptr, &mlen, pullBackDistance);
    nmb_Util::Buffer(&mptr, &mlen, distanceBetweenPunches);
    nmb_Util::Buffer(&mptr, &mlen, speed);
    nmb_Util::Buffer(&mptr, &mlen, limitOfDescent);
  }

  return msgbuf;
}

long nmm_Microscope::decode_InSewingStyle (const char ** buf,
         float * setpoint, float * bottomDelay, float * topDelay,
         float * pullBackDistance, float * distanceBetweenPunches,
         float * speed, float * limitOfDescent) {
  CHECK(nmb_Util::Unbuffer(buf, setpoint));
  CHECK(nmb_Util::Unbuffer(buf, bottomDelay));
  CHECK(nmb_Util::Unbuffer(buf, topDelay));
  CHECK(nmb_Util::Unbuffer(buf, pullBackDistance));
  CHECK(nmb_Util::Unbuffer(buf, distanceBetweenPunches));
  CHECK(nmb_Util::Unbuffer(buf, speed));
  CHECK(nmb_Util::Unbuffer(buf, limitOfDescent));

  return 0;
}

char * nmm_Microscope::encode_InSpectroscopyMode (long * len, float setpoint,
	float startDelay,
        float zStart, float zEnd, float zPullback,float forceLimit,
        float distBetweenFC, long numPoints, long numHalfcycles,
	float sampleSpeed, float pullbackSpeed, float startSpeed,
	float feedbackSpeed, long avgNum, 
	float sampleDelay, float pullbackDelay, float feedbackDelay) {

  char *msgbuf = NULL;
  char *mptr;
  long mlen;

  if (!len) return NULL;

  *len = 17 * sizeof(float);
  msgbuf = new char[*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_InSpectroscopyMode: "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, setpoint);
    nmb_Util::Buffer(&mptr, &mlen, startDelay);
    nmb_Util::Buffer(&mptr, &mlen, zStart);
    nmb_Util::Buffer(&mptr, &mlen, zEnd);
    nmb_Util::Buffer(&mptr, &mlen, zPullback);
    nmb_Util::Buffer(&mptr, &mlen, forceLimit);
    nmb_Util::Buffer(&mptr, &mlen, distBetweenFC);
    nmb_Util::Buffer(&mptr, &mlen, numPoints);
    nmb_Util::Buffer(&mptr, &mlen, numHalfcycles);
    nmb_Util::Buffer(&mptr, &mlen, sampleSpeed);
    nmb_Util::Buffer(&mptr, &mlen, pullbackSpeed);
    nmb_Util::Buffer(&mptr, &mlen, startSpeed);
    nmb_Util::Buffer(&mptr, &mlen, feedbackSpeed);
    nmb_Util::Buffer(&mptr, &mlen, avgNum);
    nmb_Util::Buffer(&mptr, &mlen, sampleDelay);
    nmb_Util::Buffer(&mptr, &mlen, pullbackDelay);
    nmb_Util::Buffer(&mptr, &mlen, feedbackDelay);
  }
  return msgbuf;
}

long nmm_Microscope::decode_InSpectroscopyMode (const char ** buf,
         float *setpoint, float * startDelay, float * zStart, float * zEnd,
         float * zPullback, float * forceLimit,
         float * distBetweenFC, long * numPoints, long * numHalfcycles,
	 float * sampleSpeed, float * pullbackSpeed, float * startSpeed,
	 float * feedbackSpeed, long * avgNum,
	 float * sampleDelay, float * pullbackDelay, float * feedbackDelay) {
  CHECK(nmb_Util::Unbuffer(buf, setpoint));
  CHECK(nmb_Util::Unbuffer(buf, startDelay));
  CHECK(nmb_Util::Unbuffer(buf, zStart));
  CHECK(nmb_Util::Unbuffer(buf, zEnd));
  CHECK(nmb_Util::Unbuffer(buf, zPullback));
  CHECK(nmb_Util::Unbuffer(buf, forceLimit));
  CHECK(nmb_Util::Unbuffer(buf, distBetweenFC));
  CHECK(nmb_Util::Unbuffer(buf, numPoints));
  CHECK(nmb_Util::Unbuffer(buf, numHalfcycles));
  CHECK(nmb_Util::Unbuffer(buf, sampleSpeed));
  CHECK(nmb_Util::Unbuffer(buf, pullbackSpeed));
  CHECK(nmb_Util::Unbuffer(buf, startSpeed));
  CHECK(nmb_Util::Unbuffer(buf, feedbackSpeed));
  CHECK(nmb_Util::Unbuffer(buf, avgNum));
  CHECK(nmb_Util::Unbuffer(buf, sampleDelay));
  CHECK(nmb_Util::Unbuffer(buf, pullbackDelay));
  CHECK(nmb_Util::Unbuffer(buf, feedbackDelay));

  return 0;
}

char * nmm_Microscope::encode_ForceParameters (long * len,
       long enableModify, float scrap) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = sizeof(long) + sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_ForceParameters:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, enableModify);
    nmb_Util::Buffer(&mptr, &mlen, scrap);
  }

  return msgbuf;
}

long nmm_Microscope::decode_ForceParameters (const char ** buf,
         long * enableModify, float * scrap) {
  CHECK(nmb_Util::Unbuffer(buf, enableModify));
  CHECK(nmb_Util::Unbuffer(buf, scrap));

  return 0;
}

char * nmm_Microscope::encode_BaseModParameters (long * len,
           float min, float max) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = 2 * sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_BaseModParameters:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, min);
    nmb_Util::Buffer(&mptr, &mlen, max);
  }

  return msgbuf;
}

long nmm_Microscope::decode_BaseModParameters (const char ** buf,
         float * min, float * max) {
  CHECK(nmb_Util::Unbuffer(buf, min));
  CHECK(nmb_Util::Unbuffer(buf, max));

  return 0;
}

char * nmm_Microscope::encode_ForceSettings (long * len,
           float min, float max, float setpoint) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = 3 * sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_ForceSettings:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, min);
    nmb_Util::Buffer(&mptr, &mlen, max);
    nmb_Util::Buffer(&mptr, &mlen, setpoint);
  }

  return msgbuf;
}

long nmm_Microscope::decode_ForceSettings (const char ** buf,
         float * min, float * max, float * setpoint) {
  CHECK(nmb_Util::Unbuffer(buf, min));
  CHECK(nmb_Util::Unbuffer(buf, max));
  CHECK(nmb_Util::Unbuffer(buf, setpoint));

  return 0;
}

char * nmm_Microscope::encode_InModModeT (long * len,
           long sec, long usec) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = 2 * sizeof(long);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_InModModeT:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, sec);
    nmb_Util::Buffer(&mptr, &mlen, usec);
  }

  return msgbuf;
}

long nmm_Microscope::decode_InModModeT (const char ** buf,
         long * sec, long * usec) {
  CHECK(nmb_Util::Unbuffer(buf, sec));
  CHECK(nmb_Util::Unbuffer(buf, usec));

  return 0;
}

char * nmm_Microscope::encode_InImgModeT (long * len,
           long sec, long usec) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = 2 * sizeof(long);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_InImgModeT:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, sec);
    nmb_Util::Buffer(&mptr, &mlen, usec);
  }

  return msgbuf;
}

long nmm_Microscope::decode_InImgModeT (const char ** buf,
         long * sec, long * usec) {
  CHECK(nmb_Util::Unbuffer(buf, sec));
  CHECK(nmb_Util::Unbuffer(buf, usec));

  return 0;
}

char * nmm_Microscope::encode_ModForceSet (long * len,
           float setpoint) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_ModForceSet:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, setpoint);
  }

  return msgbuf;
}

long nmm_Microscope::decode_ModForceSet (const char ** buf,
         float * setpoint) {
  CHECK(nmb_Util::Unbuffer(buf, setpoint));

  return 0;
}

char * nmm_Microscope::encode_ImgForceSet (long * len,
           float setpoint) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_ImgForceSet:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, setpoint);
  }

  return msgbuf;
}

long nmm_Microscope::decode_ImgForceSet (const char ** buf,
         float * setpoint) {
  CHECK(nmb_Util::Unbuffer(buf, setpoint));

  return 0;
}

char * nmm_Microscope::encode_ModSet (long * len,
           long enableModify, float min, float max, float setpoint) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = sizeof(long) + 3 * sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_ModSet:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, enableModify);
    nmb_Util::Buffer(&mptr, &mlen, min);
    nmb_Util::Buffer(&mptr, &mlen, max);
    nmb_Util::Buffer(&mptr, &mlen, setpoint);
  }

  return msgbuf;
}

long nmm_Microscope::decode_ModSet (const char ** buf,
         long * enableModify, float * min, float * max, float * setpoint) {
  CHECK(nmb_Util::Unbuffer(buf, enableModify));
  CHECK(nmb_Util::Unbuffer(buf, min));
  CHECK(nmb_Util::Unbuffer(buf, max));
  CHECK(nmb_Util::Unbuffer(buf, setpoint));

  return 0;
}

char * nmm_Microscope::encode_ImgSet (long * len,
           long enableModify, float min, float max, float setpoint) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = sizeof(long) + 3 * sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_ImgSet:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, enableModify);
    nmb_Util::Buffer(&mptr, &mlen, max);  // NOTE REVERSAL OF MAX AND MIN
    nmb_Util::Buffer(&mptr, &mlen, min);
    nmb_Util::Buffer(&mptr, &mlen, setpoint);
  }

  return msgbuf;
}

long nmm_Microscope::decode_ImgSet (const char ** buf,
         long * enableModify, float * min, float * max, float * setpoint) {
  CHECK(nmb_Util::Unbuffer(buf, enableModify));
  CHECK(nmb_Util::Unbuffer(buf, min));
  CHECK(nmb_Util::Unbuffer(buf, max));
  CHECK(nmb_Util::Unbuffer(buf, setpoint));

  return 0;
}

char * nmm_Microscope::encode_ForceSet (long * len,
           float scrap) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_ForceSet:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, scrap);
  }

  return msgbuf;
}

long nmm_Microscope::decode_ForceSet (const char ** buf,
         float * scrap) {
  CHECK(nmb_Util::Unbuffer(buf, scrap));

  return 0;
}

char * nmm_Microscope::encode_ForceSetFailure (long * len,
           float scrap) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_ForceSetFailure:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, scrap);
  }

  return msgbuf;
}

long nmm_Microscope::decode_ForceSetFailure (const char ** buf,
         float * scrap) {
  CHECK(nmb_Util::Unbuffer(buf, scrap));

  return 0;
}





char * nmm_Microscope::encode_PulseParameters (long * len,
           long enabled, float biasVoltage, float peakVoltage, float width) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = sizeof(long) + 3 * sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_PulseParameters:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, enabled);
    nmb_Util::Buffer(&mptr, &mlen, biasVoltage);
    nmb_Util::Buffer(&mptr, &mlen, peakVoltage);
    nmb_Util::Buffer(&mptr, &mlen, width);
  }

  return msgbuf;
}

long nmm_Microscope::decode_PulseParameters (const char ** buf,
        long * enabled, float * biasVoltage, float * peakVoltage,
        float * width) {
  CHECK(nmb_Util::Unbuffer(buf, enabled));
  CHECK(nmb_Util::Unbuffer(buf, biasVoltage));
  CHECK(nmb_Util::Unbuffer(buf, peakVoltage));
  CHECK(nmb_Util::Unbuffer(buf, width));

  return 0;
}

char * nmm_Microscope::encode_PulseCompletedNM (long * len,
           float x, float y) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = 2 * sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_PulseCompletedNM:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, x);
    nmb_Util::Buffer(&mptr, &mlen, y);
  }

  return msgbuf;
}

long nmm_Microscope::decode_PulseCompletedNM (const char ** buf,
        float * x, float * y) {
  CHECK(nmb_Util::Unbuffer(buf, x));
  CHECK(nmb_Util::Unbuffer(buf, y));

  return 0;
}

char * nmm_Microscope::encode_PulseFailureNM (long * len,
           float x, float y) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = 2 * sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_PulseFailureNM:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, x);
    nmb_Util::Buffer(&mptr, &mlen, y);
  }

  return msgbuf;
}

long nmm_Microscope::decode_PulseFailureNM (const char ** buf,
        float * x, float * y) {
  CHECK(nmb_Util::Unbuffer(buf, x));
  CHECK(nmb_Util::Unbuffer(buf, y));

  return 0;
}

char * nmm_Microscope::encode_SetBias (long * len, float voltage) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_SetBias:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, voltage);
  }
  return msgbuf;
}

long nmm_Microscope::decode_SetBias (const char ** buf, float * voltage) {
  CHECK(nmb_Util::Unbuffer(buf, voltage));

  return 0;
}

char * nmm_Microscope::encode_SetPulsePeak (long * len, float voltage) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_SetPulsePeak:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, voltage);
  }
  return msgbuf;
}

long nmm_Microscope::decode_SetPulsePeak (const char ** buf, float * voltage) {
  CHECK(nmb_Util::Unbuffer(buf, voltage));

  return 0;
}

char * nmm_Microscope::encode_SetPulseDuration (long * len, float duration) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_SetPulseDuration:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, duration);
  }
  return msgbuf;
}

long nmm_Microscope::decode_SetPulseDuration (const char ** buf,
                                              float * duration) {
  CHECK(nmb_Util::Unbuffer(buf, duration));

  return 0;
}

char * nmm_Microscope::encode_EnterScanlineMode (long * len,
           long enable) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = 1 * sizeof(long);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_EnterScanlineMode:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, enable);
  }

  return msgbuf;
}

long nmm_Microscope::decode_EnterScanlineMode (const char ** buf,
         long *enable) {
  CHECK(nmb_Util::Unbuffer(buf, enable));

  return 0;
}

char * nmm_Microscope::encode_InScanlineMode (long * len,
           long enable) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = 1 * sizeof(long);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_InScanlineMode:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, enable);
  }

  return msgbuf;
}

long nmm_Microscope::decode_InScanlineMode (const char ** buf,
         long *enable) {
  CHECK(nmb_Util::Unbuffer(buf, enable));

  return 0;
}

char * nmm_Microscope::encode_RequestScanLine(long *len, float x, float y, 
				float z, float angle, float slope, float width, long res,
				long enable_feedback, long check_forcelimit,
				float max_force, float max_z_step, float max_xy_step) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;

  if (!len) return NULL;

  *len = 9*sizeof(float) + 3*sizeof(long);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_ScanLine:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, x);
    nmb_Util::Buffer(&mptr, &mlen, y);
    nmb_Util::Buffer(&mptr, &mlen, z);
    nmb_Util::Buffer(&mptr, &mlen, angle);
	nmb_Util::Buffer(&mptr, &mlen, slope);
    nmb_Util::Buffer(&mptr, &mlen, width);
    nmb_Util::Buffer(&mptr, &mlen, res);
	nmb_Util::Buffer(&mptr, &mlen, enable_feedback);
	nmb_Util::Buffer(&mptr, &mlen, check_forcelimit);
	nmb_Util::Buffer(&mptr, &mlen, max_force);
	nmb_Util::Buffer(&mptr, &mlen, max_z_step);
	nmb_Util::Buffer(&mptr, &mlen, max_xy_step);
  }
  return msgbuf;
}

long nmm_Microscope::decode_RequestScanLine(const char ** buf, float *x, 
	float *y, float *z, float *angle, float *slope, float *width, long *res,
	long *enable_feedback, long *check_forcelimit, float *max_force, 
	float *max_z_step, float *max_xy_step) {
  CHECK(nmb_Util::Unbuffer(buf, x));
  CHECK(nmb_Util::Unbuffer(buf, y));
  CHECK(nmb_Util::Unbuffer(buf, z));
  CHECK(nmb_Util::Unbuffer(buf, angle));
  CHECK(nmb_Util::Unbuffer(buf, slope));
  CHECK(nmb_Util::Unbuffer(buf, width));
  CHECK(nmb_Util::Unbuffer(buf, res));
  CHECK(nmb_Util::Unbuffer(buf, enable_feedback));
  CHECK(nmb_Util::Unbuffer(buf, check_forcelimit));
  CHECK(nmb_Util::Unbuffer(buf, max_force));
  CHECK(nmb_Util::Unbuffer(buf, max_z_step));
  CHECK(nmb_Util::Unbuffer(buf, max_xy_step));

  return 0;
}

// offset gives the location of the first element of each dataset in the
// data array; successive elements for each dataset are expected to 
// immediately follow the first (i.e. the data for a given channel is one
// contiguous block)
char * nmm_Microscope::encode_ScanlineData(long *len, float x, 
	float y, float z, float angle, float slope, float width, 
	long resolution, long feedback_enabled, long checking_forcelimit,
	float max_force_setting, float max_z_step, float max_xy_step,
	long sec, long usec,
	long num_channels, long * offset, float *data) {
  char * msgbuf = NULL;
  char * mptr;
  long mlen;
  long i,j;
  float val;

  if (!len) return NULL;

  *len = 9*sizeof(float) + 6*sizeof(long) + 
	num_channels*resolution*sizeof(float);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_ScanlineData:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, x);
    nmb_Util::Buffer(&mptr, &mlen, y);
    nmb_Util::Buffer(&mptr, &mlen, z);
    nmb_Util::Buffer(&mptr, &mlen, angle);
	nmb_Util::Buffer(&mptr, &mlen, slope);
    nmb_Util::Buffer(&mptr, &mlen, width);
	nmb_Util::Buffer(&mptr, &mlen, resolution);
	nmb_Util::Buffer(&mptr, &mlen, feedback_enabled);
	nmb_Util::Buffer(&mptr, &mlen, checking_forcelimit);
	nmb_Util::Buffer(&mptr, &mlen, max_force_setting);
	nmb_Util::Buffer(&mptr, &mlen, max_z_step);
	nmb_Util::Buffer(&mptr, &mlen, max_xy_step);
    nmb_Util::Buffer(&mptr, &mlen, sec);
	nmb_Util::Buffer(&mptr, &mlen, usec);
    nmb_Util::Buffer(&mptr, &mlen, num_channels);

    for (i = 0; i < resolution; i++)
      for (j = 0; j < num_channels; j++) {
	val = (float)(data[offset[j]+i]);
        nmb_Util::Buffer(&mptr, &mlen, val);
      }
  }
  return msgbuf;
}

long nmm_Microscope::decode_ScanlineDataHeader(const char ** buf, 
	float *x, float *y, float *z, float *angle, float *slope, float *width,
        long *resolution, long *feedback_enabled, long *checking_forcelimit,
		float *max_force_setting, float * max_z_step, float *max_xy_step,
		long *sec, long *usec, long *num_channels)
{
  CHECK(nmb_Util::Unbuffer(buf, x));
  CHECK(nmb_Util::Unbuffer(buf, y));
  CHECK(nmb_Util::Unbuffer(buf, z));
  CHECK(nmb_Util::Unbuffer(buf, angle));
  CHECK(nmb_Util::Unbuffer(buf, slope));
  CHECK(nmb_Util::Unbuffer(buf, width));
  CHECK(nmb_Util::Unbuffer(buf, resolution));
  CHECK(nmb_Util::Unbuffer(buf, feedback_enabled));
  CHECK(nmb_Util::Unbuffer(buf, checking_forcelimit));
  CHECK(nmb_Util::Unbuffer(buf, max_force_setting));
  CHECK(nmb_Util::Unbuffer(buf, max_z_step));
  CHECK(nmb_Util::Unbuffer(buf, max_xy_step));
  CHECK(nmb_Util::Unbuffer(buf, sec));
  CHECK(nmb_Util::Unbuffer(buf, usec));
  CHECK(nmb_Util::Unbuffer(buf, num_channels));

  return 0;
}

long nmm_Microscope::decode_ScanlineDataPoint(const char ** buf, 
				long fieldCount, float *fieldValues){
  for (int i = 0; i < fieldCount; i++)
    CHECK(nmb_Util::Unbuffer(buf, &(fieldValues[i])));

  return 0;
}

long nmm_Microscope::dispatchMessage (long len, const char * buf, long type) {
  struct timeval now;
  long retval;

  gettimeofday(&now, NULL);
  // If we aren't connected to anything, just pretend we sent the message
  // Useful if we are viewing a file, like a Topo file.
  if(d_connection) {
      retval = d_connection->pack_message(len, now, type, d_myId, (char *) buf,
                                      vrpn_CONNECTION_RELIABLE);
  } else {
      retval = 0;
  }
  delete [] (char *) buf;

  return retval;
}

/*===3rdtech===
  Copyright (c) 2000 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/
#include "nmm_Microscope.h"

#include <vrpn_FileController.h>
#include <vrpn_Forwarder.h>
#include <vrpn_Shared.h>

#include <stm_cmd.h>	// for STM_NAME_LENGTH

#define CHECK(a) if ((a) == -1) return -1

nmm_Microscope::nmm_Microscope (
    const char * /*name*/,
    vrpn_Connection * connection)
//  d_connection (connection),             moved to nmb_Device
//  d_fileController (new vrpn_File_Controller (connection)),moved to nmb_Device
  {

  if (connection) {

/* ****************************************************************
   Can't do echo, because if you sent a message containing a message
   type then it wouldn't get translated correctly when sent back since that
   message id number may correspond to a different message on the client side

   This is why we have MarkModify and MarkImage messages but no echo message
   the way we did before vrpn.
**************************************************************** */

    d_SetRegionNM_type = connection->register_message_type
         ("nmm Microscope SetRegionNM");
    d_SetScanAngle_type = connection->register_message_type
         ("nmm Microscope SetScanAngle");
    d_ScanTo_type = connection->register_message_type
         ("nmm Microscope ScanTo");
    d_ScanToZ_type = connection->register_message_type
         ("nmm Microscope ScanToZ");
    d_ZagTo_type = connection->register_message_type
         ("nmm Microscope ZagTo");
    d_ZagToCenter_type = connection->register_message_type
         ("nmm Microscope ZagToCenter");

    d_FeelTo_type = connection->register_message_type
        ("nmm Microscope FeelTo");

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
    /* OBSOLETE */
    d_SetStdDevParams_type = connection->register_message_type
         ("nmm Microscope SetStdDevParams");

    d_SetScanWindow_type = connection->register_message_type
         ("nmm Microscope SetScanWindow");
    d_ResumeWindowScan_type = connection->register_message_type
         ("nmm Microscope ResumeWindowScan");
    d_PauseScanning_type = connection->register_message_type
         ("nmm Microscope PauseScanning");
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
    d_DrawSweepLineCenter_type = connection->register_message_type
         ("nmm Microscope DrawSweepLineCenter");
    d_DrawSweepArcCenter_type = connection->register_message_type
         ("nmm Microscope DrawSweepArcCenter");
    d_GetNewPointDatasets_type = connection->register_message_type
         ("nmm Microscope GetNewPointDatasets");
    d_GetNewScanDatasets_type = connection->register_message_type
         ("nmm Microscope GetNewScanDatasets");
    d_WithdrawTip_type = connection->register_message_type
	("nmm Microscope WithdrawTip");
    d_MarkModify_type = connection->register_message_type
	("nmm Microscope MarkModify");
    d_MarkImage_type = connection->register_message_type
        ("nmm Microscope MarkImage");
    // Used for the vrpn_dropped_connection type. 
//      d_Shutdown_type = connection->register_message_type
//           ("nmm Microscope Shutdown");
    d_QueryScanRange_type = connection->register_message_type
         ("nmm Microscope QueryScanRange");
    /* OBSOLETE */
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
    d_SuspendCommands_type = connection->register_message_type
        ("nmm Microscope SuspendCommands");
    d_ResumeCommands_type = connection->register_message_type
        ("nmm Microscope ResumeCommands");
    d_StartingToRelax_type = connection->register_message_type
         ("nmm Microscope StartingToRelax");
    d_RelaxSet_type = connection->register_message_type
         ("nmm Microscope RelaxSet");
    /* OBSOLETE */
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

    d_BeginFeelTo_type = connection->register_message_type
         ("nmm Microscope BeginFeelTo");
    d_EndFeelTo_type = connection->register_message_type
         ("nmm Microscope EndFeelTo");

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
    d_ReportScanAngle_type = connection->register_message_type
         ("nmm Microscope ReportScanAngle");
    d_Scanning_type = connection->register_message_type
         ("nmm Microscope Scanning");
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
    d_EnterOscillatingMode_type = connection->register_message_type
        ("nmm Microscope AFM EnterOscillatingMode");
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
    d_InOscillatingMode_type = connection->register_message_type
        ("nmm Microscope AFM InOscillatingMode");
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

    // unrelated to Scanline mode (this is for 2D scanning)
    d_JumpToScanLine_type = connection->register_message_type
        ("nmm Microscope JumpToScanLine");

    d_EnableUpdatableQueue_type = connection->register_message_type
        ("nmm Microscope EnableUpdatableQueue");
  }

//  if (servicename)	moved to nmb_Device
//    delete [] servicename;
}


nmm_Microscope::~nmm_Microscope (void) {

}

/*
long nmm_Microscope::mainloop (void) {
  if (d_connection)
    CHECK(d_connection->mainloop());

  return 0;
}
*/

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
         //"nmm Microscope SetStdDevParams",
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
         //"nmm Microscope QueryStdDevParams",
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
         //"nmm Microscope StdDevParameters",
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
         "nmm Microscope AFM EnterOscillatingMode",
         "nmm Microscope AFM EnterContactMode",
         "nmm Microscope AFM EnterSewingStyle",
         "nmm Microscope AFM EnterSpectroscopyMode",
         "nmm Microscope AFM SetContactForce",
         "nmm Microscope AFM QueryContactForce",
         "nmm Microscope AFM InTappingMode",
         "nmm Microscope AFM InOscillatingMode",
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

char * nmm_Microscope::encode_SetRegionNM (long * len,
                 vrpn_float32 minx, vrpn_float32 miny, 
                 vrpn_float32 maxx, vrpn_float32 maxy) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 4 * sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_SetRegionNM:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, minx);
    vrpn_buffer(&mptr, &mlen, miny);
    vrpn_buffer(&mptr, &mlen, maxx);
    vrpn_buffer(&mptr, &mlen, maxy);
  }

  return msgbuf;
}

long nmm_Microscope::decode_SetRegionNM (const char ** buf,
        vrpn_float32 * minx, vrpn_float32 * miny, 
        vrpn_float32 * maxx, vrpn_float32 * maxy) {
  CHECK(vrpn_unbuffer(buf, minx));
  CHECK(vrpn_unbuffer(buf, miny));
  CHECK(vrpn_unbuffer(buf, maxx));
  CHECK(vrpn_unbuffer(buf, maxy));

  return 0;
}

char * nmm_Microscope::encode_SetScanAngle (long * len,
					    vrpn_float32 angle) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_SetScanAngle:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, angle);
  }

  return msgbuf;
}

long nmm_Microscope::decode_SetScanAngle (const char ** buf,
        vrpn_float32 * angle) {
  CHECK(vrpn_unbuffer(buf, angle));

  return 0;
}



char * nmm_Microscope::encode_ScanTo (long * len,
                 vrpn_float32 x, vrpn_float32 y) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 2 * sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_ScanTo:  "
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
long nmm_Microscope::decode_ScanTo (const char ** buf,
         vrpn_float32 * x, vrpn_float32 * y) {
  CHECK(vrpn_unbuffer(buf, x));
  CHECK(vrpn_unbuffer(buf, y));

  return 0;
}

char * nmm_Microscope::encode_ScanTo (long * len,
                 vrpn_float32 x, vrpn_float32 y, vrpn_float32 z) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 3 * sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_ScanTo:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, x);
    vrpn_buffer(&mptr, &mlen, y);
    vrpn_buffer(&mptr, &mlen, z);
  }

  return msgbuf;
}
long nmm_Microscope::decode_ScanTo (const char ** buf,
         vrpn_float32 * x, vrpn_float32 * y, vrpn_float32 * z) {
  CHECK(vrpn_unbuffer(buf, x));
  CHECK(vrpn_unbuffer(buf, y));
  CHECK(vrpn_unbuffer(buf, z));

  return 0;
}



char * nmm_Microscope::encode_ZagTo (long * len,
                 vrpn_float32 x, vrpn_float32 y, vrpn_float32 yaw, 
                 vrpn_float32 sweepWidth, vrpn_float32 regionDiag) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 5 * sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_ZagTo:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, x);
    vrpn_buffer(&mptr, &mlen, y);
    vrpn_buffer(&mptr, &mlen, yaw);
    vrpn_buffer(&mptr, &mlen, sweepWidth);
    vrpn_buffer(&mptr, &mlen, regionDiag);
  }

  return msgbuf;
}
long nmm_Microscope::decode_ZagTo (const char ** buf,
         vrpn_float32 * x, vrpn_float32 * y, vrpn_float32 * yaw, vrpn_float32 * sweepWidth,
         vrpn_float32 * regionDiag) {
  CHECK(vrpn_unbuffer(buf, x));
  CHECK(vrpn_unbuffer(buf, y));
  CHECK(vrpn_unbuffer(buf, yaw));
  CHECK(vrpn_unbuffer(buf, sweepWidth));
  CHECK(vrpn_unbuffer(buf, regionDiag));

  return 0;
}





char * nmm_Microscope::encode_SetScanStyle (long * len,
                  vrpn_int32 style) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = sizeof(vrpn_int32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_SetScanStyle:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen,  style);
  }

  return msgbuf;
}
long nmm_Microscope::decode_SetScanStyle (const char ** buf,
         vrpn_int32 *  style) {
  CHECK(vrpn_unbuffer(buf,  style));

  return 0;
}



char * nmm_Microscope::encode_SetSlowScan (long * len,
                vrpn_int32 value) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = sizeof(vrpn_int32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_SetSlowScan:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen,  value);
  }

  return msgbuf;
}
long nmm_Microscope::decode_SetSlowScan (const char ** buf,
         vrpn_int32 *  value) {
  CHECK(vrpn_unbuffer(buf,  value));

  return 0;
}



char * nmm_Microscope::encode_SetStdDelay (long * len,
                 vrpn_int32 delay) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  // NANO BEGIN
  //fprintf(stderr, "nmm_Microscope::encode_SetStdDelay(): Entering...\n");
  // NANO END

  if (!len) return NULL;

  *len = sizeof(vrpn_int32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_SetStdDelay:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, delay);
  }

  // NANO BEGIN
//    fprintf(stderr, "nmm_Microscope::encode_SetStdDelay(): msg type = %ld\n", d_SetStdDelay_type);
//    fprintf(stderr, "nmm_Microscope::encode_SetStdDelay(): delay = %d\n", delay);
//    fprintf(stderr, "nmm_Microscope::encode_SetStdDelay(): Leaving\n");
  // NANO END

  return msgbuf;
}
long nmm_Microscope::decode_SetStdDelay (const char ** buf,
         vrpn_int32 * delay) {
  CHECK(vrpn_unbuffer(buf, delay));

  return 0;
}



char * nmm_Microscope::encode_SetStPtDelay (long * len,
                 vrpn_int32 delay) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  // NANO BEGIN
  //fprintf(stderr, "nmm_Microscope::encode_SetStPtDelay(): Entering...\n");
  // NANO END

  if (!len) return NULL;

  *len = sizeof(vrpn_int32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_SetStPtDelay:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, delay);
  }

  // NANO BEGIN
//    fprintf(stderr, "nmm_Microscope::encode_SetStPtDelay(): msg type = %ld\n", d_SetStPtDelay_type);
//    fprintf(stderr, "nmm_Microscope::encode_SetStPtDelay(): delay = %d\n", delay);
//    fprintf(stderr, "nmm_Microscope::encode_SetStPtDelay(): Leaving\n");
  // NANO END

  return msgbuf;
}

long nmm_Microscope::decode_SetStPtDelay (const char ** buf,
         vrpn_int32 * delay) {
  CHECK(vrpn_unbuffer(buf, delay));

  return 0;
}



char * nmm_Microscope::encode_SetRelax (long * len,
                 vrpn_int32 min, vrpn_int32 sep) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  // NANO BEGIN
  //fprintf(stderr, "nmm_Microscope::encode_SetRelax(): Entering...\n");
  // NANO END
  if (!len) return NULL;

  *len = 2 * sizeof(vrpn_int32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_SetRelax:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, min);
    vrpn_buffer(&mptr, &mlen, sep);
  }

  // NANO BEGIN
//    fprintf(stderr, "nmm_Microscope::encode_SetRelax(): msg type = %ld\n", d_SetRelax_type);
//    fprintf(stderr, "nmm_Microscope::encode_SetRelax(): min = %d\t sep = %d\n", min, sep);
//    fprintf(stderr, "nmm_Microscope::encode_SetRelax(): Leaving\n");
  // NANO END

  return msgbuf;
}
long nmm_Microscope::decode_SetRelax (const char ** buf,
         vrpn_int32 * min, vrpn_int32 * sep) {
  CHECK(vrpn_unbuffer(buf, min));
  CHECK(vrpn_unbuffer(buf, sep));

  return 0;
}




char * nmm_Microscope::encode_RecordResistance (long * len,
                 vrpn_int32 meter, struct timeval time, vrpn_float32 resistance,
                 vrpn_float32 v, vrpn_float32 r, vrpn_float32 f) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = sizeof(vrpn_int32) + sizeof (struct timeval) + 4 * sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_RecordResistance:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, meter);
    vrpn_buffer(&mptr, &mlen, time);
    vrpn_buffer(&mptr, &mlen, resistance);
    vrpn_buffer(&mptr, &mlen, v);
    vrpn_buffer(&mptr, &mlen, r);
    vrpn_buffer(&mptr, &mlen, f);
  }

  return msgbuf;
}
long nmm_Microscope::decode_RecordResistance (const char ** buf,
         vrpn_int32 * meter, struct timeval * time, vrpn_float32 * resistance,
         vrpn_float32 * v, vrpn_float32 * r, vrpn_float32 * f) {
  CHECK(vrpn_unbuffer(buf, meter));
  CHECK(vrpn_unbuffer(buf, time));
  CHECK(vrpn_unbuffer(buf, resistance));
  CHECK(vrpn_unbuffer(buf, v));
  CHECK(vrpn_unbuffer(buf, r));
  CHECK(vrpn_unbuffer(buf, f));

  return 0;
}


/* OBSOLETE */
char * nmm_Microscope::encode_SetStdDevParams (long * len,
                 vrpn_int32 samples, vrpn_float32 freq) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = sizeof(vrpn_int32) + sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_SetStdDevParams:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, samples);
    vrpn_buffer(&mptr, &mlen, freq);
  }

  return msgbuf;
}
long nmm_Microscope::decode_SetStdDevParams (const char ** buf,
         vrpn_int32 * samples, vrpn_float32 * freq) {
  CHECK(vrpn_unbuffer(buf, samples));
  CHECK(vrpn_unbuffer(buf, freq));

  return 0;
}



char * nmm_Microscope::encode_SetScanWindow (long * len,
                 vrpn_int32 minx, vrpn_int32 miny, vrpn_int32 maxx, vrpn_int32 maxy) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 4 * sizeof(vrpn_int32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_SetScanWindow:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, minx);
    vrpn_buffer(&mptr, &mlen, miny);
    vrpn_buffer(&mptr, &mlen, maxx);
    vrpn_buffer(&mptr, &mlen, maxy);
  }

  return msgbuf;
}
long nmm_Microscope::decode_SetScanWindow (const char ** buf,
         vrpn_int32 * minx, vrpn_int32 * miny, 
         vrpn_int32 * maxx, vrpn_int32 * maxy) {
  CHECK(vrpn_unbuffer(buf, minx));
  CHECK(vrpn_unbuffer(buf, miny));
  CHECK(vrpn_unbuffer(buf, maxx));
  CHECK(vrpn_unbuffer(buf, maxy));

  return 0;
}



char * nmm_Microscope::encode_SetGridSize (long * len,
                 vrpn_int32 x, vrpn_int32 y) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 2 * sizeof(vrpn_int32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_SetGridSize:  "
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
long nmm_Microscope::decode_SetGridSize (const char ** buf,
         vrpn_int32 * x, vrpn_int32 * y) {
  CHECK(vrpn_unbuffer(buf, x));
  CHECK(vrpn_unbuffer(buf, y));

  return 0;
}



char * nmm_Microscope::encode_SetOhmmeterSampleRate (long * len,
                 vrpn_int32 which, vrpn_int32 rate) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 2 * sizeof(vrpn_int32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_SetOhmmeterSampleRate:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, which);
    vrpn_buffer(&mptr, &mlen, rate);
  }

  return msgbuf;
}
long nmm_Microscope::decode_SetOhmmeterSampleRate (const char ** buf,
         vrpn_int32 * which, vrpn_int32 * rate) {
  CHECK(vrpn_unbuffer(buf, which));
  CHECK(vrpn_unbuffer(buf, rate));

  return 0;
}



char * nmm_Microscope::encode_EnableAmp (long * len,
                 vrpn_int32 which, vrpn_float32 offset, vrpn_float32 percentOffset,
		 vrpn_int32 gain) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 2 * sizeof(vrpn_int32) + 2 * sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_EnableAmp:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, which);
    vrpn_buffer(&mptr, &mlen, offset);
    vrpn_buffer(&mptr, &mlen, percentOffset);
    vrpn_buffer(&mptr, &mlen, gain);
  }

  return msgbuf;
}
long nmm_Microscope::decode_EnableAmp (const char ** buf,
         vrpn_int32 * which, vrpn_float32 * offset, vrpn_float32 * percentOffset,
	 vrpn_int32 * gain) {
  CHECK(vrpn_unbuffer(buf, which));
  CHECK(vrpn_unbuffer(buf, offset));
  CHECK(vrpn_unbuffer(buf, percentOffset));
  CHECK(vrpn_unbuffer(buf, gain));

  return 0;
}



char * nmm_Microscope::encode_DisableAmp (long * len,
                 vrpn_int32 which) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = sizeof(vrpn_int32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_DisableAmp:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, which);
  }

  return msgbuf;
}
long nmm_Microscope::decode_DisableAmp (const char ** buf,
         vrpn_int32 * which) {
  CHECK(vrpn_unbuffer(buf, which));

  return 0;
}



char * nmm_Microscope::encode_EnableVoltsource (long * len,
                 vrpn_int32 which, vrpn_float32 voltage) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = sizeof(vrpn_int32) + sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_EnableVoltsource:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, which);
    vrpn_buffer(&mptr, &mlen, voltage);
  }

  return msgbuf;
}
long nmm_Microscope::decode_EnableVoltsource (const char ** buf,
         vrpn_int32 * which, vrpn_float32 * voltage) {
  CHECK(vrpn_unbuffer(buf, which));
  CHECK(vrpn_unbuffer(buf, voltage));

  return 0;
}



char * nmm_Microscope::encode_DisableVoltsource (long * len,
                 vrpn_int32 which) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = sizeof(vrpn_int32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_DisableVoltsource:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, which);
  }

  return msgbuf;
}
long nmm_Microscope::decode_DisableVoltsource (const char ** buf,
         vrpn_int32 * which) {
  CHECK(vrpn_unbuffer(buf, which));

  return 0;
}



char * nmm_Microscope::encode_SetRateNM (long * len,
                 vrpn_float32 rate) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_SetRateNM:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, rate);
  }

  return msgbuf;
}
long nmm_Microscope::decode_SetRateNM (const char ** buf,
         vrpn_float32 * rate) {
  CHECK(vrpn_unbuffer(buf, rate));

  return 0;
}



char * nmm_Microscope::encode_SetMaxMove (long * len,
                 vrpn_float32 distance) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  // NANO BEGIN
  //fprintf(stderr, "nmm_Microscope::encode_SetMaxMove(): Entering...\n");
  // NANO END
  if (!len) return NULL;

  *len = sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_SetMaxMove:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, distance);
  }

  // NANO BEGIN
//    fprintf(stderr, "nmm_Microscope::encode_SetMaxMove(): msg type = %ld\n", d_SetMaxMove_type);
//    fprintf(stderr, "nmm_Microscope::encode_SetMaxMove(): distance = %f\n", distance);
//    fprintf(stderr, "nmm_Microscope::encode_SetMaxMove(): Leaving!\n");
  // NANO END
  return msgbuf;
}
long nmm_Microscope::decode_SetMaxMove (const char ** buf,
         vrpn_float32 * distance) {
  CHECK(vrpn_unbuffer(buf, distance));

  return 0;
}

char * nmm_Microscope::encode_SetModForce (int * len,
                 vrpn_float32 newforce, vrpn_float32 min, vrpn_float32 max) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = sizeof(vrpn_int32) + 3*sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_SetModForce:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    // There is an "enable" flag we put in automatically.
    vrpn_buffer(&mptr, &mlen, (vrpn_int32)1); 
    vrpn_buffer(&mptr, &mlen,min);
    vrpn_buffer(&mptr, &mlen,max);
    vrpn_buffer(&mptr, &mlen,newforce);
  }

  return msgbuf;
}
int nmm_Microscope::decode_SetModForce (const char ** buf,
         vrpn_float32 * newforce, vrpn_float32 * min, vrpn_float32 * max) {
  vrpn_int32 enable;
  CHECK(vrpn_unbuffer(buf, &enable));
  CHECK(vrpn_unbuffer(buf,min));
  CHECK(vrpn_unbuffer(buf,max));
  CHECK(vrpn_unbuffer(buf,newforce));
  return 0;
}



char * nmm_Microscope::encode_DrawSharpLine (long * len,
                 vrpn_float32 startx, vrpn_float32 starty, vrpn_float32 endx, vrpn_float32 endy,
                 vrpn_float32 stepSize) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 5 * sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_DrawSharpLine:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, startx);
    vrpn_buffer(&mptr, &mlen, starty);
    vrpn_buffer(&mptr, &mlen, endx);
    vrpn_buffer(&mptr, &mlen, endy);
    vrpn_buffer(&mptr, &mlen, stepSize);
  }

  return msgbuf;
}
long nmm_Microscope::decode_DrawSharpLine (const char ** buf,
         vrpn_float32 * startx, vrpn_float32 * starty, vrpn_float32 * endx, vrpn_float32 * endy,
         vrpn_float32 * stepSize) {
  CHECK(vrpn_unbuffer(buf, startx));
  CHECK(vrpn_unbuffer(buf, starty));
  CHECK(vrpn_unbuffer(buf, endx));
  CHECK(vrpn_unbuffer(buf, endy));
  CHECK(vrpn_unbuffer(buf, stepSize));

  return 0;
}



char * nmm_Microscope::encode_DrawSweepLine (long * len,
                 vrpn_float32 startx, vrpn_float32 starty, vrpn_float32 startYaw,
                 vrpn_float32 startSweepWidth, vrpn_float32 endx, vrpn_float32 endy,
                 vrpn_float32 endYaw, vrpn_float32 endSweepWidth, vrpn_float32 stepSize) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 9 * sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_DrawSweepLine:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, startx);
    vrpn_buffer(&mptr, &mlen, starty);
    vrpn_buffer(&mptr, &mlen, startYaw);
    vrpn_buffer(&mptr, &mlen, startSweepWidth);
    vrpn_buffer(&mptr, &mlen, endx);
    vrpn_buffer(&mptr, &mlen, endy);
    vrpn_buffer(&mptr, &mlen, endYaw);
    vrpn_buffer(&mptr, &mlen, endSweepWidth);
    vrpn_buffer(&mptr, &mlen, stepSize);
  }

  return msgbuf;
}
long nmm_Microscope::decode_DrawSweepLine (const char ** buf,
         vrpn_float32 * startx, vrpn_float32 * starty, vrpn_float32 * startYaw,
         vrpn_float32 * startSweepWidth, vrpn_float32 * endx, vrpn_float32 * endy,
         vrpn_float32 * endYaw, vrpn_float32 * endSweepWidth, vrpn_float32 * stepSize) {
  CHECK(vrpn_unbuffer(buf, startx));
  CHECK(vrpn_unbuffer(buf, starty));
  CHECK(vrpn_unbuffer(buf, startYaw));
  CHECK(vrpn_unbuffer(buf, startSweepWidth));
  CHECK(vrpn_unbuffer(buf, endx));
  CHECK(vrpn_unbuffer(buf, endy));
  CHECK(vrpn_unbuffer(buf, endYaw));
  CHECK(vrpn_unbuffer(buf, endSweepWidth));
  CHECK(vrpn_unbuffer(buf, stepSize));

  return 0;
}



char * nmm_Microscope::encode_DrawSweepArc (long * len,
                 vrpn_float32 x, vrpn_float32 y, vrpn_float32 startAngle, vrpn_float32 startSweepWidth,
                 vrpn_float32 endAngle, vrpn_float32 endSweepWidth, vrpn_float32 stepSize) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 7 * sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_DrawSweepArc:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, x);
    vrpn_buffer(&mptr, &mlen, y);
    vrpn_buffer(&mptr, &mlen, startAngle);
    vrpn_buffer(&mptr, &mlen, startSweepWidth);
    vrpn_buffer(&mptr, &mlen, endAngle);
    vrpn_buffer(&mptr, &mlen, endSweepWidth);
    vrpn_buffer(&mptr, &mlen, stepSize);
  }

  return msgbuf;
}
long nmm_Microscope::decode_DrawSweepArc (const char ** buf,
         vrpn_float32 * x, vrpn_float32 * y, vrpn_float32 * startAngle, vrpn_float32 * startSweepWidth,
         vrpn_float32 * endAngle, vrpn_float32 * endSweepWidth,
	 vrpn_float32 * stepSize) {
  CHECK(vrpn_unbuffer(buf, x));
  CHECK(vrpn_unbuffer(buf, y));
  CHECK(vrpn_unbuffer(buf, startAngle));
  CHECK(vrpn_unbuffer(buf, startSweepWidth));
  CHECK(vrpn_unbuffer(buf, endAngle));
  CHECK(vrpn_unbuffer(buf, endSweepWidth));
  CHECK(vrpn_unbuffer(buf, stepSize));

  return 0;
}



long nmm_Microscope::decode_GetNewPointDatasetHeader (const char ** buf,
         vrpn_int32 * numSets) {
  CHECK(vrpn_unbuffer(buf, numSets));

  return 0;
}

long nmm_Microscope::decode_GetNewPointDataset (const char ** buf,
         char * name, vrpn_int32 * numSamples) {
  CHECK(vrpn_unbuffer(buf, name, STM_NAME_LENGTH));
  CHECK(vrpn_unbuffer(buf, numSamples));

  return 0;
}



long nmm_Microscope::decode_GetNewScanDatasetHeader (const char ** buf,
         vrpn_int32 * numSets) {
  CHECK(vrpn_unbuffer(buf, numSets));

  return 0;
}

long nmm_Microscope::decode_GetNewScanDataset (const char ** buf,
         char * name) {
  CHECK(vrpn_unbuffer(buf, name, STM_NAME_LENGTH));

  return 0;
}

char * nmm_Microscope::encode_VoltsourceEnabled (long * len,
           vrpn_int32 which, vrpn_float32 voltage) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = sizeof(vrpn_int32) + sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_VoltsourceEnabled:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, which);
    vrpn_buffer(&mptr, &mlen, voltage);
  }

  return msgbuf;
}

long nmm_Microscope::decode_VoltsourceEnabled (const char ** buf,
         vrpn_int32 * which, vrpn_float32 * voltage) {
  CHECK(vrpn_unbuffer(buf, which));
  CHECK(vrpn_unbuffer(buf, voltage));

  return 0;
}

char * nmm_Microscope::encode_VoltsourceDisabled (long * len,
           vrpn_int32 which) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = sizeof(vrpn_int32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_VoltsourceDisabled:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, which);
  }

  return msgbuf;
}

long nmm_Microscope::decode_VoltsourceDisabled (const char ** buf,
         vrpn_int32 * which) {
  CHECK(vrpn_unbuffer(buf, which));

  return 0;
}

char * nmm_Microscope::encode_AmpEnabled (long * len,
           vrpn_int32 which, vrpn_float32 offset, vrpn_float32 percentOffset, vrpn_int32 gain) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 2 * sizeof(vrpn_int32) + 2 * sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_AmpEnabled:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, which);
    vrpn_buffer(&mptr, &mlen, offset);
    vrpn_buffer(&mptr, &mlen, percentOffset);
    vrpn_buffer(&mptr, &mlen, gain);
  }

  return msgbuf;
}

long nmm_Microscope::decode_AmpEnabled (const char ** buf,
         vrpn_int32 * which, vrpn_float32 * offset, vrpn_float32 * percentOffset, vrpn_int32 * gain) {
  CHECK(vrpn_unbuffer(buf, which));
  CHECK(vrpn_unbuffer(buf, offset));
  CHECK(vrpn_unbuffer(buf, percentOffset));
  CHECK(vrpn_unbuffer(buf, gain));

  return 0;
}

char * nmm_Microscope::encode_AmpDisabled (long * len,
           vrpn_int32 which) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = sizeof(vrpn_int32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_AmpDisabled:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, which);
  }

  return msgbuf;
}

long nmm_Microscope::decode_AmpDisabled (const char ** buf,
         vrpn_int32 * which) {
  CHECK(vrpn_unbuffer(buf, which));

  return 0;
}

char * nmm_Microscope::encode_StartingToRelax (long * len,
           vrpn_int32 sec, vrpn_int32 usec) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 2 * sizeof(vrpn_int32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_StartingToRelax:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, sec);
    vrpn_buffer(&mptr, &mlen, usec);
  }

  return msgbuf;
}

long nmm_Microscope::decode_StartingToRelax (const char ** buf,
         vrpn_int32 * sec, vrpn_int32 * usec) {
  CHECK(vrpn_unbuffer(buf, sec));
  CHECK(vrpn_unbuffer(buf, usec));

  return 0;
}


char * nmm_Microscope::encode_RelaxSet (long * len,
            vrpn_int32 min, vrpn_int32 sep) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 2 * sizeof(vrpn_int32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_RelaxSet:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, min);
    vrpn_buffer(&mptr, &mlen, sep);
  }

  return msgbuf;
}

long nmm_Microscope::decode_RelaxSet (const char ** buf,
         vrpn_int32 * min, vrpn_int32 * sep) {
  // NANO BEGIN
   //fprintf(stderr, "nmm_Microscope::decode_RelaxSet(): Entering...\n");
  // NANO END
  CHECK(vrpn_unbuffer(buf, min));
  CHECK(vrpn_unbuffer(buf, sep));

  // NANO BEGIN
  //fprintf(stderr, "nmm_Microscope::decode_RelaxSet(): msg type = %d\n", d_RelaxSet_type);
  //fprintf(stderr, "nmm_Microscope::decode_RelaxSet(): min = %lX\t sep = %lX\n", *min, *sep);
  //fprintf(stderr, "nmm_Microscope::decode_RelaxSet(): Leaving!");
  // NANO END
  return 0;
}


/* OBSOLETE */
char * nmm_Microscope::encode_StdDevParameters (long * len,
           vrpn_int32 samples, vrpn_float32 frequency) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = sizeof(vrpn_int32) + sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_StdDevParameters:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, samples);
    vrpn_buffer(&mptr, &mlen, frequency);


  }

  return msgbuf;


}

long nmm_Microscope::decode_StdDevParameters (const char ** buf,
         vrpn_int32 * samples, vrpn_float32 * frequency) {
  CHECK(vrpn_unbuffer(buf, samples));
  CHECK(vrpn_unbuffer(buf, frequency));

  return 0;
}


char * nmm_Microscope::encode_WindowLineData (long * len,
           vrpn_int32 x, vrpn_int32 y, 
           vrpn_int32 dx, vrpn_int32 dy, vrpn_int32 lineCount,
           vrpn_int32 fieldCount, vrpn_int32 sec, vrpn_int32 usec,
           vrpn_float32 ** data) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;
  long i, j;

  if (!len) return NULL;

  *len = 8 * sizeof(vrpn_int32) + lineCount * fieldCount * sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_WindowLineData:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, x);
    vrpn_buffer(&mptr, &mlen, y);
    vrpn_buffer(&mptr, &mlen, dx);
    vrpn_buffer(&mptr, &mlen, dy);
    vrpn_buffer(&mptr, &mlen, lineCount);
    vrpn_buffer(&mptr, &mlen, fieldCount);
    vrpn_buffer(&mptr, &mlen, sec);
    vrpn_buffer(&mptr, &mlen, usec);
    for (i = 0; i < lineCount; i++)
      for (j = 0; j < fieldCount; j++) {
        vrpn_buffer(&mptr, &mlen, data[j][i]);
      }
  }

  return msgbuf;
}

/*
   takes short data and buffers it as vrpn_float32 data
*/
char * nmm_Microscope::encode_WindowLineData (long * len,
           vrpn_int32 x, vrpn_int32 y, 
           vrpn_int32 dx, vrpn_int32 dy, vrpn_int32 lineCount,
           vrpn_int32 fieldCount, vrpn_int32 * offset, 
           vrpn_int32 sec, vrpn_int32 usec, 
	   unsigned short ** data) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;
  long i, j;
  vrpn_float32 val_hgt;

  if (!len) return NULL;

  *len = 8 * sizeof(vrpn_int32) + lineCount * fieldCount * sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_WindowLineData:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, x);
    vrpn_buffer(&mptr, &mlen, y);
    vrpn_buffer(&mptr, &mlen, dx);
    vrpn_buffer(&mptr, &mlen, dy);
    vrpn_buffer(&mptr, &mlen, lineCount);
    vrpn_buffer(&mptr, &mlen, fieldCount);
    vrpn_buffer(&mptr, &mlen, sec);
    vrpn_buffer(&mptr, &mlen, usec);
    for (i = 0; i < lineCount; i++)
      for (j = 0; j < fieldCount; j++) {
 	val_hgt = (vrpn_float32)(data[j][offset[j]+i]);
        vrpn_buffer(&mptr, &mlen, val_hgt);
      }
  }

  return msgbuf;
}


long nmm_Microscope::decode_WindowLineDataHeader (const char ** buf,
         vrpn_int32 * x, vrpn_int32 * y, vrpn_int32 * dx, vrpn_int32 * dy,
         vrpn_int32 * lineCount, vrpn_int32 * fieldCount, vrpn_int32 * sec, vrpn_int32 * usec) {
  CHECK(vrpn_unbuffer(buf, x));
  CHECK(vrpn_unbuffer(buf, y));
  CHECK(vrpn_unbuffer(buf, dx));
  CHECK(vrpn_unbuffer(buf, dy));
  CHECK(vrpn_unbuffer(buf, lineCount));
  CHECK(vrpn_unbuffer(buf, fieldCount));
  CHECK(vrpn_unbuffer(buf, sec));
  CHECK(vrpn_unbuffer(buf, usec));

  return 0;
}


long nmm_Microscope::decode_WindowLineDataField (const char ** buf,
         vrpn_int32 fieldCount, vrpn_float32 * data) {
  long i;

  for (i = 0; i < fieldCount; i++) {
      CHECK(vrpn_unbuffer(buf, &data[i]));
  }

  return 0;
}

char * nmm_Microscope::encode_WindowScanNM (long * len,
           vrpn_int32 x, vrpn_int32 y, vrpn_int32 sec, vrpn_int32 usec, vrpn_float32 height,
	   vrpn_float32 deviation) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 4 * sizeof(vrpn_int32) + 2 * sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_WindowScanNM:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, x);
    vrpn_buffer(&mptr, &mlen, y);
    vrpn_buffer(&mptr, &mlen, sec);
    vrpn_buffer(&mptr, &mlen, usec);
    vrpn_buffer(&mptr, &mlen, height);
    vrpn_buffer(&mptr, &mlen, deviation);
  }

  return msgbuf;
}

long nmm_Microscope::decode_WindowScanNM (const char ** buf,
         vrpn_int32 * x, vrpn_int32 * y, vrpn_int32 * sec, vrpn_int32 * usec,
         vrpn_float32 * value, vrpn_float32 * deviation) {
  CHECK(vrpn_unbuffer(buf, x));
  CHECK(vrpn_unbuffer(buf, y));
  CHECK(vrpn_unbuffer(buf, sec));
  CHECK(vrpn_unbuffer(buf, usec));
  CHECK(vrpn_unbuffer(buf, value));
  CHECK(vrpn_unbuffer(buf, deviation));

  return 0;
}

char * nmm_Microscope::encode_WindowBackscanNM (long * len,
           vrpn_int32 x, vrpn_int32 y, vrpn_int32 sec, vrpn_int32 usec, vrpn_float32 value, vrpn_float32 deviation) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 4 * sizeof(vrpn_int32) + 2 * sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_WindowBackscanNM:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, x);
    vrpn_buffer(&mptr, &mlen, y);
    vrpn_buffer(&mptr, &mlen, sec);
    vrpn_buffer(&mptr, &mlen, usec);
    vrpn_buffer(&mptr, &mlen, value);
    vrpn_buffer(&mptr, &mlen, deviation);
  }

  return msgbuf;
}

long nmm_Microscope::decode_WindowBackscanNM (const char ** buf,
         vrpn_int32 * x, vrpn_int32 * y, vrpn_int32 * sec, vrpn_int32 * usec,
         vrpn_float32 * value, vrpn_float32 * deviation) {
  CHECK(vrpn_unbuffer(buf, x));
  CHECK(vrpn_unbuffer(buf, y));
  CHECK(vrpn_unbuffer(buf, sec));
  CHECK(vrpn_unbuffer(buf, usec));
  CHECK(vrpn_unbuffer(buf, value));
  CHECK(vrpn_unbuffer(buf, deviation));

  return 0;
}

char * nmm_Microscope::encode_PointResultNM (long * len,
           vrpn_float32 x, vrpn_float32 y, vrpn_int32 sec, vrpn_int32 usec, vrpn_float32 height,
           vrpn_float32 deviation) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 2 * sizeof(vrpn_int32) + 4 * sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_PointResultNM:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, x);
    vrpn_buffer(&mptr, &mlen, y);
    vrpn_buffer(&mptr, &mlen, sec);
    vrpn_buffer(&mptr, &mlen, usec);
    vrpn_buffer(&mptr, &mlen, height);
    vrpn_buffer(&mptr, &mlen, deviation);
  }

  return msgbuf;
}

long nmm_Microscope::decode_PointResultNM (const char ** buf,
         vrpn_float32 * x, vrpn_float32 * y, vrpn_int32 * sec, vrpn_int32 * usec,
         vrpn_float32 * height, vrpn_float32 * deviation) {
  CHECK(vrpn_unbuffer(buf, x));
  CHECK(vrpn_unbuffer(buf, y));
  CHECK(vrpn_unbuffer(buf, sec));
  CHECK(vrpn_unbuffer(buf, usec));
  CHECK(vrpn_unbuffer(buf, height));
  CHECK(vrpn_unbuffer(buf, deviation));

  return 0;
}

char * nmm_Microscope::encode_ResultData (long * len,
           vrpn_float32 x, vrpn_float32 y, vrpn_int32 sec, vrpn_int32 usec, vrpn_int32 fieldCount,
	   vrpn_float32 * data) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;
  long i;

  if (!len) return NULL;

  *len = 3*sizeof(vrpn_int32) + 2*sizeof(vrpn_float32) + fieldCount * sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_ResultData:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, x);
    vrpn_buffer(&mptr, &mlen, y);
    vrpn_buffer(&mptr, &mlen, sec);
    vrpn_buffer(&mptr, &mlen, usec);
    vrpn_buffer(&mptr, &mlen, fieldCount);
    for (i = 0; i < fieldCount; i++)
      vrpn_buffer(&mptr, &mlen, data[i]);
  }

  return msgbuf;
}

long nmm_Microscope::decode_ResultData (const char ** buf,
         vrpn_float32 * x, vrpn_float32 * y, vrpn_int32 * sec, vrpn_int32 * usec, vrpn_int32 * fieldCount,
         vrpn_float32 * data) {
  long i;

  CHECK(vrpn_unbuffer(buf, x));
  CHECK(vrpn_unbuffer(buf, y));
  CHECK(vrpn_unbuffer(buf, sec));
  CHECK(vrpn_unbuffer(buf, usec));
  CHECK(vrpn_unbuffer(buf, fieldCount));

  for (i = 0; i < *fieldCount; i++)
    CHECK(vrpn_unbuffer(buf, &data[i]));

  return 0;
}

char * nmm_Microscope::encode_ResultNM (long * len,
           vrpn_float32 x, vrpn_float32 y, vrpn_int32 sec, vrpn_int32 usec, vrpn_float32 height,
           vrpn_float32 normX, vrpn_float32 normY, vrpn_float32 normZ) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 2 * sizeof(vrpn_int32) + 6 * sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_ResultNM:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, x);
    vrpn_buffer(&mptr, &mlen, y);
    vrpn_buffer(&mptr, &mlen, sec);
    vrpn_buffer(&mptr, &mlen, usec);
    vrpn_buffer(&mptr, &mlen, height);
    vrpn_buffer(&mptr, &mlen, normX);
    vrpn_buffer(&mptr, &mlen, normY);
    vrpn_buffer(&mptr, &mlen, normZ);
  }

  return msgbuf;
}

long nmm_Microscope::decode_ResultNM (const char ** buf,
         vrpn_float32 * x, vrpn_float32 * y, vrpn_int32 * sec, vrpn_int32 * usec, vrpn_float32 * height,
         vrpn_float32 * normX, vrpn_float32 * normY, vrpn_float32 * normZ) {
  CHECK(vrpn_unbuffer(buf, x));
  CHECK(vrpn_unbuffer(buf, y));
  CHECK(vrpn_unbuffer(buf, sec));
  CHECK(vrpn_unbuffer(buf, usec));
  CHECK(vrpn_unbuffer(buf, height));
  CHECK(vrpn_unbuffer(buf, normX));
  CHECK(vrpn_unbuffer(buf, normY));
  CHECK(vrpn_unbuffer(buf, normZ));

  return 0;
}

char * nmm_Microscope::encode_Scanning (long * len,
                  vrpn_int32 on_off) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = sizeof(vrpn_int32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_Scanning:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen,  on_off);
  }

  return msgbuf;
}
long nmm_Microscope::decode_Scanning (const char ** buf,
         vrpn_int32 *  on_off) {
  CHECK(vrpn_unbuffer(buf,  on_off));

  return 0;
}


char * nmm_Microscope::encode_ScanRange (long * len,
           vrpn_float32 minX, vrpn_float32 maxX, vrpn_float32 minY, vrpn_float32 maxY,
           vrpn_float32 minZ, vrpn_float32 maxZ) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 6 * sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_ScanRange:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, minX);
    vrpn_buffer(&mptr, &mlen, maxX);
    vrpn_buffer(&mptr, &mlen, minY);
    vrpn_buffer(&mptr, &mlen, maxY);
    vrpn_buffer(&mptr, &mlen, minZ);
    vrpn_buffer(&mptr, &mlen, maxZ);


  }

  return msgbuf;


}

long nmm_Microscope::decode_ScanRange (const char ** buf,
         vrpn_float32 * minX, vrpn_float32 * maxX, vrpn_float32 * minY,
         vrpn_float32 * maxY, vrpn_float32 * minZ, vrpn_float32 * maxZ) {
  CHECK(vrpn_unbuffer(buf, minX));
  CHECK(vrpn_unbuffer(buf, maxX));
  CHECK(vrpn_unbuffer(buf, minY));
  CHECK(vrpn_unbuffer(buf, maxY));
  CHECK(vrpn_unbuffer(buf, minZ));
  CHECK(vrpn_unbuffer(buf, maxZ));

  return 0;
}

char * nmm_Microscope::encode_ReportScanAngle (long * len,
					    vrpn_float32 angle) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_ReportScanAngle:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, angle);
  }

  return msgbuf;
}

long nmm_Microscope::decode_ReportScanAngle (const char ** buf,
        vrpn_float32 * angle) {
  CHECK(vrpn_unbuffer(buf, angle));

  return 0;
}


char * nmm_Microscope::encode_SetRegionC (long * len,
           vrpn_float32 minX, vrpn_float32 minY, vrpn_float32 maxX, vrpn_float32 maxY) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 4 * sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_SetRegionC:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, minX);
    vrpn_buffer(&mptr, &mlen, minY);
    vrpn_buffer(&mptr, &mlen, maxX);
    vrpn_buffer(&mptr, &mlen, maxY);
  }

  return msgbuf;
}

long nmm_Microscope::decode_SetRegionC (const char ** buf,
         vrpn_float32 * minX, vrpn_float32 * minY, vrpn_float32 * maxX, vrpn_float32 * maxY) {
  CHECK(vrpn_unbuffer(buf, minX));
  CHECK(vrpn_unbuffer(buf, minY));
  CHECK(vrpn_unbuffer(buf, maxX));
  CHECK(vrpn_unbuffer(buf, maxY));

  return 0;
}

char * nmm_Microscope::encode_ResistanceFailure (long * len,
           vrpn_int32 which) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = sizeof(vrpn_int32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_ResistanceFailure:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, which);
  }

  return msgbuf;
}

long nmm_Microscope::decode_ResistanceFailure (const char ** buf,
         vrpn_int32 * which) {
  CHECK(vrpn_unbuffer(buf, which));

  return 0;
}

char * nmm_Microscope::encode_Resistance (long * len,
           vrpn_int32 which, vrpn_int32 sec, vrpn_int32 usec, vrpn_float32 resistance) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 3 * sizeof(vrpn_int32) + sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_Resistance:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, which);
    vrpn_buffer(&mptr, &mlen, sec);
    vrpn_buffer(&mptr, &mlen, usec);
    vrpn_buffer(&mptr, &mlen, resistance);
  }

  return msgbuf;
}

long nmm_Microscope::decode_Resistance (const char ** buf,
         vrpn_int32 * which, vrpn_int32 * sec, vrpn_int32 * usec, vrpn_float32 * resistance) {
  CHECK(vrpn_unbuffer(buf, which));
  CHECK(vrpn_unbuffer(buf, sec));
  CHECK(vrpn_unbuffer(buf, usec));
  CHECK(vrpn_unbuffer(buf, resistance));

  return 0;
}

char * nmm_Microscope::encode_Resistance2(long * len,
           vrpn_int32 which, vrpn_int32 sec, vrpn_int32 usec, vrpn_float32 resistance,
           vrpn_float32 voltage, vrpn_float32 range, vrpn_float32 filter) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 3 * sizeof(vrpn_int32) + 4 * sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_Resistance2:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, which);
    vrpn_buffer(&mptr, &mlen, sec);
    vrpn_buffer(&mptr, &mlen, usec);
    vrpn_buffer(&mptr, &mlen, resistance);
    vrpn_buffer(&mptr, &mlen, voltage);
    vrpn_buffer(&mptr, &mlen, range);
    vrpn_buffer(&mptr, &mlen, filter);
  }

  return msgbuf;
}

long nmm_Microscope::decode_Resistance2(const char ** buf,
         vrpn_int32 * which, vrpn_int32 * sec, vrpn_int32 * usec, vrpn_float32 * resistance,
         vrpn_float32 * voltage, vrpn_float32 * range, vrpn_float32 * filter) {
  CHECK(vrpn_unbuffer(buf, which));
  CHECK(vrpn_unbuffer(buf, sec));
  CHECK(vrpn_unbuffer(buf, usec));
  CHECK(vrpn_unbuffer(buf, resistance));
  CHECK(vrpn_unbuffer(buf, voltage));
  CHECK(vrpn_unbuffer(buf, range));
  CHECK(vrpn_unbuffer(buf, filter));

  return 0;
}

char * nmm_Microscope::encode_ResistanceWithStatus(long * len,
           vrpn_int32 which, vrpn_int32 sec, vrpn_int32 usec, vrpn_float32 resistance,
           vrpn_float32 voltage, vrpn_float32 range, vrpn_float32 filter, vrpn_int32 status) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 4 * sizeof(vrpn_int32) + 4 * sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_ResistanceWithStatus:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, which);
    vrpn_buffer(&mptr, &mlen, sec);
    vrpn_buffer(&mptr, &mlen, usec);
    vrpn_buffer(&mptr, &mlen, resistance);
    vrpn_buffer(&mptr, &mlen, voltage);
    vrpn_buffer(&mptr, &mlen, range);
    vrpn_buffer(&mptr, &mlen, filter);
	vrpn_buffer(&mptr, &mlen, status);
  }

  return msgbuf;
}

long nmm_Microscope::decode_ResistanceWithStatus(const char ** buf,
         vrpn_int32 * which, vrpn_int32 * sec, vrpn_int32 * usec, vrpn_float32 * resistance,
         vrpn_float32 * voltage, vrpn_float32 * range, vrpn_float32 * filter, vrpn_int32 * status) {
  CHECK(vrpn_unbuffer(buf, which));
  CHECK(vrpn_unbuffer(buf, sec));
  CHECK(vrpn_unbuffer(buf, usec));
  CHECK(vrpn_unbuffer(buf, resistance));
  CHECK(vrpn_unbuffer(buf, voltage));
  CHECK(vrpn_unbuffer(buf, range));
  CHECK(vrpn_unbuffer(buf, filter));
  CHECK(vrpn_unbuffer(buf, status));

  return 0;
}

char * nmm_Microscope::encode_ReportSlowScan (long * len,
           vrpn_int32 isEnabled) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = sizeof(vrpn_int32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_ReportSlowScan:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, isEnabled);
  }

  return msgbuf;
}

long nmm_Microscope::decode_ReportSlowScan (const char ** buf,
         vrpn_int32 * isEnabled) {
  CHECK(vrpn_unbuffer(buf, isEnabled));

  return 0;
}

#if 0
char * nmm_Microscope::encode_ScanParameters (long * len,
           char * buffer) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;
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
    vrpn_buffer(&mptr, &mlen, , *len);  // TODO
  }

  return msgbuf;
}
#endif  // 0

long nmm_Microscope::decode_ScanParameters (const char ** buf,
         vrpn_int32 * length, char ** buffer) {
  long padding;

  CHECK(vrpn_unbuffer(buf, length));

  *buffer = new char [*length];
  if (!*buffer) {
    fprintf(stderr, "nmm_Microscope::decode_ScanParameters:  "
                    "Out of memory!\n");
    return -1;
  }

  CHECK(vrpn_unbuffer(buf, *buffer, *length));

  // align to a 4-byte boundary
  padding = 4 - (*length % 4);
  *buf += padding;

  return 0;
}

char * nmm_Microscope::encode_HelloMessage (long * len,
           char * magic, char * name, vrpn_int32 majorVersion, vrpn_int32 minorVersion) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = (4 + STM_NAME_LENGTH) * sizeof(char) + 2 * sizeof(vrpn_int32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_HelloMessage:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, magic, 4);
    vrpn_buffer(&mptr, &mlen, name, STM_NAME_LENGTH);
    vrpn_buffer(&mptr, &mlen, majorVersion);
    vrpn_buffer(&mptr, &mlen, minorVersion);
  }

  return msgbuf;
}

long nmm_Microscope::decode_HelloMessage (const char ** buf,
         char * magic, char * name, vrpn_int32 * majorVersion, vrpn_int32 * minorVersion) {
  CHECK(vrpn_unbuffer(buf, magic, 4));
  CHECK(vrpn_unbuffer(buf, name, STM_NAME_LENGTH));
  CHECK(vrpn_unbuffer(buf, majorVersion));
  CHECK(vrpn_unbuffer(buf, minorVersion));

  return 0;
}

char * nmm_Microscope::encode_ClientHello (long * len,
           char * magic, char * name, vrpn_int32 majorVersion, vrpn_int32 minorVersion) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = (4 + STM_NAME_LENGTH) * sizeof(char) + 2 * sizeof(vrpn_int32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_ClientHello:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, magic, 4);
    vrpn_buffer(&mptr, &mlen, name, STM_NAME_LENGTH);
    vrpn_buffer(&mptr, &mlen, majorVersion);
    vrpn_buffer(&mptr, &mlen, minorVersion);
  }

  return msgbuf;
}

long nmm_Microscope::decode_ClientHello (const char ** buf,
         char * magic, char * name, vrpn_int32 * majorVersion, vrpn_int32 * minorVersion) {
  CHECK(vrpn_unbuffer(buf, magic, 4));
  CHECK(vrpn_unbuffer(buf, name, STM_NAME_LENGTH));
  CHECK(vrpn_unbuffer(buf, majorVersion));
  CHECK(vrpn_unbuffer(buf, minorVersion));

  return 0;
}


long nmm_Microscope::decode_ScanDatasetHeader (const char ** buf,
         vrpn_int32 * count) {
  CHECK(vrpn_unbuffer(buf, count));
// NANO BEGIN
  //  fprintf(stderr, "nmm_Microscope::decode_ScanDatasetHeader(): numDatasets = %ld\n", *count);
// NANO END

  return 0;
}

long nmm_Microscope::decode_ScanDataset (const char ** buf,
         char * name, char * units, vrpn_float32 * offset,
                               vrpn_float32 * scale) {
  CHECK(vrpn_unbuffer(buf, name, STM_NAME_LENGTH));
  CHECK(vrpn_unbuffer(buf, units, STM_NAME_LENGTH));
  CHECK(vrpn_unbuffer(buf, offset));
  CHECK(vrpn_unbuffer(buf, scale));

  return 0;
}

long nmm_Microscope::decode_PointDatasetHeader (const char ** buf,
         vrpn_int32 * count) {
  CHECK(vrpn_unbuffer(buf, count));

  return 0;
}

long nmm_Microscope::decode_PointDataset (const char ** buf,
         char * name, char * units, vrpn_int32 * numSamples,
                               vrpn_float32 * offset, vrpn_float32 * scale) {
  CHECK(vrpn_unbuffer(buf, name, STM_NAME_LENGTH));
  CHECK(vrpn_unbuffer(buf, units, STM_NAME_LENGTH));
  CHECK(vrpn_unbuffer(buf, numSamples));
  CHECK(vrpn_unbuffer(buf, offset));
  CHECK(vrpn_unbuffer(buf, scale));

  return 0;
}

char * nmm_Microscope::encode_PidParameters (long * len,
           vrpn_float32 p, vrpn_float32 i, vrpn_float32 d) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 3 * sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_PidParameters:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, p);
    vrpn_buffer(&mptr, &mlen, i);
    vrpn_buffer(&mptr, &mlen, d);
  }

  return msgbuf;
}

long nmm_Microscope::decode_PidParameters (const char ** buf,
         vrpn_float32 * p, vrpn_float32 * i, vrpn_float32 * d) {
  CHECK(vrpn_unbuffer(buf, p));
  CHECK(vrpn_unbuffer(buf, i));
  CHECK(vrpn_unbuffer(buf, d));

  return 0;
}

char * nmm_Microscope::encode_ScanrateParameter (long * len,
           vrpn_float32 rate) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_ScanrateParameter:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, rate);
  }

  return msgbuf;
}

long nmm_Microscope::decode_ScanrateParameter (const char ** buf,
         vrpn_float32 * rate) {
  CHECK(vrpn_unbuffer(buf, rate));

  return 0;
}

char * nmm_Microscope::encode_ReportGridSize (long * len,
           vrpn_int32 x, vrpn_int32 y) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 2 * sizeof(vrpn_int32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_ReportGridSize:  "
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

long nmm_Microscope::decode_ReportGridSize (const char ** buf,
         vrpn_int32 * x, vrpn_int32 * y) {
  CHECK(vrpn_unbuffer(buf, x));
  CHECK(vrpn_unbuffer(buf, y));

  return 0;
}

char * nmm_Microscope::encode_ServerPacketTimestamp (long * len,
           vrpn_int32 sec, vrpn_int32 usec) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 2 * sizeof(vrpn_int32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_ServerPacketTimestamp:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, sec);
    vrpn_buffer(&mptr, &mlen, usec);
  }

  return msgbuf;
}

long nmm_Microscope::decode_ServerPacketTimestamp (const char ** buf,
         vrpn_int32 * sec, vrpn_int32 * usec) {
  CHECK(vrpn_unbuffer(buf, sec));
  CHECK(vrpn_unbuffer(buf, usec));

  return 0;
}

char * nmm_Microscope::encode_TopoFileHeader (long * len,
           char * buf, vrpn_int32 size ) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = sizeof(vrpn_int32)+size*sizeof(char);  // HACK XXX Tiger
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_TopoFileHeader:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, size);  // HACK XXX Tiger
    vrpn_buffer(&mptr, &mlen, buf, (vrpn_int32)size);  // HACK XXX Tiger
  }

  return msgbuf;
}

long nmm_Microscope::decode_TopoFileHeader (const char ** buf,
         vrpn_int32 * length, char ** header) {
  CHECK(vrpn_unbuffer(buf, length));

  *header = new char [*length];
  if (!*header) {
    fprintf(stderr, "nmm_Microscope::decode_TopoFileHeader:  "
                    "Out of memory!\n");
    return -1;
  }

  CHECK(vrpn_unbuffer(buf, *header, *length));

  return 0;
}


char * nmm_Microscope::encode_ForceCurveData (long * len, vrpn_float32 x, vrpn_float32 y, 
    vrpn_int32 num_points, vrpn_int32 num_halfcycles, vrpn_int32 sec, vrpn_int32 usec, 
    vrpn_float32 *z, vrpn_float32 **data){

  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;
  long i, j;

  if (!len) return NULL;

  *len = 2*sizeof(vrpn_int32) + 2*sizeof(vrpn_float32) + 
		num_points*(num_halfcycles+1)*sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_ForceCurveData:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, x);
    vrpn_buffer(&mptr, &mlen, y);
    vrpn_buffer(&mptr, &mlen, num_points);
    vrpn_buffer(&mptr, &mlen, num_halfcycles);
    vrpn_buffer(&mptr, &mlen, sec);
    vrpn_buffer(&mptr, &mlen, usec);
    for (i = 0; i < num_points; i++){
      vrpn_buffer(&mptr, &mlen, z[i]);
      for (j = 0; j < num_halfcycles; j++){
        vrpn_buffer(&mptr, &mlen, data[j][i]);
      }
    }
  }

  return msgbuf;
  
}

long nmm_Microscope::decode_ForceCurveDataHeader (const char ** buf,
	vrpn_float32 *x, vrpn_float32 *y, vrpn_int32 *num_points, vrpn_int32 *num_halfcycles, 
	vrpn_int32 *sec, vrpn_int32 *usec){
  CHECK(vrpn_unbuffer(buf, x));
  CHECK(vrpn_unbuffer(buf, y));
  CHECK(vrpn_unbuffer(buf, num_points));
  CHECK(vrpn_unbuffer(buf, num_halfcycles));
  CHECK(vrpn_unbuffer(buf, sec));
  CHECK(vrpn_unbuffer(buf, usec));

  return 0;
}

long nmm_Microscope::decode_ForceCurveDataSingleLevel (const char ** buf,
	vrpn_int32 num_halfcycles, vrpn_float32 *z, vrpn_float32 * data){
  long i;

  CHECK(vrpn_unbuffer(buf, z));
  for (i = 0; i < num_halfcycles; i++)
    CHECK(vrpn_unbuffer(buf, &(data[i])));

  return 0;
}


// messages for Michele Clark's experiments
char * nmm_Microscope::encode_RecvTimestamp (long * len,
           struct timeval t) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

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
    vrpn_buffer(&mptr, &mlen, t);
  }

  return msgbuf;
}

long nmm_Microscope::decode_RecvTimestamp (const char ** buf,
         struct timeval * time) {
  CHECK(vrpn_unbuffer(buf, time));

  return 0;
}

char * nmm_Microscope::encode_FakeSendTimestamp (long * len,
                                               struct timeval t) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

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
    vrpn_buffer(&mptr, &mlen, t);
  }

  return msgbuf;
}

long nmm_Microscope::decode_FakeSendTimestamp (const char ** buf,
         struct timeval * time) {
  CHECK(vrpn_unbuffer(buf, time));

  return 0;
}

char * nmm_Microscope::encode_UdpSeqNum (long * len,
           vrpn_int32 sn) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = sizeof(vrpn_int32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_UdpSeqNum:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, sn);
  }

  return msgbuf;
}

long nmm_Microscope::decode_UdpSeqNum (const char ** buf,
         vrpn_int32 * number) {
  CHECK(vrpn_unbuffer(buf, number));

  return 0;
}




char * nmm_Microscope::encode_EnterTappingMode (long * len,
           vrpn_float32 p, vrpn_float32 i, vrpn_float32 d, vrpn_float32 setpoint, vrpn_float32 amplitude) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 5 * sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_EnterTappingMode:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, p);
    vrpn_buffer(&mptr, &mlen, i);
    vrpn_buffer(&mptr, &mlen, d);
    vrpn_buffer(&mptr, &mlen, setpoint);
    vrpn_buffer(&mptr, &mlen, amplitude);
  }

  return msgbuf;
}

long nmm_Microscope::decode_EnterTappingMode (const char ** buf,
         vrpn_float32 * p, vrpn_float32 * i, vrpn_float32 * d, vrpn_float32 * setpoint,
         vrpn_float32 * amplitude) {
  CHECK(vrpn_unbuffer(buf, p));
  CHECK(vrpn_unbuffer(buf, i));
  CHECK(vrpn_unbuffer(buf, d));
  CHECK(vrpn_unbuffer(buf, setpoint));
  CHECK(vrpn_unbuffer(buf, amplitude));

  return 0;
}

char * nmm_Microscope::encode_EnterOscillatingMode (long * len,
           vrpn_float32 p, vrpn_float32 i, vrpn_float32 d, 
           vrpn_float32 setpoint, vrpn_float32 amplitude,
           vrpn_float32 frequency, vrpn_int32 input_gain,
           vrpn_bool ampl_or_phase, vrpn_int32 drive_attenuation,
           vrpn_float32 phase) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 7 * sizeof(vrpn_float32) + 2*sizeof(vrpn_int32) + sizeof(vrpn_bool);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_EnterOscillatingMode:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
      // NOTE change in order. on SGI, 4 byte types must be aligned for
      // unbuffer to work (i.e. not give a bus error). So the two byte
      // vrpn_bool must come in pairs, or must come last!
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, p);
    vrpn_buffer(&mptr, &mlen, i);
    vrpn_buffer(&mptr, &mlen, d);
    vrpn_buffer(&mptr, &mlen, setpoint);
    vrpn_buffer(&mptr, &mlen, amplitude);
    vrpn_buffer(&mptr, &mlen, frequency);
    vrpn_buffer(&mptr, &mlen, input_gain);
    vrpn_buffer(&mptr, &mlen, drive_attenuation);
    vrpn_buffer(&mptr, &mlen, phase);
    vrpn_buffer(&mptr, &mlen, ampl_or_phase);
  }

  return msgbuf;
}

long nmm_Microscope::decode_EnterOscillatingMode (const char ** buf,
           vrpn_float32 * p, vrpn_float32 * i, vrpn_float32 * d, 
           vrpn_float32 * setpoint, vrpn_float32 * amplitude,
           vrpn_float32 * frequency, vrpn_int32 * input_gain,
           vrpn_bool * ampl_or_phase, vrpn_int32 * drive_attenuation,
           vrpn_float32 * phase) {
      // NOTE change in order. on SGI, 4 byte types must be aligned for
      // unbuffer to work (i.e. not give a bus error). So the two byte
      // vrpn_bool must come in pairs, or must come last!
  CHECK(vrpn_unbuffer(buf, p));
  CHECK(vrpn_unbuffer(buf, i));
  CHECK(vrpn_unbuffer(buf, d));
  CHECK(vrpn_unbuffer(buf, setpoint));
  CHECK(vrpn_unbuffer(buf, amplitude));
  CHECK(vrpn_unbuffer(buf, frequency));
  CHECK(vrpn_unbuffer(buf, input_gain));
  CHECK(vrpn_unbuffer(buf, drive_attenuation));
  CHECK(vrpn_unbuffer(buf, phase));
  CHECK(vrpn_unbuffer(buf, ampl_or_phase));

  return 0;
}



char * nmm_Microscope::encode_EnterContactMode (long * len,
           vrpn_float32 p, vrpn_float32 i, vrpn_float32 d, vrpn_float32 setpoint) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 4 * sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_EnterContactMode:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, p);
    vrpn_buffer(&mptr, &mlen, i);
    vrpn_buffer(&mptr, &mlen, d);
    vrpn_buffer(&mptr, &mlen, setpoint);
  }

  return msgbuf;
}

long nmm_Microscope::decode_EnterContactMode (const char ** buf,
         vrpn_float32 * p, vrpn_float32 * i, vrpn_float32 * d, vrpn_float32 * setpoint) {
  CHECK(vrpn_unbuffer(buf, p));
  CHECK(vrpn_unbuffer(buf, i));
  CHECK(vrpn_unbuffer(buf, d));
  CHECK(vrpn_unbuffer(buf, setpoint));

  return 0;
}

char * nmm_Microscope::encode_EnterDirectZControl (long * len,
           vrpn_float32 max_z_step, vrpn_float32 max_xy_step, vrpn_float32 min_setpoint, 
		 vrpn_float32 max_setpoint, vrpn_float32 max_lateral_force) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 5 * sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_EnterDirectZControl:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, max_z_step);
    vrpn_buffer(&mptr, &mlen, max_xy_step);
    vrpn_buffer(&mptr, &mlen, min_setpoint);
    vrpn_buffer(&mptr, &mlen, max_setpoint);
    vrpn_buffer(&mptr, &mlen, max_lateral_force);
  }

  return msgbuf;
}
long nmm_Microscope::decode_EnterDirectZControl (const char ** buf,
           vrpn_float32 * max_z_step, vrpn_float32 * max_xy_step, vrpn_float32 * min_setpoint, 
		 vrpn_float32 * max_setpoint, vrpn_float32 * max_lateral_force) {
  CHECK(vrpn_unbuffer(buf, max_z_step));
  CHECK(vrpn_unbuffer(buf, max_xy_step));
  CHECK(vrpn_unbuffer(buf, min_setpoint));
  CHECK(vrpn_unbuffer(buf, max_setpoint));
  CHECK(vrpn_unbuffer(buf, max_lateral_force));

  return 0;
}



char * nmm_Microscope::encode_EnterSewingStyle (long * len,
           vrpn_float32 setpoint, vrpn_float32 bottomDelay, vrpn_float32 topDelay,
           vrpn_float32 pullBackDistance, vrpn_float32 distanceBetweenPunches,
           vrpn_float32 speed, vrpn_float32 limitOfDescent) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 7 * sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_EnterSewingStyle:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, setpoint);
    vrpn_buffer(&mptr, &mlen, bottomDelay);
    vrpn_buffer(&mptr, &mlen, topDelay);
    vrpn_buffer(&mptr, &mlen, pullBackDistance);
    vrpn_buffer(&mptr, &mlen, distanceBetweenPunches);
    vrpn_buffer(&mptr, &mlen, speed);
    vrpn_buffer(&mptr, &mlen, limitOfDescent);
  }

  return msgbuf;
}

long nmm_Microscope::decode_EnterSewingStyle (const char ** buf,
         vrpn_float32 * setpoint, vrpn_float32 * bottomDelay, vrpn_float32 * topDelay,
         vrpn_float32 * pullBackDistance, vrpn_float32 * distanceBetweenPunches,
         vrpn_float32 * speed, vrpn_float32 * limitOfDescent) {
  CHECK(vrpn_unbuffer(buf, setpoint));
  CHECK(vrpn_unbuffer(buf, bottomDelay));
  CHECK(vrpn_unbuffer(buf, topDelay));
  CHECK(vrpn_unbuffer(buf, pullBackDistance));
  CHECK(vrpn_unbuffer(buf, distanceBetweenPunches));
  CHECK(vrpn_unbuffer(buf, speed));
  CHECK(vrpn_unbuffer(buf, limitOfDescent));

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

char * nmm_Microscope::encode_EnterSpectroscopyMode (long * len, vrpn_float32 setpoint,
	vrpn_float32 startDelay, vrpn_float32 zStart, vrpn_float32 zEnd, vrpn_float32 zPullback,
	vrpn_float32 forceLimit, vrpn_float32 distBetweenFC, vrpn_int32 numPoints, 
	vrpn_int32 numHalfcycles, vrpn_float32 sampleSpeed, vrpn_float32 pullbackSpeed,
	vrpn_float32 startSpeed, vrpn_float32 feedbackSpeed, vrpn_int32 avgNum,
	vrpn_float32 sampleDelay, vrpn_float32 pullbackDelay, vrpn_float32 feedbackDelay) {

  char *msgbuf = NULL;
  char *mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 17 * sizeof(vrpn_float32);
  msgbuf = new char[*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_EnterSpectroscopyMode: "
		    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, setpoint);
    vrpn_buffer(&mptr, &mlen, startDelay);
    vrpn_buffer(&mptr, &mlen, zStart);
    vrpn_buffer(&mptr, &mlen, zEnd);
    vrpn_buffer(&mptr, &mlen, zPullback);
    vrpn_buffer(&mptr, &mlen, forceLimit);
    vrpn_buffer(&mptr, &mlen, distBetweenFC);
    vrpn_buffer(&mptr, &mlen, numPoints);
    vrpn_buffer(&mptr, &mlen, numHalfcycles);
    vrpn_buffer(&mptr, &mlen, sampleSpeed);
    vrpn_buffer(&mptr, &mlen, pullbackSpeed);
    vrpn_buffer(&mptr, &mlen, startSpeed);
    vrpn_buffer(&mptr, &mlen, feedbackSpeed);
    vrpn_buffer(&mptr, &mlen, avgNum);
    vrpn_buffer(&mptr, &mlen, sampleDelay);
    vrpn_buffer(&mptr, &mlen, pullbackDelay);
    vrpn_buffer(&mptr, &mlen, feedbackDelay);
  }
  return msgbuf;
}

char * nmm_Microscope::encode_FeelTo (long * len,
               vrpn_float32 x, vrpn_float32 y,
               vrpn_int32 numx, vrpn_int32 numy,
               vrpn_float32 dx, vrpn_float32 dy,
               vrpn_float32 orientation) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 5 * sizeof(vrpn_float32) + 2 * sizeof(vrpn_int32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_FeelTo:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, x);
    vrpn_buffer(&mptr, &mlen, y);
    vrpn_buffer(&mptr, &mlen, numx);
    vrpn_buffer(&mptr, &mlen, numy);
    vrpn_buffer(&mptr, &mlen, dx);
    vrpn_buffer(&mptr, &mlen, dy);
    vrpn_buffer(&mptr, &mlen, orientation);
  }

  return msgbuf;
}
  
long nmm_Microscope::decode_FeelTo (const char ** buf,
             vrpn_float32 * x, vrpn_float32 * y,
             vrpn_int32 * numx, vrpn_int32 * numy,
             vrpn_float32 * dx, vrpn_float32 * dy,
             vrpn_float32 * orientation) {
  CHECK(vrpn_unbuffer(buf, x));
  CHECK(vrpn_unbuffer(buf, y));
  CHECK(vrpn_unbuffer(buf, numx));
  CHECK(vrpn_unbuffer(buf, numy));
  CHECK(vrpn_unbuffer(buf, dx));
  CHECK(vrpn_unbuffer(buf, dy));
  CHECK(vrpn_unbuffer(buf, orientation));

  return 0;
}




long nmm_Microscope::decode_EnterSpectroscopyMode (const char ** buf,
         vrpn_float32 * setpoint, vrpn_float32 * startDelay, vrpn_float32 * zStart, vrpn_float32 * zEnd,
         vrpn_float32 * zPullback, vrpn_float32 * forceLimit,
         vrpn_float32 * distBetweenFC, vrpn_int32 * numPoints, vrpn_int32 * numHalfcycles,
	 vrpn_float32 * sampleSpeed, vrpn_float32 * pullbackSpeed, vrpn_float32 * startSpeed,
	 vrpn_float32 * feedbackSpeed, vrpn_int32 *avgNum, 
	 vrpn_float32 * sampleDelay, vrpn_float32 * pullbackDelay, vrpn_float32 * feedbackDelay) {
  CHECK(vrpn_unbuffer(buf, setpoint));
  CHECK(vrpn_unbuffer(buf, startDelay));
  CHECK(vrpn_unbuffer(buf, zStart));
  CHECK(vrpn_unbuffer(buf, zEnd));
  CHECK(vrpn_unbuffer(buf, zPullback));
  CHECK(vrpn_unbuffer(buf, forceLimit));
  CHECK(vrpn_unbuffer(buf, distBetweenFC));
  CHECK(vrpn_unbuffer(buf, numPoints));
  CHECK(vrpn_unbuffer(buf, numHalfcycles));
  CHECK(vrpn_unbuffer(buf, sampleSpeed));
  CHECK(vrpn_unbuffer(buf, pullbackSpeed));
  CHECK(vrpn_unbuffer(buf, startSpeed));
  CHECK(vrpn_unbuffer(buf, feedbackSpeed));
  CHECK(vrpn_unbuffer(buf, avgNum));
  CHECK(vrpn_unbuffer(buf, sampleDelay));
  CHECK(vrpn_unbuffer(buf, pullbackDelay));
  CHECK(vrpn_unbuffer(buf, feedbackDelay));

  return 0;
}

char * nmm_Microscope::encode_InTappingMode (long * len,
           vrpn_float32 p, vrpn_float32 i, vrpn_float32 d, vrpn_float32 setpoint,
	   vrpn_float32 amplitude) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 5 * sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_InTappingMode:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, p);
    vrpn_buffer(&mptr, &mlen, i);
    vrpn_buffer(&mptr, &mlen, d);
    vrpn_buffer(&mptr, &mlen, setpoint);
    vrpn_buffer(&mptr, &mlen, amplitude);
  }

  return msgbuf;
}

long nmm_Microscope::decode_InTappingMode (const char ** buf,
         vrpn_float32 * p, vrpn_float32 * i, vrpn_float32 * d, vrpn_float32 * setpoint,
         vrpn_float32 * amplitude) {
  CHECK(vrpn_unbuffer(buf, p));
  CHECK(vrpn_unbuffer(buf, i));
  CHECK(vrpn_unbuffer(buf, d));
  CHECK(vrpn_unbuffer(buf, setpoint));
  CHECK(vrpn_unbuffer(buf, amplitude));

  return 0;
}


char * nmm_Microscope::encode_InOscillatingMode (long * len,
           vrpn_float32 p, vrpn_float32 i, vrpn_float32 d, 
           vrpn_float32 setpoint, vrpn_float32 amplitude,
           vrpn_float32 frequency, vrpn_int32 input_gain,
           vrpn_bool ampl_or_phase, vrpn_int32 drive_attenuation,
           vrpn_float32 phase) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 7 * sizeof(vrpn_float32) + 2*sizeof(vrpn_int32) + sizeof(vrpn_bool);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_InOscillatingMode:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
      // NOTE change in order. on SGI, 4 byte types must be aligned for
      // unbuffer to work (i.e. not give a bus error). So the two byte
      // vrpn_bool must come in pairs, or must come last!
    vrpn_buffer(&mptr, &mlen, p);
    vrpn_buffer(&mptr, &mlen, i);
    vrpn_buffer(&mptr, &mlen, d);
    vrpn_buffer(&mptr, &mlen, setpoint);
    vrpn_buffer(&mptr, &mlen, amplitude);
    vrpn_buffer(&mptr, &mlen, frequency);
    vrpn_buffer(&mptr, &mlen, input_gain);
    vrpn_buffer(&mptr, &mlen, drive_attenuation);
    vrpn_buffer(&mptr, &mlen, phase);
    vrpn_buffer(&mptr, &mlen, ampl_or_phase);
  }

  return msgbuf;
}

long nmm_Microscope::decode_InOscillatingMode (const char ** buf,
           vrpn_float32 * p, vrpn_float32 * i, vrpn_float32 * d, 
           vrpn_float32 * setpoint, vrpn_float32 * amplitude,
           vrpn_float32 * frequency, vrpn_int32 * input_gain,
           vrpn_bool * ampl_or_phase, vrpn_int32 * drive_attenuation,
           vrpn_float32 * phase) {
      // NOTE change in order. on SGI, 4 byte types must be aligned for
      // unbuffer to work (i.e. not give a bus error). So the two byte
      // vrpn_bool must come in pairs, or must come last!
  CHECK(vrpn_unbuffer(buf, p));
  CHECK(vrpn_unbuffer(buf, i));
  CHECK(vrpn_unbuffer(buf, d));
  CHECK(vrpn_unbuffer(buf, setpoint));
  CHECK(vrpn_unbuffer(buf, amplitude));
  CHECK(vrpn_unbuffer(buf, frequency));
  CHECK(vrpn_unbuffer(buf, input_gain));
  CHECK(vrpn_unbuffer(buf, drive_attenuation));
  CHECK(vrpn_unbuffer(buf, phase));
  CHECK(vrpn_unbuffer(buf, ampl_or_phase));

  return 0;
}

char * nmm_Microscope::encode_InContactMode (long * len,
           vrpn_float32 p, vrpn_float32 i, vrpn_float32 d, vrpn_float32 setpoint) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 4 * sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_InContactMode:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, p);
    vrpn_buffer(&mptr, &mlen, i);
    vrpn_buffer(&mptr, &mlen, d);
    vrpn_buffer(&mptr, &mlen, setpoint);
  }

  return msgbuf;
}

long nmm_Microscope::decode_InContactMode (const char ** buf,
         vrpn_float32 * p, vrpn_float32 * i, vrpn_float32 * d, vrpn_float32 * setpoint) {
  CHECK(vrpn_unbuffer(buf, p));
  CHECK(vrpn_unbuffer(buf, i));
  CHECK(vrpn_unbuffer(buf, d));
  CHECK(vrpn_unbuffer(buf, setpoint));

  return 0;
}

char * nmm_Microscope::encode_InDirectZControl (long * len,
				vrpn_float32 max_z_step, vrpn_float32 max_xy_step, 
				vrpn_float32 min_setpoint, vrpn_float32 max_setpoint, 
				vrpn_float32 max_lateral_force,
				vrpn_float32 freespace_norm_force,
				vrpn_float32 freespace_lat_force) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 7 * sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_InDirectZControl:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, max_z_step);
    vrpn_buffer(&mptr, &mlen, max_xy_step);
    vrpn_buffer(&mptr, &mlen, min_setpoint);
    vrpn_buffer(&mptr, &mlen, max_setpoint);
    vrpn_buffer(&mptr, &mlen, max_lateral_force);
    vrpn_buffer(&mptr, &mlen, freespace_norm_force);
    vrpn_buffer(&mptr, &mlen, freespace_lat_force);
  }

  return msgbuf;
}

long nmm_Microscope::decode_InDirectZControl (const char ** buf,
    vrpn_float32 *max_z_step, vrpn_float32 *max_xy_step, vrpn_float32 *min_setpoint, 
    vrpn_float32 *max_setpoint, vrpn_float32 *max_lateral_force, 
    vrpn_float32 *freespace_norm_force, vrpn_float32 *freespace_lat_force) {
  CHECK(vrpn_unbuffer(buf, max_z_step));
  CHECK(vrpn_unbuffer(buf, max_xy_step));
  CHECK(vrpn_unbuffer(buf, min_setpoint));
  CHECK(vrpn_unbuffer(buf, max_setpoint));
  CHECK(vrpn_unbuffer(buf, max_lateral_force));
  CHECK(vrpn_unbuffer(buf, freespace_norm_force));
  CHECK(vrpn_unbuffer(buf, freespace_lat_force));

  return 0;
}

char * nmm_Microscope::encode_InSewingStyle (long * len,
           vrpn_float32 setpoint, vrpn_float32 bottomDelay, vrpn_float32 topDelay,
           vrpn_float32 pullBackDistance, vrpn_float32 distanceBetweenPunches,
           vrpn_float32 speed, vrpn_float32 limitOfDescent) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 7 * sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_InSewingStyle:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, setpoint);
    vrpn_buffer(&mptr, &mlen, bottomDelay);
    vrpn_buffer(&mptr, &mlen, topDelay);
    vrpn_buffer(&mptr, &mlen, pullBackDistance);
    vrpn_buffer(&mptr, &mlen, distanceBetweenPunches);
    vrpn_buffer(&mptr, &mlen, speed);
    vrpn_buffer(&mptr, &mlen, limitOfDescent);
  }

  return msgbuf;
}

long nmm_Microscope::decode_InSewingStyle (const char ** buf,
         vrpn_float32 * setpoint, vrpn_float32 * bottomDelay, vrpn_float32 * topDelay,
         vrpn_float32 * pullBackDistance, vrpn_float32 * distanceBetweenPunches,
         vrpn_float32 * speed, vrpn_float32 * limitOfDescent) {
  CHECK(vrpn_unbuffer(buf, setpoint));
  CHECK(vrpn_unbuffer(buf, bottomDelay));
  CHECK(vrpn_unbuffer(buf, topDelay));
  CHECK(vrpn_unbuffer(buf, pullBackDistance));
  CHECK(vrpn_unbuffer(buf, distanceBetweenPunches));
  CHECK(vrpn_unbuffer(buf, speed));
  CHECK(vrpn_unbuffer(buf, limitOfDescent));

  return 0;
}

char * nmm_Microscope::encode_InSpectroscopyMode (long * len, vrpn_float32 setpoint,
	vrpn_float32 startDelay,
        vrpn_float32 zStart, vrpn_float32 zEnd, vrpn_float32 zPullback,vrpn_float32 forceLimit,
        vrpn_float32 distBetweenFC, vrpn_int32 numPoints, vrpn_int32 numHalfcycles,
	vrpn_float32 sampleSpeed, vrpn_float32 pullbackSpeed, vrpn_float32 startSpeed,
	vrpn_float32 feedbackSpeed, vrpn_int32 avgNum, 
	vrpn_float32 sampleDelay, vrpn_float32 pullbackDelay, vrpn_float32 feedbackDelay) {

  char *msgbuf = NULL;
  char *mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 17 * sizeof(vrpn_float32);
  msgbuf = new char[*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_InSpectroscopyMode: "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, setpoint);
    vrpn_buffer(&mptr, &mlen, startDelay);
    vrpn_buffer(&mptr, &mlen, zStart);
    vrpn_buffer(&mptr, &mlen, zEnd);
    vrpn_buffer(&mptr, &mlen, zPullback);
    vrpn_buffer(&mptr, &mlen, forceLimit);
    vrpn_buffer(&mptr, &mlen, distBetweenFC);
    vrpn_buffer(&mptr, &mlen, numPoints);
    vrpn_buffer(&mptr, &mlen, numHalfcycles);
    vrpn_buffer(&mptr, &mlen, sampleSpeed);
    vrpn_buffer(&mptr, &mlen, pullbackSpeed);
    vrpn_buffer(&mptr, &mlen, startSpeed);
    vrpn_buffer(&mptr, &mlen, feedbackSpeed);
    vrpn_buffer(&mptr, &mlen, avgNum);
    vrpn_buffer(&mptr, &mlen, sampleDelay);
    vrpn_buffer(&mptr, &mlen, pullbackDelay);
    vrpn_buffer(&mptr, &mlen, feedbackDelay);
  }
  return msgbuf;
}

long nmm_Microscope::decode_InSpectroscopyMode (const char ** buf,
         vrpn_float32 *setpoint, vrpn_float32 * startDelay, vrpn_float32 * zStart, vrpn_float32 * zEnd,
         vrpn_float32 * zPullback, vrpn_float32 * forceLimit,
         vrpn_float32 * distBetweenFC, vrpn_int32 * numPoints, vrpn_int32 * numHalfcycles,
	 vrpn_float32 * sampleSpeed, vrpn_float32 * pullbackSpeed, vrpn_float32 * startSpeed,
	 vrpn_float32 * feedbackSpeed, vrpn_int32 * avgNum,
	 vrpn_float32 * sampleDelay, vrpn_float32 * pullbackDelay, vrpn_float32 * feedbackDelay) {
  CHECK(vrpn_unbuffer(buf, setpoint));
  CHECK(vrpn_unbuffer(buf, startDelay));
  CHECK(vrpn_unbuffer(buf, zStart));
  CHECK(vrpn_unbuffer(buf, zEnd));
  CHECK(vrpn_unbuffer(buf, zPullback));
  CHECK(vrpn_unbuffer(buf, forceLimit));
  CHECK(vrpn_unbuffer(buf, distBetweenFC));
  CHECK(vrpn_unbuffer(buf, numPoints));
  CHECK(vrpn_unbuffer(buf, numHalfcycles));
  CHECK(vrpn_unbuffer(buf, sampleSpeed));
  CHECK(vrpn_unbuffer(buf, pullbackSpeed));
  CHECK(vrpn_unbuffer(buf, startSpeed));
  CHECK(vrpn_unbuffer(buf, feedbackSpeed));
  CHECK(vrpn_unbuffer(buf, avgNum));
  CHECK(vrpn_unbuffer(buf, sampleDelay));
  CHECK(vrpn_unbuffer(buf, pullbackDelay));
  CHECK(vrpn_unbuffer(buf, feedbackDelay));

  return 0;
}

char * nmm_Microscope::encode_ForceParameters (long * len,
       vrpn_int32 enableModify, vrpn_float32 scrap) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = sizeof(vrpn_int32) + sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_ForceParameters:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, enableModify);
    vrpn_buffer(&mptr, &mlen, scrap);
  }

  return msgbuf;
}

long nmm_Microscope::decode_ForceParameters (const char ** buf,
         vrpn_int32 * enableModify, vrpn_float32 * scrap) {
  CHECK(vrpn_unbuffer(buf, enableModify));
  CHECK(vrpn_unbuffer(buf, scrap));

  return 0;
}

char * nmm_Microscope::encode_BaseModParameters (long * len,
           vrpn_float32 min, vrpn_float32 max) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 2 * sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_BaseModParameters:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, min);
    vrpn_buffer(&mptr, &mlen, max);
  }

  return msgbuf;
}

long nmm_Microscope::decode_BaseModParameters (const char ** buf,
         vrpn_float32 * min, vrpn_float32 * max) {
  CHECK(vrpn_unbuffer(buf, min));
  CHECK(vrpn_unbuffer(buf, max));

  return 0;
}

char * nmm_Microscope::encode_BeginFeelTo (long * len,
                                           vrpn_float32 x, vrpn_float32 y) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 2 * sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_BeginFeelTo:  "
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

long nmm_Microscope::decode_BeginFeelTo (const char ** buf,
                                         vrpn_float32 * x, vrpn_float32 * y) {
  CHECK(vrpn_unbuffer(buf, x));
  CHECK(vrpn_unbuffer(buf, y));

  return 0;
}

char * nmm_Microscope::encode_EndFeelTo (long * len,
                                         vrpn_float32 x, vrpn_float32 y,
                                         vrpn_int32 numx, vrpn_int32 numy,
                                         vrpn_float32 dx, vrpn_float32 dy,
                                         vrpn_float32 orientation) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 5 * sizeof(vrpn_float32) + 2 * sizeof(vrpn_int32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_EndFeelTo:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, x);
    vrpn_buffer(&mptr, &mlen, y);
    vrpn_buffer(&mptr, &mlen, numx);
    vrpn_buffer(&mptr, &mlen, numy);
    vrpn_buffer(&mptr, &mlen, dx);
    vrpn_buffer(&mptr, &mlen, dy);
    vrpn_buffer(&mptr, &mlen, orientation);
  }

  return msgbuf;
}

long nmm_Microscope::decode_EndFeelTo (const char ** buf,
                                       vrpn_float32 * x, vrpn_float32 * y,
                                       vrpn_int32 * numx, vrpn_int32 * numy,
                                       vrpn_float32 * dx, vrpn_float32 * dy,
                                       vrpn_float32 * orientation) {
  CHECK(vrpn_unbuffer(buf, x));
  CHECK(vrpn_unbuffer(buf, y));
  CHECK(vrpn_unbuffer(buf, numx));
  CHECK(vrpn_unbuffer(buf, numy));
  CHECK(vrpn_unbuffer(buf, dx));
  CHECK(vrpn_unbuffer(buf, dy));
  CHECK(vrpn_unbuffer(buf, orientation));

  return 0;
}



char * nmm_Microscope::encode_ForceSettings (long * len,
           vrpn_float32 min, vrpn_float32 max, vrpn_float32 setpoint) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 3 * sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_ForceSettings:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, min);
    vrpn_buffer(&mptr, &mlen, max);
    vrpn_buffer(&mptr, &mlen, setpoint);
  }

  return msgbuf;
}

long nmm_Microscope::decode_ForceSettings (const char ** buf,
         vrpn_float32 * min, vrpn_float32 * max, vrpn_float32 * setpoint) {
  CHECK(vrpn_unbuffer(buf, min));
  CHECK(vrpn_unbuffer(buf, max));
  CHECK(vrpn_unbuffer(buf, setpoint));

  return 0;
}

char * nmm_Microscope::encode_InModModeT (long * len,
           vrpn_int32 sec, vrpn_int32 usec) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 2 * sizeof(vrpn_int32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_InModModeT:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, sec);
    vrpn_buffer(&mptr, &mlen, usec);
  }

  return msgbuf;
}

long nmm_Microscope::decode_InModModeT (const char ** buf,
         vrpn_int32 * sec, vrpn_int32 * usec) {
  CHECK(vrpn_unbuffer(buf, sec));
  CHECK(vrpn_unbuffer(buf, usec));

  return 0;
}

char * nmm_Microscope::encode_InImgModeT (long * len,
           vrpn_int32 sec, vrpn_int32 usec) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 2 * sizeof(vrpn_int32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_InImgModeT:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, sec);
    vrpn_buffer(&mptr, &mlen, usec);
  }

  return msgbuf;
}

long nmm_Microscope::decode_InImgModeT (const char ** buf,
         vrpn_int32 * sec, vrpn_int32 * usec) {
  CHECK(vrpn_unbuffer(buf, sec));
  CHECK(vrpn_unbuffer(buf, usec));

  return 0;
}

char * nmm_Microscope::encode_ModForceSet (long * len,
           vrpn_float32 setpoint) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_ModForceSet:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, setpoint);
  }

  return msgbuf;
}

long nmm_Microscope::decode_ModForceSet (const char ** buf,
         vrpn_float32 * setpoint) {
  CHECK(vrpn_unbuffer(buf, setpoint));

  return 0;
}

char * nmm_Microscope::encode_ImgForceSet (long * len,
           vrpn_float32 setpoint) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_ImgForceSet:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, setpoint);
  }

  return msgbuf;
}

long nmm_Microscope::decode_ImgForceSet (const char ** buf,
         vrpn_float32 * setpoint) {
  CHECK(vrpn_unbuffer(buf, setpoint));

  return 0;
}

char * nmm_Microscope::encode_ModSet (long * len,
           vrpn_int32 enableModify, vrpn_float32 min, vrpn_float32 max, vrpn_float32 setpoint) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = sizeof(vrpn_int32) + 3 * sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_ModSet:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, enableModify);
    vrpn_buffer(&mptr, &mlen, min);
    vrpn_buffer(&mptr, &mlen, max);
    vrpn_buffer(&mptr, &mlen, setpoint);
  }

  return msgbuf;
}

long nmm_Microscope::decode_ModSet (const char ** buf,
         vrpn_int32 * enableModify, vrpn_float32 * min, vrpn_float32 * max, vrpn_float32 * setpoint) {
  CHECK(vrpn_unbuffer(buf, enableModify));
  CHECK(vrpn_unbuffer(buf, min));
  CHECK(vrpn_unbuffer(buf, max));
  CHECK(vrpn_unbuffer(buf, setpoint));

  return 0;
}

char * nmm_Microscope::encode_ImgSet (long * len,
           vrpn_int32 enableModify, vrpn_float32 min, vrpn_float32 max, vrpn_float32 setpoint) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = sizeof(vrpn_int32) + 3 * sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_ImgSet:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, enableModify);
    vrpn_buffer(&mptr, &mlen, max);  // NOTE REVERSAL OF MAX AND MIN
    vrpn_buffer(&mptr, &mlen, min);
    vrpn_buffer(&mptr, &mlen, setpoint);
  }

  return msgbuf;
}

long nmm_Microscope::decode_ImgSet (const char ** buf,
         vrpn_int32 * enableModify, vrpn_float32 * min, vrpn_float32 * max, vrpn_float32 * setpoint) {
  CHECK(vrpn_unbuffer(buf, enableModify));
  CHECK(vrpn_unbuffer(buf, min));
  CHECK(vrpn_unbuffer(buf, max));
  CHECK(vrpn_unbuffer(buf, setpoint));

  return 0;
}

char * nmm_Microscope::encode_ForceSet (long * len,
           vrpn_float32 scrap) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_ForceSet:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, scrap);
  }

  return msgbuf;
}

long nmm_Microscope::decode_ForceSet (const char ** buf,
         vrpn_float32 * scrap) {
  CHECK(vrpn_unbuffer(buf, scrap));

  return 0;
}

char * nmm_Microscope::encode_ForceSetFailure (long * len,
           vrpn_float32 scrap) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_ForceSetFailure:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, scrap);
  }

  return msgbuf;
}

long nmm_Microscope::decode_ForceSetFailure (const char ** buf,
         vrpn_float32 * scrap) {
  CHECK(vrpn_unbuffer(buf, scrap));

  return 0;
}





char * nmm_Microscope::encode_PulseParameters (long * len,
           vrpn_int32 enabled, vrpn_float32 biasVoltage, vrpn_float32 peakVoltage, vrpn_float32 width) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = sizeof(vrpn_int32) + 3 * sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_PulseParameters:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, enabled);
    vrpn_buffer(&mptr, &mlen, biasVoltage);
    vrpn_buffer(&mptr, &mlen, peakVoltage);
    vrpn_buffer(&mptr, &mlen, width);
  }

  return msgbuf;
}

long nmm_Microscope::decode_PulseParameters (const char ** buf,
        vrpn_int32 * enabled, vrpn_float32 * biasVoltage, vrpn_float32 * peakVoltage,
        vrpn_float32 * width) {
  CHECK(vrpn_unbuffer(buf, enabled));
  CHECK(vrpn_unbuffer(buf, biasVoltage));
  CHECK(vrpn_unbuffer(buf, peakVoltage));
  CHECK(vrpn_unbuffer(buf, width));

  return 0;
}

char * nmm_Microscope::encode_PulseCompletedNM (long * len,
           vrpn_float32 x, vrpn_float32 y) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 2 * sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_PulseCompletedNM:  "
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

long nmm_Microscope::decode_PulseCompletedNM (const char ** buf,
        vrpn_float32 * x, vrpn_float32 * y) {
  CHECK(vrpn_unbuffer(buf, x));
  CHECK(vrpn_unbuffer(buf, y));

  return 0;
}

char * nmm_Microscope::encode_PulseFailureNM (long * len,
           vrpn_float32 x, vrpn_float32 y) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 2 * sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_PulseFailureNM:  "
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

long nmm_Microscope::decode_PulseFailureNM (const char ** buf,
        vrpn_float32 * x, vrpn_float32 * y) {
  CHECK(vrpn_unbuffer(buf, x));
  CHECK(vrpn_unbuffer(buf, y));

  return 0;
}

char * nmm_Microscope::encode_SetBias (long * len, vrpn_float32 voltage) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_SetBias:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, voltage);
  }
  return msgbuf;
}

long nmm_Microscope::decode_SetBias (const char ** buf, vrpn_float32 * voltage) {
  CHECK(vrpn_unbuffer(buf, voltage));

  return 0;
}

char * nmm_Microscope::encode_SetPulsePeak (long * len, vrpn_float32 voltage) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_SetPulsePeak:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, voltage);
  }
  return msgbuf;
}

long nmm_Microscope::decode_SetPulsePeak (const char ** buf, vrpn_float32 * voltage) {
  CHECK(vrpn_unbuffer(buf, voltage));

  return 0;
}

char * nmm_Microscope::encode_SetPulseDuration (long * len, vrpn_float32 duration) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_SetPulseDuration:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, duration);
  }
  return msgbuf;
}

long nmm_Microscope::decode_SetPulseDuration (const char ** buf,
                                              vrpn_float32 * duration) {
  CHECK(vrpn_unbuffer(buf, duration));

  return 0;
}

char * nmm_Microscope::encode_EnterScanlineMode (long * len,
           vrpn_int32 enable) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 1 * sizeof(vrpn_int32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_EnterScanlineMode:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, enable);
  }

  return msgbuf;
}

long nmm_Microscope::decode_EnterScanlineMode (const char ** buf,
         vrpn_int32 *enable) {
  CHECK(vrpn_unbuffer(buf, enable));

  return 0;
}

char * nmm_Microscope::encode_InScanlineMode (long * len,
           vrpn_int32 enable) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 1 * sizeof(vrpn_int32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_InScanlineMode:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, enable);
  }

  return msgbuf;
}

long nmm_Microscope::decode_InScanlineMode (const char ** buf,
         vrpn_int32 *enable) {
  CHECK(vrpn_unbuffer(buf, enable));

  return 0;
}

char * nmm_Microscope::encode_RequestScanLine(long *len, vrpn_float32 x, vrpn_float32 y,
                vrpn_float32 z, vrpn_float32 angle, vrpn_float32 slope, vrpn_float32 width, vrpn_int32 res,
                vrpn_int32 enable_feedback, vrpn_int32 check_forcelimit,
                vrpn_float32 max_force, vrpn_float32 max_z_step, vrpn_float32 max_xy_step) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 9*sizeof(vrpn_float32) + 3*sizeof(vrpn_int32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_ScanLine:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, x);
    vrpn_buffer(&mptr, &mlen, y);
    vrpn_buffer(&mptr, &mlen, z);
    vrpn_buffer(&mptr, &mlen, angle);
    vrpn_buffer(&mptr, &mlen, slope);
    vrpn_buffer(&mptr, &mlen, width);
    vrpn_buffer(&mptr, &mlen, res);
    vrpn_buffer(&mptr, &mlen, enable_feedback);
    vrpn_buffer(&mptr, &mlen, check_forcelimit);
    vrpn_buffer(&mptr, &mlen, max_force);
    vrpn_buffer(&mptr, &mlen, max_z_step);
    vrpn_buffer(&mptr, &mlen, max_xy_step);
  }
  return msgbuf;
}

long nmm_Microscope::decode_RequestScanLine(const char ** buf, vrpn_float32 *x,
    vrpn_float32 *y, vrpn_float32 *z, vrpn_float32 *angle, vrpn_float32 *slope, vrpn_float32 *width, vrpn_int32 *res,
    vrpn_int32 *enable_feedback, vrpn_int32 *check_forcelimit, vrpn_float32 *max_force,
    vrpn_float32 *max_z_step, vrpn_float32 *max_xy_step) {
  CHECK(vrpn_unbuffer(buf, x));
  CHECK(vrpn_unbuffer(buf, y));
  CHECK(vrpn_unbuffer(buf, z));
  CHECK(vrpn_unbuffer(buf, angle));
  CHECK(vrpn_unbuffer(buf, slope));
  CHECK(vrpn_unbuffer(buf, width));
  CHECK(vrpn_unbuffer(buf, res));
  CHECK(vrpn_unbuffer(buf, enable_feedback));
  CHECK(vrpn_unbuffer(buf, check_forcelimit));
  CHECK(vrpn_unbuffer(buf, max_force));
  CHECK(vrpn_unbuffer(buf, max_z_step));
  CHECK(vrpn_unbuffer(buf, max_xy_step));

  return 0;
}

// offset gives the location of the first element of each dataset in the
// data array; successive elements for each dataset are expected to
// immediately follow the first (i.e. the data for a given channel is one
// contiguous block)
char * nmm_Microscope::encode_ScanlineData(long *len, 
    vrpn_float32 x, vrpn_float32 y, vrpn_float32 z,
    vrpn_float32 angle, 
    vrpn_float32 slope, vrpn_float32 width,
    vrpn_int32 resolution, vrpn_int32 feedback_enabled, 
    vrpn_int32 checking_forcelimit,
    vrpn_float32 max_force_setting, 
    vrpn_float32 max_z_step, vrpn_float32 max_xy_step,
    vrpn_int32 sec, vrpn_int32 usec,
    vrpn_int32 num_channels, vrpn_int32 * offset, 
    vrpn_float32 *data) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;
  long i,j;
  vrpn_float32 val;

  if (!len) return NULL;

  *len = 9*sizeof(vrpn_float32) + 6*sizeof(vrpn_int32) +
    num_channels*resolution*sizeof(vrpn_float32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_ScanlineData:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, x);
    vrpn_buffer(&mptr, &mlen, y);
    vrpn_buffer(&mptr, &mlen, z);
    vrpn_buffer(&mptr, &mlen, angle);
    vrpn_buffer(&mptr, &mlen, slope);
    vrpn_buffer(&mptr, &mlen, width);
    vrpn_buffer(&mptr, &mlen, resolution);
    vrpn_buffer(&mptr, &mlen, feedback_enabled);
    vrpn_buffer(&mptr, &mlen, checking_forcelimit);
    vrpn_buffer(&mptr, &mlen, max_force_setting);
    vrpn_buffer(&mptr, &mlen, max_z_step);
    vrpn_buffer(&mptr, &mlen, max_xy_step);
    vrpn_buffer(&mptr, &mlen, sec);
    vrpn_buffer(&mptr, &mlen, usec);
    vrpn_buffer(&mptr, &mlen, num_channels);

    for (i = 0; i < resolution; i++)
      for (j = 0; j < num_channels; j++) {
    val = (vrpn_float32)(data[offset[j]+i]);
        vrpn_buffer(&mptr, &mlen, val);
      }
  }
  return msgbuf;
}

long nmm_Microscope::decode_ScanlineDataHeader(const char ** buf,
    vrpn_float32 *x, vrpn_float32 *y, vrpn_float32 *z, 
    vrpn_float32 *angle, vrpn_float32 *slope, vrpn_float32 *width,
    vrpn_int32 *resolution, 
    vrpn_int32 *feedback_enabled, vrpn_int32 *checking_forcelimit,
    vrpn_float32 *max_force_setting, 
    vrpn_float32 * max_z_step, vrpn_float32 *max_xy_step,
    vrpn_int32 *sec, vrpn_int32 *usec, vrpn_int32 *num_channels)
{
  CHECK(vrpn_unbuffer(buf, x));
  CHECK(vrpn_unbuffer(buf, y));
  CHECK(vrpn_unbuffer(buf, z));
  CHECK(vrpn_unbuffer(buf, angle));
  CHECK(vrpn_unbuffer(buf, slope));
  CHECK(vrpn_unbuffer(buf, width));
  CHECK(vrpn_unbuffer(buf, resolution));
  CHECK(vrpn_unbuffer(buf, feedback_enabled));
  CHECK(vrpn_unbuffer(buf, checking_forcelimit));
  CHECK(vrpn_unbuffer(buf, max_force_setting));
  CHECK(vrpn_unbuffer(buf, max_z_step));
  CHECK(vrpn_unbuffer(buf, max_xy_step));
  CHECK(vrpn_unbuffer(buf, sec));
  CHECK(vrpn_unbuffer(buf, usec));
  CHECK(vrpn_unbuffer(buf, num_channels));

  return 0;
}

long nmm_Microscope::decode_ScanlineDataPoint(const char ** buf,
            vrpn_int32 fieldCount, vrpn_float32 *fieldValues){
  for (int i = 0; i < fieldCount; i++)
    CHECK(vrpn_unbuffer(buf, &(fieldValues[i])));

  return 0;
}

char * nmm_Microscope::encode_JumpToScanLine (long *len, 
                                              vrpn_int32 line_number)
{
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;
  
  *len = sizeof(vrpn_int32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_JumpToScanLine:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, line_number);
  }

  return msgbuf;
}

long nmm_Microscope::decode_JumpToScanLine (const char ** buf, 
                                            vrpn_int32 *line_number)
{
  CHECK(vrpn_unbuffer(buf, line_number));
  return 0;
}

char * nmm_Microscope::encode_EnableUpdatableQueue (long * len, 
                                                    vrpn_bool on) {
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;
  
  *len = sizeof(vrpn_bool);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_EnableUpdatableQueue:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, on);
  }

  return msgbuf;
}

long nmm_Microscope::decode_EnableUpdatableQueue (const char ** buf, 
                                                  vrpn_bool * on) {
  CHECK(vrpn_unbuffer(buf, on));
  return 0;
}



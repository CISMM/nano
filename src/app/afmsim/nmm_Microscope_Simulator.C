#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "nmm_Microscope_Simulator.h"
#include <BCPlane.h>
//#include "nmb_Util.h"		

#ifndef WARRENCNTSIM_H
#include "warrencntsim.h"
#endif

#include <vrpn_Types.h>

// Tom Hudson 10 June 99
// Things that were declared in the header file by the 145 team
// that should have been declared here.

int mic_start;                     // true when connection is active
float set_point;
float last_point_x, last_point_y;  // last point modified
static int num_x, num_y; // Declared for use by parser for variable grid size JakeK


typedef struct {
  long numsets;
  char name[MAX_REQUEST_DATA_SETS][64];
} REQUEST_DATASETS;

typedef struct {
  int numsets;
  short idAcq[MAX_REQUEST_DATA_SETS];
  int  applied[MAX_REQUEST_DATA_SETS];
} REQUEST_DATASETS_ID;

typedef struct {
  char name[64];
  long num_samples;
} PDATAUNIT;

typedef struct {
  long numsets;
  PDATAUNIT pdata[MAX_REQUEST_DATA_SETS];
} REQUEST_POINTSETS;



#define CHECK(a) if ((a) == -1) return -1
#define ABORT(a) if ((a) == -1) exit(0)



static void ServerError( char * txt );
static void ConnectServer (int);


/*****************************************************************
 PUBLIC GLOBAL VARIABLES
 *****************************************************************/

vrpn_Connection * connection = NULL;
nmm_Microscope_Simulator * AFMSimulator = NULL;



/*****************************************************************
 PRIVATE GLOBAL VARIABLES
 *****************************************************************/
//static char      szServerClass[] = "Server";
static vrpn_bool      ServerOn;
static vrpn_bool	 abnormal_shutdown;
//static vrpn_bool	 connectionActiveFlag;

static void ServerOutputAdd (int level, const char *__format, ...);
void ServerOutputAdd (int /*level*/, const char *__format, ...) {
  static va_list argptr;
  //  static int     number;

  /* Print text in a string */
  va_start(argptr, __format);
  vfprintf (stdout, __format, argptr);
  va_end(argptr);
  printf("\n");
}

int mic_Started()	// Added by JakeK so that scanning doesn't begin before connection is made
{			// May also be used to stop scanning when user is in Point mode
	return mic_start;
}


void StartServer (int x, int y, int port) // Added these varialbes so that the grid size can be initialized
{				// JakeK
  num_x = x;  
  num_y = y;

  if ( NULL == connection ) ConnectServer(port);
  ServerOutputAdd(1, "Generating AFMSimulator......");
  AFMSimulator = new nmm_Microscope_Simulator( "nmm_Microscope_Simulator@wherever", connection );
  if ( AFMSimulator ) {
	ServerOn = VRPN_TRUE;
  }
}

void StopServer (void)
{
	  if (ServerOn) {
	     ServerOutputAdd(1,"");
	     ServerOutputAdd(1,"STOPPING SERVER");

		AFMSimulator->ShutdownSession();
		delete AFMSimulator;
		ServerOn = VRPN_FALSE;

		delete connection;
		connection = NULL;

		ServerOutputAdd(1, "Server closed successfully");
		mic_start = 0;
	  }
}

void ConnectServer (int port)
{

  connection = new vrpn_Synchronized_Connection(port);

  if (!connection || !connection->doing_okay()) {
     ServerOutputAdd(2, "Connection could not be accepted");
	return;
  }

}






//
// Constructor
//
nmm_Microscope_Simulator::
nmm_Microscope_Simulator (const char * _name,
			  vrpn_Connection * _c) :
    nmb_SharedDevice_Server (_name, _c),
    nmm_Microscope (_name, d_connection)
{


  spmFirstPointSinceZoom = 1;
  cTimer2 = 0;
  ProcessSuspended = VRPN_FALSE;

  stm_scan_style = 0;

  stm_num_samples = 1;
  stm_sample_freq = 160E3;

  spm_driveamp_set_last = 0.0;
  mic_start = 0;   		//added by JakeK to initialize the scanning to stopped
  UpdateFeedbackParamsNow = 1;
  RegisterAmpEnabled = 1;

  if ( !d_connection ) {
    ServerOutputAdd(2, "Could not connect to \"%S\".\n", _name );
    exit( 0 );
  }

  d_connection->register_handler(d_SetScanStyle_type,
				RcvSetScanStyle,
				this);

  d_connection->register_handler(d_SetSlowScan_type,
				RcvSetSlowScan,
				this);

  d_connection->register_handler(d_SetRegionNM_type,
				RcvSetRegionNM,
				this);

  d_connection->register_handler(d_ScanTo_type,
				 RcvScanPointNM,
				this);

  d_connection->register_handler(d_ZagTo_type,
				RcvZagPointNM,
				this);

  d_connection->register_handler(d_SetStdDelay_type,
				RcvSetStdDelay,
				this);

  d_connection->register_handler(d_SetStPtDelay_type,
				RcvSetStPtDelay,
				this);

  d_connection->register_handler(d_SetRelax_type,
				RcvSetRelax,
				this);

  d_connection->register_handler(d_SetStdDevParams_type,
				RcvSetStdDevParams,
				this);

  d_connection->register_handler(d_SetScanWindow_type,
				RcvScanWindow,
				this);

  d_connection->register_handler(d_SetGridSize_type,
				RcvSetGridSize,
				this);

/* need to be implemented
  connection->register_handler(d_SetOhmmeterSampleRate_type,
*/

  d_connection->register_handler(d_EnableAmp_type,
				RcvEnableAmp,
				this);

  d_connection->register_handler(d_DisableAmp_type,
				RcvDisableAmp,
				this);

  d_connection->register_handler(d_SetRateNM_type,
				RcvSetRatenMeters,
				this);

  d_connection->register_handler(d_EnableVoltsource_type,
				RcvEnableVoltsource,
				this);

  d_connection->register_handler(d_DisableVoltsource_type,
				RcvDisableVoltsource,
				this);

  d_connection->register_handler(d_SetMaxMove_type,
				RcvSetMaxMove,
				this);

  d_connection->register_handler(d_EnterTappingMode_type,
				 RcvTappingMode,
				this);

  d_connection->register_handler(d_EnterContactMode_type,
				 RcvContactMode,
				this);

  d_connection->register_handler(d_EnterSewingStyle_type,
				 RcvSewingMode,
				this);

  d_connection->register_handler(d_DrawSharpLine_type,
				RcvSharpLine,
				this);

  d_connection->register_handler(d_DrawSweepLine_type,
				RcvSweepLine,
				this);

  d_connection->register_handler(d_DrawSweepArc_type,
				RcvSweepArc,
				this);

  d_connection->register_handler(d_GetNewPointDatasets_type,
				RcvReqPotDataset,
				this);

  d_connection->register_handler(d_GetNewScanDatasets_type,
				RcvReqScanDataset,
				this);

  d_connection->register_handler(d_QueryScanRange_type,
				RcvQueryScanRange,
				this);

	// Added by JakeK and AFMS team to resume scanning
  d_connection->register_handler(d_ResumeWindowScan_type, 
				RcvResumeWindowScan,
				this);
  //RcvEcho
  /*************************************************************/
  // Added in place of Echo to relay what mode "microscope is in"
  d_connection->register_handler(d_MarkModify_type,
                                RcvMarkModify,
                                this);

  d_connection->register_handler(d_MarkImage_type,
                                RcvMarkImage,
                                this);
  /*************************************************************/

  d_Shutdown_type = d_connection->register_message_type
	  (vrpn_dropped_last_connection);

  d_connection->register_handler(d_Shutdown_type,
				RcvShutdown,
				this);

  d_GetConnectionFromClient_type = d_connection->register_message_type
	  (vrpn_got_connection);

  d_connection->register_handler(d_GetConnectionFromClient_type,
				RcvGetConnection,
				this);

}

nmm_Microscope_Simulator::
~nmm_Microscope_Simulator()
{
}

float nmm_Microscope_Simulator::
spm_read_current_driveamp(void)
{
  return spm_driveamp_set_last;
}

void nmm_Microscope_Simulator::
spm_store_current_driveamp(float damp)
{
  spm_driveamp_set_last = damp;
  //ServerOutputAdd (1, "spm_store_current_driveamp: Drive amp now: %f", damp);

  spm_report_current_mode();
}

// This function should be called right after the client connects!  It
// sends all the default values for the controls to microscape, so
// that we agree with microscape.
void nmm_Microscope_Simulator::
stm_init(void)
{
/**************************************************************************
 * Set the initial values for program variables
 **************************************************************************/
 
  // 500 nm/sec = 0.5 microns/sec. This is fairly slow.
  stm_nmeters_per_second = 500;
  
  stm_grid_num_x = num_x;	// Added by JakeK this sets the size of the Grid
  stm_grid_num_y = num_y;	// and allows us to use variable grid size

  stm_window_xmin = 0;
  stm_window_ymin = 0;
  stm_window_xmax = stm_grid_num_x - 1;
  stm_window_ymax = stm_grid_num_y - 1;

  stm_current_x = 0;			// Where we are right now
  stm_current_y = 0;

  // XXX I'm not sure the region has been set at this point
  stm_desired_x = stm_region_xmin;	// Where we are seeking to be
  stm_desired_y = stm_region_ymin;

  stm_desired_approach = 0.0;		// Desired approach location
  stm_coarse_z = -10.0;         	// Coarse Z position

  stm_scan_x = 0;               	// Where the area scan is right now
  stm_scan_y = 0;
  stm_scan_dx = 1;
  stm_scan_dy = 1;

  fmods_enabled = 1;

  stm_current_mode = STM_IDLE_MODE;

  // Factors to convert Z ADC values to nanometers
  // The value reported by the AFM, and the value sent to microscape (?)
  // are long ints. If we have a value lz, we convert it to nanometers
  // by doing z_in_nm = lz* fDACtoWorld +fDACtoWorldZero;

  fDACtoWorld = fDACtoWorldZero = 0.0;
  //ServerOutputAdd (2, "fDACtoWorld: %f", fDACtoWorld);
  //ServerOutputAdd (2, "fDACtoWorldZero: %f", fDACtoWorldZero);

	// Initial mode is imaging mode
  force_mode = IMAGING_MODE;
  afm_mode   = CONTACT_MODE;

	// No ADC held at the beginning
  ADC1held = ADC2held = 0;
  RelaxDelay = 0;

  afm_first_punch = VRPN_TRUE;		// We haven't punched yet. This will
					// be the first time.
  afm_first_punch_distance = 0;		// If we punch out before punching
					// in, no extra to go

	// Inform the selected scanner description
	// THIS MUST COME BEFORE ANY OTHER REPORTS,
	// since it is the hello message
  report_selected_scanner_desc();


	// Reset Set region Received Flag
  SetRegionReceived = VRPN_FALSE;

  float pv=1, iv=0.5, dv=0;      	// Reasonable default values...
  spm_report_latest_PID(pv,iv,dv);	// Report PID Values

  spm_report_latest_scanrate (500);	// report nM/second
  spm_report_latest_resolution();	// report latest grid size selected.
  spm_report_latest_region();		// report currently selected scan region

  startup_p = pv;
  startup_i = iv;
  startup_d = dv;
  startup_setpoint = 10; // This is arbitrary - could be -10 to 50, maybe.

  startup_scanrate = stm_nmeters_per_second;
  startup_scanmode = afm_mode;    // Scanmode should only be TAPPING or CONTACT
  spm_report_current_mode();		// Report what mode we are
					// in and its parameters

  // We don't know what to put in the topo header, so skip for now...
  //spm_report_topo_header();      	// Report the header to allow the
					// Unix side to save files

  //////////////////////////////////////////////////////////////////////
  // Code added by JakeK and AFMS team                                //
  //////////////////////////////////////////////////////////////////////

  //int retval;
  char * msgbuf;
  long len;
 
  msgbuf = encode_ScanDataset( &len, 1, "Topography-Forward", "nm", 0, 1);	// Initialize Microscape that it is going
  Send(len, d_ScanDataset_type, msgbuf);		  	// to be scanning Z values

  msgbuf = encode_PointDataset( &len, 1, "Topography", "nm", 0, 1); 	// Initailize Microscape that it is going
  Send(len, d_PointDataset_type, msgbuf);			// to touch Z values 


  mic_start = 1;	// This tells us to start scanning the surface

}




///////////////////////////////////////////////////////////////////////
// Microscope specific info, so moved from base class. changed by JakeK
// AFMS team, no longer accepts *sets. It just takes the values now
///////////////////////////////////////////////////////////////////////
char * nmm_Microscope_Simulator::encode_ScanDataset (long * len, int numsets, char * name, char * units, int offset, int scale)
{
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;
	//  long numsets;

  if (!len) return NULL;

  *len = (int)(sizeof(long) + ( 2*64*sizeof(char) + 2*sizeof(float) ) * numsets);
  msgbuf = new char [*len];
  if (!msgbuf) 
  {
    //ServerOutputAdd(2, "nmm_Microscope_Simulator::encode_ScanDataset:  "
                    //"Out of memory.\n");
    *len = 0;
  } 
  else 
  {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, numsets);
    for (int i=0; i<numsets; i++) 
    {
        vrpn_buffer(&mptr, &mlen, name, 64);
        vrpn_buffer(&mptr, &mlen, units, 64);
        vrpn_buffer(&mptr, &mlen, (float)offset);
        vrpn_buffer(&mptr, &mlen, (float)scale);
    }
  }

  return msgbuf;
}




//////////////////////////////////////////////////////////////////////////
// Changed by JakeK and AFMS team, same changes as above
//////////////////////////////////////////////////////////////////////////

// Microscope specific info, so moved from base class.
char * nmm_Microscope_Simulator::encode_PointDataset (long * len, int numsets, char * name, char * units, int offset, int scale)
{      // XXX Tiger
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;
  vrpn_int32 numsamples = 1; // JakeK I really have no idea what this is


  if (!len) return NULL;
  *len = (sizeof(long) + ( 2*64*sizeof(char) + sizeof(int)
                         + 2*sizeof(float) ) * numsets);  // XXX Tiger

  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "nmm_Microscope::encode_PointDataset:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, numsets);
    for (int i=0; i<numsets; i++) {
       // Need to figure this out...
       //XXX        vrpn_buffer(&mptr, &mlen, GetPointTypeName(point.idAcqType[i]), 64);
        vrpn_buffer(&mptr, &mlen, name, 64);
        vrpn_buffer(&mptr, &mlen, units,64);
        vrpn_buffer(&mptr, &mlen, numsamples);
        vrpn_buffer(&mptr, &mlen, (float)offset);
        vrpn_buffer(&mptr, &mlen, (float)scale);
    }
  }

  return msgbuf;
}



// Called by real Topo AFM when the client first connects - initialization info
int nmm_Microscope_Simulator::
report_selected_scanner_desc(void)
{
/**************************************************************************
 * Report the description of the selected scanner
 * Current version = 2
 **************************************************************************/
   char * msgbuf;
   long len;
   int retval;

   // Make up a reasonable scanner description.
   char ScannerDesc[64] = "Simulator Test Scanner";


   //ServerOutputAdd (1, "Reporting Selected Scanner Description");
   //ServerOutputAdd (2, "%s", ScannerDesc);
   
   // Version needs to match that of microscape, or microscape will shut down.
   // We make this check so that we know we are sending the same messages...
   msgbuf = encode_HelloMessage( &len, "nM!", ScannerDesc, NANOM_VERSION,
				 NANOM_REVISION );
   if ( !msgbuf ) {
      ServerOutputAdd (1, "nmm_Microscope_Topometrix::report_selected_scanner_desc : Buffer overflow!!" );
      return -1;
   } else {
      retval = Send( len, d_HelloMessage_type, msgbuf );
      if ( retval ) {
	 ServerOutputAdd(1, "nmm_Microscope_Topometrix::report_selected_scanner_desc:  Couldn't pack messge to send to client." );
	 return -1;
      }
   }

   return 0;
}

// Called by real Topo AFM when the client first connects - initialization info
int nmm_Microscope_Simulator::
spm_report_latest_PID(float pv, float iv, float dv)
{
/***************************************************************************
 * Report the latest PID
 ***************************************************************************/

   long len;
   char * msgbuf;
   int retval;

   if (ServerOn && UpdateFeedbackParamsNow) {
      //ServerOutputAdd (1, "Report_Latest_Pid - Reporting in progress");

      msgbuf = encode_PidParameters( &len, pv, iv, dv );
      if ( !msgbuf ) {
         ServerOutputAdd( 1, "nmm_Microscope_Simulator::spm_report_latest_PID:  Buffer overflow!!" );
         return -1;
      } else {
        retval = Send( len, d_PidParameters_type, msgbuf );
        if ( retval ) {
           ServerOutputAdd( 1, "nmm_Microscope_Simulator::spm_report_latest_PID:  Couldn't pack messge to send to client." );
           return -1;
        }
      }

      ServerOutputAdd (2, "Proportional: %f", pv);
      ServerOutputAdd (2, "Integral: %f", iv);
      ServerOutputAdd (2, "Derivative: %f", dv);
   }
  return 0;
}

// Called by real Topo AFM when the client first connects - initialization info
int nmm_Microscope_Simulator::
spm_report_latest_scanrate(float speed)
{
/***************************************************************************
 * Report the lastest Scan Rate, in nm/sec
 ***************************************************************************/

   long len;
   char * msgbuf;
   int retval;

   if (ServerOn && UpdateFeedbackParamsNow) {
      //ServerOutputAdd (1, "REPORT_LATEST_SCANRATE - Reporting in progress");

      msgbuf = encode_ScanrateParameter( &len, speed );
      if ( !msgbuf ) {
	ServerOutputAdd( 1, "nmm_Microscope_Simulator::spm_report_latest_scanrate:  Buffer overflow!!" );
        return -1;
      } else {
        retval = Send( len, d_ScanrateParameter_type, msgbuf );
        if ( retval ) {
          ServerOutputAdd( 1, "nmm_Microscope_Simulator::spm_report_latest_scanrate:  Couldn't pack messge to send to client." );
          return -1;
        }
      }
      ServerOutputAdd (2, "Scan Rate: %f", speed);
   }
  return 0;
}

// Called by real Topo AFM when the client first connects - initialization info
int nmm_Microscope_Simulator::
spm_report_latest_resolution()
{
/***************************************************************************
 * Report the latest resolution - number of samples in the grid in x and y.
 ***************************************************************************/

   long len;
   char * msgbuf;
   int retval;

   int res;
   if (ServerOn) {
      //ServerOutputAdd (1, "REPORT_LATEST_RESOLUTION - Reporting in progress");
      res = stm_grid_num_x; // Same in x and y for AFM

      msgbuf = encode_ReportGridSize( &len, res, res );
      if ( !msgbuf ) {
        ServerOutputAdd( 1, "nmm_Microscope_Simulator::spm_report_latest_resolution:  Buffer overflow!!" );
        return -1;
      } else {
        retval = Send( len, d_ReportGridSize_type, msgbuf );
        if ( retval ) {
          ServerOutputAdd( 1, "nmm_Microscope_Simulator::spm_report_latest_resolution:  Couldn't pack messge to send to client." );
          return -1;
        }
      }

      ServerOutputAdd (2, "Resolution : %i", res);
   }
  return 0;
}




/////////////////////////////////////////////////////////////////////
// Changed by JakeK and AFMS team
/////////////////////////////////////////////////////////////////////

// Called by real Topo AFM when the client first connects - initialization info
int nmm_Microscope_Simulator::
spm_report_latest_data_sets( SET_OF_DATA_SETS * /* set */)
{
  long len;
  char * msgbuf;
  int retval;

  msgbuf = encode_ScanDataset( &len, 1, "Topography-Forward", "nm", 0, 1); // Ignore vars passed and
							  // always use these values
							  // we only deal with Z data
							  // right now
  if ( !msgbuf ) {
     ServerOutputAdd( 1, "servdata: report_latest_data_sets: Buffer Overflow!!" );
     return -1;
  } else {
    retval = Send( len, d_ScanDataset_type, msgbuf );
    if ( retval ) {
       ServerOutputAdd( 1, "servdata: report_latest_data_sets: Couldn't pack messge to send to client." );
       return -1;
    }
  }

  return 0;
}  


///////////////////////////////////////////////////////////////
//Changed by JakeK and AFMS team
///////////////////////////////////////////////////////////////

// Called by real Topo AFM when the client first connects - initialization info
int nmm_Microscope_Simulator::
spm_report_latest_point_sets( SET_OF_POINT_SETS /* point */ )
{
  long len;
  char * msgbuf;
  int retval;

  msgbuf = encode_PointDataset( &len, 1, "Topography", "nm", 0, 1 ); // Ignores vars passed
							    // and always deals with z data
  if ( !msgbuf ) {
     ServerOutputAdd( 1, "servdata: report_lastest_point_sets: Buffer Overflow!!" );
     return -1;
  } else {
    retval = Send( len, d_PointDataset_type, msgbuf );
    if ( retval ) {
       ServerOutputAdd( 1, "servdata: report_latest_poing_sets: Buffer Overflow!!" );
       return -1;
    }
  }

  return 0;
}


///////////////////////////////////////////////////////////////
// Changed by JakeK and AFMS team
///////////////////////////////////////////////////////////////

// Called by real Topo AFM when the client first connects - initialization info
int nmm_Microscope_Simulator::
spm_report_latest_region()
{
/**************************************************************************
 * Report the latest Scan region
 * This is a square, with the edges specified in nanometers.
 * I think the values have to fall within the scanner's maximum range
 * which is why spm_report_region_clipped gets called at the end.
 **************************************************************************/

  float range, xmin, ymin, xmax, ymax, midx, midy;

    if (ServerOn) {
       //ServerOutputAdd (1, "REPORT_LATEST_REGION - Reporting in progress");

	
       	// This allows us to only use even number scan regions, it can cause
	// problems if we try to use odd JakeK
       if(num_x%2==1)
	{
	  num_x--;
	  num_y--;
        }
       range = num_x; 	// range is limited to grid size, this greatly simplifies
			// many operations for now due to the fact that we don't 
			// have to map the grid to nM JakeK

       // Put the center of the scan somewhere - make the scan
       // range non-symmetric, for fun.
       midx = (num_x/2);	// was 3000
       midy = (num_y/2);	// was 4000
       xmin = (float)(midx - range *0.5);
       xmax = (float)(midx + range *0.5);
       ymin = (float)(midy - range *0.5);
       ymax = (float)(midy + range *0.5);

	// XXX when the unix side starts to accept the angle as well as the
	// region, comment out the first return and uncomment out the
	// second return.
	return (spm_report_region_clipped(xmin, ymin, xmax, ymax));
	// return (spm_report_angle(xmin, ymin, xmax, ymax));

    }
    return 0;
}

// Called by real Topo AFM when the client first connects - initialization info
int nmm_Microscope_Simulator::
spm_report_current_mode(void)
{
   if (afm_mode == TAPPING_MODE) {
      return spm_report_tapping_mode();
   } else {
     return spm_report_contact_mode();
   }
}

// Called by real Topo AFM when the client first connects - initialization info
int nmm_Microscope_Simulator::
spm_report_topo_header()
{
/***************************************************************************
 * This routine sends a copy of the header that would be written to a saved
 * file. This will allow the unix side to write a snapshot in topo file
 * format, since it will have the fields set the way they should be. Some
 * fields will have to be modified, of course, such as the scan range, data
 * type, and other fields that have to do with the specific data type being
 * written. This routine should be called whenever a connection is made.
 ***************************************************************************/

  char 	mybuf[4096];		// Hope this is large enough to hold it
  long	mylen=0;

  long len;
  char * msgbuf;
  int retval;


        	// XXX ??? need to rewrite here -> ConnectionOn is in SERVER.C
  if (ServerOn) {
	// Get the header info into a buffer and find out how long it is.
		// XXX iBufferTopoFileHeader() defined in TOPOFILE.C:1595-1702
//      mylen = iBufferTopoFileHeader(mybuf, sizeof(mybuf), pImg);
     if (mylen == -1) {
		// XXX SERVOUT
        ServerOutputAdd(1, "ERROR -- spm_report_topo_header(): Can't get header");
        return -1;
     }

     msgbuf = encode_TopoFileHeader( &len, mybuf, mylen );	// XXX HACK
     if ( !msgbuf ) {
       ServerOutputAdd( 1, "nmm_Microscope_Simulator::spm_report_topo_header:  Buffer overflow!!" );
       return -1;
     } else {
       retval = Send( len, d_TopoFileHeader_type, msgbuf );
       if ( retval ) {
         ServerOutputAdd( 1, "nmm_Microscope_Simulator::spm_report_topo_header:  Couldn't pack message to send to client." ); 
         return -1;
       }
     }
  }
  return 0;
}

// This function called by real Topo AFM, indirectly
void nmm_Microscope_Simulator::
ShutdownSession (void)
{
/*****************************************************************
* Closes the connection socket, and resets the FSM of the server.
* The server now awaits a new connection.
* Also resets the microscope.
*****************************************************************/

  if (ServerOn) {
    ServerOutputAdd (1,"");
    ServerOutputAdd (1,"SHUTDOWN SESSION");
    // Stop scanning
    spm_stop_scan ();

    if (abnormal_shutdown) {
       ServerOutputAdd (1,"  Abnormal Shutdown");
    }
  }
}







// This message handled by real Topo AFM
int nmm_Microscope_Simulator::
RcvShutdown( void *_userdata, vrpn_HANDLERPARAM  )
{
  nmm_Microscope_Simulator *tmp = (nmm_Microscope_Simulator *) _userdata;
  //const char * bufptr = _p.buffer;

  ServerOutputAdd( 2, "Shutdown command received");
	
  tmp->helpShutdown();

  return 0;	
}

// This message handled by real Topo AFM
int nmm_Microscope_Simulator::
RcvGetConnection( void *_userdata, vrpn_HANDLERPARAM )
{
  nmm_Microscope_Simulator *tmp = (nmm_Microscope_Simulator *) _userdata;
  //  const char * bufptr = _p.buffer;

  ServerOutputAdd( 2, "Connection between client and server established.");

  tmp->helpGetConnection();

  return 0;
}

int nmm_Microscope_Simulator::
RcvSetRate( void * /*_userdata*/, vrpn_HANDLERPARAM /*_p*/ )
{
	// XXX haven't implemented yet
return 0;
}

// This message handled by real Topo AFM
int nmm_Microscope_Simulator::
RcvSetGridSize( void *_userdata, vrpn_HANDLERPARAM _p )
{
  nmm_Microscope_Simulator *tmp = (nmm_Microscope_Simulator *) _userdata;
  const char * bufptr = _p.buffer;

  if ( tmp->stm_set_grid_size( bufptr ) ) {
     ServerError( "stm_set_grid_size failed" );
     return -1;
  }

  return 0;
}


// This message handled by real Topo AFM
int nmm_Microscope_Simulator::
RcvScanWindow( void *_userdata, vrpn_HANDLERPARAM _p )
{
  nmm_Microscope_Simulator *tmp = (nmm_Microscope_Simulator *) _userdata;
  const char * bufptr = _p.buffer;

  if ( tmp->stm_scan_window( bufptr ) ) {
     ServerError( "stm_scan_window failed" );
     return -1;
  }

  return 0;
}

int nmm_Microscope_Simulator::
RcvScanPoint( void */*_userdata*/, vrpn_HANDLERPARAM /*_p*/ )
{
  return 0;
}

int nmm_Microscope_Simulator::
RcvIdle( void */*_userdata*/, vrpn_HANDLERPARAM /*_p*/ )
{
  return 0;
}

/***********************************************************************
 * Starts scanning the last defined area from the top left corner.
 ***********************************************************************/
// This message handled by real Topo AFM
int nmm_Microscope_Simulator::
RcvResumeWindowScan( void *_userdata, vrpn_HANDLERPARAM )
{
  nmm_Microscope_Simulator *tmp = (nmm_Microscope_Simulator *) _userdata;
  //  const char * bufptr = _p.buffer;
  
  if ( tmp->stm_resume_window_scan() ) {
     ServerError( "stm_resume_window_scan failed" );
     return -1;
  }

  return 0;
}

int nmm_Microscope_Simulator::
RcvSetStdDevParams( void *, vrpn_HANDLERPARAM  )
{
  return 0;
}

int nmm_Microscope_Simulator::
RcvQStdDevParams( void */*_userdata*/, vrpn_HANDLERPARAM /*_p*/ )
{
  return 0;
}

int nmm_Microscope_Simulator::
RcvSetScanStyle( void *, vrpn_HANDLERPARAM  )
{
   // Sent by microscape, but we do nothing.
  return 0;
}

int nmm_Microscope_Simulator::
RcvSetContactForce( void */*_userdata*/, vrpn_HANDLERPARAM /*_p*/ )
{
	// XXX haven't implemented yet
  return 0;
}

int nmm_Microscope_Simulator::
RcvQContactForce( void */*_userdata*/, vrpn_HANDLERPARAM /*_p*/ )
{
	// XXX haven't implemented yet
  return 0;
}

// This message handled by real Topo AFM
int nmm_Microscope_Simulator::
RcvSetRegionNM( void *_userdata, vrpn_HANDLERPARAM _p )
{

  nmm_Microscope_Simulator *tmp = (nmm_Microscope_Simulator *) _userdata;
  const char * bufptr = _p.buffer;

  int sr_rc;
  sr_rc = tmp->stm_set_region_nm_gradual( bufptr ); 
  switch( sr_rc ) {
	case -1:
		ServerError( "stm_set_region failed" );
		return -1;
    case 1:
		tmp->helpTimer2();		// HACK XXX Tiger
	break;
  }
  return 0;
}

// This message handled by real Topo AFM
int nmm_Microscope_Simulator::
RcvScanPointNM( void *_userdata, vrpn_HANDLERPARAM _p )
{
/**************************************************************************
 * Request to return the height at a specific point.
 *
 * Maybe we can check in the queue if there's another message
 * in the queue, and if so cancel the current seek of the point.
 **************************************************************************/

  nmm_Microscope_Simulator *tmp = (nmm_Microscope_Simulator *) _userdata;
  const char * bufptr = _p.buffer;
  mic_start = 0;  			// added by JakeK, supposed to stop the scanning of the
					// surface while user is doing a live touch
  if ( tmp->stm_scan_point_nm( bufptr ) ) {
    ServerError( "stm_scan_point_nm failed" );
    return -1;
  }

  return 0;
}

int nmm_Microscope_Simulator::
RcvSampleApproachNM( void */*_userdata*/, vrpn_HANDLERPARAM /*_p*/ )
{
  return 0;
}

// This message handled by real Topo AFM
int nmm_Microscope_Simulator::
RcvQueryScanRange( void *_userdata, vrpn_HANDLERPARAM )
{
  nmm_Microscope_Simulator *tmp = (nmm_Microscope_Simulator *) _userdata;
  //  const char * bufptr = _p.buffer;

  if ( tmp->spm_query_scan_range() ) {
     ServerError( "spm_query_scan_range failed" );
     return -1;
  }

  return 0;
}

// This message handled by real Topo AFM
int nmm_Microscope_Simulator::
RcvEcho( void *_userdata, vrpn_HANDLERPARAM _p )
{
  nmm_Microscope_Simulator *tmp = (nmm_Microscope_Simulator *) _userdata;
  const char * bufptr = _p.buffer;

  if ( tmp->spm_echo( bufptr ) ) {
     ServerError( "spm_echo failed" );
     return -1;
  }

  return 0;
}

        // Tiger        HACK HACK HACK  handling new message type for echo Modify Mode.
int nmm_Microscope_Simulator::
RcvMarkModify( void *_userdata, vrpn_HANDLERPARAM _p )
{
  nmm_Microscope_Simulator *tmp = (nmm_Microscope_Simulator *) _userdata;
  const char * bufptr = _p.buffer;

  if ( tmp->spm_echo_ModifyMode( bufptr ) ) {
     ServerError( "nmm_Microscope_Simulator::RcvMarkModify: spm_echo_ModifyMode failed" );
     return -1;
  }

  return 0;
}

        // Tiger        HACK HACK HACK  handling new message type for echo Image Mode.
int nmm_Microscope_Simulator::
RcvMarkImage( void *_userdata, vrpn_HANDLERPARAM _p )
{
  nmm_Microscope_Simulator *tmp = (nmm_Microscope_Simulator *) _userdata;
  const char * bufptr = _p.buffer;

  if ( tmp->spm_echo_ImageMode( bufptr ) ) {
     ServerError( "nmm_Microscope_Simulator::RcvMarkImage: spm_echo_ImageMode failed" );
     return -1;
  }

  return 0;
}


// This message handled by real Topo AFM
int nmm_Microscope_Simulator::
RcvZagPointNM( void *_userdata, vrpn_HANDLERPARAM _p )
{
  nmm_Microscope_Simulator *tmp = (nmm_Microscope_Simulator *) _userdata;
  const char * bufptr = _p.buffer;

  if ( tmp->stm_sweep_point_nm( bufptr ) ) {
      ServerError ("stm_sweep_point_nm failed");
      return -1;
  }

  return 0;
}

int nmm_Microscope_Simulator::RcvFeelTo (void * userdata,
                                         vrpn_HANDLERPARAM p) {
  nmm_Microscope_Simulator * tmn = (nmm_Microscope_Simulator *) userdata;
  const char * bufptr = p.buffer;
  vrpn_float32 x, y;
  int retval;

  retval = tmn->decode_BeginFeelTo(&bufptr, &x, &y);
  if (retval) {
    ServerError("decode_BeginFeelTo failed");
    return -1;
  }

  retval = tmn->afmFeelToPoint(x, y);
  if (retval) {
    ServerError("afmFeelToPoint failed");
    return -1;
  }

  return 0;
}

// This message handled by real Topo AFM
int nmm_Microscope_Simulator::
RcvSetMaxMove( void *_userdata, vrpn_HANDLERPARAM _p )
{
  nmm_Microscope_Simulator *tmp = (nmm_Microscope_Simulator *) _userdata;
  const char * bufptr = _p.buffer;

  if ( tmp->spm_set_max_move( bufptr ) ) {
     ServerError( "spm_set_max_move failed" );
     return -1;
  }

  return 0;
}

int nmm_Microscope_Simulator::
RcvSetStdDelay( void *, vrpn_HANDLERPARAM )
{
   // Sent by microscape, but we do nothing.
  return 0;
}

int nmm_Microscope_Simulator::
RcvSetStPtDelay( void *, vrpn_HANDLERPARAM  )
{
   // Sent by microscape, but we do nothing.

  return 0;
}

// This message handled by real Topo AFM
int nmm_Microscope_Simulator::
RcvSetRelax( void *_userdata, vrpn_HANDLERPARAM _p )
{
  nmm_Microscope_Simulator *tmp = (nmm_Microscope_Simulator *) _userdata;
  const char * bufptr = _p.buffer;

  if ( tmp->spm_set_relax( bufptr ) ) {
     ServerError( "spm_set_relax failed" );
     return -1;
  }

  return 0;
}

int nmm_Microscope_Simulator::
RcvMeasureResist( void */*_userdata*/, vrpn_HANDLERPARAM /*_p*/ )
{
  return 0;
}

int nmm_Microscope_Simulator::
RcvZigPointNM( void */*_userdata*/, vrpn_HANDLERPARAM /*_p*/ )
{
  return 0;
}

int nmm_Microscope_Simulator::
RcvSnapShot( void */*_userdata*/, vrpn_HANDLERPARAM /*_p*/ )
{
  return 0;
}

int nmm_Microscope_Simulator::
RcvSnapCancel( void */*_userdata*/, vrpn_HANDLERPARAM /*_p*/ )
{
  return 0;
}

// This message handled by real Topo AFM
// Not really. This is obsolete, so it never gets called.
int nmm_Microscope_Simulator::
RcvBluntPointNM( void *_userdata, vrpn_HANDLERPARAM _p )
{
  nmm_Microscope_Simulator *tmp = (nmm_Microscope_Simulator *) _userdata;
  const char * bufptr = _p.buffer;

  if ( tmp->spm_blunt_point_nm( bufptr ) ) {
     ServerError( "spm_blunt_point_nm failed" );
     return -1;
  }

  return 0;
}

// This message handled by real Topo AFM
int nmm_Microscope_Simulator::
RcvReqScanDataset( void *_userdata, vrpn_HANDLERPARAM _p )
{
  nmm_Microscope_Simulator *tmp = (nmm_Microscope_Simulator *) _userdata;
  const char * bufptr = _p.buffer;

  if ( tmp->spm_request_scan_datasets( bufptr ) ) {
     ServerError( "spm_request_scan_datasets failed" );
     return -1;
  }

  return 0;
}

// This message handled by real Topo AFM
int nmm_Microscope_Simulator::
RcvSetSlowScan( void *_userdata, vrpn_HANDLERPARAM _p )
{
  nmm_Microscope_Simulator *tmp = (nmm_Microscope_Simulator *) _userdata;
  const char * bufptr = _p.buffer;

  if ( tmp->spm_set_slow_scan( bufptr ) ) {
     ServerError( "spm_set_slow_scan failed" );
     return -1;
  }

  return 0;
}

// This message handled by real Topo AFM
int nmm_Microscope_Simulator::
RcvReqPotDataset( void *_userdata, vrpn_HANDLERPARAM _p )
{
  nmm_Microscope_Simulator *tmp = (nmm_Microscope_Simulator *) _userdata;
  const char * bufptr = _p.buffer;

  if ( tmp->spm_request_point_datasets( bufptr ) ) {
     ServerError( "spm_request_point_datasets failed" );
     return -1;
  }
  
  return 0;
}

// This message handled by real Topo AFM
// This is probably obsolete, since we change PID using
// EnterContactMode or EnterTappingMode....
int nmm_Microscope_Simulator::
RcvSetPID( void *_userdata, vrpn_HANDLERPARAM _p )
{
  nmm_Microscope_Simulator *tmp = (nmm_Microscope_Simulator *) _userdata;
  const char * bufptr = _p.buffer;

  if ( tmp->spm_set_pid( bufptr ) ) {
     ServerError( "spm_set_pid failed" );
     return -1;
  }

  return 0;
}

// This message handled by real Topo AFM
int nmm_Microscope_Simulator::
RcvSetRatenMeters( void *_userdata, vrpn_HANDLERPARAM _p )
{
  nmm_Microscope_Simulator *tmp = (nmm_Microscope_Simulator *) _userdata;
  const char * bufptr = _p.buffer;

  if ( tmp->stm_set_rate_nmeters(bufptr) ) {
    ServerError( "stm_set_rate_nmeters failed" );
    return -1;
  }

  return 0;
}

// This message handled by real Topo AFM
// Realated to Ohmmeter. Should be obsolete
int nmm_Microscope_Simulator::
RcvEnableVoltsource( void *_userdata, vrpn_HANDLERPARAM _p )
{
  nmm_Microscope_Simulator *tmp = (nmm_Microscope_Simulator *) _userdata;
  const char * bufptr = _p.buffer;

  if ( tmp->spm_enable_voltsource(bufptr) ) {
    ServerError( "spm_enable_voltsource failed" );
    return -1;
  }

  return 0;
}

// This message handled by real Topo AFM
// Realated to Ohmmeter. Should be obsolete
int nmm_Microscope_Simulator::
RcvDisableVoltsource( void *_userdata, vrpn_HANDLERPARAM _p )
{
  nmm_Microscope_Simulator *tmp = (nmm_Microscope_Simulator *) _userdata;
  const char * bufptr = _p.buffer;

  if ( tmp->spm_disable_voltsource(bufptr) ) {
    ServerError( "spm_disable_voltsource failed" );
    return -1;
  }

  return 0;
}

// This message handled by real Topo AFM
// Realated to Ohmmeter. Should be obsolete
int nmm_Microscope_Simulator::
RcvEnableAmp( void *_userdata, vrpn_HANDLERPARAM _p )
{
  nmm_Microscope_Simulator *tmp = (nmm_Microscope_Simulator *) _userdata;
  const char * bufptr = _p.buffer;

  if ( tmp->spm_enable_amp(bufptr) ) {
    ServerError( "spm_enable_amp failed" );
    return -1;
  }

  return 0;
}

// This message handled by real Topo AFM
// Realated to Ohmmeter. Should be obsolete
int nmm_Microscope_Simulator::
RcvDisableAmp( void *_userdata, vrpn_HANDLERPARAM _p )
{
  nmm_Microscope_Simulator *tmp = (nmm_Microscope_Simulator *) _userdata;
  const char * bufptr = _p.buffer;

  if ( tmp->spm_disable_amp(bufptr) ) {
    ServerError( "spm_disable_amp failed" );
    return -1;
  }

  return 0;
}

// This message handled by real Topo AFM
// Well, not really. This is something we want to do with Microscape,
// but we can't do it yet, so microscape never sends this message.
int nmm_Microscope_Simulator::
RcvSetRegAndAngle( void *_userdata, vrpn_HANDLERPARAM _p )
{
  nmm_Microscope_Simulator *tmp = (nmm_Microscope_Simulator *) _userdata;
  const char * bufptr = _p.buffer;

  int sr_rc;

  sr_rc = tmp->spm_set_region_angle_gradual( bufptr );

  switch( sr_rc ) {

	case -1:
		ServerError( "stm_set_region_angle failed" );
		return -1;

	case 1 :
		tmp->helpTimer2();			// HACK XXX Tiger
	break;
  }

  return 0;
}

int nmm_Microscope_Simulator::
RcvClientPacketTimestamp( void */*_userdata*/, vrpn_HANDLERPARAM /*_p*/ )
{
  return 0;
}

// This message handled by real Topo AFM
int nmm_Microscope_Simulator::
RcvSharpLine( void *_userdata, vrpn_HANDLERPARAM _p )
{
  nmm_Microscope_Simulator *tmp = (nmm_Microscope_Simulator *) _userdata;
  const char * bufptr = _p.buffer;

  if ( tmp->spm_sharp_line( bufptr ) ) {
     ServerError( "spm_sharp_line failed" );
     return -1;
  }

  return 0;
}

// This message handled by real Topo AFM
int nmm_Microscope_Simulator::
RcvSweepLine( void *_userdata, vrpn_HANDLERPARAM _p )
{
  nmm_Microscope_Simulator *tmp = (nmm_Microscope_Simulator *) _userdata;
  const char * bufptr = _p.buffer;

  if ( tmp->spm_sweep_line( bufptr ) ) {
     ServerError( "spm_sweep_line failed" );
     return -1;
  }

  return 0;
}

// This message handled by real Topo AFM
int nmm_Microscope_Simulator::
RcvSweepArc( void *_userdata, vrpn_HANDLERPARAM _p )
{
  nmm_Microscope_Simulator *tmp = (nmm_Microscope_Simulator *) _userdata;
  const char * bufptr = _p.buffer;

  if ( tmp->spm_sweep_arc( bufptr ) ) {
     ServerError( "spm_sweep_arc failed" );
     return -1;
  }

  return 0;
}

int nmm_Microscope_Simulator::
RcvClientHello( void */*_userdata*/, vrpn_HANDLERPARAM /*_p*/ )
{
  return 0;
}

// This message handled by real Topo AFM
int nmm_Microscope_Simulator::
RcvSewingMode( void *_userdata, vrpn_HANDLERPARAM _p )
{
  nmm_Microscope_Simulator *tmp = (nmm_Microscope_Simulator *) _userdata;
  const char * bufptr = _p.buffer;

  if ( tmp->afm_sewing_mode( bufptr ) ) {
     ServerError( "afm_sewing_mode failed" );
     return -1;
  }
 
  return 0;
}

// This message handled by real Topo AFM
int nmm_Microscope_Simulator::
RcvContactMode( void *_userdata, vrpn_HANDLERPARAM _p )
{
  nmm_Microscope_Simulator *tmp = (nmm_Microscope_Simulator *) _userdata;
  const char * bufptr = _p.buffer;

  if ( tmp->afm_contact_mode( bufptr ) ) {
     ServerError( "afm_contact_mode failed" );
     return -1;
  }

  return 0;
}

// This message handled by real Topo AFM
int nmm_Microscope_Simulator::
RcvTappingMode( void *_userdata, vrpn_HANDLERPARAM _p )
{
  nmm_Microscope_Simulator *tmp = (nmm_Microscope_Simulator *) _userdata;
  const char * bufptr = _p.buffer;

  if ( tmp->afm_tapping_mode( bufptr ) ) {
     ServerError( "afm_tapping_mode failed" );
     return -1;
  }
 
  return 0;
}







// This function called by real Topo AFM, indirectly
int nmm_Microscope_Simulator::
stm_set_grid_size( const char *bufptr )
{
/***************************************************************************
 *
 ***************************************************************************/

  long res;


        // Get the new size from the buffer
  if (decode_SetGridSize( &bufptr, &stm_grid_num_x, &stm_grid_num_y) == -1) {
     ServerOutputAdd( 2, "nmm_Microscope_Simulator::stm_set_grid_size:  Bad parameters passed for STM_SET_GRID_SIZE" );
     return -1;
  }
  
        // Make sure the parameters are reasonable
  if ( (stm_grid_num_x <= 0) || (stm_grid_num_y <= 0) ) {
    ServerOutputAdd( 2, "stm_set_grid_size: Illegal parameters passed for STM_SET_GRID_SIZE" );
    return -1;
  }

        // We can only do square images, so make a square with
        // the largest dimension. We should send a message back
        // to the UNIX side announcing this change( Original Comment! )
  if (stm_grid_num_x > stm_grid_num_y)
    res = stm_grid_num_x;
  else
    res = stm_grid_num_y;

        //XXX void SetResolutionRemotely (int res) is a function
        //defined in SCANTOPO.C:752
        //It sets the new resolution, updates the dialog box, recalculates
        //any parameters needed for the scan, and starts scanning again
  //  SetResolutionRemotely( res );
  ServerOutputAdd( 2, "stm_set_grid_size: Set grid cmd, size %d", res);

  return 0;
}

// This function called by real Topo AFM, indirectly
int nmm_Microscope_Simulator::
stm_scan_window( const char *bufptr )
{
/***************************************************************************
 * Starts scanning the area defined by the 4 parameters.
 * It seems the parameters are in terms of "grid units"
 * This function might then be used to scan a smaller region of the grid
 * BUT, the Topo AFM can only scan the whole grid, so I think the 
 * stm_window_* variables are probably ignored (they are set here, but
 * never used).
 ***************************************************************************/

        // stm_window_xmin and stm_window_ymin are protected member variables.

  //ServerOutputAdd( 2, "stm_scan_window: Scan wndw cmd rcvd" );

  if (decode_SetScanWindow( &bufptr, &stm_window_xmin, &stm_window_ymin,
	&stm_window_xmax, &stm_window_ymax ) == -1) {
    ServerOutputAdd( 2, "nmm_Microscope_Simulator::stm_scan_window: Too few parameters passed for STM_SCAN_WINDOW" );
    return -1;
  }

        // Make sure the parameters are reasonable
  if ( (stm_window_xmin < 0) ||
        (stm_window_ymin < 0) ||
        (stm_window_xmax > stm_grid_num_x-1) ||
        (stm_window_ymax > stm_grid_num_y-1) ) {
    ServerOutputAdd( 2, "stm_scan_window: Illegal parameters passed for STM_SCAN_WINDOW" );
    return -1;
  }
  ServerOutputAdd( 2, "stm_scan_window: Scan wndw set to (%d,%d) -> (%d, %d)",
		   stm_window_xmin, stm_window_ymin, 
		   stm_window_xmax, stm_window_ymax);
  
  // Start the new scan!!

  // Record that we are scanning.
  stm_current_mode = STM_SCAN_MODE;

  return 0;
}

// This function called by real Topo AFM, indirectly
int nmm_Microscope_Simulator::
stm_resume_window_scan()
{
/***************************************************************************
 * Starts scanning the last defined area from the top left corner.
 * The area to scan would be defined by a previous call to
 * stm_scan_window
 ***************************************************************************/

  ServerOutputAdd( 2, "stm_resume_window_scan: Resume window scan cmd rcvd" );

  // Start the new scan!!
  mic_start = 1;	// Microscape is ready to start scanning again
  currentline = 0;	// Current line to scan is 0th line

  // Record that we are scanning.
  stm_current_mode = STM_SCAN_MODE;
  return 0;
}

void nmm_Microscope_Simulator::
set_the_scan_rate( float speed )
{
/***************************************************************************
 * set_the_scan_rate is called each time the scanrate dialog box is changed
 * in the topometrix interface.
 ***************************************************************************/

  stm_nmeters_per_second = speed;
  return;
}

// This function called by real Topo AFM, indirectly
int nmm_Microscope_Simulator::
stm_set_rate_nmeters( const char *bufptr )
{
/***************************************************************************
 * This is just a modified version of stm_set_rate since the topometrix
 * sets the scan rate in terms of nm/second. We are providing an alternative
 * for the unix user to submit a scan rate that is also in nm/second
 * and not lines/second (the old method).
 ***************************************************************************/

  ServerOutputAdd( 2, "stm_set_rate_nmeters: Set rate cmd rcvd" );

  	// Get the new rate from the buffer
  if (decode_SetRateNM(&bufptr, &stm_nmeters_per_second) == -1) {
    ServerOutputAdd( 2, "nmm_Microscope_Simulator::stm_set_rate_nmeters: No parameter passed for STM_SET_RATE_NMETERS" );
    return -1;
  }

  ServerOutputAdd( 2, "Set rate cmd rcvd = %f", stm_nmeters_per_second );

  return 0;
}

// This function called by real Topo AFM, indirectly, during initialization.
int nmm_Microscope_Simulator::
spm_report_region_clipped( double xmin, double ymin,
				double xmax, double ymax )
/***********************************************************************
 * Buffers the new selected region. The region is "clipped" because
 * it wasn't the exact region that the client asked for, probably.
 ***********************************************************************/
{
  long len;
  char * msgbuf;
  int retval;

  float val_xmin = (float)xmin;
  float val_ymin = (float)ymin;
  float val_xmax = (float)xmax;
  float val_ymax = (float)ymax;

  ServerOutputAdd( 1, "nmm_Microscope_Simulator::spm_report_region_clipped: Reporting the scan region back to UNIX" );

  msgbuf = encode_SetRegionC( &len, val_xmin, val_ymin, val_xmax, val_ymax );
  if ( !msgbuf ) {
    ServerOutputAdd( 1, "nmm_Microscope_Simulator::spm_report_region_clipped:  Buffer overflow!!" );
    return -1;
  } else {
    retval = Send( len, d_SetRegionClipped_type, msgbuf );
    if ( retval ) {
      ServerOutputAdd( 1, "nmm_Microscope_Simulator::spm_report_region_clipped:  Couldn't pack message to send to client.\n");
      return -1;
    }
  }

  ServerOutputAdd (2, "X-min : %f", val_xmin);  // DEBUG
  ServerOutputAdd (2, "Y-min : %f", val_ymin);  // DEBUG
  ServerOutputAdd (2, "X-max : %f", val_xmax);  // DEBUG
  ServerOutputAdd (2, "Y-max : %f", val_ymax);  // DEBUG

  return 0;
}

// This function called by real Topo AFM, indirectly
int nmm_Microscope_Simulator::
stm_set_region_nm_gradual( const char *bufptr )
{
  ServerOutputAdd( 2, "stm_set_region_nm_gradual: Set region(nm) cmd rcvd" );

  if ( SetRegionReceived ) {	// Has already received the command so far?
    stm_region_xmin_work = stm_region_xmin;
    stm_region_ymin_work = stm_region_ymin;
    stm_region_xmax_work = stm_region_xmax;
    stm_region_ymax_work = stm_region_ymax;
  }
  else {
    stm_region_xmin_work = stm_region_xmin_actual;
    stm_region_ymin_work = stm_region_ymin_actual;
    stm_region_xmax_work = stm_region_xmax_actual;
    stm_region_ymax_work = stm_region_ymax_actual;
  }

	// Get the new size from the buffer
  if (decode_SetRegionNM(&bufptr, &stm_region_xmin, &stm_region_ymin,
			&stm_region_xmax, &stm_region_ymax) == -1) {
    ServerOutputAdd( 2, "nmm_Microscope_Simulator::stm_set_region_nm: Bad parameters passed for STM_SET_REGION_NM" );
    return -1;
 }

  SetRegionReceived = VRPN_TRUE;
  ReportAngle = VRPN_FALSE;

  ServerOutputAdd( 3, "X-min received : %f", stm_region_xmin);// DEBUG
  ServerOutputAdd (3, "Y-min received : %f", stm_region_ymin);// DEBUG
  ServerOutputAdd (3, "X-max received : %f", stm_region_xmax);// DEBUG
  ServerOutputAdd (3, "Y-max received : %f", stm_region_ymax);// DEBUG

  if ( !SetRegionReceived ) { // Is this the first time to process Set Region?

	// Tell topo about new scan range and let it do all the adjustments
                // XXX SetScanRangeRemotelyNew() defined in scantopo.c:269
//     SetScanRangeRemotelyNew( stm_region_xmin, stm_region_ymin,
// 			stm_region_xmax, stm_region_ymax );

	// Force the region to the smallest square that covers the selected
	// area. This should be returned from SetScanRangeRemotely
//     PostProcessScanRange();	// Defined in SERVFUNC.C
    SetRegionReceived = VRPN_TRUE;

    return 0;

   } else {

     isrStep = 0;
     stm_region_xmin_unit = (stm_region_xmin - stm_region_xmin_work)/SR_STEPS;
     stm_region_ymin_unit = (stm_region_ymin - stm_region_ymin_work)/SR_STEPS;
     stm_region_xmax_unit = (stm_region_xmax - stm_region_xmax_work)/SR_STEPS;
     stm_region_ymax_unit = (stm_region_ymax - stm_region_ymax_work)/SR_STEPS;

//      ProcessPendingScanRange();	// Defined in SERVFUNC.C

     return 1;
  }
}

// This function called by real Topo AFM, indirectly
int nmm_Microscope_Simulator::
spm_set_region_angle_gradual( const char */*bufptr*/ )
/********************************************************************
 * This procedure is equivalent to stm_set_region_nm except that
 * the new region is not set at one time but gradually moved.
 ********************************************************************/
{
  ServerOutputAdd( 2, "spm_set_region_angle_gradual: Set region(angle) cmd rcvd" );

  if ( SetRegionReceived ) { // Has already received the command so far?
    stm_region_xmin_work = stm_region_xmin;
    stm_region_ymin_work = stm_region_ymin;
    stm_region_xmax_work = stm_region_xmax;
    stm_region_ymax_work = stm_region_ymax;
  }
  else {
    stm_region_xmin_work = stm_region_xmin_actual;
    stm_region_ymin_work = stm_region_ymin_actual;
    stm_region_xmax_work = stm_region_xmax_actual;
    stm_region_ymax_work = stm_region_ymax_actual;
  }

        // Get the new size from the buffer
/* XXX HACK
  if ( (nmb_Util::Unbuffer( bufptr, stm_region_xmin ) == -1) ||
        (nmb_Util::Unbuffer( bufptr, stm_region_ymin ) == -1) ||
        (nmb_Util::Unbuffer( bufptr, stm_region_xmax ) == -1) ||
        (nmb_Util::Unbuffer( bufptr, stm_region_ymax ) == -1) ||
	(nmb_Util::Unbuffer( bufptr, stm_region_angle ) == -1) ) {
		// XXX SERVOUT
    ServerOutputAdd( 2, "stm_set_region_angle: Bad parameters passed for STM_SET_REGION_ANGLE" );
    return -1;
 }
*/

  SetRegionReceived = VRPN_TRUE;	//XXX
  ReportAngle = VRPN_TRUE;

  ServerOutputAdd( 3, "X-min received : %f", stm_region_xmin);// DEBUG
  ServerOutputAdd( 3, "Y-min received : %f", stm_region_ymin);// DEBUG
  ServerOutputAdd( 3, "X-max received : %f", stm_region_xmax);// DEBUG
  ServerOutputAdd( 3, "Y-max received : %f", stm_region_ymax);// DEBUG
  ServerOutputAdd( 3, "Angle received : %f", stm_region_angle);// DEBUG

  isrStep = 0;
  stm_region_xmin_unit = (stm_region_xmin - stm_region_xmin_work) / SR_STEPS;
  stm_region_ymin_unit = (stm_region_ymin - stm_region_ymin_work) / SR_STEPS;
  stm_region_xmax_unit = (stm_region_xmax - stm_region_xmax_work) / SR_STEPS;
  stm_region_ymax_unit = (stm_region_ymax - stm_region_ymax_work) / SR_STEPS;

//   ProcessPendingScanRange(); // Defined in SERVFUNC.C

	// rotate to the specified angle and update the display
	// XXX vSetImageRotation() defined in AFMSTM.C TOPOWN16.H TOPOWN32.C
//   vSetImageRotation( stm_region_angle );
	// XXX LoadScanDlgParameters() defined in SCANTOPO.C:1134
//   LoadScanDlgParameters();

  return 1;
}

int nmm_Microscope_Simulator::
ProcessPendingScanRange()
{
  ServerOutputAdd( 3, "Processing Scan Range Pending : Step %i", isrStep+1 );

  return 0;
}

void nmm_Microscope_Simulator::
PostProcessScanRange()
{
  ServerOutputAdd(2, "nmm_Microscope_Simulator::PostProcessScanRange() executing ...");

  ServerOutputAdd(2, "nmm_Microscope_Simulator::PostProcessScanRange() finish execution!");
}

// New message we might send to microscape, hasn't been implemented by
// microscape yet
int nmm_Microscope_Simulator::
spm_report_latest_angle(void)
{
  return spm_report_angle(stm_region_xmin, stm_region_ymin,
                                stm_region_xmax, stm_region_ymax);
}

// New message we might send to microscape, hasn't been implemented by
// microscape yet
int nmm_Microscope_Simulator::
spm_report_angle( float /*xmin*/, float /*ymin*/, float /*xmax*/, float /*ymax*/ )
{
/*************************************************************************
 * spm_report_latest_angle buffers the region and the angle it is rotated
 *************************************************************************/

  return 0;
}

// New message we might send to microscape, hasn't been implemented by
// microscape yet
int nmm_Microscope_Simulator::
spm_report_angle_clipped( float /*xmin*/, float /*ymin*/, float /*xmax*/, float /*ymax*/ )
{
/*************************************************************************
 * spm_report_angle_clipped buffers the region and the angle it is rotated
 *************************************************************************/

  return 0;
}

// This function called by real Topo AFM, indirectly
int nmm_Microscope_Simulator::
stm_scan_point_nm( const char *bufptr )
{
/*************************************************************************
 * Request to return the height at a specific point.
 *
 * Maybe we can check in the queue if there's another message
 * in the queue, and if so cancel the current seek of the point.
 *************************************************************************/

  //ServerOutputAdd( 2, "stm_scan_point: Scan point cmd rcvd" ); // DEBUG

	// Get the point from the buffer
  if (decode_ScanTo(&bufptr, &stm_desired_x, &stm_desired_y) == -1) {
    ServerOutputAdd( 2, "nmm_Microscope_Simulator::stm_scan_point: Too few parameters passed for STM_SCAN_POINT" );
    return -1;
  }

  if ( afm_mode == SEWING_MODE ) {
    if ( move_in_sewing_mode( stm_desired_x, stm_desired_y ) ) {
      ServerOutputAdd( 1, "***ERROR***: stm_scan_point_nm" );
      return -1;
    }
  }
  else if ( afm_mode == FORCECURVE_MODE ) {
         if ( move_in_forcecurve_mode( stm_desired_x, stm_desired_y ) ) {
           ServerOutputAdd( 1, "***ERROR***: stm_scan_point_nm" );
           return -1;
         }
  }
  else {
        if ( goto_point_and_report_it( stm_desired_x, stm_desired_y ) ) {
          ServerOutputAdd( 1, "***ERROR***: stm_scan_point_nm" );
          return -1;
        }
  }

  stm_current_mode = STM_IDLE_MODE;
  return 0;
}

int nmm_Microscope_Simulator::afmFeelToPoint (vrpn_float32 x, vrpn_float32 y) {

  int retval;
  int i, j;
  double nx, ny;

  double incx = 5;
  double incy = 5;

  ServerOutputAdd(1, "afmFeelToPoint %.2f %.2f", x, y);

  sendBeginFeelTo(x, y);
  for (i = 0, nx = x - 2 * incx; i < 5; i++, nx += incx) {
    for (j = 0, ny = y - 2 * incy; j < 5; j++, ny += incy) {
      retval = goto_point_and_report_it(nx, ny);
      if (retval) {
        ServerOutputAdd(1, "***ERROR***:  afmFeelToPoint");
      }
    }
    // Hackish "backscan"
    spm_goto_xynm(nx, y - 2 * incy);
  }
  sendEndFeelTo(x, y);

  return 0;
}


// This function called by real Topo AFM, indirectly
int nmm_Microscope_Simulator::
stm_sweep_point_nm( const char *bufptr )
{
/************************************************************************
 * Request to sweep to and return the height at a specific point.
 * This implements the "ZAG_MODE" command request, which is the 
 * virtual whisk broom tool.
 ************************************************************************/

  float sweep_angle;     // Angle that sweep goes away from desired(x,y)
  float sweep_len;       // Length of the sweep
  float sweep_max_move;  // Maximum movement of sweep (XXX ignored: should
			 // become rate in another command)

	// Get the point from the buffer
  if (decode_ZagTo( &bufptr, &stm_desired_x, &stm_desired_y, &sweep_angle,
			&sweep_len, &sweep_max_move) == -1) {
     ServerOutputAdd( 2, "nmm_Microscope_Simulator::stm_sweep_point: Too few parameters passed for STM_ZAG_POINT_NM" );
     return -1;
  }

  spm_stop_scan();


	// Set the maximum linear motion to be faster than for scanning,
	// remember the value that we should put back
		// XXX SetScanRateRemotelyNMeters() defined in scantopo.c:369
//     SetScanRateRemotelyNMeters(stm_nmeters_per_second*10);

	// Do the sweep step
  if (spm_sweep_step(stm_desired_x, stm_desired_y, sweep_angle, sweep_len)==-1)
  {
                // XXX SetScanRateRemotelyNMeters() defined in scantopo.c:369
//       SetScanRateRemotelyNMeters(stm_nmeters_per_second);
      return -1;
  }

	// Set the maximum linear motion back to what it is normally
                // XXX SetScanRateRemotelyNMeters() defined in scantopo.c:369
//   SetScanRateRemotelyNMeters(stm_nmeters_per_second);

  stm_current_mode = STM_IDLE_MODE;

  return 0;
}

// This function called by real Topo AFM, indirectly
int nmm_Microscope_Simulator::
spm_query_scan_range( void )
{
/************************************************************************
 * Return min and max values in nm for the scan ranges along each axis.
 * IMPORTANT: For the moment, I'm assuming that the minimum is always 0,
 * but this is not necessary true.
 ************************************************************************/

  long len;
  char * msgbuf;
  int retval;

  float val_xmin, val_xmax, val_ymin, val_ymax, val_zmin, val_zmax;
  ServerOutputAdd (2, "spm_query_scan_range: Query scan range cmd rcvd");

  	// Get the range values
  val_xmin = val_ymin = val_zmin = 0;
  val_xmax = val_ymax = val_zmax = 10000; // 10 microns - reasonable fake value.

  // Fill in values....

  msgbuf = encode_ScanRange( &len, val_xmin, val_xmax, val_ymin, val_ymax,
					val_zmin, val_zmax );
  if ( !msgbuf ) {
    ServerOutputAdd( 2, "nmm_Microscope_Simulator::spm_query_scan_range:  Buffer overflow" );
    return -1;
  } else {
    retval = Send( len, d_ScanRange_type, msgbuf );
    if ( retval ) {
      ServerOutputAdd( 2, "nmm_Microscope_Simulator::spm_query_scan_range:  Couldn't pack message to send to client." );
      return -1;
    }
  }

  return 0;
}

// This function called by real Topo AFM, indirectly
int nmm_Microscope_Simulator::
spm_set_max_move( const char *bufptr )
{
   /* stm_max_move is set by this function, but it is not clear 
    * that the value is ever needed or used by the AFM. */
  ServerOutputAdd (2, "spm_set_max_move: Set max move cmd rcvd");

	// Get the new max move from the the buffer
  if (decode_SetMaxMove(&bufptr, &stm_max_move) == -1) {
     ServerOutputAdd (2, "nmm_Microscope_Simulator::spm_set_max_move: No parameter passed for STM_SET_RATE" );
     return -1;
  }

	// Make sure the parameters are reasonable
  if ( stm_max_move < 0.0 ) {
     ServerOutputAdd( 2, "spm_set_max_move: Illegal parameter passed for STM_SET_RATE" );
     return -1;
  }

  ServerOutputAdd( 2, "spm_set_max_move: Max move dist is %f", stm_max_move );
  return 0;
}



// This function called by real Topo AFM, indirectly
int nmm_Microscope_Simulator::
spm_set_relax( const char *bufptr )
{
/************************************************************************
 * The unit of the RelaxDelay is millisecond
 * RelaxDelay is set here,but it is not used until later, in the 
 * "time_to_relax" function
 ************************************************************************/

  long len;
  char * msgbuf;
  int retval;

  vrpn_int32 tmp_long;

  ServerOutputAdd( 2, "spm_set_relax: set relax delays cmd rcvd" );

  if (decode_SetRelax(&bufptr, &RelaxDelay, &tmp_long) == -1) {
    ServerOutputAdd (2, "nmm_Microscope_Simulator::spm_set_relax: Bad parameters passed for SPM_SET_RELAX");
    return -1;
  }
  ServerOutputAdd(2, "spm_set_relax: RelaxDelay = %d\t tmp_long = %d\n", RelaxDelay, tmp_long);

  RelaxDelay += tmp_long;
  if ( RelaxDelay < 0 ) {
    ServerOutputAdd (2, "spm_set_relax: Negative delay times passed for SPM_SET_RELAX");
    return -1;
  }

  ServerOutputAdd(2, "spm_set_relax: RelaxDelay-tmp_long = %d\t tmp_long = %d\n", RelaxDelay-tmp_long, tmp_long);

  msgbuf = encode_RelaxSet( &len, RelaxDelay-tmp_long, tmp_long );
  if ( !msgbuf ) {
    ServerOutputAdd( 2, "nmm_Microscope_Simulator::spm_set_relax:  Buffer overflow!!" );
    return -1;
  } else {
    retval = Send( len, d_RelaxSet_type, msgbuf );
    if ( retval ) {
      ServerOutputAdd( 2, "nmm_Microscope_Simulator::spm_set_relax:  Couldn't pack message to send to client." );
      return -1;
    }
  }

	// need to send out a height after RelaxDelay to finish
	// calibration. Safe to assume that at least one will
	// be sent out in the 50ms after RelaxDelay
  RelaxDelay += 50;

  return 0;
}

int nmm_Microscope_Simulator::
spm_blunt_point_nm( const char */*bufptr*/ )
{
/*****************************************************************
 * Sample height at three points around (x,y) forming an equilateral
 * triangle, return average height and plane normal.
 * Distance is the distance between (x,y) and the points to be
 * sampled.
 * This is somewhat similar to SPM_ZIG_POINT_NM in the old server.
 *****************************************************************/

  stm_current_mode = STM_IDLE_MODE;
  return 0;
}

// This function called by real Topo AFM, indirectly
int nmm_Microscope_Simulator::
spm_request_scan_datasets( const char* bufptr )
{
/**************************************************************************
 * This command specifies which data sets are to be collected when
 * scanning the surface.
 **************************************************************************/

  int i;
  char name[64];
  vrpn_int32 numsets;

  ServerOutputAdd( 2, "spm_request_scan_datasets: request data sets cmd rcvd");

  if (decode_GetNewScanDatasetHeader(&bufptr, &numsets) == -1) {
    ServerOutputAdd( 2, "nmm_Microscope_Simulator::spm_request_scan_datasets: # of data sets has not been passed" );
    return -1;
  }

  if ( numsets > MAX_REQUEST_DATA_SETS ) {
    ServerOutputAdd( 2, "nmm_Microscope_Simulator::spm_request_scan_datasets: Too many data sets" );
    return -1;
  }
  ServerOutputAdd( 3, "# of data sets requested: %i", (int)numsets );

  for (i=0;i<numsets;i++) {
		// XXX ??? maybe should be as following, not sure
      if (decode_GetNewScanDataset(&bufptr, name) == -1) {
         ServerOutputAdd (2, "nmm_Microscope_Simulator::spm_request_scan_datasets: data sets %i has not been passed", i);
      return -1;
      }

      ServerOutputAdd (3, "Data sets %i : %s", i, name);
  }

  //ServerOutputAdd (1, "Calling ApplyRequestDataSets");
		// XXX ApplyRequestedDataSets() defined in SETACQ.C: 831
//   ApplyRequestedDataSets(hDlgSetupAcquire, &request_sets);

  return 0;
}


///////////////////////////////////////////////////////////////////////////
// JakeK and AFMS team. Chagned so that it always looks for Z data when trying
// to do a touch live
///////////////////////////////////////////////////////////////////////////////


// This function called by real Topo AFM, indirectly
int nmm_Microscope_Simulator::
spm_request_point_datasets( const char *bufptr )
{
/***************************************************************************
 * This command specifies which data sets are to be collected when the
 * user is touching or modifying the surface - i.e. when the user is
 * directly controlling the tip.
 ***************************************************************************/

  int i;
  char name[64];
  vrpn_int32 numsets;
  vrpn_int32 num_samples;
//  numsets = 1;		// Added by JakeK

  ServerOutputAdd(2, "spm_request_point_datasets: request data sets cmd rcvd");

  if (decode_GetNewPointDatasetHeader(&bufptr, &numsets) == -1) {
     ServerOutputAdd( 2, "nmm_Microscope_Simulator::spm_request_point_datasets: # of data sets has not been passed" );
     return -1;
  }

		// MAX_REQUEST_POINT_SETS define in servdata.h
  if (numsets > MAX_REQUEST_POINT_SETS) {
     ServerOutputAdd (2, "spm_request_point_datasets: Too many data sets");
     return -1;
  }
  ServerOutputAdd( 2, "nmm_Microscope_Simulator::spm_request_point_datasets(): # of data sets requested: %i", (int)numsets );

  for (i=0;i<numsets;i++) {
      if (decode_GetNewPointDataset(&bufptr, name,
				&num_samples) == -1) {
         ServerOutputAdd( 2, "nmm_Microscope_Simulator::spm_request_point_datasets: data sets %i has not been passed", i );
      return -1;
      }

      ServerOutputAdd( 2, "Point Data sets %i : %s", i, name );
  }

  //int retval;
  char * msgbuf;
  long len;
  msgbuf = encode_PointDataset(&len, 1, "Topography", "nm", 0, 1);

		// XXX UpdatePointDataSets() defined in servdata.h .c
//   UpdatePointDataSets(&request_psets);

	// Update the latest data sets and inform the datasets to the client
		// XXX update_current_dataset() defined in servdata.h .c
//   update_current_dataset();
  return 0;
}

// This function called by real Topo AFM, indirectly
int nmm_Microscope_Simulator::
spm_set_pid( const char *bufptr )
{
/*************************************************************************
 * This responds to the server's request to change the PID
 *************************************************************************/

  float p, i, d;
  ServerOutputAdd (1, "spm_set_pid : request to set pid is being processed");

  if (decode_PidParameters(&bufptr, &p, &i, &d) == -1) {
     ServerOutputAdd( 2, "nmm_Microscope_Simulator::spm_set_pid: buffer overflow" );
     return -1;
  }
  ServerOutputAdd( 2, "spm_set_pid:  p: %f, i: %f, d: %f", p , i, d );

  return 0;
}

// This function called by real Topo AFM, indirectly
int nmm_Microscope_Simulator::
afm_sewing_mode( const char *bufptr )
{
/************************************************************************
 * This command unpacks the information needed for sewing machine mode.
 ************************************************************************/

  long len;
  char * msgbuf;
  int retval;

  float bot_delay, top_delay;

  if (decode_EnterSewingStyle(&bufptr, &afm_sm_set_point, &bot_delay,
		&top_delay, &afm_sm_pull_dist, &afm_sm_move_dist,
		&afm_sm_rate, &afm_sm_max_close_nm) == -1) {
     ServerOutputAdd( 2, "nmm_Microscope_Simulator::afm_sewing_mode: No parameters passed to afm_sewing_mode" );
     return -1;
  }

  ServerOutputAdd( 1, "afm_sewing_mode: Set sewing mode cmd rcvd!" );

  ServerOutputAdd (2, "afm_sewing_mode: Set Point: %f", afm_sm_set_point);
  ServerOutputAdd (2, "afm_sewing_mode: Modified Bottom Delay: %f", afm_sm_bot_delay);
  ServerOutputAdd (2, "afm_sewing_mode: Modified Top Delay: %f", afm_sm_top_delay);
  ServerOutputAdd (2, "afm_sewing_mode: Original Bottom Delay: %f", bot_delay);
  ServerOutputAdd (2, "afm_sewing_mode: Original Top Delay: %f", top_delay);
  ServerOutputAdd (2, "afm_sewing_mode: Pull Dist: %f", afm_sm_pull_dist);
  ServerOutputAdd (2, "afm_sewing_mode: Move Dist: %f", afm_sm_move_dist);
  ServerOutputAdd (2, "afm_sewing_mode: Rate: %f", afm_sm_rate);
  ServerOutputAdd (2, "afm_sewing_mode: Max Close: %f", afm_sm_max_close_nm);
  ServerOutputAdd (2, "afm_sewing_mode: New Max Close: %f", afm_sm_max_close_nm);

  msgbuf = encode_InSewingStyle( &len, afm_sm_set_point, bot_delay, top_delay,
		afm_sm_pull_dist, afm_sm_move_dist, afm_sm_rate,
		afm_sm_max_close_nm );
  if ( !msgbuf ) {
    ServerOutputAdd( 3, "nmm_Microscope_Simulator::afm_sewing_mode:  Buffer overflow!!" );
    return -1;
  } else {
    retval = Send( len, d_InSewingStyle_type, msgbuf );
    if ( retval ) {
      ServerOutputAdd( 3, "nmm_Microscope_Simulator::afm_sewing_mode:  Couldn't pack message to send to client." );
      return -1;
    }
  }

	// make the first punch at the last point we were at (think of this as
	// "pen down"). The move_and_punch() function sets the last punch
	// location as a side effect. After this is done, one punch will have
	// occurred and the "prev_point" will be the same as the 
	// "last_punch" point.
  if (move_and_punch(afm_prev_point_x,afm_prev_point_y)) {
    ServerOutputAdd(2, "***ERROR*** : afm_sewing_mode : Error in punch");
    return -1;
  }

  return 0;
}

// This function called by real Topo AFM, indirectly
int nmm_Microscope_Simulator::
afm_contact_mode( const char *bufptr )
{
/*************************************************************************
 * This is similar to afm_set_mod_force except it unbuffers values for PID
 * and the Set Point and assigns the PID and Set Point these values
 *************************************************************************/

  float p, i, d, setpt;
  //  int cur_afm_mode = afm_mode;

  ServerOutputAdd( 1, "afm_contact_mode: started to process" );

  if (decode_EnterContactMode(&bufptr, &p, &i, &d, &setpt) == -1) {
     ServerOutputAdd( 2, "nmm_Microscope_Simulator::afm_contact_mode: failed to receive parameters" );
     return -1;
  }
  ServerOutputAdd(2, "afm_contact_mode: P: %f, I: %f, D: %f", p, i, d);
  ServerOutputAdd(2, "afm_contact_mode: Set Point: %f", setpt);
  set_point = setpt;	// Added by JakeK for midify
  afm_mode = CONTACT_MODE;

  return spm_report_contact_mode();
}

// This function called by real Topo AFM, indirectly
int nmm_Microscope_Simulator::
afm_tapping_mode( const char *bufptr )
{
/***************************************************************************
 * This is similar to afm_set_img_force() except it takes unbuffers values
 * for the PID, the Set Point and the Drive Amplitude and assigns the PID,
 * the Set Point and the Drive Amplitude these values
 ***************************************************************************/

  float p, i, d, setpt, driveAmp;
  //  int cur_afm_mode = afm_mode;

  ServerOutputAdd(1, "afm_tapping_mode: started to process");

  if (decode_InTappingMode(&bufptr, &p, &i, &d, &setpt, &driveAmp) == -1) {
     ServerOutputAdd( 2, "nmm_Microscope_Simulator::afm_tapping_mode: failed to receive parameters" );
     return -1;
  }
  ServerOutputAdd(2, "afm_tapping_mode: P: %f, I: %f, D: %f", p, i, d);
  ServerOutputAdd(2, "afm_tapping_mode: Set Point: %f", setpt);
  ServerOutputAdd(2, "afm_tapping_mode: Drive Amplitude: %f", driveAmp);
  set_point = setpt;		// Added by JakeK for modify
  afm_mode = TAPPING_MODE;

  return spm_report_tapping_mode();
}

// This function called by real Topo AFM, indirectly
int nmm_Microscope_Simulator::
spm_echo( const char */*bufptr*/ )
{
/***************************************************************************
 * spm_echo simply unbuffers a long that describes the number of 4 byte
 * packets that will be coming in, then attempts to unbuffer those packets
 * and sends everything right back (rebuffers all the information).
 ***************************************************************************/

  return 0;
}

int nmm_Microscope_Simulator::
spm_echo_ModifyMode( const char * /* bufptr */ )
{
  long len = 0;
  char * msgbuf = NULL;
  int retval;

    printf("sending InModMode message\n");
    retval = Send( len, d_InModMode_type, msgbuf );
    if ( retval ) {
      ServerOutputAdd( 2, "nmm_Microscope_Topometrix::spm_echo_ModifyMode:  Couldn't pack message to send to client." );
      return -1;
        }
 // }

  return 0;
}

int nmm_Microscope_Simulator::
spm_echo_ImageMode( const char * /* bufptr */ )
{
  long len = 0;
  char * msgbuf = NULL;
  int retval;

  // NANO BEGIN         DEBUGGING
  ServerOutputAdd(2, "nmm_Microscope_Simulator::spm_echo_ImageMode(): Client want to go into Image mode!!!");
  // NANO END

  // If we were just in Forcecurve Style or Sewing Style,
  // we will get this message before we have re-enabled
  // param_reporting. So I am going to comment this out. 9/22/99
  // I did a test, and it doesn't seem to affect line mode...
  //if (!UpdateFeedbackParamsNow) {  // HACK HACK HACK  don't want to interrupt line mode
  //ServerOutputAdd(2, "nmm_Microscope_Topometrix::spm_echo_ImageMode: Line mode is reporting, cannot interrupt!!!");
  //} else {
  /*      msgbuf = encode_MarkImage(&len);
        if ( !msgbuf ) {
                ServerOutputAdd( 2, "nmm_Microscope_Simulator::spm_echo_ImageMode: Buffer overflow!!" );
                return -1;
        } else {
*/
                retval = Send( len, d_InImgMode_type, msgbuf );
                if ( retval ) {
                        ServerOutputAdd( 2, "nmm_Microscope_Simulator::spm_echo_ImageMode:  Couldn't pack message to send to client." );
                        return -1;
                }
 //       }
        //}

  return 0;
}


// This function called by real Topo AFM, indirectly
int nmm_Microscope_Simulator::
spm_enable_voltsource( const char *bufptr )
{
/***************************************************************************
 * spm_enable_voltsource unbuffers the id number for a voltage source and
 * sets the voltage output to another value that is unbuffered. Then all
 * the information is sent back to the client.
 ***************************************************************************/

  long len;
  char * msgbuf;
  int retval;

  vrpn_int32 device_num;
  vrpn_float32 voltage;

  if (decode_EnableVoltsource(&bufptr, &device_num, &voltage) == -1) {
     ServerOutputAdd(2, "nmm_Microscope_Simulator::spm_enable_voltsource: bad parameters passed to spm_enable_voltsource");
     return -1;
  }


  msgbuf = encode_VoltsourceEnabled( &len, device_num, voltage );
  if ( !msgbuf ) {
    ServerOutputAdd( 2, "nmm_Microscope_Simulator::spm_enable_voltsource:  Buffer overflow!!" );
    return -1;
  } else {
    retval = Send( len, d_VoltsourceEnabled_type, msgbuf );
    if ( retval ) {
      ServerOutputAdd( 2, "nmm_Microscope_Simulator::spm_enable_voltsource:  Couldn't pack message to send to client." );
      return -1;
    }
  }

  return 0;
}

// This function called by real Topo AFM, indirectly
int nmm_Microscope_Simulator::
spm_disable_voltsource( const char *bufptr )
{
/**************************************************************************
 * spm_disable_voltsource unbuffers an id of a voltage source that is to be
 * disabled, disables it and then reports that it has been disabled.
 **************************************************************************/

  long len;
  char * msgbuf;
  int retval;

  vrpn_int32 device_num = 0;

  if (decode_DisableVoltsource(&bufptr, &device_num) == -1) {
     ServerOutputAdd(2, "nmm_Microscope_Simulator::spm_disable_voltsource: bad parameters passed to spm_disable_voltsource");
     return -1;
  }


  msgbuf = encode_VoltsourceDisabled( &len, device_num );
  if ( !msgbuf ) {
    ServerOutputAdd( 2, "nmm_Microscope_Simulator::spm_disable_voltsource:  Buffer overflow!!" );
    return -1;
  } else {
    retval = Send( len, d_VoltsourceDisabled_type, msgbuf );
    if ( retval ) {
      ServerOutputAdd( 2, "nmm_Microscope_Simulator::spm_disable_voltsource:  Couldn't pack message to send to client." );
      return -1;
    }
  }

  return 0;
}

// This function called by real Topo AFM, indirectly
int nmm_Microscope_Simulator::
spm_enable_amp( const char *bufptr )
{
/**************************************************************************
 * spm_enable_amp unbuffers the id number for an Amp and sets the offset
 * and the gain Gain to values that are unbuffered. Then all the information
 * is sent back to the client.
 **************************************************************************/

	// XXX this function seems not complete yet

  long len;
  char * msgbuf;
  int retval;

  vrpn_int32 device_num, gain_mode;
  float offset, percent_offset;

  if (decode_EnableAmp(&bufptr, &device_num, &offset,
			&percent_offset, &gain_mode) == -1) {
     ServerOutputAdd(2, "nmm_Microscope_Simulator::spm_enable_amp: bad parameters passed to spm_enable_amp");
     return -1;
  }

  ServerOutputAdd(1, "*** Not enabling the amplifier -- it blows up ASU system");

  msgbuf = encode_AmpDisabled( &len, device_num );
  if ( !msgbuf ) {
    ServerOutputAdd( 2, "nmm_Microscope_Simulator::spm_enable_amp:  Buffer overflow!!" );
    return -1;
  } else {
    retval = Send( len, d_AmpDisabled_type, msgbuf );
    if ( retval ) {
      ServerOutputAdd( 2, "nmm_Microscope_Simulator::spm_enable_amp:  Couldn't pack message to send to client." );
      return -1;
    }
  }

  return 0;
}

// This function called by real Topo AFM, indirectly
int nmm_Microscope_Simulator::
spm_disable_amp( const char *bufptr )
{
/**************************************************************************
 * spm_disable_amp unbuffers an id of a voltage source that is to be
 * disabled, disables it and then reports that it has been disabled.
 **************************************************************************/

  long len;
  char * msgbuf;
  int retval;

  vrpn_int32 device_num = 0;

  if (decode_DisableAmp(&bufptr, &device_num) == -1) {
     ServerOutputAdd(2, "nmm_Microscope_Simulator::spm_disable_amp: bad parameters passed to spm_disable_amp");
     return -1;
  }


  msgbuf = encode_AmpDisabled( &len, device_num );
  if ( !msgbuf ) {
    ServerOutputAdd( 2, "nmm_Microscope_Simulator::spm_disable_amp: Buffer overflow!!" );
    return -1;
  } else {
    retval = Send( len, d_VoltsourceDisabled_type, msgbuf );
    if ( retval ) {
      ServerOutputAdd( 2, "nmm_Microscope_Simulator::spm_disable_amp:  Couldn't pack message to send to client." );
      return -1;
    }
  }

  return 0;
}

// This function called by real Topo AFM, indirectly
int nmm_Microscope_Simulator::
spm_set_slow_scan( const char *bufptr )
{
/**************************************************************************
 * spm_set_slow_scan -- in this function, slow scan does not refer to the
 * scan rate. It refers to line scan. If the command is sent that calls
 * this function, it means that someone on the client side wants line scan
 * to either be on or off (depending on the parameter that is passed).
 **************************************************************************/

  long len;
  char * msgbuf;
  int retval;

  vrpn_int32 slow_scan;
  //  int state;

  if (decode_SetSlowScan(&bufptr, &slow_scan) == -1) {
     ServerOutputAdd (2, "nmm_Microscope_Simulator::spm_set_slow_scan: failed to unbuffer parameter.");
     return -1;
  }

	// slow_scan will be 0 if we are supposed to start line scan and
	// 1 if we shoudl stop
  if (!slow_scan) {
	spm_stop_scan();
	// if scanning, suspend the scan
  }
  else {
	// Otherwise, restart the scan. 
  }
  // Send a confirmation message back. It should always (?) contain a 1
  msgbuf = encode_ReportSlowScan( &len, 1 );
  if ( !msgbuf ) {
    ServerOutputAdd( 2, "nmm_Microscope_Simulator::spm_set_slow_scan:  Buffer overflow!!" );
    return -1;
  } else {
    retval = Send( len, d_ReportSlowScan_type, msgbuf );
    if ( retval ) {
      ServerOutputAdd( 2, "nmm_Microscope_Simulator::spm_set_slow_scan:  Couldn't pack message to send to client." );
      return -1;
    }
  }

  return 0;
}		

// This function called by real Topo AFM, indirectly
int nmm_Microscope_Simulator::
spm_sharp_line( const char *bufptr )
{
/**************************************************************************
 * Go from one endpoint to the other in the current mode (contact or tapping,
 * but not sewing). The way this should be used by the client is to move to
 * the start point using stm_scan_point_nm, then set to modification force,
 * then issue the spm_sharp_line command to move to the far endpoint,
 * returning results along the way.
 * This routine returns starting at the starting point going in steps of
 * "spacing" units until nearer than spacing to the end, then stepping to
 * the end point.
 **************************************************************************/

  vrpn_float32   startx, starty;
  vrpn_float32   endx,endy;
  vrpn_float32   spacing;			// Distance between samples.

	// Read the parameters
  if (decode_DrawSharpLine(&bufptr, &startx, &starty,
			&endx, &endy, &spacing) == -1) {
     ServerOutputAdd (2, "nmm_Microscope_Simulator::spm_sharp_line: Too few parameters passed for SPM_SHARP_LINE");
     return -1;
  }

  //  ServerOutputAdd(2, "spm_sharp_line(%f,%f - %f,%f; %f)",startx,starty, endx,endy, spacing);

	// Check the parameters
  if ( (spacing == 0) || ( (startx==endx) && (starty==endy) ) ) {
     ServerOutputAdd( 1, "***ERROR***: spm_sharp_line(): Bad parameters" );
     return -1;
  }

  if (afm_mode == SEWING_MODE) {
     ServerOutputAdd(1,"***ERROR***: spm_sharp_line(): Can't do this in sewing mode!");
     return -1;
  }

	// Do all steps from the start location to right
	// before the end location
  {
     float   dx = endx-startx;
     float   dy = endy-starty;
     float   dist = (float)sqrt( dx*dx + dy*dy );
     int     count, numsteps = (int)floor( dist / spacing );
     float   stepx = dx / dist * spacing;
     float   stepy = dy / dist * spacing;

     for (count = 0; count < numsteps; count++) {
         if (goto_point_and_report_it(startx + count*stepx, starty + count*stepy)) {
            ServerOutputAdd(1,"**ERROR**: spm_sharp_line can't get to a point");

            return -1;
         }
     }
  }

	// Do the last step at the end point
  if (goto_point_and_report_it(endx,endy)) {
     ServerOutputAdd(1,"**ERROR**: spm_sharp_line can't get to the last point");

     return -1;
  }

  stm_current_mode = STM_IDLE_MODE;
  return 0;
}

// This function called by real Topo AFM, indirectly
int nmm_Microscope_Simulator::
spm_sweep_line( const char *bufptr )
{
/**************************************************************************
 * Request to sweep to and return the height along a specific line. This
 * implements the "ZAG_MODE" command request, which is the virtual whisk
 * broom tool.
 * The application should do a normal point to get to the beginning of the
 * line, set to modification mode, then call this routine with the current
 * location as the line starting position.
 * The angle and length are linearly interpolated between the endpoints.
 * This routine returns results starting at the starting point going in
 * steps of "spacing" units until nearer than spacing to the end, then
 * stepping to the end point.
 *************************************************************************/

  float start_x, start_y, start_angle, start_len;
  float end_x, end_y, end_angle, end_len;
  float spacing;

	// Get the point from the buffer
  if (decode_DrawSweepLine(&bufptr, &start_x, &start_y, &start_angle,
				&start_len, &end_x, &end_y, &end_angle,
				&end_len, &spacing) == -1) {
     ServerOutputAdd (2, "nmm_Microscope_Simulator::stm_sweep_point: Too few parameters passed for STM_SWEEP_LINE");
     return -1;
  }

  //  ServerOutputAdd(2, "spm_sweep_line(%f,%f - %f,%f; %f)",start_x,start_y, end_x,end_y, spacing);

	// Check the parameters
  if ( (spacing == 0) || ( (start_x == end_x) && (start_y == end_y) ) ) {
     ServerOutputAdd(1,"***ERROR***: spm_sharp_line(): Bad parameters");
     return -1;
  }

  spm_stop_scan();

	// Set the maximum linear motion to be faster than for scanning,
	// remember the value that we should put back
                // XXX SetScanRateRemotelyNMeters() defined in scantopo.c:369
//   SetScanRateRemotelyNMeters(stm_nmeters_per_second*10);

	// Do all steps from the start location to right
	// before the end location.
  {
     float   dx = end_x-start_x;
     float   dy = end_y-start_y;
     float   dangle = end_angle - start_angle;
     float   dlen = end_len - start_len;
     float   dist = (float)sqrt( dx*dx + dy*dy );
     int     count, numsteps = (int)floor( dist / spacing );
     float   stepx = dx / dist * spacing;
     float   stepy = dy / dist * spacing;
     float   stepangle = dangle / numsteps;
     float   steplen = dlen / numsteps;

     for (count = 0; count < numsteps; count++) {
         if (spm_sweep_step( start_x + count*stepx, start_y + count*stepy,
             start_angle + count*stepangle, start_len + count*steplen) == -1) {
                // XXX SetScanRateRemotelyNMeters() defined in scantopo.c:369
//             SetScanRateRemotelyNMeters(stm_nmeters_per_second);
            return -1;
         }
     }
  }

	// Do the last sweep step
  if (spm_sweep_step( end_x, end_y, end_angle, end_len) == -1) {
                // XXX SetScanRateRemotelyNMeters() defined in scantopo.c:369
//      SetScanRateRemotelyNMeters(stm_nmeters_per_second);
     return -1;
  }

	// Set the maximum linear motion back to what it is normally
                // XXX SetScanRateRemotelyNMeters() defined in scantopo.c:369
//   SetScanRateRemotelyNMeters(stm_nmeters_per_second);

  stm_current_mode = STM_IDLE_MODE;
  return 0;
}

// This function called by real Topo AFM, indirectly
int nmm_Microscope_Simulator::
spm_sweep_arc( const char *bufptr )
{
/***************************************************************************
 * Request to sweep to and return the height along a specified arc. This
 * implements the "ZAG_MODE" command request, which is the virtual whisk
 * broom tool.
 * The application should do a normal point or line to get to the location
 * of the line, set to modification mode, then call this routine with the
 * current location as the arc location.
 * The length is linearly interpolated between the endpoints.
 * This routine returns results along the outside of the arc starting at the
 * starting angle and going along the arc in steps of "spacing" units nearer
 * than spacing to the end, then stepping to the end angle.
 ***************************************************************************/

  float x, y, start_angle, start_len;
  float end_angle, end_len;
  float spacing;

	// Get the point from the buffer
  if (decode_DrawSweepArc(&bufptr, &x, &y, &start_angle, &start_len,
			&end_angle, &end_len, &spacing) == -1) {
     ServerOutputAdd (2, "nmm_Microscope_Simulator::stm_sweep_angle: Too few parameters passed for STM_SWEEP_ANGLE");
     return -1;
  }

  //  ServerOutputAdd(2, "spm_sweep_angle(%f,%f: %f - %f; %f)",x,y, start_angle,end_angle, spacing);

	// Check the parameters
  if ( (spacing == 0) || (start_angle == end_angle) ) {
     ServerOutputAdd(1,"**ERROR**: spm_sharp_angle(): Bad parameters");
     return -1;
  }

  spm_stop_scan();

	// Set the maximum linear motion to be faster than for scanning,
 	// remember the value that we should put back
                // XXX SetScanRateRemotelyNMeters() defined in scantopo.c:369
//   SetScanRateRemotelyNMeters(stm_nmeters_per_second*10);

	// Do all steps from the start location to right before the end
 	// location. Use the starting length to determine the distance
	// between two points along the arc length. Since circumference
	// = 2*pi*len and arc_len = circumference*(dangle/(2*pi)),
	// arc_len = dangle*len
  {
     float   dangle = end_angle - start_angle;
     float   dlen = end_len - start_len;
     float   arc_len = float(start_len * fabs(dangle));
     int     count, numsteps = (int)floor( arc_len / spacing );
     float   stepangle = dangle / numsteps;
     float   steplen = dlen / numsteps;

     for (count = 0; count < numsteps; count++) {
         if (spm_sweep_step( x,y,
	      start_angle + count*stepangle, start_len + count*steplen) == -1){ 
                // XXX SetScanRateRemotelyNMeters() defined in scantopo.c:369
// 	    SetScanRateRemotelyNMeters(stm_nmeters_per_second);
            return -1;
         }
     }
  }

	// Do the last sweep step
  if (spm_sweep_step( x, y, end_angle, end_len) == -1) {
                // XXX SetScanRateRemotelyNMeters() defined in scantopo.c:369
//      SetScanRateRemotelyNMeters(stm_nmeters_per_second);
     return -1;
  }

	// Set the maximum linear motion back to what it is normally
                // XXX SetScanRateRemotelyNMeters() defined in scantopo.c:369
//   SetScanRateRemotelyNMeters(stm_nmeters_per_second);

  stm_current_mode = STM_IDLE_MODE;
  return 0;
}

 



int nmm_Microscope_Simulator::
move_in_sewing_mode( float point_x, float point_y )
{
/************************************************************************
 * move_in_sewing_mode -- Moves the tip when in sewing mode, doing a
 * punch whenever we've gone afm_sm_move_dist from the last punch
 * location. It assumes that 'afm_last_punch' has been filled in
 * before it is called ( by calling move_and_punch() ).
 *	This routine tracks the afm_last_punch and afm_prev_point values
 * separately, since a punch may not have been made exactly at the end
 * of the previous move ( due to the requirement of exactly the same
 * distance between punches; the line may not have been a multiple of
 * this length ).
 *	A new punch is produced whenever the tip motion passes
 * afm_sm_move_dist from the last punched point. The prev_point gets
 * moved to the end of the previous line, and is used along with the
 * passed in (point_x, point_y) to determine the direction of travel.
 ************************************************************************/

	// First punch on this line, assigned a value here only to prevent the
	// compiler from issuing a warning. It is set to the intersection below
  float first_x = afm_prev_point_x;
  float first_y = afm_prev_point_y;

	// Step to take in x and y to move the correct distance
	// along the line. Length of the line from the previous point to
	// the one just asked for
  double dx, dy, linelen;

	// Current location as we step along the line, and how far we've
	// gone along the line from the last point to the new one.
  double curr_x;
  double curr_y;
  double dist_gone;

	// If the previout point does not lie within the circle of radius
	// afm_sm_move_dist from afm_last_punch, first punch our way over
	// from the last punch point to the previous point( this could
	// happen if afm_sm_move_dist changes since the last time ).
  if ( afm_sm_move_dist <= sqrt( sqr(afm_prev_point_x - afm_last_punch_x) +
                                 sqr(afm_prev_point_y - afm_last_punch_y)) ) {
      float prev_x = afm_prev_point_x;  // Go to the previous point
      float prev_y = afm_prev_point_y;
      afm_prev_point_x = afm_last_punch_x;   // Start from the punch point
      afm_prev_point_y = afm_last_punch_y;
      move_in_sewing_mode(prev_x, prev_y);
  }

	// If the point we want to go to lies inside a circle of radius
	// afm_sm_move_dist from the last punch point, then we won't be
	// doint any punching, so we just set the previous point to the
	// desired point and we're done.
  if ( afm_sm_move_dist > sqrt( sqr(point_x - afm_last_punch_x) +
                                sqr(point_y - afm_last_punch_y)) ) {
      afm_prev_point_x = point_x;
      afm_prev_point_y = point_y;
      return 0;
  }

	// Figure out the first point to punch. This is the intersection of
	// a circle of radius afm_sm_move_dist around the afm_last_punch
	// location with the line segment from the prev_point to the desired
	// point. We are guaranteed (by the code above) that the previous
	// point lies within this circle and the new point lies outside the
	// circle, so there is exactly one intersection with the line segment
	// between them (there will be two intersections with the line, one
	// going in the wrong direction).
  solve_for_intersection( afm_last_punch_x, afm_last_punch_y, afm_sm_move_dist,
			// Above is center, radius of circle
			afm_prev_point_x, afm_prev_point_y, point_x, point_y,
			// Above is inner, outer line endpoints
			&first_x, &first_y );
			// Above is intersection within the segment
  if ( move_and_punch( first_x, first_y ) ) {
    return -1;
  }

	// Compute the step (dx,dy) that will move along the line from the
	// previous position towards the desired position in afm_sm_move_dist
	// steps. This is used to go from one punch to the next along this line
  linelen = sqrt( sqr(point_x - afm_prev_point_x) +
                  sqr(point_y - afm_prev_point_y) );
  if (linelen == 0) { // No distance to move
     dx = dy = 0;
  } else {
      dx = (point_x - afm_prev_point_x) / linelen; // Unit step along the line
      dy = (point_y - afm_prev_point_y) / linelen; //
  }
  dx *= afm_sm_move_dist;
  dy *= afm_sm_move_dist;

	// Move from the first point along the line until taking another
	// step would bring us past the end of the line.
  curr_x = first_x;
  curr_y = first_y;
  dist_gone = sqrt( sqr(first_x - afm_prev_point_x) +
                          sqr(first_y - afm_prev_point_y) );
  while (dist_gone + afm_sm_move_dist <= linelen) {
      curr_x += dx;
      curr_y += dy;
      dist_gone += afm_sm_move_dist;
      move_and_punch((float)curr_x,(float)curr_y);

  }

	// Move the previous point to the desired endpoint. The last_punch
	// point remains where it was.
  afm_prev_point_x = point_x;
  afm_prev_point_y = point_y;

  return 0;
}

int nmm_Microscope_Simulator::
move_in_forcecurve_mode( float point_x, float point_y )
{
/*********************************************************************
 * move_in_forcecurve_mode -- Moves the tip when in force mode, doing a
 * punch whenever we've gone spm_fc_move_dist from the last force curve
 * location.  It assumes that 'spm_last_fc' has been filled in
 * before it is called (by calling move_and_fc()).
 *   This routine tracks the spm_last_fc and afm_prev_point values
 * separately, since a fc may not have been made exactly at the end
 * of the previous move (due to the requirement of exactly the same
 * distance between force curves; the line may not have been a multiple of
 * this length).
 *   A new fc is produced whenever the tip motion passes^M
 * spm_fc_move_dist from the last force curve point.  The prev_point gets
 * moved to the end of the previous line, and is used along with the
 * passed in (point_x,point_y) to determine the direction of travel.
 *********************************************************************/


	// First fc on this line, assigned a value here only to prevent the
	// compiler from issuing a warning. It's set to the intersection below.
  float first_x = afm_prev_point_x;
  float first_y = afm_prev_point_y;

	// Step to take in x and y to move the correct distance along the line,
	// Length of the line from the previous point to the one just asked for
  double dx, dy, linelen;

	// Current location as we step along the line, and how far we've gone
	// along the line from the last point to the new one.
  double curr_x;
  double curr_y;
  double dist_gone;

	// If the previous point does not lie within the circle of radius
	// spm_fc_move_dist from afm_last_fc, first fc
	// our way over from the last fc point to the previous point (this
	// could happen if spm_fc_move_dist changed since the last time).
  if ( spm_fc_move_dist <= sqrt( sqr(afm_prev_point_x - spm_last_fc_x) +
                                 sqr(afm_prev_point_y - spm_last_fc_y)) ) {
      float prev_x = afm_prev_point_x;  // Go to the previous point
      float prev_y = afm_prev_point_y;
      afm_prev_point_x = spm_last_fc_x;   // Start from the fc point
      afm_prev_point_y = spm_last_fc_y;
      move_in_forcecurve_mode(prev_x, prev_y);
  }

	// If the point we want to go to lies inside a circle of radius
	// spm_fc_move_dist from the last fc point, then we won't be doing
	// a force curve, so we just set the previous point to the desired
	// point and we're done.
  if ( spm_fc_move_dist > sqrt( sqr(point_x - spm_last_fc_x) +
                                sqr(point_y - spm_last_fc_y)) ) {
      afm_prev_point_x = point_x;
      afm_prev_point_y = point_y;
      return 0;
  }

	// Figure out the first point to punch. This is the intersection of
	// a circle of radius spm_fc_move_dist around the afm_last_punch
	// location with the line segment from the prev_point to the desired
	// point. We are guaranteed ( by the code above ) that the previous
	// point lies within this circle ad the new point lies outside the
	// circle, so there is exactly one intersection with the line segment
	// between them (there will be two intesections with the line, one
	// going in the wrong direction).
  solve_for_intersection(spm_last_fc_x,spm_last_fc_y, spm_fc_move_dist,
			// Center, radius of circle
			afm_prev_point_x,afm_prev_point_y, point_x,point_y,
			// Inner, outer line endpoints
			&first_x, &first_y);
			// Intersection within the segment
  if (move_and_forcecurve(first_x, first_y)) {
     return -1;
  }

	// Compute the step (dx,dy) that will move along the line from the
	// previous position towards the desired position in spm_fc_move_dist
	// steps. This is used to go from one punch to next along this line.
  linelen = sqrt( sqr(point_x - afm_prev_point_x) +
                  sqr(point_y - afm_prev_point_y) );
  if (linelen == 0) { // No distance to move
     dx = dy = 0;
  } else {
    dx = (point_x - afm_prev_point_x) / linelen; // Unit step along the line
    dy = (point_y - afm_prev_point_y) / linelen;
  }
  dx *= spm_fc_move_dist;  // Correct-length step along the line
  dy *= spm_fc_move_dist;

	// Move from the first point along the line until taking another
	// step would bring us past the end of the line.
  curr_x = first_x;
  curr_y = first_y;
  dist_gone=sqrt( sqr(first_x - afm_prev_point_x) +
                          sqr(first_y - afm_prev_point_y) );
  while (dist_gone + spm_fc_move_dist <= linelen) {
      curr_x += dx;
      curr_y += dy;
      dist_gone += spm_fc_move_dist;
      move_and_forcecurve((float)curr_x,(float)curr_y);

  }

	// Move the previous point to the desired endpoint. The last_fc
	// point remains where it was
  afm_prev_point_x = point_x;
  afm_prev_point_y = point_y;

  return 0;
}

int nmm_Microscope_Simulator::
spm_sweep_step (float to_x, float to_y, float sw_angle, float sw_len)
{
/*************************************************************************
 * This implements the "ZAG_MODE" command request, which is the virtual
 * whisk broom too. The trajectory goes from the current location to a
 * point that is sweep_len in the direction of sweep_angle away from the
 * point that is halfway between the current and desired loction and then
 * to the desired location. Each of these two points is returned using
 * the standard point return value.
 *************************************************************************/

  float avg_x, avg_y;    // Average of current location and desired location
  float far_x,far_y;     // Far end of the sweep

  	// Find the point halfway between the current and desired location
  avg_x = (stm_current_x + to_x) / 2;
  avg_y = (stm_current_y + to_y) / 2;

 	// Find the far end of the sweep, which is along the sweep
	// vector from the average point
  far_x = (float)(avg_x + sw_len*cos(sw_angle));
  far_y = (float)(avg_y + sw_len*sin(sw_angle));

  if (afm_mode == SEWING_MODE) {
    if ( move_in_sewing_mode(far_x,far_y) ||
         move_in_sewing_mode(to_x, to_y) ) {
      return -1;
    }
  } else {
	// Go to the point that is sweep_len along sweep_angle from the
	// point halfway between the current position and the desired
	// position and return a point sample there
    if (goto_point_and_report_it(far_x, far_y)) {
       ServerOutputAdd(1,"**ERROR**: spm_sweep_step: Cannot go to far point and scan it" );
       return -1;
    }

	// Go to the desired point and return a point sample there.
    if (goto_point_and_report_it(to_x, to_y)) {
       ServerOutputAdd(1,"**ERROR**: spm_sweep_step: Cannot go to end point and scan it" );
       return -1;
    }
  }

  return 0;
}

float nmm_Microscope_Simulator::
spm_get_z_value (int /*NUM_SAMPLES*/)
{
/*************************************************************************
 * It returns the height at the current point in nanometers
 *************************************************************************/

  float height = 0;

  return height;
}

int nmm_Microscope_Simulator::
solve_for_intersection( float cx, float cy, float r,
			float x0, float y0, float x1, float y1,
			float *ix, float *iy )
{
/*************************************************************************
 * Solve for the intersection of the circle with radius r at (cx,cy)
 * and the line segment from (x0,y0) to (x1,y1).
 *	The solution is found by binary search; the search terminates
 * when the solution point is within 0.01 units of the circle.
 * The solution returned may either inside or outside the circle.
 * XXX We should find the exact solution; it's only algebra.
 *	Returns 0 if the solution was found, 1 if it was not.
 *************************************************************************/

  double    lx = x0;  // Low (inside) guess, x component
  double    ly = y0;  // Low (inside) guess, y component
  double    hx = x1;  // High (outside) guess, x component
  double    hy = y1;  // High (outside) guess, y component
  double    mx = (lx+hx)/2;   // Middle x
  double    my = (ly+hy)/2;   // Middle y
  double    mrad = sqrt( sqr(mx-cx) + sqr(my-cy) );

	// Make sure there is a solution (low is inside and
	// high is outside circle).
     if ( (sqrt( sqr(lx-cx) + sqr(ly-cy) ) > r) ||
          (sqrt( sqr(hx-cx) + sqr(hy-cy) ) < r)) {
          *ix = cx;
          *iy = cy;
          return 1;
     }

     while ( fabs(mrad - r) > 0.01 ) {
         if (mrad > r) { // Middle is outside circle, move high down
            hx = mx; hy = my;
         } else { // Middle is inside circle, move low up
            lx = mx; ly = my;
         }
         mx = (lx+hx)/2;   // Middle x
         my = (ly+hy)/2;   // Middle y
         mrad = sqrt( sqr(mx-cx) + sqr(my-cy) );
     }

     *ix = (float)mx;
     *iy = (float)my;
     return 0;
}

int nmm_Microscope_Simulator::
move_and_punch( float the_desired_x, float the_desired_y )
{
/***************************************************************************
 * move_and_punch -- what this does is moves the tip to a specified location
 * and reports the data sets when it goes there. After this initial
 * reporting, it punches the tip down to the Set Point and reports again
 * and then pulls the tip back up.
 ***************************************************************************/

   //  nm_boolean state;

	// Go to the point
  spm_goto_xynm( the_desired_x, the_desired_y );

	// Set the last punch cordinate
  afm_last_punch_x = the_desired_x;
  afm_last_punch_y = the_desired_y;

  punch_in( the_desired_x, the_desired_y );
  if ( gather_data_and_report(d_BottomPunchResultData_type, the_desired_x,
		the_desired_y, afm_sm_bot_delay) ) {
    ServerOutputAdd( 1, "***ERROR***: move_and_punch" );
    return -1;
  }
  punch_out();
  if ( gather_data_and_report(d_TopPunchResultData_type, the_desired_x,
		the_desired_y, afm_sm_top_delay) ) {
    ServerOutputAdd( 1, "***ERROR***: move and punch" );
    return -1;
  }

  return 0;
}

int nmm_Microscope_Simulator::
move_and_forcecurve (float the_desired_x, float the_desired_y)
{
/****************************************************************************
 * move_and_forcecurve: what this does is move the tip to a specified location
 * and reports the data sets when it gets there. After this initial reporting,
 * it does a force curve and reports results from that.
 ****************************************************************************/

   //  nm_boolean state;
  int result = 0;

	// Stop scanning first
  spm_stop_scan();
	// Set the last punch coordinate
  spm_last_fc_x = the_desired_x;
  spm_last_fc_y = the_desired_y;

  ServerOutputAdd(1, "nmm_Microscope_Simulator::ForceCurveRemote(): haven't implemented yet!");
  result = 0;
  return result;
}

int nmm_Microscope_Simulator::
punch_out(void)
{
/****************************************************************************
 * punch_out moves the tip up to dist (dist is in nMeters )
 * 	This function leaves the PID values zero, so that the tip will remain
 * above the surface while it is being moved to the next location.
 ****************************************************************************/

  return 0;
}

int nmm_Microscope_Simulator::
punch_in( float /*posx*/, float /*posy*/ )
{
/***************************************************************************
 * punch_in used to move the tip to Set Point at one nM increments. Now it
 * simply turns on feedback (by setting the PID values to something
 * non-zero) and that causes the tip to drop to the surface almost 10 times
 * as fast as the old method.
 ***************************************************************************/

  return 0;
}


///////////////////////////////////////////////////////////////////////////
// JakeK and AFMS team. Changed so that it gets point data for live touches
// from our surface data structure instead of asking for it from the 
// real microscope
///////////////////////////////////////////////////////////////////////////

int nmm_Microscope_Simulator::
report_point_set( float x, float y )
{
/**************************************************************************
 * report_point_set takes in x and y coordinates and reports the data sets.
 **************************************************************************/
  
  const int numsets = 1;
  if(set_point > 1)		// Checks to see if surface is to be modified
  {
     //float point_value[numsets];
     moveTipToXYLoc( (x-(num_x/2)), (y-(num_y/2))); 
  }

  float point_value[numsets];
  float z;
  getImageHeightAtXYLoc( (x-(num_x/2)), (y-(num_y/2)), &z );
  point_value[0] = z;
  
  spm_report_point_datasets(d_PointResultData_type, x, y, point_value, numsets);
  return 0;
}


float nmm_Microscope_Simulator::
spm_get_z_piezo_like_scan (int /*NUM_SAMPLES*/)
{
/************************************************************************
 * It returns the Z piezo at the current point in DAC units
 ************************************************************************/

  float height = 0;
  return height;
}

float nmm_Microscope_Simulator::
spm_get_z_piezo_nM(int /*NUM_SAMPLES*/)
{
/************************************************************************
 * It returns the Z piezo value at the current point in nanometers
 ************************************************************************/

  float height = 0;

  return height;
}


int nmm_Microscope_Simulator::
spm_report_point_datasets(long report_type, double x, double y,
				float* data, int count )
{
/************************************************************************
 * Buffers the height and other data into the outgoing buffer.
 * If it's too slow we can always force it to send the data right now.
 ************************************************************************/

  long len;
  char * msgbuf;
  int retval;

  float val_x = (float)x,val_y = (float)y;
  struct timeval t;

  if ( connection ) { 	// XXX not sure
	// First get the time
    gettimeofday(&t, NULL);

    

	// Give the report
    msgbuf = encode_ResultData( &len, val_x, val_y, t.tv_sec, t.tv_usec,
				count, data );
    if ( !msgbuf ) { 
      ServerOutputAdd( 2, "nmm_Microscope_Simulator::spm_report_point_datasets:  Buffer overflow!!" );
      return -1;
    } else {
      retval = Send( len, report_type, msgbuf );
      if ( retval ) {
        ServerOutputAdd( 2, "nmm_Microscope_Simulator::spm_report_point_datasets:  Couldn't pack message to send to client." );
        return -1;
      }
    }
  }


	// add more cod here
        // takes data and puts it onthe buffer
	// time stuff will be put in here

  return 0;
}

int nmm_Microscope_Simulator::
time_to_relax(float /*x*/, float /*y*/)
{
/**************************************************************************
 * time_to_relax samples a point ever millisecond and buffers the
 * information until it has sampled for longer than the value specified
 * in the variable RelaxDelay (which is declared in servfunc.c).
 **************************************************************************/

  long len;
  char * msgbuf;
  int retval;

  struct timeval start_time;

  if (RelaxDelay) {
     gettimeofday(&start_time, NULL);
	// tell the client when relaxing started
     msgbuf = encode_StartingToRelax( &len, start_time.tv_sec,
					start_time.tv_usec );
     if ( !msgbuf ) {
       ServerOutputAdd( 2, "nmm_Microscope_Simulator::time_to_relax:  Buffer overflow!!" );
       return -1;
     } else {
       retval = Send( len, d_StartingToRelax_type, msgbuf );
       if ( retval ) {
         ServerOutputAdd( 2, "nmm_Microscope_Simulator::time_to_relax:  Couldn't pack message to send to client." );
         return -1;
       }
     }

  }

  return 0;
}

void nmm_Microscope_Simulator::
spm_goto_xynm( float /*x*/, float /*y*/ )
{
/*****************************************************************
 * Moves the tip to position x and y given in nanometers.
 */
}

float nmm_Microscope_Simulator::
ScanUnitsToNm( void )
{
/************************************************************************
 * Returns a factor to multiply scan units to convert them into nm.
 * Based on afmstm.c:GetScannerXYunits()
 ************************************************************************/

  float factor;

      factor = 1000;

  return factor;
}

int nmm_Microscope_Simulator::
gather_data_and_report(long /*report_type*/, float /*the_desired_x*/,
		float /*the_desired_y*/, unsigned int /*wait_time*/)
{

  return 0;
}

int nmm_Microscope_Simulator::
goto_point_and_report_it(float the_desired_x, float the_desired_y)
{
   //  unsigned int  state;


  //  stm_current_mode = STM_SEEK_MODE;
	// Go to the point
  spm_goto_xynm (the_desired_x, the_desired_y);

  if ( report_point_set(the_desired_x,the_desired_y) ) return -1;

  afm_prev_point_x = the_desired_x;
  afm_prev_point_y = the_desired_y;

  return 0;
}

void nmm_Microscope_Simulator::
spm_disable_param_reporting(void)
{
/**************************************************************************
 * disables the reporting of system scanning and feedback parameters during
 * the operation of modes where the unix side shouldn't know abut the
 * changes. This includes the time during a mode switch between tapping and
 * contact mode, and the time spent in sewing mode (when the feedback
 * parameters and scan rates vary all the time, but do not indicate desired
 * changes in the settings to be used).
 **************************************************************************/

  UpdateFeedbackParamsNow = 0;
}

void nmm_Microscope_Simulator::
spm_enable_param_reporting(void)
{
/**************************************************************************
 * sets the system to report these changes (the default behaviour).
 **************************************************************************/

  UpdateFeedbackParamsNow = 1;
}

void nmm_Microscope_Simulator::
spm_disable_register_amp(void)
{
/**************************************************************************
 * disables the registering of feedback amplitude during the time a mode
 * switch into tapping mode.
 **************************************************************************/

  RegisterAmpEnabled = 0;
}

void nmm_Microscope_Simulator::
spm_enable_register_amp(void)
{
/**************************************************************************
 * sets the system to register these changes (the default behavior).
 **************************************************************************/

  RegisterAmpEnabled = 1;
}

int nmm_Microscope_Simulator::
spm_is_register_amp_enabled(void)
{
/**************************************************************************
 * the reporting function is used by noncont.c to tell if it should
 * do this or not.
 **************************************************************************/

  return RegisterAmpEnabled;
}

int nmm_Microscope_Simulator::
spm_report_contact_mode(void)
{
/**************************************************************************
 * Report to the Unix side that we are in contact mode and specify the
 * current parameters. We don't say anything if parameter reporting has
 * been disabled, since we are in a state that should not affect the
 * current parameters on the user interface side.
 **************************************************************************/

  long len;
  char * msgbuf;
  int retval;

  float pv=0, iv=0, dv=0, spt=0;	// for reporting

  if (!ServerOn || !UpdateFeedbackParamsNow) { return 0; }

	// Find the current parameters

	// Report that we are in contact mode with the current parameters
  ServerOutputAdd (2, "report_contact_mode: reporting in progress");
  msgbuf = encode_InContactMode( &len, pv, iv, dv, spt );
  if ( !msgbuf ) {
    ServerOutputAdd( 3, "nmm_Microscope_Simulator::spm_report_contact_mode:  Buffer overflow!!" );
    return -1;
  } else {
    retval = Send( len, d_InContactMode_type, msgbuf );
    if ( retval ) {
      ServerOutputAdd( 3, "nmm_Microscope_Simulator::spm_report_contact_mode:  Couldn't pack message to send to client." );
      return -1;
    }
  }
 
  return 0;
}

int nmm_Microscope_Simulator::
spm_report_tapping_mode(void)
{
/**************************************************************************
 * Report to the Unix side that we are in tapping mode and specify the
 * current parameters. We don't say anything if parameter reporting has
 * been disabled, since we are in a state that should not affect the
 * current parameters on the user interface side.
 **************************************************************************/

  long len;
  char * msgbuf;
  int retval;

  float pv=1, iv=1, dv=1, spt=1, damp=1;	// for reporting

	// XXX ??? need to rewrite here -> ConnectionOn is in SERVER.C
  if (!ServerOn || !UpdateFeedbackParamsNow) { return 0; }

	// Find the parameters
                // XXX vGetPID() defined in afmstm.c and TOPOWN16.H TOPOWN32.C
//   NM_vGetPID(&pv, &iv, &dv);	// HACK Tiger	defined in nmm_Microscope_Simulator_func.h
//                 // XXX GetSetPoint() defined in scantopo.c: 1196
//   NM_GetSetPoint(&spt);			// HACK Tiger	defined in nmm_Microscope_Simulator_func.h
//   damp = spm_driveamp_set_last;

	// Report that we are in tapping mode with the current parameters
		// XXX SERVOUT
  ServerOutputAdd (2, "report_tapping_mode: reporting in progress");
                // XXX AFM_IN_TAPPING_MODE defined in stm_cmd.h
  msgbuf = encode_InTappingMode( &len, pv, iv, dv, spt, damp );
  if ( !msgbuf ) {
    ServerOutputAdd( 3, "nmm_Microscope_Simulator::spm_report_tapping_mode:  Buffer overflow!!" );
    return -1;
  } else {
    retval = Send( len, d_InTappingMode_type, msgbuf );
    if ( retval ) {
      ServerOutputAdd( 3, "nmm_Microscope_Simulator::spm_report_tapping_mode:  Couldn't pack message to send to client." );
      return -1;
    }
  }

  return 0;
}

void nmm_Microscope_Simulator::sendBeginFeelTo (vrpn_float32 x,
                                                vrpn_float32 y) {
  long len;
  char * msgbuf;

  msgbuf = encode_BeginFeelTo(&len, x, y);
  Send(len, d_BeginFeelTo_type, msgbuf);

}

void nmm_Microscope_Simulator::sendEndFeelTo (vrpn_float32 x,
                                              vrpn_float32 y) {
  long len;
  char * msgbuf;

  msgbuf = encode_EndFeelTo(&len, x, y);
  Send(len, d_EndFeelTo_type, msgbuf);

}


///////////////////////////////////////////////////////////////////////
// JakeK and AFMS team: This was changed so that we can stop the simulator
// from sending scan data
////////////////////////////////////////////////////////////////////////

// This function called by real Topo AFM, indirectly
void nmm_Microscope_Simulator::
spm_stop_scan(void)
{
  mic_start = 0;
  ServerOutputAdd (2,"Stopping scan");
  
}


int nmm_Microscope_Simulator::
Send( long len, long msg_type, char * buf )
{
  struct timeval now;
  int retval;

  gettimeofday(&now, NULL);
  retval = connection->pack_message(len, now, msg_type, d_myId,
					buf, vrpn_CONNECTION_RELIABLE);
  if ( buf ) {
    delete [] buf;
  }
  return retval;
}

void nmm_Microscope_Simulator::
get_current_xy(float *posx, float *posy)
{
  *posx = stm_current_x;
  *posy = stm_current_y;
}

int nmm_Microscope_Simulator::
vReportNewScanRange(float /*vleft*/, float /*vright*/, float /*vtop*/, float /*vbottom*/)
{
         float xmin=0, ymin=0, xmax=1, ymax=1;

		 ServerOutputAdd(2, "nmm_Microscope_Simulator::vReportNewScanRange() executing ...");

         /* Change Unit from whatever it is to nM */
         xmin *= ScanUnitsToNm();
         ymin *= ScanUnitsToNm();
         xmax *= ScanUnitsToNm();
         ymax *= ScanUnitsToNm();

         stm_region_xmin_actual = stm_region_xmin = xmin;
         stm_region_ymin_actual = stm_region_ymin = ymin;
         stm_region_xmax_actual = stm_region_xmax = xmax;
         stm_region_ymax_actual = stm_region_ymax = ymax;

		 ServerOutputAdd(2, "nmm_Microscope_Simulator::vReportNewScanRange() finished execution!");

         return( spm_report_region_clipped(xmin, ymin, xmax, ymax));
}

// point of insertion into SPMlab code for this function is in:
// Specurve.c::pCreateSpecData() - second to last line of this function
// we should call spm_report_force_curve_data(pSG);
int nmm_Microscope_Simulator::
spm_report_force_curve_data()
{
/*
 * Format of transferred data:
 *
 *  long time_s, time_ms;
 *      long x, y;
 *      long num_samples;
 *      long num_halfcycles;
 *  long z[0]
 *      long f1[0]      // first halfcycle (downwards)
 *      long f2[0]      // second halfcycle (upwards)
 *              :
 *      long fn[0]      // n = num_halfcycles
 *              :
 *      long z[num_samples-1]
 *      long f1[num_samples-1]
 *      long f2[num_samples-1]
 *              :
 *      long fn[num_samples-1]
 */
  return 0;
}


///////////////////////////////////////////////////////////////////////////////
// JakeK and AFMS team: This was changed to enable us to send window data from
// our data structure surface
///////////////////////////////////////////////////////////////////////////////

int nmm_Microscope_Simulator::
spm_report_window_line_data(int currentline){
 
/****************************************************************************
 * Takes a data line from each PImg in every data sets and buffer without
 * transforming the raw data at all.
 *
 * Format of trasnfered data
 *
 *   long x, y;
 *   long dx, dy;
 *   long reports; //(#of reports)
 *   long fields;  //(#of fields)
 *   long time_s, time_ms;
 *
 *   long data1[0];
 *   long data2[0];
 *         :
 *   long datan[0]; // n = fields
 *   long data1[1];
 *   long data2[1];
 *         :
 *   long datan[1]; // n = fields
 *         :
 *
 ****************************************************************/

	// Begin code added by AFMS team
	// JuanM

  struct timeval t;
                            // init t_start w/ time since 01/01/70
  gettimeofday(&t, NULL);   // can access the sec and usec

  long len; 
  long x = 0;
  long y = currentline;     // make counter that keeps track of what line
  long dx = 1; 
  long dy = 0; 
  long lineCount = num_x;   // possibly from variable in head statement
  long fieldCount = 1; 	    // Number of layers of data.
  static float ** data = NULL;
  float z;
  int i, j;

  if (data == NULL) {
    data = new float * [fieldCount];
    for (i = 0; i < fieldCount; i++) {
      data[i] = new float [lineCount];
    }
  }

  for (i = 0; i < lineCount; i++)	{
    for (j = 0; j < fieldCount; j++) {
       //getImageHeightAtXYLoc( (i-(num_x/2)), (currentline-(num_y/2)), &z);
       getImageHeightAtXYLoc(i, currentline, &z);
       data[j][i] = z;
    }
  }

	// XXX ??? need to rewrite here -> ConnectionOn is in SERVER.C
  if (!ServerOn || !UpdateFeedbackParamsNow) { return 0; }

	// Report that we are in window line data mode with the current parameters
	// XXX SERVOUT

  int retval;
  char * msgbuf;
  msgbuf = encode_WindowLineData( &len, x, y, dx, dy, lineCount, fieldCount, t.tv_sec, t.tv_usec, data);
  if ( !msgbuf )
  {
    ServerOutputAdd( 3, "nmm_Microscope_Simulator::spm_report_window_line_data:  Buffer overflow!!" );
    return -1;
  }
  else
  {
    retval = Send( len, d_WindowLineData_type, msgbuf );
    if ( retval ) 
    {
      ServerOutputAdd( 3, "nmm_Microscope_Simulator::spm_report_window_line_data:  Couldn't pack message to send to client." );
      return -1;
    }

fprintf(stderr, "Sent scan data for line %d.\n", y);
  }
  return 0;
}





float nmm_Microscope_Simulator::
GetClientScanRateInNm(void)
{
  return stm_nmeters_per_second;
}

void nmm_Microscope_Simulator::
get_startup_params(float *pv, float *iv, float *dv, float *setpoint, float *scanrate, vrpn_bool *scanmode)
{
  *pv = startup_p;
  *iv = startup_i;
  *dv = startup_d;
  *setpoint = startup_setpoint;
  *scanrate = startup_scanrate;
  *scanmode = (int)startup_scanmode;	// XXX add recasting

  return;
}

void nmm_Microscope_Simulator::
helpTimer2()
{
	//***************************************************************
	//* Enable Custom Timer 2 for moving the region gradually!!
    	//* Timer2 is a 25ms timer and uses "2" for its ID.
	//***************************************************************

		  ServerOutputAdd( 3, "Custom 2 Timer enabled successfully" );
}

// This function called by real Topo AFM, indirectly
void nmm_Microscope_Simulator::
helpShutdown()
{
	abnormal_shutdown = VRPN_FALSE;
	ShutdownSession();
}

void nmm_Microscope_Simulator::
helpGetConnection()
{
  ServerOutputAdd (1, "nmm_Microscope_Simulator::helpGetConnection(): Connection established");

  // Unless this is set to NM_FALSE later, any shutdown is treated as abnormal.
  abnormal_shutdown = VRPN_TRUE;

  // Init the microscope - and send all initial information to 
  // microscape.
  // XXX This may not be the right place to do this....
  stm_init();

}

void ServerError( char * txt )
{
  char msg[200] ;
  sprintf(msg, "**ERROR**: ");
  strcat (msg, txt);
  ServerOutputAdd (1, msg);

  AFMSimulator->ShutdownSession ();
/*
  WaitForConnection ();
*/
}


/*****************************************************************
 * END OF FILE
 *****************************************************************/

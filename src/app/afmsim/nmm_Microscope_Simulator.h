#ifndef NMM_MICROSCOPE_SIMULATOR_H
#define NMM_MICROSCOPE_SIMULATOR_H

#include "nmm_Microscope.h"
#include "nmb_Types.h"
#include "BCGrid.h"

#define NANOM_VERSION    7     // Version 7
#define NANOM_REVISION   1     // Release 1


#define SIN30    0.5
#define COS30    0.8660254037844

#define STM_SCAN_MODE           (2)
#define STM_POINT_MODE          (3)
#define STM_IDLE_MODE           (4)
#define STM_SHUTDOWN_MODE       (5)
#define STM_APPROACH_MODE       (6)
#define STM_ENTER_MODE          (7)
#define STM_PULSE_MODE          (8)
#define STM_ZAG_MODE            (9)
#define STM_ZIG_MODE            (10)
#define SPM_SNAP_MODE           (11)
#define SPM_FORCECURVE_MODE     (12)

#define BOUNCE_ERROR                    100
#define BOUNCE_QUIT                     101
#define BOUNCE_STATUSMESSAGE            102
#define BOUNCE_STATUSMESSAGECLEAR  	103

#define CONNECT_PORT    (5558)
#define BUF_SIZE        (4000)
#define DESIRED_WINSOCK_VERSION 0x0101

#define WINSOCK_EVENT   901    // Event happened in the socket
#define WINSOCK_ACCEPT  902    // Sent when connection ready in socket

#define SR_STEPS  10

#define MAX_VAR_SAMPLES (256)
#define MAX_NUM_SCAN_PARAMETERS (20)
#define MAX_ECHO_DATA   (100)	// Max 4byte words to be echoed back

#define IMAGING_MODE      1
#define MODIFICATION_MODE 2

// values for afm_mode
#define TAPPING_MODE   1
#define CONTACT_MODE   2
#define TAP_CON_MODE   3
#define SEWING_MODE    4
#define FORCECURVE_MODE 5

#define N_ADC 3

#define MAX_REQUEST_DATA_SETS 18
#define MAX_REQUEST_POINT_SETS MAX_REQUEST_DATA_SETS/2 

typedef struct {
  int numsets;
  short idAcqType[MAX_REQUEST_POINT_SETS];
  char  units[MAX_REQUEST_POINT_SETS][64];
  float scale[MAX_REQUEST_POINT_SETS];             // The real unit values are computed DAC*scale+offset
  float offset[MAX_REQUEST_POINT_SETS];
  int   num_samples[MAX_REQUEST_POINT_SETS];
} SET_OF_POINT_SETS;

typedef struct {
  char name[64];           // Label for this data set          SHO1
  char units[64];          // Units it will be in once conversion is applied  SHO1
  float scale;             // The real unit values are computed DAC*scale+offset 
  float offset;          
  short type;              // Type 
} DATA_SET;

typedef struct {
  int changed;
  int numsets;
  DATA_SET set[MAX_REQUEST_DATA_SETS];
} SET_OF_DATA_SETS;


#define   sqr(x) ((x)*(x))


typedef struct {
  float max;
  float min;
  float cur;
} FORCE;


// Start and stop server and create the simulator object
void StartServer (int x, int y, int port=4500);
void StopServer (void);

///////////////////////////////////////////////////////////////////
// Added by JakeK and AFMS team for simulator
///////////////////////////////////////////////////////////////////

int mic_Started (void);	// is active

void set_Surface_Data (BCPlane *);

///////////////////////////////////////////////////////////////////


class nmm_Microscope_Simulator;


extern int	currentline;
extern vrpn_Connection * connection;
extern nmm_Microscope_Simulator * AFMSimulator;

class nmm_Microscope_Simulator: public nmb_Device_Server, nmm_Microscope
{

	// Simulator server for Microscope

  public:

/**************************************************************************
 * PUBLIC MEMBER FUNCTIONS
 **************************************************************************/

    nmm_Microscope_Simulator (const char * name,
                                vrpn_Connection *);
	// Constructor. Takes as arguments the name of the microscope,
	// 	a pointer to a vrpn_Connection.

    virtual ~nmm_Microscope_Simulator (void);
	// Destructor


  // MANIPULATORS


//    virtual void mainloop (void);
	// virtual from nmm_Microscope.
	// A VRPN mainloop. This function should be called regularly
	// 	by the application using an object of this class to
	//	make sure outgoing message buffers get flushed and
	// 	incoming messages get read and responded to.
	

 
    void stm_init(void);

	void helpShutdown(void);

	void ShutdownSession (void);

	void helpGetConnection();

    int ProcessPendingScanRange();

    int spm_report_latest_PID(float pv, float iv, float dv);

    int spm_report_latest_scanrate(float speed);

    int spm_report_latest_resolution();

    int spm_report_latest_data_sets( SET_OF_DATA_SETS * set );

    int spm_report_latest_point_sets( SET_OF_POINT_SETS point );

    int spm_report_latest_region();

    int spm_report_current_mode(void);

    int spm_report_topo_header();

    int time_to_relax(float x, float y);

    float ScanUnitsToNm( void );

    int spm_is_register_amp_enabled(void);

    int Send( long len, long msg_type, char * buf );

    void spm_store_current_driveamp(float damp);
    float spm_read_current_driveamp(void);

    void set_the_scan_rate (float speed);
    void get_current_xy( float *posx, float *posy );

    int vReportNewScanRange(float vleft, float vright,
				float vtop, float vbottom);

    int spm_report_force_curve_data();

    int spm_report_window_line_data( int );

    float GetClientScanRateInNm(void);

    int spm_report_latest_angle(void);

    void spm_stop_scan();


/***********************************************************************
 * PUBLIC DATA MEMBERS
 ***********************************************************************/

    int	spmFirstPointSinceZoom;

    //**************************************************************
    //* Custom Timer 2 : For Set Region command
    //**************************************************************
    vrpn_bool ProcessSuspended;             // Process Suspended flag
    int cTimer2;                       // Custom Timer 2
  
  protected:

/***********************************************************************
 * PROTECTED DATA MEMBERS
 ***********************************************************************/

    // static CB 	callbacks [];


	 long d_Shutdown_type;		// HACK XXX Tiger
	 long d_GetConnectionFromClient_type;		// HACK XXX Tiger



     vrpn_float32  	stm_nmeters_per_second;
     vrpn_float32  	stm_max_move;

     vrpn_int32	stm_grid_num_x;
     vrpn_int32	stm_grid_num_y;
     vrpn_int32 	stm_window_xmin;
     vrpn_int32	stm_window_ymin;
     vrpn_int32	stm_window_xmax;
     vrpn_int32	stm_window_ymax;

     vrpn_float32	stm_region_xmin;
     vrpn_float32 	stm_region_ymin;
     vrpn_float32	stm_region_xmax;
     vrpn_float32	stm_region_ymax;
     vrpn_float32	stm_region_angle;

/*****************************************************************
 *   startup settings	// XXX do I still need them?
 *****************************************************************/
     vrpn_float32 	startup_p;
     vrpn_float32 	startup_i;
     vrpn_float32 	startup_d;
     vrpn_float32 	startup_setpoint;
     vrpn_float32 	startup_scanrate;
     vrpn_float32 	startup_scanmode;

/*****************************************************************
 *   Sewing machine mode
 *****************************************************************/
     vrpn_float32 	afm_sm_set_point;
     unsigned int afm_sm_bot_delay;  //in 0.1 milli-seconds
     unsigned int afm_sm_top_delay;  //in 0.1 milli-seconds
     vrpn_float32 	afm_sm_pull_dist;
     vrpn_float32 	afm_sm_move_dist;
     vrpn_float32 	afm_sm_rate;         // in nM per second
     vrpn_float32 	afm_sm_max_close_nm; // the max dist to punch regarless
				     // of the set point.

     vrpn_float32 	afm_last_punch_x;
     vrpn_float32 	afm_last_punch_y;
     vrpn_float32 	afm_prev_point_x;
     vrpn_float32 	afm_prev_point_y;

     vrpn_float32 	IS_scale;   // Internal Sensor scale
     vrpn_float32 	IS_offset;  // Internal Sensor offset

     vrpn_bool 	afm_first_punch;
     vrpn_float32 	afm_first_punch_distance;

     unsigned int num_punch;

/*****************************************************************
 *   Force curve mode
 *****************************************************************/
     vrpn_float32 	spm_last_fc_x;
     vrpn_float32 	spm_last_fc_y;
     vrpn_float32 	spm_fc_move_dist;
     vrpn_float32 	spm_fc_start_delay;
     vrpn_float32 	spm_fc_z_start;
     vrpn_float32 	spm_fc_z_end;
     vrpn_float32 	spm_fc_z_pullback;
     vrpn_float32 	spm_fc_force_limit;
     vrpn_int32 	spm_fc_num_layers;
     vrpn_int32 	spm_fc_num_halfcycles;

//*****************************************************************
//* Set region gradual : Changing the region gradually
//*****************************************************************
     vrpn_float32  	stm_region_xmin_work;
     vrpn_float32  	stm_region_ymin_work;
     vrpn_float32  	stm_region_xmax_work;
     vrpn_float32  	stm_region_ymax_work;
     vrpn_float32  	stm_region_xmin_unit;
     vrpn_float32  	stm_region_ymin_unit;
     vrpn_float32  	stm_region_xmax_unit;
     vrpn_float32  	stm_region_ymax_unit;
     vrpn_float32  	stm_region_xmin_actual;
     vrpn_float32  	stm_region_ymin_actual;
     vrpn_float32  	stm_region_xmax_actual;
     vrpn_float32  	stm_region_ymax_actual;

     int    	isrStep;
     vrpn_bool   	SetRegionReceived;
     vrpn_bool   	ReportAngle;
//******************************************************************

     vrpn_int32	stm_current_mode;

     vrpn_float32	stm_current_x;		// Where we are right now
     vrpn_float32	stm_current_y;		//

     vrpn_float32	stm_desired_x;		// Where we are seeking to be
     vrpn_float32	stm_desired_y;		//

     vrpn_float32	stm_desired_approach;	// Desired approach location, nm
     vrpn_float32	stm_coarse_z;		// Coarse Z position

     vrpn_int32	stm_scan_style;		// Style of scanning
     vrpn_int32	stm_scan_x;		// Where the area scan is right now
     vrpn_int32 	stm_scan_y;
     vrpn_int32 	stm_scan_dx;		// Current direction of scan
     vrpn_int32 	stm_scan_dy;

     vrpn_int32	StdDelay, StPtDelay;

// These are default settings for the system contact force
     vrpn_int32	fmods_enabled;		// Is cotact force control available?

     struct timeval sewing_start_time;

// Default values for delays after changing contact force before return to scan
     vrpn_int32	RelaxDelay;

     vrpn_int32	stm_num_samples;	// Number of A/D samples to read
     vrpn_float32	stm_sample_freq;	// Sample at 160kHz

     vrpn_float32  	fDACtoWorld, fDACtoWorldZero;

     int    	force_mode;

     int	afm_mode;
     vrpn_float32  	spm_driveamp_set_last; 	// Tracks the currently-set drive
				     	// amplitude in the dialog box

     int    	ADC1held, ADC2held;     // 1 if currenlty holding ADC

     int UpdateFeedbackParamsNow;  	// Enables reporting of PID
					//  Setpoint, Amplitude
     int RegisterAmpEnabled;      	// Enables registering drive amplitude
					// in noncont.c




/**********************************************************************
 * PROTECTED MEMBER FUNCTIONS
 **********************************************************************/

     char * encode_ScanDataset (long * len, int, char *, char *, int, int);

     char * encode_PointDataset (long * len, int, char *, char *, int, int);

     int report_selected_scanner_desc(void);

     int spm_report_region_clipped( double xmin, double ymin,
					double xmax, double ymax );

     int stm_set_grid_size( const char *bufptr );

     int stm_scan_window( const char *bufptr );

     int stm_resume_window_scan();

     int stm_set_rate_nmeters( const char *bufptr );

     int stm_scan_point_nm( const char *bufptr );

     int spm_set_region_angle_gradual( const char *bufptr );

     int stm_set_region_nm_gradual( const char *bufptr );

     int stm_sweep_point_nm( const char *bufptr );

     int spm_query_scan_range( void );

     int spm_set_max_move( const char *bufptr );

     int spm_set_relax( const char *bufptr );

     int spm_blunt_point_nm( const char *bufptr );

     int spm_request_scan_datasets( const char *bufptr );

     int spm_request_point_datasets( const char *bufptr );

     int spm_set_pid( const char *bufptr );

     int afm_sewing_mode( const char *bufptr );

     int afm_contact_mode( const char *bufptr );

     int afm_tapping_mode( const char *bufptr );

     int spm_echo( const char *bufptr );

     int spm_enable_voltsource( const char *bufptr );

     int spm_disable_voltsource( const char *bufptr );

     int spm_enable_amp( const char *bufptr );

     int spm_disable_amp( const char *bufptr );

     int spm_set_slow_scan( const char *bufptr );

     int spm_sharp_line( const char *bufptr );

     int spm_sweep_line( const char *bufptr );

     int spm_sweep_arc( const char *bufptr );




     void PostProcessScanRange();

     int spm_report_angle( float xmin, float ymin, float xmax, float ymax );

     int spm_report_angle_clipped( float xmin, float ymin,
					float xmax, float ymax );

     int spm_sweep_step(float to_x, float to_y, float sw_angle, float sw_len); 

     int move_in_sewing_mode( float point_x, float point_y );

     int move_in_forcecurve_mode( float point_x, float point_y );

     int solve_for_intersection( float cx, float cy, float r,
                        float x0, float y0, float x1, float y1,
                        float *ix, float *iy );

     int move_and_punch( float the_desired_x, float the_desired_y );

     int move_and_forcecurve( float the_desired_x, float the_desired_y );

     int punch_out(void);

     int punch_in( float posx, float posy );

     float spm_get_z_value (int NUM_SAMPLES);

     int report_point_set( float x, float y );

     float spm_get_z_piezo_like_scan (int NUM_SAMPLES);

     float spm_get_z_piezo_nM( int NUM_SAMPLES );

     int spm_report_point_datasets( long type, double x, double y,
					float* data, int count );

     void spm_goto_xynm( float x, float y );

     int gather_data_and_report(long report_type, float the_desired_x,
				float the_desired_y, unsigned int wait_time);

     int goto_point_and_report_it(float the_desired_x, float the_desired_y);

     void spm_enable_param_reporting(void);

     void spm_disable_param_reporting(void);

     void spm_enable_register_amp(void);

     void spm_disable_register_amp(void);

     int spm_report_contact_mode(void);

     int spm_report_tapping_mode(void);


	 void get_startup_params(float *pv, float *iv, float *dv,
								float *setpoint, float *scanrate, vrpn_bool *scanmode);

	 void helpTimer2();




  private:

     // Receive callbacks
     static int RcvShutdown( void *_userdata, vrpn_HANDLERPARAM _p );
	 static int RcvGetConnection( void *_userdata, vrpn_HANDLERPARAM _p ); 
     static int RcvSetRate( void *_userdata, vrpn_HANDLERPARAM _p );
     static int RcvSetGridSize( void *_userdata, vrpn_HANDLERPARAM _p );
     static int RcvScanWindow( void *_userdata, vrpn_HANDLERPARAM _p );
     static int RcvScanPoint( void *_userdata, vrpn_HANDLERPARAM _p );
     static int RcvIdle( void *_userdata, vrpn_HANDLERPARAM _p );
     static int RcvResumeWindowScan( void *_userdata, vrpn_HANDLERPARAM _p );
     static int RcvSetStdDevParams( void *_userdata, vrpn_HANDLERPARAM _p );
     static int RcvQStdDevParams( void *_userdata, vrpn_HANDLERPARAM _p );
     static int RcvSetScanStyle( void *_userdata, vrpn_HANDLERPARAM _p );
     static int RcvSetContactForce( void *_userdata, vrpn_HANDLERPARAM _p );
     static int RcvQContactForce( void *_userdata, vrpn_HANDLERPARAM _p );
     static int RcvSetRegionNM( void *_userdata, vrpn_HANDLERPARAM _p );
     static int RcvScanPointNM( void *_userdata, vrpn_HANDLERPARAM _p );
     static int RcvSampleApproachNM( void *_userdata, vrpn_HANDLERPARAM _p );
     static int RcvQueryScanRange( void *_userdata, vrpn_HANDLERPARAM _p );
     static int RcvEcho( void *_userdata, vrpn_HANDLERPARAM _p );
     static int RcvZagPointNM( void *_userdata, vrpn_HANDLERPARAM _p );
     static int RcvSetMaxMove( void *_userdata, vrpn_HANDLERPARAM _p );
     static int RcvSetStdDelay( void *_userdata, vrpn_HANDLERPARAM _p );
     static int RcvSetStPtDelay( void *_userdata, vrpn_HANDLERPARAM _p );
     static int RcvSetRelax( void *_userdata, vrpn_HANDLERPARAM _p );
     static int RcvMeasureResist( void *_userdata, vrpn_HANDLERPARAM _p );
     static int RcvZigPointNM( void *_userdata, vrpn_HANDLERPARAM _p );
     static int RcvSnapShot( void *_userdata, vrpn_HANDLERPARAM _p );
     static int RcvSnapCancel( void *_userdata, vrpn_HANDLERPARAM _p );
     static int RcvBluntPointNM( void *_userdata, vrpn_HANDLERPARAM _p );
     static int RcvReqScanDataset( void *_userdata, vrpn_HANDLERPARAM _p );
     static int RcvSetSlowScan( void *_userdata, vrpn_HANDLERPARAM _p );
     static int RcvReqPotDataset( void *_userdata, vrpn_HANDLERPARAM _p );
     static int RcvSetPID( void *_userdata, vrpn_HANDLERPARAM _p );
     static int RcvSetRatenMeters( void *_userdata, vrpn_HANDLERPARAM _p );
     static int RcvEnableVoltsource( void *_userdata, vrpn_HANDLERPARAM _p );
     static int RcvDisableVoltsource( void *_userdata, vrpn_HANDLERPARAM _p );
     static int RcvEnableAmp( void *_userdata, vrpn_HANDLERPARAM _p );
     static int RcvDisableAmp( void *_userdata, vrpn_HANDLERPARAM _p );
     static int RcvSetRegAndAngle( void *_userdata, vrpn_HANDLERPARAM _p );
     static int RcvClientPacketTimestamp(void *_userdata, vrpn_HANDLERPARAM _p);
     static int RcvSharpLine( void *_userdata, vrpn_HANDLERPARAM _p );
     static int RcvSweepLine( void *_userdata, vrpn_HANDLERPARAM _p );
     static int RcvSweepArc( void *_userdata, vrpn_HANDLERPARAM _p );
     static int RcvClientHello( void *_userdata, vrpn_HANDLERPARAM _p );

     static int RcvSewingMode( void *_userdata, vrpn_HANDLERPARAM _p );
     static int RcvContactMode( void *_userdata, vrpn_HANDLERPARAM _p );
     static int RcvTappingMode( void *_userdata, vrpn_HANDLERPARAM _p );
	


};

#endif  // NMM_MICROSCOPE_SIMULATOR_H

/*      This header file includes the command designations for the Scanning
 * Tunnelling Microscope device control routine.  The "accross the network"
 * control routine is called sdi_stm_server.c. */

#ifndef stm_cmd_h
#define stm_cmd_h

#define MAXBUF  8000

/*      This section contains definitions for the scan modes.  This is
 * the parameter passed to STM_SET_SCAN_STYLE that tells how to scan
 * the window. */

#define BOUST_X_FASTEST         (0)     /* Boustrophedonic scan, X fastest */
#define BOUST_Y_FASTEST         (1)     /* Boustrophedonic scan, Y fastest */
#define RASTER_X_FASTEST_POS    (2)     /* Raster scan +X fastest, +Y next */
#define RASTER_Y_FASTEST_POS    (3)     /* Raster scan +Y fastest, +X next */

/* The length of all names passed as strings over the network
 * (buffers are fixed size rather than variable)
 * TCH, September 97 */
#define STM_NAME_LENGTH 64

/*      This section contains the commands that are sent to the stm server
 * from the client program.
 *      The defines should be sent as binary integers with a write command
 * to the file descriptor of the control routine.
 *      The parameters listed should be passed immediately after the command
 * and in the same write command.  They should also be sent as binary data
 * rather than ascii formatted. */

#define STM_SHUTDOWN            (1)
	/* void */
#define STM_SET_RATE            (2)
	/* float lines_per_second */
#define STM_SET_GRID_SIZE       (3)
	/* int x_steps,y_steps */
/*#define       STM_SET_REGION_SIZE     (4)*/
	/* float xmin,ymin, xmax,ymax */
#define STM_SCAN_WINDOW         (5)
	/* int xmin,ymin, xmax,ymax */
/*#define       STM_SCAN_POINT          (6)*/
	/* float x,y */
#define STM_IDLE                (7)
	/* void */
#define STM_RESUME_WINDOW_SCAN  (8)
	/* void */
/*#define       STM_SAMPLE_APPROACH     (9)*/
	/* float desired_voltage */
#define STM_ENTER_TUNNELLING    (10)
	/* void */
#define STM_SET_BIAS            (11)
	/* float desired_bias */
#define STM_SET_PULSE_PEAK      (12)
	/* float desired_peak */
#define STM_SET_PULSE_DURATION  (13)
	/* float time_in_sec */
/*#define       STM_PULSE_POINT         (14)*/
	/* float x,y */
#define STM_QUERY_PULSE_PARAMS  (15)
	/* void */
#define STM_SET_STD_DEV_PARAMS  (16)
	/* int count; float frequency */
#define STM_QUERY_STD_DEV_PARAMS        (17)
	/* void */
#define STM_SET_SCAN_STYLE      (18)
	/* int style */
#define AFM_SET_CONTACT_FORCE   (19)
	/* float force */
#define AFM_QUERY_CONTACT_FORCE (20)
	/* void */
/*#define STM_SCAN_STD          (21)*/
	/* float x,y */
#define STM_SET_REGION_NM       (22)
	/* float xmin,ymin, xmax,ymax */
#define STM_SCAN_POINT_NM       (23)
	/* float x,y */
#define STM_PULSE_POINT_NM      (24)
	/* float x,y */
#define STM_SAMPLE_APPROACH_NM  (25)
	/* float desired_location */
#define SPM_QUERY_SCAN_RANGE    (26)
	/* void */
#define SPM_ECHO                (27)
	/* long # 4byte words followed by data */
#define SPM_ZAG_POINT_NM        (28)
	/* float x,y, zag angle, zag mag, step size */
#define SPM_SET_MAX_MOVE        (29)
	/* float max dist (nm) per 1000us step */
//#define AFM_SET_MOD_FORCE       (30)
	/* float [0..1] */
//#define AFM_MOD_MODE            (31)
	/* float [0..1] */
//#define AFM_SET_IMG_FORCE       (32)
	/* float [0..1] */
//#define AFM_IMG_MODE            (33)
	/* float [0..1] */
//#define AFM_IMG_PARMS           (34)
	/* float max, min DrAmp, float tapping Setpoint */
//#define AFM_MOD_PARMS           (35)
	/* float max, min Setpoint */
//#define AFM_QUERY_FORCES        (36)
	/* none */
//#define AFM_CON_MODE            (37)
//#define AFM_TAP_MODE            (38)
//#define AFM_TC_MODE             (39)
	/* what modes to image and modify in, TC switches */
#define AFM_SET_STD_DELAY       (40)
	/* long (ms) delay after changing GUI parms */
#define AFM_SET_STPT_DELAY      (41)
	/* long (ms) delay after changing GUI setpoint */
#define SPM_SET_RELAX           (42)
	/* float Tmin (millisecond!), Tsep (millisecond) */
#define SPM_MEASURE_RESIST      (43)
	/* long which_meter; float rate [0 = single sample] */
#define SPM_ZIG_POINT_NM        (44)
	/* flt x,y; flt max,min one step move */
#define SPM_SNAP_SHOT           (45)
	/* long nx, ny */
#define SPM_SNAP_CANCEL         (46)
	/* void */
#define SPM_BLUNT_POINT_NM      (47)
	/* float x, float y, float dist */
#define SPM_REQUEST_SCAN_DATASETS (48)
	// int count, {char name[64]}[count]
#define SPM_SET_SLOW_SCAN       (49)
	// int enabled (default is enabled.  send 1 to enable and 0 to disable)
#define SPM_REQUEST_POINT_DATASETS (50)
	// int count, {char name[64], int num_samples }[count]
#define SPM_SET_PID		(51)
	// float P,I,D       added by jstarmer
#define STM_SET_RATE_NMETERS	(52)
	// float nm/sec      added by jstarmer
#define AFM_SEWING_MODE		(53)
	// float SetPoint, Bottom Delay, Top Delay, Pull Back Dist, Move Dist
	// float Move rate, Max approach
#define AFM_CONTACT_MODE	(54)
	// float P, I, D, SetPoint
#define AFM_GUARDED_SCAN_MODE (71)
	// float p, i, d, setpoint, fNormalX, fNormalY, fNormalZ, fPlaneD, fGuardDepth
#define AFM_TAPPING_MODE	(55)
	// float P, I, D, SetPoint, Driving Amplitude 
#define SPM_ENABLE_VOLTSOURCE    (56)
	// int device_num, float voltage
#define SPM_DISABLE_VOLTSOURCE   (57)
	// int device_num
#define SPM_ENABLE_AMP          (58)
	   // int device_num, float offset, float percent_offset, int gain
#define SPM_DISABLE_AMP         (59)
	   // int device_num
#define SPM_SET_REGION_AND_ANGLE 	  (60)
	   /* float xmin,ymin, xmax,ymax, float angle */
#define	SPM_CLIENT_PACKET_TIMESTAMP	(61)
		// int (long) sec, usec
#define	SPM_SHARP_LINE			(62)
		// float startx,starty, endx,endy, spacing
#define	SPM_SWEEP_LINE			(63)
		// float startx,starty,startang,startmag, endx,endy,endang,endmag, spacing
#define	SPM_SWEEP_ARC			(64)
		// float x,y, startang,startmag, endeng,endmag, spacing		
#define SPM_SPECTROSCOPY_MODE	(65)
                // float setpoint,
		// float start_delay (usec), z_start (nm), z_end (nm)
                // float z_pullback (nm), force limit (nA), move_dist
                // long #layers, #half cycles
		// float sample_speed (um), float pullback_speed (um)
		// float start_speed (um), float feedback_speed (um)
                // long avg_num,
                // float sample_delay (us), pullback_delay (us)
                // float feedback_delay (us)

#define SPM_XYZ_POINT_NM     (66)
// float x,y,z
#define AFM_DIRECTZ_MODE     (67)
// float max_z_step, max_xy_step, min_setpoint, max_setpoint, max_lateral_force


// contact, tapping mode changes are done by older functions
// datasets for scanline mode are shared with scan datasets so the client
// must take care of changing the datasets and storing the reported data
// sets in the right place as there may be differences on the server as
// to what datasets can be active in the two different modes
#define SPM_SCANLINE_MODE (68)
	// long enable
#define SPM_SET_FEEDBACK_FOR_SCANLINE (69)	
	// long enable, long check_limit,
	// float limit, max_z_step, max_xy_step
#define SPM_REQUEST_SCANLINE (70)
	// float x,y,z,angle, width; long resolution


/*      This section contains the descriptors for the packets that are
 * returned by the server to the client application.  The parameters that
 * follow are also listed. */

/*#define       STM_WINDOW_SCAN_RESULT  (1)*/
	/* int x,y; int (long) sec,usec; float value */
/*#define       STM_POINT_SCAN_RESULT   (2)*/
	/* float x,y; int (long) sec,usec; float result */
#define STM_APPROACH_COMPLETE   (3)
	/* void */
/*#define       STM_TUNNELLING_ATTAINED (4)*/
	/* float voltage */
#define STM_TUNNELLING_FAILURE  (5)
	/* void */
/*#define       STM_PULSE_COMPLETED     (6)*/
	/* float x,y, */
/*#define       STM_PULSE_FAILURE       (7)*/
	/* float x,y */
#define STM_PULSE_PARAMETERS    (8)
	/* int enabled; float bias, pulse_peak, pulse_width */
/*#define       STM_STD_DEV_SCAN_RESULT (9)*/
	/* int x,y; int sec,usec; float mean, standard_deviation */
#define STM_STD_DEV_PARAMETERS  (10)
	/* int count; float frequency */
#define AFM_FORCE_SET           (11)
	/* float newforce */
#define AFM_FORCE_SET_FAILURE   (12)
	/* float attemptedforce */
#define AFM_FORCE_PARAMETERS    (13)
	/* int enabled; float force */
/*#define       STM_WINDOW_BACKSCAN     (14)*/
	/* int x,y; int (long) sec,usec; float value */
/*#define       STM_STD_DEV_BACKSCAN    (15)*/
	/* int x,y; int (long) sec,usec; float value */
/*#define       STM_POINT_STD_RESULT    (16)*/
	/* float x,y; int (long) sec,usec; float result */
#define STM_WINDOW_SCAN_NM      (17)
	/* int x,y; int (long) sec,usec; float mean, standard_deviation */
#define STM_WINDOW_BACKSCAN_NM  (18)
	/* int x,y; int (long) sec,usec; float mean, standard deviation */
#define STM_POINT_RESULT_NM     (19)
	/* float x,y; int (long) sec,usec; float result ; float deviation????*/
#define STM_PULSE_COMPLETED_NM  (20)
	/* float x,y, */
#define STM_PULSE_FAILURE_NM    (21)
	/* float x,y */
#define STM_SET_REGION_COMPLETED        (22)
	/* float xmin,ymin, xmax,ymax */
#define STM_SET_REGION_CLIPPED  (23)
	/* float xmin,ymin, xmax,ymax */
#define STM_TUNNELLING_ATTAINED_NM      (24)
	/* float position */
#define SPM_SCAN_RANGE          (25)
	/* float xmin,xmax, ymin,ymax, zmin,zmax */
#define AFM_MOD_FORCE_SET       (26)
	/* float [0..1] */
#define AFM_MOD_FORCE_SET_FAILURE (27)
	/* float [0..1] */
#define AFM_IMG_FORCE_SET       (28)
	/* float [0..1] */
#define AFM_IMG_FORCE_SET_FAILURE (29)
	/* float [0..1] */
#define AFM_IMG_SET             (30)
	/* int enabled, float max, min, cur */
#define AFM_MOD_SET             (31)
	/* int enabled, float max, min, cur */
#define AFM_IN_MOD_MODE         (32)
#define AFM_IN_IMG_MODE         (33)
	/* voids */
#define SPM_RELAX_SET           (34)
	/* float Tmin, Tsep (see relax.c) */
	// actually, these are both ints  (clark 7/25/97)
#define AFM_IN_MOD_MODE_T       (35)
	/* long s, us */
#define AFM_IN_IMG_MODE_T       (36)
	/* long s, us */
#define SPM_RESISTANCE          (37)
	/* long which_meter; int (long) sec, usec; float resistance */
#define SPM_RESISTANCE_FAILURE  (38)
	/* long which_meter */
#define STM_ZIG_RESULT_NM       (39)
	/* float x,y; int (long) sec,usec; float z, Nx,Ny,Nz */
#define SPM_SNAP_SHOT_BEGIN     (40)
	/* int nx, ny */
#define SPM_SNAP_SHOT_END       (41)
	/* void */
#define STM_SCAN_PARAMETERS     (42)
	/* */
#define SPM_BLUNT_RESULT_NM     (43)
	/* float x, y; int (long) sec, usec; float z, Nx, Ny, Nz */
#define SPM_WINDOW_LINE_DATA    (44)
	// int x,y; int dx,dy; int reports; int fields; int sec,usec;
	// { float values[fields]; }[reports]
	// (The fields should have been described by SPM_SCAN_DATASETS)
#define SPM_HELLO_MESSAGE       (45)
	// char "nM!"[4], char scope_name[64], int major_ver, int minor_ver
#define SPM_SCAN_DATASETS       (46)
	// int count,
	// { char name[64], char units[64], float offset, float scale }[count]
#define SPM_REPORT_SLOW_SCAN    (47)
	// int enabled
#define SPM_CLIENT_HELLO        (48)
	// char "nM!"[4], char client_name[64], int major_ver, int minor_ver
#define SPM_POINT_DATASETS      (49)
	// int count,
	// {char name[64], char units[64], int num_samples,
	//  float offset, float scale }[count]
#define SPM_POINT_RESULT_DATA   (50)
	// float x,y; int (long) sec,usec; int count; {float result}[count]
#define SPM_PID_PARAMETERS	(51)
	// float P,I,D
#define SPM_SCANRATE_PARAMETER	(52)
	// float scan rate (in nM)
#define SPM_REPORT_GRID_SIZE	(53)
	// int x-res, y-res 
#define AFM_IN_SEWING_MODE	(54)
	// float SetPoint, Bottom Delay, Top Delay, Pull Back Dist, Move Dist,
	// float Move rate, Max dist to approach to find SetPoint
#define AFM_IN_CONTACT_MODE	(55)
	// float P, I, D, SetPoint
#define AFM_IN_GUARDED_SCAN_MODE (76)
	// float P, I, D, SetPoint, fNormalX, fNormalY, fNormalZ, fPlaneD, fGuardDepth
#define AFM_IN_TAPPING_MODE	(56)
	// float P, I, D, SetPoint, Driving Amplitude 
#define SPM_BOTTOM_PUNCH_RESULT_DATA (57)
	// float x,y; int (long) sec,usec; int count; {float result}[count]
#define SPM_TOP_PUNCH_RESULT_DATA (58)
	// float x,y; int (long) sec,usec; int count; {float result}[count]
#define SPM_VOLTSOURCE_ENABLED	(59)
	// int device_num, float voltage
#define SPM_VOLTSOURCE_DISABLED	(60)
	// int device_num
#define SPM_AMP_ENABLED         (61)
        // int device_num, float offset, float percent_offset, int gain_mode
#define SPM_AMP_DISABLED        (62)
        // int device_num
#define SPM_STARTING_TO_RELAX   (63)
	   // long seconds, long uSeconds
#define SPM_REGION_ANGLE_SET    (64)
	   /* float xmin,ymin, xmax,ymax, float angle */
#define SPM_REGION_ANGLE_CLIPPED (65)
	   /* float xmin,ymin, xmax,ymax, float angle */
#define	SPM_TOPO_FILE_HEADER	(66)
	// long header_length, char header[header_length]
#define	SPM_SERVER_PACKET_TIMESTAMP	(67)
	// long sec,usec
#define OHM_RESISTANCE          (68)
        //  long channel; int (long) sec, usec; float resistance,
        //  float voltage (V), float range (min ohms), float filter (sec)

#define SPM_FORCE_CURVE_DATA	(69)
/*      long sec,usec
 *	long x, y
 *	long num_samples
 *	long num_halfcycles
 *      float z[0]
 *	float f1[0]	// first halfcycle (downwards)
 *	float f2[0]	// second halfcycle (upwards)
 *		:
 *	float fn[0]	// n = num_halfcycles
 *		:
 *	float z[num_samples-1]
 *	float f1[num_samples-1]
 *	float f2[num_samples-1]
 *		:
 *	float fn[num_samples-1]
 */

#define SPM_IN_SPECTROSCOPY_MODE	(70)
                // float setpoint,
		// float start_delay (usec), z_start (nm), z_end (nm)
		// float z_pullback (nm), force limit (nA), move_dist
                // long #layers, #half cycles
		// float sample_speed (um), float pullback_speed (um)
		// float start_speed (um), float feedback_speed (um)
		// long avg_num,
		// float sample_delay (us), pullback_delay (us)
		// float feedback_delay (us)

#define OHM_RESISTANCE_WSTATUS (71)
	//	long channel; int (long) sec, usec; float resistance,
	//	float voltage (V), float range (min ohms), float filter (sec),
	// 	long status

#define SPM_XYZ_RESULT_DATA   (72)
	// float x,y,z; int (long) sec,usec; int count; {float result}[count]

#define SPM_IN_SCANLINE_MODE (73)
	// long enabled
#define SPM_FEEDBACK_SET_FOR_SCANLINE (74)
	// long enable, long check_limit,
	// float limit, max_z_step, max_xy_step
#define SPM_SCANLINE_DATA       (75)
        // float x,y,z; float angle; float width;
        // long n_points; long fields; long sec,usec;
        // { float values[fields]}[n_points]
        // (The field order (value->dataset mapping) should have been
        // described by SPM_SCAN_DATASETS)

/* Parameters from 1000 up are known only to the client, and are echoed back
**      blindly by the server (using SPM_ECHO) to record info in stream
**/
#define AFM_BASE_MOD_PARAMETERS (1000)
	/* float baseforce, modforce */
#define SPM_VISIBLE_TRAIL       (1001)
	/* void */
#define SPM_INVISIBLE_TRAIL     (1002)
	/* void */
#define AFM_FORCE_SETTINGS      (1003)
	/* float base, mod, current */
#define SPM_REFRESH_GRID      (1004)
	/* void */

// Used only by microscape for writing to stream files when using network
// (clark 7/23/97)
#define NANO_RECV_TIMESTAMP   (1005)
        // struct timeval recvTime

// Used only by microscape when reading stream files from fake microscape
// (clark 9/9/97)  This is in here for use with stream_file_print.
#define FAKE_SPM_SEND_TIMESTAMP (1006)

// Sequence number added by translator (or whatever splits the streams) for
// UDP messages (clark 9/16/97)
#define SPM_UDP_SEQ_NUM (1007)

#endif



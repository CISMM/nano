#ifndef _NMA_KEITHLEY2400_H
#define _NMA_KEITHLEY2400_H

#include "nmb_Device.h"
#include "vrpn_Connection.h"
#include "vrpn_GPIBDevice.h"

// User routine to handle a message giving the a VI curve 
typedef struct {
	struct timeval msg_time;
	vrpn_int32 num_values;
	vrpn_float32 *data;
} vrpn_VIRESULTDATACB;
typedef void (*vrpn_VIRESULTDATAHANDLER)(void *userdata, 
					const vrpn_VIRESULTDATACB &info);

// User routine to handle a message giving the a GPIB Error
typedef struct {
	char *error_msg;
} vrpn_VIERRORCB;
typedef void (*vrpn_VIERRORHANDLER)(void *userdata, 
					const vrpn_VIERRORCB &info);

class nma_Keithley2400 : public nmb_Device_Client, public vrpn_GPIBDevice 
{
public:
    nma_Keithley2400(const char *name, vrpn_Connection *c = NULL);
    virtual int mainloop(const struct timeval * timeout = NULL); 
        // Handle getting any reports
    virtual ~nma_Keithley2400() {};

    // Registers functions to call when we receive data or errors.
	int register_result_handler(void *userdata,
		vrpn_VIRESULTDATAHANDLER handler);
	int unregister_result_handler(void *userdata,
		vrpn_VIRESULTDATAHANDLER handler);
    // XXX Error callback not implemented yet. 
	int register_error_handler(void *userdata,
        vrpn_VIERRORHANDLER handler);
	int unregister_error_handler(void *userdata,
        vrpn_VIERRORHANDLER handler);

	// Commands that send a related set of instructions to the Keithley
    
    // This makes the inital connection and must be re-done if the 
    // Keithley experiences an error.
	int send_Device();

    // This sends all the settings and readies the Keithley for a reading.
    int send_AllSettings();

    // Send various settings. All called by send_AllSettings.
	int send_Clear();
    int send_DisplayEnable();
	int send_Function();
	int send_Source();
	int send_Sense();
	int send_Trigger();
	int send_DataFormat();
	int send_OutputOn();
	int send_OutputOff();

    // Take some data
	int send_AcquireData();
    // Send me the data you took
    int send_ReadData();
	int send_ReadFloatData();
    // Shortcut function. Take some Float data and send it to me.
    int send_DoCurve();

	int rcv_Error(char * write_buf);
	// When the connection shuts down, close the instrument we are talking to.
	int rcv_Shutdown();

	// Index of GPIB board
	int d_board_index;
	// Address of Keithley 2400, set to 24 in factory
	int d_primary_address;
	// Secondary address is usually not used, so is set to 0
	int d_secondary_address;

	// Tells when Keithley is actively sourcing (current or voltage)
	// Must be turned off before some commands are issued.
	int d_output_on;
	// Tells when Keithley is initialized with Device command.
	// If there is an error, this initialization must be performed again.
	int d_initialized;
    // Tells us that we should take repeated measurments - 
    // initiate another measurement when the last is received. 
    int d_repeat_curve_active;
    
    // Tells whether the display on the front of the Keithley is on or not.
    // Keithley claims it will operate faster if the display is off. 
    int d_display_enable;

	// Parameters which describe the VI curve to be measured.
	int d_source;       // Voltage or current
	int d_compliance;       // Voltage or current
    float d_compliance_val;    // Voltage or current won't exceed this value 
    float d_num_power_line_cycles; // Number of power line cycles to integrate over
    // for each measurement. In the US, a power line cycle is 1/60 of a second. 
    // Min is 0.01, max is 10, default is 1. 
	int d_sweep;		// Linear or Log
	float d_sweep_start;
	float d_sweep_stop;
	int d_sweep_numpoints;		// number of points in linear or log sweep.
    float d_sweep_delay;            // Delay before a measurement is taken. Can be zero. 


	enum Channel { VOLTAGE, CURRENT };
	enum Scale { LINEAR, LOG };

protected:
    typedef struct vrpn_RESULTDATALIST_STRUCT {
        void *userdata;
        vrpn_VIRESULTDATAHANDLER handler;
        struct vrpn_RESULTDATALIST_STRUCT *next;
    } vrpn_RESULTDATALIST;
	vrpn_RESULTDATALIST *result_change_list;
    
    typedef struct vrpn_ERRORLIST_STRUCT {
        void *userdata;
        vrpn_VIERRORHANDLER handler;
        struct vrpn_ERRORLIST_STRUCT *next;
    } vrpn_ERRORLIST;
	vrpn_ERRORLIST *error_change_list;


	// Send a message, delete [] the message buffer.
	int Send( vrpn_int32 len, vrpn_int32 msg_type, char * buf );
private:

	// Receive callbacks
	static int handle_Result( void *_userdata, vrpn_HANDLERPARAM _p );
	static int handle_ResultData( void *_userdata, vrpn_HANDLERPARAM _p );
	static int handle_Error( void *_userdata, vrpn_HANDLERPARAM _p );
	static int handle_Shutdown( void *_userdata, vrpn_HANDLERPARAM _p );
	
};

#endif

#ifndef OHMMETER_H
#define OHMMETER_H

#include <tcl.h>

#include <vrpn_Ohmmeter.h>

#include <Tcl_Linkvar.h>
#include <Tcl_Netvar.h>
#include <nmb_Types.h>	// for vrpn_bool

#include <nmm_MicroscopeRemote.h>
#include <nmui_Component.h>

#define NUM_SELECTION_VALUES (6)
#define AUTORANGE_WAIT (2)
#define NUM_OHMMETER_CHANNELS (4)
#define FRENCH_OHMMETER_ID	(0)
#define DEFAULT_CHANNEL		(0)
#define MIN_AVRGING_TIME	(0.1)
#define MAX_AVRGING_TIME	(20)

// status messages
#define STATUS_OKAY_MSG "Measuring"
#define STATUS_OVERFLOW_MSG "Range Too Small"
#define STATUS_SATURATED_MSG "Saturated"
#define STATUS_ERROR_MSG "Serial Port Error"
#define STATUS_UNDERFLOW_MSG "Range Too Large"
#define STATUS_UNKNOWN_MSG "Unknown"

// Tcl variable names
#define OHM_STATUS_VAR "ohm_status"
#define OHM_CHANNEL_VAR "ohm_channel"
#define OHM_VALUE_VAR "ohm_value"

// added to file name specified for output log file as a convenience
// so user just has to give a single name to specify all output files
#define OHM_FILE_SUFFIX ".ohm"

struct OhmmeterInitializationState {
	OhmmeterInitializationState(void);

	char deviceName [128];
	char inputLogName [256];
	char outputLogName [256];

	vrpn_bool readingLogFile;
	vrpn_bool writingLogFile;
};

class OhmmeterChannelParams {
  public:
    OhmmeterChannelParams() {enable = 0; voltage = 0; range = 0; filter = 0;
		autorange = 0; avrgtime = 0.1;}
    int enable, voltage, range, filter, autorange;
    float avrgtime;
};

class nmui_ORPXChannelParameters {
  public:
    nmui_ORPXChannelParameters(const char *channel_name,
                               int c_id, vrpn_Ohmmeter_Remote *d);
    ~nmui_ORPXChannelParameters();

    TclNet_int *enable;
    TclNet_int *autorange;
    TclNet_int *voltage;
    TclNet_int *range;
    TclNet_int *filter;
    TclNet_string *resistance;
    TclNet_string *status;
    TclNet_int *error;

    int autorange_num_over;
    int autorange_num_under;
    int channel_id;
    vrpn_Ohmmeter_Remote *device;

    static char *enable_var_base_name,
                *autorange_var_base_name,
                *voltage_var_base_name,
                *range_var_base_name,
                *filter_var_base_name,
                *resistance_var_base_name,
                *status_var_base_name,
                *error_var_base_name;

    static void handle_param_change(vrpn_int32 new_value, void *ud);
};

class Ohmmeter {
  public:
    Ohmmeter (Tcl_Interp * the_tcl_interp,
              const char * tcl_script_dir,
              vrpn_Ohmmeter_Remote * device);
    ~Ohmmeter(void);

    vrpn_bool windowOpen();
    void openWindow();

    void setMicroscope(nmm_Microscope_Remote *m);

    void updateDisplay(int chnum, float resistance, int status,
        float voltage, float range, float filter);

    // used to setup synchronization on protected
    // TclNet member variables
    void ohm_SetupSync(nmui_Component * container);

    // used to teardown synchronization on protected
    // TclNet member variables
    void ohm_TeardownSync(nmui_Component * container);

  protected:

    Tcl_Interp *tk_control_interp;
    Tclvar_int d_windowOpen;
    Tclvar_list_of_strings d_voltages;
    Tclvar_list_of_strings d_ranges;
    Tclvar_list_of_strings d_filters;

    vrpn_Ohmmeter_Remote *ohmmeterDevice;

    nmm_Microscope_Remote * microscope_ptr;

    nmui_ORPXChannelParameters 
                        *d_channelParams[NUM_OHMMETER_CHANNELS];

    static char *s_channel_names[NUM_OHMMETER_CHANNELS];
    static char *s_voltage_strings[NUM_SELECTION_VALUES];
    static char *s_range_strings[NUM_SELECTION_VALUES];
    static char *s_filter_strings[NUM_SELECTION_VALUES];

    // functions for trying to match up a particular float value with
    // one of the discrete values available
    int lookupVoltageIndex(float voltage);
    int lookupRangeIndex(float range);
    int lookupFilterIndex(float filter);

    static void handle_measurement (void *userdata, 
                                    const vrpn_OHMMEASUREMENTCB &info);
};

#endif

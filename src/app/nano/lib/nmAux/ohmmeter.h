#ifndef OHMMETER_H
#define OHMMETER_H

#include <tcl.h>
#include <Tcl_Linkvar.h>
#include <Tcl_Netvar.h>
#include "vrpn_Ohmmeter.h"
#include "nmb_Types.h"	// for vrpn_bool

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

class Ohmmeter {
  public:
    Ohmmeter (Tcl_Interp * the_tcl_interp,
              const char * tcl_script_dir,
              vrpn_Ohmmeter_Remote * device);
    ~Ohmmeter(void);

    // controls for entire ohmmeter
    Tclvar_checklist *ohmmeter_enable_checkbox;
    Tclvar_checklist *channel_enable_checklist;
    Tclvar_checklist *auto_range_checklist;
    Tclvar_selector *channel_select;
    nmb_ListOfStrings *list_of_channel_names;

    // controls for each ohmmeter channel
    nmb_ListOfStrings *list_of_voltages, *list_of_ranges, *list_of_filters;
    Tclvar_selector *voltage_select, *range_select, *filter_select;
    Tclvar_float_with_scale *avrgtime_slider;

    // displays of data:
    char ohm_status_message[128];
    char ohm_channel_msg[128];
    char ohm_value_msg[128];

    void updateDisplay(int chnum, float resistance, int status,
	float voltage, float range, float filter);

    // functions for trying to match up a particular float value with
    // one of the discrete values available
    int lookupVoltageIndex(float voltage);
    int lookupRangeIndex(float range);
    int lookupFilterIndex(float filter);

    OhmmeterChannelParams c_params[NUM_OHMMETER_CHANNELS];

    int ohmmeter_enabled;		// has ohmmeter been enabled?
    int current_channel;	// channel for which settings are currently
				// being viewed on control panel
    int autorange_enabled;
    int num_over;
    int num_under;

    vrpn_Ohmmeter_Remote *ohmmeterDevice;
    
    char *voltage_strings[NUM_SELECTION_VALUES];
    char *range_strings[NUM_SELECTION_VALUES];
    char *filter_strings[NUM_SELECTION_VALUES];
    char *channel_strings[NUM_OHMMETER_CHANNELS];

};

#endif

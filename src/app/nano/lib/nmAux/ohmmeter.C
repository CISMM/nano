#include "string.h"
#include <Tcl_Linkvar.h>
#include <Tcl_Netvar.h>
#include "ohmmeter.h"
#include "orpx.h"	// for parameter values
#include "math.h"

#ifndef OHMMETER_PROGRAM	// stuff for integrating with microscape as
				// opposed to the separate ohmmeter program
#include <nmm_Globals.h>
#ifndef USE_VRPN_MICROSCOPE // #ifndef #else #endif Added by Tiger
#include <Microscope.h>
#else
#include <nmm_MicroscopeRemote.h>
#endif
#include "microscape.h"

#endif	// NOT OHMMETER_PROGRAM

#define TCL_WIDGETS_FILE "russ_widgets.tcl"

const char *VOLTAGES_STR[NUM_SELECTION_VALUES] ={"3uV","10uV","30uV","100uV",
	"300uV","1mV"};
const char *RANGES_STR[NUM_SELECTION_VALUES] = {">0.5",">5",">50",">500",">5k",
	">50k"};
const char *FILTER_TIMES_STR[NUM_SELECTION_VALUES] ={"0.1", "0.3", "1.0", "3",
	"10", "30"};
const char *CHANNELS_STR[NUM_OHMMETER_CHANNELS] ={"ch 0", "ch 1", "ch 2", 
	"ch 3"};

static Tcl_Interp *tk_control_interp;

// call back functions for widgets
//static void handle_enable_ohmmeter_change
//     (const char *entry, int newval, void *ud);
static void handle_enable_channel_change
     (const char *entry, int newval, void *ud);
static void handle_autorange_change
     (const char *changed, int newval, void *ud);
static void handle_channel_select_change
     (const char *newvalue, void *ud);
static void handle_voltage_select_change
     (const char *newvalue, void *ud);
static void handle_range_select_change
     (const char *newvalue, void *ud);
static void handle_filter_select_change
     (const char *newvalue, void *ud);
//static void handle_avrgtime_slider_change(float newvalue, void *ud);

OhmmeterInitializationState::OhmmeterInitializationState (void) :
	readingLogFile (VRPN_FALSE),
	writingLogFile (VRPN_FALSE)
{
	strcpy(deviceName, "null");
}


#define LOOKUP_TOL (0.1)
int Ohmmeter::lookupVoltageIndex(float voltage){
    int i;
    for (i = 0; i < NUM_SELECTION_VALUES; i++) {
	if (fabs(orpx_voltages[i] - voltage) < LOOKUP_TOL*orpx_voltages[i])
	    return i;
    }
    return -1;
}

int Ohmmeter::lookupRangeIndex(float range){
    int i;
    for (i = 0; i < NUM_SELECTION_VALUES; i++) {
	if (fabs(orpx_ranges[i] - range) < LOOKUP_TOL*orpx_ranges[i])
	    return i;
    }
    return -1;
}

int Ohmmeter::lookupFilterIndex(float filter){
    int i;
    for (i = 0; i < NUM_SELECTION_VALUES; i++) {
	if (fabs(orpx_filters[i] - filter) < LOOKUP_TOL*orpx_filters[i])
	    return i;
    }
    return -1;
}

void Ohmmeter::updateDisplay(int chnum, float resistance, int status,
	float voltage, float range, float filter) {
    char *statmsg;
    switch(status) {
	case MEASURING:
	    statmsg = STATUS_OKAY_MSG;
	    break;
	case ITF_ERROR:
	    statmsg = STATUS_ERROR_MSG;
	    break;
	case M_OVERFLO:
	    statmsg = STATUS_SATURATED_MSG;
	    break;
	case M_UNDERFLO:
	    statmsg = STATUS_UNDERFLOW_MSG;
	    break;
	case R_OVERFLO:
	    statmsg = STATUS_OKAY_MSG;
	    break;
	default:
	    statmsg = STATUS_UNKNOWN_MSG;
	    break;
    }
    sprintf(ohm_status_message, "status: %s", statmsg);
    sprintf(ohm_channel_msg, "channel %d", chnum);
	sprintf(ohm_value_msg, "%.4g Ohms", resistance);
	if (resistance >= 1000 && resistance < 1000000){ 
		// print the way Scott wants it
		int highorder = (int)floor(resistance/1000.0);
		int loworder = (int)resistance - highorder*1000;
		sprintf(ohm_value_msg, "%d,%03d Ohms", highorder, loworder);
	}
	else if (resistance >= 1000000 && resistance < 1000000000){
		int highorder = (int)floor(resistance/1000000.0);
		int middleorder = 
			(int)floor((resistance - highorder*1000000)/1000.0);
		int loworder = 
			(int)(resistance-highorder*1000000-middleorder*1000);
		sprintf(ohm_value_msg, "%d,%03d,%03d Ohms", highorder,
			middleorder, loworder);
	}
    Tcl_SetVar(tk_control_interp,OHM_STATUS_VAR,ohm_status_message, 
							TCL_GLOBAL_ONLY);
    Tcl_SetVar(tk_control_interp,OHM_CHANNEL_VAR,ohm_channel_msg,
							TCL_GLOBAL_ONLY);
    Tcl_SetVar(tk_control_interp,OHM_VALUE_VAR,ohm_value_msg,TCL_GLOBAL_ONLY);

    (*channel_select) = channel_strings[chnum];
    int temp = lookupVoltageIndex(voltage);
    if (temp == -1) fprintf(stderr, "Error: unexpected voltage value\n");
    else
    	(*voltage_select) = voltage_strings[temp];
    temp = lookupRangeIndex(range);
    if (temp == -1) fprintf(stderr, "Error: unexpected range value\n");
    else
    	(*range_select) = range_strings[temp];
    temp = lookupFilterIndex(filter);
    if (temp == -1) fprintf(stderr, "Error: unexpected filter value\n");
    else
    	(*filter_select) = filter_strings[temp];
}

void handle_measurement (void *userdata, const vrpn_OHMMEASUREMENTCB &info)
{
	char command[128];
    Ohmmeter *controlPanel = (Ohmmeter *)userdata;
    int ch = info.channel_num;
    OhmmeterChannelParams *cparam = &((controlPanel->c_params)[ch]);
    float fResist = (float)info.resistance;
    controlPanel->updateDisplay(info.channel_num, info.resistance, info.status,
	orpx_voltages[((controlPanel->c_params)[ch]).voltage],
	orpx_ranges[((controlPanel->c_params)[ch]).range],
	orpx_filters[((controlPanel->c_params)[ch]).filter]);

    switch(info.status) {
	case MEASURING:
	  break;
	case ITF_ERROR:
	  break;
	case M_OVERFLO:
	  controlPanel->num_over++;
	  controlPanel->num_under = 0;
	  break;
	case M_UNDERFLO:
	  controlPanel->num_over = 0;
	  controlPanel->num_under++;
	  break;
        case R_OVERFLO:
	  // if maximum range is too low we can't do anything
	  if (((controlPanel->c_params)[ch]).range != NUM_SELECTION_VALUES-1){
	  	controlPanel->num_over++;
	  }
	  controlPanel->num_under = 0;
	  break;
	default:
	  break;
    }

	// indicate error condition with red background
	if (info.status == M_OVERFLO || info.status == ITF_ERROR) {
		sprintf(command, ".french_ohmmeter configure -background LightPink1");
	} else {
		sprintf(command, ".french_ohmmeter configure -background grey");
	}
	if (Tcl_Eval(tk_control_interp, command) != TCL_OK) {
        fprintf(stderr, "Tcl_Eval(%s) failed: %s\n", command,
            tk_control_interp->result);
    }

#ifndef OHMMETER_PROGRAM    // stuff for integrating with microscape as
							// opposed to the separate ohmmeter program
// Tiger    HACK HACK HACK  temporary for compiling vrpn version
#ifdef USE_VRPN_MICROSCOPE
    microscope->RecordResistance(ch, info.msg_time, fResist,
    orpx_voltages[((controlPanel->c_params)[ch]).voltage],
    orpx_ranges[((controlPanel->c_params)[ch]).range],
    orpx_filters[((controlPanel->c_params)[ch]).filter]);
#else
    microscope->RecordResistance(ch, info.msg_time, fResist,
        orpx_voltages[((controlPanel->c_params)[ch]).voltage],
        orpx_ranges[((controlPanel->c_params)[ch]).range],
        orpx_filters[((controlPanel->c_params)[ch]).filter], info.status);
#endif

#endif // NOT SEPARATE OHMMETER PROGRAM

    int new_range = cparam->range;
    if (controlPanel->autorange_enabled) {
	if (controlPanel->num_over > AUTORANGE_WAIT){
	    //printf("autorange overflo\n");
	    if (cparam->range < NUM_SELECTION_VALUES-1)
	    	new_range = cparam->range+1;
	    controlPanel->num_over = 0;
	}
	else if (controlPanel->num_under > AUTORANGE_WAIT){
	    //printf("autorange underflo\n");
	    if (cparam->range > 0) 
		new_range = cparam->range-1;
	    controlPanel->num_under = 0;
	}
	if (new_range != cparam->range){
	    cparam->range = new_range;
	    if (controlPanel->ohmmeterDevice) {
	      controlPanel->ohmmeterDevice->set_channel_parameters(ch,
		cparam->enable, orpx_voltages[cparam->voltage],
		orpx_ranges[cparam->range], orpx_filters[cparam->filter]);
	    }
            if (controlPanel->current_channel == ch)
		*(controlPanel->range_select) =
		    controlPanel->range_strings[cparam->range];

	}
    }
}

/*
void handle_settings_change(void *userdata, const vrpn_OHMSETCB &info)
{
    // info.voltage, range_min, filter, channel_num, enabled

}
*/

static void handle_enable_channel_change(const char *changed, int newval, void *ud)
{
    Ohmmeter *theOhm = (Ohmmeter *)ud;
    OhmmeterChannelParams *cparam;
    int i;

    for (i = 0; i < NUM_OHMMETER_CHANNELS; i++)
	if (strcmp(theOhm->channel_strings[i], changed) == 0) break;
    if (strcmp(theOhm->channel_strings[i], changed) == 0){
	// enable/disable channel i
	cparam = &((theOhm->c_params)[i]);
	cparam->enable = newval;
        if (theOhm->ohmmeterDevice)
            theOhm->ohmmeterDevice->set_channel_parameters(i,
                cparam->enable, orpx_voltages[cparam->voltage],
                orpx_ranges[cparam->range], orpx_filters[cparam->filter]);
    }
}

static void handle_autorange_change (const char * /* changed */, int newval,
                                     void * ud)
{
    Ohmmeter *theOhm = (Ohmmeter *)ud;

    theOhm->autorange_enabled = newval;
}


static void handle_voltage_select_change(const char *newvalue, void *ud)
{
    Ohmmeter *theOhm = (Ohmmeter *)ud;
    int chnum = theOhm->current_channel;
    OhmmeterChannelParams *cparam = &((theOhm->c_params)[chnum]);
    int i;

    for (i = 0; i < NUM_SELECTION_VALUES; i++)
	if (strcmp(theOhm->voltage_strings[i], newvalue) == 0) break;
    if (strcmp(theOhm->voltage_strings[i],newvalue) == 0){
	if (cparam->voltage == i) return;
	cparam->voltage = i;
        if (theOhm->ohmmeterDevice)
            theOhm->ohmmeterDevice->set_channel_parameters(chnum,
                cparam->enable, orpx_voltages[cparam->voltage],
                orpx_ranges[cparam->range], orpx_filters[cparam->filter]);
    }
}

static void handle_range_select_change(const char *newvalue, void *ud)
{
    Ohmmeter *theOhm = (Ohmmeter *)ud;
    int chnum = theOhm->current_channel;
    OhmmeterChannelParams *cparam = &((theOhm->c_params)[chnum]);
    int i;

    for (i = 0; i < NUM_SELECTION_VALUES; i++)
	if (strcmp(theOhm->range_strings[i], newvalue) == 0) break;
    if (strcmp(theOhm->range_strings[i],newvalue) == 0){
	if (cparam->range == i) return;

	cparam->range = i;
	if (theOhm->ohmmeterDevice)
	    theOhm->ohmmeterDevice->set_channel_parameters(chnum,
		cparam->enable, orpx_voltages[cparam->voltage], 
		orpx_ranges[cparam->range], orpx_filters[cparam->filter]);
    }
}

static void handle_filter_select_change(const char *newvalue, void *ud)
{
    Ohmmeter *theOhm = (Ohmmeter *)ud;
    int chnum = theOhm->current_channel;
    OhmmeterChannelParams *cparam = &((theOhm->c_params)[chnum]);
    int i;

    for (i = 0; i < NUM_SELECTION_VALUES; i++)
	if (strcmp(theOhm->filter_strings[i], newvalue) == 0) break;
    if (strcmp(theOhm->filter_strings[i], newvalue) == 0){
	if (cparam->filter == i) return;
	cparam->filter = i;
	if (theOhm->ohmmeterDevice)
	    theOhm->ohmmeterDevice->set_channel_parameters(chnum,
		cparam->enable, orpx_voltages[cparam->voltage],
		orpx_ranges[cparam->range], orpx_filters[cparam->filter]);
    }
}

static void handle_channel_select_change(const char *newvalue, void *ud)
{
    Ohmmeter *theOhm = (Ohmmeter *)ud;
    int i;

    for (i = 0; i < NUM_OHMMETER_CHANNELS; i++)
	if (strcmp(theOhm->channel_strings[i], newvalue) == 0) break;
    if (strcmp(theOhm->channel_strings[i], newvalue) == 0){
	(theOhm->current_channel) = i;
	// change settings to those for channel i
	*(theOhm->voltage_select) =
		theOhm->voltage_strings[(theOhm->c_params)[i].voltage];
	*(theOhm->range_select) =
		theOhm->range_strings[(theOhm->c_params)[i].range];
	*(theOhm->filter_select) =
		theOhm->filter_strings[(theOhm->c_params)[i].filter];
//	*(theOhm->avrgtime_slider) = (theOhm->c_params)[i].avrgtime;
    }
}

/*
static void handle_avrgtime_slider_change(float newvalue, void *ud)
{
    Ohmmeter *theOhm = (Ohmmeter *)ud;
    int chnum = theOhm->current_channel;

    if ((theOhm->c_params)[chnum].avrgtime == newvalue) return;
    (theOhm->c_params)[chnum].avrgtime = newvalue;
    if (theOhm->ohmmeter_enabled)
	microscope->SetOhmmeterChannel(FRENCH_OHMMETER_ID, chnum,
	    (theOhm->c_params)[chnum]);
}
*/

Ohmmeter::Ohmmeter (Tcl_Interp * the_tcl_interp,
                    const char * tcl_script_dir,
                    vrpn_Ohmmeter_Remote * device)
{
    char command[128];
    char command2[64];
    int i;

    ohmmeterDevice = device;

    autorange_enabled = 0;
    num_over = 0;
    num_under = 0;
    //ohmmeter_enabled = 0;
    for (i = 0; i < NUM_SELECTION_VALUES; i++){
    	voltage_strings[i] = strdup(VOLTAGES_STR[i]);
    	range_strings[i] = strdup(RANGES_STR[i]);
    	filter_strings[i] = strdup(FILTER_TIMES_STR[i]);
    }

    for (i = 0; i < NUM_OHMMETER_CHANNELS; i++)
	channel_strings[i] = strdup(CHANNELS_STR[i]);

    tk_control_interp = the_tcl_interp;

    // load widgets so we can make a Tclvar_float_with_scale for setting
    // the averaging time
    // I don't think this should really be here...
    sprintf(command, "source %s%s", tcl_script_dir, TCL_WIDGETS_FILE);
    if (Tcl_Eval(tk_control_interp, command) != TCL_OK) {
	    fprintf(stderr, "Tcl_Eval(%s) failed: %s\n", command,
		    tk_control_interp->result);
	    return;
    }

    list_of_channel_names = new nmb_ListOfStrings();
    for (i = 0; i < NUM_OHMMETER_CHANNELS; i++){
	list_of_channel_names->addEntry(channel_strings[i]);
    }

    list_of_voltages = new nmb_ListOfStrings();
    list_of_ranges = new nmb_ListOfStrings();
    list_of_filters = new nmb_ListOfStrings();

    for (i = 0; i < NUM_SELECTION_VALUES; i++){
        list_of_voltages->addEntry(voltage_strings[i]);
        list_of_ranges->addEntry(range_strings[i]);
        list_of_filters->addEntry(filter_strings[i]);
    }

    // create checkbox to enable ohmmeter
/*    sprintf(command, ".french_ohmmeter");
    ohmmeter_enable_checkbox = new Tclvar_checklist(command);
    sprintf(command, "enable_ohmmeter");
    ohmmeter_enable_checkbox->addEntry(command, 0);
    // Set up a callback from the checklist
    ohmmeter_enable_checkbox->addCallback(handle_enable_ohmmeter_change, this);
*/
    // create a list of checkboxes to enable individual channels
    sprintf(command, ".french_ohmmeter");
    channel_enable_checklist = new Tclvar_checklist(command);
    for (i = 0; i < NUM_OHMMETER_CHANNELS; i++){
	channel_enable_checklist->Add_checkbox(channel_strings[i],0);
    }
    channel_enable_checklist->addCallback(handle_enable_channel_change, this);
    // autorange checkbox
    auto_range_checklist = new Tclvar_checklist(".french_ohmmeter");
    auto_range_checklist->Add_checkbox("autorange", 0);
    auto_range_checklist->addCallback(handle_autorange_change,this);

    // channel string
    sprintf(command, "channel_settings_for");
    channel_select = new Tclvar_string(command, channel_strings[0]);
    channel_select->addCallback(handle_channel_select_change, this);
    current_channel = 0;

    // create frame for settings 
    sprintf(command, "frame .french_ohmmeter.settings %s",
	"-relief raised -bd 5 -background grey");
    if (Tcl_Eval(tk_control_interp, command) != TCL_OK) {
    	fprintf(stderr, "Tcl_Eval(%s) failed: %s\n", command,
            tk_control_interp->result);
	return;
    }

    // Create a label for the settings frame
    sprintf(command, "label .french_ohmmeter.settings.label -text settings");
    if (Tcl_Eval(tk_control_interp, command) != TCL_OK) {
        fprintf(stderr, "Tcl_Eval(%s) failed: %s\n", command,
            tk_control_interp->result);
        return;
    }

    // pack the frame
    sprintf(command, "pack .french_ohmmeter.settings");
    if (Tcl_Eval(tk_control_interp, command) != TCL_OK) {
        fprintf(stderr, "Tcl_Eval(%s) failed: %s\n", command,
            tk_control_interp->result);
        return;
    }

    // pack the label
    sprintf(command, "pack .french_ohmmeter.settings.label");
    if (Tcl_Eval(tk_control_interp, command) != TCL_OK) {
        fprintf(stderr, "Tcl_Eval(%s) failed: %s\n", command,
            tk_control_interp->result);
        return;
    }

    // create pulldown menus for measurement voltage setting,
    // range setting and filter setting

    sprintf(command, "measurement_voltage");
    voltage_select = new Tclvar_string(command, voltage_strings[0]);
    voltage_select->addCallback(handle_voltage_select_change, this);

    sprintf(command, "measurement_range_ohms");
    range_select = new Tclvar_string(command, range_strings[0]);
    range_select->addCallback(handle_range_select_change, this);

    sprintf(command, "filter_sec");
    filter_select = new Tclvar_string(command, filter_strings[0]);
    filter_select->addCallback(handle_filter_select_change, this);

/*    sprintf(command, "averaging_time");
    avrgtime_slider = new Tclvar_float_with_scale(command, 
		command2, 0.1, 20, 0.1, handle_avrgtime_slider_change, this);
*/

    // create a label to display the output
    sprintf(command, "label .french_ohmmeter.outputlabel -text last_measurement");
    if (Tcl_Eval(tk_control_interp, command) != TCL_OK) {
        fprintf(stderr, "Tcl_Eval(%s) failed: %s\n", command,
                tk_control_interp->result);
        return;
    }

    Tcl_SetVar(tk_control_interp, OHM_CHANNEL_VAR, "0", TCL_GLOBAL_ONLY);
    sprintf(command, "label .french_ohmmeter.chanvaluelabel -textvariable %s",
	OHM_CHANNEL_VAR);
    if (Tcl_Eval(tk_control_interp, command) != TCL_OK) {
	fprintf(stderr, "Tcl_Eval(%s) failed: %s\n", command,
		tk_control_interp->result);
	return;
    }

    Tcl_SetVar(tk_control_interp, OHM_VALUE_VAR, "0", TCL_GLOBAL_ONLY);
    sprintf(command, "label .french_ohmmeter.measvaluelabel -textvariable %s",
	OHM_VALUE_VAR);
    if (Tcl_Eval(tk_control_interp, command) != TCL_OK) {
	fprintf(stderr, "Tcl_Eval(%s) failed: %s\n", command,
		tk_control_interp->result);
	return;
    }

    // pack the label
    sprintf(command, "pack .french_ohmmeter.outputlabel .french_ohmmeter.chanvaluelabel .french_ohmmeter.measvaluelabel");
    if (Tcl_Eval(tk_control_interp, command) != TCL_OK) {
        fprintf(stderr, "Tcl_Eval(%s) failed: %s\n", command,
                tk_control_interp->result);
        return;
    }

    // Create a status string variable
    Tcl_SetVar(tk_control_interp,OHM_STATUS_VAR,STATUS_UNKNOWN_MSG,
	TCL_GLOBAL_ONLY);
    // Create a label to display the status string
    sprintf(command, "label .french_ohmmeter.statuslabel -textvariable %s",
	OHM_STATUS_VAR);
    if (Tcl_Eval(tk_control_interp, command) != TCL_OK) {
        fprintf(stderr, "Tcl_Eval(%s) failed: %s\n", command,
                tk_control_interp->result);
        return;
    }

    sprintf(command, "pack .french_ohmmeter.statuslabel");
    if (Tcl_Eval(tk_control_interp, command) != TCL_OK) {
        fprintf(stderr, "Tcl_Eval(%s) failed: %s\n", command,
                tk_control_interp->result);
        return;
    }

    // now if there is an ohmmeter device make sure its settings are 
    // consistent with our initial control panel settings and register
    // a callback for measurements
    if (ohmmeterDevice){
	for (i = 0; i < NUM_OHMMETER_CHANNELS; i++){
	    c_params[i].enable = 0;
	    c_params[i].voltage = 0;
	    c_params[i].range = 0;
	    c_params[i].filter = 0;
	    ohmmeterDevice->set_channel_parameters(i, 0, 
		orpx_voltages[0], orpx_ranges[0], orpx_filters[0]);
	}
	ohmmeterDevice->register_measurement_handler(this,handle_measurement);
    }
}

Ohmmeter::~Ohmmeter(void){
  if (ohmmeter_enable_checkbox) delete ohmmeter_enable_checkbox;
  if (channel_enable_checklist) delete channel_enable_checklist;
  if (auto_range_checklist) delete auto_range_checklist;
  if (channel_select) delete channel_select;
  if (list_of_channel_names) delete list_of_channel_names;
  if (list_of_voltages) delete list_of_voltages;
  if (list_of_ranges) delete list_of_ranges;
  if (list_of_filters) delete list_of_filters;
  if (voltage_select) delete voltage_select;
  if (range_select) delete range_select;
  if (filter_select) delete filter_select;
  if (avrgtime_slider) delete avrgtime_slider;
/*    delete list_of_voltages;
    delete list_of_ranges;
    delete list_of_filters;
    delete voltage_select;
    delete enable;
    delete range_select;
    delete filter_select;
*/
}

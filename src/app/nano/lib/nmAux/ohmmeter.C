#include "string.h"
#include <Tcl_Linkvar.h>
#include <Tcl_Netvar.h>
#include "ohmmeter.h"
#include "orpx.h"	// for parameter values
#include "math.h"

#include <nmm_MicroscopeRemote.h>


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
    char value_msg[128];
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
    (*(d_channelParams[chnum]->status)) = statmsg;

    if (resistance >= 1000 && resistance < 1000000){ 
	// print the way Scott wants it
	int highorder = (int)floor(resistance/1000.0);
	int loworder = (int)resistance - highorder*1000;
	sprintf(value_msg, "%d,%03d Ohms", highorder, loworder);
    } else if (resistance >= 1000000 && resistance < 1000000000){
	int highorder = (int)floor(resistance/1000000.0);
	int middleorder = 
		(int)floor((resistance - highorder*1000000)/1000.0);
	int loworder = 
		(int)(resistance-highorder*1000000-middleorder*1000);
	sprintf(value_msg, "%d,%03d,%03d Ohms", highorder,
		middleorder, loworder);
    } else {
        sprintf(value_msg, "%.4g Ohms", resistance);
    }
    d_channelParams[chnum]->resistance->Set(value_msg);

    
    int temp = lookupVoltageIndex(voltage);
    if (temp == -1) {
        fprintf(stderr, "Error: unexpected voltage value\n");
    } else if ((*(d_channelParams[chnum]->voltage)) != temp){
    	(*(d_channelParams[chnum]->voltage)) = temp;
    }

    temp = lookupRangeIndex(range);
    if (temp == -1) {
        fprintf(stderr, "Error: unexpected range value\n");
    } else if ((*(d_channelParams[chnum]->range)) != temp){
    	(*(d_channelParams[chnum]->range)) = temp;
    }

    temp = lookupFilterIndex(filter);
    if (temp == -1) {
        fprintf(stderr, "Error: unexpected filter value\n");
    } else if ((*(d_channelParams[chnum]->filter)) != temp){
    	(*(d_channelParams[chnum]->filter)) = temp;
    }
}

// static
void Ohmmeter::handle_measurement (void *userdata, 
                                   const vrpn_OHMMEASUREMENTCB &info)
{
    Ohmmeter *controlPanel = (Ohmmeter *)userdata;
    nmui_ORPXChannelParameters *chan = 
                       (controlPanel->d_channelParams)[info.channel_num];
    int channel_index = info.channel_num;
    float fResist = (float)info.resistance;
    controlPanel->updateDisplay(info.channel_num, info.resistance, info.status,
	orpx_voltages[*(chan->voltage)],
	orpx_ranges[*(chan->range)],
	orpx_filters[*(chan->filter)]);

    switch(info.status) {
	case MEASURING:
	  break;
	case ITF_ERROR:
	  break;
	case M_OVERFLO:
	  chan->autorange_num_over++;
	  chan->autorange_num_under = 0;
	  break;
	case M_UNDERFLO:
	  chan->autorange_num_over = 0;
	  chan->autorange_num_under++;
	  break;
        case R_OVERFLO:
	  // if maximum range is too low we can't do anything
	  if (*(chan->range) != NUM_SELECTION_VALUES-1){
	  	chan->autorange_num_over++;
	  }
	  chan->autorange_num_under = 0;
	  break;
	default:
	  break;
    }


    // indicate error condition with red background
    if (info.status == M_OVERFLO || info.status == ITF_ERROR) {
        (*(chan->error)) = 1;
    } else {
	(*(chan->error)) = 0;
    }

    if (controlPanel->microscope_ptr) {
        controlPanel->microscope_ptr->RecordResistance(
            channel_index, info.msg_time, fResist,
            orpx_voltages[*(chan->voltage)],
            orpx_ranges[*(chan->range)],
            orpx_filters[*(chan->filter)]);
    }

    int range_index = *(chan->range);
    if (*(chan->autorange)) {
//        printf("autorange: over: %d, under: %d\n",
//            chan->autorange_num_over, chan->autorange_num_under);
	if (chan->autorange_num_over > AUTORANGE_WAIT){
	    printf("autorange overflo\n");
	    if (range_index < NUM_SELECTION_VALUES-1)
	    	range_index++;
	    chan->autorange_num_over = 0;
	}
	else if (chan->autorange_num_under > AUTORANGE_WAIT){
	    printf("autorange underflo\n");
	    if (range_index > 0) 
		range_index--;
	    chan->autorange_num_over = 0;
	}
	if (range_index != *(chan->range)){
	    *(chan->range) = range_index;
	    if (controlPanel->ohmmeterDevice) {
	      controlPanel->ohmmeterDevice->set_channel_parameters(
                channel_index, *(chan->enable),
		orpx_voltages[*(chan->voltage)],
		orpx_ranges[*(chan->range)], orpx_filters[*(chan->filter)]);
	    }
            *(chan->range) = range_index;
	}
    }
}

// used to setup synchronization on protected
// TclNet member variables
void Ohmmeter::ohm_SetupSync(nmui_Component * container)
{
  for(int i = 0; i < NUM_OHMMETER_CHANNELS; i++) {
    container->add(d_channelParams[i]->enable);
    container->add(d_channelParams[i]->autorange);
    container->add(d_channelParams[i]->voltage);
    container->add(d_channelParams[i]->range);
    container->add(d_channelParams[i]->filter);
    container->add(d_channelParams[i]->resistance);
    container->add(d_channelParams[i]->status);
    container->add(d_channelParams[i]->error);
  }
}

// used to teardown synchronization on protected
// TclNet member variables
void Ohmmeter::ohm_TeardownSync(nmui_Component * container)
{
  for(int i = 0; i < NUM_OHMMETER_CHANNELS; i++) {
    container->remove(d_channelParams[i]->enable);
    container->remove(d_channelParams[i]->autorange);
    container->remove(d_channelParams[i]->voltage);
    container->remove(d_channelParams[i]->range);
    container->remove(d_channelParams[i]->filter);
    container->remove(d_channelParams[i]->resistance);
    container->remove(d_channelParams[i]->status);
    container->remove(d_channelParams[i]->error);
  }
}

// static 
void nmui_ORPXChannelParameters::handle_param_change(
    vrpn_int32 /*new_value*/, void *ud)
{
    nmui_ORPXChannelParameters *me =
        (nmui_ORPXChannelParameters *)ud;

    if (me->device){
        me->device->set_channel_parameters(me->channel_id,
               *(me->enable), orpx_voltages[*(me->voltage)],
               orpx_ranges[*(me->range)], orpx_filters[*(me->filter)]);
    }
}

char *nmui_ORPXChannelParameters::enable_var_base_name =
                                         "french_ohmmeter_enable_";
char *nmui_ORPXChannelParameters::autorange_var_base_name = 
                                         "french_ohmmeter_autorange_";
char *nmui_ORPXChannelParameters::voltage_var_base_name =
                                         "french_ohmmeter_voltage_";
char *nmui_ORPXChannelParameters::range_var_base_name =
                                         "french_ohmmeter_range_";
char *nmui_ORPXChannelParameters::filter_var_base_name =
                                         "french_ohmmeter_filter_";
char *nmui_ORPXChannelParameters::resistance_var_base_name =
                                         "french_ohmmeter_resistance_";
char *nmui_ORPXChannelParameters::status_var_base_name =
                                         "french_ohmmeter_status_";
char *nmui_ORPXChannelParameters::error_var_base_name =
                                         "french_ohmmeter_error_";

nmui_ORPXChannelParameters::nmui_ORPXChannelParameters(
                        const char *channel_name, 
                        int c_id, vrpn_Ohmmeter_Remote *d)
{
    autorange_num_over = 0;
    autorange_num_under = 0;
    channel_id = c_id;
    device = d;

    char var_name[128];

    // we only set up callbacks for variables that need to be sent to
    // the device
    strcpy(var_name, enable_var_base_name);
    strcat(var_name, channel_name);
    enable = new TclNet_int(var_name, 0);
    enable->addCallback(handle_param_change, this);

    strcpy(var_name, autorange_var_base_name);
    strcat(var_name, channel_name);
    autorange = new TclNet_int(var_name, 0);

    strcpy(var_name, voltage_var_base_name);
    strcat(var_name, channel_name);
    voltage = new TclNet_int(var_name, 0);
    voltage->addCallback(handle_param_change, this);

    strcpy(var_name, range_var_base_name);
    strcat(var_name, channel_name);
    range = new TclNet_int(var_name, 0);
    range->addCallback(handle_param_change, this);

    strcpy(var_name, filter_var_base_name);
    strcat(var_name, channel_name);
    filter = new TclNet_int(var_name, 0);
    filter->addCallback(handle_param_change, this);

    strcpy(var_name, resistance_var_base_name);
    strcat(var_name, channel_name);
    resistance = new TclNet_string(var_name, "0.000");

    strcpy(var_name, status_var_base_name);
    strcat(var_name, channel_name);
    status = new TclNet_string(var_name, "unknown");

    strcpy(var_name, error_var_base_name);
    strcat(var_name, channel_name);
    error = new TclNet_int(var_name, 0);

    if (!enable || !autorange || !voltage || !range || !filter || 
        !resistance || !status || !error) {
        fprintf(stderr, "nmui_ORPXChannelParameters:ctor: Error, "
                        "out of memory\n");
    }
}

nmui_ORPXChannelParameters::~nmui_ORPXChannelParameters()
{
    if (enable) delete enable;
    if (autorange) delete autorange;
    if (voltage) delete voltage;
    if (range) delete range;
    if (filter) delete filter;
    if (resistance) delete resistance;
    if (status) delete status;
    if (error) delete error;
}

char *Ohmmeter::s_channel_names[NUM_OHMMETER_CHANNELS] =
      {"ch0", "ch1", "ch2", "ch3"};
char *Ohmmeter::s_voltage_strings[NUM_SELECTION_VALUES] =
      {"3uV","10uV","30uV","100uV", "300uV","1mV"};
char *Ohmmeter::s_range_strings[NUM_SELECTION_VALUES] =
      {">0.5",">5",">50",">500",">5k",">50k"};
char *Ohmmeter::s_filter_strings[NUM_SELECTION_VALUES] =
      {"0.1", "0.3", "1.0", "3", "10", "30"};

Ohmmeter::Ohmmeter (Tcl_Interp * the_tcl_interp,
                    const char * /*tcl_script_dir*/,
                    vrpn_Ohmmeter_Remote * device):
    d_windowOpen("french_ohmmeter_open", 0),
    d_voltages("french_ohmmeter_voltages"),
    d_ranges("french_ohmmeter_ranges"),
    d_filters("french_ohmmeter_filters")
{

    int i;

    ohmmeterDevice = device;
    microscope_ptr = NULL;

    tk_control_interp = the_tcl_interp;

    d_voltages.clearList();
    d_ranges.clearList();
    d_filters.clearList();

    for (i = 0; i < NUM_SELECTION_VALUES; i++){
        d_voltages.addEntry(s_voltage_strings[i]);
        d_ranges.addEntry(s_range_strings[i]);
        d_filters.addEntry(s_filter_strings[i]);
    }

    for (i = 0; i < NUM_OHMMETER_CHANNELS; i++){
        d_channelParams[i] = 
            new nmui_ORPXChannelParameters(s_channel_names[i], i, device);
    }

    if (ohmmeterDevice){
	for (i = 0; i < NUM_OHMMETER_CHANNELS; i++){
	    *(d_channelParams[i]->enable) = 0;
	    *(d_channelParams[i]->voltage) = 0;
	    *(d_channelParams[i]->range) = 0;
	    *(d_channelParams[i]->filter) = 0;
	    ohmmeterDevice->set_channel_parameters(i, 0, 
		orpx_voltages[0], orpx_ranges[0], orpx_filters[0]);
	}
	ohmmeterDevice->register_measurement_handler(this,handle_measurement);
    }
}

Ohmmeter::~Ohmmeter(void){
    int i;
    for (i = 0; i < NUM_OHMMETER_CHANNELS; i++){
        delete d_channelParams[i];
    }
}

void Ohmmeter::setMicroscope(nmm_Microscope_Remote *m){
    microscope_ptr = m;
}


vrpn_bool Ohmmeter::windowOpen()
{
    return d_windowOpen;
}

void Ohmmeter::openWindow() 
{
    char *command = "show.french_ohmmeter";
    if (Tcl_Eval(tk_control_interp, command) != TCL_OK) {
        fprintf(stderr, "Tcl_Eval(%s) failed: %s\n", command,
               tk_control_interp->result);
    }
}

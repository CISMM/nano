//*******************************************************************
// VRPN client for a Keithley 2400 meter, which does voltage or current 
// sweeps and measures current or voltage. Used to measure resistance
// at a variety of voltages, typically. 

//*******************************************************************

#ifdef _win32
#include <windows.h>
#else
#include <stdlib.h>
#endif

#include <stdio.h>

#include "nmb_Util.h"
#include "nma_Keithley2400.h"

#define CHECK(a) if ((a) == -1) return -1


nma_Keithley2400::nma_Keithley2400(const char *name, vrpn_Connection *c) :
	vrpn_GPIBDevice(name, c),
	d_board_index(0),
	d_primary_address(24),
	d_secondary_address(0),
	d_initialized(0),
	d_output_on(0),
	d_repeat_curve_active(0),
		// Set default values for a normal voltage sweep.
	d_source(VOLTAGE), 
	d_compliance(CURRENT),
	d_compliance_val((float)0.1),
	d_num_power_line_cycles((float)1.0),
	d_sweep(LINEAR),
	d_sweep_start((float)-1.0),
	d_sweep_stop((float)1.0),
	d_sweep_numpoints(21),
	d_sweep_delay((float)0.01),
	d_display_enable(1), 

	result_change_list(NULL),
	error_change_list(NULL)

{
  if ( !d_connection ) {
    fprintf(stderr, "No live connection to %s.\n", name );
    exit( 0 );
  }

  d_connection->register_handler(d_Result_type,
				handle_Result,
				this);
  d_connection->register_handler(d_ResultData_type,
				handle_ResultData,
				this);
  d_connection->register_handler(d_Error_type,
				handle_Error,
				this);

  d_connection->register_handler(d_Shutdown_type,
				handle_Shutdown,
				this);

}

int nma_Keithley2400::mainloop (const struct timeval * timeout ) {
  if (d_connection)
    CHECK(d_connection->mainloop(timeout));

  return 0;
}

int nma_Keithley2400::send_AllSettings() {
    	if (send_OutputOff()) return -1;
	if (send_Clear()) return -1;
	if (send_DisplayEnable()) return -1;
	if (send_Function()) return -1;
	if (send_Source()) return -1;
	if (send_Sense()) return -1;
	if (send_Trigger()) return -1;
	if (send_DataFormat()) return -1;
	if (send_OutputOn()) return -1;
	return 0;
}

int nma_Keithley2400::send_Device() 
{
	vrpn_int32 len;
	char * msgbuf = encode_Device(&len, d_board_index, d_primary_address, d_secondary_address);
	if (msgbuf == NULL) {
		fprintf(stderr, "nma_Keithley2400::send_Device: out of memory.\n");
		return -1;
	}
	if(Send(len, d_Device_type, msgbuf)) {
		fprintf(stderr, "nma_Keithley2400::send_Device: couldn't send message.\n");
		return -1;
	}
	// Clear the instrument right away.
	if(Send(0, d_Clear_type, NULL)) {
		fprintf(stderr, "nma_Keithley2400::send_Device: couldn't send message.\n");
		return -1;
	}
	// Remember that we have initialized the device.
	d_initialized = 1;
	return 0;
}

// This may be a more authoritive clear than the one sent in the Device command.
int nma_Keithley2400::send_Clear() 
{
	if (!d_initialized) { 
		fprintf(stderr, "send_Clear - Keithley not initialized, not sending\n");
		return -1;
	}
	vrpn_int32 len;
	char buf[]  = ":ABOR;*CLS;*RST";
	char * msgbuf = encode_Write(&len, buf);
	if (msgbuf == NULL) {
		fprintf(stderr, "nma_Keithley2400::send_Clear: out of memory.\n");
		return -1;
	}
	if(Send(len, d_Write_type, msgbuf)) {
		fprintf(stderr, "nma_Keithley2400::send_Clear: couldn't send message.\n");
		return -1;
	}

	return 0;
}

int nma_Keithley2400::send_DisplayEnable()
{
	if (!d_initialized) { 
		fprintf(stderr, "send_Function - Keithley not initialized, not sending\n");
		return -1;
	}
	vrpn_int32 len;
	char  buf[256];
	if (d_display_enable) {
		sprintf(buf, ":DISPLAY:ENABLE ON");
	} else {
		sprintf(buf, ":DISPLAY:ENABLE OFF");
	} 
	char * msgbuf = encode_Write(&len, buf);
	if (msgbuf == NULL) {
		fprintf(stderr, "nma_Keithley2400::send_Function: out of memory.\n");
		return -1;
	}
	if(Send(len, d_Write_type, msgbuf)) {
		fprintf(stderr, "nma_Keithley2400::send_Function: couldn't send message.\n");
		return -1;
	}

	return 0;

}

int nma_Keithley2400::send_Function() 
{
	if (!d_initialized) { 
		fprintf(stderr, "send_Function - Keithley not initialized, not sending\n");
		return -1;
	}
	vrpn_int32 len;
	char  buf[256];
	if (d_source == VOLTAGE) {
		sprintf(buf, ":FUNC:CONC OFF;:FUNC 'CURR';");
	} else if (d_source == CURRENT) {
		sprintf(buf, ":FUNC:CONC OFF;:FUNC 'VOLT';");
	} else {
	    fprintf(stderr, "nma_Keithley2400::send_Function: unknown source type.\n");
	    return -1; 
	}
	char * msgbuf = encode_Write(&len, buf);
	if (msgbuf == NULL) {
		fprintf(stderr, "nma_Keithley2400::send_Function: out of memory.\n");
		return -1;
	}
	if(Send(len, d_Write_type, msgbuf)) {
		fprintf(stderr, "nma_Keithley2400::send_Function: couldn't send message.\n");
		return -1;
	}

	return 0;
}

int nma_Keithley2400::send_Source() 
{
	if (!d_initialized) { 
		fprintf(stderr, "send_Source - Keithley not initialized, not sending\n");
		return -1;
	}

	// XXX This is wrong, doesn't work for log sweeps - need to switch to numpoints.
	vrpn_int32 len;
	char buf[256] ;
	if (d_source == VOLTAGE) {
		sprintf(buf, ":Sour1:Func Volt;:Sour1:Volt:Mode SWE;:Sour1:Volt:Rang:Auto On;:Sour1:Swe:Dir Up;Spac %s;:Sour1:Volt:Start %g;Stop %g;:Sour1:Swe:Poin %d;", 
			((d_sweep == LINEAR)? "LIN":"LOG"), (float)d_sweep_start,(float)d_sweep_stop, (int)d_sweep_numpoints);

	} else if (d_source == CURRENT) {
		sprintf(buf, ":Sour1:Func Curr;:Sour1:Curr:Mode SWE;:Sour1:Curr:Rang:Auto On;:Sour1:Swe:Dir Up;Spac %s;:Sour1:Curr:Start %g;Stop %g;:Sour1:Swe:Poin %d;", 
			((d_sweep == LINEAR)? "LIN":"LOG"), (float)d_sweep_start,(float)d_sweep_stop, (int)d_sweep_numpoints);
	} else {
	    fprintf(stderr, "nma_Keithley2400::send_Source: unknown source type.\n");
	    return -1; 
	}
	char * msgbuf = encode_Write(&len, buf);
	if (msgbuf == NULL) {
		fprintf(stderr, "nma_Keithley2400::send_Source: out of memory.\n");
		return -1;
	}
	if(Send(len, d_Write_type, msgbuf)) {
		fprintf(stderr, "nma_Keithley2400::send_Source: couldn't send message.\n");
		return -1;
	}

	return 0;
}

int nma_Keithley2400::send_Sense() 
{
	if (!d_initialized) { 
		fprintf(stderr, "send_Sense - Keithley not initialized, not sending\n");
		return -1;
	}

	vrpn_int32 len;
	char buf[256] ;
	if (d_compliance == CURRENT) {
		sprintf(buf, ":SENS:CURR:NPLC %g;Prot %g;:CURR:RANG:AUTO ON;", 
			(float)d_num_power_line_cycles, (float)d_compliance_val);

	} else if (d_compliance == VOLTAGE){
		sprintf(buf, ":SENS:VOLT:NPLC %g;Prot %g;:VOLT:RANG:AUTO ON;", 
			(float)d_num_power_line_cycles, (float)d_compliance_val);
	} else {
	    fprintf(stderr, "nma_Keithley2400::send_Sense: unknown compliance type.\n");
	    return -1;
	}
	char * msgbuf = encode_Write(&len, buf);
	if (msgbuf == NULL) {
		fprintf(stderr, "nma_Keithley2400::send_Sense: out of memory.\n");
		return -1;
	}
	if(Send(len, d_Write_type, msgbuf)) {
		fprintf(stderr, "nma_Keithley2400::send_Sense: couldn't send message.\n");
		return -1;
	}

	return 0;
}

int nma_Keithley2400::send_Trigger() 
{
	if (!d_initialized) { 
		fprintf(stderr, "send_Trigger - Keithley not initialized, not sending\n");
		return -1;
	}

	vrpn_int32 len;
	char buf[256] ;
	sprintf(buf, ":TRIG:COUN %d;SOUR IMM;DEL %g;TCON:DIR SOUR;", 
		(int)(d_sweep_numpoints), (float)d_sweep_delay);
	char * msgbuf = encode_Write(&len, buf);
	if (msgbuf == NULL) {
		fprintf(stderr, "nma_Keithley2400::send_Trigger: out of memory.\n");
		return -1;
	}
	if(Send(len, d_Write_type, msgbuf)) {
		fprintf(stderr, "nma_Keithley2400::send_Trigger: couldn't send message.\n");
		return -1;
	}

	return 0;
}

int nma_Keithley2400::send_DataFormat() 
{
	if (!d_initialized) { 
		fprintf(stderr, "send_DataFormat - Keithley not initialized, not sending\n");
		return -1;
	}

	vrpn_int32 len;
	char buf[256] ;
	// XXX This requests binary data, so we had better call send_ReadFloatData later!
	// Always read both voltage and current, so that we 
	// can tell what was measured in the stream file. 
	sprintf(buf, ":FORM:ELEM VOLT,CURR;DATA SREAL;BORD SWAP;");
	char * msgbuf = encode_Write(&len, buf);
	if (msgbuf == NULL) {
		fprintf(stderr, "nma_Keithley2400::send_DataFormat: out of memory.\n");
		return -1;
	}
	if(Send(len, d_Write_type, msgbuf)) {
		fprintf(stderr, "nma_Keithley2400::send_DataFormat: couldn't send message.\n");
		return -1;
	}

	return 0;
}

int nma_Keithley2400::send_OutputOn() 
{
	if (!d_initialized) { 
		fprintf(stderr, "send_OutputOn - Keithley not initialized, not sending\n");
		return -1;
	}

	vrpn_int32 len;
	char buf[256] ;
	sprintf(buf, ":Outp:Smode NORM;Interlock:State Off;:Outp On");
	char * msgbuf = encode_Write(&len, buf);
	if (msgbuf == NULL) {
		fprintf(stderr, "nma_Keithley2400::send_OutputOn: out of memory.\n");
		return -1;
	}
	if(Send(len, d_Write_type, msgbuf)) {
		fprintf(stderr, "nma_Keithley2400::send_OutputOn: couldn't send message.\n");
		return -1;
	}
	// Remember that the output is on.
	d_output_on = 1;
	return 0;
}

int nma_Keithley2400::send_AcquireData() 
{
	if (!d_initialized) { 
		fprintf(stderr, "send_StartCurve - Keithley not initialized, not sending\n");
		return -1;
	}

	vrpn_int32 len;
	char buf[256] ;

	if (! d_output_on) {
		printf("send_StartCurve: Ouput not on - did you really want to do this?\n");
	}
	sprintf(buf, ":READ?");
	char * msgbuf = encode_Write(&len, buf);
	if (msgbuf == NULL) {
		fprintf(stderr, "nma_Keithley2400::send_StartCurve: out of memory.\n");
		return -1;
	}
	if(Send(len, d_Write_type, msgbuf)) {
		fprintf(stderr, "nma_Keithley2400::send_StartCurve:"
			" couldn't send message.\n");
		return -1;
	}

	return 0;
}

int nma_Keithley2400::send_ReadData() 
{
	if (!d_initialized) { 
		fprintf(stderr, "send_GetData - Keithley not initialized, not sending\n");
		return -1;
	}

	vrpn_int32 len;
	
	// I think there are 14 ascii characters per data point, 
	// so overestimate a bit. Both voltage and current measured
	char * msgbuf = encode_Read(&len, d_sweep_numpoints*32);
	if (msgbuf == NULL) {
		fprintf(stderr, "nma_Keithley2400::send_GetData: out of memory.\n");
		return -1;
	}
	if(Send(len, d_Read_type, msgbuf)) {
		fprintf(stderr, "nma_Keithley2400::send_GetData: couldn't send message.\n");
		return -1;
	}

	return 0;
}

int nma_Keithley2400::send_ReadFloatData() 
{
	if (!d_initialized) { 
		fprintf(stderr, "send_GetData - Keithley not initialized, not sending\n");
		return -1;
	}

	vrpn_int32 len;
	// Send number of floats
	char * msgbuf = encode_ReadData(&len, 2*d_sweep_numpoints);
	if (msgbuf == NULL) {
		fprintf(stderr, "nma_Keithley2400::send_GetData: out of memory.\n");
		return -1;
	}
	if(Send(len, d_ReadData_type, msgbuf)) {
		fprintf(stderr, "nma_Keithley2400::send_GetData: couldn't send message.\n");
		return -1;
	}

	return 0;
}

int nma_Keithley2400::send_DoCurve() 
{
    if (send_AcquireData()) return -1;
    return(send_ReadFloatData());
}

int nma_Keithley2400::send_OutputOff() 
{
	if (!d_initialized) { 
		fprintf(stderr, "send_OutputOff - Keithley not initialized, not sending\n");
		return -1;
	}

	vrpn_int32 len;
	char buf[256] ;
	sprintf(buf, ":Outp Off");
	char * msgbuf = encode_Write(&len, buf);
	if (msgbuf == NULL) {
		fprintf(stderr, "nma_Keithley2400::send_OutputOff: out of memory.\n");
		return -1;
	}
	if(Send(len, d_Write_type, msgbuf)) {
		fprintf(stderr, "nma_Keithley2400::send_OutputOff: couldn't send message.\n");
		return -1;
	}
	// Remember that we turned output off. 
	d_output_on = 0;

	return 0;
}

int nma_Keithley2400::register_result_handler(void *userdata,
			    vrpn_VIRESULTDATAHANDLER handler)
{
    vrpn_RESULTDATALIST *new_entry;

    if (handler == NULL) {
        fprintf(stderr, "nma_Keithley2400::reg_handler:NULL handler\n");
        return -1;
    }
    if ((new_entry = new vrpn_RESULTDATALIST) == NULL) {
        fprintf(stderr, "nma_Keithley2400::reg_handler:out of memory\n");
        return -1;
    }
    new_entry->handler = handler;
    new_entry->userdata = userdata;
    
    new_entry->next = result_change_list;
    result_change_list = new_entry;
    return 0;
}

int nma_Keithley2400::unregister_result_handler(void *userdata,
			      vrpn_VIRESULTDATAHANDLER handler)
{
    vrpn_RESULTDATALIST *victim, **snitch;

    snitch = &result_change_list;
    victim = *snitch;
    while ( (victim != NULL) &&
        ( (victim->handler != handler) ||
          (victim->userdata != userdata) )) {
          snitch = &( (*snitch)->next );
          victim = victim->next;
    }   
    
    if (victim == NULL) {
        fprintf(stderr,"nma_Keithley2400::unreg_handler: No such handler\n");
        return -1;
    }
    // Remove the entry from the list
    *snitch = victim->next;
    delete victim;
    
    return 0;

}

int nma_Keithley2400::rcv_Error(char * my_buf)
{
	if(my_buf) {
		printf("Error: %s\n", my_buf);
		delete [] my_buf;
	}
	// An error clears the Keithley, so it's no longer initialized. 
	d_output_on = 0;
	d_initialized = 0;
	d_repeat_curve_active = 0;
	return 0;
}

int nma_Keithley2400::rcv_Shutdown()
{
	printf("Lost connection, exiting...\n");
	exit(0);
	return 0;
}

//*********
// Message handlers

// XXX does not perform any callbacks. Request for data assumes
// handle_ResultData will be called.
//static 
int nma_Keithley2400::handle_Result( void *_userdata, vrpn_HANDLERPARAM _p )
{
	nma_Keithley2400 * me = (nma_Keithley2400 *)_userdata;
	char * my_buf;
	
	me->decode_Result(&_p.buffer, &my_buf);
	if(my_buf) {
		printf("Result: %s\n", my_buf);
		delete [] my_buf;
	}

	// Do repeated IV curves, if we are set to do so. 
	if (me->d_repeat_curve_active) {
	    me->send_DoCurve();
	}
	return 0;
}

//static 
int nma_Keithley2400::handle_ResultData( void *_userdata, vrpn_HANDLERPARAM _p )
{
	nma_Keithley2400 * me = (nma_Keithley2400 *)_userdata;
	vrpn_int32 len; 
	float * my_data;
	
	me->decode_ResultData(&_p.buffer, &my_data, &len);
	/* DEBUG print results received.
	for (vrpn_int32 i = 0; i< len; i++) {
		printf("%g ", my_data[i]);
	}
	printf("\n");
	*/

	// Yank some callbacks. 
	vrpn_VIRESULTDATACB info;
	vrpn_RESULTDATALIST * handler = me->result_change_list;

	info.msg_time = _p.msg_time;
	info.num_values = len;
	info.data = my_data;

    while (handler != NULL) {
	handler->handler(handler->userdata, info);
	handler = handler->next;
    }

	if (my_data)
		delete [] my_data;

	// Do repeated IV curves, if we are set to do so. 
	if (me->d_repeat_curve_active) {
	    me->send_DoCurve();
	}
	return 0 ;
}

// XXX Does not perform any callbacks.
//static 
int nma_Keithley2400::handle_Error( void *_userdata, vrpn_HANDLERPARAM _p )
{
	nma_Keithley2400 * me = (nma_Keithley2400 *)_userdata;
	char * my_buf;
	
	me->decode_Error(&_p.buffer, &my_buf);
	return(me->rcv_Error(my_buf));
}

//static 
int nma_Keithley2400::handle_Shutdown( void *_userdata, vrpn_HANDLERPARAM )
{
	nma_Keithley2400 * me = (nma_Keithley2400 *)_userdata;

	return(me->rcv_Shutdown());
}

int nma_Keithley2400::Send( vrpn_int32 len, vrpn_int32 msg_type, char * buf )
{
  struct timeval now;
  int retval;

  gettimeofday(&now, NULL);
  // DON'T check for a null buffer - we want to send the message even if
  // it has no contents - it might be important, after all. 
  if ((!d_connection) || (!d_connection->doing_okay())) {
    //fprintf(stderr, "Send(): connection is not ok!!!\n");
    return 0;
  }
    retval = d_connection->pack_message(len, now, msg_type, d_myId,
					buf, vrpn_CONNECTION_RELIABLE);
  if ( buf ) 
    delete [] buf;

  return retval;
}


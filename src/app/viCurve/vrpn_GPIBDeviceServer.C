

#include <windows.h>
#include "decl-32.h"
#include <stdio.h>

#include "vrpn_GPIBDeviceServer.h"

#define CHECK(a) if ((a) == -1) return -1


//**************************************************************************
// vrpn_GPIBDeviceServer
//**************************************************************************

vrpn_GPIBDeviceServer::vrpn_GPIBDeviceServer(const char *name, vrpn_Connection *c) :
        nmb_Device_Server(name, c),
	vrpn_GPIBDevice(name, d_connection)
{
  if ( !d_connection ) {
    fprintf(stderr, "No live connection to %s.\n", name );
    exit( 0 );
  }
  d_connection->register_handler(d_Device_type,
				handle_Device,
				this);
  d_connection->register_handler(d_Clear_type,
				handle_Clear,
				this);
  d_connection->register_handler(d_Write_type,
				handle_Write,
				this);
  d_connection->register_handler(d_Read_type,
				handle_Read,
				this);
  d_connection->register_handler(d_ReadData_type,
				handle_ReadData,
				this);
  d_connection->register_handler(d_Shutdown_type,
				handle_Shutdown,
				this);
  init_ids();

}

int vrpn_GPIBDeviceServer::mainloop (const struct timeval * timeout) {
  if (d_connection)
    CHECK(d_connection->mainloop(timeout));

  return 0;
}

int vrpn_GPIBDeviceServer::rcv_Device(int board_index, int primary_address, int secondary_address)
{
    vrpn_int32 index = get_device_id_index( primary_address, secondary_address );
    if ( index == -1 ) { // Connecting to new device:
	d_device_id[d_num_devices] = ibdev(           // Create a unit descriptor handle         
         board_index,              // Board Index (GPIB0 = 0, GPIB1 = 1, ...) 
         primary_address,          // Device primary address                  
         secondary_address,        // Device secondary address                
         TNONE,                    // Timeout setting (T10s = 10 seconds, T1000s = 1000 seconds, TNONE = never timeout)
         1,                       // Assert EOI line at end of write         
         0);                      // EOS termination mode                    
	d_primary_address[d_num_devices] = primary_address;
	d_secondary_address[d_num_devices] = secondary_address;
	d_num_devices++;
    }
    else {                   // Re-Connecting to existing device:
	d_device_id[index] = ibdev(           // Create a unit descriptor handle         
         board_index,              // Board Index (GPIB0 = 0, GPIB1 = 1, ...) 
         primary_address,          // Device primary address                  
         secondary_address,        // Device secondary address                
         TNONE,                    // Timeout setting (T10s = 10 seconds)     
         1,                       // Assert EOI line at end of write         
         0);                      // EOS termination mode                    
	d_primary_address[index] = primary_address;
	d_secondary_address[index] = secondary_address;
    }
	if (ibsta & ERR) {             // Check for GPIB Error  
	    GpibError( d_primary_address[index], d_secondary_address[index], "ibdev Error"); 
	}
	d_board_index = board_index;
	return 0;
}

int vrpn_GPIBDeviceServer::rcv_Clear( vrpn_int32 pad, vrpn_int32 sad)
{
    int index = get_device_id_index(pad, sad);
    if ( index == -1 )
	return -1;
    ibclr(d_device_id[index]);                 // Clear the device 
    if (ibsta & ERR) {
	GpibError( d_primary_address[index], d_secondary_address[index], "ibclr Error"); 
    }
	return 0;
}

int vrpn_GPIBDeviceServer::rcv_Write(vrpn_int32 pad, vrpn_int32 sad, char * my_buf)
{
    int index = get_device_id_index(pad, sad);
    if ( index == -1 )
	return -1;
    if(my_buf) {
	ibwrt(d_device_id[index], my_buf, strlen(my_buf));
	if (ibsta & ERR) {
	    GpibError( d_primary_address[index], d_secondary_address[index], "ibwrt Error"); 
	}
	delete [] my_buf;
    }
    
    return 0;
}

int vrpn_GPIBDeviceServer::rcv_Read(vrpn_int32 pad, vrpn_int32 sad, int bytes)
{
    // allow 1 extra character for null-termination
    char * buffer = new char[bytes+1];
    vrpn_int32 len;
    if (buffer == NULL) {
	fprintf(stderr, "vrpn_GPIBDeviceServer::rcv_Read: out of memory\n");
	return -1;
    }
    int index = get_device_id_index(pad, sad);
    if ( index == -1 )
	return -1;
    ibrd(d_device_id[index], buffer, bytes);   
    if (ibsta & ERR) {
	GpibError( d_primary_address[index], d_secondary_address[index], "ibrd Error"); 
    }
    
    buffer[ibcntl] = '\0';         // Null terminate the ASCII string 
    
    //printf("%s\n", Buffer);       
    
    // Send result of read to the client.
    char * msgbuf = encode_Result(&len, pad, sad, buffer);
    if (msgbuf == NULL) {
	fprintf(stderr, "vrpn_GPIBDeviceServer::rcv_Read: out of memory.\n");
	return -1;
    }
    if(Send(len, d_Result_type, msgbuf)) {
	fprintf(stderr, "vrpn_GPIBDeviceServer::rcv_Read: couldn't send message.\n");
	return -1;
    }
    
    if (buffer) delete [] buffer;
    
    return 0;
}

// Parameter specifies the max number of 4 byte floats to read from device.
// XXX Should be modified to allow for a variable header and trailer, instead
// of hardcoding the 2byte header and 1byte trailer
int vrpn_GPIBDeviceServer::rcv_ReadData(vrpn_int32 pad, vrpn_int32 sad, int data_len)
{
	int buf_len = sizeof(vrpn_float32)*data_len + 3;
	char * buffer = new char[buf_len];
	vrpn_float32 * data_buf;
	if (buffer == NULL) {
		fprintf(stderr, "vrpn_GPIBDeviceServer::rcv_ReadData: out of memory\n");
		return -1;
	}
	// Add 2 bytes for the standard header of "#0" and 
	// one byte for the terminal end-of-data character
	int index = get_device_id_index(pad, sad);
	if ( index == -1 ) {
	    fprintf(stderr,"vrpn_GPIBDeviceServer::rcv_ReadData: Bad device index\n");
	    return -1;
	}
	ibrd(d_device_id[index], buffer, buf_len);   
	if (ibsta & ERR) {
	    GpibError( d_primary_address[index], d_secondary_address[index], "ibrd Error"); 
	    fprintf(stderr,"vrpn_GPIBDeviceServer::rcv_ReadData: Error flag set in GPIB call\n");
	    return -1;
	}
	// Check the standard header
	if ((buffer[0] != '#') || (buffer[1] != '0')) {
		fprintf(stderr,"vrpn_GPIBDeviceServer::rcv_ReadData: Standard header not received, data may be corrupt.\n");
		return -1;
	}

	// ibcntl tells us the actual number of bytes read. Subract two for header and one for the 
	// End-of-data (EOS) character.
	long data_read_len = ibcntl - 3;
	if (data_read_len % sizeof(vrpn_float32) != 0) {
		fprintf(stderr,"vrpn_GPIBDeviceServer::rcv_ReadData: Data not terminated correctly, may be truncated.\n");
		return -1;
	}
	// Translate length into number of floats, rounding down if there's extra bytes in the buffer.
	data_read_len /= sizeof(vrpn_float32);

	// Move to the 3rd byte in the char buffer to get the float data
	data_buf = (vrpn_float32 *) &(buffer[2]);
	//DEBUG print result of read.
	//for (long i = 0; i < data_read_len; i++)
	//	printf("%g ",data_buf[i]);
	//printf("\n");

	// Send result of read to the client.
	vrpn_int32 len;
	char * msgbuf = encode_ResultData(&len, pad, sad, data_buf, data_read_len);
	if (msgbuf == NULL) {
		fprintf(stderr, "vrpn_GPIBDeviceServer::rcv_ReadData: Error in encode_ResultData.\n");
		return -1;
	}
	if(Send(len, d_ResultData_type, msgbuf)) {
		fprintf(stderr, "vrpn_GPIBDeviceServer::rcv_ReadData: couldn't send message.\n");
		return -1;
	}

	if (buffer) delete [] buffer;

	return 0;
}


// When the connection shuts down, close the instrument we are talking to.
int vrpn_GPIBDeviceServer::rcv_Shutdown()
{
    //Make sure the device output is not on!
    // These are supposed to be standard GPIB commands...
    printf("Reset GPIB device.\n");
    for ( int i = 0; i < d_num_devices; i++ ) { // shutdown all devices
	ibwrt(d_device_id[i], "*CLS;*RST", strlen("*CLS;*RST"));
	if (ibsta & ERR) {
	    GpibError( d_primary_address[i], d_secondary_address[i], "ibwrt Error"); 
	}
	ibonl(d_device_id[i], 0);              /* Take the device offline                 */
    
	if (ibsta & ERR) {
	    GpibError( d_primary_address[i], d_secondary_address[i], "ibonl Error"); 
	}
    }
    ibonl(d_board_index, 0);          /* Take the interface offline              */
    if (ibsta & ERR) {
	GpibError( d_primary_address[0], d_secondary_address[0], "ibonl Error"); 
    }
    return 0;
    
}


/*
 *                      Function GPIBERROR
 * This function will notify you that a NI-488 function failed by
 * printing an error message.  The status variable IBSTA will also be
 * printed in hexadecimal along with the mnemonic meaning of the bit
 * position. The status variable IBERR will be printed in decimal
 * along with the mnemonic meaning of the decimal value.  The status
 * variable IBCNTL will be printed in decimal.
 *
 * The NI-488 function IBONL is called to disable the hardware and
 * software.
 *
 * The EXIT function will terminate this program.
 */
int vrpn_GPIBDeviceServer::GpibError(vrpn_int32 pad, vrpn_int32 sad, char *msg) {

    vrpn_int32 len;
    printf ("%s\n", msg);
    // Send error to the client.
    // XXX Should pack the complete error info and send it.
    char * msgbuf = encode_Error(&len, pad, sad, msg);
    if (msgbuf == NULL) {
	fprintf(stderr, "vrpn_GPIBDeviceServer::GpibError: out of memory.\n");
	return -1;
    }
    if(Send(len, d_Error_type, msgbuf)) {
	fprintf(stderr, "vrpn_GPIBDeviceServer::GpibError: couldn't send message.\n");
	return -1;
    }

    printf ("ibsta = &H%x  <", ibsta);
    if (ibsta & ERR )  printf (" ERR");
    if (ibsta & TIMO)  printf (" TIMO");
    if (ibsta & END )  printf (" END");
    if (ibsta & SRQI)  printf (" SRQI");
    if (ibsta & RQS )  printf (" RQS");
    if (ibsta & CMPL)  printf (" CMPL");
    if (ibsta & LOK )  printf (" LOK");
    if (ibsta & REM )  printf (" REM");
    if (ibsta & CIC )  printf (" CIC");
    if (ibsta & ATN )  printf (" ATN");
    if (ibsta & TACS)  printf (" TACS");
    if (ibsta & LACS)  printf (" LACS");
    if (ibsta & DTAS)  printf (" DTAS");
    if (ibsta & DCAS)  printf (" DCAS");
    printf (" >\n");

    printf ("iberr = %d", iberr);
    if (iberr == EDVR) printf (" EDVR <DOS Error>\n");
    if (iberr == ECIC) printf (" ECIC <Not Controller-In-Charge>\n");
    if (iberr == ENOL) printf (" ENOL <No Listener>\n");
    if (iberr == EADR) printf (" EADR <Address error>\n");
    if (iberr == EARG) printf (" EARG <Invalid argument>\n");
    if (iberr == ESAC) printf (" ESAC <Not System Controller>\n");
    if (iberr == EABO) printf (" EABO <Operation aborted>\n");
    if (iberr == ENEB) printf (" ENEB <No GPIB board>\n");
    if (iberr == EOIP) printf (" EOIP <Async I/O in progress>\n");
    if (iberr == ECAP) printf (" ECAP <No capability>\n");
    if (iberr == EFSO) printf (" EFSO <File system error>\n");
    if (iberr == EBUS) printf (" EBUS <Command error>\n");
    if (iberr == ESTB) printf (" ESTB <Status byte lost>\n");
    if (iberr == ESRQ) printf (" ESRQ <SRQ stuck on>\n");
    if (iberr == ETAB) printf (" ETAB <Table Overflow>\n");

    printf ("ibcntl = %ld\n", ibcntl);
    printf ("\n");

    /* Call ibonl to take the device and interface offline */
    int index = get_device_id_index(pad, sad);
    if ( index == -1 )
	return -1;
    ibonl (d_device_id[index],0);
    ibonl (d_board_index,0);

    //exit(1);
	return 0;
}

//*********
// Message handlers

//static 
int vrpn_GPIBDeviceServer::handle_Device( void *_userdata, vrpn_HANDLERPARAM _p )
{
	vrpn_GPIBDeviceServer * me = (vrpn_GPIBDeviceServer *)_userdata;
	vrpn_int32 board_index,pad,sad;
	
	me->decode_Device(&_p.buffer, &board_index,&pad,&sad);
	return(me->rcv_Device(board_index,pad,sad));
}

//static 
int vrpn_GPIBDeviceServer::handle_Clear( void *_userdata, vrpn_HANDLERPARAM _p )
{
	vrpn_GPIBDeviceServer * me = (vrpn_GPIBDeviceServer *)_userdata;

	vrpn_int32 pad, sad;
	me->decode_DeviceID(&_p.buffer, &pad,&sad);
	return(me->rcv_Clear(pad,sad));
	
}

//static 
int vrpn_GPIBDeviceServer::handle_Write( void *_userdata, vrpn_HANDLERPARAM _p )
{
	vrpn_GPIBDeviceServer * me = (vrpn_GPIBDeviceServer *)_userdata;
	char * my_buf;
	
	vrpn_int32 pad, sad;
	me->decode_Write(&_p.buffer, &pad, &sad, &my_buf);
	return(me->rcv_Write(pad, sad, my_buf));
}

//static 
int vrpn_GPIBDeviceServer::handle_Read( void *_userdata, vrpn_HANDLERPARAM _p )
{
	vrpn_GPIBDeviceServer * me = (vrpn_GPIBDeviceServer *)_userdata;
	vrpn_int32 max_len;
	
	vrpn_int32 pad, sad;
	me->decode_Read(&_p.buffer, &pad, &sad, &max_len);
	return(me->rcv_Read(pad, sad, max_len));
}

//static 
int vrpn_GPIBDeviceServer::handle_ReadData( void *_userdata, vrpn_HANDLERPARAM _p )
{
	vrpn_GPIBDeviceServer * me = (vrpn_GPIBDeviceServer *)_userdata;
	vrpn_int32 max_len;
	
	vrpn_int32 pad, sad;
	me->decode_ReadData(&_p.buffer, &pad, &sad, &max_len);
	return(me->rcv_ReadData(pad, sad, max_len));
}

//static 
int vrpn_GPIBDeviceServer::handle_Shutdown( void *_userdata, vrpn_HANDLERPARAM _p )
{
	vrpn_GPIBDeviceServer * me = (vrpn_GPIBDeviceServer *)_userdata;

	return(me->rcv_Shutdown());
}

int vrpn_GPIBDeviceServer::Send( long len, long msg_type, char * buf )
{
  struct timeval now;
  int retval;

  gettimeofday(&now, NULL);
  // DON'T check for a null buffer - we want to send the message even if
  // it has no contents - it might be important, after all. 
	if (!d_connection->doing_okay())
		fprintf(stderr, "Send(): connection is not ok!!!\n");
    retval = d_connection->pack_message(len, now, msg_type, d_myId,
					buf, vrpn_CONNECTION_RELIABLE);
  if ( buf ) 
    delete [] buf;

  return retval;
}

int vrpn_GPIBDeviceServer::get_device_id_index( vrpn_int32 pad, vrpn_int32 sad ) {
    for ( int i = 0; i < d_num_devices; i++ )
	if ( (d_primary_address[i] == pad) &&
	     (d_secondary_address[i] == sad)  )
	    return i;
    return -1;
}

void vrpn_GPIBDeviceServer::init_ids(  ) {
    for ( int i = 0; i < MAX_DEVICES; i++ ) {
	d_device_id[i] = 0;
	d_primary_address[i] = 0;
	d_secondary_address[i] = 0;
    }
    d_num_devices = 0;
}

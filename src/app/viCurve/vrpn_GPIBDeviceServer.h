#ifndef _VRPN_GPIBDEVICESERVER_H
#define _VRPN_GPIBDEVICESERVER_H

#include "vrpn_GPIBDevice.h"
#include "nmb_Device.h"

class vrpn_GPIBDeviceServer : public nmb_Device_Server, public vrpn_GPIBDevice 
{
public:
    vrpn_GPIBDeviceServer(const char *name, vrpn_Connection *c = NULL);
    virtual int mainloop(const struct timeval * timeout = NULL);	// Handle getting any reports
    virtual ~vrpn_GPIBDeviceServer() {};

	int rcv_Device(int board_index, int primary_address, int secondary_address);
	int rcv_Clear();
	int rcv_Write(char * write_buf);
	int rcv_Read(int bytes);
	int rcv_ReadData(int bytes);
	// When the connection shuts down, close the instrument we are talking to.
	int rcv_Shutdown();

protected:
	int d_board_index;
	int d_primary_address;
	int d_secondary_address;
	int d_device_id;

	int GpibError(char *msg);
	int Send( long len, long msg_type, char * buf );
private:

	// Receive callbacks
	static int handle_Device( void *_userdata, vrpn_HANDLERPARAM _p );
	static int handle_Clear( void *_userdata, vrpn_HANDLERPARAM _p );
	static int handle_Write( void *_userdata, vrpn_HANDLERPARAM _p );
	static int handle_Read( void *_userdata, vrpn_HANDLERPARAM _p );
	static int handle_ReadData( void *_userdata, vrpn_HANDLERPARAM _p );
	static int handle_Shutdown( void *_userdata, vrpn_HANDLERPARAM _p );
	
};
#endif

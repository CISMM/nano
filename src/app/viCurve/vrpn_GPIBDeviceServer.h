#ifndef _VRPN_GPIBDEVICESERVER_H
#define _VRPN_GPIBDEVICESERVER_H

#include "vrpn_GPIBDevice.h"
#include "nmb_Device.h"

#define MAX_DEVICES 5

class vrpn_GPIBDeviceServer : public nmb_Device_Server, public vrpn_GPIBDevice 
{
public:
    vrpn_GPIBDeviceServer(const char *name, vrpn_Connection *c = NULL);
    virtual int mainloop(const struct timeval * timeout = NULL);	// Handle getting any reports
    virtual ~vrpn_GPIBDeviceServer() {};

	int rcv_Device(int board_index, int primary_address, int secondary_address);
	int rcv_Clear(vrpn_int32 pad, vrpn_int32 sad);
	int rcv_Write(vrpn_int32 pad, vrpn_int32 sad, char * write_buf);
	int rcv_Read(vrpn_int32 pad, vrpn_int32 sad, int bytes);
	int rcv_ReadData(vrpn_int32 pad, vrpn_int32 sad, int bytes);
	// When the connection shuts down, close the instrument we are talking to.
	int rcv_Shutdown();

protected:
	int d_board_index;
	int d_primary_address[MAX_DEVICES];
	int d_secondary_address[MAX_DEVICES];
	int d_device_id[MAX_DEVICES];
        int d_num_devices;

	int GpibError(vrpn_int32 pad, vrpn_int32 sad, char *msg);
	int Send( long len, long msg_type, char * buf );
private:

        int get_device_id_index( vrpn_int32 pad, vrpn_int32 sad );
        void init_ids(  );
	// Receive callbacks
	static int handle_Device( void *_userdata, vrpn_HANDLERPARAM _p );
	static int handle_Clear( void *_userdata, vrpn_HANDLERPARAM _p );
	static int handle_Write( void *_userdata, vrpn_HANDLERPARAM _p );
	static int handle_Read( void *_userdata, vrpn_HANDLERPARAM _p );
	static int handle_ReadData( void *_userdata, vrpn_HANDLERPARAM _p );
	static int handle_Shutdown( void *_userdata, vrpn_HANDLERPARAM _p );
	
};
#endif

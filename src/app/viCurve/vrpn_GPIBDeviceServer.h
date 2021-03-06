/* The nanoManipulator and its source code have been released under the
 * Boost software license when nanoManipulator, Inc. ceased operations on
 * January 1, 2014.  At this point, the message below from 3rdTech (who
 * sublicensed from nanoManipulator, Inc.) was superceded.
 * Since that time, the code can be used according to the following
 * license.  Support for this system is now through the NIH/NIBIB
 * National Research Resource at cismm.org.

Boost Software License - Version 1.0 - August 17th, 2003

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

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

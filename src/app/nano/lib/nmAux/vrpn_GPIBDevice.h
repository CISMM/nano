#ifndef _VRPN_GPIBDEVICE_H
#define _VRPN_GPIBDEVICE_H

#include "vrpn_Connection.h"

//#ifdef _WIN32
//#include "windows.h"
//#endif


class vrpn_GPIBDevice {
  public:
    vrpn_GPIBDevice(const char *name, vrpn_Connection *c = NULL);
	// Handle getting any reports
//    virtual int mainloop(const struct timeval * timeout = NULL) = 0;
    virtual ~vrpn_GPIBDevice() {};

  protected:
//    vrpn_Connection *d_connection;	
//    vrpn_int32 d_myId;		//connection ID for device

	// Sent by client
    vrpn_int32 d_Device_type;   // sends an ibdev GPIB command	
	vrpn_int32 d_Clear_type;   // sends an ibclr GPIB command	
	vrpn_int32 d_Write_type;   // sends an ibwrt GPIB command	
	vrpn_int32 d_Read_type;   // sends an ibrd GPIB command, server replies with Result
	vrpn_int32 d_ReadData_type;   // sends an ibrd GPIB command, server replies with ResultData
	vrpn_int32 d_Shutdown_type; // when connection shuts down

	// Sent by server
	vrpn_int32 d_Result_type;   // sends results of a ibrd GPIB command back to client
	vrpn_int32 d_ResultData_type;   // sends results of an ibrd GPIB command, 
			// translates the raw, 4 byte results into network standard byte order.
	vrpn_int32 d_Error_type;   // sends text of a GPIB error	


	char * encode_Device (vrpn_int32 * len, vrpn_int32 board_index,
			vrpn_int32 primary_address, vrpn_int32 secondary_address);
	vrpn_int32 decode_Device (const char ** buf, vrpn_int32* board_index,
			vrpn_int32* primary_address, vrpn_int32* secondary_address);

	// Write commands must send a valid, null-terminated string!
	char * encode_Write (vrpn_int32 * len, char * write_buf);
	vrpn_int32 decode_Write (const char ** buf, char ** write_buf);
		// allocated memory for the write_buf - user must delete.

	// Specifies the number of 1 byte characters to read 
	char * encode_Read (vrpn_int32 * len, vrpn_int32 max_len);
	vrpn_int32 decode_Read (const char ** buf, vrpn_int32 *max_len);

	// Specifies the number of 4 byte floats to read.
	char * encode_ReadData (vrpn_int32 * len, vrpn_int32 max_len);
	vrpn_int32 decode_ReadData (const char ** buf, vrpn_int32 *max_len);

	// Result commands must send a valid, null-terminated string!
	char * encode_Result (vrpn_int32 * len, char * write_buf);
	vrpn_int32 decode_Result (const char ** buf, char ** write_buf);
		// allocated memory for the write_buf - user must delete.

	char * encode_ResultData (vrpn_int32 * len, vrpn_float32 * data, vrpn_int32 data_len);
	vrpn_int32 decode_ResultData (const char ** buf, vrpn_float32 ** data, vrpn_int32 * data_len);
		// allocated memory for the data - user must delete.

	// Error commands must send a valid, null-terminated string!
	char * encode_Error (vrpn_int32 * len, char * write_buf);
	vrpn_int32 decode_Error (const char ** buf, char ** write_buf);
		// allocated memory for the write_buf - user must delete.

};


#endif

#include <stdio.h>
#include <string.h>  // for strlen on HPs

#include "vrpn_GPIBDevice.h"
#include "nmb_Util.h"

#define CHECK(a) if ((a) == -1) return -1

vrpn_GPIBDevice::vrpn_GPIBDevice
    (const char * name,
     vrpn_Connection * connection) :
d_connection (connection) 
{
  char * servicename;           //Helps get the right name for the sender.
  servicename = vrpn_copy_service_name(name); 

  if (connection) {
    d_myId = connection->register_sender(servicename);

    d_Device_type = connection->register_message_type
         ("gpib_Device");

    d_Clear_type = connection->register_message_type
         ("gpib_Clear");

    d_Write_type = connection->register_message_type
         ("gpib_Write");

    d_Read_type = connection->register_message_type
         ("gpib_Read");

    d_ReadData_type = connection->register_message_type
         ("gpib_ReadData");

    d_Result_type = connection->register_message_type
         ("gpib_Result");

    d_ResultData_type = connection->register_message_type
         ("gpib_ResultData");

    d_Error_type = connection->register_message_type
         ("gpib_Error");

	d_Shutdown_type = d_connection->register_message_type
		(vrpn_dropped_last_connection);

  }

  if (servicename)
    delete [] servicename;
}

char * vrpn_GPIBDevice::encode_Device (vrpn_int32 * len, vrpn_int32 board_index,
		vrpn_int32 primary_address, vrpn_int32 secondary_address)
{
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 3 * sizeof(vrpn_int32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "vrpn_GPIBDevice::encode_Device:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, board_index);
    nmb_Util::Buffer(&mptr, &mlen, primary_address);
    nmb_Util::Buffer(&mptr, &mlen, secondary_address);
  }

  return msgbuf;
}

vrpn_int32 vrpn_GPIBDevice::decode_Device (const char ** buf, vrpn_int32* board_index,
		vrpn_int32* primary_address, vrpn_int32* secondary_address)
{
  CHECK(nmb_Util::Unbuffer(buf, board_index));
  CHECK(nmb_Util::Unbuffer(buf, primary_address));
  CHECK(nmb_Util::Unbuffer(buf, secondary_address));

  return 0;
}

char * vrpn_GPIBDevice::encode_Write (vrpn_int32 * len, char * my_buf)
{
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen, slen;

  if (!len) return NULL;

  // include string terminator
  slen = strlen(my_buf) + 1;
  *len = sizeof(vrpn_int32) + slen;
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "vrpn_GPIBDevice::encode_Write:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, slen);
    nmb_Util::Buffer(&mptr, &mlen, my_buf, slen);
  }

  return msgbuf;

}
vrpn_int32 vrpn_GPIBDevice::decode_Write (const char ** buf, char ** my_buf)
{
	vrpn_int32 length;
	CHECK(nmb_Util::Unbuffer(buf, &length));
	*my_buf = new char [length];
	if (!*my_buf) {
		fprintf(stderr, "vrpn_GPIBDevice::decode_Write:  "
                    "Out of memory!\n");
		return -1;
	}

	CHECK(nmb_Util::Unbuffer(buf, *my_buf, length));

	return 0;

}

char * vrpn_GPIBDevice::encode_Read (vrpn_int32 * len, vrpn_int32 max_len)
{
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 1 * sizeof(vrpn_int32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "vrpn_GPIBDevice::encode_Device:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, max_len);
  }

  return msgbuf;
}

vrpn_int32 vrpn_GPIBDevice::decode_Read (const char ** buf, vrpn_int32 *max_len)
{
  CHECK(nmb_Util::Unbuffer(buf, max_len));

  return 0;

}

char * vrpn_GPIBDevice::encode_ReadData (vrpn_int32 * len, vrpn_int32 max_len)
{
	return(encode_Read(len, max_len));
}

vrpn_int32 vrpn_GPIBDevice::decode_ReadData (const char ** buf, vrpn_int32 *max_len)
{
	return(decode_Read(buf,max_len));
}

char * vrpn_GPIBDevice::encode_Result (vrpn_int32 * len, char * my_buf)
{
	return (encode_Write(len, my_buf));
}
vrpn_int32 vrpn_GPIBDevice::decode_Result (const char ** buf, char ** my_buf)
{
	return (decode_Write(buf,my_buf));
}

char * vrpn_GPIBDevice::encode_ResultData (vrpn_int32 * len, vrpn_float32 * data, vrpn_int32 data_len)
{
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  // include string terminator
  *len = sizeof(vrpn_int32) + sizeof(vrpn_float32)*data_len;
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "vrpn_GPIBDevice::encode_ResultData:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    nmb_Util::Buffer(&mptr, &mlen, data_len);
	for (int i = 0; i< data_len; i++) {
	    nmb_Util::Buffer(&mptr, &mlen, data[i]);
	}
  }

  return msgbuf;

}
vrpn_int32 vrpn_GPIBDevice::decode_ResultData (const char ** buf, vrpn_float32 ** data, vrpn_int32 * data_len)
{
	CHECK(nmb_Util::Unbuffer(buf, data_len));
	*data = new vrpn_float32 [*data_len];
	vrpn_float32 * temp_data = *data;
	if (!*data) {
		fprintf(stderr, "vrpn_GPIBDevice::decode_ResultData:  "
                    "Out of memory!\n");
		return -1;
	}
	for (int i = 0; i< *data_len; i++) {
		CHECK(nmb_Util::Unbuffer(buf, &(temp_data[i])));
	}
	return 0;

}

char * vrpn_GPIBDevice::encode_Error (vrpn_int32 * len, char * my_buf)
{
	return (encode_Write(len, my_buf));
}
vrpn_int32 vrpn_GPIBDevice::decode_Error (const char ** buf, char ** my_buf)
{
	return (decode_Write(buf,my_buf));
}


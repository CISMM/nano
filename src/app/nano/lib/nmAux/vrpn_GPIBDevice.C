#include <stdio.h>
#include <string.h>  // for strlen on HPs

#include "vrpn_GPIBDevice.h"

#define CHECK(a) if ((a) == -1) return -1

vrpn_GPIBDevice::vrpn_GPIBDevice (
    const char * /*name*/,
    vrpn_Connection * connection) 
//: d_connection (connection)
{
//  char * servicename;           //Helps get the right name for the sender.
//  servicename = vrpn_copy_service_name(name); 

  if (connection) {
//    d_myId = connection->register_sender(servicename);

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

    d_Shutdown_type = connection->register_message_type
         (vrpn_dropped_last_connection);

  }

//  if (servicename) {
//    delete [] servicename;
//    servicename = NULL;
//  }

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
    vrpn_buffer(&mptr, &mlen, board_index);
    vrpn_buffer(&mptr, &mlen, primary_address);
    vrpn_buffer(&mptr, &mlen, secondary_address);
  }

  return msgbuf;
}

vrpn_int32 vrpn_GPIBDevice::decode_Device (const char ** buf, vrpn_int32* board_index,
		vrpn_int32* primary_address, vrpn_int32* secondary_address)
{
  CHECK(vrpn_unbuffer(buf, board_index));
  CHECK(vrpn_unbuffer(buf, primary_address));
  CHECK(vrpn_unbuffer(buf, secondary_address));

  return 0;
}

char * vrpn_GPIBDevice::encode_DeviceID (vrpn_int32 * len, vrpn_int32 primary_address, vrpn_int32 secondary_address)
{
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  *len = 2 * sizeof(vrpn_int32);
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "vrpn_GPIBDevice::encode_Device:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, primary_address);
    vrpn_buffer(&mptr, &mlen, secondary_address);
  }

  return msgbuf;
}

vrpn_int32 vrpn_GPIBDevice::decode_DeviceID (const char ** buf,
		vrpn_int32* primary_address, vrpn_int32* secondary_address)
{
  CHECK(vrpn_unbuffer(buf, primary_address));
  CHECK(vrpn_unbuffer(buf, secondary_address));

  return 0;
}

char * vrpn_GPIBDevice::encode_Write (vrpn_int32 * len, vrpn_int32 pad, vrpn_int32 sad, char * my_buf)
{
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen, slen;

  if (!len) return NULL;

  // include string terminator
  slen = strlen(my_buf) + 1;
  *len = 3 * sizeof(vrpn_int32) + slen;
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "vrpn_GPIBDevice::encode_Write:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, pad);
    vrpn_buffer(&mptr, &mlen, sad);
    vrpn_buffer(&mptr, &mlen, slen);
    vrpn_buffer(&mptr, &mlen, my_buf, slen);
  }

  return msgbuf;

}
vrpn_int32 vrpn_GPIBDevice::decode_Write (const char ** buf, vrpn_int32 *pad, vrpn_int32 *sad, char ** my_buf)
{
	vrpn_int32 length;
	CHECK(vrpn_unbuffer(buf, pad));
	CHECK(vrpn_unbuffer(buf, sad));
	CHECK(vrpn_unbuffer(buf, &length));
	*my_buf = new char [length];
	if (!*my_buf) {
		fprintf(stderr, "vrpn_GPIBDevice::decode_Write:  "
                    "Out of memory!\n");
		return -1;
	}

	CHECK(vrpn_unbuffer(buf, *my_buf, length));

	return 0;

}

char * vrpn_GPIBDevice::encode_Read (vrpn_int32 * len, vrpn_int32 pad, vrpn_int32 sad, vrpn_int32 max_len)
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
    vrpn_buffer(&mptr, &mlen, pad);
    vrpn_buffer(&mptr, &mlen, sad);
    vrpn_buffer(&mptr, &mlen, max_len);
  }

  return msgbuf;
}

vrpn_int32 vrpn_GPIBDevice::decode_Read (const char ** buf, vrpn_int32 *pad, vrpn_int32 *sad, vrpn_int32 *max_len)
{
  CHECK(vrpn_unbuffer(buf, pad));
  CHECK(vrpn_unbuffer(buf, sad));
  CHECK(vrpn_unbuffer(buf, max_len));

  return 0;

}

char * vrpn_GPIBDevice::encode_ReadData (vrpn_int32 * len, vrpn_int32 pad, vrpn_int32 sad, vrpn_int32 max_len)
{
	return(encode_Read(len, pad, sad, max_len));
}

vrpn_int32 vrpn_GPIBDevice::decode_ReadData (const char ** buf, vrpn_int32 *pad, vrpn_int32 *sad, vrpn_int32 *max_len)
{
	return(decode_Read(buf,pad, sad, max_len));
}

char * vrpn_GPIBDevice::encode_Result (vrpn_int32 * len, vrpn_int32 pad, vrpn_int32 sad, char * my_buf)
{
	return (encode_Write(len,pad,sad,my_buf));
}
vrpn_int32 vrpn_GPIBDevice::decode_Result (const char ** buf, vrpn_int32 *pad, vrpn_int32 *sad, char ** my_buf)
{
	return (decode_Write(buf,pad,sad,my_buf));
}

char * vrpn_GPIBDevice::encode_ResultData (vrpn_int32 * len, vrpn_int32 pad, vrpn_int32 sad, vrpn_float32 * data, vrpn_int32 data_len)
{
  char * msgbuf = NULL;
  char * mptr;
  vrpn_int32 mlen;

  if (!len) return NULL;

  // include string terminator
  *len = 3 * sizeof(vrpn_int32) + sizeof(vrpn_float32)*data_len;
  msgbuf = new char [*len];
  if (!msgbuf) {
    fprintf(stderr, "vrpn_GPIBDevice::encode_ResultData:  "
                    "Out of memory.\n");
    *len = 0;
  } else {
    mptr = msgbuf;
    mlen = *len;
    vrpn_buffer(&mptr, &mlen, pad);
    vrpn_buffer(&mptr, &mlen, sad);
    vrpn_buffer(&mptr, &mlen, data_len);
	for (int i = 0; i< data_len; i++) {
	    vrpn_buffer(&mptr, &mlen, data[i]);
	}
  }

  return msgbuf;

}
vrpn_int32 vrpn_GPIBDevice::decode_ResultData (const char ** buf, vrpn_int32 *pad, vrpn_int32 *sad, vrpn_float32 ** data, vrpn_int32 * data_len)
{
	CHECK(vrpn_unbuffer(buf, pad));
	CHECK(vrpn_unbuffer(buf, sad));
	CHECK(vrpn_unbuffer(buf, data_len));
	*data = new vrpn_float32 [*data_len];
	vrpn_float32 * temp_data = *data;
	if (!*data) {
		fprintf(stderr, "vrpn_GPIBDevice::decode_ResultData:  "
                    "Out of memory!\n");
		return -1;
	}
	for (int i = 0; i< *data_len; i++) {
		CHECK(vrpn_unbuffer(buf, &(temp_data[i])));
	}
	return 0;

}

char * vrpn_GPIBDevice::encode_Error (vrpn_int32 * len, vrpn_int32 pad, vrpn_int32 sad, char * my_buf)
{
	return (encode_Write(len, pad, sad, my_buf));
}
vrpn_int32 vrpn_GPIBDevice::decode_Error (const char ** buf, vrpn_int32 *pad, vrpn_int32 *sad, char ** my_buf)
{
	return (decode_Write(buf,pad, sad,my_buf));
}


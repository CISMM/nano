#include <stdio.h>
#include <stdlib.h>
#include "vrpn_Connection.h"
#include "vrpn_GPIBDeviceServer.h"

unsigned short def_port_no = 4545;

//--------------------------------------------------------------------------
// Handles VRPN messages

int handle_any_print (void * userdata, vrpn_HANDLERPARAM p) {
  vrpn_Connection * c = (vrpn_Connection *) userdata;

  fprintf(stderr, "Got message \"%s\" from \"%s\".\n",
          c->message_type_name(p.type), c->sender_name(p.sender));

  return 0;  // non-error completion
}



void main (unsigned argc, char *argv[])
{

#ifdef _WIN32
	WSADATA wsaData; 
	int status;
    if ((status = WSAStartup(MAKEWORD(1,1), &wsaData)) != 0) {
         fprintf(stderr, "WSAStartup failed with %d\n", status);
    } else {
		 //fprintf(stderr, "WSAStartup success\n");
	}
#endif

	vrpn_Synchronized_Connection *	connection;
	vrpn_GPIBDeviceServer * gpib_server;

	connection = new vrpn_Synchronized_Connection(def_port_no);

	// DEBUG print all message, incoming and outgoing, 
	// sent over our VRPN connection
	//connection->register_handler(vrpn_ANY_TYPE, handle_any_print,
	//			    connection);
         // defaults to vrpn_SENDER_ANY

	
	gpib_server = new vrpn_GPIBDeviceServer("GPIBServer@argon", connection);

	if (gpib_server == NULL) {
		fprintf(stderr, "Couldn't create gpib_server\n");
		exit(1);
	}
  struct timeval delay;

  delay.tv_sec = 0L;
  delay.tv_usec = 10L;

	// Loop forever calling the mainloop()s for all devices
	while (1) {

		// Put in a sleep to free up CPU cycles.
		vrpn_SleepMsecs(1);

		// Send/receive messages from our vrpn connection.
		// Delay a very short time to avoid using 100% cpu time.
		gpib_server->mainloop(&delay);
	}
}

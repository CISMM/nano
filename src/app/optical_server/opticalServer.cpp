
#include <vrpn_Connection.h>
#include "nmm_Microscope_SEM_diaginc.h"


int main( int argc, char* argv[] )
{

    int     port = vrpn_DEFAULT_LISTEN_PORT_NO;

    if (argc > 1) {
		port = atoi(argv[1]);
	}

    vrpn_Synchronized_Connection connection(port);

	nmm_Microscope_SEM_diaginc m( "SEM", &connection, vrpn_FALSE );

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 10000;
 
    while (1){
        m.mainloop();
        connection.mainloop(&timeout);
    }
    return 0;
}
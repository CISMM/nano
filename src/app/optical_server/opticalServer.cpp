
#include "Tcl_Linkvar.h"
#include <vrpn_Connection.h>
#include "nmm_Microscope_SEM_diaginc.h"
#include "OpticalServerInterface.h"


int main( int argc, char* argv[] )
{

    int port = 5757;

    OpticalServerInterface* iface = OpticalServerInterface::getInterface( );

    if (argc > 1) {
	port = atoi(argv[1]);
    }

    vrpn_Synchronized_Connection connection(port);

    // nmm_Microscope_SEM_diaginc m( "SEM", &connection, vrpn_FALSE );
    nmm_Microscope_SEM_diaginc m( "SEM", &connection, vrpn_TRUE );

    iface->setMicroscope( &m );
	iface->setBinning( m.getBinning( ) );
	iface->setResolutionIndex( m.getResolutionIndex( ) );

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 10000;
 
    while (1)
    {
        m.mainloop();
        connection.mainloop(&timeout);
	vrpn_SleepMsecs(10);
    }
    return 0;
}

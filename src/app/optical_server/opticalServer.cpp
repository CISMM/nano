
#include "Tcl_Linkvar.h"
#include <vrpn_Connection.h>
#include "nmm_Microscope_SEM_optical.h"
#include "nmm_Microscope_SEM_diaginc.h"
#include "nmm_Microscope_SEM_cooke.h"
#include "OpticalServerInterface.h"


void Usage(char* s)
{
  fprintf(stderr, "Usage: %s \n",s);
  fprintf(stderr, "       <-cooke|-spot>\n");
  fprintf(stderr, "       [-virtualacq]\n");
  exit(-1);
}


int main( int argc, char* argv[] )
{

    int port = 5757;

    OpticalServerInterface* iface = OpticalServerInterface::getInterface( );

	nmm_Microscope_SEM_optical* m = NULL;
	vrpn_bool do_virtualAcq = vrpn_FALSE;

	bool do_cooke = false, do_spot = false;

	int i = 1;
    while (i < argc) 
	{
      if( strcmp( argv[i], "-virtualacq" ) == 0 ) 
	  {
        do_virtualAcq = vrpn_TRUE;
      } 
	  else if( strcmp( argv[i], "-cooke" ) == 0 ) 
	  {
		  do_cooke = vrpn_TRUE;
      }
	  else if( strcmp( argv[i], "-spot" ) == 0 )
	  {
		  do_spot = vrpn_TRUE;
	  }
	  else if( argv[i][0] == '-' ) // Unknown argument starting with "-"
	  {
          Usage(argv[0]);
      } 
	  else // An argument without a "-", so we don't know what to do. 
	  {
          Usage(argv[0]);
      }
      i++;
    }
	if( do_cooke == do_spot )  Usage( argv[0] );

    vrpn_Connection connection(port);
	
	if( do_cooke )
		m = new nmm_Microscope_SEM_cooke( "SEM", &connection, do_virtualAcq );
	if( do_spot )
		m = new nmm_Microscope_SEM_diaginc( "SEM", &connection, do_virtualAcq );

    iface->setMicroscope( m );
	iface->setBinning( m->getBinning( ) );
	iface->setResolutionIndex( m->getResolutionIndex( ) );
	iface->setExposure( m->getExposure( ) );

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 10000;
 
    while (1)
    {
        m->mainloop();
        connection.mainloop(&timeout);
		vrpn_SleepMsecs(10);
    }
    return 0;
}



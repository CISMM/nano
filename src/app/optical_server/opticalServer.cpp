
#include "Tcl_Linkvar.h"
#include <vrpn_Connection.h>
#include "nmm_Microscope_SEM_optical.h"
#include "nmm_Microscope_SEM_diaginc.h"
#include "nmm_Microscope_SEM_cooke.h"
#include "OpticalServerInterface.h"
#include "opticalServer.h"
#include "Logging.h"

const char* opticalServerName = "SEM";
const int opticalServerPort = 5757;

void Usage(char* s)
{
  fprintf(stderr, "Usage: %s \n",s);
  fprintf(stderr, "       <-cooke|-spot>\n");
  fprintf(stderr, "       [-virtualacq]\n");
  exit(-1);
}


bool keepRunning = true;
bool microscopeThreadDone = false;
bool glutThreadDone = false;
vrpn_Connection* connection = NULL;
nmm_Microscope_SEM_optical* microscope = NULL;


// this will end up being called from the glut thread
// when someone hits the exit button
void __cdecl exitFunc( )
{
	keepRunning = false;
	OpticalServerInterface* iface = OpticalServerInterface::getInterface();
	if( iface != NULL )
	{
		iface->setImage( NULL, 0, 0 );
		iface->setMicroscope( NULL );
	}
	vrpn_SleepMsecs( 2000 );
	while( !microscopeThreadDone )
	{
		vrpn_SleepMsecs( 1 );  // wait for the microscope loop to stop
	}
	if( microscope )
	{
		delete microscope;
		microscope = NULL;
	}
	if( connection )
	{
		delete connection;
		connection = NULL;
	}

	fprintf( stderr, "opticalServer exitFunc done.\n" );
	fflush( stderr );
}


int main( int argc, char* argv[] )
{
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

	connection = new vrpn_Connection( opticalServerPort );
	if( do_cooke )
		microscope = new nmm_Microscope_SEM_cooke( opticalServerName, connection, do_virtualAcq );
	if( do_spot )
		microscope = new nmm_Microscope_SEM_diaginc( opticalServerName, connection, do_virtualAcq );

	OpticalServerInterface* iface = OpticalServerInterface::getInterface( );
    iface->setMicroscope( microscope );
	iface->setBinning( microscope->getBinning( ) );
	iface->setResolutionIndex( microscope->getResolutionIndex( ) );
	iface->setExposure( microscope->getExposure( ) );

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 10000;
 
	atexit( exitFunc );

	Logging* log = NULL;
	while( keepRunning )
    {
        microscope->mainloop();
        connection->mainloop(&timeout);

		//log = Logging::getInstance( );
		//if( log != NULL ) log->mainloop( );
		vrpn_SleepMsecs(10);
    }
	microscopeThreadDone = true;
	
	log = Logging::getInstance( );
	if( log != NULL ) log->stopLogging( );

	while( !OpticalServerInterface::isGlutThreadDone() )
	{
		vrpn_SleepMsecs( 1 );
	}
    return 0;
}



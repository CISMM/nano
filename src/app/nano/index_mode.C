
#include <iostream.h>
#include <string.h>

#ifndef _WIN32
#include <sys/types.h> // } for mkdir( )
#include <sys/stat.h>  // }
#include <errno.h>     // for use with mkdir( )
#endif

#include "BCPlane.h"
#include "nmg_Globals.h"
#include "nmg_Graphics.h"
#include "ImageMaker.h"

#include "microscape.h"
#include "index_mode.h"


// Initialize static members of class Index_mode
BCPlane* Index_mode::plane = NULL;
bool Index_mode::initialized = false;
bool Index_mode::snapshots_taken = 0;
const char* Index_mode::callback_username = "index_mode";
char* Index_mode::outputDir = NULL;


void
Index_mode::init( BCPlane* plane, const char* streamfileName )
{
#ifndef _WIN32
  if( outputDir != NULL )
    delete outputDir;
  outputDir = new char[ strlen( streamfileName ) + 5 /* for "_NI/" + null */ ];
  strcpy( outputDir, streamfileName );
  strcat( outputDir, "_NI/" );
  int result = mkdir( outputDir, 0755 );
  if( result < 0 && errno != EEXIST /* dir. already exists */)
    {
      cout << "Failed to create index directory:  " << outputDir << endl;
      exit( -1 );
    }

  initialized = true;
  Index_mode::newPlane( plane );
#else // _WIN32
  cout << "No index mode on the PC" << endl;
  exit( -1 );
#endif
}

bool
Index_mode::isInitialized()
{
  return initialized;
}

void
Index_mode::newPlane( BCPlane* plane )
{
#ifndef _WIN32
  if( !initialized ) return;
  
  if( Index_mode::plane != NULL )
    Index_mode::plane->remove_callback( (Plane_Valuecall) handle_new_datapoint,
				  (void*) callback_username );

  cout << "Index_mode::newPlane." << endl;
  cout << "\tfrom:  ";
  if( Index_mode::plane == NULL )
    cout << "NULL";
  else
    cout << Index_mode::plane->name( );
  cout << endl;
  cout << "\tto:    ";
  if( plane == NULL )
    cout << "NULL";
  else
    cout << plane->name( );
  cout << endl;
  
  Index_mode::plane = plane;
  if( plane == NULL ) return;
  plane->add_callback( (Plane_Valuecall) handle_new_datapoint,
		     (void*) callback_username );
#else // _WIN32
  cout << "No index mode on the PC" << endl;
  exit( -1 );
#endif
}


void
Index_mode::shutdown( )
{
#ifndef _WIN32
  if( !initialized ) return;

  if( Index_mode::plane != NULL )
    Index_mode::plane->remove_callback( (Plane_Valuecall) handle_new_datapoint,
					(void*) callback_username );
  if( outputDir != NULL )
    delete outputDir;
  outputDir = 0;

  initialized = false;
#endif
}


void
Index_mode::snapshot( )
{
#ifndef _WIN32
  if( !initialized ) return;

  char* filename = new char[ strlen( outputDir ) + 9 /* xxxxx.tif */ + 1 ];
  sprintf( filename, "%s%05d%s", outputDir, snapshots_taken, ".tif" );
  /* this should be made to do the right thing to figure out if \ or / is needed. 
     then the #ifndef _WIN32 can be removed from all over the place */
  
  cout << "Index_mode:  " << filename << endl;

  graphics->enableChartjunk( false );
  center( ); // from microscape.h
  graphics->createScreenImage( filename, TIFFImageType );
  graphics->enableChartjunk( true );
  
  snapshots_taken++;
#endif
}


void
Index_mode::handle_new_datapoint( BCPlane *plane, int x, int y, void *userdata )
{
#ifndef _WIN32
  if( !initialized ) return;

  if( x == 0 && y == 0 )
    Index_mode::snapshot( );

#endif
}


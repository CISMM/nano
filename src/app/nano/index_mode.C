
#include <iostream.h>
#include <string.h>

#ifndef _WIN32
#include <sys/types.h> // } for mkdir( )
#include <sys/stat.h>  // }
#include <errno.h>     // for use with mkdir( )
#endif

#include "BCPlane.h"
#include "nmb_Dataset.h"
#include "nmb_Decoration.h"
#include "nmg_Globals.h"
#include "nmg_Graphics.h"
#include "ImageMaker.h"

#include "microscape.h"
#include "index_mode.h"


// Initialize static members of class Index_mode
BCPlane* Index_mode::plane = NULL;
bool Index_mode::initialized = false;
const char* Index_mode::callback_username = "index_mode";
char* Index_mode::outputDir = NULL;
int Index_mode::prev_time = -1;
bool Index_mode::first_scan = true;

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
    cout << *Index_mode::plane->name( );
  cout << endl;
  cout << "\tto:    ";
  if( plane == NULL )
    cout << "NULL";
  else
    cout << *plane->name( );
  cout << endl;
  
  Index_mode::plane = plane;
  if( plane == NULL ) return;
  plane->add_callback( (Plane_Valuecall) handle_new_datapoint,
		     (void*) callback_username );

  if( dataset == NULL || graphics == NULL ) return; 
  
  // Set colorplane to match new heightplane
  dataset->colorPlaneName->Set( *plane->name() );
  graphics->setColorPlaneName( *plane->name() );  

#else // _WIN32
  cout << "No index mode on the PC" << endl;
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
#else // _WIN32
  cout << "No index mode on the PC" << endl;
#endif
}


void
Index_mode::snapshot( )
{
#ifndef _WIN32
  if( !initialized || decoration == NULL || dataset == NULL || plane == NULL ){
     cerr << "Index_mode: Unable to create snapshot."<< endl; 
     return;
  }

  prev_time = (int) decoration->elapsedTime;

  /* Create filename using elapsed time in stream file, 00000 for static file */
  char* filename = new char[ strlen( outputDir ) + 9 /* xxxxx.tif */ + 1 ];
  sprintf( filename, "%s%05d%s", outputDir, prev_time, ".tif" );
  /* this should be made to do the right thing to figure out if \ or / is needed. 
     then the #ifndef _WIN32 can be removed from all over the place */
  
  cout << "Index_mode: Preparing snapshot " << filename << endl;

  /* Pop vlib window to front */
  #ifdef V_GLUT
     v_gl_set_context_to_vlib_window();
     glutPopWindow();
     glutProcessEvents_UNC();

  #else
     /* XWindows stuff would go here */
  #endif

  graphics->setColorMapName( dataset->colorMapName->string() );
  graphics->setColorPlaneName( *plane->name() );
  graphics->enableChartjunk( false );
  center( ); // from microscape.h
  graphics->mainloop();
  graphics->createScreenImage( filename, TIFFImageType );
  graphics->enableChartjunk( true );
  
  cout<< "Index_mode: Snapshot taken:" << filename << endl;
#else // _WIN32
  cout << "No index mode on the PC" << endl;
#endif
}


void
Index_mode::handle_new_datapoint( BCPlane *plane, int x, int y, void *userdata )
{

#ifndef _WIN32
  if( !initialized ) return;

  if( x == 0 && y == plane->numY() - 1 && decoration != NULL && (int) decoration->elapsedTime > prev_time)
    {

      if( first_scan ){ 
          first_scan = false;
          return;
      }

      Index_mode::snapshot( );
    }
#endif
}


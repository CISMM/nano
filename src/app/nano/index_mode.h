
#ifndef __INDEX_MODE_H
#define __INDEX_MODE_H

#include "BCPlane.h"

////////////////
// functions for a mode to index stream files.
//
// David Marshburn, spring 2001

class Index_mode
{
 public:
  // initialize indexing mode, e.g., by setting up callbacks.
  // index mode won't do anything until it's initialized.
  static void init( BCPlane* plane, const char* streamfileName );

  // determine whether or not index mode has been initialized
  static bool isInitialized();

  // to use in case the current height plane changes.
  static void newPlane( BCPlane* plane );

  // unregister callbacks and clean up.
  static void shutdown( );

  // save a current snapshot of the graphics window.
  static void snapshot( );

  // event handler for a new data point being generated,
  // defined in accordance with BCPlane's "Plane_Valuecall"
  static void handle_new_datapoint( BCPlane *plane, int x, int y, void *userdata );
  
 protected:
  
  static BCPlane* plane;
  static bool initialized;
  static const char* callback_username;
  static char* outputDir;
  static int prev_time;
  static bool first_scan;
};

#endif

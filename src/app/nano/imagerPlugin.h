/*===3rdtech===
  Copyright (c) 2000-2002 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/

//-------------------------------------------------------------------------
// This implements an imager plug-in class for the nanoManipulator program.
// It uses the vrpn_TempImager class to send the data plane mapped to
// height out through a well-known VRPN port to which image analysis programs
// can connect.  The image analysis programs can also send back an image,
// which will be added to the list of available data planes.  It can also
// send back positioning commands (vrpn_Tracker messages) and text to be displayed
// at those positions (vrpn_Text messages).  All of this is handled within the
// class, including callback handling and such.
// There must only be one instance of this class at a time (to avoid collisions
// with the well-known port number), and it must be destroyed when there is no
// grid and reconstructed when a new grid is created (sizes change, etc.)
// The heartbeat() function should be called once per display update so that
// and incoming data can be handled and any changes can be sent to the imager
// across the connection.

// Warning -- If the order of <vector> and <URText.h> is interchanged with the VRPN
// includes, compiler warnings result.  We're mixing SensAble and Windows standard
// templates, I bet...
#include <vector>
#include <UTree.h>
#include <URText.h>
#include <vrpn_Connection.h>
#include <vrpn_Tracker.h>
#include <vrpn_Text.h>
#include <vrpn_TempImager.h>
#include <nmb_Dataset.h>

class imagerPlugin {

public:
  imagerPlugin(nmb_Dataset *data_set);
  ~imagerPlugin();

  void heartbeat(void);

protected:

  //------------------------------------------------------------------------
  // The Nano dataset which we're watching the height plane on to send to
  // the imager over the connection, and the name of the height field we
  // had mapped last time through the heartbeat.
  nmb_Dataset *d_data_set;
  BCPlane     *d_source_plane;
  BCPlane     *d_input_plane;
  nmb_String  d_last_height_name;

  //------------------------------------------------------------------------
  // Keeps track of the region over which there have been changes in the
  // plane we are using to send data about.  If the max is less than the
  // min, there have been no changes reported.  These variables are used
  // to keep track of groups of changes so that they can be batched up and
  // sent to the imager over the connection.

  int d_left, d_right, d_top, d_bottom;

  //------------------------------------------------------------------------
  // Tells whether the incoming image data set matches the grid we have.
  // If it does not, we don't do any receiving.
  bool d_incoming_data_matches;

  //------------------------------------------------------------------------
  // A TempImager client to read the image from the image analysis program.
  // A TempImager Server to send an image.  A connection object to send the
  // data on.  Also, a text receiver and tracker remote to receive
  // position and send text messages.

  vrpn_Connection		*d_connection;
  vrpn_TempImager_Remote	*d_imager_client;
  vrpn_Text_Receiver		*d_text_client;
  vrpn_Tracker_Remote		*d_tracker_client;
  vrpn_Text_Sender		*d_server_stand_in;
  vrpn_TempImager_Server	*d_imager_server;
  int				d_channel_id;

  //------------------------------------------------------------------------
  // Buffer to hold a line of data that is being sent by the imager server.
  vrpn_float32			*d_linebuf;

  //------------------------------------------------------------------------
  // Location and orientation at which to draw the next incoming text
  // message (filled in by the tracker callback handler).

  vrpn_float64	d_pos[3], d_quat[4];

  //------------------------------------------------------------------------
  // List of string objects that we've displayed.
  vector<URText *>		d_textRenderers;

  //------------------------------------------------------------------------
  // Callback handlers to deal with messages coming across the VRPN connection,
  // and with new connections.

  static int handle_new_connection(void *userdata, vrpn_HANDLERPARAM);
  static void handle_description_message(void *userdata, const struct timeval);
  static void handle_region_message(void *userdata, const vrpn_IMAGERREGIONCB info);
  static void handle_tracker_message(void *userdata, const vrpn_TRACKERCB info);
  static void handle_text_message(void *userdata, const vrpn_TEXTCB info);

  //------------------------------------------------------------------------
  // Callback handler to deal with plane data being updated.

  static void handle_height_plane_modified( BCPlane* plane, int x, int y,
			   void* userdata);

  //------------------------------------------------------------------------
  // Protected functions called by class methods.

  bool connect_new_source_plane( const char *plane_name );
  bool allocate_line_buffer( void );
  void send_whole_image(void);
  void send_region_update(void);
  void teardown(void);
};
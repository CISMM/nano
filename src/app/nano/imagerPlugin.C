/*===3rdtech===
  Copyright (c) 2000-2002 by 3rdTech, Inc.
  All Rights Reserved.

  This file may not be distributed without the permission of 
  3rdTech, Inc. 
  ===3rdtech===*/

#include "imagerPlugin.h"

/* XXX When the height plane is changed to the incoming image plane, we get all sorts of fast
updates from the other side (infinite loop caused by not creating a new channel!  It is basically
sending and receiving on the same channel?  Something causing whole plane updates?
We should have a separate control for which data set is sent to image analysis, not re-use the
height field.  */

//------------------------------------------------------------------------
// These are the "magic names" of the device that returns values to
// the nanoManipulator program and the device on the nanoManipulator
// that sends values.

const char g_return_name[] = "nMreturn";
const char g_send_name[] = "nM";

//------------------------------------------------------------------------
// The "well-known" port number to listen on for connections.

const unsigned short g_port_number = 5997;

#ifdef _WIN32
// turns off warning C4290: C++ Exception Specification ignored
#pragma warning( push )
#pragma warning( disable : 4290 )
#endif

imagerPlugin::imagerPlugin(nmb_Dataset *data_set) :
  d_data_set(data_set),
  d_linebuf(NULL),
  d_connection(NULL),
  d_imager_server(NULL),
  d_imager_client(NULL),
  d_text_client(NULL),
  d_tracker_client(NULL),
  d_server_stand_in(NULL),
  d_incoming_data_matches(false),
  d_left(30000), d_right(0), d_top(30000), d_bottom(0),
  d_channel_id(-1),
  d_source_plane(NULL),
  d_input_plane(NULL)
{
  //fprintf(stderr,"XXX Constructing imagerPlugin\n");
  // Check our parameters
  if (!d_data_set) {
    fprintf(stderr,"imagerPlugin::imagerPlugin(): NULL data set pointer, becoming inactive\n");
    return;
  }

  // Create the connection on which traffic to and from this imager
  // will flow.
  d_connection = new vrpn_Synchronized_Connection(g_port_number);
  if (d_connection == NULL) {
    fprintf(stderr,"imagerPlugin::imagerPlugin(): Cannot create connection, becoming inactive\n");
    return;
  }

  // Register a callback handler for new connections, so that we can send
  // the entire height field over when we get a new one.
  d_connection->register_handler(d_connection->register_message_type(vrpn_got_connection), handle_new_connection, this, vrpn_ANY_SENDER);

  // Create the various client and server objects that use this connection.
  // The "server_stand_in" is a device that will send responses to the
  // clients' ping requests to prevent warning and error messages from VRPN
  // that the servers do not exist.  A bit of a hack...
  d_tracker_client = new vrpn_Tracker_Remote(g_return_name, d_connection);
  d_text_client = new vrpn_Text_Receiver(g_return_name, d_connection);
  d_imager_client = new vrpn_TempImager_Remote(g_return_name, d_connection);
  d_server_stand_in = new vrpn_Text_Sender(g_return_name, d_connection);
  d_imager_server = new vrpn_TempImager_Server(g_send_name, d_connection,
    d_data_set->inputGrid->numX(), d_data_set->inputGrid->numY(),
    d_data_set->inputGrid->minX(), d_data_set->inputGrid->maxX(),
    d_data_set->inputGrid->minY(), d_data_set->inputGrid->maxY());

  // Make sure they were constructed
  if ( !d_tracker_client || !d_text_client || !d_imager_client || !d_imager_server ) {
    fprintf(stderr,"imagerPlugin::imagerPlugin(): Cannot create objects, becoming inactive\n");
    delete d_connection;
    d_connection = NULL;
    return;
  }

  // Register the callback handlers for the client objects.  Pass pointer to
  // the creating object as the userdata parameter.
  d_imager_client->register_description_handler(this, handle_description_message);
  d_imager_client->register_region_handler(this, handle_region_message);
  d_tracker_client->register_change_handler(this, handle_tracker_message);
  d_text_client->register_message_handler(this, handle_text_message);

  // Initialize the position at which text will be drawn to the origin
  d_pos[0] = d_pos[1] = d_pos[2] = 0.0;
  d_quat[0] = d_quat[1] = d_quat[2] = 0.0; d_quat[3] = 1.0;

  // Connect a new channel and set up callback handler for the plane.
  if (!connect_new_source_plane( d_data_set->heightPlaneName->string() )) {
    fprintf(stderr, "imagerPlugin::imagerPlugin(): Could not connect height plane '%s'\n",
      d_data_set->heightPlaneName->string());
    teardown();
    return;
  }

  // Allocate a buffer of the correct length to hold one line of data
  if (!allocate_line_buffer()) {
    fprintf(stderr,"imagerPlugin::imagerPlugin(): Cannot allocate line buffer\n");
    teardown();
    return;
  }

  // Send the whole image over the connection
  send_whole_image();
}

void imagerPlugin::teardown()
{
  if (d_source_plane) { d_source_plane->remove_callback( handle_height_plane_modified , this ); }
  if (d_linebuf != NULL) { delete [] d_linebuf; };

  if (d_tracker_client) { delete d_tracker_client; d_tracker_client = NULL; }
  if (d_imager_server) { delete d_imager_server; d_imager_server = NULL; }
  if (d_imager_client) { delete d_imager_client; d_imager_client = NULL; }
  if (d_text_client) { delete d_text_client; d_text_client = NULL; }
  if (d_connection) {
    d_connection->unregister_handler(d_connection->register_message_type(vrpn_got_connection), handle_new_connection, this, vrpn_ANY_SENDER);
    delete d_connection; d_connection = NULL;
  }
}

imagerPlugin::~imagerPlugin(void)
{
  //fprintf(stderr,"XXX Destroying imagerPlugin\n");
  teardown();
}

void imagerPlugin::send_whole_image(void)
{
  d_top = 0;
  d_bottom = d_data_set->inputGrid->numY() - 1;
  d_left = 0;
  d_right = d_data_set->inputGrid->numX() - 1;
  send_region_update();
}

void imagerPlugin::send_region_update(void)
{
  // Return if we don't have a source plane.
  if (!d_source_plane) {
    return;
  }

  // Exit if the region is empty (nothing changed).
  if ( (d_bottom < d_top) || (d_right < d_left) ) {
    return;
  }

  // Send the part of the region that has changed.  Send it one
  // row at a time to avoid overflowing the buffer size for
  // the send.  Make sure to mainloop() the imager client in case
  // the other side is sending something back so that we don't
  // end up hung while there is no consumer for its data.
  // We cannot use the flatValueArray to send the data because
  // it has possible padding in it.  We also need to scale and
  // offset it into the correct units.  Need to copy into our line
  // buffer first and send that.
  int r, c;
  for (r = d_top; r <= d_bottom; r++) {
    for (c = d_left; c <= d_right; c++) {
      d_linebuf[c] = d_source_plane->value(c, r);
    }
    d_imager_server->send_region_using_first_pointer(
      d_channel_id,
      d_left, d_right,	r, r, 
      &d_linebuf[d_left],
      1, d_source_plane->numX(), d_source_plane->numY());
    d_imager_client->mainloop();
  }

  // Set the change region boundaries so that they indicate no
  // region and they will be adjusted properly by the min and max
  // calls in the plane update callback.
  d_top = 30000;
  d_bottom = 0;
  d_left = 30000;
  d_right = 0;
}

bool  imagerPlugin::allocate_line_buffer(void)
{
  d_source_plane = d_data_set->inputGrid->getPlaneByName(d_data_set->heightPlaneName->string());
  if (!d_source_plane) {
    fprintf(stderr,"imagerPlugin::allocate_line_buffer(): No source plane\n");
    return false;
  }
  if (d_linebuf != NULL) { delete [] d_linebuf; d_linebuf = NULL; };
  if ( (d_linebuf = new vrpn_float32[d_source_plane->numX()]) == NULL) {
    fprintf(stderr,"imagerPlugin::allocate_line_buffer(): Out of memory for linebuf\n");
    return false;
  }
  return true;
}

bool imagerPlugin::connect_new_source_plane( const char *plane_name )
{   
  // Unregister the last callback handler, if there is one, using the old source plane.
  if (d_source_plane) { d_source_plane->remove_callback( handle_height_plane_modified , this ); }

  // Set the new source plane and register a new callback handler.
  d_source_plane = d_data_set->inputGrid->getPlaneByName(plane_name);
  if (d_source_plane == NULL) {
    fprintf(stderr, "imagerPlugin::connect_new_source_plane(): Could not get height plane '%s'\n",
      d_last_height_name.string());
    return false;
  }
  d_source_plane->add_callback( handle_height_plane_modified , this );

  /// Get a new channel to send to the server, returns index of the channel or -1 on failure.
  // Name it the same as the plane it was derived from.
  d_channel_id = d_imager_server->add_channel(plane_name, d_source_plane->units()->c_str(),
		    -1e10, 1e10);
  if (d_channel_id == -1) {
    fprintf(stderr,"imagerPlugin::connect_new_source_plane(): Invalid channel ID\n");
    return false;
  }

  return true;
}

static bool floats_close(double f1, double f2) { return fabs(f1-f2) < 1e-2; };

void imagerPlugin::heartbeat(void)
{
  // Bail if we don't have a connection
  if (!d_connection) { return; }

  // See if the resolution or footprint of the grid has changed.
  // If so, we need to set the size differently and send a new description message.
  if ( (d_data_set->inputGrid->numX() != d_imager_server->nCols()) ||
       (d_data_set->inputGrid->numY() != d_imager_server->nRows()) ||
       !floats_close(d_data_set->inputGrid->minX(), d_imager_server->minX()) ||
       !floats_close(d_data_set->inputGrid->maxX(), d_imager_server->maxX()) ||
       !floats_close(d_data_set->inputGrid->minY(), d_imager_server->minY()) ||
       !floats_close(d_data_set->inputGrid->maxY(), d_imager_server->maxY()) ) {
    d_imager_server->set_resolution(d_data_set->inputGrid->numX(), d_data_set->inputGrid->numY());
    d_imager_server->set_range(d_data_set->inputGrid->minX(), d_data_set->inputGrid->maxX(),
      d_data_set->inputGrid->minY(), d_data_set->inputGrid->maxY());

    // Allocate a new buffer to hold each line of data as it is sent.
    if (!allocate_line_buffer()) {
      fprintf(stderr,"imagerPlugin::heartbeat(): Cannot allocate line buffer\n");
      teardown();
      return;
    }
  }

  // Check to see if the data set mapped to height has changed.  If so,
  // then we need to send the whole new one over the connection.  We also need
  // to change out the callback handler so we are watching the right plane.
  if (strcmp(d_last_height_name, d_data_set->heightPlaneName->string()) ) {

    if (!connect_new_source_plane( (const char*)*d_data_set->heightPlaneName )) {
      fprintf(stderr,"imagerPlugin::heartbeat(): Cannot connect new source plane\n");
      teardown();
      return;
    }

    // Send the whole new image, then re-set the name of the height plane.
    send_whole_image();
    d_last_height_name.Set(*d_data_set->heightPlaneName);
  }

  // Send any region of the image that has changed since the last time
  // we came through
  send_region_update();

  // Call the mainloop() function on all of our clients, then servers, then
  // connection.
  d_tracker_client->mainloop();
  d_text_client->mainloop();
  d_imager_client->mainloop();
  d_imager_server->mainloop();
  d_server_stand_in->mainloop();
  d_connection->mainloop();
}

//------------------------------------------------------------------------
// The description message is guaranteed to be called before any region
// updates are called for the imager client.  We use this to initialize
// a data plane to hold the image that is coming over (after checking that
// its dimensions match what we expect).

void  imagerPlugin::handle_description_message(void *userdata, const struct timeval)
{
  // Turn the userdata pointer into a pointer to the instance of the class
  // that has registered this callback handler.
  imagerPlugin	*me = (imagerPlugin *)userdata;

  // Get any required information from the imager client.  It will
  // have been filled in by the time this callback is called.
  vrpn_float32 minX = me->d_imager_client->minX();
  vrpn_float32 maxX = me->d_imager_client->maxX();
  vrpn_float32 minY = me->d_imager_client->minY();
  vrpn_float32 maxY = me->d_imager_client->maxY();
  int nCols = me->d_imager_client->nCols();
  int nRows = me->d_imager_client->nRows();

  // Make sure that the dimensions of the incoming image match those in
  // the grid object.  If not, complain and disable the region handling
  // routine.  If so, set the d_incoming_data_matches flag so that
  // regions will be handled when they arrive.
  if ( (nCols == me->d_data_set->inputGrid->numX()) &&
       (nRows == me->d_data_set->inputGrid->numY()) &&
       floats_close(minX, me->d_data_set->inputGrid->minX()) &&
       floats_close(maxX, me->d_data_set->inputGrid->maxX()) &&
       floats_close(minY, me->d_data_set->inputGrid->minY()) &&
       floats_close(maxY, me->d_data_set->inputGrid->maxY()) ) {
    me->d_incoming_data_matches = true;
  } else {
    fprintf(stderr,"imagerPlugin::handle_description_message(): Description doesn't match grid!\n");
    me->d_incoming_data_matches = false;
    return;
  }

  // Make sure we have at least one channel defined.
  if (me->d_imager_client->nChannels() <= 0) {
    fprintf(stderr,"imagerPlugin::handle_description_message(): No channels in image description!\n");
    me->d_incoming_data_matches = false;
    return;
  }

  // See if a plane with the same name as the incoming data set matches.  If it already
  // exists, then there is not need to do the rest of the creation routine.
  // If not, create one (it will be filled in by the region handler).
  // We assume that we are reading from channel 0 of the image.
  //printf("XXX Creating plane %s\n", me->d_imager_client->channel(0)->name);
  me->d_input_plane = me->d_data_set->inputGrid->getPlaneByName(me->d_imager_client->channel(0)->name);
  if (me->d_input_plane == NULL) {
    me->d_input_plane = me->d_data_set->inputGrid->addNewPlane( 
      me->d_imager_client->channel(0)->name, me->d_imager_client->channel(0)->units, NOT_TIMED);

    if( me->d_input_plane == NULL ) {
      fprintf(stderr,"imagerPlugin::handle_description_message(): Cannot create new plane %s\n",
	me->d_imager_client->channel(0)->name);
      me->d_incoming_data_matches = false;
    }

    //Add this to the list of planes that can be mapped to display techniques
    me->d_data_set->inputPlaneNames->addEntry(me->d_imager_client->channel(0)->name);

    // Create an image entry for this plane.
    TopoFile tf;
    nmb_Image* im = me->d_data_set->dataImages->getImageByPlane( me->d_source_plane );
    nmb_Image* output_im = new nmb_ImageGrid( me->d_input_plane );
    if( im != NULL ) {
      im->getTopoFileInfo(tf);
      output_im->setTopoFileInfo(tf);
    } else {
      fprintf(stderr, "imagerPlugin::handle_description_message, "
	"input image not in list\n");
    }
    me->d_data_set->dataImages->addImage(output_im);
  }
}

void  imagerPlugin::handle_region_message(void *userdata, const vrpn_IMAGERREGIONCB info)
{
  // Turn the userdata pointer into a pointer to the instance of the class
  // that has registered this callback handler.
  imagerPlugin	*me = (imagerPlugin *)userdata;

  // Give up if the incoming data doesn't match
  if (!(me->d_incoming_data_matches)) {
    fprintf(stderr,"imagerPlugin::handle_region_message(): Got mismatched region (ignoring)\n");
    return;
  }

  // Get any required information from the imager client.  It will
  // have been filled in by the time this callback is called.
  vrpn_float32 minX = me->d_imager_client->minX();
  vrpn_float32 maxX = me->d_imager_client->maxX();
  vrpn_float32 minY = me->d_imager_client->minY();
  vrpn_float32 maxY = me->d_imager_client->maxY();
  int nCols = me->d_imager_client->nCols();
  int nRows = me->d_imager_client->nRows();

  // Fill the incoming data into the plane pointed to by
  // our calculatedPlane data member.  Read the values,
  // apply scale and offset, and store them.
  vrpn_float32	val;
  int r,c;
  vrpn_int16  channel_index = info.region->d_chanIndex;
  vrpn_float32 offset = me->d_imager_client->channel(channel_index)->offset;
  vrpn_float32 scale = me->d_imager_client->channel(channel_index)->scale;

  for (r = info.region->d_rMin; r <= info.region->d_rMax; r++) {
    for (c = info.region->d_cMin; c <= info.region->d_cMax; c++) {
      if (!info.region->read_unscaled_pixel(c, r, val)) {
	fprintf(stderr, "imagerPlugin::handle_region_message(): Pixel out of range (%d,%d)\n", c, r);
	me->d_incoming_data_matches = false;
      }
      me->d_input_plane->setValue(c, r, offset + val * scale);
    }
  }
}

// A tracker message indicates that the next text message that is recieved
// should be placed at the position and orientation indicated in the message.
// We store this away for later, in case a text message is recieved.

void  imagerPlugin::handle_tracker_message(void *userdata, const vrpn_TRACKERCB info)
{
  // Turn the userdata pointer into a pointer to the instance of the class
  // that has registered this callback handler.
  imagerPlugin	*me = (imagerPlugin *)userdata;

  // Store the position and orientation for when the next text message arrives.
  me->d_pos[0] = info.pos[0];
  me->d_pos[1] = info.pos[1];
  me->d_pos[2] = info.pos[2];
  me->d_quat[0] = info.quat[0];
  me->d_quat[1] = info.quat[1];
  me->d_quat[2] = info.quat[2];
  me->d_quat[3] = info.quat[3];
}

void  imagerPlugin::handle_text_message(void *userdata, const vrpn_TEXTCB info)
{
  if (info.type != vrpn_TEXT_NORMAL) {
    return;
  }

  // Turn the userdata pointer into a pointer to the instance of the class
  // that has registered this callback handler.
  imagerPlugin	*me = (imagerPlugin *)userdata;

  // Do the stuff
  printf("XXX Got text message: %s\n", info.message);
  printf("XXX  (would be at %f,%f,%f)\n", me->d_pos[0], me->d_pos[1], me->d_pos[2]);
  //XXX

}

int  imagerPlugin::handle_new_connection(void *userdata, vrpn_HANDLERPARAM)
{
  // Turn the userdata pointer into a pointer to the instance of the class
  // that has registered this callback handler.
  imagerPlugin	*me = (imagerPlugin *)userdata;

  // XXX Hack -- this needs to be replaced by the more general Imager send/receive request protocol when it exists.
  // Set the resolution to force the description to be sent, then send the whole
  // image.
  me->d_imager_server->set_resolution(me->d_data_set->inputGrid->numX(), me->d_data_set->inputGrid->numY());
  me->send_whole_image();

  return 0;
}

void imagerPlugin::handle_height_plane_modified( BCPlane* plane, int x, int y,
			   void* userdata)
{
  // Turn the userdata pointer into a pointer to the instance of the class
  // that has registered this callback handler.
  imagerPlugin	*me = (imagerPlugin *)userdata;

  // Keep track of where in the image changes have occured.  We do this here
  // rather than sending a new message for each point: they are batched up and
  // then sent in the heartbeat() call above to avoid tons of message
  // overhead.
  if (y < me->d_top) { me->d_top = y; }
  if (y > me->d_bottom) { me->d_bottom = y; }
  if (x < me->d_left) { me->d_left = x; }
  if (x > me->d_right) { me->d_right = x; }
}

#ifdef _WIN32
#pragma warning( pop )
#endif

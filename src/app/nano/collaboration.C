

#include <nM_coord_change.h>
#include <nmb_Decoration.h>
#include <nmb_Debug.h>
#include <Tcl_Netvar.h>

#include <nmg_Graphics.h>
#include <nmui_Component.h>
#include <nmui_PlaneSync.h>

#include "CollaborationManager.h"
#include "error_display.h"
#include "collaboration.h"
#include "microscape.h"

/// These are used for synchronizing with a remote user's streamfile playback.
static vrpn_bool isSynchronized = VRPN_FALSE;

/**
 * Radio button controlling whether we're publically or privately synched.
 * Currently assumes that only the most recently added peer is
 *  "valid";  others are a (small?) memory/network leak.
 */
void handle_synchronize_timed_change( vrpn_int32 value,
				      void * userdata ) 
{
  CollaborationManager * cm = (CollaborationManager *) userdata;
  nmui_Component * sync = cm->uiRoot();
  nmui_PlaneSync * plane_sync = cm->planeSync();
  nM_coord_change * handServer = cm->handServer();

  // are we already in the requested sync state?
  if( ( isSynchronized && value != 0 )  // already in shared state
      || ( !isSynchronized && value == 0 ) ) // already in private state
    {
      fprintf( stderr, "** In handle_synchronized_timed_change():  "
	       "redundant synchronization change.  Ignoring.\n" );
      return;
    }

  switch (value) {
    case 0:  // stop synchronizing;  use private state (#0)
      {
	// need to:
	//  - save current volatile variables into old replicas
	//  - set new replicas to be current
	//  - push new replicas into volatile variables
	
	// save volatile variables
	set_stream_time = decoration->elapsedTime;

	// switch to replicas of requested state
	sync->syncReplica(0);
	
	// start use new volatile variable
	set_stream_time_now = 1;
	
	// stop accepting new calculated planes
	plane_sync->queueUpdates();

	// turn off collaborator's hand in our view
	graphics->enableCollabHand(VRPN_FALSE);
	handServer->stopSync();

	isSynchronized = VRPN_FALSE;
      }
      break;
    default:  // use shared state (#1)
      {
	// Set our private replica without transmitting anything
	// over the network.
	set_stream_time.setReplica(0, decoration->elapsedTime);
	// this can only be reliably done with the above guard
	// against idempotent sync state changes.
	
	sync->syncReplica( sync->numPeers( ) );
	
	sync->requestSync();
	// must wait until this completes before using new stream time
	// we know this is completed with the call to 
	// handle_timed_sync_complete

	// set this so that handle_timed_sync_complete knows that it's
	// being called as the result of a sync state change
	sync->d_maintain = VRPN_TRUE;

	// turn on our collaborator's hand in our view
	if (handServer->peerIsSynchronized()) {
	  graphics->enableCollabHand(VRPN_TRUE);
	}
	handServer->startSync();

	isSynchronized = VRPN_TRUE;
      }
      break;
  } 

} // end handle_synchronize_timed_change



void handle_peer_sync_change (void * /*userdata*/, vrpn_bool value) 
{
  if (isSynchronized && value) // both synchronized
    { graphics->enableCollabHand(VRPN_TRUE); } 
  else 
    { graphics->enableCollabHand(VRPN_FALSE); }
} // end handle_peer_sync_change



int handle_timed_sync_request( void* ) 
{
  // write into the current replica of any volatile variables
  // in order for force a comparison with those of the collaborator
  set_stream_time = decoration->elapsedTime;

  return 0;
} // end handle_timed_sync_request



int local_time_sync( void* )
{
  if( isSynchronized )
    fprintf( stderr, "Warning:  in local_time_sync:  setting private "
	     "replica while in shared state.\n" );
  // Set our private replica without transmitting anything over the network.
  set_stream_time.setReplica(0, decoration->elapsedTime);
  // note that this is dangerous if we're not in private state
  return 0;
} // end local_time_sync


int handle_timed_sync_complete( void * userdata ) 
{
  CollaborationManager * cm = (CollaborationManager *) userdata;
  nmui_Component * sync = cm->uiRoot();
  nmui_PlaneSync * plane_sync = cm->planeSync();

  // requestSync() is only called, and so this will only be generated,
  // when we are going to the shared replica
  
  int useReplica = sync->numPeers();
 
  if (sync->d_maintain) // completion of a syncReplica call
    { 
      // Make sure we save the state of any volatile variables -
      // if latency is high the previous save will be off by a few
      // seconds (which we could forget).
      // handle_timed_sync_request(NULL);
      // this will write into the current replica, not the old
      // replica whose state we're coming from.
      
      // Start synchronizing planes and create any planes from 
      // the new state.
      plane_sync->acceptUpdates();
      sync->syncReplica(useReplica);
      
      // start using the new volatile variables
      set_stream_time_now = 1;
    } 
  else // completion of a copyReplica call
    { 
      sync->copyReplica(useReplica);
      
      // Create any planes from copied state, but go back to 
      // queueing new plane creations.
      plane_sync->acceptUpdates();
      plane_sync->queueUpdates();  
  }
  return 0;
} // end handle_timed_sync_complete


/**
 * Linked to button in tcl UI. If pressed, copy the shared state to
 * the private state.
 */
void handle_copy_to_private( vrpn_int32 /*value*/, void * userdata )
{
  CollaborationManager * cm = (CollaborationManager *) userdata;
  nmui_Component * sync = cm->uiRoot();
  nmui_PlaneSync * plane_sync = cm->planeSync();

  if (!sync->synchronizedTo()) {
    // we are private. We need to get current data for shared state.

    // a copyReplica needs to be deferred until the new data arrives
    // The right way to do this is probably to have a Component's
    // sync handler pack a "syncComplete" message AFTER all the
    // callbacks have been triggered (=> the sync messages are marshalled
    // for VRPN), so when that arrives we know the sync is complete and
    // we can issue a copyReplica()

    sync->requestSync();
    sync->d_maintain = VRPN_FALSE;
  } else {
    // get up to date stream time. 
    handle_timed_sync_request( NULL );
    
    // we are shared, copy to local state immediately.
    // shared state is in the replica from the most recent peer.
    sync->copyFromToReplica(sync->numPeers(), 0);
    
    plane_sync->acceptUpdates();
    plane_sync->queueUpdates();

    // get up-to-date stream time.
    local_time_sync( NULL );
  }
} // end handle_copy_to_private


/**
 * Linked to button in tcl UI. If pressed, copy the private state to
 * the shared state.
 */
void handle_copy_to_shared (vrpn_int32 /*value*/, void * userdata) {
  CollaborationManager * cm = (CollaborationManager *) userdata;
  nmui_Component * sync = cm->uiRoot();
  nmui_PlaneSync * plane_sync = cm->planeSync();

  if (!sync->synchronizedTo()) {
      // we are local. We can copy to shared state immediately
      // shared state is in the replica from the most recent peer.
      sync->copyFromToReplica(0, sync->numPeers());

      // we also want to get any planes which might be from the shared state 
      plane_sync->acceptUpdates();
      plane_sync->queueUpdates();
 
  } else {
      // we are shared, want to copy from local
      // We know the current shared state, because we are shared,
      // so copy immediately.
      // shared state is in the replica from the most recent peer.
      sync->copyFromToReplica(0, sync->numPeers());

      // Any planes created in local state will already have been copied to
      // shared state, so no need to sync planes.

  }
}



/// Updating both X and Y position of the line at the same time
/// doesn't work - it locks down one coord. This semaphor prevents that. 
static vrpn_bool ignoreCollabMeasureChange = 0;

// NANOX
void handle_collab_red_measure_change (vrpn_float64 /*newValue*/,
				       void * userdata) 
{ handle_collab_measure_change( (nmb_Dataset *) userdata, 0); }

void handle_collab_green_measure_change (vrpn_float64 /*newValue*/,
					 void * userdata) 
{ handle_collab_measure_change( (nmb_Dataset *) userdata, 1); }

void handle_collab_blue_measure_change (vrpn_float64 /*newValue*/,
					void * userdata) 
{ handle_collab_measure_change( (nmb_Dataset *) userdata, 2); }

/// Line position has changed in tcl (probably due to update from
/// collaborator) so change the position onscreen.
void handle_collab_measure_change (nmb_Dataset * data,
                                          int whichLine) 
{
  // Ignore some changes caused by tcl variables. 
  if (ignoreCollabMeasureChange) return;
  
  BCPlane * heightPlane = data->inputGrid->getPlaneByName
    (data->heightPlaneName->string());
  if (!heightPlane) {
    display_error_dialog( "Internal: Couldn't find height plane named %s.\n",
			  data->heightPlaneName->string());
    return;
  }
  collabVerbose(5, "handle_collab_measure_change for line %d.\n", whichLine);
  switch (whichLine) {
    
  case 0:
    collabVerbose(5, "Moving RED line to %.3f, %.3f due to Tcl change.\n",
		  measureRedX, measureRedY);
    decoration->red.moveTo(measureRedX, measureRedY, heightPlane);
    // DO NOT doCallbacks()
    updateRulergridOffset();
    updateRulergridAngle();  // Do this so angle stays right, if toggle is on. 
    break;
    
  case 1:
    collabVerbose(5, "Moving GREEN line to %.3f, %.3f due to Tcl change.\n",
		  measureRedX, measureRedY);
    decoration->green.moveTo(measureGreenX, measureGreenY, heightPlane);
    // DO NOT doCallbacks()
    updateRulergridAngle();
    break;
    
  case 2:
    collabVerbose(5, "Moving BLUE line to %.3f, %.3f due to Tcl change.\n",
		  measureRedX, measureRedY);
    decoration->blue.moveTo(measureBlueX, measureBlueY, heightPlane);
    // DO NOT doCallbacks()
    break;
  }
}

// NANOX
/** If the user changes the measure line positon, update Tcl variables.
If we are collaborating, this will update our collaborator.
Because we want to set both the X and Y position of the line at 
the same time, we explicitly tell the other handler to ignore
the change we make to X, and pay attention to the change to Y. 

doMeasure() in interaction.c triggers handle_collab_measure_move.
Through the magic of Tcl_Linkvar/Tcl_Netvar, this will take care of
any necessary network synchronization or collaboration, and at the end
will call handle_collab_measure_change(), above, which executes the
final moveTo() on each nmb_Line and updates rulergrid parameters if
necessary.
*/
void handle_collab_measure_move (float x, float y,
					void * userdata) 
{
  int whichLine = (int) userdata;   // hack to get the data here
  
  collabVerbose(5, "handle_collab_measure_move for line %d.\n", whichLine);
  
  switch (whichLine) {
  case 0:
    ignoreCollabMeasureChange = VRPN_TRUE;
    measureRedX = x;
    ignoreCollabMeasureChange = VRPN_FALSE;
    measureRedY = y;
    break;
  case 1:
    ignoreCollabMeasureChange = VRPN_TRUE;
    measureGreenX = x;
    ignoreCollabMeasureChange = VRPN_FALSE;
    measureGreenY = y;
    // DO NOT doCallbacks()
    break;
  case 2:
    ignoreCollabMeasureChange = VRPN_TRUE;
    measureBlueX = x;
    ignoreCollabMeasureChange = VRPN_FALSE;
    measureBlueY = y;
    // DO NOT doCallbacks()
    break;
  }
}



void handle_collab_machine_name_change( const char * new_value,
					void * userdata )
{
  if (!new_value || !strlen(new_value)) {
    // transitory excitement during startup
    return;
  }
  CollaborationManager * cm;
  cm = (CollaborationManager *) userdata;
  
  if (cm) {
    cm->setPeerName( new_value,
		     (void *) &V_TRACKER_FROM_HAND_SENSOR,
		     handle_collab_sensor2tracker_change,
		     NULL, handle_collab_mode_change);
  }
}


void handle_collab_sensor2tracker_change(void *, 
					const vrpn_TRACKERCB info) {
    graphics->setCollabHandPos( (double*) info.pos, (double*) info.quat );
}

void handle_collab_mode_change(void *, const vrpn_ANALOGCB info) {
    graphics->setCollabMode(info.channel[0]);
}





extern CollaborationManager * collaborationManager;

/// synchronization UI handlers
// generic synchronization
//static void handle_synchronize_change (vrpn_int32, void *);
//static void handle_get_sync (vrpn_int32, void *);
//static void handle_lock_sync (vrpn_int32, void *);
// since streamfiles are time-based, we need to send a syncRequest()
static void handle_synchronize_timed_change (vrpn_int32, void *);
static void handle_timed_sync (vrpn_int32, void *);

static void handle_collab_measure_change (vrpn_float64, void *);
static void handle_center_pressed (vrpn_int32, void *);

static void handle_mutex_request (vrpn_int32, void *);
static void handle_mutex_release (vrpn_int32, void *);

// TCH 19 Jan 00 - Turned these into netvars that were logged on
// interfaceLogConnection but were NOT part of rootUIControl so that
// they wouldn't be shared but that while replaying logs we could
// determine which state to use.
TclNet_int share_sync_state ("share_sync_state", 0);
TclNet_int copy_inactive_state ("copy_inactive_state", 0);
TclNet_int copy_to_private_state ("copy_to_private_state", 0);
TclNet_int copy_to_shared_state ("copy_to_shared_state", 0);

///to get the name of the machine where the collaborator is whose hand
///position we want to track
TclNet_string collab_machine_name ("collab_machine_name", "");

TclNet_int request_mutex ("request_mutex", 0);
TclNet_int release_mutex ("release_mutex", 0);


/// Quick method of sharing measure line locations
TclNet_float measureRedX ("measure_red_x", 0.0,
                           handle_collab_measure_change, (void *) 0);
TclNet_float measureRedY ("measure_red_y", 0.0,
                           handle_collab_measure_change, (void *) 0);
TclNet_float measureGreenX ("measure_green_x", 0.0,
                           handle_collab_measure_change, (void *) 1);
TclNet_float measureGreenY ("measure_green_y", 0.0,
                           handle_collab_measure_change, (void *) 1);
TclNet_float measureBlueX ("measure_blue_x", 0.0,
                           handle_collab_measure_change, (void *) 2);
TclNet_float measureBlueY ("measure_blue_y", 0.0,
                           handle_collab_measure_change, (void *) 2);





void handle_finegrained_changed (vrpn_int32 value, void *) {
  Tcl_Interp * tk_control_interp = get_the_interpreter();
  char command [1000];
  int retval;

fprintf(stderr, "handle_finegrained_changed\n");

  if (value) {
    sprintf(command, "pack_finegrained_coupling");
  } else {
    sprintf(command, "unpack_finegrained_coupling");
  }

  retval = Tcl_Eval(tk_control_interp, command);
  if (retval != TCL_OK) {
    fprintf(stderr, "Tcl_Eval failed in handle_mutexTaken:  %s.\n",
            tk_control_interp->result);
  }
}


Tclvar_int finegrained_coupling ("finegrained_coupling", 0,
                                 handle_finegrained_changed, NULL);


void handle_mutex_request (vrpn_int32 value, void * userdata) {
  nmm_Microscope_Remote * microscope = (nmm_Microscope_Remote *) userdata;

//fprintf(stderr, "handle_mutex_request (%d)\n", value);

  if (value) {
    microscope->requestMutex();
    value = 0;
  }
}

void handle_mutex_release (vrpn_int32 value, void * userdata) {
  nmm_Microscope_Remote * microscope = (nmm_Microscope_Remote *) userdata;

//fprintf(stderr, "handle_mutex_release (%d)\n", value);

  if (value) {
    microscope->releaseMutex();
    value = 0;
  }
}

// We asked for the mutex and got it.
void handle_mutexRequestGranted (void *, nmb_SharedDevice *) {
  Tcl_Interp * tk_control_interp = get_the_interpreter();
  char command [1000];
  int retval;

//fprintf(stderr, "handle_mutexRequestGranted\n");

  sprintf(command, "mutex_gotRequest_callback");
  retval = Tcl_Eval(tk_control_interp, command);
  if (retval != TCL_OK) {
    fprintf(stderr, "Tcl_Eval failed in handle_mutexRequestGranted:  %s.\n",
            tk_control_interp->result);
  }
}

// We asked for the mutex, but somebody said "no".
void handle_mutexRequestDenied (void *, nmb_SharedDevice *) {
  Tcl_Interp * tk_control_interp = get_the_interpreter();
  char command [1000];
  int retval;

//fprintf(stderr, "handle_mutexRequestDenied\n");

  sprintf(command, "mutex_deniedRequest_callback");
  retval = Tcl_Eval(tk_control_interp, command);
  if (retval != TCL_OK) {
    fprintf(stderr, "Tcl_Eval failed in handle_mutexRequestDenied:  %s.\n",
            tk_control_interp->result);
  }
}

// Somebody else (NOT US?!) got the mutex.
void handle_mutexTaken (void *, nmb_SharedDevice *) {
  Tcl_Interp * tk_control_interp = get_the_interpreter();
  char command [1000];
  int retval;

//fprintf(stderr, "handle_mutexTaken\n");

  sprintf(command, "mutex_taken_callback");
  retval = Tcl_Eval(tk_control_interp, command);
  if (retval != TCL_OK) {
    fprintf(stderr, "Tcl_Eval failed in handle_mutexTaken:  %s.\n",
            tk_control_interp->result);
  }
}

// Anybody released the mutex.
void handle_mutexReleased (void *, nmb_SharedDevice *) {
  Tcl_Interp * tk_control_interp = get_the_interpreter();
  char command [1000];
  int retval;

//fprintf(stderr, "handle_mutexReleased\n");

  sprintf(command, "mutex_release_callback");
  retval = Tcl_Eval(tk_control_interp, command);
  if (retval != TCL_OK) {
    fprintf(stderr, "Tcl_Eval failed in handle_mutexReleased:  %s.\n",
            tk_control_interp->result);
  }
}


/// Updating both X and Y position of the line at the same time
/// doesn't work - it locks down one coord. This semaphor prevents that. 
static vrpn_bool ignoreCollabMeasureChange = 0;

/// Line position has changed in tcl (probably due to update from
/// collaborator) so change the position onscreen.
static void handle_collab_measure_change (vrpn_float64 /*newValue*/,
                                          void * userdata) {
    // Ignore some changes caused by tcl variables.
    if (ignoreCollabMeasureChange) return;

  BCPlane * heightPlane = dataset->inputGrid->getPlaneByName
                 (dataset->heightPlaneName->string());
  if (!heightPlane) {
    fprintf(stderr, "Couldn't find height plane named %s.\n",
            dataset->heightPlaneName->string());
  }
  int whichLine = (int) userdata;   // hack to get the data here

  switch (whichLine) {

    case 0:
//fprintf(stderr, "Moving RED line due to Tcl change.\n");
     decoration->red.moveTo(measureRedX, measureRedY, heightPlane);
     // DO NOT doCallbacks()
     updateRulergridOffset();
     break;

    case 1:
//fprintf(stderr, "Moving GREEN line due to Tcl change.\n");
     decoration->green.moveTo(measureGreenX, measureGreenY, heightPlane);
     // DO NOT doCallbacks()
     updateRulergridAngle();
     break;

    case 2:
//fprintf(stderr, "Moving BLUE line due to Tcl change.\n");
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
static void handle_collab_measure_move (float x, float y,
                                          void * userdata) {
  int whichLine = (int) userdata;   // hack to get the data here

  // TODO - a message needs to be sent so that this timer
  // gets unblocked!
  collaborationTimer.block(collaborationTimer.getListHead());

  switch (whichLine) {
    case 0:
//  fprintf(stderr, "Moving RED line , change Tcl .\n");
        ignoreCollabMeasureChange = VRPN_TRUE;
        measureRedX = x;
        ignoreCollabMeasureChange = VRPN_FALSE;
        measureRedY = y;
     break;
    case 1:
//fprintf(stderr, "Moving GREEN line , change Tcl .\n");
        ignoreCollabMeasureChange = VRPN_TRUE;
        measureGreenX = x;
        ignoreCollabMeasureChange = VRPN_FALSE;
        measureGreenY = y;
     // DO NOT doCallbacks()
     break;
    case 2:
//fprintf(stderr, "Moving BLUE line , change Tcl .\n");
        ignoreCollabMeasureChange = VRPN_TRUE;
        measureBlueX = x;
        ignoreCollabMeasureChange = VRPN_FALSE;
        measureBlueY = y;
     // DO NOT doCallbacks()
     break;
  }
}

// NANOX
// synchronization UI handlers
/* UNUSED
// Checkbox - if checked, request that the peer keep us synced;
//   if unchecked, stop keeping synced.
// Currently assumes that only the most recently added peer is
//   "valid";  others are a (small?) memory/network leak.
static void handle_synchronize_change (vrpn_int32 value, void * userdata) {
  nmui_Component * sync = (nmui_Component *) userdata;
  int s;

  switch (value) {
    case 0:
      // stop synchronizing;  use local state (#0)
      s = 0;
      break;
    default:
      // use shared state (#1)
      //s = 1;  // SYNC-ROBUSTNESS
      s = sync->numPeers();
fprintf(stderr, "Synchronizing with peer #%d.\n", s);
      break;
  }

  sync->syncReplica(s);
//fprintf(stderr, "++ Synchronized to replica #%d.\n", sync->synchronizedTo());

}

// Button - if pressed, request immediate sync.
// (If synchronize_stream is checked, this is meaningless.)
// Currently assumes that only the most recently added peer is
//   "valid";  others are a (small?) memory/network leak.
static void handle_get_sync (vrpn_int32, void * userdata) {
  nmui_Component * sync = (nmui_Component *) userdata;
  int c;
  // Christmas sync
  // handles 2-way;  may handle n-way sync
  switch (sync->synchronizedTo()) {
    case 0:
      // copy shared state (#1) into local state (#0)
      //c = 1;  // SYNC-ROBUSTNESS
      c = sync->numPeers();
fprintf(stderr, "Copying peer #%d.\n", c);
      break;
    default:
      // copy local state (#0) into shared state (#1)
      c = 0;
      break;
  }
  sync->copyReplica(c);
//fprintf(stderr, "++ Copied inactive replica (#%d).\n", c);
}
*/
static int handle_timed_sync_request (void *);

struct sync_plane_struct {
    nmui_Component * component;
    nmui_PlaneSync * planesync;
};

/** Checkbox - if checked, request that the peer keep us synced;
  if unchecked, stop keeping synced.
Currently assumes that only the most recently added peer is
  "valid";  others are a (small?) memory/network leak.
*/
static void handle_synchronize_timed_change (vrpn_int32 value,
                                             void * userdata) {
  CollaborationManager * cm = (CollaborationManager *) userdata;
  nmui_Component * sync = cm->uiRoot();
  nmui_PlaneSync * plane_sync = cm->planeSync();
  nM_coord_change * handServer = cm->handServer();

fprintf(stderr, "++ In handle_synchronized_timed_change() to %d\n", value);

  // First write out the current values of any volatile variables.
  // There is some risk of this doing the wrong thing, since we're
  // likely to be slightly out of sync with our replica and they may
  // get this as an update message, causing their stream to jump
  // backwards - HACK XXX.  Correct behavior may need to be implemented
  // at a lower level that knows whether or not we're the synchronizer.
  // ALL of the sync code was written for optimism, and requires some
  // tougher semantics to use centralized serialization and
  // single-shared-state.
  handle_timed_sync_request(NULL);

  switch (value) {
    case 0:
      // stop synchronizing;  use local state (#0)
fprintf(stderr, "++   ... stopped synchronizing.\n");
      sync->syncReplica(0);
      plane_sync->queueUpdates();
      graphics->enableCollabHand(VRPN_FALSE);
      handServer->stopSync();
      isSynchronized = VRPN_FALSE;
      break;
    default:
      // use shared state (#1)
fprintf(stderr, "++   ... sent synch request to peer.\n");

      // You'd think that this call to block() wouldn't need to be explicitly
      // unblocked, since we're going to get a syncComplete message from
      // the peer, but it does - we need to know what index to unblock
      // after the syncComplete, which requres an additional message.
      collaborationTimer.block(collaborationTimer.getListHead());

      sync->requestSync();
      sync->d_maintain = VRPN_TRUE;
      if (handServer->peerIsSynchronized()) {
        graphics->enableCollabHand(VRPN_TRUE);
      }
      handServer->startSync();
      isSynchronized = VRPN_TRUE;
      // We defer the syncReplica until after the requestSync() completes.
      break;
  }

}

static void handle_peer_sync_change (void * /*userdata*/, vrpn_bool value) {

fprintf(stderr, "handle_peer_sync_change called, value %d\n",value);
  if (isSynchronized && value) {  // both synchronized
    graphics->enableCollabHand(VRPN_TRUE);
  } else {
    graphics->enableCollabHand(VRPN_FALSE);
  }

}

/// Linked to button in tcl UI. If pressed, copy the shared state to
/// the private state.
static void handle_copy_to_private (vrpn_int32 /*value*/, void * userdata) {
  CollaborationManager * cm = (CollaborationManager *) userdata;
  nmui_Component * sync = cm->uiRoot();
  nmui_PlaneSync * plane_sync = cm->planeSync();

  if (!sync->synchronizedTo()) {
      // we are local. We need to get current data for shared state.

    // a copyReplica needs to be deferred until the new data arrives
    // The right way to do this is probably to have a Component's
    // sync handler pack a "syncComplete" message AFTER all the
    // callbacks have been triggered (=> the sync messages are marshalled
    // for VRPN), so when that arrives we know the sync is complete and
    // we can issue a copyReplica()

    // You'd think that this call to block() wouldn't need to be explicitly
    // unblocked, since we're going to get a syncComplete message from
    // the peer, but it does - we need to know what index to unblock
    // after the syncComplete, which requres an additional message.
    collaborationTimer.block(collaborationTimer.getListHead());

    sync->requestSync();
    sync->d_maintain = VRPN_FALSE;
fprintf(stderr, "++ In handle_copy_to_private()sent synch request\n");
  } else {
      // get up to date stream time.
      handle_timed_sync_request(NULL);

      // we are shared, copy to local state immediately.
      // shared state is in the replica from the most recent peer.
      sync->copyFromToReplica(sync->numPeers(), 0);
fprintf(stderr, "++ In handle_copy_to_private() copied immediately.\n");
    plane_sync->acceptUpdates();
    plane_sync->queueUpdates();

  }

}

/// Linked to button in tcl UI. If pressed, copy the private state to
/// the shared state.
static void handle_copy_to_shared (vrpn_int32 /*value*/, void * userdata) {
  CollaborationManager * cm = (CollaborationManager *) userdata;
  nmui_Component * sync = cm->uiRoot();
  nmui_PlaneSync * plane_sync = cm->planeSync();

  if (!sync->synchronizedTo()) {
      // we are local. We can copy to shared state immediately
      // shared state is in the replica from the most recent peer.
      sync->copyFromToReplica(0, sync->numPeers());
fprintf(stderr, "++ In handle_copy_to_shared() request sync.\n");

      // we also want to get any planes which might be from the shared state
      plane_sync->acceptUpdates();
      plane_sync->queueUpdates();

  } else {
      // we are shared, want to copy from local
      // We know the current shared state, because we are shared,
      // so copy immediately.
      // shared state is in the replica from the most recent peer.
      sync->copyFromToReplica(0, sync->numPeers());
fprintf(stderr, "++ In handle_copy_to_shared() copy immediately.\n");
      // Any planes created in local state will already have been copied to
      // shared state, so no need to sync planes.

  }

}

/** Button - if pressed, request immediate sync.
(If synchronize_stream is checked, this is meaningless.)
Currently assumes that only the most recently added peer is
  "valid";  others are a (small?) memory/network leak.
*/
static void handle_timed_sync (vrpn_int32 /*value*/, void * userdata) {
  CollaborationManager * cm = (CollaborationManager *) userdata;
  nmui_Component * sync = cm->uiRoot();
  nmui_PlaneSync * plane_sync = cm->planeSync();
  int copyFrom = !sync->synchronizedTo();

  //// only run once
  //if (!value) {
    //return;
  //}

  if (copyFrom) {

    // a copyReplica needs to be deferred until the new data arrives
    // The right way to do this is probably to have a Component's
    // sync handler pack a "syncComplete" message AFTER all the
    // callbacks have been triggered (=> the sync messages are marshalled
    // for VRPN), so when that arrives we know the sync is complete and
    // we can issue a copyReplica()

    // You'd think that this call to block() wouldn't need to be explicitly
    // unblocked, since we're going to get a syncComplete message from
    // the peer, but it does - we need to know what index to unblock
    // after the syncComplete, which requres an additional message.
    collaborationTimer.block(collaborationTimer.getListHead());

    sync->requestSync();
    sync->d_maintain = VRPN_FALSE;
fprintf(stderr, "++ In handle_timed_sync() to %d;  "
"sent synch request to peer.\n", copyFrom);
  } else {

    sync->copyReplica(copyFrom);
fprintf(stderr, "++ In handle_timed_sync() to %d;  copied immediately.\n",
copyFrom);
    plane_sync->acceptUpdates();
    plane_sync->queueUpdates();

  }

}

static int handle_timed_sync_request (void *) {
  // Write the current elapsed time of the stream
  // into set_stream_time / replica[0] without messing up
  // our playback.

  set_stream_time = decoration->elapsedTime;

fprintf(stderr, "++ In handle_timed_sync_request() at %ld seconds;  "
"wrote data into replica.\n", decoration->elapsedTime);

  return 0;
}

static int handle_timed_sync_complete (void * userdata) {
  CollaborationManager * cm = (CollaborationManager *) userdata;
  nmui_Component * sync = cm->uiRoot();
  nmui_PlaneSync * plane_sync = cm->planeSync();

  //int useReplica = !sync->synchronizedTo();
  // requestSync() is only called, and so this will only be generated,
  // when we are going to the shared replica.
  //int useReplica = 1;  // SYNC-ROBUSTNESS
  int useReplica = sync->numPeers();

fprintf(stderr, "++ In handle_timed_sync_complete();  "
"getting data from replica %d.\n", useReplica);

  // Once we have the *latest* state of time-depenedent values,
  // update with that.

  if (sync->d_maintain) {

    // Make sure we save the state of any volatile variables -
    // if latency is high the previous save will be off by a few
    // seconds (which we could forget).
    handle_timed_sync_request(NULL);

    // Start synchronizing planes and create any planes from
    // the new state.
    plane_sync->acceptUpdates();
    sync->syncReplica(useReplica);
//fprintf(stderr, "++   ... synched.\n");
  } else {
    sync->copyReplica(useReplica);

    // Create any planes from copied state, but go back to
    // queueing new plane creations.
    plane_sync->acceptUpdates();
    plane_sync->queueUpdates();

//fprintf(stderr, "++   ... copied.\n");
  }

  return 0;
}








void shutdown_collaboration (void) {


  measureRedX.bindConnection(NULL);
  measureRedY.bindConnection(NULL);
  measureGreenX.bindConnection(NULL);
  measureGreenY.bindConnection(NULL);
  measureBlueX.bindConnection(NULL);
  measureBlueY.bindConnection(NULL);

}


// NANOX
// Tom Hudson, September 1999
// Sets up synchronization callbacks and nmui_Component/ComponentSync
//#define MIN_SYNC

void setupSynchronization (CollaborationManager * cm,
                           nmb_Dataset * dset,
#ifdef USE_VRPN_MICROSCOPE
                           nmm_Microscope_Remote * m) {
#else
                           Microscope * m) {
#endif

  // NOTE
  // If you add() any Netvar in this function, make sure you
  // call bindConnection(NULL) on it in shutdown_connections().

  vrpn_Connection * serverConnection = cm->peerServerConnection();
  vrpn_Connection * logConnection = cm->interfaceLogConnection();

  // These really don't need to be visible globally - they don't
  // need their mainloops called, just their connection's
  // (presumably that's collaboratingPeerServerConnection)

  // Streamfile control

  nmui_Component * streamfileControls;
  streamfileControls = new nmui_Component ("Stream");

  // Whenever replay_rate, rewind_stream, or set_stream_time changes,
  // and we are synchronizing, handle_synchronization() and
  // streamfileSync will send a message over collaboratingPeerServerConnection
  // (or whatever connection was passed into this function) to synchronize
  // the same objects on the other end.

  streamfileControls->add(&replay_rate);
  streamfileControls->add(&rewind_stream);
  streamfileControls->add(&set_stream_time);
  // Don't want to synchronize this;  TCL sets it when set_stream_time
  // sync occurs.
  //streamfileControls->add(&set_stream_time_now);


  //set_stream_time.d_permitIdempotentChanges = VRPN_TRUE;


  // View control

  nmui_Component * viewControls;
  viewControls = new nmui_Component ("View");

  // Hierarchical decomposition of view control

  nmui_Component * viewPlaneControls;
  viewPlaneControls = new nmui_Component ("View Plane");

  viewPlaneControls->add(&m->state.stm_z_scale);
  viewPlaneControls->add((TclNet_string *) dset->heightPlaneName);
  viewPlaneControls->add(&tcl_wfr_xlate_X);
  viewPlaneControls->add(&tcl_wfr_xlate_Y);
  viewPlaneControls->add(&tcl_wfr_xlate_Z);
  viewPlaneControls->add(&tcl_wfr_rot_0);
  viewPlaneControls->add(&tcl_wfr_rot_1);
  viewPlaneControls->add(&tcl_wfr_rot_2);
  viewPlaneControls->add(&tcl_wfr_rot_3);
  viewPlaneControls->add(&tcl_wfr_scale);
  //viewPlaneControls->add(&tcl_wfr_changed);
  viewPlaneControls->add(&tclstride);

  nmui_Component * viewColorControls;
  viewColorControls = new nmui_Component ("View Color");

  viewColorControls->add((TclNet_string *) dset->colorPlaneName);
  viewColorControls->add((TclNet_string *) dset->colorMapName);
  viewColorControls->add(&color_slider_min);
  viewColorControls->add(&color_slider_max);

  nmui_Component * viewMeasureControls;
  viewMeasureControls = new nmui_Component ("View Measure");

  viewMeasureControls->add(&measureRedX);
  viewMeasureControls->add(&measureRedY);
  viewMeasureControls->add(&measureGreenX);
  viewMeasureControls->add(&measureGreenY);
  viewMeasureControls->add(&measureBlueX);
  viewMeasureControls->add(&measureBlueY);

  nmui_Component * viewLightingControls;
  viewLightingControls = new nmui_Component ("View Lighting");

  viewPlaneControls->add(&tcl_lightDirX);
  viewPlaneControls->add(&tcl_lightDirY);
  viewPlaneControls->add(&tcl_lightDirZ);
  viewColorControls->add(&shiny);
  viewColorControls->add(&diffuse);
  viewColorControls->add(&surface_alpha);
  viewColorControls->add(&specular_color);

  nmui_Component * viewContourControls;
  viewContourControls = new nmui_Component ("View Contour");

  viewContourControls->add(&texture_scale);
  viewContourControls->add(&contour_width);
  viewContourControls->add(&contour_r);
  viewContourControls->add(&contour_g);
  viewContourControls->add(&contour_b);
  viewContourControls->add(&contour_changed);
  viewContourControls->add((TclNet_string *) dset->contourPlaneName);

  nmui_Component * viewGridControls;
  viewGridControls = new nmui_Component ("View Grid");

  viewGridControls->add(&rulergrid_position_line);
  viewGridControls->add(&rulergrid_orient_line);
  viewGridControls->add(&rulergrid_xoffset);
  viewGridControls->add(&rulergrid_yoffset);
  viewGridControls->add(&rulergrid_scale);
  viewGridControls->add(&rulergrid_angle);
  viewGridControls->add(&ruler_width_x);
  viewGridControls->add(&ruler_width_y);
  viewGridControls->add(&ruler_opacity);
  viewGridControls->add(&ruler_r);
  viewGridControls->add(&ruler_g);
  viewGridControls->add(&ruler_b);
  viewGridControls->add(&rulergrid_changed);
  viewGridControls->add(&rulergrid_enabled);

  viewControls->add(viewPlaneControls);
  viewControls->add(viewColorControls);
  viewControls->add(viewMeasureControls);
  viewControls->add(viewLightingControls);
  viewControls->add(viewContourControls);
  viewControls->add(viewGridControls);

  //nmui_Component * derivedPlaneControls;
  //derivedPlaneControls = new nmui_Component ("DerivedPlanes");

  // Christmas sync

  nmui_Component * rootUIControl;
  rootUIControl = new nmui_Component ("ROOT");

  //rootUIControl->add(derivedPlaneControls);
  rootUIControl->add(viewControls);
  rootUIControl->add(streamfileControls);
  rootUIControl->bindConnection(serverConnection);

  if (logConnection) {
    rootUIControl->bindLogConnection(logConnection);

    // these should be logged but not shared;  we declare them as
    // TclNet objects but don't make them part of the UI Components.

    share_sync_state.bindLogConnection(logConnection);
    copy_inactive_state.bindLogConnection(logConnection);
      copy_to_private_state.bindLogConnection(logConnection);
      copy_to_shared_state.bindLogConnection(logConnection);
    collab_machine_name.bindLogConnection(logConnection);
  }

  // User Interface to synchronization

  // NANOX FLAT
  // Set up a utility class to make sure derived planes are synchronized
  // between all replicas.

  nmui_PlaneSync * ps;

  ps = new nmui_PlaneSync (dset, &(m->state.data), serverConnection);

  // Since streamfileControls are timed, the toplevel MUST use
  // the timed callbacks.  Oops.  Took an hour or more to find,
  // that one.

  share_sync_state.addCallback
      (handle_synchronize_timed_change, cm);
  copy_inactive_state.addCallback
      (handle_timed_sync, cm);

    copy_to_private_state.addCallback
        (handle_copy_to_private, cm);
    copy_to_shared_state.addCallback
        (handle_copy_to_shared, cm);

  // need to pass rootUIControl to handle_timed_sync_complete
  // so that it sees d_maintain as TRUE!

  streamfileControls->registerSyncRequestHandler
          (handle_timed_sync_request, streamfileControls);
  streamfileControls->registerSyncCompleteHandler
          (handle_timed_sync_complete, cm);




  // which machine collaborator is using
  cm->setUI(rootUIControl);
  cm->setPlaneSync(ps);
  collab_machine_name.addCallback
        (handle_collab_machine_name_change, cm);

}





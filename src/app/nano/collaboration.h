
// A number of independent functions written for collaboration.
// Some are handlers for TclVars.

// synchronization time handlers
extern int handle_timed_sync_request( void* );
extern int local_time_sync( void* );
extern void handle_synchronize_timed_change( vrpn_int32, void * );
extern int handle_timed_sync_complete( void * userdata );
extern void handle_copy_to_private( vrpn_int32 value, void * userdata );
extern void handle_copy_to_shared( vrpn_int32 value, void * userdata );
extern void handle_peer_sync_change( void * userdata, vrpn_bool value );


/// synchronization UI handlers
extern void handle_collab_red_measure_change( vrpn_float64 newValue,
                                              void * userdata );
extern void handle_collab_green_measure_change( vrpn_float64 newValue,
                                                void * userdata );
extern void handle_collab_blue_measure_change( vrpn_float64 newValue,
                                               void * userdata );
extern void handle_collab_measure_change( nmb_Dataset * data,
                                          int which_line );
extern void handle_collab_measure_move( float x, float y,
					void * userdata ); 



/// Collaboration. so we can track hand position of collaborator(s)
extern void handle_collab_machine_name_change( const char *new_value, 
					       void *userdata);

/** handle_collab_sensor2tracker_change is the callback for the position
    and orientation messages sent from the nM_coord_change_server (to
    track a collaborator's hand position) */
extern void handle_collab_sensor2tracker_change( void *userdata,
						 const vrpn_TRACKERCB info);

/** handle_collab_mode_change is the callback for the mode
    that track a collaborator's mode */
extern void handle_collab_mode_change( void *userdata,
				       const vrpn_ANALOGCB info);


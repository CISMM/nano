# Amy Henderson's shared pointers (Aug 99)
# Tom Hudson's streamfile synchronization (Sept 99)

set sharedptr(sp) [create_closing_toplevel sharedptr "Collaboration Tools"]
# We can show the window with show.sharedptr procedure.

# Choose a name for the machine to collaborate with
# Changed TCH 31 Oct 99 to a generic_entry so that if the name
# is set with command-line parameters it shows up in the tcl interface

# TCH 5 March 01 changed to a pull-down list with optional entry box
# to reduce typing errors.  Based on Aron's dialog box in filemenu.tcl.

set collaborationNames { \
  "Phillips 100" "Phillips 101" \
  "Chemistry" "Gene Therapy" \
  "Chemistry (nanoNet)" "Gene Therapy (nanoNet)" \
}
set collaborationConnections { \
  "nano.cs.unc.edu" "histidine.cs.unc.edu" \
  "chem-gfx.cs.unc.edu" "gt-gfx.cs.unc.edu" \
  "chem-gfx-nn.nanonet.unc.edu" "gt-gfx-nn.nanonet.unc.edu" \
}



iwidgets::dialog .choose_collaborator_dialog -title "Choose Collaborator" \
  -modality application
.choose_collaborator_dialog hide Help
.choose_collaborator_dialog buttonconfigure OK -text "Connect" -command {
   focus .choose_collaborator_dialog.shellchildsite.bbox
  .choose_collaborator_dialog deactivate 1
}
.choose_collaborator_dialog hide Apply

set win [.choose_collaborator_dialog childsite]
generic_optionmenu_with_index $win.site_name chosen_site_index \
  "Collaborator site:" collaborationNames

# force focus to be taken off the entry box so that collaborator is set
# from whatever has been typed in so far
set buttbox .choose_collaborator_dialog.shellchildsite.bbox 
bind $buttbox <Enter> "focus $buttbox"

pack $win.site_name -anchor nw -side top

set newSite ""
generic_entry $win.newSite newSite "New Site:" ""
set newSiteConnection ""
generic_entry $win.newSiteConnection newSiteConnection "Hostname:" ""

pack $win.newSite $win.newSiteConnection -side left

set collab_machine_name ""

proc open_collaboration_connection {} {
  global collab_machine_name
  global sharedptr

  global collaborationNames collaborationConnections
  global chosen_site_index
  global newSite newSiteConnection

  if { [.choose_collaborator_dialog activate] } {

    # OK

    if { ($newSite != "") && ($newSiteConnection != "") } {

      # Add this site to the list
      # Doesn't mean anything yet, since we can't reopen the
      # dialog.  We'd really like to save this list so that it's
      # persistent across sessions.

      set collaborationNames [ lappend collaborationNames $newSite ]
      set collaborationConnections \
          [ lappend collaborationConnections $newSiteConnection ]

      # specify new site
      set collab_machine_name $newSiteConnection

    } else {

      # use known site
      set collab_machine_name \
            [lindex $collaborationConnections $chosen_site_index]

    }

    # Not safe to change collaborator again, so disable it

    $sharedptr(sp).choose_collaborator configure -state disabled

  } else {

    # cancel

  }

  # These don't work, but will be important once we stop disabling
  # this dialog.
  # set newSite ""
  # set newSiteConnection ""

}

button $sharedptr(sp).choose_collaborator \
  -text "Choose Collaborator" -command open_collaboration_connection

pack $sharedptr(sp).choose_collaborator -side top -fill x

#set collab_machine_name ""
#generic_entry $sharedptr(sp).collab_machine_name \
              collab_machine_name \
              "Connect to machine:" \
              ""
#pack $sharedptr(sp).collab_machine_name -side top -fill x




# NANOX

# Load some images of arrows, pointing up and down, to use with
# the copy buttons. They are stored in the images sub-directory
# of the tcl directory.
image create photo sharedptr_down_arrow \
	-file [file join ${tcl_script_dir} images darrw.gif] \
	-format GIF 
image create photo sharedptr_up_arrow \
	-file [file join ${tcl_script_dir} images uarrw.gif] \
	-format GIF 


# Christmas 99 scheme -
#   each machine has a private state copy
#   all machines have a shared state copy
# "copy" button copies inactive state to active state
# "share" checkbox selects between private and shared states

frame $sharedptr(sp).state

set share_sync_state 0

frame $sharedptr(sp).state.copy
# NOTE text option is ignored if the image option is used. 
button $sharedptr(sp).state.copy.copy_to_shared_button \
	-image sharedptr_down_arrow \
        -state disabled \
  -text "Copy p to s" -command {set copy_to_shared_state 1}
label $sharedptr(sp).state.copy.copy_label -text "Copy"
button $sharedptr(sp).state.copy.copy_to_private_button \
	-image sharedptr_up_arrow \
        -state disabled \
  -text "Copy s to p" -command {set copy_to_private_state 1}

radiobutton $sharedptr(sp).state.share_private_button \
  -text "View private state" -variable share_sync_state -value 0
radiobutton $sharedptr(sp).state.share_public_button \
  -text "View shared state" -variable share_sync_state -value 1 \
        -state disabled \

pack $sharedptr(sp).state.share_private_button -side top
pack $sharedptr(sp).state.share_public_button -side bottom

# No parameters means this frame stays centered in the widget, like
# the share buttons above. 
pack $sharedptr(sp).state.copy
pack $sharedptr(sp).state.copy.copy_to_shared_button \
     $sharedptr(sp).state.copy.copy_label \
     $sharedptr(sp).state.copy.copy_to_private_button -side left -padx 5 

pack $sharedptr(sp).state

# T. Hudson 6 Mar 2001
# Enable buttons once we've got a good collaborative connection
proc collab_connection_good {} {
  global sharedptr

  $sharedptr(sp).state.copy.copy_to_shared_button configure -state normal
  $sharedptr(sp).state.copy.copy_to_private_button configure -state normal
  $sharedptr(sp).state.share_public_button configure -state normal

}

# Mutex
# T. Hudson 2 May 2000

set request_mutex 0
set release_mutex 0

frame $sharedptr(sp).mutex

button $sharedptr(sp).mutex.request_mutex_button \
  -text "Request Microscope Lock" -command {mutex_request_command}
button $sharedptr(sp).mutex.release_mutex_button \
  -text "Release Microscope Lock" -command {mutex_release_command}

pack $sharedptr(sp).mutex.request_mutex_button -side top
pack $sharedptr(sp).mutex.release_mutex_button -side bottom

pack $sharedptr(sp).mutex -side bottom

# Disable one button.
# Currently assumes we're coming up with the lock.
# Should instead probably just trigger this in the GotMutex callback
# from C++, so that if we solve the initial races in the distributed
# code we'll also be correct here.
# XXX

#$sharedptr(sp).mutex.request_mutex_button configure -state disabled
$sharedptr(sp).mutex.release_mutex_button configure -state disabled



# (Some of) the effects of request button have to come through C++
# because they can't take place until we get a GrantRequest/DenyRequest.
# We COULD disable request, and just not enable either button until we
# got one of the two callbacks.

#--------- implement release button ------------

# If the user hits "release":
#   have the C++ code release the mutex
#   enable request button
#   disable release button

proc mutex_release_command {} {
  global release_mutex
  global sharedptr

  set release_mutex 1
#  Don't disable it here;  let the callback do that.
#  $sharedptr(sp).mutex.release_mutex_button configure \
         #-state disabled

}

# If the user hits "request":
#   have the C++ code request the traffic
#   leave BOTH buttons disabled (while we're doing network traffic)

proc mutex_request_command {} {
  global request_mutex
  global sharedptr

#puts "We requested the mutex"

  set request_mutex 1
  $sharedptr(sp).mutex.request_mutex_button configure \
          -state disabled
  $sharedptr(sp).mutex.release_mutex_button configure \
          -state disabled
}

proc mutex_gotRequest_callback {} {
  global sharedptr
  global collab_commands_suspended
  global view

#puts "We got the mutex"

  $sharedptr(sp).mutex.request_mutex_button configure \
          -state disabled
  $sharedptr(sp).mutex.release_mutex_button configure \
          -state normal

  # trigger a trace function in mainwin.tcl
  set collab_commands_suspended 0

}

proc mutex_deniedRequest_callback {} {
  global sharedptr
  global collab_commands_suspended
  global view

#puts "We were denied the mutex"

  $sharedptr(sp).mutex.request_mutex_button configure \
          -state normal
  $sharedptr(sp).mutex.release_mutex_button configure \
          -state disabled

  # trigger a trace function in mainwin.tcl
  set collab_commands_suspended 1

}

proc mutex_taken_callback {} {
  global sharedptr
  global collab_commands_suspended
  global view

#puts "Somebody took the mutex"

  $sharedptr(sp).mutex.request_mutex_button configure \
          -state disabled

  # Don't set collab_commands_suspended, or disable anything else,
  # since we get this message when WE get the mutex
  # trigger a trace function in mainwin.tcl
  # set collab_commands_suspended 1
  #$sharedptr(sp).mutex.release_mutex_button configure \
          #-state disabled
}

proc mutex_release_callback {} {
  global sharedptr
  global collab_commands_suspended
  global view

#puts "Somebody released the mutex"

  $sharedptr(sp).mutex.request_mutex_button configure \
          -state normal
  $sharedptr(sp).mutex.release_mutex_button configure \
          -state disabled

  # trigger a trace function in mainwin.tcl
  set collab_commands_suspended 1

}


# Default state:  assume somebody else holds the mutex
#   The C++ code sends a request every time it connects to a
# new microscope to see if we can grab the mutex for this connection.

$sharedptr(sp).mutex.request_mutex_button configure \
          -state disabled


#----- hierarchical fine-grained coupling controls -----

frame $sharedptr(sp).view_frame
frame $sharedptr(sp).view_frame.sf

# .5 cm left padding
frame $sharedptr(sp).view_frame.sf.pad -width .5c
pack $sharedptr(sp).view_frame.sf.pad -side left

frame $sharedptr(sp).view_frame.sf.plane_frame
pack $sharedptr(sp).view_frame.sf.plane_frame -fill x -side top

checkbutton \
  $sharedptr(sp).view_frame.sf.plane_frame.synch_view_plane \
  -text "Plane"
pack $sharedptr(sp).view_frame.sf.plane_frame.synch_view_plane \
  -side left

frame $sharedptr(sp).view_frame.sf.color_frame
pack $sharedptr(sp).view_frame.sf.color_frame -fill x -side top

checkbutton $sharedptr(sp).view_frame.sf.color_frame.synch_view_color \
  -text "Color"
pack $sharedptr(sp).view_frame.sf.color_frame.synch_view_color -side left

frame $sharedptr(sp).view_frame.sf.measure_frame
pack $sharedptr(sp).view_frame.sf.measure_frame -fill x -side top

checkbutton $sharedptr(sp).view_frame.sf.measure_frame.synch_view_measure \
  -text "Measure Lines"
pack $sharedptr(sp).view_frame.sf.measure_frame.synch_view_measure -side left

frame $sharedptr(sp).view_frame.sf.lighting_frame
pack $sharedptr(sp).view_frame.sf.lighting_frame -fill x -side top

checkbutton $sharedptr(sp).view_frame.sf.lighting_frame.synch_view_lighting \
  -text "Lighting"
pack $sharedptr(sp).view_frame.sf.lighting_frame.synch_view_lighting -side left

# pack $sharedptr(sp).view_frame.sf.synchronize_view -side left

frame $sharedptr(sp).view_frame.sf.contour_frame
pack $sharedptr(sp).view_frame.sf.contour_frame -fill x -side top

checkbutton $sharedptr(sp).view_frame.sf.contour_frame.synch_view_contour \
  -text "Contour Lines"
pack $sharedptr(sp).view_frame.sf.contour_frame.synch_view_contour -side left


frame $sharedptr(sp).view_frame.sf.grid_frame
pack $sharedptr(sp).view_frame.sf.grid_frame -fill x -side top

checkbutton $sharedptr(sp).view_frame.sf.grid_frame.synch_view_grid \
  -text "Rulergrid"
pack $sharedptr(sp).view_frame.sf.grid_frame.synch_view_grid -side left

pack $sharedptr(sp).view_frame.sf






#----- hierarchical fine-grained coupling drivers -----


proc pack_finegrained_coupling {} {
  global sharedptr

  pack $sharedptr(sp).view_frame -side bottom
}

proc unpack_finegrained_coupling {} {
  global sharedptr

  pack forget $sharedptr(sp).view_frame
}




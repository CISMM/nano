# Amy Henderson's shared pointers (Aug 99)
# Tom Hudson's streamfile synchronization (Sept 99)

set sharedptr(sp) [create_closing_toplevel sharedptr "Collaboration Tools"]
# We can show the window with show.sharedptr procedure.

# Choose a name for the machine to collaborate with
# Changed TCH 31 Oct 99 to a generic_entry so that if the name
# is set with command-line parameters it shows up in the tcl interface

# iwidgets::entryfield $sharedptr(sp).collab_machine_name \
# 	-labeltext "Machine to connect to:" \
# 	-command { set collab_machine_name \
# 		[$sharedptr(sp).collab_machine_name get]}
#set collab_machine_name ""
generic_entry $sharedptr(sp).collab_machine_name \
              collab_machine_name \
              "Connect to machine:" \
              ""
pack $sharedptr(sp).collab_machine_name -side top -fill x



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

set copy_inactive_state 0
set share_sync_state 0

frame $sharedptr(sp).state.copy
# NOTE text option is ignored if the image option is used. 
button $sharedptr(sp).state.copy.copy_to_shared_button \
	-image sharedptr_down_arrow \
  -text "Copy p to s" -command {set copy_to_shared_state 1}
label $sharedptr(sp).state.copy.copy_label -text "Copy"
button $sharedptr(sp).state.copy.copy_to_private_button \
	-image sharedptr_up_arrow \
  -text "Copy s to p" -command {set copy_to_private_state 1}

radiobutton $sharedptr(sp).state.share_private_button \
  -text "View private state" -variable share_sync_state -value 0
radiobutton $sharedptr(sp).state.share_public_button \
  -text "View shared state" -variable share_sync_state -value 1

pack $sharedptr(sp).state.share_private_button -side top
pack $sharedptr(sp).state.share_public_button -side bottom

# No parameters means this frame stays centered in the widget, like
# the share buttons above. 
pack $sharedptr(sp).state.copy
pack $sharedptr(sp).state.copy.copy_to_shared_button \
     $sharedptr(sp).state.copy.copy_label \
     $sharedptr(sp).state.copy.copy_to_private_button -side left -padx 5 

pack $sharedptr(sp).state

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
$sharedptr(sp).mutex.request_mutex_button configure -state disabled



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
  $sharedptr(sp).mutex.release_mutex_button configure \
          -state disabled

}

# If the user hits "request":
#   have the C++ code request the traffic
#   leave BOTH buttons disabled (while we're doing network traffic)

proc mutex_request_command {} {
  global request_mutex
  global sharedptr

  set request_mutex 1
  $sharedptr(sp).mutex.request_mutex_button configure \
          -state disabled
}

proc mutex_gotRequest_callback {} {
  global sharedptr

  $sharedptr(sp).mutex.release_mutex_button configure \
          -state normal
}

proc mutex_deniedRequest_callback {} {
  global sharedptr

  $sharedptr(sp).mutex.request_mutex_button configure \
          -state normal
}

proc mutex_taken_callback {} {
  global sharedptr

  $sharedptr(sp).mutex.request_mutex_button configure \
          -state disabled
}

proc mutex_release_callback {} {
  global sharedptr

  $sharedptr(sp).mutex.request_mutex_button configure \
          -state normal
}

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

set copy_inactive_state 0
set share_sync_state 0

#button $sharedptr(sp).copy_button \
#  -text "Copy from inactive" -command {set copy_inactive_state 1}

frame $sharedptr(sp).copy
# NOTE text option is ignored if the image option is used. 
button $sharedptr(sp).copy.copy_to_shared_button \
	-image sharedptr_down_arrow \
  -text "Copy p to s" -command {set copy_to_shared_state 1}
label $sharedptr(sp).copy.copy_label -text "Copy"
button $sharedptr(sp).copy.copy_to_private_button \
	-image sharedptr_up_arrow \
  -text "Copy s to p" -command {set copy_to_private_state 1}

radiobutton $sharedptr(sp).share_private_button \
  -text "View private state" -variable share_sync_state -value 0
radiobutton $sharedptr(sp).share_public_button \
  -text "View shared state" -variable share_sync_state -value 1

pack $sharedptr(sp).share_private_button -side top
pack $sharedptr(sp).share_public_button -side bottom

#pack $sharedptr(sp).copy_button -side left

# No parameters means this frame stays centered in the widget, like
# the share buttons above. 
pack $sharedptr(sp).copy
pack $sharedptr(sp).copy.copy_to_shared_button $sharedptr(sp).copy.copy_label \
	$sharedptr(sp).copy.copy_to_private_button -side left -padx 5 


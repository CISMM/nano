# Amy Henderson's shared pointers (Aug 99)
# Tom Hudson's streamfile synchronization (Sept 99)

set sharedptr(sp) .sharedptr

toplevel $sharedptr(sp)
wm withdraw $sharedptr(sp)

button $sharedptr(sp).close -text "Close" -command {
	wm withdraw $sharedptr(sp)
}

wm protocol $sharedptr(sp) WM_DELETE_WINDOW {
	$sharedptr(sp).close invoke
}
pack $sharedptr(sp).close -anchor nw

proc show_shared_ptr_win {} {
	global sharedptr
	wm deiconify $sharedptr(sp)
	raise $sharedptr(sp)
}

# Choose a name for the machine to collaborate with
# Changed TCH 31 Oct 99 to a generic_entry so that if the name
# is set with command-line parameters it shows up in the tcl interface

# iwidgets::entryfield $sharedptr(sp).collab_machine_name \
# 	-labeltext "Machine to connect to:" \
# 	-command { set collab_machine_name \
# 		[$sharedptr(sp).collab_machine_name get]}
set collab_machine_name ""
generic_entry $sharedptr(sp).collab_machine_name \
              collab_machine_name \
              "Machine to connect to:" \
              ""
pack $sharedptr(sp).collab_machine_name -side top



# NANOX

# Christmas 99 scheme -
#   each machine has a private state copy
#   all machines have a shared state copy
# "copy" button copies inactive state to active state
# "share" checkbox selects between private and shared states

set copy_inactive_state 0
set share_sync_state 0

button $sharedptr(sp).copy_button \
  -text "Copy" -command {set copy_inactive_state 1}

checkbutton $sharedptr(sp).share_button \
  -text "Share" -variable share_sync_state

pack $sharedptr(sp).share_button -side top
pack $sharedptr(sp).copy_button -side top



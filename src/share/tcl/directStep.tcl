#!/bin/sh
# exec wish "$0" "$@"

# generic_entry .base -width 6 -relief sunken -



set new_step_x_size 1.0
set new_step_y_size 1.0
set new_step_z_size 1.0
set go_to_pos 1

set takestep(ds) [create_closing_toplevel directStep "Direct Step"]




label $takestep(ds).label -text "Direct Step Controls"
pack $takestep(ds).label -side top -anchor nw -padx 5 -pady 5

set takestep(buttons) [frame $takestep(ds).buttons]
pack $takestep(buttons) -side top -fill x -pady 5

button $takestep(buttons).minus_x -text "-X"  \
	-command {minus_x} -padx 8

button $takestep(buttons).plus_x -text " +X" \
	 -command {plus_x} -padx 8

generic_entry $takestep(ds).step_x_size new_step_x_size \
	"Step X Size" real

pack $takestep(buttons).minus_x \
	$takestep(buttons).plus_x \
	$takestep(ds).step_x_size -side left


button $takestep(buttons).minus_y -text "-Y"  \
	-command {minus_y} -padx 8

button $takestep(buttons).plus_y -text " +Y" \
	 -command {plus_y} -padx 8

generic_entry $takestep(ds).step_y_size new_step_y_size \
	"Step Y Size" real

pack $takestep(buttons).minus_y \
	$takestep(buttons).plus_y \
	$takestep(ds).step_y_size -side left

button $takestep(buttons).minus_z -text "-Z"  \
	-command {minus_z} -padx 8

button $takestep(buttons).plus_z -text " +Z" \
	 -command {plus_z} -padx 8

generic_entry $takestep(ds).step_z_size new_step_z_size \
	"Step Z Size" real

pack $takestep(buttons).minus_z \
	$takestep(buttons).plus_z \
	$takestep(ds).step_z_size -side left

# position inputs
generic_entry $takestep(ds).x_pos step_x_pos \
	"X Position" real

generic_entry $takestep(ds).y_pos step_y_pos \
	"Y Pos" real

generic_entry $takestep(ds).z_pos step_z_pos \
	"Z Pos" real

button $takestep(buttons).go_to_pos -text "Go To Position" \
	 -command {go_to_pos} -padx 8

pack $takestep(ds).x_pos \
	$takestep(ds).y_pos \
	$takestep(ds).z_pos -side left

pack  $takestep(buttons).go_to_pos -side right



#X coord functions
proc plus_x {} {
	global take_x_step
	global new_step_x_size
	set take_x_step $new_step_x_size
 }

proc minus_x {} {
global take_x_step
global new_step_x_size
set take_x_step -$new_step_x_size
}


#y coord functions
proc plus_y {} {
	global take_y_step
	global new_step_y_size
	set take_y_step $new_step_y_size
 }

proc minus_y {} {
global take_y_step
global new_step_y_size
set take_y_step -$new_step_y_size
}

#Z coord functions
proc plus_z {} {
	global take_z_step
	global new_step_z_size
	set take_z_step $new_step_z_size
 }

proc minus_z {} {
global take_z_step
global new_step_z_size
set take_z_step -$new_step_z_size
}

proc go_to_pos {} {
global go_to_pos
global step_go_to_pos
set step_go_to_pos $go_to_pos
}

global newmodifyp_control

 trace variable newmodifyp_control w show_direct_step_controls

proc show_direct_step_controls {nm el op} {
	global newmodifyp_control newmodifyp_tool takestep
	if { $newmodifyp_control == 0} {
	pack forget $takestep(ds).z_pos
	pack forget $takestep(buttons).minus_z 
	pack forget $takestep(buttons).plus_z
	

	} else { 
	pack $takestep(ds).z_pos
	pack $takestep(buttons).minus_z -side left
	pack $takestep(buttons).plus_z -side left
       }
}
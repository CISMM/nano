#!/bin/sh
# exec wish "$0" "$@"

# generic_entry .base -width 6 -relief sunken -



set new_step_x_size 1.0
set new_step_y_size 1.0
set new_step_z_size 1.0
set cur_x 0.0
set cur_y 0.0
set cur_z 0.0
set go_to_pos 1

set takestep(ds) [create_closing_toplevel directStep "Direct Step"]

image create photo dstep_down_arrow \
	-file [file join ${tcl_script_dir} images darrw.gif] \
	-format GIF 

image create photo dstep_up_arrow \
	-file [file join ${tcl_script_dir} images uarrw.gif] \
	-format GIF 

image create photo dstep_left_arrow \
	-file [file join ${tcl_script_dir} images larrw.gif] \
	-format GIF

image create photo dstep_right_arrow \
	-file [file join ${tcl_script_dir} images rarrw.gif] \
	-format GIF 


label $takestep(ds).label -text "Direct Step Controls"
pack $takestep(ds).label -side top -anchor nw -padx 5 -pady 5

iwidgets::Labeledframe $takestep(ds).stepping -labeltext "step X/Y" \
	-labelpos nw
set takestep(stepping) [$takestep(ds).stepping childsite]
pack $takestep(ds).stepping -side left -fill y -expand yes -anchor nw

iwidgets::Labeledframe $takestep(ds).z_stepping -labeltext "Step Z" \
	-labelpos nw
set takestep(z_stepping) [$takestep(ds).z_stepping childsite]
pack $takestep(ds).z_stepping -side left -fill both -expand yes -anchor ne


iwidgets::Labeledframe $takestep(ds).step_size_frame -labeltext "Step Size" \
	-labelpos nw
set takestep(step_size_frame) [$takestep(ds).step_size_frame childsite]
pack $takestep(ds).step_size_frame -side left -fill both -expand yes -anchor ne


iwidgets::Labeledframe $takestep(ds).go_to_pos_frame -labeltext "set position" \
	-labelpos nw
set takestep(go_to_pos_frame) [$takestep(ds).go_to_pos_frame childsite]
pack $takestep(ds).z_stepping -side left -fill both -expand yes -anchor ne


iwidgets::Labeledframe $takestep(ds).current_pos -labeltext "Current position" \
	-labelpos nw
set takestep(current_pos) [$takestep(ds).current_pos childsite]
pack $takestep(ds).current_pos -side left -fill both -expand yes -anchor ne


set takestep(buttons) [frame $takestep(stepping).buttons]
pack $takestep(buttons) -side top -fill x -pady 5

set takestep(buttons2) [frame $takestep(stepping).buttons2]
pack $takestep(buttons2) -side top -fill x -pady 5

set takestep(buttons3) [frame $takestep(stepping).buttons3]
pack $takestep(buttons3) -side top -fill x -pady 5

set takestep(buttons_z) [frame $takestep(z_stepping).buttons_z]
pack $takestep(buttons_z) -side top -fill x -pady 5

set takestep(buttons_z2) [frame $takestep(z_stepping).buttons_z2]
pack $takestep(buttons_z2) -side top -fill x -pady 5

set takestep(buttons_z3) [frame $takestep(z_stepping).buttons_z3]
pack $takestep(buttons_z3) -side top -fill x -pady 5

set takestep(stepSize) [frame $takestep(step_size_frame).stepSize]
pack $takestep(stepSize) -side top -fill x -pady 5

set takestep(pos) [frame $takestep(go_to_pos_frame).pos]
pack $takestep(pos) -side top -fill x -pady 5 

set takestep(goto) [frame $takestep(ds).goto]
pack $takestep(goto) -side top -fill x -pady 5


# buttons
button $takestep(buttons2).minus_x -image dstep_left_arrow \
	-command {minus_x} -padx 8

button $takestep(buttons2).plus_x -image dstep_right_arrow \
	 -command {plus_x} -padx 8

button $takestep(buttons3).minus_y -image dstep_down_arrow  \
	-command {minus_y} -padx 8

button $takestep(buttons).plus_y -image dstep_up_arrow \
	 -command {plus_y} -padx 8

button $takestep(buttons_z3).minus_z -image dstep_down_arrow \
	-command {minus_z} -padx 8

button $takestep(buttons_z).plus_z -image dstep_up_arrow \
	 -command {plus_z} -padx 8
button $takestep(goto).go_to_pos -text "Go To Position" \
	 -command {go_to_pos} -padx 8


generic_entry $takestep(stepSize).step_x_size new_step_x_size \
	"X Step     " real

generic_entry $takestep(stepSize).step_y_size new_step_y_size \
	" Y Step " real

generic_entry $takestep(stepSize).step_z_size new_step_z_size \
	"            Z Step" real


# position inputs
generic_entry $takestep(pos).x_pos step_x_pos \
	"X Position" real

generic_entry $takestep(pos).y_pos step_y_pos \
	"Y Pos  " real

generic_entry $takestep(pos).z_pos step_z_pos \
	"             Z Pos" real

#current position
label $takestep(current_pos).cur_x -text "  Current x: "
label $takestep(current_pos).cur_y -text "  Current y: "
label $takestep(current_pos).cur_z -text "  Current z: "


label $takestep(current_pos).value_cur_x -textvariable cur_x
label $takestep(current_pos).value_cur_y -textvariable cur_y
label $takestep(current_pos).value_cur_z -textvariable cur_z

#spacer frames
frame $takestep(buttons).space -width 1c -height 1c -relief flat
frame $takestep(buttons2).space -width 1c -height 1c -relief flat
frame $takestep(buttons3).space -width 1c -height 1c -relief flat
frame $takestep(buttons_z2).space -width 2c -height 1c -relief flat



#pack frames
pack $takestep(buttons).space \
	$takestep(buttons).plus_y -side left

pack  $takestep(buttons2).minus_x \
	$takestep(buttons2).space \
	$takestep(buttons2).plus_x -side left

pack $takestep(buttons3).space \
	$takestep(buttons3).minus_y -side left

pack $takestep(stepSize).step_x_size \
     $takestep(stepSize).step_y_size -side left

pack $takestep(pos).x_pos \
	$takestep(pos).y_pos \
	$takestep(pos).z_pos -side left


pack $takestep(stepSize).step_z_size -side left


pack  $takestep(goto).go_to_pos -side left

pack $takestep(current_pos).cur_x $takestep(current_pos).value_cur_x -side left
pack $takestep(current_pos).cur_y $takestep(current_pos).value_cur_y -side left


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
	pack forget $takestep(pos).z_pos
	pack forget $takestep(ds).z_stepping
	pack forget $takestep(stepSize).step_z_size
	pack forget $takestep(current_pos).cur_z
	pack forget $takestep(current_pos).value_cur_z
    } else {

	pack forget $takestep(ds).step_size_frame
	pack forget $takestep(ds).current_pos
	pack forget $takestep(ds).goto
	pack forget $takestep(ds).go_to_pos_frame

	pack $takestep(ds).z_stepping -side left -anchor nw
	pack $takestep(ds).step_size_frame -anchor nw
	pack $takestep(ds).go_to_pos_frame -anchor nw
	pack $takestep(ds).current_pos -anchor nw
	pack $takestep(ds).goto	-anchor nw

	pack $takestep(stepSize).step_z_size -side left
	pack $takestep(pos).z_pos -side left
	pack $takestep(buttons_z).plus_z 
	pack $takestep(buttons_z2).space
	pack $takestep(buttons_z3).minus_z -side top
	pack $takestep(current_pos).cur_z $takestep(current_pos).value_cur_z -side left
    }
}
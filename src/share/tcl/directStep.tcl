############################################################################
# Controls for direct Step
# Jameson Miller
# 
# this file contains the controls for the direct step window
# there are 2 very similair setups for this window
# based on if we are using a regular axis or a user defined axis.
# the user defind axis is denoted by the _c suffix at the end of 
# variables.
#############################################################################

global device_only_controls

set step_x_size 1.0
set step_y_size 1.0
set step_z_size 1.0
set cur_x 0.0
set cur_y 0.0
set cur_z 0.0
set go_to_pos 1
set keep_stepping 0

# ---------------------------------------------------------------
# 
# regular arrows to take steps along normal axis
# 
# ---------------------------------------------------------------

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

#--------------------------------------------------------------
#
# colored arrows for taking steps along a user defined
# axis
#---------------------------------------------------------------

image create photo green_down_arrow \
	-file [file join ${tcl_script_dir} images green_darrw.gif] \
	-format GIF 

image create photo green_up_arrow \
	-file [file join ${tcl_script_dir} images green_uarrw.gif] \
	-format GIF 

image create photo blue_down_arrow \
	-file [file join ${tcl_script_dir} images blue_darrw.gif] \
	-format GIF 

image create photo blue_up_arrow \
	-file [file join ${tcl_script_dir} images blue_uarrw.gif] \
	-format GIF 

image create photo red_right_arrow \
	-file [file join ${tcl_script_dir} images red_rarrw.gif] \
	-format GIF 

image create photo red_left_arrow \
	-file [file join ${tcl_script_dir} images red_larrw.gif] \
	-format GIF 
#--------------------------------------------
# frames, regular stepping axis
# -------------------------------------------

label $takestep(ds).label -text "Direct Step Controls"
pack $takestep(ds).label -side top -anchor nw -padx 5 -pady 5

iwidgets::Labeledframe $takestep(ds).stepping -labeltext "step X/Y" \
	-labelpos nw
set takestep(stepping) [$takestep(ds).stepping childsite]
# pack $takestep(ds).stepping -side left -fill y -expand yes -anchor nw

iwidgets::Labeledframe $takestep(ds).z_stepping -labeltext "Step Z" \
	-labelpos nw
set takestep(z_stepping) [$takestep(ds).z_stepping childsite]
# pack $takestep(ds).z_stepping -side left -fill both -expand yes -anchor ne


iwidgets::Labeledframe $takestep(ds).step_size_frame -labeltext "Step Size" \
	-labelpos nw
set takestep(step_size_frame) [$takestep(ds).step_size_frame childsite]
# pack $takestep(ds).step_size_frame -side left -fill both -expand yes -anchor ne


iwidgets::Labeledframe $takestep(ds).go_to_pos_frame -labeltext "set position" \
	-labelpos nw
set takestep(go_to_pos_frame) [$takestep(ds).go_to_pos_frame childsite]
# pack $takestep(ds).z_stepping -side left -fill both -expand yes -anchor ne


iwidgets::Labeledframe $takestep(ds).current_pos -labeltext "Current position" \
	-labelpos nw
set takestep(current_pos) [$takestep(ds).current_pos childsite]
# pack $takestep(ds).current_pos -side left -fill both -expand yes -anchor ne

#-----------------------------------------------------------------------------
# frames, colored stepping axis
# ----------------------------------------------------------------------------
iwidgets::Labeledframe $takestep(ds).color_axis
set takestep(color_axis) [$takestep(ds).color_axis childsite]

iwidgets::Labeledframe $takestep(color_axis).stepping_c -labeltext "step red/green" \
	-labelpos nw
set takestep(stepping_c) [$takestep(color_axis).stepping_c childsite]
pack $takestep(color_axis).stepping_c -side left -fill y -expand yes -anchor nw

iwidgets::Labeledframe $takestep(color_axis).z_stepping_c -labeltext "blue" \
	-labelpos nw
set takestep(z_stepping_c) [$takestep(color_axis).z_stepping_c childsite]
#pack $takestep(ds).z_stepping_c -side left -fill both -expand yes -anchor ne



iwidgets::Labeledframe $takestep(color_axis).step_size_frame_c -labeltext "Step Size" \
	-labelpos nw
set takestep(step_size_frame_c) [$takestep(color_axis).step_size_frame_c childsite]
pack $takestep(color_axis).step_size_frame_c -side left -fill both -expand yes -anchor ne


iwidgets::Labeledframe $takestep(color_axis).go_to_pos_frame_c -labeltext "set position" \
	-labelpos nw
set takestep(go_to_pos_frame_c) [$takestep(color_axis).go_to_pos_frame_c childsite]
pack $takestep(color_axis).go_to_pos_frame_c -side left -fill both -expand yes -anchor ne


iwidgets::Labeledframe $takestep(color_axis).current_pos_c -labeltext "Current position" \
	-labelpos nw
set takestep(current_pos_c) [$takestep(color_axis).current_pos_c childsite]



iwidgets::Labeledframe $takestep(ds).axis_controls -labeltext "axis control" \
    -labelpos nw
set takestep(axis_controls) [$takestep(ds).axis_controls childsite]
# pack $takestep(ds).axis_controls -side bottom -fill both -expand yes -anchor ne


#----------------------------------------------------------------------
# set frames within stepping frame (which will contain the arrow buttons
# to get the setup to come out right.
# maybe there is a better way to get the window to look right
# --------------------------------------------------------------------

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

#--------------------------------------------------------------------
# same as above, but for colored axis frame
# ------------------------------------------------------------
set takestep(buttons_c) [frame $takestep(stepping_c).buttons_c]
pack $takestep(buttons_c) -side top -fill x -pady 5

set takestep(buttons2_c) [frame $takestep(stepping_c).buttons2_c]
pack $takestep(buttons2_c) -side top -fill x -pady 5

set takestep(buttons3_c) [frame $takestep(stepping_c).buttons3_C]
pack $takestep(buttons3_c) -side top -fill x -pady 5

set takestep(buttons_z_c) [frame $takestep(z_stepping_c).buttons_z_c]
pack $takestep(buttons_z_c) -side top -fill x -pady 5

set takestep(buttons_z2_c) [frame $takestep(z_stepping_c).buttons_z2_c]
pack $takestep(buttons_z2_c) -side top -fill x -pady 5

set takestep(buttons_z3_c) [frame $takestep(z_stepping_c).buttons_z3_C]
pack $takestep(buttons_z3_c) -side top -fill x -pady 5

set takestep(stepSize_c) [frame $takestep(step_size_frame_c).stepSize_c]
pack $takestep(stepSize_c) -side top -fill x -pady 5

set takestep(pos_c) [frame $takestep(go_to_pos_frame_c).pos]
pack $takestep(pos_c) -side top -fill x -pady 5 

set takestep(goto_c) [frame $takestep(color_axis).goto_c]
pack $takestep(goto_c) -side top -fill x -pady 5

# --------------------------------------------------------------------
# set up buttons with correct arrow image and correct function to call
# ---------------------------------------------------------------------

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

#-------------------------------------------------------------------------
# step buttons for use with user defined axis
# --------------------------------------------------------------------------
button $takestep(buttons2_c).minus_x_c -image red_left_arrow \
	-command {minus_x} -padx 8

button $takestep(buttons2_c).plus_x_c -image red_right_arrow \
	 -command {plus_x} -padx 8

button $takestep(buttons3_c).minus_y_c -image green_down_arrow  \
	-command {minus_y} -padx 8

button $takestep(buttons_c).plus_y_c -image green_up_arrow \
	 -command {plus_y} -padx 8

button $takestep(buttons_z3_c).minus_z_c -image blue_down_arrow \
	-command {minus_z} -padx 8

button $takestep(buttons_z_c).plus_z_c -image blue_up_arrow \
	 -command {plus_z} -padx 8

#--------------------------------------------------------------------
# buttons that are the same in both states.
# ---------------------------------------------------------------------

button $takestep(goto).go_to_pos -text "Go To Position" \
	 -command {go_to_pos} -padx 8

checkbutton $takestep(goto).test -text "axis" \
         -variable setting_direct_step_axis

checkbutton $takestep(goto).keep_stepping_check -text "keep stepping" \
    -variable keep_stepping

generic_entry $takestep(stepSize).step_x_size step_x_size \
	"X Step     " real

generic_entry $takestep(stepSize).step_y_size step_y_size \
	" Y Step " real

generic_entry $takestep(stepSize).step_z_size step_z_size \
	"            Z Step" real

# position inputs
generic_entry $takestep(pos).x_pos step_x_pos \
	"X Position" real

generic_entry $takestep(pos).y_pos step_y_pos \
	"Y Pos  " real

generic_entry $takestep(pos).z_pos step_z_pos \
	"             Z Pos" real

# ------------------------------------------------------------
#  
# buttons that go in the color_axis frame
#
# ------------------------------------------------


button $takestep(goto_c).go_to_pos_c -text "Go To Position" \
	 -command {go_to_pos} -padx 8

checkbutton $takestep(goto_c).test_c -text "axis" \
         -variable setting_direct_step_axis

checkbutton $takestep(goto_c).keep_stepping_check_c -text "keep stepping" \
    -variable keep_stepping

generic_entry $takestep(stepSize_c).step_x_size_c step_x_size \
	"Red Step     " real

generic_entry $takestep(stepSize_c).step_y_size_c step_y_size \
	"  Green Step " real

generic_entry $takestep(stepSize_c).step_z_size_c step_z_size \
	"     Blue Step" real

# position inputs
generic_entry $takestep(pos_c).x_pos_c step_x_pos \
	"X Position" real

generic_entry $takestep(pos_c).y_pos_c step_y_pos \
	"     Y Pos  " real

generic_entry $takestep(pos_c).z_pos_c step_z_pos \
	"             Z Pos" real

# put these controls in the device only list.
# so you can only use them if you have control of the device.

eval lappend device_only_controls "$takestep(buttons2).minus_x \
    $takestep(buttons2).plus_x \
    $takestep(buttons3).minus_y \
    $takestep(buttons).plus_y \
    $takestep(buttons_z3).minus_z \
    $takestep(buttons_z).plus_z \
    $takestep(stepSize).step_x_size \
    $takestep(stepSize).step_y_size \
    $takestep(stepSize).step_z_size \
    $takestep(pos).x_pos \
    $takestep(pos).y_pos \
    $takestep(pos).z_pos \
    $takestep(buttons2_c).minus_x_c \
    $takestep(buttons2_c).plus_x_c \
    $takestep(buttons3_c).minus_y_c \
    $takestep(buttons_c).plus_y_c \
    $takestep(buttons_z3_c).minus_z_c \
    $takestep(buttons_z_c).plus_z_c "

#current position
label $takestep(current_pos).cur_x -text "  Current x: "
label $takestep(current_pos).cur_y -text "  Current y: "
label $takestep(current_pos).cur_z -text "  Current z: "


label $takestep(current_pos).value_cur_x -textvariable cur_x
label $takestep(current_pos).value_cur_y -textvariable cur_y
label $takestep(current_pos).value_cur_z -textvariable cur_z

#current position
label $takestep(current_pos_c).cur_x_c -text "  Current x: "
label $takestep(current_pos_c).cur_y_c -text "  Current y: "
label $takestep(current_pos_c).cur_z_c -text "  Current z: "


label $takestep(current_pos_c).value_cur_x_c -textvariable cur_x
label $takestep(current_pos_c).value_cur_y_c -textvariable cur_y
label $takestep(current_pos_c).value_cur_z_c -textvariable cur_z





# --------------------------------------------------------------
# these are blank frames that are the same size as the arrow buttons
# that are see-through, just to get the position of the arrow buttons 
# correct.
# -------------------------------------------------------------
#spacer frames
frame $takestep(buttons).space -width 1c -height 1c -relief flat
frame $takestep(buttons2).space -width 1c -height 1c -relief flat
frame $takestep(buttons3).space -width 1c -height 1c -relief flat
frame $takestep(buttons_z2).space -width 2c -height 1c -relief flat

#colored spacer frames
frame $takestep(buttons_c).space_c -width 1c -height 1c -relief flat
frame $takestep(buttons2_c).space_c -width 1c -height 1c -relief flat
frame $takestep(buttons3_c).space_c -width 1c -height 1c -relief flat
frame $takestep(buttons_z2_c).space_c -width 2c -height 1c -relief flat



# ----------------------------------------------------
# pack the arrow frames in an organized manner
# regular axis button set up
# ---------------------------------------------------

pack $takestep(buttons).space \
	$takestep(buttons).plus_y -side left

pack  $takestep(buttons2).minus_x \
	$takestep(buttons2).space \
	$takestep(buttons2).plus_x -side left

pack $takestep(buttons3).space \
	$takestep(buttons3).minus_y -side left

pack $takestep(buttons_z).plus_z
pack $takestep(buttons_z2).space
pack $takestep(buttons_z3).minus_z

pack $takestep(stepSize).step_x_size \
     $takestep(stepSize).step_y_size -side left

pack $takestep(pos).x_pos \
	$takestep(pos).y_pos \
	$takestep(pos).z_pos -side left


pack $takestep(stepSize).step_z_size -side left

#---------------------------------------------------------
# colored axis button set up
# -----------------------------------------------------

pack $takestep(buttons_c).space_c \
	$takestep(buttons_c).plus_y_c -side left

pack  $takestep(buttons2_c).minus_x_c \
	$takestep(buttons2_c).space_c \
	$takestep(buttons2_c).plus_x_c -side left

pack $takestep(buttons3_c).space_c \
	$takestep(buttons3_c).minus_y_c -side left

pack $takestep(buttons_z_c).plus_z_c
pack $takestep(buttons_z2_c).space_c
pack $takestep(buttons_z3_c).minus_z_c

pack $takestep(stepSize_c).step_x_size_c \
     $takestep(stepSize_c).step_y_size_c -side left

pack $takestep(pos_c).x_pos_c \
	$takestep(pos_c).y_pos_c \
	$takestep(pos_c).z_pos_c -side left


pack $takestep(stepSize).step_z_size -side left

#--------------------------------------------------------------
# set up frame with controls to move axis through interface
# ------------------------------------------------------------

# global variables so that we have access to the import 
# object variables

global import_transx import_transy import_transz
global import_lock_transx import_lock_transy import_lock_transz
global import_rotx_slide import_roty_slide import_rotz_slide
global import_lock_rotx_button import_lock_roty_button import_lock_rotz_button
global import_tune_rot import_tune_trans import_grab_object
global import_scale
set import_scale 300

# frames to organize translation and rotation controls
# number at end of frame name indicates which row frame is in
set takestep(axis_controls_1) [frame $takestep(axis_controls).axis_controls_1 ]
set takestep(axis_controls_2) [frame $takestep(axis_controls).axis_controls_2 ]
set takestep(axis_controls_3) [frame $takestep(axis_controls).axis_controls_3 ]
set takestep(axis_controls_4) [frame $takestep(axis_controls).axis_controls_4 ]
set takestep(axis_controls_5) [frame $takestep(axis_controls).axis_controls_5 ]

pack $takestep(axis_controls_1) -anchor nw
pack $takestep(axis_controls_2) -anchor nw
pack $takestep(axis_controls_3) -anchor nw
pack $takestep(axis_controls_4) -anchor nw
pack $takestep(axis_controls_5) -anchor nw

# level 1
# translation sliders

floatscale $takestep(axis_controls_1).import_transx_slide -1000.0 6000.0 100 1 1 \
	import_transx "X Translation"
pack $takestep(axis_controls_1).import_transx_slide -side left

floatscale $takestep(axis_controls_1).import_transy_slide -1000.0 6000.0 100 1 1 \
	import_transy "Y Translation"
pack $takestep(axis_controls_1).import_transy_slide -side left

floatscale $takestep(axis_controls_1).import_transz_slide -1000.0 6000.0 100 1 1 \
	import_transz "Z Translation"
pack $takestep(axis_controls_1).import_transz_slide -side left

# level 2 buttons
# locking the translation

checkbutton $takestep(axis_controls_2).import_lock_transx_button \
    -text "Lock X Translation" -variable import_lock_transx
pack $takestep(axis_controls_2).import_lock_transx_button -side left

checkbutton $takestep(axis_controls_2).import_lock_transy_button \
    -text "Lock Y Translation" -variable import_lock_transy
pack $takestep(axis_controls_2).import_lock_transy_button -side left

checkbutton $takestep(axis_controls_2).import_lock_transz_button \
    -text "Lock Z Translation" -variable import_lock_transz
pack $takestep(axis_controls_2).import_lock_transz_button -side left

checkbutton $takestep(axis_controls_2).import_tune_trans_button \
    -text "Fine Tune Translations" -variable import_tune_trans
pack $takestep(axis_controls_2).import_tune_trans_button -side left

# level 3
# rotation sliders

floatscale $takestep(axis_controls_3).import_rotx_slide -360 360 1000 1 1 \
	import_rotx "X Rotation"
pack $takestep(axis_controls_3).import_rotx_slide -side left

floatscale $takestep(axis_controls_3).import_roty_slide -360 360 1000 1 1 \
	import_roty "Y Rotation"
pack $takestep(axis_controls_3).import_roty_slide -side left

floatscale $takestep(axis_controls_3).import_rotz_slide -360 360 1000 1 1 \
	import_rotz "Z Rotation"
pack $takestep(axis_controls_3).import_rotz_slide -side left

# level 4
# rotation locks
checkbutton $takestep(axis_controls_4).import_lock_rotx_button \
    -text "Lock X Rotation" -variable import_lock_rotx
pack $takestep(axis_controls_4).import_lock_rotx_button -side left

checkbutton $takestep(axis_controls_4).import_lock_roty_button \
    -text "Lock Y Rotation" -variable import_lock_roty
pack $takestep(axis_controls_4).import_lock_roty_button -side left

checkbutton $takestep(axis_controls_4).import_lock_rotz_button \
    -text "Lock Z Rotation" -variable import_lock_rotz
pack $takestep(axis_controls_4).import_lock_rotz_button -side left

checkbutton $takestep(axis_controls_4).import_tune_rot_button \
    -text "Fine Tune Rotations" -variable import_tune_rot
pack $takestep(axis_controls_4).import_tune_rot_button -side left

#level 5
# grabing the object and other useful functions

# this should also put us into grab mode. 
checkbutton $takestep(axis_controls_5).ds_grab_object \
    -text "Grab axis" -variable import_grab_object -command set_ds_grab_object
pack $takestep(axis_controls_5).ds_grab_object -side left

checkbutton $takestep(axis_controls_5).ds_sphere_axis \
	-text "sphere_axis" -variable sphere_axis
pack $takestep(axis_controls_5).ds_sphere_axis -side left

button $takestep(axis_controls_5).ds_reset_axis \
  -text "reset axis" -command {reset_ds_obj}
pack $takestep(axis_controls_5).ds_reset_axis -side left

# ---------------------------------------------------------
pack $takestep(goto).go_to_pos -side left
pack $takestep(goto).test -side left
pack $takestep(goto).keep_stepping_check -side right

pack $takestep(current_pos).cur_x $takestep(current_pos).value_cur_x -side left
pack $takestep(current_pos).cur_y $takestep(current_pos).value_cur_y -side left

pack $takestep(goto_c).go_to_pos_c -side left
pack $takestep(goto_c).test_c -side left
pack $takestep(goto_c).keep_stepping_check_c -side right

pack $takestep(current_pos_c).cur_x_c $takestep(current_pos_c).value_cur_x_c -side left
pack $takestep(current_pos_c).cur_y_c $takestep(current_pos_c).value_cur_y_c -side left

#X coord functions
proc plus_x {} {
	global take_x_step
	global step_x_size
	set take_x_step $step_x_size
 }

proc minus_x {} {
global take_x_step
global step_x_size
set take_x_step -$step_x_size
}


#y coord functions
proc plus_y {} {
	global take_y_step
	global step_y_size
	set take_y_step $step_y_size
 }

proc minus_y {} {
global take_y_step
global step_y_size
set take_y_step -$step_y_size
}

#Z coord functions
proc plus_z {} {
	global take_z_step
	global step_z_size
	set take_z_step $step_z_size
 }

proc minus_z {} {
global take_z_step
global step_z_size
set take_z_step -$step_z_size
}

proc go_to_pos {} {
global go_to_pos
global step_go_to_pos
set step_go_to_pos $go_to_pos
}

global newmodifyp_control
global user_0_mode

trace variable user_0_mode w set_direct_step_state
trace variable newmodifyp_control w show_direct_step_controls
trace variable setting_direct_step_axis w set_ds_axis
trace variable setting_direct_step_axis w show_direct_step_controls

trace variable step_x_size w set_graphics_x_step
trace variable step_y_size w set_graphics_y_step
trace variable step_z_size w set_graphics_z_step

proc set_graphics_x_step {nm el op} {
global ds_red_axis_ss step_x_size
set ds_red_axis_ss $step_x_size
}

proc set_graphics_y_step {nm el op} {
global ds_green_axis_ss step_y_size
set ds_green_axis_ss $step_y_size
}

proc set_graphics_z_step {nm el op} {
global ds_blue_axis_ss step_z_size
set ds_blue_axis_ss $step_z_size
}

#enables and disables direct step controls.
#if we are not in touch mode, disable the stepping buttons
#and the go to buttons.

proc set_direct_step_state {nm el op} {
global user_0_mode
global takestep modifyp_tool collab_commands_suspended 
# user_0_mode == 12 means we are in touch mode
if { $collab_commands_suspended == 0 } {
if {$user_0_mode == 12} {
  #regular axis buttons
  $takestep(buttons).plus_y configure -state normal
  $takestep(buttons2).plus_x configure -state normal
  $takestep(buttons2).minus_x configure -state normal
  $takestep(buttons3).minus_y configure -state normal

  $takestep(buttons_z).plus_z configure -state normal
  $takestep(buttons_z3).minus_z configure -state normal

  #colored axis
  $takestep(buttons_c).plus_y_c configure -state normal
  $takestep(buttons2_c).plus_x_c configure -state normal
  $takestep(buttons2_c).minus_x_c configure -state normal
  $takestep(buttons3_c).minus_y_c configure -state normal

  $takestep(buttons_z_c).plus_z_c configure -state normal
  $takestep(buttons_z3_c).minus_z_c configure -state normal

  #goto button
  $takestep(goto).go_to_pos configure -state normal
  $takestep(goto_c).go_to_pos_c configure -state normal
  } else {
  #regular axis buttons
  $takestep(buttons).plus_y configure -state disabled
  $takestep(buttons2).plus_x configure -state disabled
  $takestep(buttons2).minus_x configure -state disabled
  $takestep(buttons3).minus_y configure -state disabled

  $takestep(buttons_z).plus_z configure -state disabled
  $takestep(buttons_z3).minus_z configure -state disabled

  #colored axis
  $takestep(buttons_c).plus_y_c configure -state disabled
  $takestep(buttons2_c).plus_x_c configure -state disabled
  $takestep(buttons2_c).minus_x_c configure -state disabled
  $takestep(buttons3_c).minus_y_c configure -state disabled

  $takestep(buttons_z_c).plus_z_c configure -state disabled
  $takestep(buttons_z3_c).minus_z_c configure -state disabled

  #goto button
  $takestep(goto).go_to_pos configure -state disabled
  $takestep(goto_c).go_to_pos_c configure -state disabled
 }
}
}

proc set_ds_axis {nm el op} {
    global modelFile import_file_label current_object current_object_new
    global setting_direct_step_axis
    if { $setting_direct_step_axis == 1 } {

	# "show.import_objects"
	set filename "/axis.dsa"	
	set modelFile $filename
	set current_object_new "axis.dsa"
	set import_scale 300
    } else {
    #close the axis object here?
    global modelFile nmInfo import_file_label

    set modelFile ""
    set import_file_label "all"
    }

}

proc reset_ds_obj { } {
global import_transx import_transy import_transz
global import_rotx import_roty import_rotz
set import_transx 0
set import_transy 0
set import_transz 0

set import_rotx 0
set import_roty 0
set import_rotz 0
}
proc set_ds_grab_object { } {
global import_grab_object
global user_0_mode
if {$import_grab_object == 1} {
   set user_0_mode 1
}

}

proc pack_forget_all { } {
global takestep
    pack forget $takestep(ds).stepping
    pack forget $takestep(ds).z_stepping
    pack forget $takestep(ds).step_size_frame
    pack forget $takestep(ds).current_pos
    pack forget $takestep(ds).goto
    pack forget $takestep(ds).go_to_pos_frame

    # pack forget $takestep(ds).stepping_c
    pack forget $takestep(ds).color_axis
    pack forget $takestep(color_axis).z_stepping_c
    pack forget $takestep(color_axis).step_size_frame_c
    pack forget $takestep(color_axis).current_pos_c
    pack forget $takestep(color_axis).goto_c
    pack forget $takestep(color_axis).go_to_pos_frame_c

    pack forget $takestep(ds).axis_controls
   
}

proc show_direct_step_controls {nm el op} {
    global newmodifyp_control newmodifyp_tool takestep
    global setting_direct_step_axis
    global import_lock_rotx import_lock_roty
    global import_rotx import_roty
# ---------------------------------------------------------------- #
# every time we are possibly changing the window, we are going to   #
# get rid of all the frames and rebuild from scratch               #
# pack_forget_all gets rid of all the frames. If we are in the     #
# regular axis mode, we then check to see if we are in Direct_z    #
# mode. If we are not in direct Z mode, we remove any possible     #
# widgets in the frames that are related to direct Z               #
# if we are in direct Z mode, than we add any widgets that are used#
# in direct Z mode. the same thing happens if we are using an      #
# interface for a user defined axis, but with the _c added on the  #
# end, to refer to the controls for colered axis.                  #
# ---------------------------------------------------------------- #



    pack_forget_all

    if { $setting_direct_step_axis == 0 } {
	if { $newmodifyp_control == 0} {
	    #hide all widgets in frames that use the Z position
	    pack forget $takestep(pos).z_pos
	    pack forget $takestep(stepSize).step_z_size
	    pack forget $takestep(current_pos).cur_z
	    pack forget $takestep(current_pos).value_cur_z

	    #enable frames for stepping along regular axis
	    pack $takestep(ds).stepping -side left -anchor nw
	    pack $takestep(ds).step_size_frame -anchor nw
	    pack $takestep(ds).go_to_pos_frame -anchor nw
	    pack $takestep(ds).current_pos -anchor nw
	    pack $takestep(ds).goto	-anchor nw

	} else {

	    #regular steping axis in x/y	    
	    pack $takestep(ds).stepping -side left -anchor nw

	    # add Z stepping Frame
	    pack $takestep(ds).z_stepping -side left -anchor nw

	    # add back the frames that we had taken out.
	    pack $takestep(ds).step_size_frame -anchor nw
	    pack $takestep(ds).go_to_pos_frame -anchor nw
	    pack $takestep(ds).current_pos -anchor nw
	    pack $takestep(ds).goto	-anchor nw

	    # add appropiate Z widgets back into frames
	    pack $takestep(stepSize).step_z_size -side left
	    pack $takestep(pos).z_pos -side left
	    pack $takestep(current_pos).cur_z $takestep(current_pos).value_cur_z -side left
	}
    } else {
	if { $newmodifyp_control == 0} {

	    # hide widgets used in direct Z only that might be turned on
	    pack forget $takestep(pos_c).z_pos_c
	    pack forget $takestep(stepSize_c).step_z_size_c
	    pack forget $takestep(current_pos_c).cur_z
	    pack forget $takestep(current_pos_c).value_cur_z

	    pack $takestep(ds).color_axis -side left
	    pack $takestep(color_axis).stepping_c -side left -anchor nw
	    pack $takestep(color_axis).step_size_frame_c -anchor nw
	    pack $takestep(color_axis).go_to_pos_frame_c -anchor nw
	    pack $takestep(color_axis).current_pos_c -anchor nw
	    pack $takestep(color_axis).goto_c	-anchor nw

	    pack $takestep(ds).axis_controls -side left -anchor nw

	    #we must set x,y rotations to 0, lock them, and remove their controls from the window

	    #setx, y rotations to 0
	    set import_rotx 0
	    set import_roty 0

	    #lock x and y rotations,
	    set import_lock_rotx 1
	    set import_lock_roty 1

	    #  and remove sliders from the screen.
	    pack forget $takestep(axis_controls_3).import_rotx_slide
	    pack forget $takestep(axis_controls_3).import_roty_slide
	    
	    pack forget $takestep(axis_controls_4).import_lock_rotx_button
	    pack forget $takestep(axis_controls_4).import_lock_roty_button
	    

	} else {
	    
	    pack $takestep(ds).color_axis -side left
	    pack $takestep(color_axis).stepping_c -side left
	    pack $takestep(color_axis).z_stepping_c -side left -anchor nw
	    pack $takestep(color_axis).step_size_frame_c -anchor nw
	    pack $takestep(color_axis).go_to_pos_frame_c -anchor nw
	    pack $takestep(color_axis).current_pos_c -anchor nw
	    pack $takestep(color_axis).goto_c	-anchor nw
	    
	    pack $takestep(stepSize_c).step_z_size_c -side left
	    pack $takestep(pos_c).z_pos_c -side left
	    pack $takestep(current_pos_c).cur_z_c $takestep(current_pos_c).value_cur_z_c -side left

	    #lock the z rotation
	    pack forget $takestep(axis_controls_4).import_tune_rot_button
	    pack forget $takestep(axis_controls_3).import_rotz_slide
	    pack forget $takestep(axis_controls_4).import_lock_rotz_button

	    pack $takestep(axis_controls_3).import_rotx_slide -side left
	    pack $takestep(axis_controls_3).import_roty_slide -side left
	    pack $takestep(axis_controls_3).import_rotz_slide -side left

	    pack $takestep(axis_controls_4).import_lock_rotx_button -side left
	    pack $takestep(axis_controls_4).import_lock_roty_button -side left
	    pack $takestep(axis_controls_4).import_lock_rotz_button -side left



	    pack $takestep(ds).axis_controls -anchor nw
	}
	
    }
}

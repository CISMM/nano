#/*===3rdtech===
#  Copyright (c) 2000 by 3rdTech, Inc.
#  All Rights Reserved.
#
#  This file may not be distributed without the permission of 
#  3rdTech, Inc. 
#  ===3rdtech===*/

# Create a toplevel window for the Keithley VI Curve generator controls.
# It is initially hidden, but can be shown by calling
# the "show" procedure.
# ----------------------------------------------------------------------
set vi_win [create_closing_toplevel vi_win "VI Curves"]

# ----------------------------------------------------------------------
# Create a toplevel window for the navigation pad, which moves the surface.
# It is initially hidden, but can be shown by calling
# the "show" procedure.
# ----------------------------------------------------------------------
set nav_win [create_closing_toplevel nav_win "Navigate Tool"]

basicjoys_create $nav_win.joy "Translate" "Rotate"

# -------
# Phantom controls 
set phantom_win [create_closing_toplevel phantom_win "Phantom Settings"]
button $phantom_win.phantom_reset -text "Reset Phantom" -command "set reset_phantom 1"

if { $thirdtech_ui } {
    set rb_list { "Press and Hold" "Toggle" }
} else {
    set rb_list { "Press and Hold" "Toggle" "ButtonBox" }
}
generic_radiobox $phantom_win.phantom_button_mode \
	phantom_button_mode \
	"Phantom Button" $rb_list

# Phantom spring constant, 
set spring_slider_min_limit 0.01
set spring_slider_max_limit 1
set spring_k_slider 0

floatscale $phantom_win.spring_k $spring_slider_min_limit $spring_slider_max_limit \
	    100 1 1 spring_k_slider "Spring Constant"

pack $phantom_win.phantom_reset $phantom_win.phantom_button_mode $phantom_win.spring_k \
	-side top -fill x

# --------
# Magellan controls
set magellan_win [create_closing_toplevel magellan_win "Magellan Settings"]
button $magellan_win.magellan_reset -text "Reconnect to Magellan" -command "set reconnect_magellan 1"
pack $magellan_win.magellan_reset -side top -fill x
image create photo view_b0 \
	-file [file join ${tcl_script_dir} images b0.gif] \
	-format GIF 
pack [label $magellan_win.instr1 -text "Toggle"]
pack [label $magellan_win.instr2 -image view_b0]
pack [label $magellan_win.instr3 -text "to move and rotate 
the surface with the puck."] 
# -------
# Mouse Phantom controls - let the mouse emulate a phantom tracker and button
# (but not forces, of course...)
set mouse_phantom_win [create_closing_toplevel mouse_phantom_win "Mouse Phantom Settings"]
#checkbutton $mouse_phantom_win.mp_enable \
#	-text "Enabled" -variable mp_enabled -anchor nw

button $mouse_phantom_win.phantom_reset -text "Reset Mouse Phantom" -command "set reset_phantom 1"

# Rotational and translational motion scale for mouse control. 
set mp_rot_scale 1.0
floatscale $mouse_phantom_win.mp_rot_scale 0.3 3.0 \
	    100 1 1 mp_rot_scale "Rotational Motion Scale"
set mp_trans_scale 1.0
floatscale $mouse_phantom_win.mp_trans_scale 0.3 3.0 \
	    100 1 1 mp_trans_scale "Translational Motion Scale"

pack $mouse_phantom_win.phantom_reset \
        $mouse_phantom_win.mp_rot_scale \
        $mouse_phantom_win.mp_trans_scale \
	-side top -fill x -expand yes

pack [label $mouse_phantom_win.instr -justify left -text \
"Left button-down translates 
the hand in the X-Y plane.
Hold the Control key for rotation.
Hold the Shift key for Z motion.
Right button-down translates 
the hand and pushes the 
Phantom button. "] 
# This file sets up the $scanline frame, and must be sourced from a
# specific location inside mainwin.tcl
# 
# for widgets that change behavior of the scanline modification mode
# Adam Seeger July 1999

frame $scanline -relief raised -bd 3 -bg $bc

frame $scanline.mode -bg $bc
frame $scanline.modeparam -bg $bc
frame $scanline.forcelimit -bg $bc
frame $scanline.forcelimitparam -bg $bc
frame $scanline.lineposition -bg $bc
frame $scanline.scancontrol -bg $bc

set accepted_scanline_params 0
set display_scanline_position 0

set start_linescan 0

# tapping or contact
set scanlinep_mode 0	

# boolean parameters controlled by checkboxes
set scanlinep_use_feedback 1
set scanlinep_use_forcelimit 0
set scanlinep_continuous 0

# parameters for the scanline position (controlled by floatscales)
set scanlinep_endpos_x 0.0
set scanlinep_endpos_y 0.0
set scanlinep_endpos_z 0.0
set scanlinep_angle 0.0
set scanlinep_slope_nm_per_micron 0.0

# floatscale parameters
set scanlinep_resolution 100.0
set scanlinep_setpoint 1.0
set scanlinep_p_gain 1.0
set scanlinep_i_gain 0.5
set scanlinep_d_gain 0.01
set scanlinep_rate 20.0
set scanlinep_amplitude 0.1
set scanlinep_width 500.0
set scanlinep_forcelimit 10.0
set scanlinep_max_z_step 1.0
set scanlinep_max_xy_step 1.0

# these are the parameters that one must hit the Accept button to
# let changes take effect (i.e. get copied from newscanlinep_x to
# scanlinep_x)

set scanlineplist "{mode use_feedback use_forcelimit \
  setpoint p_gain i_gain d_gain \
  rate amplitude forcelimit max_z_step max_xy_step}"

# continuous, display_scanline_position,
# pos_x_offset, pos_y_offset, pos_z_offset,
# pos_angle_offset, width, resolution will take effect immediately

set newscanlinep_mode $scanlinep_mode
set newscanlinep_use_feedback $scanlinep_use_feedback
set newscanlinep_use_forcelimit $scanlinep_use_forcelimit
set newscanlinep_setpoint $scanlinep_setpoint
set newscanlinep_p_gain $scanlinep_p_gain
set newscanlinep_i_gain $scanlinep_i_gain
set newscanlinep_d_gain $scanlinep_d_gain
set newscanlinep_rate $scanlinep_rate 
set newscanlinep_amplitude $scanlinep_amplitude
set newscanlinep_forcelimit $scanlinep_forcelimit
set newscanlinep_max_z_step $scanlinep_max_z_step
set newscanlinep_max_xy_step $scanlinep_max_xy_step

trace variable newscanlinep_mode w flip_scanline_mode
trace variable newscanlinep_use_forcelimit w flip_scanline_forcelimit

# proc updateFromC defined inside image.tcl
# it must be sourced before this file!!! - and note that it references
# the global variable "scanline"
trace variable scanlinep_mode w updateFromC
trace variable scanlinep_use_feedback w updateFromC
trace variable scanlinep_use_forcelimit w updateFromC
trace variable scanlinep_setpoint w updateFromC
trace variable scanlinep_p_gain w updateFromC
trace variable scanlinep_i_gain w updateFromC
trace variable scanlinep_d_gain w updateFromC
trace variable scanlinep_rate w updateFromC
trace variable scanlinep_amplitude w updateFromC
trace variable scanlinep_forcelimit w updateFromC
trace variable scanlinep_max_z_step w updateFromC
trace variable scanlinep_max_xy_step w updateFromC

# these traces change the color of Accept and Cancel when you haven't
# pressed Accept yet.
trace variable newscanlinep_mode w scanlineBackgChReal
trace variable newscanlinep_use_feedback w scanlineBackgChReal
trace variable newscanlinep_use_forcelimit w scanlineBackgChReal
trace variable newscanlinep_setpoint w scanlineBackgChReal
trace variable newscanlinep_p_gain w scanlineBackgChReal
trace variable newscanlinep_i_gain w scanlineBackgChReal
trace variable newscanlinep_d_gain w scanlineBackgChReal
trace variable newscanlinep_rate w scanlineBackgChReal
trace variable newscanlinep_amplitude w scanlineBackgChReal
trace variable newscanlinep_forcelimit w scanlineBackgChReal
trace variable newscanlinep_max_z_step w scanlineBackgChReal
trace variable newscanlinep_max_xy_step w scanlineBackgChReal

pack $scanline.mode $scanline.modeparam $scanline.forcelimit \
     $scanline.forcelimitparam $scanline.lineposition $scanline.scancontrol \
     -side left -padx 2m -fill both

#setup Scanline mode box
label $scanline.mode.label -text "Scanline Mode" -bg $bc
pack $scanline.mode.label -side top -anchor nw
radiobutton $scanline.mode.tapping -text "Tapping" -variable newscanlinep_mode -value 0 -bg $fc
radiobutton $scanline.mode.contact -text "Contact" -variable newscanlinep_mode -value 1 -bg $fc
button $scanline.mode.accept -text "Accept" -bg $fc \
        -command "acceptScanlineVars $scanlineplist"
button $scanline.mode.cancel -text "Cancel" -bg $fc \
        -command "cancelScanlineVars $scanlineplist"


pack $scanline.mode.tapping $scanline.mode.contact -side top -fill x
pack $scanline.mode.cancel $scanline.mode.accept -side bottom -fill x

#setup Scanline modeparam box
label $scanline.modeparam.label -text "Mode parameters" -bg $bc
pack $scanline.modeparam.label -side top -anchor nw


floatscale $scanline.modeparam.setpoint -70 70 141 1 1 newscanlinep_setpoint \
        "Setpoint"
floatscale $scanline.modeparam.p-gain 0 5 51 1 1 newscanlinep_p_gain "P-Gain"
floatscale $scanline.modeparam.i-gain 0 2 21 1 1 newscanlinep_i_gain "I-Gain"
floatscale $scanline.modeparam.d-gain 0 5 51 1 1 newscanlinep_d_gain "D-Gain"
floatscale $scanline.modeparam.rate 0.1 50.0 101 1 1 newscanlinep_rate \
	"Rate (uM/sec)"
floatscale $scanline.modeparam.amplitude 0 1 101 1 1 newscanlinep_amplitude \
        "Amplitude"


pack    $scanline.modeparam.setpoint $scanline.modeparam.p-gain \
        $scanline.modeparam.i-gain $scanline.modeparam.d-gain \
        $scanline.modeparam.rate \
        -side top -fill x -pady $fspady

if {$newscanlinep_mode == 0} {
  pack $scanline.modeparam.amplitude -side top -fill x
}

set scanline_tapping_list "$scanline.modeparam.amplitude"

#setup Scanline force limit box
label $scanline.forcelimit.label -text "Force Limit" -bg $bc
pack $scanline.forcelimit.label -side top -anchor nw
radiobutton $scanline.forcelimit.ignore_force -text "No Limit!!" \
    -variable newscanlinep_use_forcelimit -value 0 -bg $fc
radiobutton $scanline.forcelimit.watch_force -text "Force Limited" \
    -variable newscanlinep_use_forcelimit -value 1 -bg $fc
pack $scanline.forcelimit.ignore_force $scanline.forcelimit.watch_force \
	-side top -fill x

#setup Scanline force limit param box
label $scanline.forcelimitparam.label -text "Force Limit Parameters" -bg $bc
pack $scanline.forcelimitparam.label -side top -anchor nw

floatscale $scanline.forcelimitparam.max_z_step 0 5 51 1 1 \
	newscanlinep_max_z_step "max_z_step"
floatscale $scanline.forcelimitparam.max_xy_step 0 5 51 1 1 \
	newscanlinep_max_xy_step "max_xy_step"
floatscale $scanline.forcelimitparam.forcelimit 0 70 51 1 1 \
	newscanlinep_forcelimit "force limit"

if {$newscanlinep_use_forcelimit} {
  pack $scanline.forcelimitparam.max_z_step \
	$scanline.forcelimitparam.max_xy_step \
	$scanline.forcelimitparam.forcelimit -side top -fill x
}

set scanline_forcelimited_list  "$scanline.forcelimitparam.max_z_step $scanline.forcelimitparam.max_xy_step $scanline.forcelimitparam.forcelimit"

###################################
label $scanline.lineposition.label -text "Line Settings" -bg $bc
pack $scanline.lineposition.label -side top -anchor nw

checkbutton $scanline.lineposition.showposition_check -text \
	"Show Line Position" -variable display_scanline_position
floatscale $scanline.lineposition.x_finish 0 100 1001 1 1 \
	scanlinep_endpos_x "x at finish (% of region)"
floatscale $scanline.lineposition.y_finish 0 100 1001 1 1 \
	scanlinep_endpos_y "y at finish (% of region)"
floatscale $scanline.lineposition.z_finish 0 10000 1001 1 1 \
	scanlinep_endpos_z "z at finish (nm)"
floatscale $scanline.lineposition.angle 0 360 361 1 1 \
	scanlinep_angle "scan angle (degrees)"
floatscale $scanline.lineposition.width 1 100 1000 1 1 scanlinep_width \
        "Width (% of region)"
intscale $scanline.lineposition.resolution 10 1000 991 1 1 \
        scanlinep_resolution "# points"

pack $scanline.lineposition.showposition_check \
	$scanline.lineposition.x_finish \
	$scanline.lineposition.y_finish \
	$scanline.lineposition.z_finish \
	$scanline.lineposition.angle \
	$scanline.lineposition.width \
        $scanline.lineposition.resolution \
	-side top -fill x -pady $fspady

######################################
label $scanline.scancontrol.label -text "Scan Control" -bg $bc
pack $scanline.scancontrol.label -side top -anchor nw

checkbutton $scanline.scancontrol.use_feedback -text "Use Feedback" \
	-variable newscanlinep_use_feedback
checkbutton $scanline.scancontrol.continuous -text "Continuous Scan" \
	-variable scanlinep_continuous
button $scanline.scancontrol.doscan -text "Start Scan" -bg $fc \
        -command "doScanline"
pack $scanline.scancontrol.use_feedback $scanline.scancontrol.continuous \
	$scanline.scancontrol.doscan \
	-side top -fill x -pady $fspady

#
#
######################
# Flip_* procedures
#   These change the displayed widgets, depending on the
# value of a global variable, i.e. which radiobutton is pressed.
#

# flips $scanline.modeparam widgets
proc flip_scanline_mode {scanline_mode element op} {
    global scanline
    global scanline_tapping_list
    global fspady

    upvar $scanline_mode k

    if {$k==0} {
        # selected tapping
        # Magic number 6 = number of floatscales to leave alone + 1
        set plist [lrange [pack slaves $scanline.modeparam] 6 end]
        foreach widg $plist {pack forget $widg}
        foreach widg $scanline_tapping_list \
		{pack $widg -side top -fill x -pady $fspady}
    } elseif {$k==1} {
        # selected contact
        set plist [lrange [pack slaves $scanline.modeparam] 6 end]
        foreach widg $plist {pack forget $widg}
    }
}

# flips $scanline.forcelimitparam widgets
proc flip_scanline_forcelimit {scanline_forcelimited element op} {
    global scanline
    global scanline_forcelimited_list
    global fspady

    upvar $scanline_forcelimited k

    if {$k==0} {
        # selected no limit
        set plist [lrange [pack slaves $scanline.forcelimitparam] 1 end]
        foreach widg $plist {pack forget $widg}
    } elseif {$k==1} {
        # selected limit
        set plist [lrange [pack slaves $scanline.forcelimitparam] 1 end]
        foreach widg $plist {pack forget $widg}
        foreach widg $scanline_forcelimited_list \
		{pack $widg -side top -fill x -pady $fspady}
    }
}

#
# Change the background of Accept and Cancel buttons
#    when you haven't yet committed your changes by clicking Accept
#

proc scanlineBackgChReal {fooa element op} {
    global scanline

    $scanline.mode.accept configure -background LightPink1
    $scanline.mode.cancel configure -background LightPink1
}

#
# Bound to the accept button
#
proc acceptScanlineVars {varlist} {
    global accepted_scanline_params
    global scanline
    global fc

    foreach val $varlist {
        global scanlinep_$val
        global newscanlinep_$val
        global accepted_scanline_params
        if {[set newscanlinep_$val] != [set scanlinep_$val] } {
            set scanlinep_$val [set newscanlinep_$val]
        }
    }
    # this is linked so C code will do something with new parameters
    set accepted_scanline_params 1
    # None of the newscanline_* vars are now changed
    $scanline.mode.accept configure -background $fc
    $scanline.mode.cancel configure -background $fc

}

proc cancelScanlineVars {varlist} {

    global scanline
    global fc

    foreach val $varlist {
        global scanlinep_$val
        global newscanlinep_$val
        if {[set newscanlinep_$val] != [set scanlinep_$val] } {
            set newscanlinep_$val [set scanlinep_$val]
        }
    }
    # None of the newscanline_* vars are now changed
    $scanline.mode.accept configure -background $fc
    $scanline.mode.cancel configure -background $fc
}

proc doScanline {} {
    global start_linescan
    set start_linescan 1
}

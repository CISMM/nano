# This file sets up the $modify frame, and must be sourced from a
# specific location inside tools.tcl
#
# for widgets that change behavior of modify mode
#
frame $modify -relief raised -bd 3 -bg $bc
frame $modify.mode -bg $bc
frame $modify.modeparam -bg $bc
frame $modify.style -bg $bc
frame $modify.styleparam -bg $bc
frame $modify.tool -bg $bc
frame $modify.toolparam -bg $bc
frame $modify.control -bg $bc
frame $modify.controlparam -bg $bc

# Tool variable initialization
# in the real thing these will be inherited from microscope - i think

# traced by C code so it knows there are new modify parameters
set accepted_modify_params 0

set modifyp_mode 1
set modifyp_control 0
set modifyp_style 0
set modifyp_tool 0

set modifyp_setpoint 1.0
set modifyp_p_gain 1.0
set modifyp_i_gain 0.5
set modifyp_d_gain 0.01
set modifyp_rate 20.0
set modifyp_amplitude 0.1

#DEBUG
#proc printvar {fooa element op} {
#    global modifyp_amplitude
#    puts "XXXX new mod amplitude $modifyp_amplitude"
#}
#trace variable modifyp_amplitude w printvar



#set modifyp_tri_size 1
#set modifyp_tri_speed 5000
set modifyp_sweep_width 500

set modifyp_bot_delay 1.0
set modifyp_top_delay 1.0
set modifyp_z_pull 50.0
set modifyp_punchdist 2.0
set modifyp_speed 200
set modifyp_watchdog 100

set modifyp_start_delay 10.0
set modifyp_z_start -100.0
set modifyp_z_end 0.0
set modifyp_z_pullback -100.0
set modifyp_force_limit 0.5
set modifyp_fcdist 100.0
set modifyp_num_layers 100
set modifyp_num_hcycles 1
set modifyp_sample_speed .1
set modifyp_pullback_speed 1
set modifyp_start_speed 0.1
set modifyp_feedback_speed 1
set modifyp_avg_num 3
set modifyp_sample_delay 0.0
set modifyp_pullback_delay 0.0
set modifyp_feedback_delay 200.0

set modifyp_step_size 1

set modifyp_max_z_step 1
set modifyp_max_xy_step 1
set modifyp_min_z_setpoint -50
set modifyp_max_z_setpoint 50
set modifyp_max_lat_setpoint 50

set modifyplist "{mode control style tool 
  setpoint p_gain i_gain d_gain rate amplitude 
  sweep_width 
  bot_delay top_delay z_pull punchdist speed watchdog amplitude
  step_size
  start_delay z_start z_end z_pullback force_limit fcdist 
  num_layers num_hcycles
  sample_speed pullback_speed start_speed feedback_speed
  avg_num sample_delay pullback_delay feedback_delay
  max_z_step max_xy_step min_z_setpoint max_z_setpoint max_lat_setpoint}"

set newmodifyp_mode $modifyp_mode
set newmodifyp_control $modifyp_control
set newmodifyp_style $modifyp_style
set newmodifyp_tool $modifyp_tool

set newmodifyp_setpoint $modifyp_setpoint
set newmodifyp_p_gain $modifyp_p_gain
set newmodifyp_i_gain $modifyp_i_gain
set newmodifyp_d_gain $modifyp_d_gain 
set newmodifyp_amplitude $modifyp_amplitude
set newmodifyp_rate $modifyp_rate

#set newmodifyp_tri_size $modifyp_tri_size
#set newmodifyp_tri_speed $modifyp_tri_speed 
set newmodifyp_sweep_width $modifyp_sweep_width

set newmodifyp_bot_delay $modifyp_bot_delay
set newmodifyp_top_delay $modifyp_top_delay 
set newmodifyp_z_pull $modifyp_z_pull
set newmodifyp_punchdist $modifyp_punchdist
set newmodifyp_speed $modifyp_speed 
set newmodifyp_watchdog $modifyp_watchdog

set newmodifyp_start_delay $modifyp_start_delay
set newmodifyp_z_start $modifyp_z_start
set newmodifyp_z_end $modifyp_z_end
set newmodifyp_z_pullback $modifyp_z_pullback
set newmodifyp_force_limit $modifyp_force_limit
set newmodifyp_fcdist $modifyp_fcdist
set newmodifyp_num_layers $modifyp_num_layers
set newmodifyp_num_hcycles $modifyp_num_hcycles
set newmodifyp_sample_speed $modifyp_sample_speed
set newmodifyp_pullback_speed $modifyp_pullback_speed
set newmodifyp_start_speed $modifyp_start_speed
set newmodifyp_feedback_speed $modifyp_feedback_speed
set newmodifyp_avg_num $modifyp_avg_num
set newmodifyp_sample_delay $modifyp_sample_delay
set newmodifyp_pullback_delay $modifyp_pullback_delay
set newmodifyp_feedback_delay $modifyp_feedback_delay

set newmodifyp_step_size $modifyp_step_size

set newmodifyp_max_z_step $modifyp_max_z_step
set newmodifyp_max_xy_step $modifyp_max_xy_step
set newmodifyp_min_z_setpoint $modifyp_min_z_setpoint
set newmodifyp_max_z_setpoint $modifyp_max_z_setpoint
set newmodifyp_max_lat_setpoint $modifyp_max_lat_setpoint

trace variable newmodifyp_mode w flip_mod_mode

trace variable newmodifyp_style w flip_mod_style

trace variable newmodifyp_tool w flip_mod_tool

trace variable newmodifyp_control w flip_mod_control

# proc updateFromC defined inside image.tcl 
# it must be sourced before this file!!!
trace variable modifyp_mode w updateFromC
trace variable modifyp_control w updateFromC
trace variable modifyp_style w updateFromC
trace variable modifyp_tool w updateFromC

trace variable modifyp_setpoint w updateFromC
trace variable modifyp_p_gain w updateFromC
trace variable modifyp_i_gain w updateFromC
trace variable modifyp_d_gain w updateFromC
trace variable modifyp_amplitude w updateFromC
trace variable modifyp_rate w updateFromC

#trace variable modifyp_tri_size w updateFromC
#trace variable modifyp_tri_speed w updateFromC
trace variable modifyp_sweep_width w updateFromC

trace variable modifyp_bot_delay w updateFromC
trace variable modifyp_top_delay w updateFromC
trace variable modifyp_z_pull w updateFromC
trace variable modifyp_punchdist w updateFromC
trace variable modifyp_speed w updateFromC
trace variable modifyp_watchdog w updateFromC

trace variable modifyp_start_delay w updateFromC
trace variable modifyp_z_start w updateFromC
trace variable modifyp_z_end w updateFromC
trace variable modifyp_z_pullback w updateFromC
trace variable modifyp_force_limit w updateFromC
trace variable modifyp_fcdist w updateFromC
trace variable modifyp_num_layers w updateFromC
trace variable modifyp_num_hcycles w updateFromC
trace variable modifyp_sample_speed w updateFromC
trace variable modifyp_pullback_speed w updateFromC
trace variable modifyp_start_speed w updateFromC
trace variable modifyp_feedback_speed w updateFromC
trace variable modifyp_avg_num w updateFromC
trace variable modifyp_sample_delay w updateFromC
trace variable modifyp_pullback_delay w updateFromC
trace variable modifyp_feedback_delay w updateFromC

trace variable modifyp_step_size w updateFromC

trace variable modifyp_max_z_step w updateFromC
trace variable modifyp_max_xy_step w updateFromC
trace variable modifyp_min_z_setpoint w updateFromC
trace variable modifyp_max_z_setpoint w updateFromC
trace variable modifyp_max_lat_setpoint w updateFromC


# these traces change the color of Accept and Cancel when you haven't
# pressed Accept yet.
trace variable newmodifyp_mode w modBackgChReal
trace variable newmodifyp_control w modBackgChReal
trace variable newmodifyp_style w modBackgChReal
trace variable newmodifyp_tool w modBackgChReal

trace variable newmodifyp_setpoint w modBackgChReal
trace variable newmodifyp_p_gain w modBackgChReal
trace variable newmodifyp_i_gain w modBackgChReal
trace variable newmodifyp_d_gain w modBackgChReal
trace variable newmodifyp_amplitude w modBackgChReal
trace variable newmodifyp_rate w modBackgChReal

#trace variable newmodifyp_tri_size w modBackgChReal
#trace variable newmodifyp_tri_speed w modBackgChReal
trace variable newmodifyp_sweep_width w modBackgChReal

trace variable newmodifyp_bot_delay w modBackgChReal
trace variable newmodifyp_top_delay w modBackgChReal
trace variable newmodifyp_z_pull w modBackgChReal
trace variable newmodifyp_punchdist w modBackgChReal
trace variable newmodifyp_speed w modBackgChReal
trace variable newmodifyp_watchdog w modBackgChReal

trace variable newmodifyp_start_delay w modBackgChReal
trace variable newmodifyp_z_start w modBackgChReal
trace variable newmodifyp_z_end w modBackgChReal
trace variable newmodifyp_z_pullback w modBackgChReal
trace variable newmodifyp_force_limit w modBackgChReal
trace variable newmodifyp_fcdist w modBackgChReal
trace variable newmodifyp_num_layers w modBackgChReal
trace variable newmodifyp_num_hcycles w modBackgChReal
trace variable newmodifyp_sample_speed w modBackgChReal
trace variable newmodifyp_pullback_speed w modBackgChReal
trace variable newmodifyp_start_speed w modBackgChReal
trace variable newmodifyp_feedback_speed w modBackgChReal
trace variable newmodifyp_avg_num w modBackgChReal
trace variable newmodifyp_sample_delay w modBackgChReal
trace variable newmodifyp_pullback_delay w modBackgChReal 
trace variable newmodifyp_feedback_delay w modBackgChReal


trace variable newmodifyp_step_size w modBackgChReal

trace variable newmodifyp_max_z_step w modBackgChReal
trace variable newmodifyp_max_xy_step w modBackgChReal
trace variable newmodifyp_min_z_setpoint w modBackgChReal
trace variable newmodifyp_max_z_setpoint w modBackgChReal
trace variable newmodifyp_max_lat_setpoint w modBackgChReal

#
#setup Modify box
#
pack $modify.mode $modify.modeparam $modify.style $modify.styleparam \
    $modify.tool $modify.toolparam $modify.control $modify.controlparam \
    -side left -padx 2m -fill both

#setup Modify mode box
label $modify.mode.label -text "Modify Mode" -bg $bc
pack $modify.mode.label -side top -anchor nw
radiobutton $modify.mode.oscillating -text "Oscillating" -variable newmodifyp_mode -value 0 -bg $fc
radiobutton $modify.mode.contact -text "Contact" -variable newmodifyp_mode -value 1 -bg $fc
button $modify.mode.accept -text "Accept" -bg $fc \
	-command "acceptModifyVars $modifyplist"
button $modify.mode.cancel -text "Cancel" -bg $fc \
	-command "cancelModifyVars $modifyplist"

checkbutton $modify.mode.relaxcomp -text "Relax Comp on" -variable doRelaxComp \
	-bg $fc

pack $modify.mode.oscillating $modify.mode.contact -side top -fill x
pack $modify.mode.relaxcomp -side top -fill x -pady 20

pack $modify.mode.cancel $modify.mode.accept -side bottom -fill x


#setup Modify modeparam box
label $modify.modeparam.label -text "Mode parameters" -bg $bc
pack $modify.modeparam.label -side top -anchor nw

floatscale $modify.modeparam.setpoint -70 70 141 1 1 newmodifyp_setpoint \
	"Setpoint" 
floatscale $modify.modeparam.p-gain 0 5 51 1 1 newmodifyp_p_gain "P-Gain" 
floatscale $modify.modeparam.i-gain 0 2 21 1 1 newmodifyp_i_gain "I-Gain" 
floatscale $modify.modeparam.d-gain 0 5 51 1 1 newmodifyp_d_gain "D-Gain" 
floatscale $modify.modeparam.rate 0.1 50.0 101 1 1 newmodifyp_rate "Rate (uM/sec)" 
floatscale $modify.modeparam.amplitude 0 1 101 1 1 newmodifyp_amplitude \
	"Amplitude" 


pack    $modify.modeparam.setpoint $modify.modeparam.p-gain \
	$modify.modeparam.i-gain $modify.modeparam.d-gain \
        $modify.modeparam.rate \
	-side top -fill x -pady $fspady
if {$newmodifyp_mode == 0} {
  pack $modify.modeparam.amplitude -side top -fill x 
}
set mod_oscillating_list "$modify.modeparam.amplitude"

#setup Modify style box
label $modify.style.label -text "Style" -bg $bc
pack $modify.style.label -side top -anchor nw
radiobutton $modify.style.sharp -text "Sharp" -variable newmodifyp_style \
	-value 0 -bg $fc
#radiobutton $modify.style.blunt -text "Blunt" -variable newmodifyp_style \
#	-value 1 -bg $fc

radiobutton $modify.style.sweep -text "Sweep" -variable newmodifyp_style \
	-value 2 -bg $fc
radiobutton $modify.style.sewing -text "Sewing" -variable newmodifyp_style \
	-value 3 -bg $fc
radiobutton $modify.style.forcecurve -text "ForceCurve" \
	-variable newmodifyp_style -value 4 -bg $fc
pack $modify.style.sharp $modify.style.sweep $modify.style.sewing \
	$modify.style.forcecurve -side top -fill x

#setup Modify styleparam box
label $modify.styleparam.label -text "Style parameters" -bg $bc
pack $modify.styleparam.label -side top -anchor nw

#floatscale $modify.styleparam.tri-size 0.5 5 101 1 1 \
#	newmodifyp_tri_size "Tri Size" 
#floatscale $modify.styleparam.tri-speed 1000 10000 101 1 1 \
#	newmodifyp_tri_speed "Tri Speed" 

floatscale $modify.styleparam.sweepwidth 0 1000 101 1 1 newmodifyp_sweep_width\
	"Sweep Width (nm)" 
checkbutton $modify.styleparam.sweeplock -text "Sweep Lock" \
	-variable sweep_lock_pressed 

floatscale $modify.styleparam.bot-delay 0 10 101 1 1 newmodifyp_bot_delay \
	"Bottom Delay" 
floatscale $modify.styleparam.top-delay 0 10 101 1 1 newmodifyp_top_delay \
	"Top Delay" 
floatscale $modify.styleparam.z-pull 10 1000 491 1 1 newmodifyp_z_pull \
	"Pullout Height" 
floatscale $modify.styleparam.punchdist 0 100 50 1 1 newmodifyp_punchdist \
	"Punch Dist." 
floatscale $modify.styleparam.speed 0 1000 491 1 1 newmodifyp_speed \
	"Speed" 
floatscale $modify.styleparam.watchdog 50 500 101 1 1 newmodifyp_watchdog \
	"Watchdog Dist." 

#floatscale $modify.styleparam.start-delay 0 200 201 1 1 newmodifyp_start_delay \
#	"Start Delay (us)" 
floatscale $modify.styleparam.z-start -1000 1000 1002 1 1 newmodifyp_z_start \
	"Start Height (nm)" 
floatscale $modify.styleparam.z-end -1000 1000 1002 1 1 newmodifyp_z_end \
	"End Height (nm)" 
floatscale $modify.styleparam.z-pullback -1000 0 500 1 1 newmodifyp_z_pullback \
	"Pullout Height (nm)" 
floatscale $modify.styleparam.force-limit 0 1000 491 1 1 newmodifyp_force_limit\
	 "Force limit (nA)" 
floatscale $modify.styleparam.forcecurvedist 0 500 100 1 1 newmodifyp_fcdist \
	"F.C. Dist. (nm)" 
floatscale $modify.styleparam.num-layers 1 100 100 1 1 newmodifyp_num_layers \
	"# samples" 
floatscale $modify.styleparam.num-halfcycles 1 4 4 1 1 newmodifyp_num_hcycles \
	"# half cycles" 
floatscale $modify.styleparam.sample-speed 0 10 1000 1 1 \
	newmodifyp_sample_speed "sample speed (um/s)" 
floatscale $modify.styleparam.pullback-speed 0 100 1000 1 1 \
	newmodifyp_pullback_speed "pullback speed (um/s)" 
floatscale $modify.styleparam.start-speed 0 10 1000 1 1 \
	newmodifyp_start_speed "start speed (um/s)" 
floatscale $modify.styleparam.fdback-speed 0 100 1000 1 1 \
	newmodifyp_feedback_speed "feedback speed (um/s)" 
floatscale $modify.styleparam.avg-num 1 10 10 1 1 \
	newmodifyp_avg_num "averaging" 
#floatscale $modify.styleparam.sample-delay 0 1000 1000 1 1 \
#	newmodifyp_sample_delay "sample delay (us)" 
#floatscale $modify.styleparam.pullback-delay 0 1000 1000 1 1 \
#	newmodifyp_pullback_delay "pullback delay (us)" 
#floatscale $modify.styleparam.feedback-delay 0 1000 1000 1 1 \
#	newmodifyp_feedback_delay "feedback delay (us)" 

#set mod_blunt_list "$modify.styleparam.tri-size $modify.styleparam.tri-speed"
set mod_sweep_list "$modify.styleparam.sweepwidth $modify.styleparam.sweeplock"
set mod_sewing_list "$modify.styleparam.bot-delay \
	$modify.styleparam.top-delay $modify.styleparam.z-pull \
        $modify.styleparam.punchdist $modify.styleparam.speed \
	$modify.styleparam.watchdog"
set mod_forcecurve_list " \
	$modify.styleparam.z-start $modify.styleparam.z-end \
	$modify.styleparam.z-pullback $modify.styleparam.force-limit \
	$modify.styleparam.forcecurvedist $modify.styleparam.num-layers \
	$modify.styleparam.num-halfcycles \
	$modify.styleparam.sample-speed $modify.styleparam.pullback-speed \
	$modify.styleparam.start-speed $modify.styleparam.fdback-speed \
	$modify.styleparam.avg-num "
#	$modify.styleparam.start-delay \
#	$modify.styleparam.sample-delay \
#	$modify.styleparam.pullback-delay $modify.styleparam.feedback-delay"

#setup Modify tool box
label $modify.tool.label -text "Tool" -bg $bc
pack $modify.tool.label -side top -anchor nw
radiobutton $modify.tool.freehand -text "Freehand" -variable newmodifyp_tool \
	-value 0 -bg $fc 
radiobutton $modify.tool.line -text "Line" -variable newmodifyp_tool \
	-value 1 -bg $fc 
radiobutton $modify.tool.constrfree -text "Constr. Free" -variable newmodifyp_tool \
	-value 2 -bg $fc 
radiobutton $modify.tool.slow_line -text "Slow Line" -variable newmodifyp_tool\
	-value 3 -bg $fc 
pack $modify.tool.freehand $modify.tool.line $modify.tool.constrfree \
	$modify.tool.slow_line -side top -fill x

#setup Modify toolparam box
label $modify.toolparam.label -text "Tool parameters" -bg $bc
pack $modify.toolparam.label -side top -anchor nw

floatscale $modify.toolparam.step-size 0 5 51 1 1 newmodifyp_step_size \
	"Step Size" 
set slow_line_playing 0
button $modify.toolparam.slow_line_play -text "Play" -command {
    if {$slow_line_playing} {
	set slow_line_playing 0
    } else {
	set slow_line_playing 1
    }
}

# Trace the variable and change the name of the button. Why a trace?
#Because this way the C code can set the variable and the button will
#display the right name.
proc config_play_name {name elem op} {
    global slow_line_playing modify
    if {$slow_line_playing} {
	$modify.toolparam.slow_line_play configure -text "Pause"
    } else {
	$modify.toolparam.slow_line_play configure -text "Play"
    }
}
trace variable slow_line_playing w config_play_name

button $modify.toolparam.slow_line_step -text "Step" -command {
    set slow_line_step 1
  }

set slow_line_direction 0
radiobutton $modify.toolparam.slow_line_forward -text "Forward" \
	-variable slow_line_direction -value 0 
radiobutton $modify.toolparam.slow_line_reverse -text "Reverse" \
	-variable slow_line_direction -value 1 

set mod_line_list  "$modify.toolparam.step-size"
set mod_slow_line_list "$modify.toolparam.step-size \
	$modify.toolparam.slow_line_play \
	$modify.toolparam.slow_line_step \
	$modify.toolparam.slow_line_forward \
	$modify.toolparam.slow_line_reverse"

#setup Modify control box
label $modify.control.label -text "Control" -bg $bc
pack $modify.control.label -side top -anchor nw
radiobutton $modify.control.feedback -text "Feedback" \
    -variable newmodifyp_control -value 0 -bg $fc
radiobutton $modify.control.directz -text "Direct Z" \
    -variable newmodifyp_control -value 1 -bg $fc 
pack $modify.control.feedback $modify.control.directz -side top -fill x

#setup Modify controlparam box
label $modify.controlparam.label -text "Control parameters" -bg $bc
pack $modify.controlparam.label -side top -anchor nw

floatscale $modify.controlparam.max_z_step 0 5 51 1 1 newmodifyp_max_z_step \
	"max_z_step" 
floatscale $modify.controlparam.max_xy_step 0 5 51 1 1 newmodifyp_max_xy_step \
	"max_xy_step" 
floatscale $modify.controlparam.min_z_setpoint -70 70 101 1 1 newmodifyp_min_z_setpoint \
	"min_z_setpoint" 
floatscale $modify.controlparam.max_z_setpoint 0 70 51 1 1 newmodifyp_max_z_setpoint \
	"max_z_setpoint" 
floatscale $modify.controlparam.max_lat_setpoint 0 70 51 1 1 newmodifyp_max_lat_setpoint \
	"max_lat_setpoint" 

set mod_directz_list  "$modify.controlparam.max_z_step $modify.controlparam.max_xy_step $modify.controlparam.min_z_setpoint $modify.controlparam.max_z_setpoint $modify.controlparam.max_lat_setpoint"


#
#
######################
# Flip_* procedures
#   These change the displayed widgets, depending on the
# value of a global variable, i.e. which radiobutton is pressed.
#

# flips $modify.modeparam widgets
proc flip_mod_mode {mod_mode element op} {
    global modify
    global mod_oscillating_list
    global fspady

    upvar $mod_mode k

    if {$k==0} {
        # selected oscillating
	# Magic number 6 = number of floatscales to leave alone + 1
	set plist [lrange [pack slaves $modify.modeparam] 6 end] 
	foreach widg $plist {pack forget $widg} 
	foreach widg $mod_oscillating_list {pack $widg -side top -fill x -pady $fspady}
    } elseif {$k==1} {
	# selected contact
	set plist [lrange [pack slaves $modify.modeparam] 6 end] 
	foreach widg $plist {pack forget $widg}
    }
}

# flips $modify.styleparam widgets
proc flip_mod_style {mod_style element op} {
    global modify
#    global mod_blunt_list
    global mod_sweep_list
    global mod_sewing_list
    global mod_forcecurve_list
    global fspady

    upvar $mod_style k

    if {$k==0} {
        # selected sharp
	set plist [lrange [pack slaves $modify.styleparam] 1 end] 
	foreach widg $plist {pack forget $widg} 
#    } elseif {$k==1} {
#	# selected blunt
#	set plist [lrange [pack slaves $modify.styleparam] 1 end] 
#	foreach widg $plist {pack forget $widg}
#	foreach widg $mod_blunt_list {pack $widg -side top -fill x -pady $fspady}
    } elseif {$k==2} {
	# selected sweep
	set plist [lrange [pack slaves $modify.styleparam] 1 end] 
	foreach widg $plist {pack forget $widg}
	foreach widg $mod_sweep_list {pack $widg -side top -fill x -pady $fspady}
    } elseif {$k==3} {
	# selected sewing
	set plist [lrange [pack slaves $modify.styleparam] 1 end] 
	foreach widg $plist {pack forget $widg}
	foreach widg $mod_sewing_list {pack $widg -side top -fill x -pady $fspady}
    } elseif {$k==4} {
	# selected force curve
	set plist [lrange [pack slaves $modify.styleparam] 1 end]
	foreach widg $plist {pack forget $widg}
	foreach widg $mod_forcecurve_list {pack $widg -side top -fill x -pady $fspady}
    }
}

# flips $modify.toolparam widgets
proc flip_mod_tool {mod_tool element op} {
    global modify
    global mod_line_list mod_slow_line_list
    global fspady

    upvar $mod_tool k

    if {$k==0} {
        # selected freehand
	set plist [lrange [pack slaves $modify.toolparam] 1 end] 
	foreach widg $plist {pack forget $widg} 
    } elseif {$k==1} {
	# selected line
	set plist [lrange [pack slaves $modify.toolparam] 1 end] 
	foreach widg $plist {pack forget $widg}
	foreach widg $mod_line_list {pack $widg -side top -fill x -pady $fspady}
    } elseif {$k==2} {
	# selected constrained freehand
	set plist [lrange [pack slaves $modify.toolparam] 1 end] 
	foreach widg $plist {pack forget $widg}
    } elseif {$k==3} {
	# selected slow line
	set plist [lrange [pack slaves $modify.toolparam] 1 end] 
	foreach widg $plist {pack forget $widg}
	foreach widg $mod_slow_line_list {pack $widg -side top -fill x -pady $fspady}
    }
}

# flips $modify.controlparam widgets
proc flip_mod_control {mod_control element op} {
    global modify
    global mod_directz_list
    global fspady

    upvar $mod_control k

    if {$k==0} {
        # selected feedback
	set plist [lrange [pack slaves $modify.controlparam] 1 end] 
	foreach widg $plist {pack forget $widg} 
    } elseif {$k==1} {
	# selected contact
	set plist [lrange [pack slaves $modify.controlparam] 1 end] 
	foreach widg $plist {pack forget $widg}
	foreach widg $mod_directz_list {pack $widg -side top -fill x -pady $fspady}
    }
}

#
# Change the background of Accept and Cancel buttons
#    when you haven't yet committed your changes by clicking Accept
#

proc modBackgChReal {fooa element op} {
    global modify

    $modify.mode.accept configure -background LightPink1
    $modify.mode.cancel configure -background LightPink1
}

#
# Bound to the accept button 
#
proc acceptModifyVars {varlist} {
    global accepted_modify_params
    global modify
    global fc

    foreach val $varlist {
	global modifyp_$val
	global newmodifyp_$val
	global accepted_modify_params
	if {[set newmodifyp_$val] != [set modifyp_$val] } {
	    set modifyp_$val [set newmodifyp_$val]
	}
    }
    # this is linked so C code will do something with new parameters
    set accepted_modify_params 1
    # None of the newmodify_* vars are now changed
    $modify.mode.accept configure -background $fc
    $modify.mode.cancel configure -background $fc

}


proc cancelModifyVars {varlist} {

    global modify
    global fc

    foreach val $varlist {
	global modifyp_$val
	global newmodifyp_$val
	if {[set newmodifyp_$val] != [set modifyp_$val] } {
	    set newmodifyp_$val [set modifyp_$val]
	}
    }
    # None of the newmodify_* vars are now changed
    $modify.mode.accept configure -background $fc
    $modify.mode.cancel configure -background $fc
}


#
# for widgets that change behavior of modify mode
#
set modify [create_closing_toplevel modify "Modify parameters"]

frame $modify.mode 
frame $modify.modeparam 
frame $modify.style 
frame $modify.styleparam 
frame $modify.tool 
frame $modify.toolparam 
frame $modify.control 
frame $modify.controlparam 

# Tool variable initialization
# in the real thing these will be inherited from microscope - i think

# traced by C code so it knows there are new modify parameters
set accepted_modify_params 0

set modifyp_mode 1
set modifyp_control 0
set modifyp_style 0
set modifyp_tool 0
set modifyp_constr_xyz_mode 0

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

trace variable modifyp_mode w "updateFromC modifyp_mode "
trace variable modifyp_control w "updateFromC modifyp_control "
trace variable modifyp_style w "updateFromC modifyp_style "
trace variable modifyp_tool w "updateFromC modifyp_tool "

trace variable modifyp_setpoint w "updateFromC modifyp_setpoint "
trace variable modifyp_p_gain w "updateFromC modifyp_p_gain "
trace variable modifyp_i_gain w "updateFromC modifyp_i_gain "
trace variable modifyp_d_gain w "updateFromC modifyp_d_gain "
trace variable modifyp_amplitude w "updateFromC modifyp_amplitude "
trace variable modifyp_rate w "updateFromC modifyp_rate "

#trace variable modifyp_tri_size w "updateFromC modifyp_tri_size "
#trace variable modifyp_tri_speed w "updateFromC modifyp_tri_speed "
trace variable modifyp_sweep_width w "updateFromC modifyp_sweep_width "

trace variable modifyp_bot_delay w "updateFromC modifyp_bot_delay "
trace variable modifyp_top_delay w "updateFromC modifyp_top_delay "
trace variable modifyp_z_pull w "updateFromC modifyp_z_pull "
trace variable modifyp_punchdist w "updateFromC modifyp_punchdist "
trace variable modifyp_speed w "updateFromC modifyp_speed "
trace variable modifyp_watchdog w "updateFromC modifyp_watchdog "

trace variable modifyp_start_delay w "updateFromC modifyp_start_delay "
trace variable modifyp_z_start w "updateFromC modifyp_z_start "
trace variable modifyp_z_end w "updateFromC modifyp_z_end "
trace variable modifyp_z_pullback w "updateFromC modifyp_z_pullback "
trace variable modifyp_force_limit w "updateFromC modifyp_force_limit "
trace variable modifyp_fcdist w "updateFromC modifyp_fcdist "
trace variable modifyp_num_layers w "updateFromC modifyp_num_layers "
trace variable modifyp_num_hcycles w "updateFromC modifyp_num_hcycles "
trace variable modifyp_sample_speed w "updateFromC modifyp_sample_speed "
trace variable modifyp_pullback_speed w "updateFromC modifyp_pullback_speed "
trace variable modifyp_start_speed w "updateFromC modifyp_start_speed "
trace variable modifyp_feedback_speed w "updateFromC modifyp_feedback_speed "
trace variable modifyp_avg_num w "updateFromC modifyp_avg_num "
trace variable modifyp_sample_delay w "updateFromC modifyp_sample_delay "
trace variable modifyp_pullback_delay w "updateFromC modifyp_pullback_delay "
trace variable modifyp_feedback_delay w "updateFromC modifyp_feedback_delay "

trace variable modifyp_step_size w "updateFromC modifyp_step_size "

trace variable modifyp_max_z_step w "updateFromC modifyp_max_z_step "
trace variable modifyp_max_xy_step w "updateFromC modifyp_max_xy_step "
trace variable modifyp_min_z_setpoint w "updateFromC modifyp_min_z_setpoint "
trace variable modifyp_max_z_setpoint w "updateFromC modifyp_max_z_setpoint "
trace variable modifyp_max_lat_setpoint w "updateFromC modifyp_max_lat_setpoint "


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
    -side left -padx 4 -fill both

#setup Modify mode box
label $modify.mode.label -text "Modify Mode" 
pack $modify.mode.label -side top -anchor nw
radiobutton $modify.mode.oscillating -text "Oscillating" -variable newmodifyp_mode -value 0 -anchor nw
radiobutton $modify.mode.contact -text "Contact" -variable newmodifyp_mode -value 1 -anchor nw
button $modify.mode.accept -text "Accept"  \
	-command "acceptModifyVars $modifyplist"
button $modify.mode.cancel -text "Revert"  \
	-command "cancelModifyVars $modifyplist"

checkbutton $modify.mode.relaxcomp -text "Relax Comp on" -variable doRelaxComp \
	

pack $modify.mode.oscillating $modify.mode.contact -side top -fill x
pack $modify.mode.relaxcomp -side top -fill x -pady 20

pack $modify.mode.cancel $modify.mode.accept -side bottom -fill x


#setup Modify modeparam box
label $modify.modeparam.label -text "Mode parameters" 
pack $modify.modeparam.label -side top -anchor nw

generic_entry $modify.modeparam.setpoint newmodifyp_setpoint \
	"Setpoint(-70 70)" real
generic_entry $modify.modeparam.p-gain newmodifyp_p_gain "P-Gain (0,5)" real 
generic_entry $modify.modeparam.i-gain newmodifyp_i_gain "I-Gain (0,2)" real 
generic_entry $modify.modeparam.d-gain newmodifyp_d_gain "D-Gain (0,5)" real 
generic_entry $modify.modeparam.rate newmodifyp_rate "Rate (0.1,50.0 uM/sec)" real 
generic_entry $modify.modeparam.amplitude newmodifyp_amplitude \
	"Amplitude (0,1)" real 

iwidgets::Labeledwidget::alignlabels \
    $modify.modeparam.setpoint $modify.modeparam.p-gain \
    $modify.modeparam.i-gain $modify.modeparam.d-gain \
    $modify.modeparam.rate \
    $modify.modeparam.amplitude 

pack    $modify.modeparam.setpoint $modify.modeparam.p-gain \
	$modify.modeparam.i-gain $modify.modeparam.d-gain \
        $modify.modeparam.rate \
	-side top -fill x -pady $fspady
if {$newmodifyp_mode == 0} {
  pack $modify.modeparam.amplitude -side top -fill x 
}
set mod_oscillating_list "$modify.modeparam.amplitude"

#setup Modify style box
label $modify.style.label -text "Style" 
pack $modify.style.label -side top -anchor nw
radiobutton $modify.style.sharp -text "Sharp" -variable newmodifyp_style \
	-value 0 -anchor nw
#radiobutton $modify.style.blunt -text "Blunt" -variable newmodifyp_style \
#	-value 1 -anchor nw

radiobutton $modify.style.sweep -text "Sweep" -variable newmodifyp_style \
	-value 2 -anchor nw
radiobutton $modify.style.sewing -text "Sewing" -variable newmodifyp_style \
	-value 3 -anchor nw
radiobutton $modify.style.forcecurve -text "ForceCurve" \
	-variable newmodifyp_style -value 4  -anchor nw
pack $modify.style.sharp $modify.style.sweep $modify.style.sewing \
	$modify.style.forcecurve -side top -fill x

#setup Modify styleparam box
label $modify.styleparam.label -text "Style parameters" 
pack $modify.styleparam.label -side top -anchor nw

#generic_entry $modify.styleparam.tri-size newmodifyp_tri_size "Tri Size" real 
#generic_entry $modify.styleparam.tri-speed newmodifyp_tri_speed "Tri Speed" real 

generic_entry $modify.styleparam.sweepwidth newmodifyp_sweep_width\
	"Sweep Width (0,1000 nm)" real 

generic_entry $modify.styleparam.bot-delay newmodifyp_bot_delay \
	"Bottom Delay (0,10)" real 
generic_entry $modify.styleparam.top-delay newmodifyp_top_delay \
	"Top Delay (0,10)" real 
generic_entry $modify.styleparam.z-pull newmodifyp_z_pull \
	"Pullout Height (10,1000 nm)" real 
generic_entry $modify.styleparam.punchdist newmodifyp_punchdist \
	"Punch Dist. (0,100 nm)" real 
generic_entry $modify.styleparam.speed newmodifyp_speed \
	"Speed (0,1000)" real 
generic_entry $modify.styleparam.watchdog newmodifyp_watchdog \
	"Watchdog Dist. (50,500 nm)" real 

#generic_entry $modify.styleparam.start-delay newmodifyp_start_delay \
#	"Start Delay (0,200 us)" real 
generic_entry $modify.styleparam.z-start newmodifyp_z_start \
	"Start Height (-1000,1000 nm)" real 
generic_entry $modify.styleparam.z-end newmodifyp_z_end \
	"End Height (-1000,1000 nm)" real 
generic_entry $modify.styleparam.z-pullback newmodifyp_z_pullback \
	"Pullout Height (-1000,0 nm)" real 
generic_entry $modify.styleparam.force-limit newmodifyp_force_limit\
	 "Force limit (0,1000 nA)" real 
generic_entry $modify.styleparam.forcecurvedist newmodifyp_fcdist \
	"F.C. Dist. (0,500 nm)" real 
generic_entry $modify.styleparam.num-layers newmodifyp_num_layers \
	"# samples (1,100)" real 
generic_entry $modify.styleparam.num-halfcycles newmodifyp_num_hcycles \
	"# half cycles (1,4)" real 
generic_entry $modify.styleparam.sample-speed newmodifyp_sample_speed "sample speed (0,10 um/s)" real 
generic_entry $modify.styleparam.pullback-speed	newmodifyp_pullback_speed "pullback speed (0,100 um/s)" real 
generic_entry $modify.styleparam.start-speed newmodifyp_start_speed "start speed (0,10 um/s)" real 
generic_entry $modify.styleparam.fdback-speed newmodifyp_feedback_speed "feedback speed (0,100 um/s)" real 
generic_entry $modify.styleparam.avg-num newmodifyp_avg_num "averaging (1,10)" real 
#generic_entry $modify.styleparam.sample-delay newmodifyp_sample_delay "sample delay (0,1000 us)" real 
#generic_entry $modify.styleparam.pullback-delay newmodifyp_pullback_delay "pullback delay (0,1000 us)" real 
#generic_entry $modify.styleparam.feedback-delay newmodifyp_feedback_delay "feedback delay (0,1000 us)" real 

#set mod_blunt_list "$modify.styleparam.tri-size $modify.styleparam.tri-speed"
set mod_sweep_list "$modify.styleparam.sweepwidth"

set mod_sewing_list "$modify.styleparam.bot-delay \
	$modify.styleparam.top-delay $modify.styleparam.z-pull \
        $modify.styleparam.punchdist $modify.styleparam.speed \
	$modify.styleparam.watchdog"
eval [concat iwidgets::Labeledwidget::alignlabels $mod_sewing_list]
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
eval [concat iwidgets::Labeledwidget::alignlabels $mod_forcecurve_list]

#setup Modify tool box
label $modify.tool.label -text "Tool" 
pack $modify.tool.label -side top -anchor nw
radiobutton $modify.tool.freehand -text "Freehand" -variable newmodifyp_tool \
	-value 0   -anchor nw
radiobutton $modify.tool.line -text "Line" -variable newmodifyp_tool \
	-value 1   -anchor nw
radiobutton $modify.tool.constrfree -text "Constr. Free" -variable newmodifyp_tool \
	-value 2   -anchor nw
radiobutton $modify.tool.constrfree_xyz -text "Constr. Free XYZ" \
	-variable newmodifyp_tool -value 3 -anchor nw 
radiobutton $modify.tool.slow_line -text "Slow Line" -variable newmodifyp_tool \
	-value 4   -anchor nw
radiobutton $modify.tool.slow_line_3d -text "Slow Line 3D"\
	-variable newmodifyp_tool -value 5 -anchor nw
pack $modify.tool.freehand $modify.tool.line $modify.tool.constrfree \
	$modify.tool.constrfree_xyz $modify.tool.slow_line \
	$modify.tool.slow_line_3d -side top -fill x 

#setup Modify toolparam box
label $modify.toolparam.label -text "Tool parameters" 
pack $modify.toolparam.label -side top -anchor nw

generic_entry $modify.toolparam.step-size newmodifyp_step_size \
	"Step Size (0,5 nm)" real 
set mod_line_list  "$modify.toolparam.step-size"
set mod_slow_line_list "$modify.toolparam.step-size"

#setup Constr. Freehand XYZ params box
radiobutton $modify.toolparam.line -text "Line Mode" \
	-variable modifyp_constr_xyz_mode -value 0 -anchor nw
radiobutton $modify.toolparam.plane -text "Plane Mode" \
	-variable modifyp_constr_xyz_mode -value 1 -anchor nw
set constr_xyz_param_list "$modify.toolparam.line $modify.toolparam.plane"

#setup Modify control box
label $modify.control.label -text "Control" 
pack $modify.control.label -side top -anchor nw
radiobutton $modify.control.feedback -text "Feedback" \
    -variable newmodifyp_control -value 0  -anchor nw
radiobutton $modify.control.directz -text "Direct Z" \
    -variable newmodifyp_control -value 1   -anchor nw
pack $modify.control.feedback $modify.control.directz -side top -fill x 
set mod_control_list "$modify.control.feedback $modify.control.directz"

#setup Modify controlparam box
label $modify.controlparam.label -text "Control parameters" 
pack $modify.controlparam.label -side top -anchor nw

generic_entry $modify.controlparam.max_z_step newmodifyp_max_z_step \
	"max_z_step (0,5 nm)" real 
generic_entry $modify.controlparam.max_xy_step newmodifyp_max_xy_step \
	"max_xy_step (0,5 nm)" real 
generic_entry $modify.controlparam.min_z_setpoint newmodifyp_min_z_setpoint \
	"min_z_setpoint (-70,70 nA)" real 
generic_entry $modify.controlparam.max_z_setpoint newmodifyp_max_z_setpoint \
	"max_z_setpoint (0,70 nA)" real 
generic_entry $modify.controlparam.max_lat_setpoint newmodifyp_max_lat_setpoint \
	"max_lat_setpoint (0,70 nA)" real 

set mod_directz_list  "$modify.controlparam.max_z_step $modify.controlparam.max_xy_step $modify.controlparam.min_z_setpoint $modify.controlparam.max_z_setpoint $modify.controlparam.max_lat_setpoint"
eval [concat iwidgets::Labeledwidget::alignlabels $mod_directz_list]

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
	# Magic number 6 = number of widgets to leave alone + 1
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
    global mod_line_list mod_slow_line_list mod_control_list \
	    constr_xyz_param_list
    global fspady
    global newmodifyp_control

    upvar $mod_tool k

    if {$k==0} {
        # selected freehand
	set plist [lrange [pack slaves $modify.toolparam] 1 end] 
	foreach widg $plist {pack forget $widg} 
	foreach widg $mod_control_list {pack forget $widg}
	foreach widg $mod_control_list {pack $widg -side top -fill x}
    } elseif {$k==1} {
	# selected line
	set plist [lrange [pack slaves $modify.toolparam] 1 end] 
	foreach widg $plist {pack forget $widg}
	foreach widg $mod_line_list {pack $widg -side top -fill x -pady $fspady}
	foreach widg $mod_control_list {pack forget $widg}
	foreach widg $mod_control_list {pack $widg -side top -fill x}
    } elseif {$k==2} {
	# selected constrained freehand
	set plist [lrange [pack slaves $modify.toolparam] 1 end] 
	foreach widg $plist {pack forget $widg}
	foreach widg $mod_control_list {pack forget $widg}
	foreach widg $mod_control_list {pack $widg -side top -fill x}
    } elseif {$k==3} {
       # selected constrained freehand xyz
	set plist [lrange [pack slaves $modify.toolparam] 1 end] 
	foreach widg $plist {pack forget $widg}
        foreach widg $mod_control_list {pack forget $widg}
        foreach widg $constr_xyz_param_list {pack $widg -side top -fill x}
        foreach widg $mod_control_list {pack $widg -side top -fill x}
    } elseif {$k==4} {
	# selected slow line
	set plist [lrange [pack slaves $modify.toolparam] 1 end] 
	foreach widg $plist {pack forget $widg}
	foreach widg $mod_slow_line_list {pack $widg -side top -fill x -pady $fspady}
	foreach widg $mod_control_list {pack forget $widg}
	foreach widg $mod_control_list {pack $widg -side top -fill x}
    } elseif {$k==5} {
	# selected slow line 3d
	# anyone want to make this more elegant?
	foreach widg $mod_control_list {pack forget $widg}
	foreach widg $mod_control_list {pack $widg -side top -fill x}
	foreach widg $constr_xyz_param_list {pack forget $widg}
	set plist [lrange [pack slaves $modify.control] 1 1]
	foreach widg $plist {pack forget $widg}
	set newmodifyp_control 1
	# show the modify live controls
	show.modify_live
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
    global save_bg

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
    $modify.mode.accept configure -background $save_bg
    $modify.mode.cancel configure -background $save_bg

    #close the window when the Accept Button is pressed
    #wm withdraw $modify
}


proc cancelModifyVars {varlist} {

    global modify
    global save_bg

    foreach val $varlist {
	global modifyp_$val
	global newmodifyp_$val
	if {[set newmodifyp_$val] != [set modifyp_$val] } {
	    set newmodifyp_$val [set modifyp_$val]
	}
    }
    # None of the newmodify_* vars are now changed
    $modify.mode.accept configure -background $save_bg
    $modify.mode.cancel configure -background $save_bg
}



#
#
######################
# LIVE Modify controls
# Controls you need access to _during_ a modification.
# The most important - slow line controls
#
set modify_live [create_closing_toplevel modify_live "Live modify controls"]
iwidgets::Labeledframe $modify_live.slow_line -labeltext "Slow Line" \
	-labelpos nw
set nmInfo(ml_slow_line) [$modify_live.slow_line childsite]
pack $modify_live.slow_line -fill both -expand yes

set slow_line_playing 0
button $nmInfo(ml_slow_line).slow_line_play -text "Play" -command {
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
    global slow_line_playing nmInfo
    if {$slow_line_playing} {
	$nmInfo(ml_slow_line).slow_line_play configure -text "Pause"
    } else {
	$nmInfo(ml_slow_line).slow_line_play configure -text "Play"
    }
}
trace variable slow_line_playing w config_play_name

button $nmInfo(ml_slow_line).slow_line_step -text "Step" -command {
    set slow_line_step 1
  }

set slow_line_direction 0
radiobutton $nmInfo(ml_slow_line).slow_line_forward -text "Forward" \
	-variable slow_line_direction -value 0 
radiobutton $nmInfo(ml_slow_line).slow_line_reverse -text "Reverse" \
	-variable slow_line_direction -value 1 

generic_entry $nmInfo(ml_slow_line).step-size modifyp_step_size \
	"Step Size (0,5 nm)" real 

pack $nmInfo(ml_slow_line).slow_line_play \
	$nmInfo(ml_slow_line).slow_line_step \
	$nmInfo(ml_slow_line).slow_line_forward \
	$nmInfo(ml_slow_line).slow_line_reverse \
	$nmInfo(ml_slow_line).step-size \
	-side top -pady 2 -anchor nw

iwidgets::Labeledframe $modify_live.directz -labeltext "Direct Z" \
	-labelpos nw
set nmInfo(ml_directz) [$modify_live.directz childsite]
pack $modify_live.directz -fill both -expand yes

generic_entry $nmInfo(ml_directz).directz_force_scale directz_force_scale\
	"Force scale (0,3)" real

pack $nmInfo(ml_directz).directz_force_scale -side top -pady 2 -anchor nw





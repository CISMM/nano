#/*===3rdtech===
#  Copyright (c) 2000 by 3rdTech, Inc.
#  All Rights Reserved.
#
#  This file may not be distributed without the permission of 
#  3rdTech, Inc. 
#  ===3rdtech===*/
#
# for widgets that change behavior of modify mode
#
set nmInfo(modify) [create_closing_toplevel modify "Modify parameters"]
set nmInfo(modifyquick) [frame $nmInfo(modify).quick]
set nmInfo(modifyfull) [frame $nmInfo(modify).full]

# Button swaps between quick and full param frames.  
set modify_quick_or_full "quick"
button $nmInfo(modify).quick_or_full -text "Full params" -command {
    global modify_quick_or_full nmInfo
    if {$modify_quick_or_full == "quick"} {
        pack forget $nmInfo(modifyquick)
        pack $nmInfo(modifyfull) -side top -expand yes -fill both
        set modify_quick_or_full "full"
        $nmInfo(modify).quick_or_full configure -text "Quick params"
    } else {
        pack forget $nmInfo(modifyfull)
        pack $nmInfo(modifyquick) -side top -expand yes -fill both
        set modify_quick_or_full "quick"
        $nmInfo(modify).quick_or_full configure -text "Full params"
    }
}
pack $nmInfo(modify).quick_or_full -side top -anchor nw

pack $nmInfo(modifyquick) -side top -expand yes -fill both

set nmInfo(modifydisplay) [frame $nmInfo(modify).bottom]
pack $nmInfo(modifydisplay) -side bottom -fill x
#------------------
# Quick modify controls. Changes to these controls take effect 
# immediately, but can't change modes. 
set nmInfo(modifypage) [frame $nmInfo(modifyquick).middle]
iwidgets::Labeledframe $nmInfo(modifyquick).modifystate -labeltext "Modify state" \
	-labelpos nw
pack $nmInfo(modifyquick).modifystate $nmInfo(modifypage) \
        -side top -fill x 

set nmInfo(modifystate) [$nmInfo(modifyquick).modifystate childsite]
generic_entry $nmInfo(modifypage).setpoint modifyp_setpoint \
	"Setpoint (0,100%)" real \
        { set accepted_modify_params 1 }
generic_entry $nmInfo(modifypage).p-gain modifyp_p_gain "P-Gain (0,5)" real \
        { set accepted_modify_params 1 }
generic_entry $nmInfo(modifypage).i-gain modifyp_i_gain "I-Gain (0,5)" real \
        { set accepted_modify_params 1 }
generic_entry $nmInfo(modifypage).d-gain modifyp_d_gain "D-Gain (0,5)" real \
        { set accepted_modify_params 1 }
generic_entry $nmInfo(modifypage).rate modifyp_rate "Rate (um/sec)" real \
        { set accepted_modify_params 1 }

	
pack    $nmInfo(modifypage).setpoint $nmInfo(modifypage).p-gain \
	$nmInfo(modifypage).i-gain $nmInfo(modifypage).d-gain \
	$nmInfo(modifypage).rate \
	-side top -anchor nw

iwidgets::Labeledwidget::alignlabels \
	$nmInfo(modifypage).setpoint $nmInfo(modifypage).p-gain \
	$nmInfo(modifypage).i-gain $nmInfo(modifypage).d-gain \
	$nmInfo(modifypage).rate 

label $nmInfo(modifystate).mod_mode -text "Contact"
label $nmInfo(modifystate).mod_style -text "Sharp"
label $nmInfo(modifystate).mod_tool -text "Freehand"
label $nmInfo(modifystate).mod_control -text "Feedback"

proc show_mode {name path nm element op} {
    upvar #0 $name mode
    if { $mode == 0 } {
	#oscillating mode
	$path configure -text "Oscillate"
    } elseif { $mode == 1 } {
	#contact mode
	$path configure -text "Contact"
    } 
}

proc show_style {name path nm element op} {
    upvar #0 $name style
    if { $style == 0 } {
	#Sharp style
	$path configure -text "Sharp"
	#  elseif  $style == 1 
	#blunt style Obsolete
    } elseif { $style == 2 } {
	#sweep style
	$path configure -text "Sweep"
    } elseif { $style == 3 } {
	#Sewing style
	$path configure -text "Sewing"
    } elseif { $style == 4 } {
	# force curvestyle
	$path configure -text "Force Curve"
    }
}

proc show_tool {name path nm element op} {
    upvar #0 $name tool
    if { $tool == 0 } {
	#Freehand tool
	$path configure -text "Freehand"
    } elseif { $tool == 1 } {
	#line tool
	$path configure -text "Line"
    } elseif { $tool == 2 } {
	#Constrained freehand tool
	$path configure -text "Constr. Free"
    } elseif { $tool == 3 } {
	# Slow line tool
	$path configure -text "Slow Line"
    }
}

proc show_control {name path nm element op} {
    upvar #0 $name control
    if { $control == 0 } {
	#oscillating control
	$path configure -text "Feedback"
    } elseif { $control == 1 } {
	#contact control
	$path configure -text "Direct Z"
    } 
}

trace variable imagep_mode w \
	"show_mode imagep_mode $nmInfo(imagestate).image_mode"

trace variable modifyp_mode w \
	"show_mode modifyp_mode $nmInfo(modifystate).mod_mode"
trace variable modifyp_style w \
	"show_style modifyp_style $nmInfo(modifystate).mod_style"
trace variable modifyp_tool w \
	"show_tool modifyp_tool $nmInfo(modifystate).mod_tool"
trace variable modifyp_control w \
	"show_control modifyp_control $nmInfo(modifystate).mod_control"

pack $nmInfo(modifystate).mod_control -side bottom -anchor nw
pack $nmInfo(modifystate).mod_mode $nmInfo(modifystate).mod_style \
	$nmInfo(modifystate).mod_tool \
	-side left -anchor nw

label $nmInfo(modifydisplay).mod_mode -text "Modify Display"
pack $nmInfo(modifydisplay).mod_mode -side top -anchor nw -pady 4

set number_of_markers_shown 500
set marker_height 100
generic_entry $nmInfo(modifydisplay).num_mod_markers number_of_markers_shown \
	"Num. Markers" real
generic_entry $nmInfo(modifydisplay).mod_marker_height marker_height \
	"Marker Height (nm)" real

iwidgets::Labeledwidget::alignlabels \
	$nmInfo(modifydisplay).num_mod_markers \
	$nmInfo(modifydisplay).mod_marker_height
	

button $nmInfo(modifydisplay).modmarkclr -text "Clear Markers" \
	-command "set term_input C"

pack $nmInfo(modifydisplay).num_mod_markers \
	$nmInfo(modifydisplay).mod_marker_height \
	$nmInfo(modifydisplay).modmarkclr \
	-side top -anchor nw

#------------------
# Full modify controls, allow switch between any mode using 
# Accept and Revert buttons.

frame $nmInfo(modifyfull).mode 
frame $nmInfo(modifyfull).modeparam 
frame $nmInfo(modifyfull).style 
frame $nmInfo(modifyfull).styleparam 
frame $nmInfo(modifyfull).tool 
frame $nmInfo(modifyfull).toolparam 
frame $nmInfo(modifyfull).control 
frame $nmInfo(modifyfull).controlparam 

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
pack $nmInfo(modifyfull).mode $nmInfo(modifyfull).modeparam $nmInfo(modifyfull).style $nmInfo(modifyfull).styleparam \
    $nmInfo(modifyfull).tool $nmInfo(modifyfull).toolparam $nmInfo(modifyfull).control $nmInfo(modifyfull).controlparam \
    -side left -padx 4 -fill both

#setup Modify mode box
label $nmInfo(modifyfull).mode.label -text "Modify Mode" 
pack $nmInfo(modifyfull).mode.label -side top -anchor nw
radiobutton $nmInfo(modifyfull).mode.oscillating -text "Oscillating" -variable newmodifyp_mode -value 0 -anchor nw
radiobutton $nmInfo(modifyfull).mode.contact -text "Contact" -variable newmodifyp_mode -value 1 -anchor nw
button $nmInfo(modifyfull).mode.accept -text "Accept"  \
	-command "acceptModifyVars $modifyplist"
button $nmInfo(modifyfull).mode.cancel -text "Revert"  \
	-command "cancelModifyVars $modifyplist"

checkbutton $nmInfo(modifyfull).mode.relaxcomp -text "Relax Comp on" -variable doRelaxComp \
	

pack $nmInfo(modifyfull).mode.oscillating $nmInfo(modifyfull).mode.contact -side top -fill x
pack $nmInfo(modifyfull).mode.relaxcomp -side top -fill x -pady 20

pack $nmInfo(modifyfull).mode.cancel $nmInfo(modifyfull).mode.accept -side bottom -fill x


#setup Modify modeparam box
label $nmInfo(modifyfull).modeparam.label -text "Mode parameters" 
pack $nmInfo(modifyfull).modeparam.label -side top -anchor nw

generic_entry $nmInfo(modifyfull).modeparam.setpoint newmodifyp_setpoint \
	"Setpoint(-70 70)" real
generic_entry $nmInfo(modifyfull).modeparam.p-gain newmodifyp_p_gain "P-Gain (0,5)" real 
generic_entry $nmInfo(modifyfull).modeparam.i-gain newmodifyp_i_gain "I-Gain (0,2)" real 
generic_entry $nmInfo(modifyfull).modeparam.d-gain newmodifyp_d_gain "D-Gain (0,5)" real 
generic_entry $nmInfo(modifyfull).modeparam.rate newmodifyp_rate "Rate (0.1,50.0 uM/sec)" real 
generic_entry $nmInfo(modifyfull).modeparam.amplitude newmodifyp_amplitude \
	"Amplitude (0,1)" real 

iwidgets::Labeledwidget::alignlabels \
    $nmInfo(modifyfull).modeparam.setpoint $nmInfo(modifyfull).modeparam.p-gain \
    $nmInfo(modifyfull).modeparam.i-gain $nmInfo(modifyfull).modeparam.d-gain \
    $nmInfo(modifyfull).modeparam.rate \
    $nmInfo(modifyfull).modeparam.amplitude 

pack    $nmInfo(modifyfull).modeparam.setpoint $nmInfo(modifyfull).modeparam.p-gain \
	$nmInfo(modifyfull).modeparam.i-gain $nmInfo(modifyfull).modeparam.d-gain \
        $nmInfo(modifyfull).modeparam.rate \
	-side top -fill x -pady $fspady
if {$newmodifyp_mode == 0} {
  pack $nmInfo(modifyfull).modeparam.amplitude -side top -fill x 
}
set mod_oscillating_list "$nmInfo(modifyfull).modeparam.amplitude"

#setup Modify style box
label $nmInfo(modifyfull).style.label -text "Style" 
pack $nmInfo(modifyfull).style.label -side top -anchor nw
radiobutton $nmInfo(modifyfull).style.sharp -text "Sharp" -variable newmodifyp_style \
	-value 0 -anchor nw
#radiobutton $nmInfo(modifyfull).style.blunt -text "Blunt" -variable newmodifyp_style \
#	-value 1 -anchor nw

radiobutton $nmInfo(modifyfull).style.sweep -text "Sweep" -variable newmodifyp_style \
	-value 2 -anchor nw
radiobutton $nmInfo(modifyfull).style.sewing -text "Sewing" -variable newmodifyp_style \
	-value 3 -anchor nw
radiobutton $nmInfo(modifyfull).style.forcecurve -text "ForceCurve" \
	-variable newmodifyp_style -value 4  -anchor nw
pack $nmInfo(modifyfull).style.sharp $nmInfo(modifyfull).style.sweep $nmInfo(modifyfull).style.sewing \
	$nmInfo(modifyfull).style.forcecurve -side top -fill x

#setup Modify styleparam box
label $nmInfo(modifyfull).styleparam.label -text "Style parameters" 
pack $nmInfo(modifyfull).styleparam.label -side top -anchor nw

#generic_entry $nmInfo(modifyfull).styleparam.tri-size newmodifyp_tri_size "Tri Size" real 
#generic_entry $nmInfo(modifyfull).styleparam.tri-speed newmodifyp_tri_speed "Tri Speed" real 

generic_entry $nmInfo(modifyfull).styleparam.sweepwidth newmodifyp_sweep_width\
	"Sweep Width (0,1000 nm)" real 

generic_entry $nmInfo(modifyfull).styleparam.bot-delay newmodifyp_bot_delay \
	"Bottom Delay (0,10)" real 
generic_entry $nmInfo(modifyfull).styleparam.top-delay newmodifyp_top_delay \
	"Top Delay (0,10)" real 
generic_entry $nmInfo(modifyfull).styleparam.z-pull newmodifyp_z_pull \
	"Pullout Height (10,1000 nm)" real 
generic_entry $nmInfo(modifyfull).styleparam.punchdist newmodifyp_punchdist \
	"Punch Dist. (0,100 nm)" real 
generic_entry $nmInfo(modifyfull).styleparam.speed newmodifyp_speed \
	"Speed (0,1000)" real 
generic_entry $nmInfo(modifyfull).styleparam.watchdog newmodifyp_watchdog \
	"Watchdog Dist. (50,500 nm)" real 

#generic_entry $nmInfo(modifyfull).styleparam.start-delay newmodifyp_start_delay \
#	"Start Delay (0,200 us)" real 
generic_entry $nmInfo(modifyfull).styleparam.z-start newmodifyp_z_start \
	"Start Height (-1000,1000 nm)" real 
generic_entry $nmInfo(modifyfull).styleparam.z-end newmodifyp_z_end \
	"End Height (-1000,1000 nm)" real 
generic_entry $nmInfo(modifyfull).styleparam.z-pullback newmodifyp_z_pullback \
	"Pullout Height (-1000,0 nm)" real 
generic_entry $nmInfo(modifyfull).styleparam.force-limit newmodifyp_force_limit\
	 "Force limit (0,1000 nA)" real 
generic_entry $nmInfo(modifyfull).styleparam.forcecurvedist newmodifyp_fcdist \
	"F.C. Dist. (0,500 nm)" real 
generic_entry $nmInfo(modifyfull).styleparam.num-layers newmodifyp_num_layers \
	"# samples (1,100)" real 
generic_entry $nmInfo(modifyfull).styleparam.num-halfcycles newmodifyp_num_hcycles \
	"# half cycles (1,4)" real 
generic_entry $nmInfo(modifyfull).styleparam.sample-speed newmodifyp_sample_speed "sample speed (0,10 um/s)" real 
generic_entry $nmInfo(modifyfull).styleparam.pullback-speed	newmodifyp_pullback_speed "pullback speed (0,100 um/s)" real 
generic_entry $nmInfo(modifyfull).styleparam.start-speed newmodifyp_start_speed "start speed (0,10 um/s)" real 
generic_entry $nmInfo(modifyfull).styleparam.fdback-speed newmodifyp_feedback_speed "feedback speed (0,100 um/s)" real 
generic_entry $nmInfo(modifyfull).styleparam.avg-num newmodifyp_avg_num "averaging (1,10)" real 
#generic_entry $nmInfo(modifyfull).styleparam.sample-delay newmodifyp_sample_delay "sample delay (0,1000 us)" real 
#generic_entry $nmInfo(modifyfull).styleparam.pullback-delay newmodifyp_pullback_delay "pullback delay (0,1000 us)" real 
#generic_entry $nmInfo(modifyfull).styleparam.feedback-delay newmodifyp_feedback_delay "feedback delay (0,1000 us)" real 

#set mod_blunt_list "$nmInfo(modifyfull).styleparam.tri-size $nmInfo(modifyfull).styleparam.tri-speed"
set mod_sweep_list "$nmInfo(modifyfull).styleparam.sweepwidth"
set mod_sewing_list "$nmInfo(modifyfull).styleparam.bot-delay \
	$nmInfo(modifyfull).styleparam.top-delay $nmInfo(modifyfull).styleparam.z-pull \
        $nmInfo(modifyfull).styleparam.punchdist $nmInfo(modifyfull).styleparam.speed \
	$nmInfo(modifyfull).styleparam.watchdog"
eval [concat iwidgets::Labeledwidget::alignlabels $mod_sewing_list]
set mod_forcecurve_list " \
	$nmInfo(modifyfull).styleparam.z-start $nmInfo(modifyfull).styleparam.z-end \
	$nmInfo(modifyfull).styleparam.z-pullback $nmInfo(modifyfull).styleparam.force-limit \
	$nmInfo(modifyfull).styleparam.forcecurvedist $nmInfo(modifyfull).styleparam.num-layers \
	$nmInfo(modifyfull).styleparam.num-halfcycles \
	$nmInfo(modifyfull).styleparam.sample-speed $nmInfo(modifyfull).styleparam.pullback-speed \
	$nmInfo(modifyfull).styleparam.start-speed $nmInfo(modifyfull).styleparam.fdback-speed \
	$nmInfo(modifyfull).styleparam.avg-num "
#	$nmInfo(modifyfull).styleparam.start-delay \
#	$nmInfo(modifyfull).styleparam.sample-delay \
#	$nmInfo(modifyfull).styleparam.pullback-delay $nmInfo(modifyfull).styleparam.feedback-delay"
eval [concat iwidgets::Labeledwidget::alignlabels $mod_forcecurve_list]

#setup Modify tool box
label $nmInfo(modifyfull).tool.label -text "Tool" 
pack $nmInfo(modifyfull).tool.label -side top -anchor nw
radiobutton $nmInfo(modifyfull).tool.freehand -text "Freehand" -variable newmodifyp_tool \
	-value 0   -anchor nw
radiobutton $nmInfo(modifyfull).tool.line -text "Line" -variable newmodifyp_tool \
	-value 1   -anchor nw
radiobutton $nmInfo(modifyfull).tool.constrfree -text "Constr. Free" -variable newmodifyp_tool \
	-value 2   -anchor nw
radiobutton $nmInfo(modifyfull).tool.slow_line -text "Slow Line" -variable newmodifyp_tool\
	-value 3   -anchor nw
pack $nmInfo(modifyfull).tool.freehand $nmInfo(modifyfull).tool.line $nmInfo(modifyfull).tool.constrfree \
	$nmInfo(modifyfull).tool.slow_line -side top -fill x 

#setup Modify toolparam box
label $nmInfo(modifyfull).toolparam.label -text "Tool parameters" 
pack $nmInfo(modifyfull).toolparam.label -side top -anchor nw

generic_entry $nmInfo(modifyfull).toolparam.step-size newmodifyp_step_size \
	"Step Size (0,5 nm)" real 
set mod_line_list  "$nmInfo(modifyfull).toolparam.step-size"
set mod_slow_line_list "$nmInfo(modifyfull).toolparam.step-size"

#setup Modify control box
label $nmInfo(modifyfull).control.label -text "Control" 
pack $nmInfo(modifyfull).control.label -side top -anchor nw
radiobutton $nmInfo(modifyfull).control.feedback -text "Feedback" \
    -variable newmodifyp_control -value 0  -anchor nw
radiobutton $nmInfo(modifyfull).control.directz -text "Direct Z" \
    -variable newmodifyp_control -value 1   -anchor nw
pack $nmInfo(modifyfull).control.feedback $nmInfo(modifyfull).control.directz -side top -fill x 

#setup Modify controlparam box
label $nmInfo(modifyfull).controlparam.label -text "Control parameters" 
pack $nmInfo(modifyfull).controlparam.label -side top -anchor nw

generic_entry $nmInfo(modifyfull).controlparam.max_z_step newmodifyp_max_z_step \
	"max_z_step (0,5 nm)" real 
generic_entry $nmInfo(modifyfull).controlparam.max_xy_step newmodifyp_max_xy_step \
	"max_xy_step (0,5 nm)" real 
generic_entry $nmInfo(modifyfull).controlparam.min_z_setpoint newmodifyp_min_z_setpoint \
	"min_z_setpoint (-70,70 nA)" real 
generic_entry $nmInfo(modifyfull).controlparam.max_z_setpoint newmodifyp_max_z_setpoint \
	"max_z_setpoint (0,70 nA)" real 
generic_entry $nmInfo(modifyfull).controlparam.max_lat_setpoint newmodifyp_max_lat_setpoint \
	"max_lat_setpoint (0,70 nA)" real 

set mod_directz_list  "$nmInfo(modifyfull).controlparam.max_z_step $nmInfo(modifyfull).controlparam.max_xy_step $nmInfo(modifyfull).controlparam.min_z_setpoint $nmInfo(modifyfull).controlparam.max_z_setpoint $nmInfo(modifyfull).controlparam.max_lat_setpoint"
eval [concat iwidgets::Labeledwidget::alignlabels $mod_directz_list]

#
#
######################
# Flip_* procedures
#   These change the displayed widgets, depending on the
# value of a global variable, i.e. which radiobutton is pressed.
#

# flips $nmInfo(modifyfull).modeparam widgets
proc flip_mod_mode {mod_mode element op} {
    global nmInfo
    global mod_oscillating_list
    global fspady

    upvar $mod_mode k

    if {$k==0} {
        # selected oscillating
	# Magic number 6 = number of widgets to leave alone + 1
	set plist [lrange [pack slaves $nmInfo(modifyfull).modeparam] 6 end] 
	foreach widg $plist {pack forget $widg} 
	foreach widg $mod_oscillating_list {pack $widg -side top -fill x -pady $fspady}
    } elseif {$k==1} {
	# selected contact
	set plist [lrange [pack slaves $nmInfo(modifyfull).modeparam] 6 end] 
	foreach widg $plist {pack forget $widg}
    }
}

# flips $nmInfo(modifyfull).styleparam widgets
proc flip_mod_style {mod_style element op} {
    global nmInfo
#    global mod_blunt_list
    global mod_sweep_list
    global mod_sewing_list
    global mod_forcecurve_list
    global fspady

    upvar $mod_style k

    if {$k==0} {
        # selected sharp
	set plist [lrange [pack slaves $nmInfo(modifyfull).styleparam] 1 end] 
	foreach widg $plist {pack forget $widg} 
#    } elseif {$k==1} {
#	# selected blunt
#	set plist [lrange [pack slaves $nmInfo(modifyfull).styleparam] 1 end] 
#	foreach widg $plist {pack forget $widg}
#	foreach widg $mod_blunt_list {pack $widg -side top -fill x -pady $fspady}
    } elseif {$k==2} {
	# selected sweep
	set plist [lrange [pack slaves $nmInfo(modifyfull).styleparam] 1 end] 
	foreach widg $plist {pack forget $widg}
	foreach widg $mod_sweep_list {pack $widg -side top -fill x -pady $fspady}
    } elseif {$k==3} {
	# selected sewing
	set plist [lrange [pack slaves $nmInfo(modifyfull).styleparam] 1 end] 
	foreach widg $plist {pack forget $widg}
	foreach widg $mod_sewing_list {pack $widg -side top -fill x -pady $fspady}
    } elseif {$k==4} {
	# selected force curve
	set plist [lrange [pack slaves $nmInfo(modifyfull).styleparam] 1 end]
	foreach widg $plist {pack forget $widg}
	foreach widg $mod_forcecurve_list {pack $widg -side top -fill x -pady $fspady}
    }
}

# flips $nmInfo(modifyfull).toolparam widgets
proc flip_mod_tool {mod_tool element op} {
    global nmInfo
    global mod_line_list mod_slow_line_list
    global fspady

    upvar $mod_tool k

    if {$k==0} {
        # selected freehand
	set plist [lrange [pack slaves $nmInfo(modifyfull).toolparam] 1 end] 
	foreach widg $plist {pack forget $widg} 
    } elseif {$k==1} {
	# selected line
	set plist [lrange [pack slaves $nmInfo(modifyfull).toolparam] 1 end] 
	foreach widg $plist {pack forget $widg}
	foreach widg $mod_line_list {pack $widg -side top -fill x -pady $fspady}
    } elseif {$k==2} {
	# selected constrained freehand
	set plist [lrange [pack slaves $nmInfo(modifyfull).toolparam] 1 end] 
	foreach widg $plist {pack forget $widg}
    } elseif {$k==3} {
	# selected slow line
	set plist [lrange [pack slaves $nmInfo(modifyfull).toolparam] 1 end] 
	foreach widg $plist {pack forget $widg}
	foreach widg $mod_slow_line_list {pack $widg -side top -fill x -pady $fspady}
    }
}

# flips $nmInfo(modifyfull).controlparam widgets
proc flip_mod_control {mod_control element op} {
    global nmInfo
    global mod_directz_list
    global fspady

    upvar $mod_control k

    if {$k==0} {
        # selected feedback
	set plist [lrange [pack slaves $nmInfo(modifyfull).controlparam] 1 end] 
	foreach widg $plist {pack forget $widg} 
    } elseif {$k==1} {
	# selected contact
	set plist [lrange [pack slaves $nmInfo(modifyfull).controlparam] 1 end] 
	foreach widg $plist {pack forget $widg}
	foreach widg $mod_directz_list {pack $widg -side top -fill x -pady $fspady}
    }
}

#
# Change the background of Accept and Cancel buttons
#    when you haven't yet committed your changes by clicking Accept
#

proc modBackgChReal {fooa element op} {
    global nmInfo

    $nmInfo(modifyfull).mode.accept configure -background LightPink1
    $nmInfo(modifyfull).mode.cancel configure -background LightPink1
}

#
# Bound to the accept button 
#
proc acceptModifyVars {varlist} {
    global accepted_modify_params
    global nmInfo
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
    $nmInfo(modifyfull).mode.accept configure -background $save_bg
    $nmInfo(modifyfull).mode.cancel configure -background $save_bg

    #close the window when the Accept Button is pressed
    #wm withdraw $modify
}


proc cancelModifyVars {varlist} {

    global nmInfo
    global save_bg

    foreach val $varlist {
	global modifyp_$val
	global newmodifyp_$val
	if {[set newmodifyp_$val] != [set modifyp_$val] } {
	    set newmodifyp_$val [set modifyp_$val]
	}
    }
    # None of the newmodify_* vars are now changed
    $nmInfo(modifyfull).mode.accept configure -background $save_bg
    $nmInfo(modifyfull).mode.cancel configure -background $save_bg
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





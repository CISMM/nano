#/*===3rdtech===
#  Copyright (c) 2000-2002 by 3rdTech, Inc.
#  All Rights Reserved.
#
#  This file may not be distributed without the permission of 
#  3rdTech, Inc. 
#  ===3rdtech===*/
#
# For widgets that change behavior of modify mode
#
# The following methods can be found in this file:
#   init_modify:                   sets up the .modify window
#   set_view:                      calls pack_quick or pack_full depending 
#                                  on $pack_quick_or_full
#   switch_view:                   switches from quick to full and vice versa
#                                  window resizing is done here
#   init_quick:                    initializes the quick view of .modify
#   flip_optimize_selection_mode:  does nothing right now
#   init_full:                     intializes the full view of .modify
#   pack_quick:                    packs the quick view of .modify
#   pack_full:                     packs the full view of .modify
#   change_made:                   called whenever a variable whose value could 
#                                  effect the display of .modify is changed
#   show_modify_live:              opens/closes modifylive window
#   set_enabling:                  changes enabling based on selected mode/style/tool/control
#   init_display:                  initializes the bottom part of .modify, 
#                                  and packs it
#   modBackgChReal:                changes the color of accept/revert button
#   acceptModifyVars:              bound to "Accept" button
#   cancelModifyVars:              bound to "Revert" button
#   config_play_name:              changes play to pause and vice versa on modify live window
#   init_live_controls:            initialize the live controls
#
#   modify_change_made:            when a modifyp_* variable changes, update corresponding
#                                  newmodifyp_* variable
#
#
#  Go to the end of the file to see the intialization calls.

# useful for debugging
proc errorOut {string} {puts $string}
##############################
# Sets up the .modify window #
##############################
proc init_modify {} {
    global nmInfo modify modify_quick_or_full

    # create .modify
    create_closing_toplevel modify "Modify Parameters"
    wm resizable .modify 0 0
    wm maxsize .modify [expr [winfo screenwidth .] - 50] [expr [winfo screenheight .] - 50]

    # create canvas for scrolling
    set nmInfo(canvas) [canvas .modify.modifyCanvas -xscrollcommand ".modify.scrollH set"]

    # create scrollbars
    set nmInfo(scrollH) [scrollbar .modify.scrollH -orient horizontal -command ".modify.modifyCanvas xview"]

    # add frame to canvas so that widgets can be added to frame
    set nmInfo(modify) [frame $nmInfo(canvas).modifyFrame -relief flat -bd 0]

    $nmInfo(canvas) create window 0 0 -window $nmInfo(canvas).modifyFrame -anchor nw
    # bind scrollbars to canvas
    bind $nmInfo(canvas).modifyFrame <Configure> {$nmInfo(canvas) configure -scrollregion \
						       [list 2 2 [expr %w - 2] [expr %h - 2]]}

    set nmInfo(modifyquick) [frame $nmInfo(modify).quick]
    set nmInfo(modifyfull) [frame $nmInfo(modify).full]

    pack $nmInfo(canvas) -side left -expand yes -fill both

    # Set to full so we can switch to quick immediately,
    #  so that .modify is sized for quick view
    set modify_quick_or_full "full"

    # Button swaps between quick and full param frames.
    button $nmInfo(modify).quick_or_full -text "Full params" -command {switch_view}
    pack $nmInfo(modify).quick_or_full -side top -anchor nw
}

######################################
# Called whenever .modify needs      #
# to be repacked (ie. when switching #
# views or rearranging full view).   #
######################################
proc set_view {} {
    global modify_quick_or_full nmInfo full_height full_width

    if {$modify_quick_or_full == "full"} {
	pack_full
	pack $nmInfo(modifyfull) -side left -expand no 
	
	# reset height in case it has changed 
	update idletasks

	# if the window has not been initialized, set the window size
	# to the regular size for what has been selected
	if {$full_width == 0} {
	    set full_width [expr [winfo reqwidth $nmInfo(modifyfull)]]
	}

	set full_height [expr [winfo reqheight $nmInfo(modifyfull)]\
		         + [winfo reqheight $nmInfo(modifydisplay)]\
			 + [winfo reqheight $nmInfo(modify).quick_or_full]\
			 + [winfo reqheight $nmInfo(scrollH)]]
	
	wm geometry .modify =[set full_width]x[set full_height]
    } else {
	pack_quick
        pack $nmInfo(modifyquick) -side top -expand yes -fill both
    }
}

########################################
# Bound to "Quick/Full" button.        # 
# Switches $modify_quick_or_full,      #
# resizes window, and copies variables #
# (if switching from full to quick).   #
########################################
proc switch_view {} {
    global modify_quick_or_full nmInfo full_width full_height save_bg collab_commands_suspended

    # Switching to full view
    if {$modify_quick_or_full == "quick"} {
	set modify_quick_or_full "full"

        pack forget $nmInfo(modifyquick)
	pack forget $nmInfo(canvas)
	pack $nmInfo(scrollH) -side bottom -fill x
	pack $nmInfo(canvas) -side left -expand yes -fill both

	# eval set_view

        $nmInfo(modify).quick_or_full configure -text "Quick params"

	# so that winfo will give the correct info
	update idletasks

	#set_window_size
	wm resizable .modify 1 1
	#if {$full_width == 0} {
	#    set full_width [expr [winfo reqwidth $nmInfo(modifyfull)]]
	#}

#	set full_height [expr [winfo reqheight $nmInfo(modifyfull)]\
\#		         + [winfo reqheight $nmInfo(modifydisplay)]\
	\#		 + [winfo reqheight $nmInfo(modify).quick_or_full]\
		\#	 + [winfo reqheight $nmInfo(scrollH)]]
	
	# wm geometry .modify =[set full_width]x[set full_height]

	eval set_view

	# Set button background to inactive
	$nmInfo(modifyfull).mode.accept configure -background $save_bg
	$nmInfo(modifyfull).mode.cancel configure -background $save_bg	

    } else {
	# Switching to quick view
	set modify_quick_or_full "quick"

	# reset newmodifyp_*, if you have mutex
	if { $collab_commands_suspended == 0 } {
	    global modifyplist
	    foreach var $modifyplist {
		global newmodifyp_$var modifyp_$var
		set newmodifyp_$var [set modifyp_$var]
	    }
	}
	
	# save full view size
	if {$full_width != 0 && $full_height != 0} {
	    set full_width [winfo width .modify]
	    set full_height [winfo height .modify]
	}

        pack forget $nmInfo(modifyfull)
	pack forget $nmInfo(scrollH)

	eval set_view

        $nmInfo(modify).quick_or_full configure -text "Full params"

	# so that winfo gives the correct info
	update idletasks

	# set window size
	wm geometry .modify =[winfo reqwidth $nmInfo(modifyquick)]x[expr [winfo reqheight $nmInfo(modifyquick)] \
			   + [winfo reqheight $nmInfo(modifydisplay)] + [winfo reqheight $nmInfo(modify).quick_or_full]] 

	wm resizable .modify  0 0
    }

    # This is a hack.
    # The window was losing focus every time this function
    # got called.  I think it has something to do with the
    # packing and unpacking, used above.  This fixes it, but
    # it ain't pretty.
    focus -force .modify
}

###########################################
# Initializes the quick view.             #
# Variables that are common to quick view #
# and full view get set here.             #
###########################################
proc init_quick {} {
    global nmInfo modifyp_setpoint modifyp_p_gain modifyp_i_gain \
	   modifyp_d_gain modifyp_rate newmodifyp_setpoint \
	   newmodifyp_p_gain newmodifyp_i_gain \
	   newmodifyp_d_gain newmodifyp_rate \
           device_only_controls

    # Variables dealt with in quick param window
    set modifyp_setpoint 1.0
    set modifyp_p_gain 1.0
    set modifyp_i_gain 0.5
    set modifyp_d_gain 0.01
    set modifyp_rate 20.0

    # Label at top of quick params window
    iwidgets::Labeledframe $nmInfo(modifyquick).modifystate -labeltext "Modify state" -labelpos nw
    set nmInfo(modifystate) [$nmInfo(modifyquick).modifystate childsite]
    label $nmInfo(modifystate).mod_mode -text "Mode:\n"
    label $nmInfo(modifystate).mod_style -text "Style:\n"
    label $nmInfo(modifystate).mod_tool -text "Tool:\n"
    label $nmInfo(modifystate).mod_control -text "Control:\n"

    # Quick modify controls. Changes to these controls take effect 
    # immediately, but can't change modes. 
    set nmInfo(modifypage) [frame $nmInfo(modifyquick).middle]
    pack $nmInfo(modifyquick).modifystate $nmInfo(modifypage) \
        -side top -fill x 

    generic_entry $nmInfo(modifypage).setpoint_nA modifyp_setpoint \
	"Setpoint (-64,64 nA)" real "set newmodifyp_setpoint \$modifyp_setpoint"
    generic_entry $nmInfo(modifypage).setpoint_pcnt modifyp_setpoint \
	"Setpoint (0,100 %)" real "set newmodifyp_setpoint \$modifyp_setpoint"
    generic_entry $nmInfo(modifypage).p-gain modifyp_p_gain \
	"P-Gain (0,5)" real "set newmodifyp_p_gain \$modifyp_p_gain"
    generic_entry $nmInfo(modifypage).i-gain modifyp_i_gain \
	"I-Gain (0,5)" real "set newmodifyp_i_gain \$modifyp_i_gain"
    generic_entry $nmInfo(modifypage).d-gain modifyp_d_gain \
	"D-Gain (0,5)" real "set newmodifyp_d_gain \$modifyp_d_gain"
    generic_entry $nmInfo(modifypage).rate modifyp_rate \
	 "Rate (um/sec)" real "set newmodifyp_rate \$modifyp_rate"

    iwidgets::Labeledwidget::alignlabels \
	$nmInfo(modifypage).setpoint_nA \
	$nmInfo(modifypage).setpoint_pcnt \
        $nmInfo(modifypage).p-gain \
	$nmInfo(modifypage).i-gain \
	$nmInfo(modifypage).d-gain \
	$nmInfo(modifypage).rate 

    eval lappend device_only_controls "$nmInfo(modifypage).setpoint_nA \
	$nmInfo(modifypage).setpoint_pcnt \
        $nmInfo(modifypage).p-gain \
	$nmInfo(modifypage).i-gain \
	$nmInfo(modifypage).d-gain \
	$nmInfo(modifypage).rate" 
}

###########################################
# Currently does nothing.  It is bound    #
# to the optimize_now variable.  I'm      #
# not sure what the author had originally #
# intended with this function, so I left  #
# it in the code.                         #
###########################################
proc flip_optimize_selection_mode {optimize_mode_param element op} {
    upvar $optimize_mode_param k
    global user_0_mode

    # This doesn't work! User can switch back to touch after clicking
    # Optimize Now tool.
    # AND it sets user_0_mode to touch incorrectly on startup. 
    if { $k==0 } {
	# use the touch tool for line selection
	#set user_0_mode 12
    } elseif { $k==1 } {
	# use the select tool for area selection
	#set user_0_mode 4
    }
}

####################################################
# Recursive procedure called by enforce_directz_heightplane
# to set the heightplane to Z Piezo. We have to wait
# for the AFM to respond so that Z Piezo is available to choose.
####################################################
proc set_directz_heightplane { plane_name { rec_level 0 } } {
    global z_comes_from inputPlaneNames nmInfo scandatalist data_sets
    global newmodifyp_control
    # Check to see if Z Piezo is available in height plane list. 
    set id [lsearch -exact $inputPlaneNames $plane_name]
    #puts "$plane_name next $rec_level"
    if { $id != -1 } {
        set response [tk_messageBox -type okcancel -title "Z Piezo Heightfield" \
                -parent .modify -default ok -message "
The Direct Z control requires that 
Z Piezo (Forward or Reverse) be displayed as the 
heightfield. 
Press OK to use $plane_name as the heightfield.
Press Cancel to resolve yourself." ]
        switch -- $response {
            ok {
                puts "DirectZ: Setting Z Piezo as heightplane"
                # Switch to viewing Z Piezo. 
                #Doesn't send change to C: 
                #$nmInfo(z_mapping).z_dataset select $id
                # but this works:
                set z_comes_from $plane_name
                return
            }
            cancel {
                # User backed out, set Control back to feedback. 
                #set newmodifyp_control 0
                return
            }
        }
    } elseif { $rec_level > 15 } {
        # We've waited more that 1.5 seconds, give up.
        tk_messageBox -type ok -title "Z Piezo Heightfield" \
                -parent .modify -default ok -message "
Direct Z control is unable to set 
$plane_name as the heightfield. 
Please resolve before using 
Direct Z control."
        set newmodifyp_control 0
        return
    } else {
        # Wait some more for Z Peizo to become available as a heightplane.
        #puts "Waiting $rec_level"
        after 100 [list set_directz_heightplane $plane_name [expr $rec_level +1]]
    }
}

####################################################
# Direct Z requires that the height plane be
# Z Peizo forward or reverse. Put up a dialog
# to enforce that requirement.
####################################################
proc enforce_directz_heightplane {} {
    global z_comes_from inputPlaneNames nmInfo scandatalist data_sets
    global newmodifyp_control
    # Check to see if height plane is Z Piezo forward/reverse
    if { !([string equal "$z_comes_from" "Z Piezo-Forward"] || \
            [string equal "$z_comes_from" "Z Piezo-Reverse"]) } {
        # Check to see that Z Piezo is being collected as a scan dataset
        set id2 [lsearch -exact $scandatalist {Z Piezo-Forward}]
        set id3 [lsearch -exact $scandatalist {Z Piezo-Reverse}]
        if { $id2 == -1 || $id3 == -1 } { 
            nano_error "Internal: Z Piezo not available as scan dataset"
            return
        }
        # Variable is 1 if Z Piezo is selected. 
        if { (!$data_sets(scan$id2)) && (!$data_sets(scan$id3)) } {
            # Neither is selected, so ask the user if they want to 
            # select Z Piezo Forward
            set response [tk_messageBox -type okcancel -title "Scan Z Piezo" \
                    -parent .modify -default ok -message "
The Direct Z control requires that 
Z Piezo (Forward or Reverse) be collected
as a scan dataset and displayed as the 
heightfield. 
Press OK to begin collecting Z Piezo Forward.
Press Cancel to resolve yourself." ]
            switch -- $response {
                ok {
                    $nmInfo(data_sets).scan.scanbutton$id2 select
                    after 100 [list set_directz_heightplane "Z Piezo-Forward" ]
                    return
                }
                cancel {
                    # User backed out, set Control back to feedback. 
                    #set newmodifyp_control 0
                    return
                }
            }
        }
        # Otherwise, Z Piezo being collected, but is not the height plane
        # Ask user to make it the height plane. 
        if { $data_sets(scan$id2) } {
            set_directz_heightplane "Z Piezo-Forward" 
        } elseif { $data_sets(scan$id3) } {
            set_directz_heightplane "Z Piezo-Reverse"
        }
    } else {
        #puts "nothing to be done."
    }
}

####################################################
# Initializes the full view.  All of the variables #
# that weren't set in init_quick get set here.     #
####################################################
proc init_full {} {
    global nmInfo modifyplist device_only_controls thirdtech_ui changing_widgets

    # Variables that store the size of the full view
    # so that the user can switch between full and quick
    # without having to resize the full view every time.
    global full_height full_width
    set full_height 0
    set full_width 0

    # changing_widgets are the radiobuttons
    set changing_widgets ""

    set modifyplist [list mode control style tool \
			 setpoint p_gain i_gain d_gain amplitude \
			 frequency input_gain ampl_or_phase drive_attenuation \
			 phase rate sweep_width bot_delay top_delay z_pull \
			 punchdist speed watchdog step_size z_start \
			 z_end z_pullback force_limit fcdist num_layers \
			 num_hcycles sample_speed pullback_speed start_speed feedback_speed \
			 avg_num optimize_now direct_step]

    if { !$thirdtech_ui } {    
        lappend modifyplist max_z_step max_xy_step min_z_setpoint \
			 max_z_setpoint max_lat_setpoint 
    }
    # Get globals for modifyp_* and newmodifyp_*.
    # newmodifyp_* gets updated as the user changes
    # selections, but only when Accept is pressed
    # does modifyp_* get updated. 
    # modifyp_* links with the C code.
    foreach modifyvar $modifyplist {
	global modifyp_$modifyvar newmodifyp_$modifyvar
    }

    ####################################################################
    ### MODE ###
    global doRelaxComp

    set modifyp_mode 1

    frame $nmInfo(modifyfull).mode
    label $nmInfo(modifyfull).mode.label -text "Modify Mode"

    radiobutton $nmInfo(modifyfull).mode.contact -text "Contact" \
	        -variable newmodifyp_mode -value 1 -anchor nw

    radiobutton $nmInfo(modifyfull).mode.oscillating -text "Oscillating" \
	        -variable newmodifyp_mode -value 0 -anchor nw

    eval lappend changing_widgets "$nmInfo(modifyfull).mode.contact \
                             $nmInfo(modifyfull).mode.oscillating"

    checkbutton $nmInfo(modifyfull).mode.relaxcomp -text "Offset Comp on" \
	        -variable doRelaxComp -state normal

    button $nmInfo(modifyfull).mode.accept -text "Accept"  \
	   -command "acceptModifyVars modifyplist" -highlightthickness 0
    button $nmInfo(modifyfull).mode.cancel -text "Revert"  \
	   -command "cancelModifyVars modifyplist" -highlightthickness 0

       # Special binding for accept button to make sure that all
       # entry widgets are finalized - happens when they loose focus.
       bind $nmInfo(modifyfull).mode.accept <Enter> "focus $nmInfo(modifyfull).mode.accept"

    eval lappend device_only_controls "$nmInfo(modifyfull).mode.oscillating \
                                  $nmInfo(modifyfull).mode.contact \
                                  $nmInfo(modifyfull).mode.relaxcomp \
                                  $nmInfo(modifyfull).mode.cancel  \
                                  $nmInfo(modifyfull).mode.accept"

    ### MODE PARAMETERS ###
    set modifyp_amplitude 0.1
    set modifyp_frequency 100
    set modifyp_input_gain 1.0
    # boolean, value of 1 is amplitude, 0 is phase
    set modifyp_ampl_or_phase 1
    set modifyp_drive_attenuation 1
    # this is the actual phase angle to use for feedback. 
    set modifyp_phase 0.0
    set modifyp_rate 1.0
    set modifyp_setpoint 1.0
    set modifyp_p_gain 1.0
    set modifyp_i_gain 0.5
    set modifyp_d_gain 0.01

    frame $nmInfo(modifyfull).modeparam
    label $nmInfo(modifyfull).modeparam.label -text "Mode parameters" 

    generic_entry $nmInfo(modifyfull).modeparam.setpoint_nA newmodifyp_setpoint \
	"Setpoint(-64,64 nA)" real
    generic_entry $nmInfo(modifyfull).modeparam.setpoint_pcnt newmodifyp_setpoint \
	"Setpoint(0,100 %)" real
    generic_entry $nmInfo(modifyfull).modeparam.p-gain newmodifyp_p_gain "P-Gain (0,5)" real 
    generic_entry $nmInfo(modifyfull).modeparam.i-gain newmodifyp_i_gain "I-Gain (0,2)" real 
    generic_entry $nmInfo(modifyfull).modeparam.d-gain newmodifyp_d_gain "D-Gain (0,5)" real 
    generic_entry $nmInfo(modifyfull).modeparam.rate newmodifyp_rate "Rate (0.1,50.0 um/sec)" real 
    generic_entry $nmInfo(modifyfull).modeparam.amplitude newmodifyp_amplitude \
	"Amplitude (0,2)" real 
    generic_entry $nmInfo(modifyfull).modeparam.frequency newmodifyp_frequency \
	"Frequency (10,200 kHz)" real 
    # input_gain_list should already be defined in image.tcl
    global input_gain_list
    generic_optionmenu $nmInfo(modifyfull).modeparam.input_gain \
        newmodifyp_input_gain \
	"Input Gain" input_gain_list
    generic_radiobox $nmInfo(modifyfull).modeparam.ampl_or_phase newmodifyp_ampl_or_phase \
	"" { "Phase" "Amplitude" }
    # drive_attenuation_list should already be defined in image.tcl
    global drive_attenuation_list
    generic_optionmenu $nmInfo(modifyfull).modeparam.drive_attenuation \
        newmodifyp_drive_attenuation \
	"Drive Attenuation" drive_attenuation_list
    generic_entry $nmInfo(modifyfull).modeparam.phase newmodifyp_phase \
	"Phase (0 360)" real 

    iwidgets::Labeledwidget::alignlabels \
	$nmInfo(modifyfull).modeparam.setpoint_nA \
	$nmInfo(modifyfull).modeparam.setpoint_pcnt \
	$nmInfo(modifyfull).modeparam.p-gain \
	$nmInfo(modifyfull).modeparam.i-gain \
	$nmInfo(modifyfull).modeparam.d-gain \
	$nmInfo(modifyfull).modeparam.rate \
	$nmInfo(modifyfull).modeparam.amplitude \
	$nmInfo(modifyfull).modeparam.frequency \
	$nmInfo(modifyfull).modeparam.input_gain \
	$nmInfo(modifyfull).modeparam.drive_attenuation \
	$nmInfo(modifyfull).modeparam.phase

    global mod_oscillating_list
    set mod_oscillating_list "$nmInfo(modifyfull).modeparam.amplitude \
			      $nmInfo(modifyfull).modeparam.frequency \
			      $nmInfo(modifyfull).modeparam.input_gain \
			      $nmInfo(modifyfull).modeparam.drive_attenuation \
			      $nmInfo(modifyfull).modeparam.ampl_or_phase \
			      $nmInfo(modifyfull).modeparam.phase "

    lappend device_only_controls $nmInfo(modifyfull).modeparam.setpoint_nA \
	$nmInfo(modifyfull).modeparam.setpoint_pcnt \
	$nmInfo(modifyfull).modeparam.p-gain \
	$nmInfo(modifyfull).modeparam.i-gain \
	$nmInfo(modifyfull).modeparam.d-gain \
	$nmInfo(modifyfull).modeparam.rate \
	$nmInfo(modifyfull).modeparam.amplitude \
	$nmInfo(modifyfull).modeparam.frequency \
	$nmInfo(modifyfull).modeparam.input_gain \
	$nmInfo(modifyfull).modeparam.drive_attenuation \
        [list $nmInfo(modifyfull).modeparam.ampl_or_phase buttonconfigure 0 ] \
	[list $nmInfo(modifyfull).modeparam.ampl_or_phase buttonconfigure 1 ] \
	$nmInfo(modifyfull).modeparam.phase

    ### STYLE ###
    set modifyp_style 0     

    frame $nmInfo(modifyfull).style 
    label $nmInfo(modifyfull).style.label -text "Style"

    radiobutton $nmInfo(modifyfull).style.sharp -text "Sharp" -variable newmodifyp_style \
	-value 0 -anchor nw
    # 1 was Blunt  --  now obsolete
    radiobutton $nmInfo(modifyfull).style.sweep -text "Sweep" -variable newmodifyp_style \
	-value 2 -anchor nw
    radiobutton $nmInfo(modifyfull).style.sewing -text "Sewing" -variable newmodifyp_style \
	-value 3 -anchor nw
    radiobutton $nmInfo(modifyfull).style.forcecurve -text "ForceCurve" -variable newmodifyp_style \
	-value 4 -anchor nw

    eval lappend changing_widgets "$nmInfo(modifyfull).style.sharp \
                             $nmInfo(modifyfull).style.sweep \
                             $nmInfo(modifyfull).style.sewing \
                             $nmInfo(modifyfull).style.forcecurve"

    eval lappend device_only_controls "$nmInfo(modifyfull).style.sharp \
	$nmInfo(modifyfull).style.sweep \
        $nmInfo(modifyfull).style.sewing \
	$nmInfo(modifyfull).style.forcecurve"

    ### STYLE PARAMETERS ###
    set modifyp_sweep_width 500

    set modifyp_bot_delay 1.0
    set modifyp_top_delay 1.0
    set modifyp_z_pull 50.0
    set modifyp_punchdist 2.0
    set modifyp_speed 200
    set modifyp_watchdog 100

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

    frame $nmInfo(modifyfull).styleparam
    label $nmInfo(modifyfull).styleparam.label -text "Style parameters"

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

    generic_entry $nmInfo(modifyfull).styleparam.z-start newmodifyp_z_start \
	"Start Height (-1000,1000 nm)" real 
    generic_entry $nmInfo(modifyfull).styleparam.z-end newmodifyp_z_end \
	"End Height (-1000,1000 nm)" real 
    generic_entry $nmInfo(modifyfull).styleparam.z-pullback newmodifyp_z_pullback \
	"Pullout Height (-1000,0 nm)" real 
    generic_entry $nmInfo(modifyfull).styleparam.force-limit newmodifyp_force_limit\
	"Force Limit (0,1000 nA)" real 
    generic_entry $nmInfo(modifyfull).styleparam.forcecurvedist newmodifyp_fcdist \
	"F.C. Dist. (0,500 nm)" real 
    generic_entry $nmInfo(modifyfull).styleparam.num-layers newmodifyp_num_layers \
	"# Samples (100,500)" real 
    generic_entry $nmInfo(modifyfull).styleparam.num-halfcycles newmodifyp_num_hcycles \
	"# Half Cycles (1,4)" real 
    generic_entry $nmInfo(modifyfull).styleparam.sample-speed newmodifyp_sample_speed \
	"Sample Speed (0,10 um/s)" real 
    generic_entry $nmInfo(modifyfull).styleparam.pullback-speed newmodifyp_pullback_speed \
	"Pullback Speed (0,100 um/s)" real 
    generic_entry $nmInfo(modifyfull).styleparam.start-speed newmodifyp_start_speed \
	"Start Speed (0,10 um/s)" real 
    generic_entry $nmInfo(modifyfull).styleparam.fdback-speed newmodifyp_feedback_speed \
	"Feedback Speed (0,100 um/s)" real 
    generic_entry $nmInfo(modifyfull).styleparam.avg-num newmodifyp_avg_num \
	"Averaging (1,10)" real 

    global mod_sweep_list
    set mod_sweep_list "$nmInfo(modifyfull).styleparam.sweepwidth"
   
    global mod_sewing_list
    set mod_sewing_list "$nmInfo(modifyfull).styleparam.bot-delay \
	$nmInfo(modifyfull).styleparam.top-delay $nmInfo(modifyfull).styleparam.z-pull \
        $nmInfo(modifyfull).styleparam.punchdist $nmInfo(modifyfull).styleparam.speed \
	$nmInfo(modifyfull).styleparam.watchdog"

    eval iwidgets::Labeledwidget::alignlabels $mod_sewing_list

    global mod_forcecurve_list
    set mod_forcecurve_list " \
	$nmInfo(modifyfull).styleparam.z-start $nmInfo(modifyfull).styleparam.z-end \
	$nmInfo(modifyfull).styleparam.z-pullback $nmInfo(modifyfull).styleparam.force-limit \
	$nmInfo(modifyfull).styleparam.forcecurvedist $nmInfo(modifyfull).styleparam.num-layers \
	$nmInfo(modifyfull).styleparam.num-halfcycles \
	$nmInfo(modifyfull).styleparam.sample-speed $nmInfo(modifyfull).styleparam.pullback-speed \
	$nmInfo(modifyfull).styleparam.start-speed $nmInfo(modifyfull).styleparam.fdback-speed \
	$nmInfo(modifyfull).styleparam.avg-num "

    eval iwidgets::Labeledwidget::alignlabels $mod_forcecurve_list

    # eval command expands the lists so we get one list of single elements. 
    eval lappend device_only_controls "$mod_sweep_list \
	$mod_sewing_list \
        $mod_forcecurve_list"

    ### TOOL ###
    set modifyp_tool 0

    frame $nmInfo(modifyfull).tool 
    label $nmInfo(modifyfull).tool.label -text "Tool" 

    radiobutton $nmInfo(modifyfull).tool.freehand -text "Freehand" \
	-variable newmodifyp_tool -value 0   -anchor nw
    radiobutton $nmInfo(modifyfull).tool.line -text "Line" \
	-variable newmodifyp_tool -value 1   -anchor nw
    radiobutton $nmInfo(modifyfull).tool.constrfree -text "Constr. Free" \
	-variable newmodifyp_tool -value 2   -anchor nw
    # 3 was Constrained Freehand XYZ  --  This is now Constrained Freehand w/ DirectZ
    radiobutton $nmInfo(modifyfull).tool.slow_line -text "Slow Line" \
	-variable newmodifyp_tool -value 4 -anchor nw
    # 5 was Slow Line 3D  --  This is now Slow Line w/ DirectZ
    if { !$thirdtech_ui } {    
        radiobutton $nmInfo(modifyfull).tool.feelahead -text "FeelAhead" \
                -variable newmodifyp_tool -value 6 -anchor nw
        radiobutton $nmInfo(modifyfull).tool.optimize_now -text "Optimize Now" \
                -variable newmodifyp_tool -value 7 -anchor nw
        radiobutton $nmInfo(modifyfull).tool.direct_step -text "Direct Step" \
                -variable newmodifyp_tool -value 8 -anchor nw
    }
    eval lappend changing_widgets "$nmInfo(modifyfull).tool.freehand \
        $nmInfo(modifyfull).tool.line \
        $nmInfo(modifyfull).tool.constrfree \
	$nmInfo(modifyfull).tool.slow_line"
    if { !$thirdtech_ui } {    
	eval lappend changing_widgets "$nmInfo(modifyfull).tool.feelahead \
	$nmInfo(modifyfull).tool.optimize_now \
	$nmInfo(modifyfull).tool.direct_step "
    }
    eval lappend device_only_controls "$nmInfo(modifyfull).tool.freehand \
        $nmInfo(modifyfull).tool.line \
        $nmInfo(modifyfull).tool.constrfree \
	$nmInfo(modifyfull).tool.slow_line"
    if { !$thirdtech_ui } {    
	eval lappend device_only_controls "$nmInfo(modifyfull).tool.feelahead \
	$nmInfo(modifyfull).tool.optimize_now \
	$nmInfo(modifyfull).tool.direct_step "
    }

    ### TOOL PARAMETERS ###
    set modifyp_step_size 1
    set modifyp_optimize_now 3
    set modifyp_direct_step 0

    frame $nmInfo(modifyfull).toolparam
    label $nmInfo(modifyfull).toolparam.label -text "Tool parameters" 

    generic_entry $nmInfo(modifyfull).toolparam.step-size newmodifyp_step_size \
	"Step Size (0,5 nm)" real 
    if { !$thirdtech_ui } {    
        #setup Optimize Now params box
        radiobutton $nmInfo(modifyfull).toolparam.optimize_line -text "Optimize Line" \
	   -variable newmodifyp_optimize_now -value 0 -anchor nw
        radiobutton $nmInfo(modifyfull).toolparam.optimize_area -text "Optimize Area" \
	   -variable newmodifyp_optimize_now -value 1 -anchor nw
    }
    global mod_line_list
    set mod_line_list  "$nmInfo(modifyfull).toolparam.step-size"

    global mod_slow_line_list
    set mod_slow_line_list "$nmInfo(modifyfull).toolparam.step-size"

    if { !$thirdtech_ui } {    
        global optimize_now_param_list
        set optimize_now_param_list "$nmInfo(modifyfull).toolparam.optimize_line \
	$nmInfo(modifyfull).toolparam.optimize_area"
    }
    # eval command expands the lists so we get one list of single elements. 
    eval lappend device_only_controls "$mod_line_list \
	$mod_slow_line_list"
    if { !$thirdtech_ui } {    
        eval lappend device_only_controls "$optimize_now_param_list "
    }

    ### CONTROL ###
    set modifyp_control 0
    if { !$thirdtech_ui } {    

	frame $nmInfo(modifyfull).control
	label $nmInfo(modifyfull).control.label -text "Control" 

	radiobutton $nmInfo(modifyfull).control.feedback -text "Feedback" \
	    -variable newmodifyp_control -value 0  -anchor nw
	radiobutton $nmInfo(modifyfull).control.directz -text "Direct Z" \
	    -variable newmodifyp_control -value 1 -anchor nw \
            -command { enforce_directz_heightplane }

	global mod_control_list
	set mod_control_list "$nmInfo(modifyfull).control.feedback \
                              $nmInfo(modifyfull).control.directz"

	eval lappend changing_widgets "$nmInfo(modifyfull).control.feedback \
	    $nmInfo(modifyfull).control.directz" 

	eval lappend device_only_controls "$nmInfo(modifyfull).control.feedback \
	    $nmInfo(modifyfull).control.directz" 

	### CONTROL PARAMETERS ###
	set modifyp_max_z_step 1
	set modifyp_max_xy_step 1
	set modifyp_min_z_setpoint -50
	set modifyp_max_z_setpoint 50
	set modifyp_max_lat_setpoint 50

	frame $nmInfo(modifyfull).controlparam
	label $nmInfo(modifyfull).controlparam.label -text "Control parameters" 

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

	global mod_directz_list
	set mod_directz_list  "$nmInfo(modifyfull).controlparam.max_z_step \
                               $nmInfo(modifyfull).controlparam.max_xy_step \
                               $nmInfo(modifyfull).controlparam.min_z_setpoint \
                               $nmInfo(modifyfull).controlparam.max_z_setpoint \
                               $nmInfo(modifyfull).controlparam.max_lat_setpoint"

	eval iwidgets::Labeledwidget::alignlabels $mod_directz_list

	eval lappend device_only_controls $mod_directz_list
    }

    ##########################################################################
    # These variables only exist in tcl - the user changes
    # them, and then the "accept" button copies them into the vars
    # above, so the C code sees them.
    foreach modifyvar $modifyplist {
	set newmodifyp_$modifyvar [set modifyp_$modifyvar]
    }


    # The following is a list of variables that could potentially
    # cause changes to the window when they are written to (for
    # instance, when a radio button gets selected).  Each member
    # of the list has corresponding variables, newmodifyp_* and
    # modifyp_*.  These variables are traced, so that when a
    # potentially screen-changing event happens, the change_made
    # function is called.
    foreach modifyvar [list mode control style tool ampl_or_phase] {
	trace variable newmodifyp_$modifyvar w "change_made"
    }

    # These traces change the color of Accept and Cancel when you haven't
    # pressed Accept yet. and if a modify var changes outside of nano,
    # set the newmodifyvar.
    foreach modifyvar $modifyplist {
	trace variable newmodifyp_$modifyvar w modBackgChReal
	trace variable modifyp_$modifyvar w "modify_change_made $modifyvar"
    }
    
    # The following variables have been rendered
    # obsolete.  I put them here so that they can
    # be found if you ever want to use them again.
       # set modifyp_constr_xyz_mode 0
       # set modifyp_optimize_now 3
       # set modifyp_tri_size 1
       # set modifyp_tri_speed 5000
       # set modifyp_start_delay 10.0
       # set modifyp_sample_delay 0.0
       # set modifyp_pullback_delay 0.0
       # set modifyp_feedback_delay 200.0

    trace variable newmodifyp_optimize_now w flip_optimize_selection_mode

    # These variables could cause the modify_live window to pop up
    trace variable newmodifyp_tool w show_modify_live
    trace variable newmodifyp_control w show_modify_live 

	#these variables could cause the directStep window to pop up
	trace variable newmodifyp_tool w show_directStep
	trace variable newmodifyp_control w show_directStep


    # Packing list is important in pack_full, but it needs to be
    # initialized here
    global packing_list
    set packing_list ""
}

#######################################
# Repacks quick  view when necessary. #
# Called from set_view.               #
#######################################
proc pack_quick {} {
    global nmInfo thirdtech_ui \
	   modifyp_mode modifyp_style modifyp_tool modifyp_control \
	   modifyp_ampl_or_phase
    # set label at top
    switch $modifyp_mode {
	0   {   #oscillating mode
	        $nmInfo(modifystate).mod_mode configure -text "Mode: Oscillate"
	    }
	1   {   #contact mode
	        $nmInfo(modifystate).mod_mode configure -text "Mode: Contact"
	    }
    } 
    switch $modifyp_style {
	0   {   #Sharp style
	        $nmInfo(modifystate).mod_style configure -text "Style: Sharp"
	    }
	1   {   #Blunt style OBSOLETE
	    }
	2   {   #Sweep style
	        $nmInfo(modifystate).mod_style configure -text "Style: Sweep"
	    }
	3   {   #Sewing style
	        $nmInfo(modifystate).mod_style configure -text "Style: Sewing"
	    }
	4   {   # Force Curve style
	        $nmInfo(modifystate).mod_style configure -text "Style: Force Curve"
	    }
    } 
    switch $modifyp_tool {
	0   {  	#Freehand tool
	        $nmInfo(modifystate).mod_tool configure -text "Tool: Freehand"
	    }
	1   {  	#Line tool
	        $nmInfo(modifystate).mod_tool configure -text "Tool: Line"
	    }
	2   {   #Constrained Freehand tool
	        $nmInfo(modifystate).mod_tool configure -text "Tool: Constr. Free"
	    }
	3   {   #Constrained Freehand xyz tool OBSOLETE
	    }
	4   {  	# Slow Line tool
	        $nmInfo(modifystate).mod_tool configure -text "Tool: Slow Line"
	    }
	5   {   # Slow Line 3d tool OBSOLETE
	    }
	6   {	# Feelahead tool
	        $nmInfo(modifystate).mod_tool configure -text "Tool: Feelahead"
	    }
	7   {   # Optimize Now tool
	        $nmInfo(modifystate).mod_tool configure -text "Tool: Optimize Now"
	    }
	8   {   #direct_Step tool
	        $nmInfo(modifystate).mod_tool configure -text "Tool: Direct Step"
	}
    }

    if { !$thirdtech_ui } {
	switch $modifyp_control {
	    0   {   #Feedback control
		    $nmInfo(modifystate).mod_control configure -text "Control: Feedback"
	        }
	    1   {   #DirectZ control
		    $nmInfo(modifystate).mod_control configure -text "Control: Direct Z"
	        }
	}
    }
    # pack label at top
       # Label info
       if { !$thirdtech_ui } {
	   pack $nmInfo(modifystate).mod_control -side bottom -anchor nw
       }
       pack $nmInfo(modifystate).mod_tool \
	    $nmInfo(modifystate).mod_style \
	    $nmInfo(modifystate).mod_mode \
	    -side bottom -anchor nw
       # Label itself
       pack $nmInfo(modifyquick).modifystate \
            $nmInfo(modifypage) \
            -side top -fill x 
    # pack window
    # fist forget everything in it, in case modifyp_mode has changed
	pack forget $nmInfo(modifypage).setpoint_pcnt \
	            $nmInfo(modifypage).setpoint_nA \
	            $nmInfo(modifypage).p-gain \
	            $nmInfo(modifypage).i-gain \
                    $nmInfo(modifypage).d-gain \
	            $nmInfo(modifypage).rate
    # first line is dependent upon value of modifyp_mode
    if {$modifyp_mode == 0 && $modifyp_ampl_or_phase == 1} { 
	pack $nmInfo(modifypage).setpoint_pcnt \
	     -side top -anchor nw -fill x
    } else { 
	pack $nmInfo(modifypage).setpoint_nA \
	     -side top -anchor nw -fill x
    }
    # pack the rest
    pack    $nmInfo(modifypage).p-gain \
	    $nmInfo(modifypage).i-gain \
            $nmInfo(modifypage).d-gain \
	    $nmInfo(modifypage).rate \
	    -side top -anchor nw -fill x
}

#############################################
# Repacks full view when neccessary.        #
# Dynamically changes display based on      #
# the parameters needed for the mode/       #
# style/tool/control that the user selects. #
# Called from set_view.                     #
#############################################
proc pack_full {} {
    global nmInfo fspady thirdtech_ui packing_list

    # First, unpack all items that might change from call to call
    foreach element $packing_list {
	pack forget $element
    }

    # Zero out packing_list
    set packing_list ""
    
    ### MODE ###
    # Always pack the mode frame
    pack $nmInfo(modifyfull).mode -side left -padx 4 -fill both
    pack $nmInfo(modifyfull).mode.label -side top -anchor nw
    
    lappend packing_list $nmInfo(modifyfull).mode 

    # Oscillating is 0 and Contact is 1, but
    # Contact should come before Oscillating in the User Interface
    # because Contact is the default.
    # At some point we should change Contact to 0 and Oscillating to 1
    # so that the numbers match up with what's on the screen.
    pack $nmInfo(modifyfull).mode.contact $nmInfo(modifyfull).mode.oscillating -side top -fill x

    pack $nmInfo(modifyfull).mode.relaxcomp -side top -fill x -pady 20

    pack $nmInfo(modifyfull).mode.accept $nmInfo(modifyfull).mode.cancel -side top -fill x  
  
    ### MODE PARAMETERS ###
    global newmodifyp_mode newmodifyp_ampl_or_phase
    # pack only the parameters that should be shown, but this frame
    # always gets packed
    pack $nmInfo(modifyfull).modeparam -side left -padx 4 -fill both
    pack $nmInfo(modifyfull).modeparam.label -side top -anchor nw

    lappend packing_list $nmInfo(modifyfull).modeparam

    # setpoint_nA if Contact or Oscillating/Phase, setpoint_pcnt if Oscillating/Amplitude
    if {$newmodifyp_mode == 1 || ($newmodifyp_mode == 0 && $newmodifyp_ampl_or_phase == 0)} {
	pack $nmInfo(modifyfull).modeparam.setpoint_nA -side top -fill x -pady $fspady
	lappend packing_list $nmInfo(modifyfull).modeparam.setpoint_nA 
    } else {
	if {$newmodifyp_mode == 0 && $newmodifyp_ampl_or_phase == 1} {
	    pack $nmInfo(modifyfull).modeparam.setpoint_pcnt -side top -fill x -pady $fspady
	    lappend packing_list $nmInfo(modifyfull).modeparam.setpoint_pcnt
	}
    }
 
    pack $nmInfo(modifyfull).modeparam.p-gain \
	 $nmInfo(modifyfull).modeparam.i-gain \
         $nmInfo(modifyfull).modeparam.d-gain \
         $nmInfo(modifyfull).modeparam.rate \
	 -side top -fill x -pady $fspady

    eval lappend packing_list "$nmInfo(modifyfull).modeparam.p-gain \
	 $nmInfo(modifyfull).modeparam.i-gain \
         $nmInfo(modifyfull).modeparam.d-gain \
         $nmInfo(modifyfull).modeparam.rate"

    # if Oscillating, show additional params
    if { $newmodifyp_mode == 0 } {
	pack $nmInfo(modifyfull).modeparam.amplitude \
	     $nmInfo(modifyfull).modeparam.frequency \
	     $nmInfo(modifyfull).modeparam.input_gain \
	     $nmInfo(modifyfull).modeparam.drive_attenuation \
	     $nmInfo(modifyfull).modeparam.ampl_or_phase \
	     $nmInfo(modifyfull).modeparam.phase \
	     -side top -fill x -pady $fspady

	eval lappend packing_list "$nmInfo(modifyfull).modeparam.amplitude \
	                      $nmInfo(modifyfull).modeparam.frequency \
	                      $nmInfo(modifyfull).modeparam.input_gain \
	                      $nmInfo(modifyfull).modeparam.drive_attenuation \
	                      $nmInfo(modifyfull).modeparam.ampl_or_phase \
	                      $nmInfo(modifyfull).modeparam.phase" 
    }

    ### STYLE ###
    # Always pack this frame
    pack $nmInfo(modifyfull).style -side left -padx 4 -fill both
    pack $nmInfo(modifyfull).style.label -side top -anchor nw

    lappend packing_list $nmInfo(modifyfull).style

    pack $nmInfo(modifyfull).style.sharp \
	 $nmInfo(modifyfull).style.sweep \
         $nmInfo(modifyfull).style.sewing \
	 $nmInfo(modifyfull).style.forcecurve \
	 -side top -fill x

    ### STYLE PARAMETERS ###
    global newmodifyp_style
    # Only pack when style is Sweep, Sewing, or Force Curve
    if { $newmodifyp_style != 0 } {
	pack $nmInfo(modifyfull).styleparam -side left -padx 4 -fill both
	pack $nmInfo(modifyfull).styleparam.label -side top -anchor nw

	lappend packing_list $nmInfo(modifyfull).styleparam
    }

    global mod_sweep_list mod_sewing_list mod_forcecurve_list
    # What to pack depends on which style is selected
    switch $newmodifyp_style {
	2  {
	    foreach element $mod_sweep_list {
		pack $element -side top -fill x -pady $fspady
		lappend packing_list $element
	    }
	   }
	
	3  {
	    foreach element $mod_sewing_list {
		pack $element -side top -fill x -pady $fspady
		lappend packing_list $element
	    }
	   }
	
	4  {
	    foreach element $mod_forcecurve_list {
		pack $element -side top -fill x -pady $fspady
		lappend packing_list $element
	    }
	   }
	
	default {}
    }

    ### TOOL ###
    # Always pack this frame
    pack $nmInfo(modifyfull).tool -side left -padx 4 -fill both
    pack $nmInfo(modifyfull).tool.label -side top -anchor nw

    lappend packing_list $nmInfo(modifyfull).tool

    pack $nmInfo(modifyfull).tool.freehand \
	 $nmInfo(modifyfull).tool.line \
         $nmInfo(modifyfull).tool.constrfree \
	 $nmInfo(modifyfull).tool.slow_line -side top -fill x
    if { !$thirdtech_ui } {    
	 pack $nmInfo(modifyfull).tool.feelahead \
	 $nmInfo(modifyfull).tool.optimize_now \
	 $nmInfo(modifyfull).tool.direct_step \
	 -side top -fill x
    }
    ### TOOL PARAMETERS ###
    global newmodifyp_tool
    # Only pack when tool is Line, Slow Line, or Optimize Now
    if { $newmodifyp_tool == 1 || $newmodifyp_tool == 4 || $newmodifyp_tool == 7} {
	pack $nmInfo(modifyfull).toolparam -side left -padx 4 -fill both
	pack $nmInfo(modifyfull).toolparam.label -side top -anchor nw
	lappend packing_list $nmInfo(modifyfull).toolparam
    }

    global mod_line_list mod_slow_line_list optimize_now_param_list
    # What to pack depends on which style is selected
    switch $newmodifyp_tool {
	1  {
	    foreach element $mod_line_list {
		pack $element -side top -fill x -pady $fspady
		lappend packing_list $element
	    }
	   }
	
	4  {
	    foreach element $mod_slow_line_list {
		pack $element -side top -fill x -pady $fspady
		lappend packing_list $element
	    }
	   }

	7  {
	    foreach element $optimize_now_param_list {
		pack $element -side top -fill x -pady $fspady
		lappend packing_list $element
	    }
	   }
	
	default {}
    }

    if { !$thirdtech_ui } {    
	### CONTROL ###
	# Always show this frame
	pack $nmInfo(modifyfull).control -side left -padx 4 -fill both
	pack $nmInfo(modifyfull).control.label -side top -anchor nw   

	lappend packing_list $nmInfo(modifyfull).control

	pack $nmInfo(modifyfull).control.feedback \
	    $nmInfo(modifyfull).control.directz \
	    -side top -fill x

	### CONTROL PARAMETERS ###
	global newmodifyp_control
	# Only pack when tool is DirectZ
	if { $newmodifyp_control == 1 } {
	    pack $nmInfo(modifyfull).controlparam -side left -padx 4 -fill both
	    pack $nmInfo(modifyfull).controlparam.label -side top -anchor nw
	    lappend packing_list $nmInfo(modifyfull).controlparam

	    global mod_directz_list
	    foreach element $mod_directz_list {
		pack $element -side top -fill x -pady $fspady  
	    }
	}  
    }
}

##################################
# Show/hide modify_live window.  #
# Called when tool or control is #
# changed.                       #
##################################
proc show_modify_live {nm el op} {
    # Show if control is DirectZ or tool is Slow Line,
    # hide otherwise.
    global newmodifyp_control newmodifyp_tool modify_live

    if { $newmodifyp_control == 1 || $newmodifyp_tool == 4 } {
	show.modify_live } else { hide.modify_live }

    # Even though we show modify_live window, we don't want it
    # in front, so we force focus back to .modify
    focus -force .modify
}

#################################
# show/hide direct_step window  #
#Called when tool or control is #
#changed.			#
#################################
proc show_directStep {nm el op} {
	#show if tool is direct_step
	global newmodifyp_tool direct_step newmodifyp_control newmodifyp_direct_step
if {$newmodifyp_tool == 8} {
    show.directStep} else { hide.directStep }
if {$newmodifyp_control == 0 } {
	set newmodifyp_direct_step 0 
      } else { set newmodifyp_direct_step 1
	}
}



#########################################
# Traced to variables that might change #
# full view (mode/style/tool/control).  # 
# Simply sets enabling and repacks.     #
#########################################
proc change_made {nm el op} {
    global modify_quick_or_full nmInfo full_width full_height 

    #Don't do anything if Full Params window isn't open
    if { $modify_quick_or_full == "quick" || [winfo ismapped $nmInfo(modify)] == 0 } {
	return
    } else {

	# this is now set at the end, so window is automatically resized
        # set full_width [winfo width .modify]
	set_enabling

	#pack here so we can figure out how big we need to make the window
	pack_full
	#so winfo gives correct size of window.
	update idletasks

	# set the full width size here, but do not set the window parameters
	# here, this is done in set_view
        set full_width [expr [winfo reqwidth $nmInfo(modifyfull)]]
	set_view

	# reset window size, so everything fits
        #set full_width [expr [winfo reqwidth $nmInfo(modifyfull)]]
	#wm geometry .modify =[set full_width]x[set full_height]
    }
}
##############################################
# when a value of a modify variable changes, #
# change the newmodify value as well         #
#                                            #
##############################################
proc modify_change_made {val nm el op} {
    global nmInfo
    global save_bg
	global modifyp_$val
	global newmodifyp_$val
	if {[set newmodifyp_$val] != [set modifyp_$val] } {
	    set newmodifyp_$val [set modifyp_$val]
	}

    $nmInfo(modifyfull).mode.accept configure -background $save_bg
    $nmInfo(modifyfull).mode.cancel configure -background $save_bg
}

##############################################
# Dynamically enables/disables widgets       #
# based on other widgets that have been      #
# selected by the user.                      #
# For a listing of which number corresponds  #
# to which mode/style/tool/control, see the  #
# big switch statement in init_quick.        #
# To add an item to set_enabling, do the     #
# following:                                 #
# 1) Determine which combinations of         #
#    mode/style/tool/control should cause    #
#    your new widget to be disabled.         #
# 2) Write an if statement that checks for   #
#    all of the possibilities found in 1).   #
# 3) If the if statement is true, disable    #
#    your new widget.                        #
# 4) Determine which widgets should be       #
#    disabled when your widget is selected.  #
#    For each of those widgets, add the      #
#    condition for your widget to their if   #
#    statement.                              #
#                                            #
#  Example:                                  #
#   Let's pretend that you are adding a tool #
#   to the window.  Let's say that your tool #
#   is radio button number 12 (radio buttons #
#   get assigned numbers in init_full).      #
#   Let's also say that you can't use your   #
#   new tool if you're in contact mode       #
#   ($mode == 1) , or if the style is sharp  #
#   ($style == 0).  You should then add the  #
#   following code:                          #
#                                            #############################
#   # disable your_widget                                                #
#   if { $mode == 1  ||                                                  #
#        $style == 0 }{                                                  #
#                                                                        #
#        $nmInfo(modifyfull).tool.your_widget configure -state disabled  #
#   }                                                                    #
#                                                                        #
#   ...and you should add the following statement                        #
#      to the #disable contact if statement:                             #
#     $tool == 12  ||                                                    #
#                                                                        #
#   ...and you should add the following statement                        #
#      to the #disable sharp if statement:                               #
#     $tool == 12  ||                                                    #
##########################################################################
proc set_enabling {} {
    global nmInfo changing_widgets thirdtech_ui collab_commands_suspended

    #if we don't have mutex, we don't want to turn on any widgets
    if { $collab_commands_suspended == 1 } {
	return
    }

    upvar #0 newmodifyp_mode mode
    upvar #0 newmodifyp_style style
    upvar #0 newmodifyp_tool tool
    upvar #0 newmodifyp_control control

    # First, enable everything
    foreach widget $changing_widgets {
	$widget configure -state normal
    }

    # disable oscillating
    if { $style == 4                                     || \
	 $style == 3                                     || \
	($style == 2 && ($control == 1 || $tool == 7))   || \
	($style == 0 && $tool == 1 && $control == 1)     || \
	$tool == 8						    } {
	
	$nmInfo(modifyfull).mode.oscillating configure -state disabled
    }

    # disable contact
    if { ($style != 0 && ($control == 1 || $tool == 7))                  || \
	 (($style == 4 || $style == 3) && $tool == 4)                    || \
	 ($style == 0 && $control == 1 && ($tool == 1 || $tool == 7))    } {
	
	$nmInfo(modifyfull).mode.contact configure -state disabled
    }

    # disable sharp
    if { ($mode == 0 && $tool == 1 && $control == 1)                   || \
	 ($mode == 1 && ($tool == 1 || $tool == 7) && $control == 1)   } {

	$nmInfo(modifyfull).style.sharp configur -state disabled
    }

    # disable sweep
    if { $tool == 7     || \
	 $control == 1  || $tool == 8 } {

	$nmInfo(modifyfull).style.sweep configure -state disabled
    }

    # disable sewing
    if { $mode == 0     || \
	 $tool == 4     || \
	 $tool == 8     || \
	 $tool == 7     || \
	 $control == 1  } {

	$nmInfo(modifyfull).style.sewing configure -state disabled
    }

    # disable forcecurve
    if { $mode == 0     || \
	 $tool == 4     || \
	 $tool == 7     || \
	 $tool == 8     || \
	 $control == 1  } {

	$nmInfo(modifyfull).style.forcecurve configure -state disabled
    }

    # disable freehand
    if { ($style != 0 && $control == 1)                || \
	 ($mode == 0 && ($style == 3 || $style == 4)) } {

	$nmInfo(modifyfull).tool.freehand configure -state disabled
    }

    # disable line
    if { $control == 1                                 || \
	 ($mode == 0 && ($style == 3 || $style == 4))} {

	$nmInfo(modifyfull).tool.line configure -state disabled
    }

    # disable constrained freehand
    if { ($style != 0 && $control == 1)                || \
	 ($mode == 0 && ($style == 3 || $style == 4))  } {

	$nmInfo(modifyfull).tool.constrfree configure -state disabled
    }

    # disable slow line
    if { ($style != 0 && $control == 1) || \
	 $style == 3                    || \
         $style == 4                    } {

	$nmInfo(modifyfull).tool.slow_line configure -state disabled
    }

    if { !$thirdtech_ui } {    
        # disable optimize now
        if { $style != 0                                   || \
                ($style == 0 && $mode == 1 && $control == 1)  } {
            
            $nmInfo(modifyfull).tool.optimize_now configure -state disabled
        }
	
	# disable direct step
	if {$style != 0} {
            $nmInfo(modifyfull).tool.direct_step configure -state disabled
        }
        
        # disable feedback
        if { ($style != 0 && $tool == 7)                                   || \
                (($mode == 0 || $tool == 4) && ($style == 3 || $style == 4))  } {
            
            $nmInfo(modifyfull).control.feedback configure -state disabled
        }
        
        # disable directz
        if { $style != 0                || \
                $tool == 1                 || \
                ($mode == 1 && $tool == 7) } {
            
            $nmInfo(modifyfull).control.directz configure -state disabled
        }
    }
}

#######################################
# Modify display controls.            #
# Display and clear the "tic" markers #
# that show where a mod occured.      #
#######################################
proc init_display {} {
    global nmInfo

    set nmInfo(modifydisplay) [frame $nmInfo(modify).display]
    pack [frame $nmInfo(modifydisplay).divider -relief raised -height 4 -borderwidth 2] \
        -fill x -side top -pady 4
    label $nmInfo(modifydisplay).mod_mode -text "Modify Marker Display"
    pack $nmInfo(modifydisplay).mod_mode -side top -anchor nw 

    set number_of_markers_shown 500
    set marker_height 100
    generic_entry $nmInfo(modifydisplay).num_mod_markers number_of_markers_shown \
	"Num. Markers" real
    generic_entry $nmInfo(modifydisplay).mod_marker_height marker_height \
	"Marker Hgt. (nm)" real

    iwidgets::Labeledwidget::alignlabels \
	$nmInfo(modifydisplay).num_mod_markers \
	$nmInfo(modifydisplay).mod_marker_height	

    button $nmInfo(modifydisplay).modmarkclr -text "Clear Markers" \
	-command "set clear_markers 1"
    
    pack $nmInfo(modifydisplay).num_mod_markers \
	$nmInfo(modifydisplay).mod_marker_height \
	$nmInfo(modifydisplay).modmarkclr \
	-side top -anchor nw

    # This window can always stay packed. 
    pack $nmInfo(modifydisplay) -side bottom -fill x
}

#############################################
# Change the background of Accept and       #
# Cancel buttons when you haven't yet       #
# committed your changes by clicking Accept #
#############################################
proc modBackgChReal {fooa element op} {
    global nmInfo

    $nmInfo(modifyfull).mode.accept configure -background LightPink1
    $nmInfo(modifyfull).mode.cancel configure -background LightPink1
}

##############################
# Bound to the accept button #
##############################
proc acceptModifyVars {varlist} {
    global nmInfo
    global save_bg $varlist

    #direct step variables
    global sphere_axis setting_direct_step_axis modifyp_tool newmodifyp_tool
#take care of direct step issues
if {$modifyp_tool == 8 } {
  #we were in direct step
  if {$newmodifyp_tool != 8 } {
     #we are switching out of direct step
     #disable direct step axis visualizations
     set sphere_axis 0
     set setting_direct_step_axis 0
     set import_grab_object 0
    }
   }

    # Save changes made by the user
    # modifyp_* = newmodifyp_*
    foreach val [set $varlist] {
	global modifyp_$val
	global newmodifyp_$val
	if {[set newmodifyp_$val] != [set modifyp_$val] } {
	    set modifyp_$val [set newmodifyp_$val]
	}
    }

    # Mimick action of "Quick/Full" button
    switch_view

}

##############################
# Bound to the Cancel button #
##############################
proc cancelModifyVars {varlist} {
    global nmInfo
    global save_bg $varlist

    # Erase changes made by the user
    # newmodifyp_* = modifyp_*
    foreach val [set $varlist] {
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

#########################################################
# Trace the variable and change the name of the button. #
# Why a trace? Because this way the C code can set the  #
# variable and the button will display the right name.  #
#########################################################
proc config_play_name {name elem op} {
    global slow_line_playing nmInfo
    if {$slow_line_playing} {
	$nmInfo(top_slow_line).slow_line_play configure -text "Pause"
    } else {
	$nmInfo(top_slow_line).slow_line_play configure -text "Play"
    }
}


###########################################################
# LIVE Modify controls                                    #
# Controls you need access to _during_ a modification.    #
# The most important - slow line controls                 #
# init_live_conrols initializes AND packs (since we only  #
# need to pack these once)                                #
###########################################################
proc init_live_controls {} {
    global nmInfo \
	   slow_line_playing slow_line_direction \
           device_only_controls thirdtech_ui

    set nmInfo(modify_live) [create_closing_toplevel modify_live "Live Modify Controls"]

    iwidgets::Labeledframe $nmInfo(modify_live).slow_line -labeltext "Slow Line" \
	-labelpos nw
    set nmInfo(ml_slow_line) [$nmInfo(modify_live).slow_line childsite]
    pack $nmInfo(modify_live).slow_line -fill both -expand yes

    set nmInfo(top_slow_line) [frame $nmInfo(ml_slow_line).top_slow_line]

	#collect data even when playing is paused?
	checkbutton $nmInfo(top_slow_line).slow_line_collect_data -text "Collect data\n when paused" \
	-variable collect_data -padx 0 -pady 0

	set collect_data 1

    set slow_line_playing 0
    button $nmInfo(top_slow_line).slow_line_play -text "Play" -command {
	if {$slow_line_playing} {
	    set slow_line_playing 0
	} else {
	    set slow_line_playing 1
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
	"Step Size (nm)" real 

    pack $nmInfo(top_slow_line) -side top
    pack $nmInfo(top_slow_line).slow_line_collect_data \
	$nmInfo(top_slow_line).slow_line_play -side right

    pack $nmInfo(ml_slow_line).slow_line_step \
	$nmInfo(ml_slow_line).slow_line_forward \
	$nmInfo(ml_slow_line).slow_line_reverse \
	$nmInfo(ml_slow_line).step-size \
	-side top -pady 2 -anchor nw

    eval lappend device_only_controls "$nmInfo(top_slow_line).slow_line_play \
	$nmInfo(ml_slow_line).slow_line_step \
	$nmInfo(ml_slow_line).slow_line_forward \
	$nmInfo(ml_slow_line).slow_line_reverse \
	$nmInfo(ml_slow_line).step-size $nmInfo(top_slow_line).slow_line_collect_data" 

    if { !$thirdtech_ui } {
	iwidgets::Labeledframe $nmInfo(modify_live).directz -labeltext "Direct Z" \
	    -labelpos nw
	set nmInfo(ml_directz) [$nmInfo(modify_live).directz childsite]
	pack $nmInfo(modify_live).directz -fill both -expand yes

	generic_entry $nmInfo(ml_directz).directz_force_scale directz_force_scale\
	    "Force scale (1,3)" real

	pack $nmInfo(ml_directz).directz_force_scale -side top -pady 2 -anchor nw

	lappend device_only_controls \
	    $nmInfo(ml_directz).directz_force_scale
    }
}

#### The Initialization calls
init_modify
init_quick
init_display
init_full
init_live_controls

#set collab command variable so it can be read
global collab_commands_suspended
set collab_commands_suspended 1

# Call this so that the quick window
# starts off at the right size
switch_view

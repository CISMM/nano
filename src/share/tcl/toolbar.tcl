set nmInfo(toolbar) [toplevel .toolbar]

wm title .toolbar "Toolbar"
# Group with the main window
wm group .toolbar .

#We don't want the user to be able to close the toolbar window
# independently of the main window. 
wm protocol .toolbar WM_DELETE_WINDOW ";"

iwidgets::Labeledframe $nmInfo(toolbar).imagestate -labeltext "Image state" \
	-labelpos nw
set nmInfo(imagepage) [frame $nmInfo(toolbar).imagepage]
pack $nmInfo(toolbar).imagestate $nmInfo(toolbar).imagepage -side top \
	-fill x


set nmInfo(imagestate) [$nmInfo(toolbar).imagestate childsite]

label $nmInfo(imagestate).image_mode -text "Oscillate" -anchor nw
pack $nmInfo(imagestate).image_mode -side top -fill x 

# Callback procedures set a variable to tell C code that
# it's time to send new values to AFM. 
generic_entry $nmInfo(imagepage).setpoint imagep_setpoint \
	"Setpoint (0,100%)" real \
        { set accepted_image_params 1 }
generic_entry $nmInfo(imagepage).p-gain imagep_p_gain "P-Gain (0,5)" real \
        { set accepted_image_params 1 }
generic_entry $nmInfo(imagepage).i-gain imagep_i_gain "I-Gain (0,5)" real\
        { set accepted_image_params 1 }
generic_entry $nmInfo(imagepage).d-gain imagep_d_gain "D-Gain (0,5)" real\
        { set accepted_image_params 1 }
generic_entry $nmInfo(imagepage).rate imagep_rate "Rate (um/sec)" real\
        { set accepted_image_params 1 }

pack    $nmInfo(imagepage).setpoint $nmInfo(imagepage).p-gain \
	$nmInfo(imagepage).i-gain $nmInfo(imagepage).d-gain \
	$nmInfo(imagepage).rate \
	-side top -anchor nw

iwidgets::optionmenu $nmInfo(toolbar).taskchoice -labeltext "Task:" -labelpos w -command {
      $nmInfo(toolbar).taskbook view [$nmInfo(toolbar).taskchoice get]
}
pack $nmInfo(toolbar).taskchoice -side top -anchor nw

$nmInfo(toolbar).taskchoice insert end "Setup" "Modify" "Analysis"
$nmInfo(toolbar).taskchoice select "Modify"

set nmInfo(taskbook) [iwidgets::notebook $nmInfo(toolbar).taskbook \
	-width 180 -height 350]

### Setup page
set nmInfo(setuppage) [$nmInfo(taskbook) add -label "Setup"]

### Analysis page
set nmInfo(analysispage) [$nmInfo(taskbook) add -label "Analysis"]

### Modify page
set nmInfo(modifypage) [$nmInfo(taskbook) add -label "Modify"]

frame $nmInfo(modifypage).left
iwidgets::Labeledframe $nmInfo(modifypage).right -labeltext "Modify state" \
	-labelpos nw
pack $nmInfo(modifypage).right $nmInfo(modifypage).left -side top \
	-fill x 

set nmInfo(modifypageright) [$nmInfo(modifypage).right childsite]
generic_entry $nmInfo(modifypage).left.setpoint modifyp_setpoint \
	"Setpoint (0,100%)" real \
        { set accepted_modify_params 1 }
generic_entry $nmInfo(modifypage).left.p-gain modifyp_p_gain "P-Gain (0,5)" real \
        { set accepted_modify_params 1 }
generic_entry $nmInfo(modifypage).left.i-gain modifyp_i_gain "I-Gain (0,5)" real \
        { set accepted_modify_params 1 }
generic_entry $nmInfo(modifypage).left.d-gain modifyp_d_gain "D-Gain (0,5)" real \
        { set accepted_modify_params 1 }
generic_entry $nmInfo(modifypage).left.rate modifyp_rate "Rate (um/sec)" real \
        { set accepted_modify_params 1 }

	
pack    $nmInfo(modifypage).left.setpoint $nmInfo(modifypage).left.p-gain \
	$nmInfo(modifypage).left.i-gain $nmInfo(modifypage).left.d-gain \
	$nmInfo(modifypage).left.rate \
	-side top -anchor nw

label $nmInfo(modifypageright).mod_mode -text "Contact"
label $nmInfo(modifypageright).mod_style -text "Sharp"
label $nmInfo(modifypageright).mod_tool -text "Freehand"
label $nmInfo(modifypageright).mod_control -text "Feedback"

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
	"show_mode modifyp_mode $nmInfo(modifypageright).mod_mode"
trace variable modifyp_style w \
	"show_style modifyp_style $nmInfo(modifypageright).mod_style"
trace variable modifyp_tool w \
	"show_tool modifyp_tool $nmInfo(modifypageright).mod_tool"
trace variable modifyp_control w \
	"show_control modifyp_control $nmInfo(modifypageright).mod_control"

pack $nmInfo(modifypageright).mod_control -side bottom -anchor nw
pack $nmInfo(modifypageright).mod_mode $nmInfo(modifypageright).mod_style \
	$nmInfo(modifypageright).mod_tool \
	-side left -anchor nw

pack [frame $nmInfo(modifypage).left.spacer -height 5 -width 5] -side top -fill x -pady 2

label $nmInfo(modifypage).left.mod_mode -text "Modify Display"
pack $nmInfo(modifypage).left.mod_mode -side top -anchor nw -pady 4

set number_of_markers_shown 500
set marker_height 100
generic_entry $nmInfo(modifypage).left.num_mod_markers number_of_markers_shown \
	"Num. Markers" real
generic_entry $nmInfo(modifypage).left.mod_marker_height marker_height \
	"Marker Height (nm)" real

iwidgets::Labeledwidget::alignlabels \
	$nmInfo(modifypage).left.setpoint $nmInfo(modifypage).left.p-gain \
	$nmInfo(modifypage).left.i-gain $nmInfo(modifypage).left.d-gain \
	$nmInfo(modifypage).left.rate \
	$nmInfo(imagepage).setpoint $nmInfo(imagepage).p-gain \
	$nmInfo(imagepage).i-gain $nmInfo(imagepage).d-gain \
	$nmInfo(imagepage).rate \
	$nmInfo(modifypage).left.num_mod_markers \
	$nmInfo(modifypage).left.mod_marker_height
	

button $nmInfo(modifypage).modmarkclr -text "Clear Markers" \
	-command "set term_input C"

pack $nmInfo(modifypage).left.num_mod_markers \
	$nmInfo(modifypage).left.mod_marker_height \
	$nmInfo(modifypage).modmarkclr \
	-side top -anchor nw


pack $nmInfo(taskbook) -side top -expand yes -fill both
$nmInfo(taskbook) view Modify



# Analysis menu

#
##############
# Ruler grid
#

# keep track of whether the rulergrid has been changed
# this is traced by a variable in c
set rulergrid_changed 0

# keep track of whether user is positioning rulergrid with lines
set rulergrid_position_line 0
set rulergrid_orient_line 0


################################
#
# Procedure to adjust rulergrid scaling, translation, and color

#set these so we can see do " wishx <mainwin.tcl" and test interface
set ruler_r 255
set ruler_g 255
set ruler_b 100

set ruler_color [format #%02x%02x%02x $ruler_r $ruler_g $ruler_b]

proc set_rulergrid_color {} {
    global ruler_r ruler_g ruler_b rulergrid_changed ruler_color
    # Extract three component colors of ruler_color 
    # and save into ruler_r g b
    scan $ruler_color #%02x%02x%02x ruler_r ruler_g ruler_b
    set rulergrid_changed 1
}


proc set_position_choice {} {
    global rulergrid_position_line 
    global nmInfo
    #if using the red line, disable the position sliders
    if {$rulergrid_position_line == 1} {
	$nmInfo(rulergrid).left.rulergrid_xoffset configure -state disabled
	$nmInfo(rulergrid).left.rulergrid_yoffset configure -state disabled
    } elseif {$rulergrid_position_line == 0} {
	$nmInfo(rulergrid).left.rulergrid_xoffset configure -state normal
	$nmInfo(rulergrid).left.rulergrid_yoffset configure -state normal
    }
}

proc set_orient_choice {} {
    global rulergrid_orient_line
    global nmInfo
    #if using the green line, disable the angle sliders
    if {$rulergrid_orient_line == 1} {
	$nmInfo(rulergrid).left.rulergrid_angle configure -state disabled
    } elseif {$rulergrid_orient_line == 0} {
	$nmInfo(rulergrid).left.rulergrid_angle configure -state normal
    }
}


set nmInfo(rulergrid) [create_closing_toplevel rulergrid "Rulergrid Analysis"]

set framepady 0

frame $nmInfo(rulergrid).left 
frame $nmInfo(rulergrid).right 
pack $nmInfo(rulergrid).left $nmInfo(rulergrid).right -side left -fill both -padx 2m

checkbutton $nmInfo(rulergrid).left.rulergrid_enable \
	-text "Rulergrid Enabled" -variable rulergrid_enabled -anchor nw
pack $nmInfo(rulergrid).left.rulergrid_enable

label $nmInfo(rulergrid).left.label -text "Rulergrid Parameters" 

button $nmInfo(rulergrid).left.set_color -text "Set rulergrid color" -command {
    choose_color ruler_color "Choose rulergrid color"
    $nmInfo(rulergrid).left.colorsample configure -bg $ruler_color
    set_rulergrid_color
}

    # This sample frame displays the color of the rulergrid
    frame $nmInfo(rulergrid).left.colorsample -height 64 -width 46 -relief groove -bd 5 -bg $ruler_color

checkbutton $nmInfo(rulergrid).left.rulergrid_pos_line \
	-text "Set grid offset to red line" -variable rulergrid_position_line \
	-command set_position_choice -anchor nw

generic_entry $nmInfo(rulergrid).left.rulergrid_xoffset rulergrid_x \
	"Grid X offset" real
generic_entry $nmInfo(rulergrid).left.rulergrid_yoffset rulergrid_y \
	"Grid Y offset" real

generic_entry $nmInfo(rulergrid).left.rulergrid_scale rulergrid_scale \
	"Grid spacing" real

checkbutton $nmInfo(rulergrid).left.rulergrid_orient_line \
	-text "Set angle to green line" -variable rulergrid_orient_line \
	-command set_orient_choice -anchor nw

generic_entry $nmInfo(rulergrid).left.rulergrid_angle rulergrid_angle \
	"Grid angle (degrees)" real

generic_entry $nmInfo(rulergrid).left.linewidthx ruler_width_x \
	"X line width (0,100%)" numeric
generic_entry $nmInfo(rulergrid).left.linewidthy ruler_width_y \
	"Y line width (0,100%)" numeric

set ruler_opacity 255
generic_entry $nmInfo(rulergrid).left.opacity ruler_opacity \
	"Opacity (0,255)" numeric

set_position_choice

#pack $nmInfo(rulergrid).left.label -side top -anchor nw

#pack the scaling sliders
pack $nmInfo(rulergrid).left.rulergrid_scale -side top -fill x -pady $fspady
#pack the orient sliders
pack $nmInfo(rulergrid).left.rulergrid_angle -side top -fill x -pady $fspady
#pack the colors
pack $nmInfo(rulergrid).left.set_color $nmInfo(rulergrid).left.colorsample \
	-side top -anchor e -pady $framepady -padx 3m -fill x

#pack the position checkbutton
pack $nmInfo(rulergrid).left.rulergrid_pos_line \
	-anchor nw -side top -fill x -pady $fspady
#pack the orient checkbutton
pack $nmInfo(rulergrid).left.rulergrid_orient_line \
	-anchor nw -side top -fill x -pady $fspady

pack $nmInfo(rulergrid).left.rulergrid_xoffset -side top -fill x -pady $fspady
pack $nmInfo(rulergrid).left.rulergrid_yoffset -side top -fill x -pady $fspady

#pack the linewidth and opacity
    pack $nmInfo(rulergrid).left.linewidthx \
	    $nmInfo(rulergrid).left.linewidthy \
	    $nmInfo(rulergrid).left.opacity \
	    -side top -fill x -pady $fspady

## End rulergrid 


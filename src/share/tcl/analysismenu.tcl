
# Analysis menu

#
##############
# Calculate Planes
#
set nmInfo(calc_planes) [create_closing_toplevel calc_planes \
        "Calculate Data Planes"]


#Flat plane
iwidgets::Labeledframe $nmInfo(calc_planes).flat \
	-labeltext "Create a flattened plane" \
	-labelpos nw
set nmInfo(flatplane) [$nmInfo(calc_planes).flat childsite]

pack $nmInfo(calc_planes).flat -side top -fill x

# Allow the user to create a flattened plane from the height plane:
label $nmInfo(flatplane).flatlabel -justify left -text \
	"Position measure lines then\nenter a plane name:"
generic_entry $nmInfo(flatplane).flatplane flatplane_name \
	"Flatten plane" ""
pack $nmInfo(flatplane).flatlabel $nmInfo(flatplane).flatplane \
	-side top -anchor nw


#Line-by-Line Flat plane
iwidgets::Labeledframe $nmInfo(calc_planes).lblflat \
	-labeltext "Create a line-by-line flattened plane" \
	-labelpos nw
set nmInfo(lblflatplane) [$nmInfo(calc_planes).lblflat childsite]

pack $nmInfo(calc_planes).lblflat -side top -fill x

# Allow the user to create a flattened plane from the height plane:
label $nmInfo(lblflatplane).lblflatlabel -justify left -text \
	"Enter a plane name:"
generic_entry $nmInfo(lblflatplane).lblflatplane lblflatplane_name \
	"Line-by-line flatten plane" ""
pack $nmInfo(lblflatplane).lblflatlabel $nmInfo(lblflatplane).lblflatplane \
	-side top -anchor nw


#Sum plane - algebraically combine data from two planes
iwidgets::Labeledframe $nmInfo(calc_planes).sum \
	-labeltext "Create a sum plane" \
	-labelpos nw
set nmInfo(sumplane) [$nmInfo(calc_planes).sum childsite]

pack $nmInfo(calc_planes).sum -side top -fill x

# Allow the user to create a sum plane from two other planes:
label $nmInfo(sumplane).sumlabel -justify left -text \
	"Choose which planes to sum\nand a scale factor for the second plane"
generic_optionmenu $nmInfo(sumplane).sumplane1 sum_first_plane \
	"First plane" inputPlaneNames
generic_entry $nmInfo(sumplane).sum_scale sum_scale \
	"Scale factor" real
generic_optionmenu $nmInfo(sumplane).sumplane2 sum_second_plane \
	"Second plane" inputPlaneNames
generic_entry $nmInfo(sumplane).sumplane sumplane_name \
	"Sum plane" ""
pack $nmInfo(sumplane).sumlabel $nmInfo(sumplane).sumplane1 \
        $nmInfo(sumplane).sum_scale $nmInfo(sumplane).sumplane2 \
        $nmInfo(sumplane).sumplane \
	-side top -anchor nw




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
	"Grid spacing (nm)" real

checkbutton $nmInfo(rulergrid).left.rulergrid_orient_line \
	-text "Set angle to yellow line" -variable rulergrid_orient_line \
	-command set_orient_choice -anchor nw

generic_entry $nmInfo(rulergrid).left.rulergrid_angle rulergrid_angle \
	"Grid angle (degrees)" real

generic_entry $nmInfo(rulergrid).left.linewidthx ruler_width_x \
	"X line width (0,100%)" numeric
generic_entry $nmInfo(rulergrid).left.linewidthy ruler_width_y \
	"Y line width (0,100%)" numeric

set ruler_opacity 255
generic_entry $nmInfo(rulergrid).left.opacity ruler_opacity \
	"Opacity (0,100%)" numeric

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


#/*===3rdtech===
#  Copyright (c) 2001 by 3rdTech, Inc.
#  All Rights Reserved.
#
#  This file may not be distributed without the permission of 
#  3rdTech, Inc. 
#  ===3rdtech===*/

#
# Analysis menu
#

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

if { !$thirdtech_ui } {    
#Simulated Scan plane
iwidgets::Labeledframe $nmInfo(calc_planes).simscan \
	-labeltext "Create plane from simulated AFM scan" \
	-labelpos nw
set nmInfo(simscanplane) [$nmInfo(calc_planes).simscan childsite]

pack $nmInfo(calc_planes).simscan -side top -fill x

# Allow the user to create a sim. scan plane:
label $nmInfo(simscanplane).simscanlabel -justify left -text \
	"Enter a plane name and the name of the\ncomputer the simulator is running on:\n(e.g. radium-cs@cs.unc.edu)"
generic_entry $nmInfo(simscanplane).simscanplane simscanplane_name \
	"Simulated AFM Scan Plane" ""
# Allow the user to specify the computer the simulator is running on:
generic_entry $nmInfo(simscanplane).simIPaddress simscanIPaddress \
	"IP Address" ""
pack $nmInfo(simscanplane).simscanlabel $nmInfo(simscanplane).simscanplane \
		$nmInfo(simscanplane).simIPaddress \
	-side top -anchor nw

#Eroder plane
iwidgets::Labeledframe $nmInfo(calc_planes).eroder \
	-labeltext "Create plane showing eroded scan" \
	-labelpos nw
set nmInfo(eroderplane) [$nmInfo(calc_planes).eroder childsite]

pack $nmInfo(calc_planes).eroder -side top -fill x

# Allow the user to create a eroded plane:
label $nmInfo(eroderplane).eroderlabel -justify left -text \
	"Enter a plane name and the name of the\ncomputer the eroder is running on:"
generic_entry $nmInfo(eroderplane).eroderplane eroderplane_name \
	"Eroded Scan Plane" ""
# Allow the user to specify the computer the eroder is running on:
generic_entry $nmInfo(eroderplane).eroderIPaddress eroderIPaddress \
	"IP Address" ""
pack $nmInfo(eroderplane).eroderlabel $nmInfo(eroderplane).eroderplane \
		$nmInfo(eroderplane).eroderIPaddress \
	-side top -anchor nw

}


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
#set ruler_r 255
#set ruler_g 255
#set ruler_b 100

#set ruler_color [format #%02x%02x%02x $ruler_r $ruler_g $ruler_b]
set ruler_color #ffff64

# Procedure to change color-swatch color at collaborator
proc ruler_color_changed { name el op } {
    global nmInfo ruler_color
    $nmInfo(rulergrid).c.colorsample configure -bg $ruler_color
}
trace variable ruler_color w ruler_color_changed

proc set_rulergrid_color {} {
    global rulergrid_changed ruler_color
    # Extract three component colors of ruler_color 
    # and save into ruler_r g b
    #scan $ruler_color #%02x%02x%02x ruler_r ruler_g ruler_b
    set rulergrid_changed 1
}


proc set_position_choice {} {
    global rulergrid_position_line 
    global nmInfo
    #if using the red line, disable the position sliders
    if {$rulergrid_position_line == 1} {
	$nmInfo(rulergrid).rulergrid_xoffset configure -state disabled
	$nmInfo(rulergrid).rulergrid_yoffset configure -state disabled
    } elseif {$rulergrid_position_line == 0} {
	$nmInfo(rulergrid).rulergrid_xoffset configure -state normal
	$nmInfo(rulergrid).rulergrid_yoffset configure -state normal
    }
}

proc set_orient_choice {} {
    global rulergrid_orient_line
    global nmInfo
    #if using the green line, disable the angle sliders
    if {$rulergrid_orient_line == 1} {
	$nmInfo(rulergrid).rulergrid_angle configure -state disabled
    } elseif {$rulergrid_orient_line == 0} {
	$nmInfo(rulergrid).rulergrid_angle configure -state normal
    }
}


set nmInfo(rulergrid) [create_closing_toplevel rulergrid "Rulergrid Analysis"]

set framepady 0

checkbutton $nmInfo(rulergrid).rulergrid_enable \
	-text "Rulergrid Enabled" -variable rulergrid_enabled -anchor nw
pack $nmInfo(rulergrid).rulergrid_enable

label $nmInfo(rulergrid).label -text "Rulergrid Parameters" 


frame $nmInfo(rulergrid).c
button $nmInfo(rulergrid).c.set_color -text "Set rulergrid color" -command {
    choose_color ruler_color "Choose rulergrid color" $nmInfo(rulergrid)
    $nmInfo(rulergrid).c.colorsample configure -bg $ruler_color
    set_rulergrid_color
}

# This sample frame displays the color of the rulergrid
button $nmInfo(rulergrid).c.colorsample -relief groove -bd 2 -bg $ruler_color \
        -command { $nmInfo(rulergrid).c.set_color invoke}

checkbutton $nmInfo(rulergrid).rulergrid_pos_line \
	-text "Set grid offset to red line" -variable rulergrid_position_line \
	-command set_position_choice -anchor nw

generic_entry $nmInfo(rulergrid).rulergrid_xoffset rulergrid_x \
	"Grid X offset" real
generic_entry $nmInfo(rulergrid).rulergrid_yoffset rulergrid_y \
	"Grid Y offset" real

generic_entry $nmInfo(rulergrid).rulergrid_scale rulergrid_scale \
	"Grid spacing (nm)" real

checkbutton $nmInfo(rulergrid).rulergrid_orient_line \
	-text "Set angle to yellow line" -variable rulergrid_orient_line \
	-command set_orient_choice -anchor nw

generic_entry $nmInfo(rulergrid).rulergrid_angle rulergrid_angle \
	"Grid angle (degrees)" real

generic_entry $nmInfo(rulergrid).linewidthx ruler_width_x \
	"X line width (0,100%)" real
generic_entry $nmInfo(rulergrid).linewidthy ruler_width_y \
	"Y line width (0,100%)" real

set ruler_opacity 255
generic_entry $nmInfo(rulergrid).opacity ruler_opacity \
	"Opacity (0,100%)" numeric

set_position_choice

iwidgets::Labeledwidget::alignlabels \
        $nmInfo(rulergrid).rulergrid_scale \
        $nmInfo(rulergrid).rulergrid_angle \
        $nmInfo(rulergrid).rulergrid_xoffset \
        $nmInfo(rulergrid).rulergrid_yoffset \
        $nmInfo(rulergrid).linewidthx \
        $nmInfo(rulergrid).linewidthy \
        $nmInfo(rulergrid).opacity 
#pack $nmInfo(rulergrid).label -side top -anchor nw

#pack the scaling sliders
pack $nmInfo(rulergrid).rulergrid_scale -side top -fill x -pady $fspady
#pack the orient sliders
pack $nmInfo(rulergrid).rulergrid_angle -side top -fill x -pady $fspady

#pack the colors
pack $nmInfo(rulergrid).c -side top -fill x
pack $nmInfo(rulergrid).c.set_color -side left -fill x
pack $nmInfo(rulergrid).c.colorsample -side left -fill x -expand yes
#pack the position checkbutton
pack $nmInfo(rulergrid).rulergrid_pos_line \
	-anchor nw -side top -fill x -pady $fspady
#pack the orient checkbutton
pack $nmInfo(rulergrid).rulergrid_orient_line \
	-anchor nw -side top -fill x -pady $fspady

pack $nmInfo(rulergrid).rulergrid_xoffset -side top -fill x -pady $fspady
pack $nmInfo(rulergrid).rulergrid_yoffset -side top -fill x -pady $fspady

#pack the linewidth and opacity
    pack $nmInfo(rulergrid).linewidthx \
	    $nmInfo(rulergrid).linewidthy \
	    $nmInfo(rulergrid).opacity \
	    -side top -fill x -pady $fspady

## End rulergrid 

#
################################
# Shape Analysis
# This part of the script describes the framework for a control panel
# for controlling the shape analysis code

set nmInfo(shape_analysis) [create_closing_toplevel shape_analysis "Shape Analysis"]

set framepady 0

label $nmInfo(shape_analysis).label -text "Shape Analysis Parameters" 

frame $nmInfo(shape_analysis).shapeframe
button $nmInfo(shape_analysis).shapeframe.analyze_now -text "Analyze Now" -command {
	set analyze_shape 1    
}

set blurring 4
set aspect_ratio 2
set intensity_thresh 0.6
set correlation 0.6
set pre_flatten 1
set auto_adapt 1
set shape_mask 1
set shape_order 0
set shape_mask_file ""
set shape_order_file ""

proc set_intensity_thresh {} {
	global auto_adapt
	global nmInfo
	#if using auto-adaptation, disable the intensity threshold field
	if {$auto_adapt == 1} {
		$nmInfo(shape_analysis).intensity_thresh configure -state disabled
	} elseif {$auto_adapt == 0} {
		$nmInfo(shape_analysis).intensity_thresh configure -state normal
	}
}


generic_entry $nmInfo(shape_analysis).blurring blurring \
	"Gaussian Blurring (pixels)" real

generic_entry $nmInfo(shape_analysis).aspect_ratio aspect_ratio \
	"Aspect Ratio for Tube Recognition" real

generic_entry $nmInfo(shape_analysis).intensity_thresh intensity_thresh \
	"Intensity Threshold for Tube Recognition (0-1)" real

generic_entry $nmInfo(shape_analysis).correlation correlation \
	"Tube Width Correlation (0-1)" real

checkbutton $nmInfo(shape_analysis).pre_flatten \
	-text "Pre-Flatten Image" -variable pre_flatten \
	-anchor nw

checkbutton $nmInfo(shape_analysis).auto_adapt \
	-text "Auto-Set Intensity Threshold" -variable auto_adapt \
	-command set_intensity_thresh -anchor nw

checkbutton $nmInfo(shape_analysis).shape_mask \
	-text "Output Image Mask File" -variable shape_mask \
	-anchor nw

checkbutton $nmInfo(shape_analysis).shape_order \
	-text "Output Image Order File" -variable shape_order \
	-anchor nw

generic_entry $nmInfo(shape_analysis).shape_mask_file shape_mask_file \
	"Image Mask File Name" ""

generic_entry $nmInfo(shape_analysis).shape_order_file shape_order_file \
	"Image Order File Name" ""

set_intensity_thresh

iwidgets::Labeledwidget::alignlabels \
		$nmInfo(shape_analysis).blurring \
		$nmInfo(shape_analysis).aspect_ratio \
		$nmInfo(shape_analysis).intensity_thresh \
		$nmInfo(shape_analysis).correlation \
		$nmInfo(shape_analysis).shape_mask_file \
		$nmInfo(shape_analysis).shape_order_file 
#pack $nmInfo(shape_analysis).label -side top -anchor nw

#pack blurring
pack $nmInfo(shape_analysis).blurring -side top -fill x -pady $fspady
#pack the aspect ratio
pack $nmInfo(shape_analysis).aspect_ratio -side top -fill x -pady $fspady
#pack intensity threshold
pack $nmInfo(shape_analysis).intensity_thresh -side top -fill x -pady $fspady
#pack correlation
pack $nmInfo(shape_analysis).correlation -side top -fill x -pady $fspady
#pack mask file
pack $nmInfo(shape_analysis).shape_mask_file -side top -fill x -pady $fspady
#pack order file
pack $nmInfo(shape_analysis).shape_order_file -side top -fill x -pady $fspady

#pack the pre flatten checkbutton
pack $nmInfo(shape_analysis).pre_flatten \
	-anchor nw -side top -fill x -pady $fspady

#pack the auto adaption checkbutton
pack $nmInfo(shape_analysis).auto_adapt \
	-anchor nw -side top -fill x -pady $fspady

#pack the mask checkbutton
pack $nmInfo(shape_analysis).shape_mask \
	-anchor nw -side top -fill x -pady $fspady

#pack the order checkbutton
pack $nmInfo(shape_analysis).shape_order \
	-anchor ne -side top -fill x -pady $fspady

#pack the analyze now button
pack $nmInfo(shape_analysis).shapeframe -side top -fill x
pack $nmInfo(shape_analysis).shapeframe.analyze_now -side left -fill x

## End shape_analysis

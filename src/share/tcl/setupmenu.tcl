#/*===3rdtech===
#  Copyright (c) 2000 by 3rdTech, Inc.
#  All Rights Reserved.
#
#  This file may not be distributed without the permission of 
#  3rdTech, Inc. 
#  ===3rdtech===*/
# Provide a toplevel widget to allow
# control over which data sets are enabled in scan and touch
# mode.  
# Put in one sub-area for scan and one for touch. */
# ----------------------------------------------------------------------
set nmInfo(data_sets) [create_closing_toplevel data_sets "Datasets Setup" ]

frame $nmInfo(data_sets).scan -relief sunken -bd 2
label $nmInfo(data_sets).scan.label -text "Scan"
frame $nmInfo(data_sets).touch -relief sunken -bd 2
label $nmInfo(data_sets).touch.label -text "Touch and Modify"
label $nmInfo(data_sets).touch.instr -text "Hit Enter to change no. of samples"
frame $nmInfo(data_sets).forcecurve -relief sunken -bd 2
label $nmInfo(data_sets).forcecurve.label -text "Force Curve"
#frame $nmInfo(data_sets).scanline -relief sunken -bd 2
#label $nmInfo(data_sets).scanline.label -text "Line Scan"

pack $nmInfo(data_sets).scan.label -side top
pack $nmInfo(data_sets).scan -side left -anchor n
pack $nmInfo(data_sets).touch.label $nmInfo(data_sets).touch.instr -side top
pack $nmInfo(data_sets).touch -side left -anchor n
pack $nmInfo(data_sets).forcecurve.label -side top
pack $nmInfo(data_sets).forcecurve -side left -anchor n
#pack $nmInfo(data_sets).scanline.label -side top
#pack $nmInfo(data_sets).scanline -side left -anchor n 

set scandatalist [list \
        "Topography-Forward" \
        "Topography-Reverse" \
        "Internal Sensor-Forward" \
        "Internal Sensor-Reverse" \
        "Z Modulation-Forward" \
        "Z Modulation-Reverse" \
        "Lateral Force-Forward" \
        "Lateral Force-Reverse" \
        "IN 1-Forward" \
        "IN 1-Reverse" \
        "IN 2-Forward" \
        "IN 2-Reverse" \
        "FastTrack-Forward" \
        "FastTrack-Reverse" \
        "Z Piezo-Forward" \
        "Z Piezo-Reverse" ]

set i 0
foreach name $scandatalist {
    checkbutton $nmInfo(data_sets).scan.scanbutton$i -text "$name" \
            -variable data_sets(scan$i)
    pack $nmInfo(data_sets).scan.scanbutton$i -side top -anchor nw
    incr i
}
#set num_scandata [expr $i -1]

set touchdatalist [list \
        "Topography" \
        "Internal Sensor" \
        "Z Modulation" \
        "Lateral Force" \
        "IN 1" \
        "IN 2" \
        "FastTrack" \
        "Z Piezo" ]

set i 0
foreach name $touchdatalist {
    frame $nmInfo(data_sets).touch.f$i
    checkbutton $nmInfo(data_sets).touch.f$i.touchbutton$i -text "$name" \
            -variable data_sets(touch$i)
    generic_entry $nmInfo(data_sets).touch.f$i.touchentry$i \
            data_sets(touch_samples$i) "" integer
    pack $nmInfo(data_sets).touch.f$i -side top -fill x -expand yes
    pack $nmInfo(data_sets).touch.f$i.touchbutton$i \
            -side left -anchor nw
    pack $nmInfo(data_sets).touch.f$i.touchentry$i \
            -side right -anchor ne
    incr i
}
#set num_touchdata [expr $i -1]

# spmlab allows two out of three including the following and
# the two auxiliary channels, but selection of other than the default
# hasn't been implemented yet
set forcecurvedatalist [list "Internal Sensor" ]
set i 0
foreach name $forcecurvedatalist {
    checkbutton $nmInfo(data_sets).forcecurve.forcecurvebutton$i -text "$name"\
            -variable data_sets(forcecurve$i)
    pack $nmInfo(data_sets).forcecurve.forcecurvebutton$i -side top -anchor nw
    incr i
}
#set num_forcecurvedata [expr $i -1]

#
################################
# This part of the script describes the framework for a control panel
# for the external filter programs.
#
set nmInfo(external_filters) [create_closing_toplevel external_filters \
	"External Filter Programs"]

generic_optionmenu $nmInfo(external_filters).pick_program_name pick_program \
	"External Filter Program" filter_names
pack $nmInfo(external_filters).pick_program_name

generic_optionmenu $nmInfo(external_filters).pick_plane pick_plane \
	"Plane to Filter" inputPlaneNames
pack $nmInfo(external_filters).pick_plane

label $nmInfo(external_filters).plabel -text "Program Parameters"
set proc_params ""
entry $nmInfo(external_filters).entry -relief sunken -bd 2 \
	-textvariable proc_params
label $nmInfo(external_filters).dlabel -text "Filtered Plane Name"
pack $nmInfo(external_filters).plabel $nmInfo(external_filters).entry \
	$nmInfo(external_filters).dlabel -side top
newlabel_dialogue filterplane_name $nmInfo(external_filters)


#
################################
#
# This part of the script describes the framework for a control panel
# for the Z data.  This allows both the selection of the plane to use
# for mapping to Z and the scale to apply to that mapping.  Both of the
# tools for selecting these are created using Tcl_Linkvar variables in
# the code.
#
set nmInfo(z_mapping) [create_closing_toplevel z_mapping "Height Plane Setup"]
#show.z_mapping

generic_optionmenu $nmInfo(z_mapping).z_dataset z_comes_from \
	"Height plane" inputPlaneNames
pack $nmInfo(z_mapping).z_dataset -anchor nw -padx 3 -pady 3

set z_scale 1
generic_entry $nmInfo(z_mapping).scale z_scale "Z scale" real
pack $nmInfo(z_mapping).scale -anchor nw -padx 3 -pady 3

#
#################################    
# Controls for the various visualizations

set nmInfo(visualizations) [create_closing_toplevel visualizations "Visualizations Setup"]
#show.visualizations

set viz_choice 0
set viz_min 0
set viz_max 1
set viz_min_limit 0
set viz_max_limit 1
set viz_alpha 0.5

#Viz choices plane
iwidgets::Labeledframe $nmInfo(visualizations).viz \
	-labeltext "Choose visualization to use" \
	-labelpos nw
set nmInfo(viz_choices) [$nmInfo(visualizations).viz childsite]

pack $nmInfo(visualizations).viz -side top -fill x

frame $nmInfo(viz_choices).r1
frame $nmInfo(viz_choices).r2
radiobutton $nmInfo(viz_choices).r1.opaque -text "Opaque" \
    -variable viz_choice -value 0  -padx 0 -pady 0
radiobutton $nmInfo(viz_choices).r1.transparent -text "Transparent" \
    -variable viz_choice -value 1  -padx 0 -pady 0
radiobutton $nmInfo(viz_choices).r2.wire_frame -text "Wire Frame" \
	-variable viz_choice -value 2  -padx 0 -pady 0
radiobutton $nmInfo(viz_choices).r2.opaque_tex -text "Opacity Texture" \
	-variable viz_choice -value 3  -padx 0 -pady 0

pack $nmInfo(viz_choices).r1 $nmInfo(viz_choices).r2 -side left -fill x
pack $nmInfo(viz_choices).r1.opaque $nmInfo(viz_choices).r1.transparent -anchor nw
pack $nmInfo(viz_choices).r2.wire_frame $nmInfo(viz_choices).r2.opaque_tex -anchor nw

#Viz choices plane
iwidgets::Labeledframe $nmInfo(visualizations).control \
	-labeltext "Visualization Controls" \
	-labelpos nw
set nmInfo(viz_controls) [$nmInfo(visualizations).control childsite]

pack $nmInfo(visualizations).control -side top -fill x

generic_optionmenu $nmInfo(viz_controls).viz_dataset \
        viz_comes_from "Visualization plane" inputPlaneNames

label $nmInfo(viz_controls).slidelabel -justify left -text \
	"Set the values below to determine the area that\nthe alternate visualization method is used for:"

label $nmInfo(viz_controls).planemin -justify left -text \
	"Visualization plane min height $viz_min_limit"

generic_entry $nmInfo(viz_controls).viz_min viz_min "Min height:" real

label $nmInfo(viz_controls).planemax -justify left -text \
	"Visualization plane max height $viz_max_limit"

generic_entry $nmInfo(viz_controls).viz_max viz_max "Max height:" real

label $nmInfo(viz_controls).alphalabel -justify left -text \
	"Alpha value to use for visualization"

generic_entry $nmInfo(viz_controls).viz_alpha viz_alpha "Alpha:" real

pack $nmInfo(viz_controls).viz_dataset -side top -anchor w
pack $nmInfo(viz_controls).slidelabel -side top -anchor w
pack $nmInfo(viz_controls).planemin -side top -anchor w
pack $nmInfo(viz_controls).viz_min -side top -anchor w -padx 1m
pack $nmInfo(viz_controls).planemax -side top -anchor w
pack $nmInfo(viz_controls).viz_max -side top -anchor w
pack $nmInfo(viz_controls).alphalabel -side top -anchor w
pack $nmInfo(viz_controls).viz_alpha -side top -anchor w -padx 9m
trace variable viz_min_limit w update_minmax_label
trace variable viz_max_limit w update_minmax_label

proc update_minmax_label {name el op} {
	global viz_min_limit viz_max_limit
    global nmInfo

	$nmInfo(viz_controls).planemin configure -text \
	"Visualization plane min height $viz_min_limit"

	$nmInfo(viz_controls).planemax configure -text \
	"Visualization plane max height $viz_max_limit"
}

#
#################################    
# Controls for the colormap are in a separate file (colormap.tcl)
# sourced from mainwin.tcl.

############################
# Contour lines
#set these so we can see do " wish mainwin.tcl" and test interface
if {![info exists contour_r] } { set contour_r 255 }
if {![info exists contour_g] } { set contour_g 55 }
if {![info exists contour_b] } { set contour_b 55 }

if {![info exists contour_width] } { set contour_width 1 }
if {![info exists texture_spacing] } { set texture_spacing 10 }

proc set_contour_color {} {
    global contour_r contour_g contour_b contour_changed contour_color
    global nmInfo
    # Extract three component colors of contour_color 
    # and save into contour_r g b
    scan $contour_color #%02x%02x%02x contour_r contour_g contour_b
    set contour_changed 1
    #puts "Contour color $contour_r $contour_g $contour_b"
}

#
################################
#
# This part of the script describes the framework for a control panel
# for contour lines.  This allows both the selection of the plane to use
# for mapping to contour lines and the inter-line spacing.  
#

set nmInfo(contour_lines) [create_closing_toplevel contour_lines \
        "Contour Lines Setup" ]

generic_optionmenu $nmInfo(contour_lines).contour_dataset contour_comes_from \
	"Contour plane" inputPlaneNames
pack $nmInfo(contour_lines).contour_dataset -anchor nw

# Group the set color button and sample
frame $nmInfo(contour_lines).c
    
button $nmInfo(contour_lines).c.set_color -text "Set contour color" -command {
    choose_color contour_color "Choose contour color" $nmInfo(contour_lines)
    $nmInfo(contour_lines).c.colorsample configure -bg $contour_color
    set_contour_color
}
# this sets the color of the sample frame to the color of the scales
set contour_color [format #%02x%02x%02x $contour_r $contour_g $contour_b]
button $nmInfo(contour_lines).c.colorsample \
        -relief groove -bd 2 -bg $contour_color \
        -command { $nmInfo(contour_lines).c.set_color invoke}

#Choose the width of the lines that make up the contour display
frame $nmInfo(contour_lines).lineslider
generic_entry $nmInfo(contour_lines).lineslider.linespacing \
        texture_spacing "Line spacing" real
generic_entry $nmInfo(contour_lines).lineslider.linewidth \
        contour_width "Line width" real
#generic_entry $nmInfo(contour_lines).lineslider.opacity contour_opacity "opacity" integer
# this triggers the callback to actually set the c variables
pack $nmInfo(contour_lines).c -side top -fill x
pack $nmInfo(contour_lines).c.set_color -side left -fill x
pack $nmInfo(contour_lines).c.colorsample -side left -fill x -expand yes

pack $nmInfo(contour_lines).lineslider \
        -side top -anchor w -fill x -pady 1m -padx 3m
pack $nmInfo(contour_lines).lineslider.linewidth \
	$nmInfo(contour_lines).lineslider.linespacing \
	-side top -anchor nw -pady $fspady
#pack $nmInfo(contour_lines).lineslider.opacity -side top -fill x -pady $fspady

iwidgets::Labeledwidget::alignlabels \
        $nmInfo(contour_lines).lineslider.linewidth \
	$nmInfo(contour_lines).lineslider.linespacing 

# Make it easy to create a flat plane to use for contours.
button $nmInfo(contour_lines).calc_planes -text "Calculate Data Planes..."  \
    -command "show.calc_planes"
pack $nmInfo(contour_lines).calc_planes -side bottom -anchor nw

#######
# end contour line section

######
# Alpha-blended texture section.
# Users will see it as "texture blend"

set alpha_slider_min_limit 0
set alpha_slider_max_limit 1
set alpha_slider_min 0
set alpha_slider_max 1

#
#################################    
#
# This part of the script brings up a min/max slider to control the
# mapping of values into the alpha parameter on a checkerboard.
# First, we create a toplevel widget to serve as a control panel.
# Then, we pack a min/max slider into the control panel
#
set nmInfo(alphascale) [create_closing_toplevel alphascale "Texture Blend Setup" ]

#
# It requires the the russ_widgets scripts have been executed to define
# the minmaxscale procedure.
# Also the alpha_slider_min_limit and alpha_slider_max_limit variables
# must have been set to the proper values for initialization.
# This provides control over the alpha_slider_min and alpha_slider_max
# variables.
# Set traces on the max and min variables.  If they change, destroy the
# old slider and set up a new one within the old frame.
#

minmaxscale $nmInfo(alphascale).scale $alpha_slider_min_limit \
	$alpha_slider_max_limit 50 alpha_slider_min alpha_slider_max
# Make a frame to hold the pull-down menu that selects from the list
generic_optionmenu $nmInfo(alphascale).alpha_dataset alpha_comes_from \
	"Texture blend dataset" inputPlaneNames
pack $nmInfo(alphascale).alpha_dataset -side top -fill x
pack $nmInfo(alphascale).scale -fill x -side top
trace variable alpha_slider_min_limit w alpha_scale_newscale
trace variable alpha_slider_max_limit w alpha_scale_newscale

#
# Helper routine for the alpha scale that destroys and then recreates the
# slider with new values if the endpoints change.
#

proc alpha_scale_newscale {name element op} {
        global  alpha_slider_min_limit alpha_slider_max_limit
    global nmInfo

        destroy $nmInfo(alphascale).scale
        minmaxscale $nmInfo(alphascale).scale $alpha_slider_min_limit \
                $alpha_slider_max_limit 50 alpha_slider_min alpha_slider_max
        pack $nmInfo(alphascale).scale -fill x -side top
}

##########
# end alpha blended texture section

##############
# Haptic controls, friction, bump, buzz, adhesion, compliance.

# adhesion part of UI commented out because adhesion not fully implemented

set friction_slider_min_limit 0 
set friction_slider_max_limit 1

set bump_slider_min_limit 0
set bump_slider_max_limit 1

set buzz_slider_min_limit 0
set buzz_slider_max_limit 1

#set adhesion_slider_min_limit 0 
#set adhesion_slider_max_limit 1

set compliance_slider_min_limit 0 
set compliance_slider_max_limit 1

set spring_slider_min_limit 0.01
set spring_slider_max_limit 1
set spring_k_slider 0


#proc adjust_adhesion {} {
#    global bc
#    global fspady
    
#    toplevel .a
    
#    frame .a.menubar -relief raised -bd 4
#    pack .a.menubar -side top -fill both
    
#    button .a.menubar.exitwin -bg red -text "Close Adhesion Panel" -command "destroy .a"
#    pack .a.menubar.exitwin -side top -anchor e -ipadx 3m
    
#    frame .a.label
#    label .a.label.a -text "Adhesion Model Parameters"
#    pack .a.label.a -side left
#    pack .a.label -side top -anchor n
#    frame .a.sliders
#    pack .a.sliders -side top -anchor w -fill x -pady 1m -padx 3m
    
#    floatscale .a.sliders.adh_const 6000 12000 12001 1 1 adhesion_constant "Adhesion_Constant" 
#    floatscale .a.sliders.adhesion_peak 0 100 101 1 1 adhesion_peak "Adhesion_Peak"
#    floatscale .a.sliders.adhesion_min 0 100 101 1 1 adhesion_min "Adhesion_Min"
#    floatscale .a.sliders.adhesion_decrease_per 0 10 1001 1 1 adhesion_decrease_per "Adhesion_Decrease_Per"

    #adhesion_peak and adhesion_min sliders should be
    #ganged together by minmaxscale
    #the scaling for minmax here is fixed, so no need to destroy and
    #recreate the sliders, because endpoints don't change
#    minmaxscale .a.sliders.adh_min_peak 0 100 101 adhesion_min adhesion_peak "Adhesion_Min" "Adhesion_Peak" 0 10 11

#    pack .a.sliders.adh_const .a.sliders.adhesion_decrease_per .a.sliders.adh_min_peak -side top -fill x -pady $fspady
#}

#
#################################    
#
# This part of the script brings up a min/max slider to control the
# mapping of values into friction on the surface.
# First, we create a toplevel widget to serve as a control panel.
# Then, we pack a min/max slider into the control panel
#
set nmInfo(haptic) [create_closing_toplevel haptic "Haptics Setup" ]
set nmInfo(frictionscale) [frame $nmInfo(haptic).frictionscale -relief raised -bd 4]
pack $nmInfo(frictionscale) -fill both 


#
# It requires the the russ_widgets scripts have been executed to define
# the minmaxscale procedure.
# Also the friction_slider_min_limit and friction_slider_max_limit variables
# must have been set to the proper values for initialization.
# This provides control over the friction_slider_min and friction_slider_max
# variables.
# Set traces on the max and min variables.  If they change, destroy the
# old slider and set up a new one within the old frame.
#

minmaxscale $nmInfo(frictionscale).scale $friction_slider_min_limit \
	$friction_slider_max_limit 50 friction_slider_min friction_slider_max

generic_optionmenu $nmInfo(frictionscale).friction_dataset \
        friction_comes_from "Friction plane" inputPlaneNames

checkbutton $nmInfo(frictionscale).linear -text "linearize friction" -variable \
	friction_linear

# Make a frame to hold the pull-down menu that selects from the list
frame $nmInfo(frictionscale).pickframe
pack $nmInfo(frictionscale).pickframe -side left -fill y
pack $nmInfo(frictionscale).friction_dataset
pack $nmInfo(frictionscale).scale -fill x -side top
pack $nmInfo(frictionscale).linear
trace variable friction_slider_min_limit w friction_scale_newscale
trace variable friction_slider_max_limit w friction_scale_newscale

#
# Helper routine for the friction scale that destroys and then recreates the
# slider with new values if the endpoints change.
#

proc friction_scale_newscale {name element op} {
    global  friction_slider_min_limit friction_slider_max_limit
    global nmInfo
    destroy $nmInfo(frictionscale).scale
    minmaxscale $nmInfo(frictionscale).scale $friction_slider_min_limit \
        $friction_slider_max_limit 50 friction_slider_min friction_slider_max
    pack $nmInfo(frictionscale).scale -fill x -side top
}



#
#################################
#
# This part of the script brings up a min/max slider to control the
# mapping of values into bump size on the surface.
# First, we create a toplevel widget to serve as a control panel.
# Then, we pack a min/max slider into the control panel
#
set nmInfo(bumpscale) [frame $nmInfo(haptic).bumpscale -relief raised -bd 4]
pack $nmInfo(bumpscale) -fill both 


#
# It requires the the russ_widgets scripts have been executed to define
# the minmaxscale procedure.
# Also the bump_slider_min_limit and bump_slider_max_limit variables
# must have been set to the proper values for initialization.
# This provides control over the bump_slider_min and bump_slider_max
# variables.
# Set traces on the max and min variables.  If they change, destroy the
# old slider and set up a new one within the old frame.
# Only do all this if the range is nonzero (ie, if there is something to
# control).
#


if {$bump_slider_min_limit != $bump_slider_max_limit} {
    minmaxscale $nmInfo(bumpscale).scale $bump_slider_min_limit \
        $bump_slider_max_limit 50 bump_slider_min bump_slider_max
    generic_optionmenu $nmInfo(bumpscale).bump_dataset bumpsize_comes_from \
	    "Bump plane" inputPlaneNames
    checkbutton $nmInfo(bumpscale).linear -text "linearize bumpscale" -variable \
	    bumpscale_linear
    # Make a frame to hold the pull-down menu that selects from the list
    frame $nmInfo(bumpscale).pickframe
    pack $nmInfo(bumpscale).pickframe -side left -fill y
    pack $nmInfo(bumpscale).bump_dataset $nmInfo(bumpscale).scale -fill x -side left
    pack $nmInfo(bumpscale).linear
    trace variable bump_slider_min_limit w bump_scale_newscale
    trace variable bump_slider_max_limit w bump_scale_newscale
}

#button $nmInfo(bumpscale).pickframe.setbumpsize -text "Set Bump Parameters" \
#	-command adjust_bumpsize
#pack $nmInfo(bumpscale).pickframe.setbumpsize

#
# Helper routine for the bump scale that destroys and then recreates the
# slider with new values if the endpoints change.
#

proc bump_scale_newscale {name element op} {
    global  bump_slider_min_limit bump_slider_max_limit
    global nmInfo
    destroy $nmInfo(bumpscale).scale
    minmaxscale $nmInfo(bumpscale).scale $bump_slider_min_limit \
        $bump_slider_max_limit 50 bump_slider_min bump_slider_max
    pack $nmInfo(bumpscale).scale -fill x -side top
}


#
#################################
#
# This part of the script brings up a min/max slider to control the
# mapping of values into buzzing on the surface.
# First, we create a toplevel widget to serve as a control panel.
# Then, we pack a min/max slider into the control panel
#
set nmInfo(buzzscale) [frame $nmInfo(haptic).buzzscale -relief raised -bd 4]
pack $nmInfo(buzzscale) -fill both 


#
# It requires the the russ_widgets scripts have been executed to define
# the minmaxscale procedure.
# Also the buzz_slider_min_limit and buzz_slider_max_limit variables
# must have been set to the proper values for initialization.
# This provides control over the buzz_slider_min and buzz_slider_max
# variables.
# Set traces on the max and min variables.  If they change, destroy the
# old slider and set up a new one within the old frame.
# Only do all this if the range is nonzero (ie, if there is something to
# control).
#


if {$buzz_slider_min_limit != $buzz_slider_max_limit} {
    minmaxscale $nmInfo(buzzscale).scale $buzz_slider_min_limit \
        $buzz_slider_max_limit 50 buzz_slider_min buzz_slider_max
    generic_optionmenu $nmInfo(buzzscale).buzz_dataset buzzing_comes_from \
            "Buzzing plane" inputPlaneNames
    checkbutton $nmInfo(buzzscale).linear -text "linearize buzzing" -variable \
	    buzzscale_linear
    # Make a frame to hold the pull-down menu that selects from the list
    frame $nmInfo(buzzscale).pickframe
    pack $nmInfo(buzzscale).pickframe -side left -fill y
    pack $nmInfo(buzzscale).buzz_dataset $nmInfo(buzzscale).scale -fill x \
            -side left
    pack $nmInfo(buzzscale).linear
    trace variable buzz_slider_min_limit w buzz_scale_newscale
    trace variable buzz_slider_max_limit w buzz_scale_newscale
}

#button $nmInfo(buzzscale).pickframe.setbuzzamp -text "Set Buzzing Parameters" \
#	-command adjust_buzzing
#pack $nmInfo(buzzscale).pickframe.setbuzzamp



#
# Helper routine for the buzz scale that destroys and then recreates the
# slider with new values if the endpoints change.
#

proc buzz_scale_newscale {name element op} {
	global  buzz_slider_min_limit buzz_slider_max_limit
    global nmInfo
	destroy $nmInfo(buzzscale).scale
	minmaxscale $nmInfo(buzzscale).scale $buzz_slider_min_limit \
		$buzz_slider_max_limit 50 buzz_slider_min buzz_slider_max
	pack $nmInfo(buzzscale).scale -fill x -side top
}



#
#################################
#
# This part of the script brings up a min/max slider to control the
# mapping of values into adhesion on the surface.
# First, we create a toplevel widget to serve as a control panel.
# Then, we pack a min/max slider into the control panel
#

# commented out because adhesion not fully implemented

#set nmInfo(adhesionscale) [frame $nmInfo(haptic).adhesionscale -relief raised -bd 4]
#pack $nmInfo(adhesionscale) -fill both

#generic_entry $nmInfo(adhesionscale).num_to_avg adhesion_average \
#	"Num samples to avg: (3,100???)" real
#checkbutton $nmInfo(adhesionscale).linear -text "linearize adhesion" \
#	-variable adhesion_linear

#if {$adhesion_slider_min_limit != $adhesion_slider_max_limit} {
#    minmaxscale $nmInfo(adhesionscale).scale $adhesion_slider_min_limit \
#	    $adhesion_slider_max_limit 50 adhesion_slider_min adhesion_slider_max
#    generic_optionmenu $nmInfo(adhesionscale).adhesion_dataset \
#            adhesion_comes_from "Adhesion plane" inputPlaneNames
    # Make a frame to hold the pull-down menu that selects from the list
#    frame $nmInfo(adhesionscale).pickframe
#    pack $nmInfo(adhesionscale).pickframe -side left -fill y
#    pack $nmInfo(adhesionscale).adhesion_dataset
#    pack $nmInfo(adhesionscale).scale -fill x -side left
#    trace variable adhesion_slider_min_limit w adhesion_scale_newscale
#    trace variable adhesion_slider_max_limit w adhesion_scale_newscale
#}

#button $nmInfo(adhesionscale).pickframe.setadhesion -text "Set Adhesion Parameters" -command adjust_adhesion
#pack $nmInfo(adhesionscale).pickframe.setadhesion
#pack $nmInfo(adhesionscale).num_to_avg
#pack $nmInfo(adhesionscale).linear
#
# Helper routine for the color scale that destroys and then recreates the
# slider with new values if the endpoints change.
#
#proc adhesion_scale_newscale {name element op} {
#        global  adhesion_slider_min_limit adhesion_slider_max_limit
#    global nmInfo
#        destroy $nmInfo(adhesionscale).scale
#        minmaxscale $nmInfo(adhesionscale).scale $adhesion_slider_min_limit \
#                $adhesion_slider_max_limit 50 adhesion_slider_min adhesion_slider_max
#        pack $nmInfo(adhesionscale).scale -fill x -side top
#}

#################################    
#
# This part of the script brings up a min/max slider to control the
# mapping of values into compliance on the surface.
# First, we create a toplevel widget to serve as a control panel.
# Then, we pack a min/max slider into the control panel
#
set nmInfo(compliancescale) [frame $nmInfo(haptic).compliancescale -relief raised -bd 4]
pack $nmInfo(compliancescale) -fill both

#
# It requires the the russ_widgets scripts have been executed to define
# the minmaxscale procedure.
# Also the compliance_slider_min_limit and compliance_slider_max_limit variables
# must have been set to the proper values for initialization.
# This provides control over the compliance_slider_min and compliance_slider_max
# variables.
# Set traces on the max and min variables.  If they change, destroy the
# old slider and set up a new one within the old frame.
# Only do all this if the range is nonzero (ie, if there is something to
# control).
#


if {$compliance_slider_min_limit != $compliance_slider_max_limit} {
	minmaxscale $nmInfo(compliancescale).scale $compliance_slider_min_limit \
		$compliance_slider_max_limit 50 compliance_slider_min compliance_slider_max
    generic_optionmenu $nmInfo(compliancescale).compliance_dataset \
            compliance_comes_from "Compliance plane" inputPlaneNames
    checkbutton $nmInfo(compliancescale).linear -text "linearize compliance" \
	    -variable compliance_linear
	# Make a frame to hold the pull-down menu that selects from the list
	frame $nmInfo(compliancescale).pickframe
	pack $nmInfo(compliancescale).pickframe -side left -fill y
	pack $nmInfo(compliancescale).compliance_dataset $nmInfo(compliancescale).scale -fill x -side left
    pack $nmInfo(compliancescale).linear
	trace variable compliance_slider_min_limit w compliance_scale_newscale
	trace variable compliance_slider_max_limit w compliance_scale_newscale
}

#
# Helper routine for the color scale that destroys and then recreates the
# slider with new values if the endpoints change.
#

proc compliance_scale_newscale {name element op} {
	global  compliance_slider_min_limit compliance_slider_max_limit
    global nmInfo
	destroy $nmInfo(compliancescale).scale
	minmaxscale $nmInfo(compliancescale).scale $compliance_slider_min_limit \
		$compliance_slider_max_limit 50 compliance_slider_min compliance_slider_max
	pack $nmInfo(compliancescale).scale -fill x -side top
}


#################################
#
# This part of the script brings up a min/max slider to control the
# mapping of values into the spring constant, assuming the compliance
# plane button is set to 'none'
#
set nmInfo(springkscale) [frame $nmInfo(haptic).springkscale -relief raised -bd 4]
pack $nmInfo(springkscale) -fill both

if {$spring_slider_min_limit != $spring_slider_max_limit} {
    floatscale $nmInfo(springkscale).scale $spring_slider_min_limit $spring_slider_max_limit \
	    100 1 1 spring_k_slider "Spring Constant"
    pack $nmInfo(springkscale).scale
}


############################
# display_settings

# This is for anything the user won't want to change very often, but
# which affects the general appearance of the screen and
# application. 
set nmInfo(display_settings) [create_closing_toplevel display_settings "Display Settings..." ]

#
# Surface shading parameters
#
iwidgets::Labeledframe $nmInfo(display_settings).surf_shading \
	-labeltext "Surface Shading" \
	-labelpos nw
set nmInfo(pref_surf_shading) [$nmInfo(display_settings).surf_shading childsite]
# Spec Color, Spec highlight sharpness, diffuse color, opacity of surface. 
generic_entry $nmInfo(pref_surf_shading).shiny shiny \
	"Highlight sharpness (1,128)" integer
generic_entry $nmInfo(pref_surf_shading).specular_color specular_color \
	"Amount of specular lighting (0,1)" real
generic_entry $nmInfo(pref_surf_shading).diffuse diffuse \
	"Amount of diffuse lighting (0,1)" real
generic_radiobox $nmInfo(pref_surf_shading).smooth \
	smooth_shading \
	"Shading" { "Flat" "Smooth" }
pack $nmInfo(display_settings).surf_shading -fill both
pack $nmInfo(pref_surf_shading).shiny \
	$nmInfo(pref_surf_shading).specular_color \
	$nmInfo(pref_surf_shading).diffuse \
	$nmInfo(pref_surf_shading).smooth -fill x
	
#
# Surface tesselation
#
iwidgets::Labeledframe $nmInfo(display_settings).surf_tris \
	-labeltext "Surface Tesselation" \
	-labelpos nw
set nmInfo(pref_surf_tris) [$nmInfo(display_settings).surf_tris childsite]
generic_radiobox $nmInfo(pref_surf_tris).filled \
	filled_triangles \
	"Surface triangles" { "Wireframe" "Filled" }
generic_entry $nmInfo(pref_surf_tris).surface_alpha surface_alpha \
	"Surface opacity (0,1)" real

pack $nmInfo(display_settings).surf_tris -fill both
pack $nmInfo(pref_surf_tris).filled \
	$nmInfo(pref_surf_tris).surface_alpha -fill x


#
# Icons
#
iwidgets::Labeledframe $nmInfo(display_settings).icons \
	-labeltext "Icons" \
	-labelpos nw
set nmInfo(pref_icons) [$nmInfo(display_settings).icons childsite]

set sphere_scale 12.5
generic_entry $nmInfo(pref_icons).icon_scale global_icon_scale \
	"Icon scale (0.1,10)" real
generic_entry $nmInfo(pref_icons).sphere_scale sphere_scale \
	"Sphere scale (1,100nm)" real

pack $nmInfo(display_settings).icons -fill both
pack $nmInfo(pref_icons).icon_scale $nmInfo(pref_icons).sphere_scale \
	-side top -fill x
#
# Acquisition Parameters
#
iwidgets::Labeledframe $nmInfo(display_settings).acquisition \
	-labeltext "Acquisition Parameters" \
	-labelpos nw
set nmInfo(pref_acq) [$nmInfo(display_settings).acquisition childsite]

set num_lines_to_jump_back 1000
generic_entry $nmInfo(pref_acq).jump_back num_lines_to_jump_back \
	"Post-modify jumpback" integer
checkbutton $nmInfo(pref_acq).null_data_alpha \
	-text "Alpha blend null data" \
	-variable null_data_alpha_pressed

if { !$thirdtech_ui } {
pack $nmInfo(display_settings).acquisition -fill both
pack $nmInfo(pref_acq).jump_back -side top -fill x
pack $nmInfo(pref_acq).null_data_alpha -fill x -side left
}
iwidgets::Labeledwidget::alignlabels \
	$nmInfo(pref_surf_shading).shiny \
	$nmInfo(pref_surf_shading).specular_color \
	$nmInfo(pref_surf_shading).diffuse \
	$nmInfo(pref_surf_tris).surface_alpha  \
	$nmInfo(pref_icons).icon_scale \
	$nmInfo(pref_icons).sphere_scale \
	$nmInfo(pref_acq).jump_back
#	$nmInfo(pref_acq).null_alpha_data

#
# Coupling
#
if { !$thirdtech_ui } {
iwidgets::Labeledframe $nmInfo(display_settings).coupling \
        -labeltext "Coupling" \
        -labelpos nw
set nmInfo(pref_coupling) [$nmInfo(display_settings).coupling childsite]

checkbutton $nmInfo(pref_coupling).finegrained_coupling \
        -text "fine-grained coupling" -variable finegrained_coupling

pack $nmInfo(display_settings).coupling -fill both
pack $nmInfo(pref_coupling).finegrained_coupling -side top -fill x
}

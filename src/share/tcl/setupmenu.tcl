# Provide a toplevel widget to allow
# control over which data sets are enabled in scan and touch
# mode.  
# Put in one sub-area for scan and one for touch. */
# ----------------------------------------------------------------------
set nmInfo(data_sets) [create_closing_toplevel data_sets "Datasets Setup" ]

frame $nmInfo(data_sets).scan -relief sunken -bd 2
label $nmInfo(data_sets).scan.label -text "Image Scan"
frame $nmInfo(data_sets).touch -relief sunken -bd 2
label $nmInfo(data_sets).touch.label -text Touch
label $nmInfo(data_sets).touch.instr -text "Hit Enter to change no. of samples"
frame $nmInfo(data_sets).forcecurve -relief sunken -bd 2
label $nmInfo(data_sets).forcecurve.label -text ForceCurve
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
#
# This part of the script brings up sliders to control the
# mapping of values into colors on the surface.
# First, we create a toplevel widget to serve as a control panel.
# Then, we pack center and range sliders into the control panel
#
set nmInfo(colorscale) [create_closing_toplevel colorscale "Color Map Setup" ]

# Make a frame to hold the pull-down menu that selects from the list
frame $nmInfo(colorscale).scales
frame $nmInfo(colorscale).pickframe
iwidgets::Labeledframe $nmInfo(colorscale).flat \
	-labeltext "Create a flattened plane" \
	-labelpos nw
set nmInfo(flatplane) [$nmInfo(colorscale).flat childsite]

pack  $nmInfo(colorscale).flat -side bottom -fill x
pack $nmInfo(colorscale).pickframe $nmInfo(colorscale).scales -side left -fill both

# Pop-up menu to choose which plane is displayed as a color map.
generic_optionmenu $nmInfo(colorscale).pickframe.colormap_plane \
	color_comes_from \
	"Colormap plane" inputPlaneNames

set colorMapNames {custom }
generic_optionmenu $nmInfo(colorscale).pickframe.colormap \
	color_map \
	"Colormap" colorMapNames

iwidgets::Labeledwidget::alignlabels \
	$nmInfo(colorscale).pickframe.colormap_plane \
	$nmInfo(colorscale).pickframe.colormap

pack $nmInfo(colorscale).pickframe.colormap_plane \
	$nmInfo(colorscale).pickframe.colormap -anchor nw -fill x

#
# It requires the the russ_widgets scripts have been executed to define
# the floatscale procedure.
# Also the color_slider_min_limit and color_slider_max_limit variables
# must have been set to the proper values for initialization.
# Set traces on the max_limit and min_limit variables.  If they
# change, destroy the old sliders and set up new ones within the old
# frame.
#

set color_slider_min 0
set color_slider_max 1

set color_slider_min_limit 0
set color_slider_max_limit 1

set color_slider_center [expr ($color_slider_min_limit+$color_slider_max_limit)/2.0]
set color_slider_range [expr $color_slider_max-$color_slider_min]

generic_entry $nmInfo(colorscale).scales.center color_slider_center \
	"Center ($color_slider_min_limit $color_slider_max_limit)" real
generic_entry $nmInfo(colorscale).scales.range color_slider_range \
	"Width (0 [expr $color_slider_max_limit -$color_slider_min_limit ])" \
	real

iwidgets::Labeledwidget::alignlabels \
	$nmInfo(colorscale).scales.center \
	$nmInfo(colorscale).scales.range

pack  $nmInfo(colorscale).scales.center \
	$nmInfo(colorscale).scales.range -side top -anchor nw

# Autoscales the color map, by setting color_comes_from to it's
# current value, which prompts the C code to do it.
button $nmInfo(colorscale).scales.autoscale -text "Autoscale now" \
	-command {
    set color_comes_from [$nmInfo(colorscale).pickframe.colormap_plane get]
}
pack $nmInfo(colorscale).scales.autoscale -side top -anchor nw

# Allow the user to create a flattened plane from the height plane:
label $nmInfo(flatplane).flatlabel -justify left -text \
	"Position measure lines then\nenter a plane name:"
generic_entry $nmInfo(flatplane).flatplane flatplane_name \
	"Flatten plane" ""
pack $nmInfo(flatplane).flatplane $nmInfo(flatplane).flatlabel \
	-side bottom -anchor nw


# these are semaphores, to prevent recursive callbacks from
# screwing things up. "Trace" can be very annoying!
set setting_minmax 0
set setting_cr 0

#
# Helper routine for the color scale that destroys and then recreates the
# slider with new values if the endpoints change.
# Checks for the scales existence before destroying so it is safe to 
# call the first time to create the scales. (see below)
#

proc color_scale_newscale {name element op} {
    global  color_slider_min_limit color_slider_max_limit
    global  color_slider_min color_slider_max
    global  color_slider_center color_slider_range
    global setting_cr
    global nmInfo

    # check for absurd conditions
    if {$color_slider_min > $color_slider_max} then return
    if {$color_slider_min_limit > $color_slider_max_limit} then return
    
    # Change titles to reflect new range
    # format statements keep the labels from getting really long. 
    set newtext [format "%.0f %.0f" $color_slider_min_limit $color_slider_max_limit]
    $nmInfo(colorscale).scales.center config -labeltext \
	"Center ($newtext)"
    set newtext [format "%.3f" [expr $color_slider_max_limit -$color_slider_min_limit ]]
    $nmInfo(colorscale).scales.range config -labeltext \
	"Width (0 $newtext)"

    iwidgets::Labeledwidget::alignlabels \
	$nmInfo(colorscale).scales.center \
	$nmInfo(colorscale).scales.range

    # new scale, so reset params to the default values 
    set setting_cr 1
    set color_slider_center [expr ($color_slider_max_limit+$color_slider_min_limit)/2.0]
    # decided not to expand the width of color maps above 
    # current data range - that's just annoying. But min/max of slider
    # are still wider than current data range. 
    set color_slider_range [expr ($color_slider_max_limit -$color_slider_min_limit)]
    set setting_cr 0
}

# 
# Helper routine so we can change the sliders representing the
# center and range of the color map into the min and max values
# of the color map used in the C code.
proc color_scale_change {name element op} {
    global color_slider_center color_slider_range 
    global color_slider_max color_slider_min
    global setting_minmax setting_cr

# check the semaphore to prevent recursive callbacks
    if { $setting_cr } then return

    set setting_minmax 1
    set color_slider_max [expr $color_slider_center + $color_slider_range/2.0]
    set color_slider_min [expr $color_slider_center - $color_slider_range/2.0]
    set setting_minmax 0
    #puts "csc min max $color_slider_min $color_slider_max"
}

# 
# Helper routine so if min and max get set from C, 
# we can update center and range
proc color_scale_change_from_c {name element op} {
    global color_slider_center color_slider_range 
    global color_slider_max color_slider_min
    global setting_minmax setting_cr

# check the semaphore to prevent recursive callbacks
    if { $setting_minmax } then return

    #puts "from c minmax $color_slider_min $color_slider_max"

    set setting_cr 1
    
    set color_slider_center [expr ($color_slider_min + $color_slider_max)/2.0]
    set color_slider_range [expr $color_slider_max-$color_slider_min]
    set setting_cr 0

    #puts "from c center range $color_slider_center $color_slider_range"
}

#
#Finally, create some controls for the color map.
#
# call procedure to create the scales for the first time
color_scale_newscale "" "" ""

# set up traces on variables using the procedures above. 
trace variable color_slider_min_limit w color_scale_newscale
trace variable color_slider_max_limit w color_scale_newscale
trace variable color_slider_center w color_scale_change
trace variable color_slider_range w color_scale_change
trace variable color_slider_min w color_scale_change_from_c
trace variable color_slider_max w color_scale_change_from_c


#
# A button to bring up the control panel for min/max color
# and other parameters of the CUSTOM color map. 
#
button $nmInfo(colorscale).pickframe.setcolor -text "Adjust Custom Colormap" -command adjust_color
pack $nmInfo(colorscale).pickframe.setcolor

#set these so we can see do " wishx <mainwin.tcl" and test interface
set minR 0
set minG 0
set minB 0
set maxR 255
set maxG 255
set maxB 255

proc adjust_color {} {
    global minR minG minB maxR maxG maxB color_flag surface_changed
    toplevel .c 

    set pwidth 3c

    frame .c.menubar -relief raised -bd 4
    frame .c.label
    frame .c.samplemax -height 3c -width 3c -relief groove -bd 5
    frame .c.colorbutts

    label .c.label.a -textvariable color_flag
    label .c.label.b -text "Color Setting"
    scale .c.red -label Red -from 0 -to 255 -orient horizontal \
           -command new_color -bg tan -length $pwidth -relief raised -bd 3
    scale .c.green -label Green -from 0 -to 255 -orient horizontal \
           -command new_color -bg tan -length $pwidth -relief raised -bd 3
    scale .c.blue -label Blue -from 0 -to 255 -orient horizontal \
           -command new_color -bg tan -length $pwidth -relief raised -bd 3

    pack .c.menubar -side top -fill both
    pack .c.label.a .c.label.b -side left
    pack .c.label -side top -anchor n
    pack .c.red .c.green .c.blue -side top -anchor w -fill x -pady 1m -padx 3m
    pack .c.samplemax -side top -anchor e -padx 3m -fill x
    pack .c.colorbutts -side top 
 
    button .c.menubar.exitwin -bg red -text "Close Color Panel" -command "destroy .c"
    radiobutton .c.colorbutts.min -text "set min"  -variable color_flag \
	    -value "Minimum" -bg SteelBlue -command toggle_color 
    radiobutton .c.colorbutts.max -text "set max"  -variable color_flag \
            -value "Maximum" -bg SteelBlue -command toggle_color
    button .c.colorbutts.go -bg yellow -text "OK" -command set_color

    pack .c.colorbutts.min .c.colorbutts.max  .c.colorbutts.go -side left -pady 3m \
	     -padx 3m -ipadx 1m -ipady 1m -fill both
 
    pack .c.menubar.exitwin -side top -anchor e -ipadx 3m

    toggle_color
}

proc new_color {value} {
    set color [format #%02x%02x%02x [.c.red get] [.c.green get] [.c.blue get]]
    .c.samplemax configure -bg $color
}
proc set_color {} {
    global minR minG minB maxR maxG maxB color_flag surface_changed 
    if {$color_flag=="Minimum"} {
	set minR [.c.red get]
	set minG [.c.green get]
	set minB [.c.blue get]
    }
    if {$color_flag=="Maximum"} { 
	set maxR [.c.red get]
	set maxG [.c.green get]
	set maxB [.c.blue get]
    }
    set surface_changed 1
}

proc toggle_color {} {
    global minR minG minB maxR maxG maxB color_flag
    if {$color_flag=="Minimum"} {
	.c.red set $minR
	.c.green set $minG
	.c.blue set $minB
    }
    if {$color_flag=="Maximum"} { 
	.c.red set $maxR
	.c.green set $maxG
	.c.blue set $maxB
    }
    new_color 0
}

##########
# end ColorMap section.

############################
# Contour lines
#set these so we can see do " wish mainwin.tcl" and test interface
set contour_r 100
set contour_g 100
set contour_b 100

set contour_width 1
set texture_spacing 10

proc set_contour_color {} {
    global contour_r contour_g contour_b contour_changed contour_color
    global nmInfo
    # Extract three component colors of contour_color 
    # and save into contour_r g b
    scan $contour_color #%02x%02x%02x contour_r contour_g contour_b
    set contour_changed 1
    puts "Contour color $contour_r $contour_g $contour_b"
}

#
################################
#
# This part of the script describes the framework for a control panel
# for contour lines.  This allows both the selection of the plane to use
# for mapping to contour lines and the inter-line spacing.  
#

iwidgets::dialog .contour_line_dialog -title "Adjust Contour Lines" 

.contour_line_dialog hide Help
.contour_line_dialog buttonconfigure OK -command {
    set_contour_color
    .contour_line_dialog deactivate 1
}
.contour_line_dialog buttonconfigure Apply -command {
    set_contour_color
}
# "Cancel" button is already set up correctly

proc show.contour_lines {} {
    .contour_line_dialog activate
}

set nmInfo(contour_lines) [.contour_line_dialog childsite]

generic_optionmenu $nmInfo(contour_lines).contour_dataset contour_comes_from \
	"Contour dataset" inputPlaneNames
pack $nmInfo(contour_lines).contour_dataset -anchor nw
    
button $nmInfo(contour_lines).set_color -text "Set contour color" -command {
    choose_color contour_color "Choose contour color"
    $nmInfo(contour_lines).colorsample configure -bg $contour_color
}


# this sets the color of the sample frame to the color of the scales
set contour_color [format #%02x%02x%02x $contour_r $contour_g $contour_b]
frame $nmInfo(contour_lines).colorsample -height 32 -width 32 -relief groove -bd 2 -bg $contour_color

#Choose the width of the lines that make up the contour display
frame $nmInfo(contour_lines).lineslider
floatscale $nmInfo(contour_lines).lineslider.linespacing 1 100 101 1 1 texture_spacing "Line spacing"
floatscale $nmInfo(contour_lines).lineslider.linewidth 1 100 101 1 1 contour_width "Line width"
#floatscale $nmInfo(contour_lines).lineslider.opacity 0 255 256 1 1 contour_opacity "opacity"
# this triggers the callback to actually set the c variables

pack $nmInfo(contour_lines).set_color $nmInfo(contour_lines).colorsample -side top -anchor e -pady 1m -padx 3m -fill x
pack $nmInfo(contour_lines).lineslider -side top -anchor w -fill x -pady 1m -padx 3m
pack $nmInfo(contour_lines).lineslider.linewidth \
	$nmInfo(contour_lines).lineslider.linespacing \
	-side top -fill x -pady $fspady
#pack $nmInfo(contour_lines).lineslider.opacity -side top -fill x -pady $fspady

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

proc adjust_friction {} {
    global bc
    global fspady

    toplevel .f

    frame .f.menubar -relief raised -bd 4
    pack .f.menubar -side top -fill both
    
    button .f.menubar.exitwin -bg red -text "Close Friction Panel" -command "destroy .f"
    pack .f.menubar.exitwin -side top -anchor e -ipadx 3m

    frame .f.label
    label .f.label.a -text "Friction Model Parameters"
    pack .f.label.a -side left
    pack .f.label -side top -anchor n
    frame .f.sliders
    pack .f.sliders -side top -anchor w -fill x -pady 1m -padx 3m

    floatscale .f.sliders.lateral_spring_K 5000 30000 25001 1 1 lateral_spring_K "Lateral_Spring_Constant"

    pack .f.sliders.lateral_spring_K -side top -fill x -pady $fspady
}


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

button $nmInfo(frictionscale).pickframe.setfriction -text "Set Friction Parameters" -command adjust_friction
pack $nmInfo(frictionscale).pickframe.setfriction

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


############################
# Preferences

# This is for anything the user won't want to change very often, but
# which affects the general appearance of the screen and
# application. Should be saved in a preferences file, but it won't be
# right now.
set nmInfo(preferences) [create_closing_toplevel preferences "Preferences" ]

#
# Surface shading parameters
#
iwidgets::Labeledframe $nmInfo(preferences).surf_shading \
	-labeltext "Surface Shading" \
	-labelpos nw
set nmInfo(pref_surf_shading) [$nmInfo(preferences).surf_shading childsite]
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
pack $nmInfo(preferences).surf_shading -fill both
pack $nmInfo(pref_surf_shading).shiny \
	$nmInfo(pref_surf_shading).specular_color \
	$nmInfo(pref_surf_shading).diffuse \
	$nmInfo(pref_surf_shading).smooth -fill x
	
#
# Surface tesselation
#
iwidgets::Labeledframe $nmInfo(preferences).surf_tris \
	-labeltext "Surface Tesselation" \
	-labelpos nw
set nmInfo(pref_surf_tris) [$nmInfo(preferences).surf_tris childsite]
generic_radiobox $nmInfo(pref_surf_tris).filled \
	filled_triangles \
	"Surface triangles" { "Wireframe" "Filled" }
generic_entry $nmInfo(pref_surf_tris).surface_alpha surface_alpha \
	"Surface opacity (0,1)" real

pack $nmInfo(preferences).surf_tris -fill both
pack $nmInfo(pref_surf_tris).filled \
	$nmInfo(pref_surf_tris).surface_alpha -fill x


#
# Icons
#
iwidgets::Labeledframe $nmInfo(preferences).icons \
	-labeltext "Icons" \
	-labelpos nw
set nmInfo(pref_icons) [$nmInfo(preferences).icons childsite]

set sphere_scale 12.5
generic_entry $nmInfo(pref_icons).icon_scale global_icon_scale \
	"Icon scale (1,100)" real
generic_entry $nmInfo(pref_icons).sphere_scale sphere_scale \
	"Sphere scale (1,100)" real

pack $nmInfo(preferences).icons -fill both
pack $nmInfo(pref_icons).icon_scale $nmInfo(pref_icons).sphere_scale \
	-side top -fill x

iwidgets::Labeledwidget::alignlabels \
	$nmInfo(pref_surf_shading).shiny \
	$nmInfo(pref_surf_shading).specular_color \
	$nmInfo(pref_surf_shading).diffuse \
	$nmInfo(pref_surf_tris).surface_alpha  \
	$nmInfo(pref_icons).icon_scale $nmInfo(pref_icons).sphere_scale

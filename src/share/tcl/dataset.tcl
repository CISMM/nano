 # Data sets
#   These are widgets specifically used to map or create data sets
#
# Sets up widgets inside the $dataset frame, and must be sourced
# from a specific location inside tools.tcl
#


global  x_min_scale x_max_scale
global  x_min_value x_max_value

global  color_slider_min_limit color_slider_max_limit
global  color_slider_min color_slider_max

global  friction_slider_min_limit friction_slider_max_limit
global  friction_slider_min friction_slider_max

global	bump_slider_min_limit bump_slider_max_limit
global	bump_slider_min bump_slider_max

global  adhesion_slider_min_limit adhesion_slider_max_limit
global  adhesion_slider_min adhesion_slider_max

global  compliance_slider_min_limit compliance_slider_max_limit
global  compliance_slider_min compliance_slider_max

global  sound_slider_min_limit sound_slider_max_limit
global  sound_slider_min sound_slider_max

global  alpha_slider_min_limit alpha_slider_max_limit
global  alpha_slider_min alpha_slider_max

proc updateParamChange {name element op} {
}

#
################################
#
# This part of the script enables the creation of dialog boxes that will
# allow the user to enter a string and have it set the value of a global
# variable when the "Create" button is pushed.  This is intended to be
# used to allow the user to specify the creation of new data sets.
#

proc copy_newlabel {from to} {
        global  $from $to
        set $to [eval set $from]
}

proc newlabel_dialogue {varname parent} {
        global ${varname}nfdvar

        # Make the two frames, top for the value and bottom for the buttons
        frame $parent.top -relief raised -bd 1
        pack $parent.top -side top -fill both
        frame $parent.bottom -relief raised -bd 1
        pack $parent.bottom -side bottom -fill both

        # Make a place to enter the new value, initially blank
        set ${varname}nfdvar ""
        entry $parent.top.entry -relief sunken -bd 2 \
              -textvariable ${varname}nfdvar
        pack $parent.top.entry

        # Make Create and Clear buttons.
        # Create will set the global variable to the same value as the
        #    filled-in value, and clear the filled-in value
        # Clear just clears the filled-in value
        button $parent.bottom.button1 -text "Create" -command \
             "copy_newlabel ${varname}nfdvar $varname ; set ${varname}nfdvar {}"
        button $parent.bottom.button2 -text "Clear" \
               -command "set ${varname}nfdvar {}"
        pack $parent.bottom.button1 -side left -fill x
        pack $parent.bottom.button2 -side right

        # Make Create what happens on <Return>
        # Make Clear what happens on <Escape>
        bind $parent <Return> "$parent.bottom.button1 invoke"
        bind $parent <KP_Enter> "$parent.bottom.button1 invoke"
        bind $parent <Escape> "$parent.bottom.button2 invoke"
        bind $parent.top.entry <Return> "$parent.bottom.button1 invoke"
        bind $parent.top.entry <KP_Enter> "$parent.bottom.button1 invoke"
        bind $parent.top.entry <Escape> "$parent.bottom.button2 invoke"
}


# packside1 is the -side parameter for all the widgets below
set packside1 left

frame $dataset1 -bg $bc
frame $dataset2 -bg $bc
frame $dataset3 -bg $bc
frame $dataset2.savestuff1 -bg $bc
frame $dataset2.savestuff2 -bg $bc
# -relief raised -bd 3 
set savestuff1 $dataset2.savestuff1
set savestuff2 $dataset2.savestuff2


# Adding dataset3 to split up the interface again
# This frame will contain haptic and sound controls.
frame $dataset3.mapping1 -bg $bc
frame $dataset3.mapping5 -bg $bc
frame $dataset3.mapping6 -bg $bc
# -relief raised -bd 3 

frame $dataset1.mapping2 -bg $bc
frame $dataset1.mapping3 -bg $bc
frame $dataset1.mapping4 -bg $bc

set mapping1 $dataset3.mapping1
set mapping5 $dataset3.mapping5
set mapping6 $dataset3.mapping6

set mapping2 $dataset1.mapping2
set mapping3 $dataset1.mapping3
set mapping4 $dataset1.mapping4

set x_min_scale 0 
set x_max_scale 1


set color_slider_min_limit 0
set color_slider_max_limit 1


set friction_slider_min_limit 0 
set friction_slider_max_limit 1

set bump_slider_min_limit 0
set bump_slider_max_limit 1

set buzz_slider_min_limit 0
set buzz_slider_max_limit 1

set adhesion_slider_min_limit 0 
set adhesion_slider_max_limit 1

set compliance_slider_min_limit 0 
set compliance_slider_max_limit 1

set alpha_slider_min_limit 0
set alpha_slider_max_limit 1

set sound_slider_min_limit 80
set sound_slider_max_limit 5000

frame $dataset2.create -bg $bc
# -relief raised -bd 3 
set create $dataset2.create

trace variable adhesion_constant w updateParamChange
trace variable lateral_spring_K w updateParamChange
trace variable adhesion_peak w updateParamChange
trace variable adhesion_min w updateParamChange
trace variable adhesion_decrease_per w updateParamChange


#
# Pack the frames we have set up
#
pack $dataset2.savestuff1 -side top -pady 1m -fill both
pack $dataset2.savestuff2 -side top -pady 1m -fill both
pack $dataset1.mapping2 $dataset1.mapping3 $dataset1.mapping4 \
	-side top -fill both
pack $dataset3.mapping1 $dataset3.mapping5 $dataset3.mapping6\
	-side top -fill both
pack $dataset2.create -side top -pady 1m -fill both

# keep track of whether the rulergrid has been changed
# this is traced by a variable in c
set rulergrid_changed 0

# keep track of whether user is positioning rulergrid with lines
set rulergrid_position_line 0
set rulergrid_orient_line 0

#set these so we can see do " wishx <mainwin.tcl" and test interface
set minR 0
set minG 0
set minB 0
set maxR 255
set maxG 255
set maxB 255
set polish 25

proc adjust_color {} {
    global minR minG minB maxR maxG maxB color_flag polish surface_changed
    toplevel .c -bg grey50

    set pwidth 3c

    frame .c.menubar -bg grey50 -relief raised -bd 4
    frame .c.label -bg grey50
    frame .c.samplemax -height 3c -width 3c -relief groove -bd 5
    frame .c.colorbutts -bg grey50
    frame .c.lighting -bg grey50
    frame .c.separator -height 1m -width $pwidth -relief sunken -bd 1 -bg grey50

    label .c.label.a -textvariable color_flag -bg grey50
    label .c.label.b -text "Color Setting" -bg grey50
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
    pack .c.separator -side top -fill x -padx 3m
    pack .c.lighting -side top -fill x
 
    button .c.menubar.exitwin -bg red -text "Close Color Panel" -command "destroy .c"
    radiobutton .c.colorbutts.min -text "set min"  -variable color_flag \
	    -value "Minimum" -bg SteelBlue -command toggle_color 
    radiobutton .c.colorbutts.max -text "set max"  -variable color_flag \
            -value "Maximum" -bg SteelBlue -command toggle_color
    button .c.colorbutts.go -bg yellow -text "OK" -command set_color

    pack .c.colorbutts.min .c.colorbutts.max  .c.colorbutts.go -side left -pady 3m \
	     -padx 3m -ipadx 1m -ipady 1m -fill both
 
    scale .c.lighting.shiny -label "Specular Sharpness"  -from 0 -to 128 -orient horizontal \
	    -bg tan -length $pwidth -relief raised -bd 3 -command "set polish" 
    button .c.lighting.go -bg yellow -text "OK" -command "set surface_changed 1"
    pack .c.lighting.shiny .c.lighting.go -side left -fill x -pady 3m -padx 3m -fill y -ipadx 1
    pack .c.menubar.exitwin -side top -anchor e -ipadx 3m

    toggle_color
}

proc adjust_adhesion {} {
    global bc
    global fspady
    
    toplevel .a -bg grey50
    
    frame .a.menubar -bg grey50 -relief raised -bd 4
    pack .a.menubar -side top -fill both
    
    button .a.menubar.exitwin -bg red -text "Close Adhesion Panel" -command "destroy .a"
    pack .a.menubar.exitwin -side top -anchor e -ipadx 3m
    
    frame .a.label -bg grey50
    label .a.label.a -text "Adhesion Model Parameters" -bg grey50
    pack .a.label.a -side left
    pack .a.label -side top -anchor n
    frame .a.sliders -bg grey50
    pack .a.sliders -side top -anchor w -fill x -pady 1m -padx 3m
    
    floatscale .a.sliders.adh_const 6000 12000 12001 1 1 adhesion_constant "Adhesion_Constant" 
    floatscale .a.sliders.adhesion_peak 0 100 101 1 1 adhesion_peak "Adhesion_Peak"
    floatscale .a.sliders.adhesion_min 0 100 101 1 1 adhesion_min "Adhesion_Min"
    floatscale .a.sliders.adhesion_decrease_per 0 10 1001 1 1 adhesion_decrease_per "Adhesion_Decrease_Per"

    #adhesion_peak and adhesion_min sliders should be
    #ganged together by minmaxscale
    #the scaling for minmax here is fixed, so no need to destroy and
    #recreate the sliders, because endpoints don't change
    minmaxscale .a.sliders.adh_min_peak 0 100 101 adhesion_min adhesion_peak "Adhesion_Min" "Adhesion_Peak" 0 10 11

#    pack .a.sliders.adh_const .a.sliders.adhesion_peak .a.sliders.adhesion_min .a.sliders.adhesion_decrease_per -side top -fill x -pady $fspady

    pack .a.sliders.adh_const .a.sliders.adhesion_decrease_per .a.sliders.adh_min_peak -side top -fill x -pady $fspady
}

proc adjust_friction {} {
    global bc
    global fspady

    toplevel .f -bg grey50

    frame .f.menubar -bg grey50 -relief raised -bd 4
    pack .f.menubar -side top -fill both
    
    button .f.menubar.exitwin -bg red -text "Close Friction Panel" -command "destroy .f"
    pack .f.menubar.exitwin -side top -anchor e -ipadx 3m

    frame .f.label -bg grey50
    label .f.label.a -text "Friction Model Parameters" -bg grey50
    pack .f.label.a -side left
    pack .f.label -side top -anchor n
    frame .f.sliders -bg grey50
    pack .f.sliders -side top -anchor w -fill x -pady 1m -padx 3m

    floatscale .f.sliders.lateral_spring_K 5000 30000 25001 1 1 lateral_spring_K "Lateral_Spring_Constant"

    pack .f.sliders.lateral_spring_K -side top -fill x -pady $fspady
}

################################
#
# Procedure to adjust rulergrid scaling, translation, and color

#set these so we can see do " wishx <mainwin.tcl" and test interface
set ruler_r 255
set ruler_g 255
set ruler_b 100
set rulergrid_changed 0

proc adjust_rulergrid {} {
    global fspady ruler_r ruler_g ruler_b rulergrid_changed

    set framepady 0

    toplevel .r -bg grey50

    frame .r.menubar -bg grey50 -relief raised -bd 4
    pack .r.menubar -side top -fill both

    button .r.menubar.exitwin -bg red -text "Close Rulergrid Panel" -command "destroy .r"
    pack .r.menubar.exitwin -side top -anchor e -ipadx 3m

    frame .r.left -bg grey50
    frame .r.right -bg grey50
    pack .r.left .r.right -side left -fill both -padx 2m

    frame .r.left.label -bg grey50
    label .r.left.label.a -text "Rulergrid Parameters" -bg grey50

    scale .r.right.red -label Red -from 0 -to 255 -orient horizontal \
           -command new_rulergrid_color -bg tan -relief raised -bd 3
    scale .r.right.green -label Green -from 0 -to 255 -orient horizontal \
           -command new_rulergrid_color -bg tan -relief raised -bd 3
    scale .r.right.blue -label Blue -from 0 -to 255 -orient horizontal \
           -command new_rulergrid_color -bg tan -relief raised -bd 3
    # this sets the initial values of the scales properly every time
    # the dialog pops up
    .r.right.red set $ruler_r
    .r.right.green set $ruler_g
    .r.right.blue set $ruler_b
    # this sets the color of the sample frame to the color of the scales
    set color [format #%02x%02x%02x [.r.right.red get] [.r.right.green get] [.r.right.blue get]]
    frame .r.right.colorsample -height 64 -width 46 -relief groove -bd 5 -bg $color

    #frame for the rulergrid position checkbutton
    frame .r.left.position -bg grey50
    checkbutton .r.left.position.rulergrid_pos_line -text "Set Position to Red Line" -variable rulergrid_position_line -bg tan -command set_position_choice

    #frame for sliders that disappear when using line for position
    frame .r.left.psliders -bg grey50
    floatscale .r.left.psliders.rulergrid_xoffset 0 10000 10001 1 1 rulergrid_x "rulergrid_x"
    floatscale .r.left.psliders.rulergrid_yoffset 0 10000 10001 1 1 rulergrid_y "rulergrid_y"

    #frame for scaling slider
    frame .r.left.ssliders -bg grey50
    floatscale .r.left.ssliders.rulergrid_scale 1 1000 1001 1 1 rulergrid_scale "rulergrid_scale"

    #frame for rulergrid orientation checkbutton
    frame .r.left.orient -bg grey50
    checkbutton .r.left.orient.rulergrid_orient_line -text "Set Angle to Green Line" -variable rulergrid_orient_line -bg tan -command set_orient_choice

    #frame for angle slider
    frame .r.left.asliders -bg grey50
    floatscale .r.left.asliders.rulergrid_angle 0 360 361 1 1 rulergrid_angle "rulergrid_angle"

    #frame for line width & opacity sliders
    frame .r.left.lineslider -bg grey50
    floatscale .r.left.lineslider.linewidthx 1 100 101 1 1 ruler_width_x "linewidth_x_%"
    floatscale .r.left.lineslider.linewidthy 1 100 101 1 1 ruler_width_y "linewidth_y_%"

    floatscale .r.left.lineslider.opacity 0 255 256 1 1 ruler_opacity "opacity"

    # this triggers the callback to actually set the c variables
    button .r.right.colorok -bg yellow -text "OK" -command set_rulergrid_color

    set_position_choice

    pack .r.left.label.a -side left
    pack .r.left.label -side top -anchor n

    #pack the position checkbutton
    pack .r.left.position -side top -anchor w -fill x -pady $framepady -padx 3m
    pack .r.left.position.rulergrid_pos_line -side top -fill x -pady $fspady
    #pack the position sliders
    pack .r.left.psliders -side top -anchor w -fill x -pady $framepady -padx 3m
    pack .r.left.psliders.rulergrid_xoffset -side top -fill x -pady $fspady
    pack .r.left.psliders.rulergrid_yoffset -side top -fill x -pady $fspady

    #pack the scaling sliders
    pack .r.left.ssliders -side top -anchor w -fill x -pady $framepady -padx 3m
    pack .r.left.ssliders.rulergrid_scale -side top -fill x -pady $fspady

    #pack the orient checkbutton
    pack .r.left.orient -side top -anchor w -fill x -pady $framepady -padx 3m
    pack .r.left.orient.rulergrid_orient_line -side top -fill x -pady $fspady
    #pack the orient sliders
    pack .r.left.asliders -side top -anchor w -fill x -pady $framepady -padx 3m
    pack .r.left.asliders.rulergrid_angle -side top -fill x -pady $fspady

    #pack the colors
    pack .r.right.red .r.right.green .r.right.blue -side top -anchor w \
		   -fill x -pady 1m -padx 3m
    pack .r.right.colorsample -side top -anchor e -pady $framepady -padx 3m -fill x

    #pack the linewidth and opacity
    pack .r.left.lineslider -side top -anchor w -fill x -pady $framepady -padx 3m
    pack .r.left.lineslider.linewidthx -side top -fill x -pady $fspady
    pack .r.left.lineslider.linewidthy -side top -fill x -pady $fspady
    pack .r.left.lineslider.opacity -side top -fill x -pady $fspady

    #pack the ok button for the color
    pack .r.right.colorok -side top -pady 3m -padx 3m -ipadx 1m -ipady 1m
}

#set these so we can see do " wishx <mainwin.tcl" and test interface
set contour_r 100
set contour_g 100
set contour_b 100

proc adjust_contour {} {
    global fspady contour_r contour_g contour_b contour_changed
    
    toplevel .cl -bg grey50
    frame .cl.menubar -bg grey50 -relief raised -bd 4
    pack .cl.menubar -side top -fill both

    button .cl.menubar.exitwin -bg red -text "Close Contour Line Panel" -command "destroy .cl"
    pack .cl.menubar.exitwin -side top -anchor e -ipadx 3m

    frame .cl.label -bg grey50
    label .cl.label.a -text "Contour Line Parameters" -bg grey50

    frame .cl.sliders -bg grey50
    scale .cl.red -label Red -from 0 -to 255 -orient horizontal \
           -command new_contour_color -bg tan -relief raised -bd 3
    scale .cl.green -label Green -from 0 -to 255 -orient horizontal \
           -command new_contour_color -bg tan -relief raised -bd 3
    scale .cl.blue -label Blue -from 0 -to 255 -orient horizontal \
           -command new_contour_color -bg tan -relief raised -bd 3
    # this sets the initial values of the scales properly every time
    # the dialog pops up
    .cl.red set $contour_r
    .cl.green set $contour_g
    .cl.blue set $contour_b
    # this sets the color of the sample frame to the color of the scales
    set color [format #%02x%02x%02x [.cl.red get] [.cl.green get] [.cl.blue get]]
    frame .cl.colorsample -height 64 -width 46 -relief groove -bd 5 -bg $color

    frame .cl.lineslider -bg grey50
    floatscale .cl.lineslider.linewidth 1 100 101 1 1 contour_width "linewidth"
    #floatscale .cl.lineslider.opacity 0 255 256 1 1 contour_opacity "opacity"
    # this triggers the callback to actually set the c variables
    button .cl.colorok -bg yellow -text "OK" -command set_contour_color

    pack .cl.label.a -side left
    pack .cl.label -side top -anchor n
    pack .cl.sliders -side top -anchor w -fill x -pady 1m -padx 3m
    pack .cl.red .cl.green .cl.blue -side top -anchor w -fill x -pady 1m -padx 3m
    pack .cl.colorsample -side top -anchor e -pady 1m -padx 3m -fill x
    pack .cl.lineslider -side top -anchor w -fill x -pady 1m -padx 3m
    pack .cl.lineslider.linewidth -side top -fill x -pady $fspady
    #pack .cl.lineslider.opacity -side top -fill x -pady $fspady
    pack .cl.colorok -side left -pady 3m -padx 3m -ipadx 1m -ipady 1m
}

#
################################
#
# This provides a selector to allow the user to export a file that
# holds the values for a given plane, in any of several formats
#
set export_filename ""
frame .export -relief raised -bd 4
frame .export.planechoice
frame .export.filetypechoice
frame .export.name
label .export.label1 -text "Save plane "
label .export.label2 -text " to "
label .export.label3 -text " file named "
pack .export.label1 .export.planechoice .export.label2 \
	.export.filetypechoice .export.label3 .export.name -side left
newlabel_dialogue export_filename .export.name
pack .export -fill both -side $packside1 -in $savestuff1


#
################################
#
# This provides a dialog that allows you to save a snapshot.  It only works
# with the openGL version of the code.  Use the $ command to save a snapshot
# in the PPHIGS version of the code.
#

# New interface for capturing OpenGL buffer and saving to image type.
set screenImage_filename ""
frame .screenImage -relief raised -bd 4
frame .screenImage.filetype
frame .screenImage.filename
label .screenImage.label1 -text "Save screen image to "
label .screenImage.label2 -text " file named:"
pack .screenImage.label1 .screenImage.filetype .screenImage.label2 \
     .screenImage.filename -side left
newlabel_dialogue screenImage_filename .screenImage.filename
pack .screenImage -fill both -side $packside1 -in $savestuff2

# Old interface when we used a script to do the job (Chris Weigle)
#frame .snapshot -relief raised -bd 4
#frame .snapshot.choice
#label .snapshot.label -text "Save screen to TIFF file named:"
#pack .snapshot.label -side left
#pack .snapshot.choice
#pack .snapshot -fill x -side $packside1 -in $savestuff
#
## Make a place to enter the new value, initially blank
#set snapshotentry ""
#entry .snapshot.choice.entry -relief sunken -bd 2 -textvariable snapshotentry
#pack .snapshot.choice.entry
#
## Make Create and Clear buttons.
## Create will set the global variable to the same value as the
##    filled-in value, and clear the filled-in value
## Clear just clears the filled-in value
#button .snapshot.choice.button1 -text "Save" -command \
     #{exec snap ${snapshotentry}.tif >& /dev/null; set snapshotentry {}}
#button .snapshot.choice.button2 -text "Cancel" -command "set snapshotentry {}"
#pack .snapshot.choice.button1 -side left -fill x
#pack .snapshot.choice.button2 -side right
#
## Make Create what happens on <Return>
## Make Clear what happens on <Escape>
#bind .snapshot <Return> ".snapshot.choice.button1 invoke"
#bind .snapshot <Escape> ".snapshot.choice.button2 invoke"
#bind .snapshot.choice.entry <Return> ".snapshot.choice.button1 invoke"
#bind .snapshot.choice.entry <Escape> ".snapshot.choice.button2 invoke"
#
#
##Make a button to print screen to printer
#frame .printscreen -relief raised -bd 4
#pack .printscreen -fill x -side $packside1 -in $savestuff2
#button .printscreen.print -text "Print screen" -command {exec /net/nano/nano1/bin/print}
#pack .printscreen.print -side right
#


#
################################
#
# This part of the script describes the framework for a control panel
# for the Z data.  This allows both the selection of the plane to use
# for mapping to Z and the scale to apply to that mapping.  Both of the
# tools for selecting these are created using Tcl_Linkvar variables in
# the code.
#

frame .z_mapping -relief raised -bd 4
frame .z_mapping.choice
frame .z_mapping.slider
pack .z_mapping.choice -side left
pack .z_mapping.slider
pack .z_mapping -fill both -side $packside1 -in $mapping3

#
#################################
#
# This part of the script brings up a min/max slider to control the
# mapping of values into the x_ parameter on a checkerboard.
# First, we create a toplevel widget to serve as a control panel.
# Then, we pack a min/max slider into the control panel

frame .x_scale -relief raised -bd 4
pack .x_scale -fill both -side $packside1 -in $mapping3


if {$x_min_scale != $x_max_scale} {
        minmaxscale .x_scale.scale $x_min_scale \
                $x_max_scale 50 x_min_value x_max_value
        # Make a frame to hold the pull-down menu that selects from the list
        frame .x_scale.pickframe
        pack .x_scale.pickframe -side left -fill y
        pack .x_scale.scale -fill x -side top
        trace variable x_min_scale w x_scale_newscale
        trace variable x_max_scale w x_scale_newscale
}

proc x_scale_newscale {name element op} {
        global  x_min_scale x_max_scale 


        destroy .x_scale.scale
        minmaxscale .x_scale.scale $x_min_scale \
                $x_max_scale 50 x_min_value x_max_value
        pack .x_scale.scale -fill x -side top
}


#
#################################    
#
# This part of the script brings up sliders to control the
# mapping of values into colors on the surface.
# First, we create a toplevel widget to serve as a control panel.
# Then, we pack center and range sliders into the control panel
#
frame .colorscale -relief raised -bd 4
pack .colorscale -fill both -side $packside1 -in $mapping2

# Make a frame to hold the pull-down menu that selects from the list
    frame .colorscale.scales
    frame .colorscale.pickframe
    pack .colorscale.pickframe -side left -fill y
    pack .colorscale.scales -fill x -side top

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
set color_slider_center [expr ($color_slider_min_limit+$color_slider_max_limit)/2.0]
set color_slider_range [expr $color_slider_max-$color_slider_min]

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
    global setting_cr

    upvar #0  color_slider_center center
    upvar #0 color_slider_range range
    # check for absurd conditions
    if {$color_slider_min > $color_slider_max} then return
    if {$color_slider_min_limit > $color_slider_max_limit} then return

    if { [winfo exist .colorscale.scales.center] } {
	destroy .colorscale.scales.center
    }
    if { [winfo exist .colorscale.scales.range] } {
	destroy .colorscale.scales.range
    }
    
    floatscale .colorscale.scales.center $color_slider_min_limit \
	$color_slider_max_limit 50 1 1 color_slider_center "Center"
    floatscale .colorscale.scales.range 0 \
	[expr $color_slider_max_limit -$color_slider_min_limit ] 50 1 1 \
	color_slider_range "Width"
    pack .colorscale.scales.center .colorscale.scales.range -fill x -side top

    # new scale, so reset params to the default values 
    set setting_cr 1
    set center [expr ($color_slider_max+$color_slider_min)/2.0]
    # decided not to expand the width of color maps above 
    # current data range - that's just annoying. But min/max of slider
    # are still wider than current data range. 
    set range [expr ($color_slider_max -$color_slider_min)]
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
if {$color_slider_min_limit != $color_slider_max_limit} {

    # call procedure to create the scales for the first time
    color_scale_newscale "" "" ""

    # set up traces on variables using the procedures above. 
    trace variable color_slider_min_limit w color_scale_newscale
    trace variable color_slider_max_limit w color_scale_newscale
    trace variable color_slider_center w color_scale_change
    trace variable color_slider_range w color_scale_change
    trace variable color_slider_min w color_scale_change_from_c
    trace variable color_slider_max w color_scale_change_from_c
}


#
# A button to bring up the control panel for min/max color
# and other parameters of the CUSTOM color map. 
#
button .colorscale.pickframe.setcolor -text "Set Color Parameters" -command adjust_color
pack .colorscale.pickframe.setcolor

#
#################################    
#
# This part of the script brings up a min/max slider to control the
# mapping of values into sound on the surface.
# First, we create a toplevel widget to serve as a control panel.
# Then, we pack a min/max slider into the control panel
#

#frame .sound_mapping -relief raised -bd 4
#frame .sound_mapping.choice
#frame .sound_mapping.slider
#pack .sound_mapping.choice -side left
#pack .sound_mapping.slider
#pack .sound_mapping -fill both -side $packside1 -in $mapping3

frame .soundscale -relief raised -bd 4
pack .soundscale -fill both -side $packside1 -in $mapping6

if {$sound_slider_min_limit != $sound_slider_max_limit} {
	minmaxscale .soundscale.scale $sound_slider_min_limit \
		$sound_slider_max_limit 50 sound_slider_min sound_slider_max
	# Make a frame to hold the pull-down menu that selects from the list
	frame .soundscale.pickframe
	pack .soundscale.pickframe -side left -fill y
	pack .soundscale.scale -fill x -side top
	trace variable sound_slider_min_limit w sound_scale_newscale
	trace variable sound_slider_max_limit w sound_scale_newscale
}

proc sound_scale_newscale {name element op} {
	global  sound_slider_min_limit sound_slider_max_limit

	destroy .soundscale.scale
	minmaxscale .soundscale.scale $sound_slider_min_limit \
		$sound_slider_max_limit 50 sound_slider_min sound_slider_max
	pack .soundscale.scale -fill x -side top
}

#
#################################    
#
# This part of the script brings up a min/max slider to control the
# mapping of values into friction on the surface.
# First, we create a toplevel widget to serve as a control panel.
# Then, we pack a min/max slider into the control panel
#
frame .frictionscale -relief raised -bd 4
pack .frictionscale -fill both -side $packside1 -in $mapping1


#
# It requires the the russ_widgets scripts have been executed to define
# the minmaxscale procedure.
# Also the friction_slider_min_limit and friction_slider_max_limit variables
# must have been set to the proper values for initialization.
# This provides control over the friction_slider_min and friction_slider_max
# variables.
# Set traces on the max and min variables.  If they change, destroy the
# old slider and set up a new one within the old frame.
# Only do all this if the range is nonzero (ie, if there is something to
# control).
#


if {$friction_slider_min_limit != $friction_slider_max_limit} {
	minmaxscale .frictionscale.scale $friction_slider_min_limit \
		$friction_slider_max_limit 50 friction_slider_min friction_slider_max
	# Make a frame to hold the pull-down menu that selects from the list
	frame .frictionscale.pickframe
	pack .frictionscale.pickframe -side left -fill y
	pack .frictionscale.scale -fill x -side top
	trace variable friction_slider_min_limit w friction_scale_newscale
	trace variable friction_slider_max_limit w friction_scale_newscale
}

button .frictionscale.pickframe.setfriction -text "Set Friction Parameters" -command adjust_friction
pack .frictionscale.pickframe.setfriction 

#
# Helper routine for the friction scale that destroys and then recreates the
# slider with new values if the endpoints change.
#

proc friction_scale_newscale {name element op} {
    global  friction_slider_min_limit friction_slider_max_limit

    destroy .frictionscale.scale
    minmaxscale .frictionscale.scale $friction_slider_min_limit \
        $friction_slider_max_limit 50 friction_slider_min friction_slider_max
    pack .frictionscale.scale -fill x -side top
}



#
#################################
#
# This part of the script brings up a min/max slider to control the
# mapping of values into bump size on the surface.
# First, we create a toplevel widget to serve as a control panel.
# Then, we pack a min/max slider into the control panel
#
frame .bumpscale -relief raised -bd 4
pack .bumpscale -fill both -side $packside1 -in $mapping5


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
    minmaxscale .bumpscale.scale $bump_slider_min_limit \
        $bump_slider_max_limit 50 bump_slider_min bump_slider_max
    # Make a frame to hold the pull-down menu that selects from the list
    frame .bumpscale.pickframe
    pack .bumpscale.pickframe -side left -fill y
    pack .bumpscale.scale -fill x -side top
    trace variable bump_slider_min_limit w bump_scale_newscale
    trace variable bump_slider_max_limit w bump_scale_newscale
}

#button .bumpscale.pickframe.setbumpsize -text "Set Bump Parameters" \
#	-command adjust_bumpsize
#pack .bumpscale.pickframe.setbumpsize

#
# Helper routine for the bump scale that destroys and then recreates the
# slider with new values if the endpoints change.
#

proc bump_scale_newscale {name element op} {
    global  bump_slider_min_limit bump_slider_max_limit

    destroy .bumpscale.scale
    minmaxscale .bumpscale.scale $bump_slider_min_limit \
        $bump_slider_max_limit 50 bump_slider_min bump_slider_max
    pack .bumpscale.scale -fill x -side top
}


#
#################################
#
# This part of the script brings up a min/max slider to control the
# mapping of values into buzzing on the surface.
# First, we create a toplevel widget to serve as a control panel.
# Then, we pack a min/max slider into the control panel
#
frame .buzzscale -relief raised -bd 4
pack .buzzscale -fill both -side $packside1 -in $mapping5


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
    minmaxscale .buzzscale.scale $buzz_slider_min_limit \
        $buzz_slider_max_limit 50 buzz_slider_min buzz_slider_max
    # Make a frame to hold the pull-down menu that selects from the list
    frame .buzzscale.pickframe
    pack .buzzscale.pickframe -side left -fill y
    pack .buzzscale.scale -fill x -side top
    trace variable buzz_slider_min_limit w buzz_scale_newscale
    trace variable buzz_slider_max_limit w buzz_scale_newscale
}

#button .buzzscale.pickframe.setbuzzamp -text "Set Buzzing Parameters" \
#	-command adjust_buzzing
#pack .buzzscale.pickframe.setbuzzamp



#
# Helper routine for the buzz scale that destroys and then recreates the
# slider with new values if the endpoints change.
#

proc buzz_scale_newscale {name element op} {
	global  buzz_slider_min_limit buzz_slider_max_limit

	destroy .buzzscale.scale
	minmaxscale .buzzscale.scale $buzz_slider_min_limit \
		$buzz_slider_max_limit 50 buzz_slider_min buzz_slider_max
	pack .buzzscale.scale -fill x -side top
}



#
#################################
#
# This part of the script brings up a min/max slider to control the
# mapping of values into adhesion on the surface.
# First, we create a toplevel widget to serve as a control panel.
# Then, we pack a min/max slider into the control panel
#

frame .adhesionscale -relief raised -bd 4
pack .adhesionscale -fill both -side $packside1 -in $mapping1



if {$adhesion_slider_min_limit != $adhesion_slider_max_limit} {
        minmaxscale .adhesionscale.scale $adhesion_slider_min_limit \
                $adhesion_slider_max_limit 50 adhesion_slider_min adhesion_slider_max
        # Make a frame to hold the pull-down menu that selects from the list
        frame .adhesionscale.pickframe
        pack .adhesionscale.pickframe -side left -fill y
        pack .adhesionscale.scale -fill x -side top
        trace variable adhesion_slider_min_limit w adhesion_scale_newscale
        trace variable adhesion_slider_max_limit w adhesion_scale_newscale
}

button .adhesionscale.pickframe.setadhesion -text "Set Adhesion Parameters" -command adjust_adhesion
pack .adhesionscale.pickframe.setadhesion

#
# Helper routine for the color scale that destroys and then recreates the
# slider with new values if the endpoints change.
#
proc adhesion_scale_newscale {name element op} {
        global  adhesion_slider_min_limit adhesion_slider_max_limit

        destroy .adhesionscale.scale
        minmaxscale .adhesionscale.scale $adhesion_slider_min_limit \
                $adhesion_slider_max_limit 50 adhesion_slider_min adhesion_slider_max
        pack .adhesionscale.scale -fill x -side top
}




#################################    
#
# This part of the script brings up a min/max slider to control the
# mapping of values into compliance on the surface.
# First, we create a toplevel widget to serve as a control panel.
# Then, we pack a min/max slider into the control panel
#
frame .compliancescale -relief raised -bd 4
pack .compliancescale -fill both -side $packside1 -in $mapping1

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
	minmaxscale .compliancescale.scale $compliance_slider_min_limit \
		$compliance_slider_max_limit 50 compliance_slider_min compliance_slider_max
	# Make a frame to hold the pull-down menu that selects from the list
	frame .compliancescale.pickframe
	pack .compliancescale.pickframe -side left -fill y
	pack .compliancescale.scale -fill x -side top
	trace variable compliance_slider_min_limit w compliance_scale_newscale
	trace variable compliance_slider_max_limit w compliance_scale_newscale
}

#
# Helper routine for the color scale that destroys and then recreates the
# slider with new values if the endpoints change.
#

proc compliance_scale_newscale {name element op} {
	global  compliance_slider_min_limit compliance_slider_max_limit

	destroy .compliancescale.scale
	minmaxscale .compliancescale.scale $compliance_slider_min_limit \
		$compliance_slider_max_limit 50 compliance_slider_min compliance_slider_max
	pack .compliancescale.scale -fill x -side top
}



#
################################
#
# This part of the script describes the framework for a control panel
# for contour lines.  This allows both the selection of the plane to use
# for mapping to contour lines and the inter-line spacing.  Both of the
# tools for selecting these are created using Tcl_Linkvar variables in
# the code.
#


frame .contour_lines -relief raised -bd 4
frame .contour_lines.choice
frame .contour_lines.slider
button .contour_lines.choice.but -text "Set Contour Parameters" -command adjust_contour
pack .contour_lines.choice -side left
pack .contour_lines.choice.but -side top
pack .contour_lines.slider
pack .contour_lines -fill both -side $packside1 -in $mapping2

#
#################################    
#
# This part of the script brings up a min/max slider to control the
# mapping of values into the alpha parameter on a checkerboard.
# First, we create a toplevel widget to serve as a control panel.
# Then, we pack a min/max slider into the control panel
#
frame .alphascale -relief raised -bd 4
pack .alphascale -fill both -side $packside1 -in $mapping2

#
# It requires the the russ_widgets scripts have been executed to define
# the minmaxscale procedure.
# Also the alpha_slider_min_limit and alpha_slider_max_limit variables
# must have been set to the proper values for initialization.
# This provides control over the alpha_slider_min and alpha_slider_max
# variables.
# Set traces on the max and min variables.  If they change, destroy the
# old slider and set up a new one within the old frame.
# Only do all this if the range is nonzero (ie, if there is something to
# control).
#


if {$alpha_slider_min_limit != $alpha_slider_max_limit} {
        minmaxscale .alphascale.scale $alpha_slider_min_limit \
                $alpha_slider_max_limit 50 alpha_slider_min alpha_slider_max
        # Make a frame to hold the pull-down menu that selects from the list
        frame .alphascale.pickframe
        pack .alphascale.pickframe -side left -fill y
        pack .alphascale.scale -fill x -side top
        trace variable alpha_slider_min_limit w alpha_scale_newscale
        trace variable alpha_slider_max_limit w alpha_scale_newscale
}

#
# Helper routine for the alpha scale that destroys and then recreates the
# slider with new values if the endpoints change.
#

proc alpha_scale_newscale {name element op} {
        global  alpha_slider_min_limit alpha_slider_max_limit

        destroy .alphascale.scale
        minmaxscale .alphascale.scale $alpha_slider_min_limit \
                $alpha_slider_max_limit 50 alpha_slider_min alpha_slider_max
        pack .alphascale.scale -fill x -side top
}





#
#################################    
#
# This part of the script brings up widgets to control use of 
# external image processing modules on our image data.
#
frame .procimage -relief raised -bd 4
pack .procimage -fill both -side $packside1 -in $dataset2

# Make a frame to hold the pull-down menu that selects from the list
frame .procimage.c1
frame .procimage.c2
frame .procimage.c3

pack .procimage.c1 .procimage.c2 .procimage.c3 -side left -fill y


#
# Stuff to start the image processing going.
#
label .procimage.c2.plabel -text "Program Parameters"
set proc_params ""
entry .procimage.c2.entry -relief sunken -bd 2 \
	-textvariable proc_params
label .procimage.c2.dlabel -text "Filtered Plane Name"
pack .procimage.c2.plabel .procimage.c2.entry .procimage.c2.dlabel -side top
newlabel_dialogue filterplane_name .procimage.c2

#
# Genetic Textures
#    global variable and controls.
#    C code depends on a .genetic_texture frame
set gen_tex_activate 0
set gen_send_data 0
frame .genetic_texture -relief raised -bd 4
button .genetic_texture.button -text "Set Genetic Texture Parameters" -bg $fc \
 -command "set gen_tex_activate 1"

# Genetic Texture Button & Toggle
pack .genetic_texture -fill both -side $packside1 -in $dataset2 
pack .genetic_texture.button 
#-side left



###########################
#
# Also put the rulergrid parameters in this mapping section, because
# the sliders dialog is getting too full
#
frame .rulergrid -relief raised -bd 4
pack .rulergrid -fill both -side $packside1 -in $mapping4

frame .rulergrid.pickframe
pack .rulergrid.pickframe -side left -fill y

button .rulergrid.pickframe.setrulergrid -text "Set Rulergrid Parameters" -command adjust_rulergrid
pack .rulergrid.pickframe.setrulergrid

frame .rulergrid.pickframe.onoff
pack .rulergrid.pickframe.onoff 

###########################
#
# Create a slider to scale the sphere icon
frame .sphere -relief raised -bd 4
pack .sphere -fill both -side $packside1 -in $mapping4
frame .sphere.slider
frame .sphere.icons
pack .sphere.slider .sphere.icons
floatscale .sphere.icons.icons_scale 0.1 1 101 1 1 global_icon_scale "icon scale"
 pack .sphere.icons.icons_scale

###########################
#
# This frame controls the appearance of the surface, 
# through the specular shininess, color and opaqueness.
frame .surfaceprop -relief raised -bd 4
pack .surfaceprop -fill both -side $packside1 -in $mapping4
# these sliders should only have integer values.
intscale .surfaceprop.shiny 1 100 100 1 1 shiny "Specular shinyness"
floatscale .surfaceprop.diffuse 0 1.0 100 1 1 diffuse "Diffuse lighting"

floatscale .surfaceprop.spec_color 0 1.0 100 1 1 specular_color "Specular color"
floatscale .surfaceprop.surface_alpha 0 1.0 100 1 1 surface_alpha "Alpha (Opacity)"

pack .surfaceprop.shiny .surfaceprop.spec_color .surfaceprop.diffuse .surfaceprop.surface_alpha

# This section defines the new dialog boxes for creation, using the above
# routines to get the dialogs.  It will allow creation of flattened, subtraction
# or adhesion planes.  In each case, the pull-down menus that define
# the set of input planes to use should be placed into the empty frames left
# for them by each dialog box below.

set flatplane_name ""
set flatten_from ""
frame .flatten_plane -relief raised -bd 4
frame .flatten_plane.choice
frame .flatten_plane.name
pack .flatten_plane.choice -side left
pack .flatten_plane.name
label .flatten_plane.name.label -text "Flatten plane name"
pack .flatten_plane.name.label
newlabel_dialogue flatplane_name .flatten_plane.name
pack .flatten_plane -fill both -side $packside1 -in $create


#Added by Amy Henderson 1-9-99
set lblflatplane_name ""
set lblflatten_from ""
frame .lblflatten_plane -relief raised -bd 4
frame .lblflatten_plane.choice
frame .lblflatten_plane.name
pack .lblflatten_plane.choice -side left
pack .lblflatten_plane.name
label .lblflatten_plane.name.label -text "Line-by-line flatten plane name"
pack .lblflatten_plane.name.label
newlabel_dialogue lblflatplane_name .lblflatten_plane.name
pack .lblflatten_plane -fill both -side $packside1 -in $create


set sumplane_name ""
frame .sum_plane -relief raised -bd 4
frame .sum_plane.choice
frame .sum_plane.name
pack .sum_plane.choice -side left
pack .sum_plane.name
label .sum_plane.name.label -text "Sum plane name"
pack .sum_plane.name.label
newlabel_dialogue sumplane_name .sum_plane.name
pack .sum_plane -fill both -side $packside1 -in $create

set adhesionplane_name ""
frame .adhesion_plane -relief raised -bd 4
frame .adhesion_plane.choice
frame .adhesion_plane.name
pack .adhesion_plane.choice -side left
pack .adhesion_plane.name
label .adhesion_plane.name.label -text "Adhesion plane name"
pack .adhesion_plane.name.label
newlabel_dialogue adhesionplane_name .adhesion_plane.name
pack .adhesion_plane -fill both -side $packside1 -in $create

#
################################
#

proc new_color {value} {
    set color [format #%02x%02x%02x [.c.red get] [.c.green get] [.c.blue get]]
    .c.samplemax configure -bg $color
}

proc new_rulergrid_color {value} {
    set color [format #%02x%02x%02x [.r.right.red get] [.r.right.green get] [.r.right.blue get]]
    .r.right.colorsample configure -bg $color
}

proc set_rulergrid_color {} {
    global ruler_r ruler_g ruler_b rulergrid_changed
    set ruler_r [.r.right.red get]
    set ruler_g [.r.right.green get]
    set ruler_b [.r.right.blue get]
    set rulergrid_changed 1
}

proc new_contour_color {value} {
    set color [format #%02x%02x%02x [.cl.red get] [.cl.green get] [.cl.blue get]]
    .cl.colorsample configure -bg $color
}

proc set_contour_color {} {
    global contour_r contour_g contour_b contour_changed
    set contour_r [.cl.red get]
    set contour_g [.cl.green get]
    set contour_b [.cl.blue get]
    set contour_changed 1
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
    global minR minG minB maxR maxG maxB color_flag polish 
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
    .c.lighting.shiny set $polish
    new_color 0
}

proc set_position_choice {} {
    global rulergrid_position_line 
    global fspady
    #if using the red line, disable the position sliders
    if {$rulergrid_position_line == 1} {
	.r.left.psliders.rulergrid_xoffset.s.scale configure -state disabled
	.r.left.psliders.rulergrid_yoffset.s.scale configure -state disabled
    } elseif {$rulergrid_position_line == 0} {
	.r.left.psliders.rulergrid_xoffset.s.scale configure -state normal
	.r.left.psliders.rulergrid_yoffset.s.scale configure -state normal
    }
}

proc set_orient_choice {} {
    global rulergrid_orient_line
    #if using the green line, disable the angle sliders
    if {$rulergrid_orient_line == 1} {
	.r.left.asliders.rulergrid_angle.s.scale configure -state disabled
    } elseif {$rulergrid_orient_line == 0} {
	.r.left.asliders.rulergrid_angle.s.scale configure -state normal
    }
}

#
# Realigning Textures
#
# The following procedures and whatnot are for realigning
# textures interactively. Search for "End of Realign Texture TCL Code"
# to find the end of this section.
#

#
# -- This procedure creates the realign textures help window --
# prints what the mouse buttons do...
#
proc realign_texture_help {} {
    toplevel .realign_help
    label .realign_help.operations -text "Operations:"
    pack .realign_help.operations -anchor w
    label .realign_help.rotate -text "Left button -- Rotate"
    pack .realign_help.rotate
    label .realign_help.translate -text "Middle button -- Translate"
    pack .realign_help.translate
    label .realign_help.scale -text "Right button -- Scale"
    pack .realign_help.scale
    label .realign_help.zoom -text "Left + Middle buttons -- Uniform Scale"
    pack .realign_help.zoom 
    label .realign_help.shear -text "Middle + Right buttons -- Shear"
    pack .realign_help.shear 
    label .realign_help.null1 -text ""
    pack .realign_help.null1 
    label .realign_help.center1 -text "To change the center about which the transformations"
    label .realign_help.center2 -text "are performed, select the \"set_center\" toggle button"
    label .realign_help.center3 -text "and move the red sphere with the middle mouse button"
    pack .realign_help.center1  .realign_help.center2  .realign_help.center3
    label .realign_help.null2 -text ""
    pack .realign_help.null2 
    button .realign_help.button -text "Close" -bg red -command "destroy .realign_help"
    pack .realign_help.button

}

#
# Frames for the realign texture portion of the Data Mapping Window:
#
#
frame .realign_texture -relief raised -bd 4
pack .realign_texture -fill both -side $packside1 -in $mapping4
frame .realign_texture.scales
frame .realign_texture.selection
pack .realign_texture.selection .realign_texture.scales -side left


#
# Tcl dialog for saving the realigned texture as a new
# plane.
#
set realignplane_name ""
frame .realign_texture.realign_plane
frame .realign_texture.realign_plane.choice
frame .realign_texture.realign_plane.name
pack .realign_texture.realign_plane.choice -side left
pack .realign_texture.realign_plane.name
label .realign_texture.realign_plane.name.label -text "Realign plane name"
pack .realign_texture.realign_plane.name.label
newlabel_dialogue realignplane_name .realign_texture.realign_plane.name
pack .realign_texture.realign_plane

#
# Realign Texture Help Button
#
button .realign_texture.button -text "Help" -command realign_texture_help
pack .realign_texture.button .realign_texture.scales

#
# The rest of the realign tcl code is for the color map
# center and width scales:
#
set realign_textures_slider_min_limit 0
set realign_textures_slider_max_limit 1.0
set realign_textures_slider_center [expr ($realign_textures_slider_min_limit+$realign_textures_slider_max_limit)/2.0]
set realign_textures_slider_range [expr $realign_textures_slider_max_limit-$realign_textures_slider_min_limit]
set realign_textures_slider_min 0
set realign_textures_slider_max 1

# these are semaphores, to prevent recursive callbacks from
# screwing things up. "Trace" can be very annoying!
set rt_setting_minmax 0
set rt_setting_cr 0

#
# Helper routine for the realign_textures scale that destroys and then recreates the
# slider with new values if the endpoints change.
# Checks for the scales existence before destroying so it is safe to 
# call the first time to create the scales. (see below)
#

proc realign_textures_scale_newscale {name element op} {
    global  realign_textures_slider_min_limit realign_textures_slider_max_limit
    global  realign_textures_slider_min realign_textures_slider_max

    upvar #0  realign_textures_slider_center center
    upvar #0 realign_textures_slider_range range

    # check for absurd conditions
    if {$realign_textures_slider_min > $realign_textures_slider_max} then return
    if {$realign_textures_slider_min_limit > $realign_textures_slider_max_limit} then return

    if { [winfo exist .realign_texture.scales.center] } {
	destroy .realign_texture.scales.center
    }
    if { [winfo exist .realign_texture.scales.range] } {
	destroy .realign_texture.scales.range
    }
    
    floatscale .realign_texture.scales.center $realign_textures_slider_min_limit \
	$realign_textures_slider_max_limit 50 1 1 realign_textures_slider_center "Center"
    floatscale .realign_texture.scales.range 0 \
	[expr $realign_textures_slider_max_limit -$realign_textures_slider_min_limit ] 50 1 1 \
	realign_textures_slider_range "Width"
    pack .realign_texture.scales.center .realign_texture.scales.range -fill x -side top

    # new scale, so reset params to the default values 
    set center [expr ($realign_textures_slider_max_limit+$realign_textures_slider_min_limit)/2.0]
    # set the range to 1/2 it's maximum value, b/c it's useful for
    # streamfiles
    set range [expr ($realign_textures_slider_max_limit -$realign_textures_slider_min_limit)/2.0]

}

# 
# Helper routine so we can change the sliders representing the
# center and range of the color map into the min and max values
# of the color map used in the C code.
proc realign_textures_scale_change {name element op} {
    global realign_textures_slider_center realign_textures_slider_range 
    global realign_textures_slider_max realign_textures_slider_min
    global rt_setting_minmax rt_setting_cr

# check the semaphore to prevent recursive callbacks
    if { $rt_setting_cr } then return

    set rt_setting_minmax 1
    set realign_textures_slider_max [expr $realign_textures_slider_center + $realign_textures_slider_range/2.0]
    set realign_textures_slider_min [expr $realign_textures_slider_center - $realign_textures_slider_range/2.0]
    set rt_setting_minmax 0
    #puts "csc min max $realign_textures_slider_min $realign_textures_slider_max"
}

# 
# Helper routine so if min and max get set from C, 
# we can update center and range
proc realign_textures_scale_change_from_c {name element op} {
    global realign_textures_slider_center realign_textures_slider_range 
    global realign_textures_slider_max realign_textures_slider_min
    global rt_setting_minmax rt_setting_cr

# check the semaphore to prevent recursive callbacks
    if { $rt_setting_minmax } then return

    #puts "from c minmax $realign_textures_slider_min $realign_textures_slider_max"

    set rt_setting_cr 1
    
    set realign_textures_slider_center [expr ($realign_textures_slider_min + $realign_textures_slider_max)/2.0]
    set realign_textures_slider_range [expr $realign_textures_slider_max-$realign_textures_slider_min]
    set rt_setting_cr 0

    #puts "from c center range $realign_textures_slider_center $realign_textures_slider_range"
}

#
#Finally, create some controls for the color map.
#
if {$realign_textures_slider_min_limit != $realign_textures_slider_max_limit} {

    # call procedure to create the scales for the first time
    realign_textures_scale_newscale "" "" ""

    # set up traces on variables using the procedures above. 
    trace variable realign_textures_slider_min_limit w realign_textures_scale_newscale
    trace variable realign_textures_slider_max_limit w realign_textures_scale_newscale
    trace variable realign_textures_slider_center w realign_textures_scale_change
    trace variable realign_textures_slider_range w realign_textures_scale_change
    trace variable realign_textures_slider_min w realign_textures_scale_change_from_c
    trace variable realign_textures_slider_max w realign_textures_scale_change_from_c
}

#
# End of Realign Texture TCL Code
#


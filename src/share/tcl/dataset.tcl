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




set sound_slider_min_limit 80
set sound_slider_max_limit 5000

frame $dataset2.create -bg $bc
# -relief raised -bd 3 
set create $dataset2.create


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


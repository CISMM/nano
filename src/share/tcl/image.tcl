# This file sets up the $image frame, and must be sourced from a
# specific location inside tools.tcl
# 
# for widgets that change behavior of image mode
#
 
frame $image -relief raised -bd 3 -bg $bc
frame $image.mode -bg $bc
frame $image.modeparam -bg $bc
frame $image.style -bg $bc
frame $image.styleparam -bg $bc
frame $image.tool -bg $bc
frame $image.toolparam -bg $bc

# Tool variable initialization
# in the real thing these will be inherited from microscope - i think

# traced by C code so it knows there are new image parameters
set accepted_image_params 0

#variables linked to the C code.
set imagep_mode 0
set imagep_style 0
set imagep_tool 0

set imagep_setpoint 50.0
set imagep_p_gain 1.0
set imagep_i_gain 0.3
set imagep_d_gain 0.0
set imagep_amplitude 0.1
set imagep_rate 1.0
#set imagep_tri_size 1
#set imagep_tri_speed 5000

# list of all the variables above, imagep_*
set imageplist "{mode style tool setpoint p_gain i_gain d_gain amplitude rate}"
# removed tri_speed and tri_size

# These variables only exist in tcl - the user changes
# them, and then the "accept" button copies them into the vars
# above, so the C code sees them.
set newimagep_mode $imagep_mode
set newimagep_style $imagep_style
set newimagep_tool $imagep_tool

set newimagep_setpoint $imagep_setpoint
set newimagep_p_gain $imagep_p_gain
set newimagep_i_gain $imagep_i_gain
set newimagep_d_gain $imagep_d_gain
set newimagep_amplitude $imagep_amplitude
set newimagep_rate $imagep_rate
#set newimagep_tri_size $imagep_tri_size
#set newimagep_tri_speed $imagep_tri_speed

# flips between sets of parameters
trace variable newimagep_mode w flip_im_mode
trace variable newimagep_style w flip_im_style

# checks to see if C code changes values of our variables.
trace variable imagep_mode w updateFromC
trace variable imagep_style w updateFromC 
trace variable imagep_tool  w updateFromC

trace variable imagep_setpoint  w updateFromC
trace variable imagep_p_gain  w updateFromC
trace variable imagep_i_gain  w updateFromC
trace variable imagep_d_gain  w updateFromC
trace variable imagep_amplitude  w updateFromC
trace variable imagep_rate    w updateFromC
#trace variable imagep_tri_size  w updateFromC
#trace variable imagep_tri_speed  w updateFromC

# Changes the accept and cancel buttons to pink
# when these variables change, as a reminder to user.
trace variable newimagep_mode w imBackgChReal
trace variable newimagep_style w imBackgChReal
trace variable newimagep_tool w imBackgChReal
trace variable newimagep_setpoint w imBackgChReal
trace variable newimagep_p_gain  w imBackgChReal
trace variable newimagep_i_gain  w imBackgChReal
trace variable newimagep_d_gain  w imBackgChReal
trace variable newimagep_amplitude  w imBackgChReal
trace variable newimagep_rate    w imBackgChReal
#trace variable newimagep_tri_size  w imBackgChReal
#trace variable newimagep_tri_speed  w imBackgChReal

#
#setup Image box
#
pack $image.mode $image.modeparam $image.style $image.styleparam $image.tool \
	$image.toolparam -side left -padx 3m -fill both

#setup Image mode box
label $image.mode.label -text "Image Mode" -bg $bc
pack $image.mode.label -side top -anchor nw
radiobutton $image.mode.oscillating -text "Oscillating" -variable newimagep_mode -value 0 -bg $fc
radiobutton $image.mode.contact -text "Contact" -variable newimagep_mode -value 1 -bg $fc
button $image.mode.accept -text "Accept" -bg $fc -command "acceptImageVars $imageplist"
button $image.mode.cancel -text "Cancel" -bg $fc -command "cancelImageVars $imageplist"
pack $image.mode.oscillating $image.mode.contact -side top -anchor nw -fill x
pack $image.mode.cancel $image.mode.accept -side bottom -fill x

#setup Image modeparam box
label $image.modeparam.label -text "Mode parameters" -bg $bc
pack $image.modeparam.label -side top -anchor nw 

floatscale $image.modeparam.setpoint 0 100 101 1 1 newimagep_setpoint \
	"Set Point" 
floatscale $image.modeparam.p-gain 0 5 51 1 1 newimagep_p_gain "P-Gain" 
floatscale $image.modeparam.i-gain 0 2 21 1 1 newimagep_i_gain "I-Gain" 
floatscale $image.modeparam.d-gain 0 5 51 1 1 newimagep_d_gain "D-Gain" 
floatscale $image.modeparam.amplitude 0 1 101 1 1 newimagep_amplitude \
	"Amplitude" 
floatscale $image.modeparam.rate 0.1 50.0 100 1 1 newimagep_rate "Rate (uM/sec)" 

pack    $image.modeparam.setpoint  $image.modeparam.p-gain \
	$image.modeparam.i-gain $image.modeparam.d-gain \
        $image.modeparam.rate \
	-side top -fill x -pady $fspady

if {$newimagep_mode==0} {
  pack $image.modeparam.amplitude  -side top -fill x -pady $fspady
}

set im_oscillating_list "$image.modeparam.amplitude"

#setup Image style box
label $image.style.label -text "Style" -bg $bc
pack $image.style.label -side top -anchor nw
radiobutton $image.style.sharp -text "Sharp" -variable newimagep_style -value 0 -bg $fc
#radiobutton $image.style.blunt -text "Blunt" -variable newimagep_style -value 1 -bg $fc

pack $image.style.sharp -side top -fill x 


#setup Image styleparam box
label $image.styleparam.label -text "Style parameters" -bg $bc
pack $image.styleparam.label -side top 

#floatscale $image.styleparam.tri-size 0.5 5 101 1 1 newimagep_tri_size \
#	"Tri Size" 
#floatscale $image.styleparam.tri-speed 1000 10000 101 1 1 newimagep_tri_speed \
#	"Tri Speed" 

#set im_blunt_list "$image.styleparam.tri-size $image.styleparam.tri-speed"


#setup Image tool box
label $image.tool.label -text "Tool" -bg $bc
pack $image.tool.label -side top 

radiobutton $image.tool.freehand -text "Freehand" -variable newimagep_tool -value 0 -bg $fc 

pack $image.tool.freehand  -side top -fill x

#setup Image toolparam box
label $image.toolparam.label -text "Tool parameters" -bg $bc
pack $image.toolparam.label -side top -anchor nw
#label $image.toolparam.step-sizel -text "Step-Size" -bg $bc
#entry $image.toolparam.step-size -width 10 -relief sunken
#pack  $image.toolparam.step-sizel $image.toolparam.step-size \
#	-side top -fill x -anchor nw





#
#
######################
# Flip_* procedures
#   These change the displayed widgets, depending on the
# value of a global variable, i.e. which radiobutton is pressed.
# They are called by "trace variable"

# flip $image.modeparam widgets
proc flip_im_mode {im_mode element op} {
    global image
    global im_oscillating_list
    global fspady

    upvar $im_mode k

    if {$k==0} {
        # selected oscillating
	# 6 is magic number - number of floatscales to leave alone + 1
	set plist [lrange [pack slaves $image.modeparam] 6 end] 
	foreach widg $plist {pack forget $widg} 
	foreach widg $im_oscillating_list {pack $widg -side top -fill x -pady $fspady}
    } elseif {$k==1} {
	# selected contact
	# magic number 6 again
	set plist [lrange [pack slaves $image.modeparam] 6 end] 
	foreach widg $plist {pack forget $widg}
    }
}


# flips $image.styleparam widgets
proc flip_im_style {im_style element op} {
    global image
#    global im_blunt_list
    global fspady

    upvar $im_style k

    if {$k==0} {
        # selected sharp
	set plist [lrange [pack slaves $image.styleparam] 1 end] 
	foreach widg $plist {pack forget $widg} 
#    } elseif {$k==1} {
	# selected blunt
#	set plist [lrange [pack slaves $image.styleparam] 1 end] 
#	foreach widg $plist {pack forget $widg}
#	foreach widg $im_blunt_list {pack $widg -side top -fill x -pady $fspady}
    }
}

#
# Change the background of Accept and Cancel buttons
#    when you haven't yet committed your changes by clicking Accept
#

# variable tracks how many of newimagep_* have been changed
set num_im_new_changed 0

proc imBackgChReal {fooa element op} {
    global image

	$image.mode.accept configure -background LightPink1
	$image.mode.cancel configure -background LightPink1
}

proc acceptImageVars {varlist} {
    global accepted_image_params
    global image
    global fc

    foreach val $varlist {
	global imagep_$val
	global newimagep_$val
	set k [set newimagep_$val]
#puts "newimagep_$val = $k"
	if {$k !=  [set imagep_$val]} {
	    set imagep_$val $k
	}
    }
    set accepted_image_params 1
    # None of the newimage_* vars are now changed
	$image.mode.accept configure -background $fc
	$image.mode.cancel configure -background $fc

}


proc cancelImageVars {varlist} {
    global image
    global fc

    foreach val $varlist {
	global imagep_$val
	global newimagep_$val
	if {[set newimagep_$val] !=  [set imagep_$val]} {
	    set newimagep_$val [set imagep_$val]
	}
    }
    # None of the newimage_* vars are now changed
	$image.mode.accept configure -background $fc
	$image.mode.cancel configure -background $fc
}

proc updateFromC {name element op} {
    global image
    global modify
    global scanline
    global fc
    
    upvar $name oldimvar
    global new$name
    set new$name $oldimvar
#    puts "Update from C: new$name $oldimvar"

    $image.mode.accept configure -background $fc
    $image.mode.cancel configure -background $fc
    $modify.mode.accept configure -background $fc
    $modify.mode.cancel configure -background $fc
    $scanline.mode.accept configure -background $fc
    $scanline.mode.cancel configure -background $fc

}

# This file sets up the $image frame, and must be sourced from a
# specific location inside tools.tcl
# 
# for widgets that change behavior of image mode
#
 
set image [create_closing_toplevel image "Image Parameters"]

frame $image.mode 
frame $image.modeparam 
frame $image.style 
frame $image.styleparam 
frame $image.tool 
frame $image.toolparam 

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

# list of all the variables above, imagep_*
set imageplist "{mode style tool setpoint p_gain i_gain d_gain amplitude rate}"

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

# flips between sets of parameters
trace variable newimagep_mode w flip_im_mode

proc updateFromC {realname name element op} {
    global image
    global modify
    global scanline
    global fc
    
    upvar #0 $realname oldimvar
    global new$realname
    set new$realname $oldimvar
#    puts "Update from C: new$realname $oldimvar"

    $image.mode.accept configure -background $fc
    $image.mode.cancel configure -background $fc
    $modify.mode.accept configure -background $fc
    $modify.mode.cancel configure -background $fc
#    $scanline.mode.accept configure -background $fc
#    $scanline.mode.cancel configure -background $fc

}

# checks to see if C code changes values of our variables.
trace variable imagep_mode w "updateFromC imagep_mode "
trace variable imagep_style w "updateFromC imagep_style "
trace variable imagep_tool w "updateFromC imagep_tool "

trace variable imagep_setpoint  w "updateFromC imagep_setpoint "
trace variable imagep_p_gain  w "updateFromC imagep_p_gain "
trace variable imagep_i_gain  w "updateFromC imagep_i_gain "
trace variable imagep_d_gain  w "updateFromC imagep_d_gain "
trace variable imagep_amplitude  w "updateFromC imagep_amplitude "
trace variable imagep_rate    w "updateFromC imagep_rate "

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

#
#setup Image box
#
pack $image.mode $image.modeparam $image.style $image.styleparam $image.tool \
	$image.toolparam -side left -padx 4 -fill both

#setup Image mode box
label $image.mode.label -text "Image Mode" 
pack $image.mode.label -side top -anchor nw
radiobutton $image.mode.oscillating -text "Oscillating" -variable newimagep_mode -value 0 
radiobutton $image.mode.contact -text "Contact" -variable newimagep_mode -value 1 
button $image.mode.accept -text "Accept" -command "acceptImageVars $imageplist"
button $image.mode.cancel -text "Revert" -command "cancelImageVars $imageplist"
pack $image.mode.oscillating $image.mode.contact -side top -anchor nw -fill x
pack $image.mode.cancel $image.mode.accept -side bottom -fill x

#setup Image modeparam box
label $image.modeparam.label -text "Mode parameters" 
pack $image.modeparam.label -side top -anchor nw 

generic_entry $image.modeparam.setpoint newimagep_setpoint \
	"Set Point (0,100)" real 
generic_entry $image.modeparam.p-gain newimagep_p_gain "P-Gain (0,5)" real 
generic_entry $image.modeparam.i-gain newimagep_i_gain "I-Gain (0,5)" real 
generic_entry $image.modeparam.d-gain newimagep_d_gain "D-Gain (0,5)" real 
generic_entry $image.modeparam.amplitude newimagep_amplitude \
	"Amplitude (0,2)" real 
generic_entry $image.modeparam.rate newimagep_rate "Rate (1,50 uM/sec)" real 

pack    $image.modeparam.setpoint  $image.modeparam.p-gain \
	$image.modeparam.i-gain $image.modeparam.d-gain \
        $image.modeparam.rate \
	-side top -fill x -pady $fspady

iwidgets::Labeledwidget::alignlabels \
    $image.modeparam.setpoint  $image.modeparam.p-gain \
	$image.modeparam.i-gain $image.modeparam.d-gain \
	$image.modeparam.amplitude $image.modeparam.rate 
	
if {$newimagep_mode==0} {
  pack $image.modeparam.amplitude  -side top -fill x -pady $fspady
}

set im_oscillating_list "$image.modeparam.amplitude"

#setup Image style box
label $image.style.label -text "Style" 
pack $image.style.label -side top -anchor nw
radiobutton $image.style.sharp -text "Sharp" -variable newimagep_style -value 0
pack $image.style.sharp -side top -fill x 


#setup Image styleparam box
label $image.styleparam.label -text "Style parameters" 
pack $image.styleparam.label -side top 


#setup Image tool box
label $image.tool.label -text "Tool" 
pack $image.tool.label -side top 

radiobutton $image.tool.freehand -text "Freehand" -variable newimagep_tool -value 0  

pack $image.tool.freehand  -side top -fill x

#setup Image toolparam box
label $image.toolparam.label -text "Tool parameters" 
pack $image.toolparam.label -side top -anchor nw


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
	# 6 is magic number - number of widgets to leave alone + 1
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


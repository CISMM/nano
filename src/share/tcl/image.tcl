#/*===3rdtech===
#  Copyright (c) 2000 by 3rdTech, Inc.
#  All Rights Reserved.
#
#  This file may not be distributed without the permission of 
#  3rdTech, Inc. 
#  ===3rdtech===*/
# 
# for widgets that change behavior of image mode
#
 
set nmInfo(image) [create_closing_toplevel image "Image Parameters"]
set nmInfo(imagequick) [frame $nmInfo(image).quick]
set nmInfo(imagefull) [frame $nmInfo(image).full]

# Button swaps between quick and full param frames.  
set image_quick_or_full "quick"
button $nmInfo(image).quick_or_full -text "Full params" -command {
    global image_quick_or_full nmInfo
    if {$image_quick_or_full == "quick"} {
        pack forget $nmInfo(imagequick)
        pack $nmInfo(imagefull) -side top -expand yes -fill both
        set image_quick_or_full "full"
        $nmInfo(image).quick_or_full configure -text "Quick params"
    } else {
        pack forget $nmInfo(imagefull)
        pack $nmInfo(imagequick) -side top -expand yes -fill both
        set image_quick_or_full "quick"
        $nmInfo(image).quick_or_full configure -text "Full params"
    }
}
pack $nmInfo(image).quick_or_full -side top -anchor nw

pack $nmInfo(imagequick) -side top -expand yes -fill both

#------------------
# Quick image controls. Changes to these controls take effect 
# immediately, but can't change modes. 
iwidgets::Labeledframe $nmInfo(imagequick).imagestate -labeltext "Image state" \
	-labelpos nw
set nmInfo(imagepage) [frame $nmInfo(imagequick).imagepage]
pack $nmInfo(imagequick).imagestate $nmInfo(imagequick).imagepage -side top \
	-fill x


set nmInfo(imagestate) [$nmInfo(imagequick).imagestate childsite]

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

iwidgets::Labeledwidget::alignlabels \
	$nmInfo(imagepage).setpoint $nmInfo(imagepage).p-gain \
	$nmInfo(imagepage).i-gain $nmInfo(imagepage).d-gain \
	$nmInfo(imagepage).rate 



#------------------
# Full image controls, allow switch between any mode using 
# Accept and Revert buttons.

frame $nmInfo(imagefull).mode 
frame $nmInfo(imagefull).modeparam 
frame $nmInfo(imagefull).style 
frame $nmInfo(imagefull).styleparam 
frame $nmInfo(imagefull).tool 
frame $nmInfo(imagefull).toolparam 

# dummy entry widget to force focus to move away from real entry widgets.
entry $nmInfo(imagefull).dummy_entry

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
    global nmInfo
    global scanline
    global save_bg
    
    upvar #0 $realname oldimvar
    global new$realname
    set new$realname $oldimvar
#    puts "Update from C: new$realname $oldimvar"

    $nmInfo(imagefull).mode.accept configure -background $save_bg
    $nmInfo(imagefull).mode.cancel configure -background $save_bg
    $nmInfo(modifyfull).mode.accept configure -background $save_bg
    $nmInfo(modifyfull).mode.cancel configure -background $save_bg
#    $scanline.mode.accept configure -background $save_bg
#    $scanline.mode.cancel configure -background $save_bg

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
pack $nmInfo(imagefull).mode $nmInfo(imagefull).modeparam $nmInfo(imagefull).style $nmInfo(imagefull).styleparam $nmInfo(imagefull).tool \
	$nmInfo(imagefull).toolparam -side left -padx 4 -fill both

#setup Image mode box
label $nmInfo(imagefull).mode.label -text "Image Mode" 
pack $nmInfo(imagefull).mode.label -side top -anchor nw
radiobutton $nmInfo(imagefull).mode.oscillating -text "Oscillating" -variable newimagep_mode -value 0 
radiobutton $nmInfo(imagefull).mode.contact -text "Contact" -variable newimagep_mode -value 1 
button $nmInfo(imagefull).mode.accept -text "Accept" -command "acceptImageVars $imageplist"
button $nmInfo(imagefull).mode.cancel -text "Revert" -command "cancelImageVars $imageplist"
pack $nmInfo(imagefull).mode.oscillating $nmInfo(imagefull).mode.contact -side top -anchor nw -fill x
pack $nmInfo(imagefull).mode.cancel $nmInfo(imagefull).mode.accept -side bottom -fill x

# save the background color we use for "accept" and "revert" buttons
set save_bg [$nmInfo(imagefull).mode.accept cget -background]

#setup Image modeparam box
label $nmInfo(imagefull).modeparam.label -text "Mode parameters" 
pack $nmInfo(imagefull).modeparam.label -side top -anchor nw 

generic_entry $nmInfo(imagefull).modeparam.setpoint newimagep_setpoint \
	"Set Point (0,100)" real 
generic_entry $nmInfo(imagefull).modeparam.p-gain newimagep_p_gain "P-Gain (0,5)" real 
generic_entry $nmInfo(imagefull).modeparam.i-gain newimagep_i_gain "I-Gain (0,5)" real 
generic_entry $nmInfo(imagefull).modeparam.d-gain newimagep_d_gain "D-Gain (0,5)" real 
generic_entry $nmInfo(imagefull).modeparam.amplitude newimagep_amplitude \
	"Amplitude (0,2)" real 
generic_entry $nmInfo(imagefull).modeparam.rate newimagep_rate "Rate (1,50 uM/sec)" real 

pack    $nmInfo(imagefull).modeparam.setpoint  $nmInfo(imagefull).modeparam.p-gain \
	$nmInfo(imagefull).modeparam.i-gain $nmInfo(imagefull).modeparam.d-gain \
        $nmInfo(imagefull).modeparam.rate \
	-side top -fill x -pady $fspady

iwidgets::Labeledwidget::alignlabels \
    $nmInfo(imagefull).modeparam.setpoint  $nmInfo(imagefull).modeparam.p-gain \
	$nmInfo(imagefull).modeparam.i-gain $nmInfo(imagefull).modeparam.d-gain \
	$nmInfo(imagefull).modeparam.amplitude $nmInfo(imagefull).modeparam.rate 
	
if {$newimagep_mode==0} {
  pack $nmInfo(imagefull).modeparam.amplitude  -side top -fill x -pady $fspady
}

set im_oscillating_list "$nmInfo(imagefull).modeparam.amplitude"

#setup Image style box
label $nmInfo(imagefull).style.label -text "Style" 
pack $nmInfo(imagefull).style.label -side top -anchor nw
radiobutton $nmInfo(imagefull).style.sharp -text "Sharp" -variable newimagep_style -value 0
pack $nmInfo(imagefull).style.sharp -side top -fill x 


#setup Image styleparam box
label $nmInfo(imagefull).styleparam.label -text "Style parameters" 
pack $nmInfo(imagefull).styleparam.label -side top 


#setup Image tool box
label $nmInfo(imagefull).tool.label -text "Tool" 
pack $nmInfo(imagefull).tool.label -side top 

radiobutton $nmInfo(imagefull).tool.freehand -text "Freehand" -variable newimagep_tool -value 0  

pack $nmInfo(imagefull).tool.freehand  -side top -fill x

#setup Image toolparam box
label $nmInfo(imagefull).toolparam.label -text "Tool parameters" 
pack $nmInfo(imagefull).toolparam.label -side top -anchor nw


#
#
######################
# Flip_* procedures
#   These change the displayed widgets, depending on the
# value of a global variable, i.e. which radiobutton is pressed.
# They are called by "trace variable"

# flip $nmInfo(imagefull).modeparam widgets
proc flip_im_mode {im_mode element op} {
    global nmInfo
    global im_oscillating_list
    global fspady

    upvar $im_mode k

    if {$k==0} {
        # selected oscillating
	# 6 is magic number - number of widgets to leave alone + 1
	set plist [lrange [pack slaves $nmInfo(imagefull).modeparam] 6 end] 
	foreach widg $plist {pack forget $widg} 
	foreach widg $im_oscillating_list {pack $widg -side top -fill x -pady $fspady}
    } elseif {$k==1} {
	# selected contact
	# magic number 6 again
	set plist [lrange [pack slaves $nmInfo(imagefull).modeparam] 6 end] 
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
    global nmInfo

	$nmInfo(imagefull).mode.accept configure -background LightPink1
	$nmInfo(imagefull).mode.cancel configure -background LightPink1
}

proc acceptImageVars {varlist} {
    global accepted_image_params
    global nmInfo
    global save_bg

    # Entry widgets commit their value when they loose focus. 
    # Change the focus to force them to commit their values. 
    # We're going to close the window later anyway...
    focus $nmInfo(imagefull).dummy_entry
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
	$nmInfo(imagefull).mode.accept configure -background $save_bg
	$nmInfo(imagefull).mode.cancel configure -background $save_bg

    #close the window when the Accept Button is pressed
    #wm withdraw $image
}


proc cancelImageVars {varlist} {
    global nmInfo
    global save_bg

    foreach val $varlist {
	global imagep_$val
	global newimagep_$val
	if {[set newimagep_$val] !=  [set imagep_$val]} {
	    set newimagep_$val [set imagep_$val]
	}
    }
    # None of the newimage_* vars are now changed
	$nmInfo(imagefull).mode.accept configure -background $save_bg
	$nmInfo(imagefull).mode.cancel configure -background $save_bg
}


#
# Helper procedure that will bring up a dialogue box to edit a value.
# It assumes that the variable whose name is passed to it is the one it
# it to assign a new float value to.  It allows cancellation (returning the
# old value).
#
# Author: Russ Taylor
# Last changed by : Aron Helser
#

proc newvalue_dialogue {varname {tlabel ""} } {
    set w .nvd$varname

    # Check to see if the window is out there already. If so, raise it
    # and give it focus.
    if {[winfo exists $w] } {
	raise $w
	# Focus input on Entry in the EntryField in the dialog box 
	focus [[$w component prompt] component entry]
	return
    }

    # We only sometimes have a useful text label, to tell us what
    # we're setting.
    if {$tlabel != ""} {
	iwidgets::promptdialog $w -title "Set value" -labeltext "Set $tlabel"
    } else {
	iwidgets::promptdialog $w -title "Set value" -labeltext "Set value"
    }
    # Extra buttons we don't want to see. 
    $w hide Apply
    $w hide Help

    # OK button has command to set the value of the global variable we were
    # passed as an argument. The command must be in "" because
    # we want to evaluate $w now, but we precede the [] and "" by \ 
    # so they _dont_ get executed until the button is pressed.  
    # "after idle" is added to be consistent with cancel button below.
    $w buttonconfigure OK -text "Set" -command "
	if {\[$w get \] != \"\"} {
	    set $varname \[$w get \]
	}
	$w deactivate 1
	after idle destroy $w 
    "
    
    # Nothing to do if user hits "cancel" - the variable isn't set. 
    # For some reason, the "deactivate" command was happening after
    # the "destroy" command when I used tab-tab-return to invoke the
    # cancel button (Error!). I added "after idle", and the problem went away. 
    $w buttonconfigure Cancel -command "
	$w deactivate 0
	after idle destroy $w
    "

    # Return and enter work properly with the promptdialog, 
    # but we want "Esc" to be the same as cancel:
    bind $w <Escape> "$w invoke Cancel"

	# Focus input on Entry in the EntryField in the dialog box 
	# and wait for the response
	focus [[$w component prompt] component entry]
	
	# Center the dialog on the main window.
	# Maybe not. Only seems to work if its after activate, 
	# and then it causes an unpleasant hop. 
	$w center .
	# Activate! When user hits a button, commands above will be
	# executed. 
	$w activate
	
}

#
# Helper procedure for floatscale that modifies the global variable when
# the scale changes.  It assumes that it is passed the whole record which
# contains various fields of information that it needs.
#
# This routine does nothing (except reset the skip indicator) if the
# skip_this_change entry has been set.  This indicates that the slider was
# set in response to a change in the variable, rather than vice-versa.
#
# Author: Russ Taylor
# Last changed by : Russ Taylor
#

proc floatscale.scalechanged {name element op} {
#    puts "R-$name $element"
	set temp ${name}(realvar); upvar $temp varname; upvar $varname realvar
	set temp ${name}(value); upvar $temp value
#    puts "R-$varname $realvar $value"
	set temp ${name}(min); upvar $temp min
	set temp ${name}(max); upvar $temp max
	set temp ${name}(skip_this_change); upvar $temp skip

	if {$skip} {
		# Nothing to do here
	} else {
	    set realvar $value
	}
	# Don't skip it the next time
	set skip 0
}

#
# This procedure creates a scale that locks itself onto a floating-point
# variable.  When the scale is moved, the variable is updated.  When the
# variable is set, the scale position is not changed, but the label on the
# slider is updated to show the new value.
# If the max or min variable changes value (the routine traces this), the
# slider adjusts its endpoints to match.
# This widget only works to control global variables, maybe.
# This widget works with simple variables like "ruler_scale" and 
#   array elements, like "joy1(x)"
#
# Author: Russ Taylor
# Last changed by : Aron Helser
#

proc floatscale {name min max steps labelfirst showrange variable 
		 {tlabel ""} } {
	global $name.record

	# Create the parent window to hold the scale and label
	frame $name 
        frame $name.l 
        frame $name.s 
	# Create the scale and label subwindows.
	scale $name.s.scale -from $min -to $max \
		-orient horizontal -showvalue 0 \
	        -resolution -1 \
		-command "set $name.record(value)" 
	uplevel label $name.l.label -textvariable $variable 
	if {$showrange} {
		label $name.s.minlabel -text $min 
		label $name.s.maxlabel -text $max 
	}

	# Set the initial value for the slider if the variable exists and
	# has a value.
	global $variable
    if { [info exists $variable] } {
	# use this funky expression, because sometimes $variable == "", 
	# and [expr [set $variable] + 0 ] converts that to 0. 
	# it's the same as " + 0" if the variable is ""
	$name.s.scale set [expr [set $variable] + 0 ]
    } else {
	$name.s.scale set $min
    }

	# Pack them into the parent window.
	if {$tlabel != ""} {
	    label $name.l.tlabel -text $tlabel 
	    pack $name.l.tlabel -side left
	}

	pack $name.l.label -side top -fill x

	if {$labelfirst} {
		pack $name.l -side top  -fill x 
	} else {
		pack $name.l -side bottom -fill x
	}

	pack $name.s -side left

	if {$showrange} {
		pack $name.s.minlabel -side left
	}
	pack $name.s.scale -side left
	if {$showrange} {
		pack $name.s.maxlabel -side left
	}
	
    
	# Squirrel away information that we will need later
	set $name.record(min) $min
	set $name.record(max) $max
	set $name.record(realvar) $variable
	# Skip the change that happens when the slider is first packed.
	set $name.record(skip_this_change) 1

	# Set up a linkage between the "local" variable and the variable that
	# the slider is to control so that the slider controls it.
	# (Remove any prior trace first, in case this widget has been
	#  destroyed and then restarted with the same name)
	trace vdelete $name.record(value) w floatscale.scalechanged
	trace variable $name.record(value) w floatscale.scalechanged

	# Set up a callback for button 1 pressed within the label that brings
	# up a dialogue box to allow the user to type in the new value
	bind $name.l.label <Button-1> "newvalue_dialogue $variable \"$tlabel\""

	# Set up a callback so that if the variable is changed by another
	# (like the dialog box) the slider setting moves to match.
	# Tell the local/remote callback that this it should not reset the
	# global based on this change in the slider.

	proc $name.varchanged {nm element op} "
	    global $name.record
	    global $variable
	    set $name.record(skip_this_change) 1
 	    $name.s.scale set \$$variable
	"

	# (Remove any prior trace first, in case this widget has been
	#  destroyed and then restarted with the same name)
	trace vdelete $variable w $name.varchanged
	trace variable $variable w $name.varchanged
    
}

#
# This procedure creates a scale that locks itself onto a integer
# variable.  When the scale is moved, the variable is updated.  When the
# variable is set, the scale position is not changed, but the label on the
# slider is updated to show the new value.
# If the max or min variable changes value (the routine traces this), the
# slider adjusts its endpoints to match.
# This widget only works to control global variables, maybe.
# This widget works with simple variables like "ruler_scale" and 
#   array elements, like "joy1(x)"
#
# Because integer and floating point variables are treated the same in Tcl
# this proceedure just calls the floating point routines and adds a command
# to change the scale widget so it only accepts integer values.
# new capability with tcl 7.4, I think. 
#
# Author: Aron Helser
# Last changed by : Aron Helser
#

proc intscale {name min max steps labelfirst showrange variable 
		 {tlabel ""} } {
    floatscale $name $min $max $steps $labelfirst $showrange $variable \
	$tlabel 
    $name.s.scale config -resolution 1
}


#
# This creates a pair of sliders with the same end values to control the min
# and max settings (stored in two other variables).
# This widget only works to control global variables, maybe.
#
# Author: Russ Taylor
# Last changed by : Renee Maheshwari 
#  3/7/97:
#    I added min2, max2, and steps2 so that ganged variables could have
#    different endpoints
#  2/28/97:
#    I added the two optional parameters, tlabel1 and tlabel2 so that
#    labels could be put on the minmax sliders, and they could be used
#    to gang up any two variables

proc minmaxscale {name min max steps var1 var2 
		  {tlabel1 ""} {tlabel2 ""} {min2 ""} {max2 ""} {steps2 ""} } {
	# Create the parent frame to hold both of the floatscales
	frame $name

	if {$min2 != "" && $max2 != "" && $steps2 !=""} {
	    # Create the two floatscale widgets to live under here
	    floatscale $name.min $min2 $max2 $steps2 0 1 $var1 $tlabel1
	    floatscale $name.max $min $max $steps 1 1 $var2 $tlabel2
	    pack $name.max $name.min -side top
	} else {
	    floatscale $name.min $min $max $steps 0 1 $var1 $tlabel1
	    floatscale $name.max $min $max $steps 1 1 $var2 $tlabel2
	    pack $name.max $name.min -side top
	}

	# Make sure that max stays more than min when min is moved
	proc $name.guardmax {nm element op} "
		global $var1 $var2
		if {\$$var1 > \$$var2} { set $var2 \$$var1 }
	"
	global $var1
	# (Remove any prior trace first, in case this widget has been
	#  destroyed and then restarted with the same name)
	trace vdelete $var1 w $name.guardmax
	trace variable $var1 w $name.guardmax

	# Make sure that min stays less than max when max is moved
	proc $name.guardmin {nm element op} "
		global $var1 $var2
		if {\$$var1 > \$$var2} { set $var1 \$$var2 }
	"
	global $var2
	# (Remove any prior trace first, in case this widget has been
	#  destroyed and then restarted with the same name)
	trace vdelete $var2 w $name.guardmin
	trace variable $var2 w $name.guardmin
}


#Procedures that support the Tclvar_int_with_entry class
# When the entry value has not been finalized by hitting "Enter", 
# the entry background will be red. 

# called when the C code sets the tcl variable
proc updateEntry {entry var name element op} { 
    upvar #0 $entry.textbg mybg
    upvar #0 $var varval 
    #if { $varval != [$entry get] } {
      $entry delete 0 end 
      $entry insert 0 $varval 
    #}
    $entry configure -textbackground $mybg
}

# Called when the user hits "enter" in the entry widget
proc setEntry {entry var } {
    upvar #0 $entry.textbg mybg
    upvar #0 $var varval 
    #if { $varval != [$entry get] } {
      set varval [$entry get]
    #}
    $entry configure -textbackground $mybg
}

#Called when the user types a new character in the entry widget
# Warns them that their changes have not been finalized.
proc setWarnBackground { entry name element op } {
    $entry configure -textbackground LightPink1
}   

# Create an integer entry field.
proc generic_entry { name var label validation} {
    upvar #0 $var varval
    iwidgets::entryfield "$name" \
        -command "setEntry $name $var" \
        -validate $validation \
        -textvariable "$name.gvar" \
        -labeltext "$label"
    #set the initial contents.
    $name insert 0 $varval

    global $name.textbg $name.gvar

    #Store the default background so we can use it later
    set $name.textbg [$name cget -textbackground]

    # Find out when the user types in the entry field
    trace variable $name.gvar w "setWarnBackground $name"

    # Find out when the global variable connected with the entry
    # gets set by someone else.
    trace variable varval w "updateEntry $name $var "

    return $name
}

# Create an integer entry field.
proc intentry { name var } {
    generic_entry $name $var "" integer

    pack $name
}










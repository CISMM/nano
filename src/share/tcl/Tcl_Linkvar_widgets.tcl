#/*===3rdtech===
#  Copyright (c) 2000 by 3rdTech, Inc.
#  All Rights Reserved.
#
#  This file may not be distributed without the permission of 
#  3rdTech, Inc. 
#  ===3rdtech===*/
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
#    puts "updateEntry $var $varval"

    # the entry text won't change unless it is in "normal" state. 
    set was_disabled 0
    if { [$entry cget -state] == "disabled" } {
        set was_disabled 1
        $entry configure -state normal
    }
    $entry delete 0 end 
    $entry insert 0 $varval 
    $entry configure -textbackground $mybg
    if { $was_disabled } {
        $entry configure -state disabled
    }
}

# Called when the user hits "enter" in the entry widget
# Also invokes a callback command, if there is one.
proc setEntry {entry var {my_command ""} } {
    upvar #0 $entry.textbg mybg
    upvar #0 $var varval 
#    puts "setEntry $var $varval"
      set varval [$entry get]
    $entry configure -textbackground $mybg
    if {$my_command != "" } {
        uplevel #0 $my_command
    }
}
# Called when the entry widget looses focus. 
# Very similar to the setEntry procedure, but we only want 
# to set the global variable and yank the callback if the
# value has changed. Why? Because this procedure gets called
# every time focus leaves the entry, and we don't want to generate
# that many callbacks.
proc focus_setEntry {entry var {my_command ""} } {
    upvar #0 $entry.textbg mybg
    upvar #0 $var varval 
#    puts "focus_setEntry $var $varval"
    $entry configure -textbackground $mybg
    set curr_entry_val [$entry get]
    if { $varval != $curr_entry_val } {
        set varval $curr_entry_val
        if {$my_command != "" } {
            uplevel #0 $my_command
        }
    }
}

# Called when the user hits "Esc" in the entry widget
proc revertEntry {entry var } {
    upvar #0 $entry.textbg mybg
    upvar #0 $var varval 
#    puts "revertEntry $var $varval"
    $entry delete 0 end 
    $entry insert 0 $varval 
    $entry configure -textbackground $mybg
}

#Called when the user types a new character in the entry widget
# Warns them that their changes have not been finalized.
proc setWarnBackground { entry name element op } {
    $entry configure -textbackground LightPink1
}   

# Create an entry field. Validation determines what kind of
# numbers/letters can be entered.
# If the global variable passed in as "var" exists
# the entry shows it's initial value.
# A callback function can be passed in, which gets executed anytime
#   the global variable is set. 
# Note: this is inspired by "Effective Tcl/Tk programming", Harrison, 
# and we could add percent substitution so we could automatically
# pass additional information into the callback. See sect 8.2.3 pg 338.
proc generic_entry { name var label validation {my_command "" }} {
    upvar #0 $var varval
    iwidgets::entryfield "$name" \
	-command [list setEntry $name $var $my_command] \
	-validate $validation \
	-textvariable "$name.gvar" \
	-labeltext "$label" \
	-width 8
    if {[info exists varval]} {
	#set the initial contents.
	$name insert 0 $varval
    }

    global $name.textbg $name.gvar

    #Store the default background so we can use it later
    set $name.textbg [$name cget -textbackground]

    # Find out when the user types in the entry field
    trace variable $name.gvar w "setWarnBackground $name"

    # Find out when the global variable connected with the entry
    # gets set by someone else.
    trace variable varval w "updateEntry $name $var "

    # Allow the user to hit "Esc" to go back to the old value
    bind [$name component entry] <Escape> [list revertEntry $name $var]

    # Change the global variable when the entry looses focus, not
    # just when the user hits Enter
    bind [$name component entry] <FocusOut> [list focus_setEntry $name $var $my_command]
    
    return $name
}

# Changes the label of a generic_entry.
proc generic_entry_change_label { name label } {
    $name configure -labeltext "$label" 
}

# Create an integer entry field.
proc intentry { name var } {
    generic_entry $name $var "" integer

    pack $name
}


# Help prevent nested callbacks.
set optionmenu_setting_list 0

# help prevent setting global when it doesn't need to be
set optionmenu_selecting_default 0

# called when the C code sets the global value variable
proc updateOptionmenu {menu entry_list_name var name element op} { 
    global optionmenu_setting_list
    upvar #0 $entry_list_name entry_list
    upvar #0 $var varval 

    # If this callback results from the entry_list being changed
    # ignore it.
    if { $optionmenu_setting_list } { return; }
    #puts "TCL: updateOptionmenu $var $varval"

    set old_menu_val "[$menu get]"
    # If the new variable value is not one of the menu choices,
    # use the old selection
    if { "$varval" != "$old_menu_val" } {
	# Check to see if menu item exists using a list search
        set menu_item [lsearch -exact $entry_list "$varval"]
	if {$menu_item == -1} {
	    # We force this menu to contain the value the 
	    # global variable was set to. Programmer knows best :)
	    #$menu insert end "$varval"
	    #$menu select end

            # BZZT - that's weird behavior - let's select item 0 instead.
            $menu select 0
	} else {
            # avoid using $menu select "$varval" because this
            # will fail if $varval is a numeric value. 
            # Numeric index has precedence, and should always work. 
	    $menu select $menu_item
	}
    }
}

# called when the C code changes the list of menu entries.
proc updateOptionmenuEntries {menu entry_list_name var name_or_index name element op} { 
    global optionmenu_setting_list optionmenu_selecting_default
    upvar #0 $entry_list_name entry_list
    upvar #0 $var varval 
    #puts "TCL: updateOptionmenuEntries $var $varval "

    set old_menu_val [$menu get]
    if { $entry_list == "" } {
	set entry_list "none"
    }
    # Tell the updateOptionmenu callback we are deleting the list.
    set optionmenu_setting_list 1
    # Tell the optionmenu command to ignore the "$menu select 0" event caused
    # by resetting the list
    set optionmenu_selecting_default 1

    $menu delete 0 end
    # Use "eval" so the list members are treated as individual args.
    eval $menu insert end $entry_list

    set optionmenu_selecting_default 0
    set optionmenu_setting_list 0

    # Try to select the same entry as before the menu entries changed
    # Check to see if the item is in the new menu entries.
    # Menu entries must be in quotes to handle values with spaces!
    set menu_item [lsearch -exact $entry_list "$old_menu_val"]
    if {$menu_item == -1} {
	#puts "    TCL: optionmenu search if $var"
	# The old menu item doesn't exist, so just choose the first item
	$menu select 0
	# this is necessary - the "select" command doesn't do it, 
	# because the first menu item is already selected.
        if {$name_or_index} {
            # generic_optionmenu behavior
            set varval [$menu get]
        } else {
            # generic_optionmenu_with_index behavior
            set varval [$menu index select]
        }
    } else {
	#puts "    TCL: optionmenu search else $var"
	# We don't ever want callbacks for the global variable
	# to be activated if we are just adjusting the menu entries
	# so guard the "select" command so it won't re-set the 
	# global variable. 
	set optionmenu_selecting_default 1
            # avoid using $menu select "$varval" because this
            # will fail if $varval is a numeric value. 
            # Numeric index has precedence, and should always work. 
	$menu select $menu_item
	set optionmenu_selecting_default 0
    }
}

# Create an option menu for choosing from a list of strings.
# Two global variable are associated with it.
# "var" is a global representing the currently selected item
# "entry_list_name" is a global list representing all the items to
# choose from in the menu.
proc generic_optionmenu { name var label entry_list_name } {
    upvar #0 $var varval
    upvar #0 $entry_list_name entry_list

    iwidgets::optionmenu $name \
	    -labeltext "$label" -labelpos w \
	    -command "if {!\$optionmenu_selecting_default} {
	set $var \[ $name get \];
    }" 

    if { $entry_list != "" } {
	# Use "eval" so the list members are treated as individual args.
	eval $name insert 0 $entry_list
    } else {
	$name insert 0 "none"
    }

    # If global variable already exists, make the menu show it's value
    if {[info exists varval]} {
        updateOptionmenu $name $entry_list_name $var 0 0 0
    } else {
        # Force the global variable to be set to the initial menu entry.
        set varval "[ $name get ]"
    }

    # Find out when the global variable connected with the entry
    # gets set by someone else.
    trace variable varval w "updateOptionmenu $name $entry_list_name $var "

    # Find out when the global variable connected with the menu entries
    # gets set by someone else.
    trace variable entry_list w \
	    "updateOptionmenuEntries $name $entry_list_name $var yes "

    return $name
}

# called when the C code sets the global value variable
proc updateOptionmenuWithIndex {menu var name element op} {
    upvar #0 $var varval
    global optionmenu_setting_list

    if { $optionmenu_setting_list } { return; }

    set optionmenu_setting_list 1

    # Sets the menu according to the numeric index in the global var. 
    $menu select $varval

    set optionmenu_setting_list 0
}


# Create an option menu for choosing from a list of strings.
# Two global variable are associated with it.
# "var" is a global representing the currently selected item
# "entry_list_name" is a global list representing all the items to
# choose from in the menu.
proc generic_optionmenu_with_index { name var label entry_list_name } {
    upvar #0 $var varval
    upvar #0 $entry_list_name entry_list

    iwidgets::optionmenu $name \
            -labeltext "$label" -labelpos w \
            -command "if {!\$optionmenu_selecting_default} {
        set $var \[ $name index select \];
    }"

    if { $entry_list != "" } {
        # Use "eval" so the list members are treated as individual args.
        eval $name insert end $entry_list
    } else {
        $name insert end "none"
    }

    # If global variable already exists, make the menu show it's value
    if {[info exists varval]} {
        updateOptionmenuWithIndex $name $var 0 0 0
    } else {
        # Force the global variable to be set to the initial menu entry.
        set varval "[ $name index select ]"
    }

    # Find out when the global variable connected with the entry
    # gets set by someone else.
    trace variable varval w "updateOptionmenuWithIndex $name $var "

    # Find out when the global variable connected with the menu entries
    # gets set by someone else.
    trace variable entry_list w \
            "updateOptionmenuEntries $name $entry_list_name $var no "

    return $name
}


# Create a window with a "close" button at the top, 
# and a procedure to open it again. 
proc create_closing_toplevel { win_name {title "" } } {

    toplevel .$win_name
    wm withdraw .$win_name
    
    if { "$title" != "" } {
	wm title .$win_name "$title"
    }

    button .$win_name.close -text "Close" -command "
	wm withdraw .$win_name
    "
    wm protocol .$win_name WM_DELETE_WINDOW ".$win_name.close invoke"
    #pack .$win_name.close -anchor nw
    
    proc show.${win_name} {} "
	wm deiconify .$win_name
	raise .$win_name
    "
    proc hide.${win_name} {} "
        .$win_name.close invoke
    "
    return .$win_name
}

# called when the C code sets the global value variable
proc showWindow {win_name var_name name1 name2 op} {
    upvar #0 $var_name make_visible
#    puts "showWindow $var_name $name1 $name2 $op"
    if {$make_visible == 1} {
       wm deiconify .$win_name
       raise .$win_name
    } else {
       wm withdraw .$win_name
    }
}

# Create a window with a "close" button at the top,
# and a procedure to open it again. The global variable passed in is
# set to 1 while the window is visible and 0 while it is hidden
proc create_closing_toplevel_with_notify { win_name signal_var_name {title "" } } {

    toplevel .$win_name
    wm withdraw .$win_name

    if { "$title" != "" } {
        wm title .$win_name "$title"
    }

    button .$win_name.close -text "Close" -command "
        wm withdraw .$win_name
        upvar #0 $signal_var_name signal_var
        set signal_var 0
    "
    wm protocol .$win_name WM_DELETE_WINDOW ".$win_name.close invoke"
    #pack .$win_name.close -anchor nw

    upvar #0 $signal_var_name signal_var
    trace variable signal_var w \
          "showWindow $win_name $signal_var_name"

    proc show.${win_name} {} "
        wm deiconify .$win_name
        raise .$win_name
        upvar #0 $signal_var_name signal_var
        set signal_var 1
    "

    proc hide.${win_name} {} "
        .$win_name.close invoke
    "

    return .$win_name
}


# Creates a standard dialog for choosing a color, and sets the
# value of a global variable based on the user's choice. If the
# user hits "cancel", the global variable's value is unchanged. 
# Returns 1 if user hit OK, and 0 if user hit Cancel
proc choose_color { color_var_name {title "Choose color"} {parent .} } {
    global nmInfo
    upvar #0 $color_var_name color_var
    set color [tk_chooseColor -title "$title" \
	    -initialcolor $color_var -parent $parent]
    if { $color != "" } {
	set color_var $color
        return 1
    }
    return 0
}

# called when the C code sets the tcl global variable
proc updateRadiobox {boxname var name element op} { 
    upvar #0 $var varval 

    # Make sure new value of variable is inside legal limits for 
    # the radiobox.
    if { ($varval >= 0) && ($varval <= [$boxname index end]) } {
        # The radiobox won't change unless its state is "normal"
        set was_disabled 0
        if { [lindex [$boxname buttonconfigure $varval -state] end] == "disabled" } {
            set was_disabled 1
            $boxname buttonconfigure $varval -state normal
        }
	$boxname select $varval
        if { $was_disabled } {
            $boxname buttonconfigure $varval -state disabled
        }
    } else {
	#puts " Illegal value ! $var $varval [$boxname get]"
	# Return to whatever value the radiobox used to have.
	set varval [$boxname get]
    }
}

# Command looks up the value of the selected radiobox item. It
# is a number starting at 0, because we set it up that way. 
proc radiobox_command {name var } {
    upvar #0 $var varval
    if {[$name get]!= ""} {
	set varval [$name get]
    } 
    #puts "Radiobox setting $var to [$name get]"
}

# This radiobox takes a list of items as input and a global variable
# as input. Each item in the list is assigned a number starting with 0.
# If the user selects an item in the list, the global variable gets
# set to it's numerical index. 
proc generic_radiobox { name var label entry_list} {
    upvar #0 $var varval
    
    # Set default value for radiobox, if it doesn't exist already
    if { 0==[info exists varval] } { set varval 0 }
    iwidgets::radiobox $name -labeltext "$label" -labelpos nw \
            -command [list radiobox_command $name $var]

    # add each of the elements in the list passed in. Use a numerical tag.
    set i 0
    foreach entry $entry_list {
	$name add $i -text "$entry"
	incr i
    }

    if { ($varval < $i) && ($varval >= 0) } {
	$name select $varval
    } else {
	$name select 0
    }

    # Find out when the global variable connected with the radiobox
    # gets set by someone else.
    trace variable varval w "updateRadiobox $name $var "
}

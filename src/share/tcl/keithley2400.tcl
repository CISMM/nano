#!/bin/sh
# ----------------------------------------------------------------------
#  PROGRAM: VI Curve interface for Keithley 2400 digital ohm/current/volt meter
# ----------------------------------------------------------------------
#\
exec wish "$0" ${1+"$@"}

# Import Itcl and Iwidgets, for the tabnotebook widget and others we
# will use. This import method is a BUG - we shouldn't have to import
# the itcl and itk namespaces, but at least we can use it...
package require Itcl 
catch {namespace import itcl::*}
package require Itk 
catch {namespace import itk::*}
package require Iwidgets
# the extended file selection dialog does not work properly if 
# the iwidget namespace is imported.
#catch {namespace import -force iwidgets::*}
# Unfortunately, iwidgets and BLT have a namespace conflict 
# with the command "clock" - requires the -force option to be successful.

# Include the BLT package, which provides vectors, stripchart and
# graph widgets
package require BLT
catch {namespace import -force blt::*}
catch {namespace import -force blt::tile::*}

#makes it graph lines instead of scattered points
option add *Graph.*symbol ""
#option add *Graph.*pixels 1.25m
option add *Graph.width 600
option add *Graph.height 300
option add *Graph.*Smooth linear
option add *Graph.plotBackground black




# When the entry value has not been finalized by hitting "Enter", 
# the entry background will be red. 

# called when the C code sets the tcl variable
proc vi_updateEntry {entry var name element op} { 
    upvar #0 $entry.textbg mybg
    upvar #0 $var varval 
    if { $varval != [$entry get] } {
    $entry delete 0 end 
    $entry insert 0 $varval 
    }
    $entry configure -textbackground $mybg
}

# Called when the user hits "enter" in the entry widget
proc vi_setEntry {entry var } {
    upvar #0 $entry.textbg mybg
    upvar #0 $var varval 
    if { $varval != [$entry get] } {
    set varval [$entry get]
    }
    $entry configure -textbackground $mybg
}

#Called when the user types a new character in the entry widget
# Warns them that their changes have not been finalized.
proc vi_setWarnBackground { entry name element op } {
    $entry configure -textbackground LightPink1
}   

# Create an integer entry field.
proc my_entry { name var label validation} {
    upvar #0 $var varval 
    iwidgets::entryfield "$name" \
	-command "vi_setEntry $name $var" \
	-validate $validation \
	-textvariable "$name.gvar" \
	-labeltext "$label"
    #set the initial contents.
    $name insert 0 $varval 

    global $name.textbg $name.gvar

    #Store the default background so we can use it later
    set $name.textbg [$name cget -textbackground]

    # Find out when the user types in the entry field
    trace variable $name.gvar w "vi_setWarnBackground $name"

    # Find out when the global variable connected with the entry
    # gets set by someone else.
    trace variable varval w "vi_updateEntry $name $var " 

    return $name
}

# GLOBAL variables
# ----------------------------------------------------------------------
# If set to 1, the C code will quit the application
set vi_quit 0

# Default padding around the entry widgets, in pixels. 
set def_pad 1



# widgets is a global array for containing widget paths and names.
# vi is a global array for containing control variables, 
#   especially those linked to C++ code. 

# vi_win should be set to the path where these controls will 
# show up. In a standalone application, do something like this:
# set vi_win [toplevel .vi_win] 
# But if you have another place to put it, do:
# set vi_win [frame .path.vi_win]
# pack $vi_win 
# ------------------------------------------------- end GLOBAL variables

# Menu bar
#frame $vi_win.mbar -borderwidth 2 -relief raised
#pack $vi_win.mbar -fill x
#menubutton $vi_win.mbar.main -text "Main" -menu $vi_win.mbar.main.m
#pack $vi_win.mbar.main -side left

#menu $vi_win.mbar.main.m
#$vi_win.mbar.main.m add command -label "Quit" -command exit
#button $vi_win.mbar.close -text "Close" -command { set vi_quit 1 }
#pack $vi_win.mbar.close -anchor nw

pack [frame $vi_win.left] [frame $vi_win.middle] [frame $vi_win.right] -side left -expand yes -fill both -padx 10
#-----------------------------
iwidgets::labeledframe $vi_win.left.source_frame -labeltext "Source configuration"
set widgets(source_frame) [$vi_win.left.source_frame childsite]


iwidgets::labeledframe $vi_win.left.measure_frame -labeltext "Measure configuration"
set widgets(measure_frame) [$vi_win.left.measure_frame childsite]

pack $vi_win.left.source_frame $vi_win.left.measure_frame -side top -fill both

# show the time we've been connected, or the time in the stream
set vi(stream_time) 0
label $vi_win.right.timelabel -text "Time:  "
label $vi_win.right.time -textvariable $vi(stream_time)
pack $vi_win.right.timelabel $vi_win.right.time -side right

# Button to connect, clear, and completely initialize the Keithley
set widgets(connect_and_init) [button $vi_win.left.connect_and_init -text "Connect and Init" \
	-command { set vi(connect_and_init) 1 } ]
pack $widgets(connect_and_init) -anchor w

# Controls whether the display on the front of the Keithley is on. 
# Measurements are supposedly faster if it is off. 
set widgets(display_enable) [checkbutton $vi_win.left.display_enable -text "Keithley Display" \
	-variable vi(display_enable) ]
pack $widgets(display_enable) -anchor w
#set vi(display_enable) 1

# Bother. Radiobox seems to execute it's command when it gets created.
# So we'll define the change_source procedure here, then re-define it later
# after the chart has been created.
proc change_source {} {
}

# Is the Keithley going to supply (source) voltage or current
if { 0==[info exists vi(source)] } { set vi(source) 0 }
set widgets(source) [iwidgets::radiobox $widgets(source_frame).source \
	 -labeltext "Source:" -labelpos nw -command \
	 { change_source } ]

$widgets(source) add 0 -text "Voltage"
$widgets(source) add 1 -text "Current"

$widgets(source) select $vi(source)

# We can't handle sourcing current yet, so disable it
##$widgets(source) buttonconfigure 1 -state disabled
pack $widgets(source) -padx $def_pad -pady $def_pad -fill both 


# What is the compliance limit?
set widgets(compliance) [iwidgets::radiobox $widgets(measure_frame).compliance \
     -labeltext "Compliance:" -labelpos nw -command \
     {if {[$widgets(compliance) get]!= ""} {set vi(compliance) [$widgets(compliance) get]} }]
pack $widgets(compliance) -padx $def_pad -pady $def_pad -fill both 

$widgets(compliance) add 0 -text "Voltage"
$widgets(compliance) add 1 -text "Current"

$widgets(compliance) select 1
##$widgets(compliance) buttonconfigure 0 -state disabled

set widgets(four_wire) [iwidgets::radiobox $widgets(measure_frame).four_wire \
     -labeltext "Two/Four Wire:" -labelpos nw -command \
     {if {[$widgets(four_wire) get]!= ""} {set vi(wire_type) [$widgets(four_wire) get]} }]
pack $widgets(four_wire) -padx $def_pad -pady $def_pad -fill both 

$widgets(four_wire) add 2 -text "Two Probe"
$widgets(four_wire) add 4 -text "Four Probe"

$widgets(four_wire) select 0

if { 0==[info exists vi(compliance_val)] } { set vi(compliance_val) 0 }
set widgets(compliance_val) [my_entry $widgets(measure_frame).compliance_val vi(compliance_val) \
				 "Compliance Value" real ]
pack $widgets(compliance_val) -padx $def_pad -pady $def_pad

if { 0==[info exists vi(num_power_line_cycles)] } { set vi(num_power_line_cycles) 0 }
set widgets(num_power_line_cycles) [my_entry $widgets(measure_frame).num_power_line_cycles vi(num_power_line_cycles) \
				 "Number Power line cycles" real ]
pack $widgets(num_power_line_cycles) -padx $def_pad -pady $def_pad

#What type of sweep will we do, and what are it's limits?
set widgets(sweep) [iwidgets::radiobox $widgets(source_frame).sweep \
	-labeltext "Sweep:" -labelpos nw -command \
	{ if {[$widgets(sweep) get]!= ""} {set vi(sweep) [$widgets(sweep) get]} }]
pack $widgets(sweep) -padx $def_pad -pady $def_pad -fill both 

$widgets(sweep) add 0 -text "Linear"
$widgets(sweep) add 1 -text "Log"

$widgets(sweep) select 0

# Set some default values for the current sweep, which are much
# smaller than those for the voltage sweep.
set vi(sweep_start_save) -1e-6
set vi(sweep_stop_save)   1e-6

if { 0==[info exists vi(sweep_start)] } { set vi(sweep_start) 0 }
set widgets(sweep_start) [my_entry $widgets(source_frame).sweep_start vi(sweep_start) \
			      "Sweep start"  real ]
pack $widgets(sweep_start) -padx $def_pad -pady $def_pad

if { 0==[info exists vi(sweep_stop)] } { set vi(sweep_stop) 0 }
set widgets(sweep_stop) [my_entry $widgets(source_frame).sweep_stop vi(sweep_stop) \
			     "Sweep stop" real ]
pack $widgets(sweep_stop) -padx $def_pad -pady $def_pad

if { 0==[info exists vi(sweep_stepsize)] } { set vi(sweep_stepsize) 0 }
#set widgets(sweep_stepsize) [my_entry $widgets(source_frame).sweep_stepsize vi(sweep_stepsize) \
				 "Step Size" real ]
set widgets(sweep_stepsize) [iwidgets::labeledwidget $widgets(source_frame).ss_label -labeltext "Step Size"]
pack $widgets(sweep_stepsize) -padx $def_pad -pady $def_pad -anchor w
set cs [$widgets(sweep_stepsize) childsite]
label $cs.ss_label_var -textvariable vi(sweep_stepsize)
pack $cs.ss_label_var

if { 0==[info exists vi(sweep_numpoints)] } { set vi(sweep_numpoints) 0 }
set widgets(sweep_numpoints) [my_entry $widgets(source_frame).sweep_numpoints vi(sweep_numpoints) \
				  "Number of Points" real ]
pack $widgets(sweep_numpoints) -padx $def_pad -pady $def_pad

if { 0==[info exists vi(sweep_delay)] } { set vi(sweep_delay) 0 }
set widgets(sweep_delay) [my_entry $widgets(source_frame).sweep_delay vi(sweep_delay) \
			      "Delay" real ]
pack $widgets(sweep_delay) -padx $def_pad -pady $def_pad

if { 0==[info exists vi(num_sweeps)] } { set vi(num_sweeps) 1 }
set widgets(num_sweeps) [my_entry $widgets(source_frame).num_sweeps vi(num_sweeps) \
			     "Number of Sweeps"  integer ]
pack $widgets(num_sweeps) -padx $def_pad -pady $def_pad


# align all the labels so it looks nice
iwidgets::Labeledwidget::alignlabels $widgets(sweep_start) $widgets(sweep_stop) \
    $widgets(sweep_stepsize) $widgets(sweep_numpoints) $widgets(sweep_delay) \
    $widgets(num_sweeps) $widgets(compliance_val) $widgets(num_power_line_cycles)
	
# Command the Keithley to take and IV curve right now!
set widgets(take_iv_curves) [button $vi_win.left.take_iv_curves -text "Take VI Curve" \
	-command { set vi(take_iv_curves) 1 } ]
pack $widgets(take_iv_curves) -anchor w

# Command the Keithley to take IV curves until turned off.
set widgets(take_repeat_iv_curves) [checkbutton $vi_win.left.take_repeat_iv_curves -text "Take Repeat VI Curves" \
	-variable vi(take_repeat_iv_curves) ]
pack $widgets(take_repeat_iv_curves) -anchor w

# Command the Keithley to take IV curves during any modification
set widgets(curves_during_mod) [checkbutton $vi_win.left.curves_during_mod \
	  -text "Take VI Curves during line modification" \
	-variable vi(curves_during_mod) ]
pack $widgets(curves_during_mod) -anchor w

#-----------------------------------------------------------------
# Graph the data returned by the Keithley 2400
#-----------------------------------------------------------------
set widgets(top) [frame $vi_win.right.top]
set widgets(bottom) [frame $vi_win.right.bottom]
pack $widgets(top) $widgets(bottom)  -side top -expand yes -fill both

# Make  a chart
set vi(chart) [graph $widgets(top).chart -title "V vs. I Chart"]
pack $vi(chart)  -expand yes -fill both

# Hide the legend - it takes up space and doesn't tell us anything.
#$vi(chart) legend configure -hide yes

# We are always going to chart voltage vs. current, so use the two default axis
$vi(chart) xaxis configure -title "Voltage"
$vi(chart) yaxis configure -title "Current"

#Make some controls so the min and max of each axis can be set. 
set widgets(volt_max) [iwidgets::entryfield $widgets(bottom).vmax -labeltext "Volt Max:" \
			   -validate real -width 12 \
			   -command {$vi(chart) xaxis configure -max [$widgets(volt_max) get]}]
set widgets(volt_min) [iwidgets::entryfield $widgets(bottom).vmin -labeltext "Volt Min:" \
			   -validate real -width 12 \
			   -command {$vi(chart) xaxis configure -min [$widgets(volt_min) get]}]
set widgets(curr_max) [iwidgets::entryfield $widgets(bottom).imax -labeltext "Curr Max:" \
			   -validate real -width 12 \
			   -command {$vi(chart) yaxis configure -max [$widgets(curr_max) get]}]
set widgets(curr_min) [iwidgets::entryfield $widgets(bottom).imin -labeltext "Curr Min:" \
			   -validate real -width 12 \
			   -command {$vi(chart) yaxis configure -min [$widgets(curr_min) get]}]
 pack $widgets(volt_max) $widgets(volt_min) $widgets(curr_max) $widgets(curr_min) -side top -anchor nw

# align all the labels so it looks nice
iwidgets::Labeledwidget::alignlabels \
    $widgets(volt_max) $widgets(volt_min) $widgets(curr_max) $widgets(curr_min)

# Clears any curves that have been graphed so far. 
set widgets(clear_curves) [button $widgets(bottom).clear_curves -text "Clear Curves" \
			   -command { clear_curves_now }]

pack $widgets(clear_curves) -side top -anchor nw

# keep track of which vectors we have saved so far
set vi(first_curve_not_saved) 0

# Save the data we have collected.
# We expect that the data will be saved inside the vectors
# created in C++ code, vi_curr_vec*.
# So, we step through these vectors and build up a text variable 
# for each line of the data, then save to a file. 
proc vi_save_data { filename } {
    global vi
    # Find out the names of all the data vectors
    set vec_end $vi(first_curve_not_saved)
    set vec_start $vi(first_curve_not_saved)
    global vi_volt_vec$vec_end
    global vi_curr_vec$vec_end
    while { [info exists vi_curr_vec$vec_end] } {
	if { ![info exists vi_volt_vec$vec_end] } {
	    tk_messageBox -icon error -message "Current vector $vec_end has no corresponding voltage vector" -type ok
	    # so we can save the next vectors, if any.
	    set vi(first_curve_not_saved) [expr $vec_end +1]
	    return 
	}
	incr vec_end
	global vi_curr_vec$vec_end vi_volt_vec$vec_end
    }
    if {$vec_start == $vec_end} { 
	tk_messageBox -icon error -message  "No new data to save" -type ok
	return }

    set volt_len [vi_volt_vec$vec_start length]
    set old_volt_len $volt_len

    # Include a heading for each column
    set line(-1) "V$vec_start\tI$vec_start"

    set volt_vec      vi_volt_vec$vec_start
    set curr_vec      vi_curr_vec$vec_start
    # Make the first column contain the voltage readings,
    # and the second column contain the first current readings.
    for { set i 0} { $i < $volt_len } { incr i } {
	set line($i) "[set ${volt_vec}($i)]\t[set ${curr_vec}($i)]"
    }

    # Loop through the data vectors. We expect the voltage vector to
    # stay the same for several current vectors. While it's the same,
    # just append the values in the current vectors to the line($i)
    # variables. When it changes, print a warning and stop saving.
    # i is an index into a vector
    # j is the index of the voltage and current vector pair we are dealing with
    set j [expr $vec_start +1]
    while {$j < $vec_end} {
	set volt_vec      vi_volt_vec$j
	set old_volt_vec  vi_volt_vec[expr $j -1]
	set curr_vec      vi_curr_vec$j
	
	# Check to see if the voltage vectors are the same
	set diff 0
	# First, check their lengths
	if { $volt_len != [$volt_vec length] } {
	    # if they are different, notify the user and stop saving.

	    puts "Saving to $filename"
	    set myfile [open $filename w]
	    # start at -1 to include the header line, too. 
	    for { set i -1} { $i < $volt_len } { incr i } {
		puts $myfile $line($i)
	    }
	    flush $myfile
	    close $myfile
	    exec unix_to_dos $filename
	    
	    # remember which vectors we haven't saved yet
	    set vi(first_curve_not_saved) $j

	    # Notify the user.
	    tk_messageBox -icon info -message "Saved curves $vec_start - [expr $j -1] to file $filename, but curve $j has a different number of points. You might want to hit Save again." -type ok


	    return
	# Check their, start, stop, and middle values. Middle distinguishes
	# a switch in linear/log sweep. 
	} elseif { ([set ${volt_vec}(0)] != [set ${old_volt_vec}(0)]) || \
		   ([set ${volt_vec}(end)] != [set ${old_volt_vec}(end)]) ||
		   ([set ${volt_vec}([expr int($volt_len/2)])] != [set ${old_volt_vec}([expr int($volt_len/2)])]) } {
	    
	    # This voltage vector might be different, so include it in
	    # the file. It might also be the same, and look different
	    # because of roundoff error.

	    # Append the header for this voltage vector
	    append line(-1) "\tV$j"

	    # Now set up the line vars for the next file
	    # Make the first column contain the voltage readings
	    for { set i 0} { $i < $volt_len } { incr i } {
		append line($i) "\t[set ${volt_vec}($i)]"
	    }
	    
	}
	# Append the header for this current vector
	append line(-1) "\tI$j"

	# Append the current vector's elements
	for { set i 0} { $i < $volt_len } { incr i } {
	    # append element of vector onto text variables
	    append line($i) "\t[set ${curr_vec}($i)]"
	}
	incr j
    }
    #Save the last file.
    puts "Saving to $filename"
    set myfile [open $filename w]
    # start at -1 to include the header line, too. 
    for { set i -1} { $i < $volt_len } { incr i } {
	puts $myfile $line($i)
    }
    flush $myfile
    close $myfile
    exec unix_to_dos $filename

    set vi(first_curve_not_saved) $j
    tk_messageBox -icon info -message "Saved curves $vec_start - [expr $j -1] to file $filename." -type ok

}

# This widget has a problem - if you type in a file name, it doesn't
# return the path that you selected with the "filter" box.
#iwidgets::extfileselectiondialog $vi_win.efsd -modality application 

# Allow the user to save 
#button $widgets(bottom).select -text "Select file to export data..." -command {
#    if {[$vi_win.efsd activate]} {
#        vi_save_data [$vi_win.efsd get]
#    } else {
#        #puts ""
#    }
#}
button $widgets(bottom).select -text "Select file to export data..." -command {
    set types { {"All files" *} }
    set file [tk_getSaveFile -filetypes $types -initialfile ivdata ]    
    if {$file != ""} {
        vi_save_data $file
    } 
    # otherwise do nothing.
}
pack $widgets(bottom).select -side left


# If the C code sets this variable to 1, we will pop up the dialog 
# box to save IV curves into a file. In particular, this should happen
# after a set of IV curves has been taken. 
proc vi_save_curve {name element op} {
    global widgets
    $widgets(bottom).select invoke
}

set vi(save_curves_now) 0
trace variable vi(save_curves_now) w "vi_save_curve"



# Graphs the data in vectors name_x and name_y
proc vi_add_chart_element { name_x name_y id } {
    global vi 
    if { [$vi(chart) element exists $name_y] } { return ; }

    $vi(chart) element create $id \
	-xdata $name_x -ydata $name_y -color [vi_unique_color $id]
# Limit the number of chart elements displayed to 9
    if { $id >= 9 } {
	if { [$vi(chart) element exists [expr $id -9]] } {
	    $vi(chart) element delete [expr $id -9]
	}
    }
}

proc clear_curves_now  {} {
    global vi
    set elem_list [$vi(chart) element names]
    foreach id $elem_list {
	$vi(chart) element delete $id
    }

}

proc change_source {} {
    global vi
    global widgets

    if {[$widgets(source) get]!= ""} {
	set vi(source) [$widgets(source) get]
	# If we source voltage, leave axis as-is. If we source
	# current, invert them.
	$vi(chart) configure -invertxy $vi(source) 
	
	# Set the defaults for the limits of the sweep. It will be
	#based on the default values or the value we last set when entering
	#this type of sweep.
	set temp_start $vi(sweep_start)
	set temp_stop $vi(sweep_stop)
	set vi(sweep_start) $vi(sweep_start_save)
	set vi(sweep_stop) $vi(sweep_stop_save)
	# save values we were using for the next time we enter this kind
	# of sweep. 
	set vi(sweep_start_save) $temp_start
	set vi(sweep_stop_save) $temp_stop 
    }
}

# return one of several unique colors, based on an integer index. 
# OK, I know it's not unique, but it's close...
proc vi_unique_color {index} {
    set r 0
    set g 0
    set b 0
    set index [expr $index%9]
    switch -- $index {
	0 {
	    #red
	    set r 255
	    set g 0
	    set b 0
	}
	1 {
	    #green
	    set r 0
	    set g 255
	    set b 0
	}
	2 {
	    #orange
	    set r 255
	    set g 180
	    set b 0
	}
	3 {
	    #yellow
	    set r 255
	    set g 255
	    set b 0
	}
	4 {
	    #cyan
	    set r 0
	    set g 255
	    set b 255
	}
	5 {
	    # purple
	    set r 255
	    set g 0
	    set b 255
	}
	6 {
	    # white
	    set r 255
	    set g 255
	    set b 255
	}
	7 {
	    #blue isn't very visible on black background
	    set r 0
	    set g 0
	    set b 255
	}
	default {
	    set r 128
	    set g 128
	    set b 128
	}
    }
    set color [format "#%2.2x%2.2x%2.2x" $r $g $b]
    
    
    return $color
}

# Uncomment to test graphing or saving some data.
#vector vi_volt_vec0 vi_volt_vec1 vi_curr_vec0 vi_curr_vec1
#vi_volt_vec0 seq 20 40 1
#vi_volt_vec1 seq 21 42 1
#vi_curr_vec0 seq 40 20 -1
#vi_curr_vec1 seq 30 51 1

#vi_add_chart_element vi_volt_vec0 vi_curr_vec0 0
#vi_add_chart_element vi_volt_vec0 vi_curr_vec1 1
#$vi(chart) element delete 0

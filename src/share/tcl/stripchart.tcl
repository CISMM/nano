#!/bin/sh
# the next line restarts using wishx (see man wish) \
	exec wish "$0" "$@"

#!/net/nano/nano3/tcltk/bin/bltwish
#!../bltwish

#options for stripchart
#option add *x.autoRange 25.0
#option add *x.shiftBy 0.1
option add *Stripchart.*bufferElements no
#makes it graph lines instead of scattered points
option add *Stripchart.*symbol ""
option add *Stripchart.*pixels 1.25m

option add *Stripchart.*PlotPad 0
option add *Stripchart.width 700
option add *Stripchart.height 200
option add *Stripchart.*Smooth linear
option add *Stripchart.plotBackground black

#options for tabbed notebook
option add *Tabnotebook.equalTabs no startupFile
option add *Tabnotebook.backdrop tan startupFile
option add *Tabnotebook.tabBackground tan startupFile

# ----------------------------------------------------------------------
set graphmod(sc) [create_closing_toplevel stripchart "Stripchart Tool"]
# We can show the window with show.stripchart procedure.


# ----------------------------------------------------------------------
# We used to have a "bottom", too, but now it's a pop-up window. 
set graphmod(top) [frame $graphmod(sc).top]
pack $graphmod(sc).top -expand yes -fill both

# ----------------------------------------------------------------------
# topmargin of 5 forces the graph to fit snugly against the top of the window.
# title is blank to save space.
set graphmod(stripchart) [stripchart $graphmod(top).sc -title "" -topmargin 5]
pack $graphmod(stripchart) -expand yes -fill both

#$graphmod(stripchart) xaxis configure -title "Time (sec)"
$graphmod(stripchart) yaxis configure -title "Point Results"

# Four possibilities for the x axis: time, arclength (s), x and y (on
# the sample surface). Configure these to autorange and shift
# individually, because configuring "xaxis" only affects the current
# x-axis
$graphmod(stripchart) axis create gm_timevec -title "Time (sec)" \
    -autorange 25.0 -shiftby 0.1 
$graphmod(stripchart) axis create gm_svec -title "Path Length (nm)" \
    -autorange 2000.0 -shiftby 0.1
$graphmod(stripchart) axis create gm_xsurfvec_x -title "Surface X axis (nm)" \
    -autorange 2000.0 -shiftby 0.1
$graphmod(stripchart) axis create gm_ysurfvec_x -title "Surface Y axis (nm)" \
    -autorange 2000.0 -shiftby 0.1

$graphmod(stripchart) xaxis use gm_timevec
set graphmod(cur_x_axis) gm_timevec


proc set_for_scanline_mode {} {
    global graphmod

    $graphmod(stripchart) axis configure gm_timevec \
	-autorange 0.0 -shiftby 0.0
    $graphmod(stripchart) axis configure gm_svec \
	-autorange 0.0 -shiftby 0.0
    $graphmod(stripchart) axis configure gm_xsurfvec_x \
	-autorange 0.0 -shiftby 0.0
    $graphmod(stripchart) axis configure gm_ysurfvec_x \
	-autorange 0.0 -shiftby 0.0
}

proc set_for_point_mode {} {
    global graphmod

    $graphmod(stripchart) axis configure gm_timevec \
        -autorange 25.0 -shiftby 0.1
    $graphmod(stripchart) axis configure gm_svec \
        -autorange 2000.0 -shiftby 0.1
    $graphmod(stripchart) axis configure gm_xsurfvec_x \
        -autorange 2000.0 -shiftby 0.1
    $graphmod(stripchart) axis configure gm_ysurfvec_x \
        -autorange 2000.0 -shiftby 0.1

}

# Changes the x axis back and forth between time, arclength
# and the x and y position on the surface.
proc change_x_axis { newaxis {newdata ""}} {
    global graphmod

    set graphmod(cur_x_axis) $newaxis

    $graphmod(stripchart) xaxis use $newaxis

    # The axis and the data are named the same.
    if { $newdata == "" } { 
	foreach name [$graphmod(stripchart) element names] {
	    $graphmod(stripchart) element configure $name \
		-xdata $newaxis -mapx $newaxis
	}
    } else {
	# The axis and the data are named differently.
	foreach name [$graphmod(stripchart) element names] {
	    $graphmod(stripchart) element configure $name \
		-xdata $newdata -mapx $newaxis
	}
    }
}

set graphmod(axis_config) [create_closing_toplevel stripchart_axis_config]
button $graphmod(top).axis_config -text "Axis config..." -command \
	show.stripchart_axis_config
pack $graphmod(top).axis_config -side bottom -anchor nw

#set up the controls to control the X axis, which will display either
# time, length along path (s), surface x distance or surface y distance.
proc add_time_dist_controls { } {
    global graphmod

    set win [frame $graphmod(axis_config).time_dist -relief raised -bd 2]
    pack $win -side left -expand yes -fill both

    pack [label $win.label -text "X axis controls"] -side top -anchor nw
    iwidgets::radiobox $win.rb -labeltext "X axis" -labelpos nw

    # switch which variable corresponds to the X axis
    $win.rb add time -text "Time" -command {change_x_axis gm_timevec}
    $win.rb add dist -text "Arc length" -command {change_x_axis gm_svec}
    # Add a trailing _x to the axis namse so they are not the same as the
    # axis added in "Graphmod.C" by calling "add_stripchart_element"
    # However, we need to pass in vector name without trailing "_x" 
    # so we get the real data.
    $win.rb add xdist -text "Surface X axis" \
	-command {change_x_axis gm_xsurfvec_x gm_xsurfvec}
    $win.rb add ydist -text "Surface Y axis" \
	-command {change_x_axis gm_ysurfvec_x gm_ysurfvec}
    $win.rb select dist

    pack $win.rb -padx 4 -pady 4 -side top -anchor nw

    # change the range of the X axis 
    # Don't set it's min and max, because we want it to slide with
    # new values.
    iwidgets::entryfield $win.range -labeltext "Range:" -validate real \
	-width 12 \
	-command "\$graphmod(stripchart) axis configure \$graphmod(cur_x_axis) -autorange \[$win.range get\]"
    pack $win.range -side top -anchor nw -pady 4 

    generic_entry $win.max_points gm_max_num_points "Max pts:" numeric
    generic_entry $win.stride gm_stride "Use every nth pt:" numeric
    pack $win.max_points $win.stride -side top -anchor nw -pady 4 
}

#set time/dist controls up!
add_time_dist_controls

# create an element which will be graphed in the stripchart. A BLT
# vector named $name should exist before this procedure is called, and
# the vectors for the x axis (gm_timevec, gm_svec, gm_xsurfvec,
# gm_ysurfvec) should exist as well. $id is used to get a unique color
# for this element, and to determine whether it is the first element,
# so should be linked to the main y axis. $intensity should be between
# 0 and 1 and is a factor which multiplies the color.

# min and max determine the scale for the y axis, if they are both 0 then
# the min and max for the y axis are set from the minimum and maximum in the
# data vector. This argument was added for the purpose of ensuring that
# a set of vectors representing the same type of data 
# will be displayed on the same y axis so that they can be compared to
# each other.

proc add_stripchart_element { name id intensity min max} {
    global graphmod

    if { [$graphmod(stripchart) element exists $name] } { 
        if { $min != 0 || $max != 0} {
	    $graphmod(stripchart) axis configure $name -min $min -max $max
	} 
	return; 
    }

    $graphmod(stripchart) element create $name \
	-xdata $graphmod(cur_x_axis) -ydata $name -color [unique_color $id $intensity]
    # new BLT version
    if {[catch {$graphmod(stripchart) axis create $name \
	    -limitscolor [unique_color $id $intensity] \
	    -limitsformat \"%4.4g\" -title $name } ]} {
	# Do it the old way if the new way fails:
	$graphmod(stripchart) axis create $name \
	    -limitcolor [unique_color $id $intensity] \
	    -limits \"%4.4g\" -title $name
	
    }
    $graphmod(stripchart) element configure $name \
	-mapy $name -mapx $graphmod(cur_x_axis)
    if { $id == 0 } {
	$graphmod(stripchart) yaxis use $name
    }

    if { $min != 0 || $max != 0} {
	$graphmod(stripchart) axis configure $name -min $min -max $max
    }

    # Creates some controls for an element in the stripchart. 
    # A checkbox turns the graph of the element on and off.
    # A checkbox to invert the Y axis of the element
    # A button to let this element use the main Y axis.
    # Entries allow the min and max of the axis to be set. 
    set win [frame $graphmod(axis_config).$name -relief raised -bd 2]
    pack $win -side left -expand yes -fill both
    scan $name gm_%s labelname
    pack [label $win.label -text "$labelname"] -side top -anchor nw

    checkbutton $win.active -text "Active" -variable graphmod($name-active) \
	-command "
	if { \$graphmod($name-active) } {
          \$graphmod(stripchart) element configure $name -hide no
        } else {
          \$graphmod(stripchart) element configure $name -hide yes
        }"
    button $win.getaxis -text "Use Y axis" \
	-command "\$graphmod(stripchart) yaxis use $name"
    checkbutton $win.invert -text "Invert" -variable graphmod($name-invert) \
	-command "
	if { \$graphmod($name-invert) } {
          \$graphmod(stripchart) axis configure $name -descending yes
        } else {
          \$graphmod(stripchart) axis configure $name -descending no
        }"


    pack $win.active $win.invert $win.getaxis -side top -anchor nw
    iwidgets::entryfield $win.max -labeltext "Max:" -validate real -width 12 \
	-command "\$graphmod(stripchart) axis configure $name -max \[$win.max get\]"
    iwidgets::entryfield $win.min -labeltext "Min:" -validate real -width 12 \
	-command "\$graphmod(stripchart) axis configure $name -min \[$win.min get\]"
    iwidgets::Labeledwidget::alignlabels  $win.max $win.min
    pack $win.max $win.min -side top -anchor nw

    # Now we do some fudging to only turn on those axis that we think
    # are interesting
    if { ("$name" == "gm_Topography_nm") || ("$name"=="gm_Lateral_Force_nA") } {
	$win.active select
	$win.getaxis invoke
    } else {
	# Here we will select it to turn the state on, 
	# then invoke it to turn the state off and run the command
	# which will hide the graph element
	$win.active select
	$win.active invoke
    }

}

# name should be the name passed to add_stripchart_element
proc remove_stripchart_element { name } {
    global graphmod

    if { [$graphmod(stripchart) element exists $name] } {
	#puts "removing $name graph"
	# for some reason, the limits don't go away when you delete the vector
	$graphmod(stripchart) axis configure $name -limits \"\"
	$graphmod(stripchart) axis delete $name
	$graphmod(stripchart) element delete $name
	$graphmod(notebook) delete $name
    }
}

# return one of several unique colors, based on an integer index. intensity
# is multiplied times that color to get the actual color
# OK, I know it's not unique, but it's close...
proc unique_color {index intensity} {
    set r 0
    set g 0
    set b 0
    set index [expr $index%9]
    switch -- $index {
	0 {
	    #red
	    set r [expr int(255*$intensity)]
	    set g 0
	    set b 0
	}
	1 {
	    #green
	    set r 0
	    set g [expr int(255*$intensity)]
	    set b 0
	}
	2 {
	    #orange
	    set r [expr int(255*$intensity)]
	    set g [expr int(180*$intensity)]
	    set b 0
	}
	3 {
	    #yellow
	    set r [expr int(255*$intensity)]
	    set g [expr int(255*$intensity)]
	    set b 0
	}
	4 {
	    #cyan
	    set r 0
	    set g [expr int(255*$intensity)]
	    set b [expr int(255*$intensity)]
	}
	5 {
	    # purple
	    set r [expr int(255*$intensity)]
	    set g 0
	    set b [expr int(255*$intensity)]
	}
	6 {
	    # white
	    set r [expr int(255*$intensity)]
	    set g [expr int(255*$intensity)]
	    set b [expr int(255*$intensity)]
	}
	7 {
	    #blue isn't very visible on black background
	    set r 0
	    set g 0
	    set b [expr int(255*$intensity)]
	}
	default {
	    set r [expr int(200*$intensity)]
	    set g [expr int(200*$intensity)]
	    set b [expr int(200*$intensity)]
	}
    }
    set color [format "#%2.2x%2.2x%2.2x" $r $g $b]
    
    
    return $color
}




#!/bin/sh
# the next line restarts using wishx (see man wish) \
	exec wish "$0" "$@"

#!/net/nano/nano3/tcltk/bin/bltwish
#!../bltwish

# Import Itcl and Iwidgets, for the tabnotebook widget and others we
# will use. This import method is a BUG - we shouldn't have to import
# the itcl and itk namespaces, but at least we can use it...
package require Itcl 
catch { namespace import itcl::* }
package require Itk 
catch { namespace import itk::* }
package require Iwidgets

# Include the BLT package, which provides vectors, stripchart and
# graph widgets
package require BLT
catch { namespace import blt::* }
catch { namespace import -force blt::tile::* }

#options for stripchart
#option add *x.autoRange 10.0
#option add *x.shiftBy 0.1
option add *Stripchart.*bufferElements no
#makes it graph lines instead of scattered points
option add *Stripchart.*symbol ""
option add *Stripchart.*pixels 1.25m

option add *Stripchart.*PlotPad 25
option add *Stripchart.width 800
option add *Stripchart.height 400
option add *Stripchart.*Smooth linear
option add *Stripchart.plotBackground black

#options for tabbed notebook
option add *Tabnotebook.equalTabs no startupFile
option add *Tabnotebook.backdrop tan startupFile
option add *Tabnotebook.tabBackground tan startupFile

# ----------------------------------------------------------------------
set graphmod(sc) .stripchart

# ----------------------------------------------------------------------
toplevel $graphmod(sc)
wm withdraw $graphmod(sc)

button $graphmod(sc).close -text "Close" -command {
    wm withdraw $graphmod(sc)
}
wm protocol $graphmod(sc) WM_DELETE_WINDOW {$graphmod(sc).close invoke}
pack $graphmod(sc).close -anchor nw

# ----------------------------------------------------------------------
set pw [iwidgets::panedwindow $graphmod(sc).pw -width 820 -height 600]
pack $pw -expand yes -fill both 

$pw add "top"
set graphmod(top) [$pw childsite "top"]
$pw add "bottom"
set graphmod(bottom) [$pw childsite "bottom"]
$pw fraction 70 30

# ----------------------------------------------------------------------
set graphmod(stripchart) [stripchart $graphmod(top).sc -title "Stripchart"]
pack $graphmod(stripchart) -expand yes -fill both

#$graphmod(stripchart) xaxis configure -title "Time (sec)"
$graphmod(stripchart) yaxis configure -title "Point Results"

# Four possibilities for the x axis: time, arclength (s), x and y (on
# the sample surface). Configure these to autorange and shift
# individually, because configuring "xaxis" only affects the current
# x-axis
$graphmod(stripchart) axis create gm_timevec -title "Time (sec)" \
    -autorange 10.0 -shiftby 0.1
$graphmod(stripchart) axis create gm_svec -title "Path Length (nm)" \
    -autorange 400.0 -shiftby 0.1
$graphmod(stripchart) axis create gm_xsurfvec_x -title "Surface X axis (nm)" \
    -autorange 400.0 -shiftby 0.1
$graphmod(stripchart) axis create gm_ysurfvec_x -title "Surface Y axis (nm)" \
    -autorange 400.0 -shiftby 0.1

$graphmod(stripchart) xaxis use gm_timevec
set graphmod(cur_x_axis) gm_timevec

frame $graphmod(bottom).sources
pack $graphmod(bottom).sources -expand yes -fill both -padx 10 -pady 4
set graphmod(notebook) [iwidgets::tabnotebook $graphmod(bottom).sources.tabnote \
			    -tabpos w]
pack $graphmod(notebook) -side left -expand yes -fill both -padx 4 -pady 4

proc set_for_scanline_mode {} {
    global graphmod

    $graphmod(stripchart) yaxis configure -title "Scanline Results"
    $graphmod(stripchart) configure -title "Stripchart - Scanline Mode"
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

    $graphmod(stripchart) yaxis configure -title "Point Results"
    $graphmod(stripchart) configure -title "Stripchart - Modify Mode"
    $graphmod(stripchart) axis configure gm_timevec \
        -autorange 10.0 -shiftby 0.1
    $graphmod(stripchart) axis configure gm_svec \
        -autorange 400.0 -shiftby 0.1
    $graphmod(stripchart) axis configure gm_xsurfvec_x \
        -autorange 400.0 -shiftby 0.1
    $graphmod(stripchart) axis configure gm_ysurfvec_x \
        -autorange 400.0 -shiftby 0.1

}

proc show_stripchart_win {} {
    global graphmod
    wm deiconify $graphmod(sc)
    raise $graphmod(sc)
}

# Changes the x axis back and forth between time and arclength
proc change_x_axis { newaxis {newdata ""}} {
    global graphmod

    set graphmod(cur_x_axis) $newaxis

    $graphmod(stripchart) xaxis use $newaxis

    if { $newdata == "" } { 
	foreach name [$graphmod(stripchart) element names] {
	    $graphmod(stripchart) element configure $name \
		-xdata $newaxis -mapx $newaxis
	}
    } else {
	foreach name [$graphmod(stripchart) element names] {
	    $graphmod(stripchart) element configure $name \
		-xdata $newdata -mapx $newaxis
	}
    }
}

#set up the controls to control the X axis, which will display either
# time, length along path (s), surface x distance or surface y distance.
proc add_time_dist_controls { } {
    global graphmod

    set win [$graphmod(notebook) add  -label "X axis"]
    $graphmod(notebook) select "X axis"
    
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
    $win.rb select time

    pack $win.rb -padx 4 -pady 4 -side left -anchor nw

    # change the range of the X axis 
    # Don't set it's min and max, because we want it to slide with
    # new values.
    iwidgets::entryfield $win.range -labeltext "Range:" -validate real \
	-width 12 \
	-command "\$graphmod(stripchart) axis configure \$graphmod(cur_x_axis) -autorange \[$win.range get\]"
    pack $win.range -side left -anchor nw

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
    $graphmod(stripchart) axis create $name -limitcolor [unique_color $id $intensity] \
	-limits \"%4.4g\" -title $name
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
    set win [$graphmod(notebook) add  -label $name]
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
    $win.active select
    iwidgets::entryfield $win.max -labeltext "Max:" -validate real -width 12 \
	-command "\$graphmod(stripchart) axis configure $name -max \[$win.max get\]"
    iwidgets::entryfield $win.min -labeltext "Min:" -validate real -width 12 \
	-command "\$graphmod(stripchart) axis configure $name -min \[$win.min get\]"
    pack $win.max $win.min -side top -anchor nw
}

# name should be the name passed to add_stripchart_element
proc remove_stripchart_element { name } {
    global graphmod

    if { [$graphmod(stripchart) element exists $name] } {
	puts "removing $name graph"
	# for some reason, the limits don't go away when you delete the vector
	$graphmod(stripchart) axis configure $name -limits \"\"
	$graphmod(stripchart) axis delete $name
	$graphmod(stripchart) element delete $name
	$graphmod(notebook) delete $name
	return}

     return
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




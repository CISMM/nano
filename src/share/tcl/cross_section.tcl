#/*===3rdtech===
#  Copyright (c) 2000 by 3rdTech, Inc.
#  All Rights Reserved.
#
#  This file may not be distributed without the permission of 
#  3rdTech, Inc. 
#  ===3rdtech===*/
#!/bin/sh
# the next line restarts using wishx (see man wish) \
	exec wish "$0" ${1+"$@"}


#################################################################
# ----------------------------------------------------------------------
set xswidget(main) [create_closing_toplevel cross_section \
        "Cross Section Analysis"]
# We can show the window with show.cross_section procedure.
set xswidget(display_data) [create_closing_toplevel cross_section_data \
        "Cross Section Data"]


# ----------------------------------------------------------------------
set xswidget(cntrl) [frame $xswidget(main).cntrl]
pack $xswidget(cntrl) -side left -expand yes -fill both

# ----------------------------------------------------------------------
# topmargin of 5 forces the graph to fit snugly against the top of the window.
# title is blank to save space.
# This could easily be "graph" instead of "stripchart", but they are 
# essentially the same, and option are already set up for stripchart. 
set xswidget(stripchart) [stripchart $xswidget(main).sc -title "" -topmargin 5]
pack $xswidget(stripchart) -side left -expand yes -fill both

$xswidget(stripchart) yaxis configure -title "Height (nm)"

# Hide the legend - we'll display legend-like info elsewhere. 
$xswidget(stripchart) legend configure -hide yes

# Cross sections graphed vs path length only.
#$xswidget(stripchart) axis create xs_svec -title "Path Length (nm)" 
#$xswidget(stripchart) xaxis use xs_svec

$xswidget(stripchart) marker create line -name meas1 -xor no -linewidth 2 \
        -coords "0.5 0 0.5 1" -outline red -bindtags meas
$xswidget(stripchart) marker create line -name meas2 -xor no -linewidth 2 \
        -coords "1 0 1 1" -outline yellow -bindtags meas
$xswidget(stripchart) marker bind meas <B1-Motion> {
    set x [$xswidget(stripchart) axis invtransform x %x]
    if {$x < 0 } { set x 0 }
    if {$x > $xs0_Path(299) } { set x $xs0_Path(299) }
    catch { %W marker configure [%W marker get current] -coords "$x 0 $x 1" }
    show_xs_values $x [%W marker get current]
}

set xswidget(data1) [frame $xswidget(display_data).f1]
set xswidget(data2) [frame $xswidget(display_data).f2]
set xswidget(data3) [frame $xswidget(display_data).f3]
pack $xswidget(data1) $xswidget(data2) $xswidget(data3) \
        -side top -expand yes -fill both
set xsect(data1_init) 0

proc show_xs_values { x marker } {
    global xsect xswidget
    set gridrow 1
    
    # Find the proper index into the data vectors to display.
    # Hack - we know it's 300 elements long. 
    set vecname "xs0"
    upvar ${vecname}_Path pathvec
    if { $marker == "meas1" } {
        set index1 [expr (( $x - $pathvec(0)) \
                / ($pathvec(end) - $pathvec(0)) ) \
                * ([${vecname}_Path length] -1)]
        set coords [$xswidget(stripchart) marker cget meas2 -coords]
        set index2 [expr (( [lindex $coords 0] - $pathvec(0)) \
                / ($pathvec(end) - $pathvec(0)) ) \
                * ([${vecname}_Path length] -1)]
    } else {
        set index2 [expr (( $x - $pathvec(0)) \
                / ($pathvec(end) - $pathvec(0)) ) \
                * ([${vecname}_Path length] -1)]
        set coords [$xswidget(stripchart) marker cget meas1 -coords]
        set index1 [expr (( [lindex $coords 0] - $pathvec(0)) \
                / ($pathvec(end) - $pathvec(0)) ) \
                * ([${vecname}_Path length] -1)]

    }

    #puts -nonewline "$marker $index"
    foreach datavec [lsort [info globals "${vecname}_*"]] {
        if { $datavec != "${vecname}_Path" } {
            upvar $datavec dvec
            $xswidget(data1).data_m1_$gridrow configure -text \
                    "[format %.3f $dvec($index1)]"
             $xswidget(data1).data_m2_$gridrow configure -text \
                    "[format %.3f $dvec($index2)]"
             $xswidget(data1).diff_$gridrow configure -text \
                    "[format %.3f [expr $dvec($index1) - $dvec($index2)]]"
            incr gridrow
        }
    }
    #puts ""
}
# ----------------------------------------------------------------------
set xswidget(point_frame) [frame $xswidget(cntrl).point_frame]
pack $xswidget(point_frame) -side right -expand yes -fill both

checkbutton $xswidget(cntrl).active -text "Active" -variable xsect(active)
pack $xswidget(cntrl).active -side top

generic_radiobox $xswidget(cntrl).type xsect(type) "Type" { "Free" "Constr 45" }
pack $xswidget(cntrl).type -side top

set xswidget(del_frame) [frame $xswidget(cntrl).del_frame]
pack $xswidget(del_frame) -side top -expand yes -fill both



proc create_new_cross_section { vecname index } {
    upvar #0 $vecname datavec
    global xsect xswidget
    set gridrow 1

    # Create controls for this cross-section
    button $xswidget(del_frame).$vecname -text "Delete $vecname" 
    pack $xswidget(del_frame).$vecname -side top

    set win $xswidget(point_frame)
    # labels for controlling this cross section
    grid x x [label $win.l1 -text "Active"] \
            [label $win.l2 -text "Inv"] \
            [label $win.l3 -text "Use Y"] \
            [label $win.l4 -text "Min"] \
            [label $win.l5 -text "Max"] 
    set intens 1.0
    grid x x [label $xswidget(data1).l1 -text "Red"] \
            [label $xswidget(data1).l2 -text "Yellow"] \
            [label $xswidget(data1).l3 -text "Diff"] 
            
    # labels for data from this cross section
    
    # Create the graphs of this cross-section
    # Title strips off the leading "xs0_"
    foreach datavec [lsort [info globals "${vecname}_*"]] {
        if { $datavec != "${vecname}_Path" } {
            $xswidget(stripchart) element create $datavec \
                    -xdata ${vecname}_Path -ydata $datavec \
                    -color [unique_color $index $intens] \
                    -label [string range $datavec 4 end]
            $xswidget(stripchart) axis create $datavec \
                    -limitscolor [unique_color $index $intens] \
                    -limitsformat \"%4.4g\" \
                    -title [string range $datavec 4 end]
            $xswidget(stripchart) element configure $datavec \
                    -mapy $datavec

            # Creates some controls for an element in the stripchart. 
            # A checkbox turns the graph of the element on and off.
            # A checkbox to invert the Y axis of the element
            # A button to let this element use the main Y axis.
            # Entries allow the min and max of the axis to be set. 
    #set win [frame $xswidget(point_frame).$datavec -relief raised -bd 2]
    #pack $win -side left -expand yes -fill both

            frame $win.line$gridrow -width 20 -height 3 \
                    -bg [unique_color $index $intens]
            scan $datavec xs%d_%s bogus labelname
            label $win.label$gridrow -text "$labelname"

            # When un-checked, stop graphing this data, stop displaying
            # element in the legend, hide it's axis
            set xswidget($datavec-active) 1
    checkbutton $win.active$gridrow  -variable xswidget($datavec-active) \
	-command "
	if { \$xswidget($datavec-active) } {
          \$xswidget(stripchart) element configure $datavec -hide no \
                  -label [string range $datavec 4 end]
          \$xswidget(stripchart) axis configure $datavec -hide no 
#-limitsformat \"%4.4g\"
        } else {
          \$xswidget(stripchart) element configure $datavec -hide yes -label \"\"
          \$xswidget(stripchart) axis configure $datavec -hide yes 
#-limitsformat \"\"
        }"
    checkbutton $win.getaxis$gridrow -variable xswidget($datavec-getaxis) \
            -command "
            if { \$xswidget($datavec-active) } { 
                \$xswidget(stripchart) yaxis use $datavec 
            }"
    checkbutton $win.invert$gridrow -variable xswidget($datavec-invert) \
	-command "
	if { \$xswidget($datavec-invert) } {
          \$xswidget(stripchart) axis configure $datavec -descending yes
        } else {
          \$xswidget(stripchart) axis configure $datavec -descending no
        }"

            #pack $win.active $win.invert $win.getaxis -side top -anchor nw
            generic_entry $win.max$gridrow ${datavec}_max "" real \
                    "axis_max_change ${datavec}_max ${datavec}_min $datavec"
            generic_entry $win.min$gridrow ${datavec}_min "" real \
                    "axis_min_change ${datavec}_min ${datavec}_max $datavec"
            
            grid $win.line$gridrow $win.label$gridrow $win.active$gridrow \
                    $win.invert$gridrow \
                    $win.getaxis$gridrow \
                    $win.max$gridrow $win.min$gridrow \
                    -row $gridrow -sticky w
            #pack $win.max $win.min -side top -anchor nw
            
            # Now create some widgets to display data for this xs
            frame $xswidget(data1).line$gridrow -width 20 -height 3 \
                    -bg [unique_color $index $intens]
            label $xswidget(data1).label$gridrow -text "$labelname"
            label $xswidget(data1).data_m1_$gridrow 
            label $xswidget(data1).data_m2_$gridrow 
            label $xswidget(data1).diff_$gridrow 

            grid $xswidget(data1).line$gridrow \
                    $xswidget(data1).label$gridrow \
                    $xswidget(data1).data_m1_$gridrow \
                    $xswidget(data1).data_m2_$gridrow \
                    $xswidget(data1).diff_$gridrow \
                    -sticky e

            set intens [expr $intens -0.2]
            incr gridrow

        }
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


#vector s1_Path s1_Topography s1_Z_Piezo 
#s1_Path set { 0 1 2 3 }
#s1_Topography set { 3 4 2 1 }
#s1_Z_Piezo set { 9 4.5 5 3 }
#create_new_cross_section s1 1

#vector s2_Path s2_Topography s2_Z_Piezo 
#s2_Path set { 0 1.2 2.8 4.5 }
#s2_Topography set { .3 4.2 4.3 3.8 }
#s2_Z_Piezo set { 2 3 4 5.4 }
#create_new_cross_section s2 2




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

package require Itcl 
namespace import itcl::*
package require Itk 
namespace import itk::*
package require Iwidgets

# Include the BLT package, which provides vectors, stripchart and
# graph widgets
package require BLT
namespace import blt::*
namespace import -force blt::tile::*

# This needs to be made dependent on how big the font is on the screen.
catch { option add *font {helvetica -15 } startupFile}
catch { option add *Font {helvetica -15 } startupFile}

#options for stripchart
#option add *x.autoRange 25.0
#option add *x.shiftBy 0.1
option add *Stripchart.*bufferElements no
#makes it graph lines instead of scattered points
option add *Stripchart.*symbol ""
option add *Stripchart.*pixels 1.25m

option add *Stripchart.*PlotPad 0
option add *Stripchart.width 665
option add *Stripchart.height 200
option add *Stripchart.*Smooth linear
option add *Stripchart.plotBackground black

# where do I find supporting files?
# If the environment variable NM_TCL_DIR is not set, 
# check for NANO_ROOT. If that is not set, 
# assume files are in the current directory. Allows
# "wish mainwin.tcl" to bring up the interface.
if {[info exists env(NM_TCL_DIR)] } {
    set tcl_script_dir $env(NM_TCL_DIR)
} elseif {[info exists env(NANO_ROOT)]} {
    if {$viewer_only} {
        set tcl_script_dir [file join $env(NANO_ROOT) share tcl_view]
    } else {
        set tcl_script_dir [file join $env(NANO_ROOT) share tcl]
    }
} else {
    set tcl_script_dir .
}
if {[string match -nocase "*wish*" [info nameofexecutable]] } {
    set tcl_script_dir .
}

# contains definition of useful widgets for the 
# Tcl_Linkvar library.
source [file join ${tcl_script_dir} Tcl_Linkvar_widgets.tcl]

#################################################################
# ----------------------------------------------------------------------
set xswidget(main) [create_closing_toplevel cross_section "Cross Section Analysis"]
# We can show the window with show.stripchart procedure.


# ----------------------------------------------------------------------
set xswidget(cntrl) [frame $xswidget(main).cntrl]
pack $xswidget(cntrl) -side left -expand yes -fill both

# ----------------------------------------------------------------------
# topmargin of 5 forces the graph to fit snugly against the top of the window.
# title is blank to save space.
set xswidget(stripchart) [stripchart $xswidget(main).sc -title "" -topmargin 5]
pack $xswidget(stripchart) -side left -expand yes -fill both

$xswidget(stripchart) yaxis configure -title "Height (nm)"

# Cross sections graphed vs path length only.
$xswidget(stripchart) axis create xs_svec -title "Path Length (nm)" 
$xswidget(stripchart) xaxis use xs_svec


# ----------------------------------------------------------------------
checkbutton $xswidget(cntrl).active -text "Active" -variable xsect(active)
pack $xswidget(cntrl).active -side top

generic_radiobox $xswidget(cntrl).type xsect(type) "Type" { "Free" "Constr 45" }
pack $xswidget(cntrl).type -side top

set xswidget(del_frame) [frame $xswidget(cntrl).del_frame]
pack $xswidget(del_frame) -side top -expand yes -fill both

set xswidget(point_frame) [frame $xswidget(cntrl).point_frame]
pack $xswidget(point_frame) -side top -expand yes -fill both


proc create_new_cross_section { vecname index } {
    upvar #0 $vecname datavec
    global xsect xswidget

    # Create controls for this cross-section
    button $xswidget(del_frame).$vecname -text "Delete $vecname" 
    pack $xswidget(del_frame).$vecname -side top

    set intens 1.0
    # Create the graphs of this cross-section
    foreach datavec [lsort [info globals "${vecname}_*"]] {
        if { $datavec != "${vecname}_Path" } {
            $xswidget(stripchart) element create $datavec \
                    -xdata ${vecname}_Path -ydata $datavec \
                    -color [unique_color $index $intens] \
                    -label $datavec
        }
        set intens [expr $intens -0.2]
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


vector s1_Path s1_Topography s1_Z_Piezo 
s1_Path set { 0 1 2 3 }
s1_Topography set { 3 4 2 1 }
s1_Z_Piezo set { 9 4 5 3 }
create_new_cross_section s1 1

vector s2_Path s2_Topography s2_Z_Piezo 
s2_Path set { 0 1.2 2.8 4.5 }
s2_Topography set { .3 4.2 4.3 3.8 }
s2_Z_Piezo set { 2 3 4 5.4 }
create_new_cross_section s2 2




show.cross_section
wm geom . 10x10+0+0
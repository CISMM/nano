#!/bin/sh
# ----------------------------------------------------------------------
#  PROGRAM: demo program for nanomanipulator
# ----------------------------------------------------------------------
#\
exec wish "$0" ${1+"$@"}

# Import Itcl and Iwidgets, for the hierarchy widget and others we
# will use. This import method is a BUG - we shouldn't have to import
# the itcl and itk namespaces, but at least we can use it...
package require Itcl 3.0
namespace import itcl::*
package require Itk 3.0
namespace import itk::*
package require Iwidgets 3.0

# Import BLT, for the stripchart widget.
# package require BLT

# Set up the options database. This controls the overall appearance of
# our application, without having to specify options for all the
# widgets individually.
# ----------------------------------------------------------------------
option add *background tan startupFile
option add *highlightBackground tan startupFile


# GLOBAL variables
# ----------------------------------------------------------------------
# widget is a global for containing widget paths and names.

# gstate is for all variable linked to the C code, so we can keep track.

# overload a few critical functions that might be used by demo programs...
 rename exit tcl_exit
proc exit {{status 0}} {
    global quit
    set quit 1
    tcl_exit
}

# puts executed on a single argument will be displayed in the status line
rename puts tcl_puts
proc puts {args} {
    global widgets
    if {[llength $args] == 1} {
        set_status [lindex $args 0]
    } else {
        eval tcl_puts $args
    }
}


# ----------------------------------------------------------------------
# USAGE:  iw_lock <state>
#
# Locks or unlocks the main window.  Sets a grab on the main menu,
# so that all events are sent to it.
# ----------------------------------------------------------------------
proc iw_lock {state} {
    global widgets
    if {$state} {
        grab set $widgets(mainMenu)
        . configure -cursor watch
    } else {
        grab release $widgets(mainMenu)
        . configure -cursor ""
    }
}


# ----------------------------------------------------------------------
# USAGE:  set_status <message>
#
# Sets the status line to a text message
# ----------------------------------------------------------------------
proc set_status { message } {
    global widgets
#    tcl_puts "set status $message"
# which way is better?
    set widgets(statusVar) $message
#    $widgets(status) configure -text $message
    update
}

# Sets a global variable when a radiobox changes.
proc radiobox_var_update { rb gvar } {
    upvar #0 $gvar var
# cannot use "global $gvar" - it doesn't set an array element correctly!
    set var [$rb get]
    #puts "$rb $gvar $var"
}

#debugging procedure - see if variable is changing.
proc trace_print {name element op} {
    tcl_puts "$name $element changed"
}

proc mark_move { canv x y } {
    global gstate
    $canv raise cursor
    set x [$canv canvasx $x]
    set y [$canv canvasy $y]
    set dx [expr $x - $gstate(cursor,x)]
    set dy [expr $y - $gstate(cursor,y)]
    $canv move cursor $dx $dy
    set gstate(cursor,x) $x
    set gstate(cursor,y) $y
} 

proc mark_size {canv change } {
    global gstate
    set x $gstate(cursor,x) 
    set y $gstate(cursor,y) 
    set size [expr $gstate(cursor,size) + $change]
    if { $size < 1 } { set size 1 }
    $canv coords cursor [expr $x -$size] [expr $y -$size] \
	[expr $x +$size] [expr $y +$size]
    set gstate(cursor,size) $size
}

# ----------------------------------------------------------------------
wm title . {Core prototype}
#wm geometry . 800x500

# Makes sure program exits when window is destroyed.
# Also makes program exit if ctrl-c is pressed in main window. 
wm protocol . WM_DELETE_WINDOW { exit 0 }
bind all <Control-KeyPress-c> { exit 0 } 
#bind all <KeyPress-q> { DoExit 0 }
focus .

# Menu bar
frame .mbar -borderwidth 2 -relief raised
pack .mbar -fill x
set widgets(mainMenu) [menubutton .mbar.main -text "Main" -menu .mbar.main.m]
pack .mbar.main -side left

menu .mbar.main.m
.mbar.main.m add command -label "About..." -command {.about activate}
.mbar.main.m add separator
.mbar.main.m add command -label "Quit" -command exit

# Status line, below the paned window. 
pack [frame .statusframe -relief sunken -borderwidth 2] -side bottom -fill x 
set widgets(status) [label .statusframe.status -textvariable widgets(statusVar) -justify left]
pack $widgets(status) -anchor w ;
set_status "Ready"

# ----------------------------------------------------------------------
# Main control frame
# ----------------------------------------------------------------------

#top and bottom halves
#pack [frame .image_fr] [frame .contr_fr] -expand yes -fill both
pack [frame .contr_fr] [frame .image_fr] -expand yes -fill both
# ----------------------------------------------------------------------
# Top - Image
set widgets(image_canv) [iwidgets::scrolledcanvas .image_fr.canv]
pack $widgets(image_canv) -anchor nw -padx 10 -pady 10

set gstate(cursor,x) 15
set gstate(cursor,y) 15
set gstate(cursor,size) 5
$widgets(image_canv) create oval 10 10 20 20 -outline red -tags cursor

set gstate(do_core) 0

$widgets(image_canv) bind all <Motion> { mark_move %W %x %y }
$widgets(image_canv) bind all <Button-2> { mark_size %W 1 }
$widgets(image_canv) bind all <Button-3> { mark_size %W -1 }
$widgets(image_canv) bind all <Button-1> { set gstate(do_core) 1 }

# ----------------------------------------------------------------------
# Bottom - Controls
iwidgets::labeledframe .contr_fr.ridge_op -labeltext "Ridge Options"
#iwidgets::labeledframe .contr_fr.display_op -labeltext "Display Options"
iwidgets::labeledframe .contr_fr.core_op -labeltext "Core Options"
#iwidgets::labeledframe .contr_fr.ridge_str -labeltext "Ridge Strength"

#pack .contr_fr.ridge_op .contr_fr.display_op .contr_fr.core_op \
#    .contr_fr.ridge_str \
#    -side left -expand yes -fill both

pack .contr_fr.ridge_op .contr_fr.core_op \
    -side left -expand yes -fill both

set widgets(ridge_op) [.contr_fr.ridge_op childsite]
#set widgets(display_op) [.contr_fr.display_op childsite]
set widgets(core_op) [.contr_fr.core_op childsite]
#set widgets(ridge_str) [.contr_fr.ridge_str childsite]

# Ridge options
#checkbutton $widgets(ridge_op).strong_ridge -text "Strong ridge" \
#    -variable gstate(strong_ridge)
checkbutton $widgets(ridge_op).light_dark -text "Light on Dark" \
    -variable gstate(light_dark) -command {
	if { $gstate(light_dark) == 1 } {
	    $widgets(ridge_op).light_dark configure -text "Dark on Light"
	} else {
	     $widgets(ridge_op).light_dark configure -text "Light on Dark"
	}
    }
#scale $widgets(ridge_op).sampling -from 0 -to 1.0 -label "Sampling" \
#    -variable gstate(sampling) -orient horizontal -resolution -1
#set gstate(sampling) 0.5
iwidgets::radiobox $widgets(ridge_op).medialness -labeltext "Medialness:" \
    -command "radiobox_var_update $widgets(ridge_op).medialness gstate(medialness)"

$widgets(ridge_op).medialness add 0 -text "Laplacian"
$widgets(ridge_op).medialness add 1 -text "Lpp"
$widgets(ridge_op).medialness add 2 -text "Edge"
$widgets(ridge_op).medialness select 0

#pack $widgets(ridge_op).strong_ridge $widgets(ridge_op).light_dark \
#     $widgets(ridge_op).sampling $widgets(ridge_op).medialness \
#    -side top -fill x -anchor nw

pack $widgets(ridge_op).light_dark \
     $widgets(ridge_op).medialness \
     -side top -fill x -anchor nw

# Display options
#checkbutton $widgets(display_op).core_view -text "Core View" \
#    -variable gstate(core_view)
#checkbutton $widgets(display_op).draw_discs -text "Draw Discs" \
#    -variable gstate(draw_discs)
#checkbutton $widgets(display_op).draw_boundaries -text "Draw Boundaries" \
#    -variable gstate(draw_boundaries)
#scale $widgets(display_op).radius -from 0.0 -to 2.0 -label "Radius/Scale" \
#    -variable gstate(radius) -orient horizontal -resolution -1
#set gstate(radius) 1.0
#scale $widgets(display_op).boundary_ratio -from 0.0 -to 10.0 \
#    -label "Boundary Ratio" \
#    -variable gstate(boundary_ratio) -orient horizontal -resolution -1
#set gstate(boundary_ratio) 0
#scale $widgets(display_op).zoom -from 0.5 -to 4.0 -label "Image Zoom" \
#    -variable gstate(zoom) -orient horizontal -resolution -1
#set gstate(zoom) 1.0

#pack $widgets(display_op).core_view $widgets(display_op).draw_discs \
#    $widgets(display_op).draw_boundaries $widgets(display_op).radius \
#    $widgets(display_op).boundary_ratio $widgets(display_op).zoom \
#    -side top -fill x -anchor nw

# Core options
proc delete_core { } {
    puts "Delete core"
}

proc load_image { filename } {
    global widgets gstate
    set gstate(load_this_file) $filename
    puts "Load: $gstate(load_this_file)"
    set widgets(image) [image create photo \
			    -file $gstate(load_this_file) -palette 256]
    $widgets(image_canv) create image 0 0 -image $widgets(image) \
	-anchor nw
    $widgets(image_canv) configure -width [image width $widgets(image)] \
	-height [image height $widgets(image)]
}

iwidgets::extfileselectiondialog .efsd -modality application
proc load_image_dialog { } {
    global widgets gstate
    if {[.efsd activate]} {
	load_image [.efsd get]
    } else {
#	set gstate(load_this_file) ""
	puts ""
    }
}

button $widgets(core_op).del_last -command "delete_core" \
    -text "Delete last core"
button $widgets(core_op).load_image -command "load_image_dialog" \
    -text "Load image"
set gstate(load_this_file) ""

#iwidgets::radiobox $widgets(core_op).core_dir -labeltext "Core direction:" \
#    -command "radiobox_var_update $widgets(core_op).core_dir gstate(core_dir)"

#$widgets(core_op).core_dir add both -text "Both"
#$widgets(core_op).core_dir add right -text "Right"
#$widgets(core_op).core_dir add left -text "Left"
#$widgets(core_op).core_dir select both

#pack $widgets(core_op).del_last $widgets(core_op).load_image \
#    $widgets(core_op).core_dir \
#    -side top -fill x -anchor nw

pack $widgets(core_op).del_last $widgets(core_op).load_image \
     -side top -fill x -anchor nw

#Ridge Strength
#pack [frame $widgets(ridge_str).sc] -side top  -fill x
#scale $widgets(ridge_str).sc.gradp -from 0.0 -to 1.0 -label "gradP" \
#    -variable gstate(gradP) -orient horizontal -resolution -1
#set gstate(gradP) 0.25
#scale $widgets(ridge_str).sc.alpha -from 0.0 -to 1.0 -label "alpha" \
#    -variable gstate(alpha) -orient horizontal -resolution -1
#set gstate(alpha) 0.25
#scale $widgets(ridge_str).sc.mss -from 0.0 -to 1.0 -label "Mss" \
#    -variable gstate(Mss) -orient horizontal -resolution -1
#set gstate(Mss) 0.25

#pack $widgets(ridge_str).sc.gradp $widgets(ridge_str).sc.alpha \
#    $widgets(ridge_str).sc.mss -side left -fill x -expand yes

#checkbutton $widgets(ridge_str).ridge_strength -text "Show ridge strength" \
#    -variable gstate(ridge_strength)

#pack $widgets(ridge_str).ridge_strength -side top -fill x -anchor nw

# ----------------------------------------------------------------------
# "About" window
# ----------------------------------------------------------------------
iwidgets::dialog .about -title {About: Core Software} -modality none
.about hide "Apply"
.about hide "Help"
.about hide "Cancel"
.about buttonconfigure "OK" -command ".about deactivate"
.about default "OK"

set win [.about childsite]
label $win.title -text {Nanomanipulator Core project}
pack $win.title
catch {$win.title configure -font -*-helvetica-bold-o-normal-*-*-180-*}

# set file [file join ${iwidgets::library} demos iwidgets.gif]
# label $win.icon -image [image create photo -file $file]
# pack $win.icon -side left

label $win.by -text "Interface by"
pack $win.by
catch {$win.by configure -font -*-helvetica-medium-r-normal-*-*-100-*}

label $win.authors -text "Mark Foskey
Aron Helser
"
pack $win.authors
catch {$win.authors configure -font -*-helvetica-medium-o-normal-*-*-120-*}


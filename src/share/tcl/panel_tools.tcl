#  ##############################################################
#
#  PixelFlow Software
#
#  Copyright (c) 1995 by
# 		The University of North Carolina at Chapel Hill
# 		Division Inc.
#
#  pxfl_idstring = "SW Version @(#)panel_tools.tcl	1.6 98/01/05 "
#
#  ##############################################################
#
# control panel creation tools for pftest application


##################################################
# first some code that is really executed

# tell if we're using tk or not
# if we were loaded from the pxfl app, use_tk will be 0 or 1.  So, if it's not
# defined, we must be testing the control panel without pxfl.
if {! [info exists use_tk]} {
    # if tk is loaded, a global called "tk_version" should exist
    set use_tk [expr {[info global tk_version] == {tk_version}}]
}

# change to smaller font
#if $use_tk {option add *font {-adobe-courier-*-r-*-*-14-100-*-*-m-*-*-*}}


##################################################
# all the rest should be procedures...


########################################
# label: just a text label
#
# label_create widget_path label_text
#
# creates text label.  Nothing special or particularly difficult
proc label_create {widget label_text} {
    global use_tk
    # extract name from widget path
    # and hack off the widget_name to get the path to this widget
    regexp -indices {[^.]*$} $widget widget_ind
    set w_start [lindex [split $widget_ind] 0]
    set w_end [lindex [split $widget_ind] 1]
    
    set widget_name [string range $widget $w_start $w_end]
    set widget_path [string range $widget 0 [expr $w_start -2]]

    if $use_tk {
	# check to see if the parent window exists before packing. 
	if { [winfo exist $widget_path] } {
	    pack [label $widget -text $label_text]
	}
    }

}


########################################
# pad: a 2d control surface
#
# pad_create widget_path  xmin x xmax  ymin y ymax
#
# creates a 2D pad with the given widget path (e.g. .xy0 or
#   .joy0.xy0) which allows you to place it into a frame or other such
#   window heirarchy control)
# The last element of the widget path (after the last .) is taken to
#   be the pad_name (e.g. xy0 in both of the above examples).  A
#   global array with this name is created, and all pad values are set
#   in it.  As a result, this name MUST be unique.  Otherwise .joy0.xy
#   and .joy1.xy would both be updating state in the xy array!
#
# widget_name(b) is the button state (0 = not pressed, 1 = button held down)
#
# widget_name(x) and widget_name(y) are the mouse position in the pad,
#   ranging from 0 to 1 for both x and y.

# create a control pad with [minx,maxx] x [miny,maxy] range
# and initial value x,y
proc pad_create {widget  xmin x xmax  ymin y ymax} {
    # extract name from widget path
    # and hack off the widget_name to get the path to this widget
    regexp -indices {[^.]*$} $widget widget_ind
    set w_start [lindex [split $widget_ind] 0]
    set w_end [lindex [split $widget_ind] 1]
    
    set widget_name [string range $widget $w_start $w_end]
    set widget_path [string range $widget 0 [expr $w_start -2]]

    # create global variable for interaction (and get local alias for it)
    upvar #0 $widget_name var

    # store default values and ranges
    set var(b) 0

    set var(x) $x
    set var(xbase) $xmin
    set var(xrange) [expr $xmax-$xmin]
    
    set var(y) $y
    set var(ybase) $ymax
    set var(yrange) [expr $ymin-$ymax]

    global use_tk
    if $use_tk {
	# create canvas to operate as the pad
	canvas $widget -width 200 -height 200 -relief raised \
	    -bg lightskyblue -borderwidth 3

	# compute starting center for marker circle
	set var(markx) [expr 200 * ($x-$var(xbase)) / $var(xrange)]
	set var(marky) [expr 200 * ($y-$var(ybase)) / $var(yrange)]

	# draw marker circle centered at starting location
	$widget create oval \
		[expr $var(markx)-2.5] [expr $var(marky)-2.5] \
		[expr $var(markx)+2.5] [expr $var(marky)+2.5]

	# bind control events
	bind $widget <ButtonPress-1>   {pad_update %W %x %y 1}
	bind $widget <Button1-Motion>  {pad_update %W %x %y 1}
	bind $widget <ButtonRelease-1> {pad_update %W %x %y 0}

	# instantiate widget
	pack $widget
    }
}

# update pad position
# doesn't need use_tk tests since it is only be run from tk events
proc pad_update {widget  x y button_state} {
    # extract name from widget path
    # and hack off the widget_name to get the path to this widget
    regexp -indices {[^.]*$} $widget widget_ind
    set w_start [lindex [split $widget_ind] 0]
    set w_end [lindex [split $widget_ind] 1]
    
    set widget_name [string range $widget $w_start $w_end]
    set widget_path [string range $widget 0 [expr $w_start -2]]

    # need to reference global interaction variable
    upvar #0 $widget_name var

    # set button state based on button_state parameter
    set var(b) $button_state

    # now move and update marker position
    $widget move all  [expr $x - $var(markx)] [expr $y - $var(marky)]
    set var(markx) $x
    set var(marky) $y

    # get width and height of pad
    set w [winfo width $widget]
    set h [winfo height $widget]

    # set x and y from scaled click location
    set var(x) [expr $var(xbase) + $var(xrange)*double($x)/$w]
    set var(y) [expr $var(ybase) + $var(yrange)*double($y)/$w]
}



########################################
# pad3d: combo of pad and a slider
#
# pad3d_create widget_path  xmin x xmax  ymin y ymax  zmin z zmax

# create combined pad
proc pad3d_create {widget  xmin x xmax  ymin y ymax  zmin z zmax} {
    # extract name from widget path
    # and hack off the widget_name to get the path to this widget
    regexp -indices {[^.]*$} $widget widget_ind
    set w_start [lindex [split $widget_ind] 0]
    set w_end [lindex [split $widget_ind] 1]
    
    set widget_name [string range $widget $w_start $w_end]
    set widget_path [string range $widget 0 [expr $w_start -2]]

    # create global variable for interaction (and get local alias for it)
    upvar #0 $widget_name var
    
    # store default value (for z, pad_create will handle x & y)
    set var(z) $z
    
    # create frames to contain both widgets
    global use_tk
    if $use_tk {
	if { [winfo exist $widget_path] } {
	    pack [frame $widget]
	    pack [frame $widget.slider] [frame $widget.pad] -side bottom
	}
    }

    # create pad for x and y
    pad_create $widget.pad.$widget_name  $xmin $x $xmax  $ymin $y $ymax

    # create slider for z
    if $use_tk {
	# create slider
	if { [winfo exist $widget_path] } {
	    pack [scale $widget.slider.$widget_name -length 200 -width 10 \
		-orient horizontal  -showval 0 \
		-from $zmin -to $zmax  -resolution -1 \
		-variable ${widget_name}(z)]
	}
    }
}


########################################
# joypad: another 2d control surface
#
# joypad_create widget_path
#
# Similar to pad, but range is from -.5 to .5 for x and y.  Also, there
#   is a dead zone in the center tenth where the value is 0 (allowing
#   change in one axis only without accidentally changing the other).
#   Unlike the pad, when the button is released, the joypad recenters
#   to 0,0

# create a joypad
# mouse operation in the pad causes changes to pad(x), pad(y), and pad(b)
proc joypad_create {widget} {
    # extract name from widget path
    # and hack off the widget_name to get the path to this widget
    regexp -indices {[^.]*$} $widget widget_ind
    set w_start [lindex [split $widget_ind] 0]
    set w_end [lindex [split $widget_ind] 1]
    
    set widget_name [string range $widget $w_start $w_end]
    set widget_path [string range $widget 0 [expr $w_start -2]]

    # create global variable for interaction (and get local alias for it)
    upvar #0 $widget_name var

    # store default values and ranges
    set var(b) 0
    
    set var(x) 0
    set var(xbase) -0.5
    set var(xrange) 1.0
    
    set var(y) 0
    set var(ybase) 0.5
    set var(yrange) -1.0

    global use_tk
    if $use_tk {
	if { [winfo exist $widget_path] } {
	    # create canvas to operate as the pad
	    canvas $widget -width 200 -height 200 -relief raised \
		-bg lightskyblue -borderwidth 3
	    
	    # pack adds some border, use the new "corrected" width and height
	    set w 210
	    set h 210
	    
	    # draw lines indicating dead-zone on pad
	    $widget create line 0 [expr .45*$h] $w [expr .45*$h]
	    $widget create line 0 [expr .55*$h] $w [expr .55*$h]
	    $widget create line [expr .45*$w] 0 [expr .45*$w] $h
	    $widget create line [expr .55*$w] 0 [expr .55*$w] $h
	    
	    # bind control events
	    # set a press to -1 so we can tell the difference in microscape
	    bind $widget <ButtonPress-1>   {joypad_update %W %x %y -1}
	    bind $widget <Button1-Motion>  {joypad_update %W %x %y 1}
	    bind $widget <ButtonRelease-1> {joypad_update %W %x %y 0}
	    
	    # instantiate widget
	    pack $widget
	}
    }
}

# update joypad position
# set position in pad_name(x) and pad_name(y), button state in pad_name(b)
# (where pad_name = name of the pad from the last element of the
#   widget path "pad")
# doesn't need use_tk tests since it is only be run from tk events
proc joypad_update {widget x y button_state} {
    # extract name from widget path
    regexp {[^.]*$} $widget widget_name

    # need to reference global interaction variable
    upvar #0 $widget_name var

    # set button state based on button_state parameter
    set var(b) $button_state

    # if button is pressed (-1) or moves (1), set value, otherwise recenter
    if {($button_state==-1) || ($button_state==1)} {
	# get x and y position scaled to 0-1 range
	set tx [expr double($x) / [winfo width $widget]]
	set ty [expr double($y) / [winfo height $widget]]

	# set x and y from scaled click location
	set var(x) [deadzone [expr $var(xbase) + $var(xrange)*$tx]]
	set var(y) [deadzone [expr $var(ybase) + $var(yrange)*$ty]]
    } else {
	set var(x) 0.0
	set var(y) 0.0
    }
}


########################################
# joyslider: a 1d control surface similar to joypad
#
# joyslider_create slider_widget_path
#
# Similar to joyslider, see comments for joyslider for details.  The
#   only difference is that joyslider only has one dimension (z).

# create a joyslider
# mouse operation in the joyslider causes changes to slider(z) and slider(b)
proc joyslider_create {widget} {
    # extract name from widget path
    # and hack off the widget_name to get the path to this widget
    regexp -indices {[^.]*$} $widget widget_ind
    set w_start [lindex [split $widget_ind] 0]
    set w_end [lindex [split $widget_ind] 1]
    
    set widget_name [string range $widget $w_start $w_end]
    set widget_path [string range $widget 0 [expr $w_start -2]]

    # create global variable for interaction (and get local alias for it)
    upvar #0 $widget_name var

    # These should be reset to default values every time the
    # widget is recreated, so don't check if they already exist. 

    # store default values and ranges
    set var(b) 0
    
    set var(z) 0
    set var(zbase) -0.5
    set var(zrange) 1.0

    global use_tk
    if $use_tk {
	if { [winfo exist $widget_path] } {
	    # create canvas to operate as the pad
	    canvas $widget -width 200 -height 40 -relief raised \
		-bg lightskyblue -borderwidth 3

	    # pack adds some border, use the new "corrected" width and height
	    set w 210
	    set h 50

	    # draw lines indicating dead-zone on pad
	    # includes slight overdraw at end to avoid short lines
	    $widget create line [expr .45*$w] 0 [expr .45*$w] $h
	    $widget create line [expr .55*$w] 0 [expr .55*$w] $h
	    
	    # -1 distinguishes a button press from a drag.
	    bind $widget <ButtonPress-1>   {joyslider_update %W %x -1}
	    bind $widget <Button1-Motion>  {joyslider_update %W %x 1}
	    bind $widget <ButtonRelease-1> {joyslider_update %W %x 0}
	    
	    # instantiate widget
	    pack $widget
	}
    }
}

# update pad position
# doesn't need use_tk tests since it is only be run from tk events
proc joyslider_update {widget x button_state} {
    # extract name from widget path
    regexp {[^.]*$} $widget widget_name

    # need to reference global interaction variable
    upvar #0 $widget_name var

    # set button state based on button_state parameter
    set var(b) $button_state

    # if button is pressed, set value, otherwise recenter
    if {($button_state==-1) || ($button_state==1)} {
	# get position scaled to 0-1 range
	set tx [expr double($x) / [winfo width $widget]]

	# set z from scaled click location
	set var(z) [deadzone [expr $var(zbase) + $var(zrange)*$tx]]
    } else {
	set var(z) 0.0
    }
}


# create a dead zone from -.05 to .05 for a variable
# e.g. for a val in [-.5,.5]:
#   -.5<=val<=-0.05 maps to [-.5,0]
#   -0.05<=val<=0.05 maps to 0
#   0.05<=val<=.5 maps to [0,.5]
proc deadzone {val} {
    if {$val < -0.05} {
	return [expr ($val + 0.05)/0.9]
    } elseif {$val > 0.05} {
	return [expr ($val - 0.05)/0.9]
    } else {
	return 0.0
    }
}


########################################
# joy3d: combo of joypad and joyslider
#
# joy3d_create widget_path

# create combined joypad
proc joy3d_create {widget} {
    # extract name from widget path
    # and hack off the widget_name to get the path to this widget
    regexp -indices {[^.]*$} $widget widget_ind
    set w_start [lindex [split $widget_ind] 0]
    set w_end [lindex [split $widget_ind] 1]
    
    set widget_name [string range $widget $w_start $w_end]
    set widget_path [string range $widget 0 [expr $w_start -2]]

    # create frames to contain both widgets
    global use_tk
    if $use_tk {
	if { [winfo exist $widget_path] } {
	    pack [frame $widget]
	    pack [frame $widget.slider] [frame $widget.pad] -side bottom
	}
    }

    joypad_create $widget.pad.$widget_name
    joyslider_create $widget.slider.$widget_name
}


########################################
# toggle: toggle button
#
# toggle_create widget_path toggle_text
#
# Create a toggle button the state of the button is stored in a global
#   variable with the same name as the last element of the widget path

# create a toggle button
proc toggle_create {widget toggle_text} {
    # extract name from widget path
    # and hack off the widget_name to get the path to this widget
    regexp -indices {[^.]*$} $widget widget_ind
    set w_start [lindex [split $widget_ind] 0]
    set w_end [lindex [split $widget_ind] 1]
    
    set widget_name [string range $widget $w_start $w_end]
    set widget_path [string range $widget 0 [expr $w_start -2]]

    # create global variable for interaction (and get local alias for it)
    upvar #0 $widget_name var
    if { ![info exist var] } {
	set var 0
    }

    # create button
    global use_tk
    if $use_tk {
	# check to see if the parent window exists before packing. 
	if { [winfo exist $widget_path] } {
	    pack [checkbutton $widget -text $toggle_text]
	}
    }
}


########################################
# slider: floating point slider
#
# slider_create widget_path  min val max
#
# Create a slider ranging from min to max with initial value val.
#   Value of the slider is stored in the global variable with the same
#   name as the last element of the widget path.

# create a slider
proc slider_create {widget  min val max} {
    # extract name from widget path
    # and hack off the widget_name to get the path to this widget
    regexp -indices {[^.]*$} $widget widget_ind
    set w_start [lindex [split $widget_ind] 0]
    set w_end [lindex [split $widget_ind] 1]
    
    set widget_name [string range $widget $w_start $w_end]
    set widget_path [string range $widget 0 [expr $w_start -2]]

    #puts "slider_create: $widget_path $widget_name"
    # create global variable for interaction (and get local alias for it)
    # could use global here, but I'll stick with upvar for consistancy
    upvar #0 $widget_name var
    # only set the var to default value if it doesn't already exist
    if { ![info exist var] } {
	set var $val
    }
    global use_tk
    if $use_tk {
	# check to see if the parent window exists before packing. 
	if { [winfo exist $widget_path] } {
	    # create slider
	    scale $widget -orient horizontal -from $min -to $max \
		-resolution -1 -label $widget_name -variable $widget_name
	    
	    # tell slider to expand if possible
	    pack $widget -expand 1 -fill x
	} 
    }
}


########################################
# colorpick: a color picker
#
# colorpick_create widget_path  r g b
#
# Creates three thin sliders and a sample color block
# Colors are in the range [0,1), the initial color is given by r,g,b
# 
# The last element of the widget path (after the last .) is taken to
#   be the color picker name (similar to the way pad or joypad works).  A
#   global array with this name is created (which should be unique), and all
#   color picker values are set in it as name(r), name(g) and name(b).

# create a color picker
proc colorpick_create {widget  r g b} {
    # extract name from widget path
    # and hack off the widget_name to get the path to this widget
    regexp -indices {[^.]*$} $widget widget_ind
    set w_start [lindex [split $widget_ind] 0]
    set w_end [lindex [split $widget_ind] 1]
    
    set widget_name [string range $widget $w_start $w_end]
    set widget_path [string range $widget 0 [expr $w_start -2]]

    # create global variable for interaction (and get local alias for it)
    global $widget_name
    upvar #0 $widget_name var

    # store default values and ranges
    if { ![info exist var] } {
	set var(r) $r
	set var(g) $g
	set var(b) $b
    }

    global use_tk
    if $use_tk {
	# check to see if the parent window exists before packing. 
	if { [winfo exist $widget_path] } {

	    # create frames
	    # +-----------------------------------+ <- $widget
	    # |+-----------------+                |
	    # ||$widget.sliders.r|                |
	    # ||$widget.sliders.g| $widget.swatch |
	    # ||$widget.sliders.b|		      |
	    # |+-----------------+		      |
	    # +-----------------------------------+
	    pack [frame $widget] -expand 1 -fill x
	    pack [frame $widget.sliders] -side left -expand 1 -fill x
	    
	    # create sliders
	    pack [scale $widget.sliders.r -var ${widget_name}(r) -bg red \
		      -orient h -width 5 -borderwidth 1 -showval 0 \
		      -from 0 -to 0.999 -resolution -1 \
		      -command "colorpick_update $widget.swatch $widget_name" \
		     ] -expand 1 -fill x 
	    pack [scale $widget.sliders.g -var ${widget_name}(g) -bg green \
		      -orient h -width 5 -borderwidth 1 -showval 0 \
		      -from 0 -to 0.999 -resolution -1 \
		      -command "colorpick_update $widget.swatch $widget_name" \
		     ] -expand 1 -fill x
	    pack [scale $widget.sliders.b -var ${widget_name}(b) -bg blue \
		      -orient h -width 5 -borderwidth 1 -showval 0 \
		      -from 0 -to 0.999 -resolution -1 \
		      -command "colorpick_update $widget.swatch $widget_name" \
		     ] -expand 1 -fill x
	    
	    # create color swatch
	    pack [canvas $widget.swatch -width 40 -height 40]
	    colorpick_update $widget.swatch ${widget_name} r
	}
    }
}    

# update color
# doesn't need use_tk tests since it is only be run from tk events
# called with the swatch widget name, and the color variable, and the new
# value for whichever component changed.  I ignore the new value part
proc colorpick_update {swatch colorvar newval} {
    # need access to global variable
    upvar #0 $colorvar var

    # compute hex colors
    set red [format "%02x" [expr int($var(r)*256)]]
    set grn [format "%02x" [expr int($var(g)*256)]]
    set blu [format "%02x" [expr int($var(b)*256)]]

    # set color
    $swatch configure -bg rgb:$red/$grn/$blu
}




########################################
# basicjoys: a basic joystick layout
#
# basicjoys_create widget_path left_label right_label
#
# creates a basic joystick layout in a frame under widget_path
#
# Contains two joysticks: one labeled with left_label, with results in
# joy0(x), joy0(y), and joy0(z), and one labeled with right_label with results
# in joy1(x), joy1(y) and joy1(x).
#
# basicjoys_fov_create widget_path left_label right_label
# Also contains a field of view slider with results in $fov, a dump
# toggle to dump the current frame,  and a quit button,
# which sets quit to true then destroys the main window
#
# layout
#+---------------------------+ <- $widget_path
#|+-------------------------+| <- $widget_path.pads
#||+----------+ +----------+|| <- $widget_path.pads.joy[01]
#|||   joy0   | |   joy1   |||
#|||          | |          |||
#||+----------+ +----------+||
#|+-------------------------+|
#|                           |
#|+-------------------------+| <- $widget_path.bottom
#|| [==fov-slider==] [quit] ||
#|+-------------------------+|
#+---------------------------+

# update field of view slider when real joybox slider moves
proc real_slider_update {widget var args} {
    # get global access to real slider's variable
    upvar #0 $var real_slider_value

    # scale this value to the slider's range
    set base [$widget cget -from]
    set range [expr [$widget cget -to] - $base]
    set val [expr $base + ($real_slider_value + .5) * double($range)]

    # set new value for the slider
    $widget set $val
}

proc basicjoys_create {widget left_label right_label} {
    global use_tk

    # extract name from widget path
    # and hack off the widget_name to get the path to this widget
    regexp -indices {[^.]*$} $widget widget_ind
    set w_start [lindex [split $widget_ind] 0]
    set w_end [lindex [split $widget_ind] 1]
    
    set widget_name [string range $widget $w_start $w_end]
    set widget_path [string range $widget 0 [expr $w_start -2]]

    # create frame hierarchy
    if $use_tk {
#	toplevel $widget
	if { [winfo exist $widget_path] } {
	    pack [frame $widget] -side bottom
	    pack [frame $widget.bottom] [frame $widget.pads] -side bottom
	}
    }

    # translation joypad
    joy3d_create $widget.pads.joy0
    # pack so they appear side by side
    if {($use_tk)&&([winfo exist $widget_path])} {
	pack $widget.pads.joy0 -side left
    }
    label_create $widget.pads.joy0.label $left_label

    # rotation joypad
    joy3d_create $widget.pads.joy1
    # pack so they appear side by side
    if {($use_tk)&&([winfo exist $widget_path])} {
	pack $widget.pads.joy1 -side right
    }
    label_create $widget.pads.joy1.label $right_label

}

proc basicjoys_fov_create {widget left_label right_label} {
    global use_tk

    # extract name from widget path
    # and hack off the widget_name to get the path to this widget
    regexp -indices {[^.]*$} $widget widget_ind
    set w_start [lindex [split $widget_ind] 0]
    set w_end [lindex [split $widget_ind] 1]
    
    set widget_name [string range $widget $w_start $w_end]
    set widget_path [string range $widget 0 [expr $w_start -2]]

    # create frame hierarchy
    if $use_tk {
#	toplevel $widget
	if { [winfo exist $widget_path] } {
	    pack [frame $widget] -side bottom
	    pack [frame $widget.bottom] [frame $widget.pads] -side bottom
	}
    }

    # translation joypad
    joy3d_create $widget.pads.joy0
    # pack so they appear side by side
    if {($use_tk)&&([winfo exist $widget_path])} {
	pack $widget.pads.joy0 -side left
    }
    label_create $widget.pads.joy0.label $left_label

    # rotation joypad
    joy3d_create $widget.pads.joy1
    # pack so they appear side by side
    if {($use_tk)&&([winfo exist $widget_path])} {
	pack $widget.pads.joy1 -side right
    }
    label_create $widget.pads.joy1.label $right_label


    # bottom row (fov and quit button)

    #  create field of view slider with limits [0,180] and value 90
    slider_create $widget.bottom.fov  0 90 180

    # update fov slider when real joybox slider changes.  Maps real joybox
    # slider range of [-.5,.5] to whatever the fov slider is set for
    global real_joyslider
    trace variable real_joyslider w "real_slider_update $widget.bottom.fov"

    if $use_tk {
	#  create frame dump and quit buttons
	pack [frame $widget.bottom.right] -side right
	pack [checkbutton $widget.bottom.right.framedump -text "Dump"] \
		-side top
	pack [button $widget.bottom.right.quit -text "Quit" \
		-command {set quit 1; destroy .}] -side bottom

	# expand fov slider to fill space under pads
	pack $widget.bottom.fov -side left
	pack $widget.bottom -fill x
    }
}

#creates a button that has a popup menu attached to it. Also
# create a global variable named after the widget. When the user
#selects an item from the menu, the global variable is set to the 
# index of that menu item, starting from zero.
#defval determines which menu item is initially selected.
proc popmenu_create {widget label_list defval} {

    # extract name from widget path
    # and hack off the widget_name to get the path to this widget
    regexp -indices {[^.]*$} $widget widget_ind
    set w_start [lindex [split $widget_ind] 0]
    set w_end [lindex [split $widget_ind] 1]
    
    set widget_name [string range $widget $w_start $w_end]
    set widget_path [string range $widget 0 [expr $w_start -2]]

    # create global variable for interaction (and get local alias for it)
    global $widget_name
    upvar #0 $widget_name var

    if { ![info exist var] } {
	set var $defval
    }

    # create frame hierarchy
    global use_tk
    if $use_tk {
	# check to see if the parent window exists before packing. 
	if { [winfo exist $widget_path] } {

	    pack [frame $widget] -expand 1 -fill x
	    label $widget.label -text $widget_name
	    #	puts [lindex $label_list 0]
	    
	    menubutton $widget.mb -textvariable $widget.item \
		-bd 2 -relief raised -menu $widget.mb.menu
	    menu $widget.mb.menu
	    set i 0
	    foreach item $label_list {
		$widget.mb.menu add command -label $item \
		    -command "set $widget_name $i; set $widget.item $item"
		incr i 1
	    }
	    #tcl numbers the menus from 1
	    $widget.mb.menu invoke [expr $var + 1]
	    pack $widget.label  -side left
	    pack $widget.mb -side right -expand 1 -fill x
	}
    }
}


########################################
# cmd_button: a command button
#
# cmd_button_create widget_path toggle_text command
#
# create a button that runs a command when it is pressed

proc cmd_button_create {widget cmd_button_text cmd} {
    # extract name from widget path
    # and hack off the widget_name to get the path to this widget
    regexp -indices {[^.]*$} $widget widget_ind
    set w_start [lindex [split $widget_ind] 0]
    set w_end [lindex [split $widget_ind] 1]
    
    set widget_name [string range $widget $w_start $w_end]
    set widget_path [string range $widget 0 [expr $w_start -2]]

    # create button
    global use_tk
    if $use_tk {
	# check to see if the parent window exists before packing. 
	if { [winfo exist $widget_path] } {
	    pack [button $widget -text $cmd_button_text -command "$cmd"]
	}
    }
}

# This file sets up the $latency frame, and must be sourced from a
# specific location inside mainwin.tcl
# 
# for widgets that change behavior of latency compensation techniques
# Tom Hudson, January 1999

set latency(la) [create_closing_toplevel latency "Latency Adaptations"]

# Belongs in a popup window off of the Tool menu - look at
# streamfile.tcl for example, and put into mainwin.tcl Tool menu.

# frame $latency(la) -relief raised -bd 3

# True Tip in Graphics

# Variables linked to C code
set truetip_showing 0
set truetip_scale 1.0

# Button to turn on and off
checkbutton $latency(la).truetip_showing -text "Show true tip location" -bg $fc \
        -variable truetip_showing

# Slider to control scale
floatscale $latency(la).truetip_scale 0.1 10.0 100 1.0 1 \
        truetip_scale "Scale true tip"

pack $latency(la).truetip_showing -side left
pack $latency(la).truetip_scale -side left



# Spring in Phantom

frame $latency(la).constraint

# Variables linked to C code
set constraint_mode 0
set constraint_kspring 10.0

# Buttons to select mode
radiobutton $latency(la).constraint.none_enabled -text \
         "No constraint" -bg $fc -variable constraint_mode -value 0
radiobutton $latency(la).constraint.point_enabled -text \
         "Point constraint" -bg $fc -variable constraint_mode -value 1
radiobutton $latency(la).constraint.line_enabled -text \
         "Line constraint" -bg $fc -variable constraint_mode -value 2

# Slider to control spring constant
floatscale $latency(la).constraint.kspring 1.0 100.0 100 10.0 1 \
        constraint_kspring "Spring Constant"

pack $latency(la).constraint.none_enabled $latency(la).constraint.point_enabled \
     $latency(la).constraint.line_enabled
pack $latency(la).constraint.kspring 

pack $latency(la).constraint -side bottom


generic_entry $latency(la).hand_tracker_rate handTracker_update_rate \
     "Phantom update rate (Hz)" real

pack $latency(la).hand_tracker_rate -side right




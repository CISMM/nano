# This file sets up the $latency frame, and must be sourced from a
# specific location inside mainwin.tcl
# 
# for widgets that change behavior of latency compensation techniques
# Tom Hudson, January 1999

frame $latency -relief raised -bd 3 -bg $bc

# True Tip in Graphics

# Variables linked to C code
set truetip_showing 0
set truetip_scale 1.0

# Button to turn on and off
checkbutton $latency.truetip_showing -text "Show true tip location" -bg $fc \
        -variable truetip_showing

# Slider to control scale
floatscale $latency.truetip_scale 0.1 10.0 100 1.0 1 \
        truetip_scale "Scale true tip"

pack $latency.truetip_showing -side left
pack $latency.truetip_scale -side left



# Spring in Phantom

frame $latency.constraint -bg $bc

# Variables linked to C code
set constraint_mode 0
set constraint_kspring 10.0

# Buttons to select mode
radiobutton $latency.constraint.none_enabled -text \
         "No constraint" -bg $fc -variable constraint_mode -value 0
radiobutton $latency.constraint.point_enabled -text \
         "Point constraint" -bg $fc -variable constraint_mode -value 1
radiobutton $latency.constraint.line_enabled -text \
         "Line constraint" -bg $fc -variable constraint_mode -value 2

# Slider to control spring constant
floatscale $latency.constraint.kspring 1.0 100.0 100 10.0 1 \
        constraint_kspring "Spring Constant"

pack $latency.constraint.none_enabled $latency.constraint.point_enabled \
     $latency.constraint.line_enabled
pack $latency.constraint.kspring 
pack $latency.constraint -side bottom




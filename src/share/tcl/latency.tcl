# This file sets up the $latency frame, and must be sourced from a
# specific location inside mainwin.tcl
# 
# for widgets that change behavior of latency compensation techniques
# Tom Hudson, January 1999
# November 2000:  added Forward Error Correction, Queue Monitoring

set latency(la) [create_closing_toplevel latency "Latency Adaptations"]

frame $latency(la).c1


#
# Graphics Adaptations
#

# Show true tip position in graphics (lagging behind "Tip" that shows
# where user's hand is on the Phantom)

frame $latency(la).c1.tt -relief raised -bd 3

# Variables linked to C code
set truetip_showing 0
set truetip_scale 1.0

# Button to turn on and off
checkbutton $latency(la).c1.tt.truetip_showing -text "Show true tip location" \
        -variable truetip_showing

# Slider to control scale
floatscale $latency(la).c1.tt.truetip_scale 0.1 10.0 100 1.0 1 \
        truetip_scale "Scale true tip"

pack $latency(la).c1.tt.truetip_showing -side left
pack $latency(la).c1.tt.truetip_scale -side left

pack $latency(la).c1.tt -side top

#
# Network Adaptations
#

# Variables linked to C code
set feel_use_redundant 0
set feel_num_redundant 0
set feel_redundant_interval 0.015
set feel_use_monitor 0
set feel_monitor_threshold 150
set feel_monitor_decay 2.0


# Forward Error Correction
# Reduces latency (& jitter) by using UDP instead of TCP.
# Setting number of transmissions > 0 sends extra copies to avoid
# loss in the network.

frame $latency(la).c1.ad -relief raised -bd 3

checkbutton $latency(la).c1.ad.active -text "Activate FEC" \
        -variable feel_use_redundant
generic_entry $latency(la).c1.ad.num feel_num_redundant \
        "Number of transmissions" integer
generic_entry $latency(la).c1.ad.interv feel_redundant_interval \
        "Interval (sec)" real

pack  $latency(la).c1.ad.active -side top
pack  $latency(la).c1.ad.num -side top
pack  $latency(la).c1.ad.interv -side top

pack $latency(la).c1.ad -side top

# Queue Monitoring
# Smooths out point result data (removes gaps by adding latency).

frame $latency(la).c1.qm -relief raised -bd 3

checkbutton $latency(la).c1.qm.active -text "Activate QM" \
        -variable feel_use_monitor
generic_entry $latency(la).c1.qm.thresh feel_monitor_threshold \
        "QM Threshold (2)" integer
generic_entry $latency(la).c1.qm.decay feel_monitor_decay \
        "QM Decay" real

pack  $latency(la).c1.qm.active -side top
pack  $latency(la).c1.qm.thresh -side top
pack  $latency(la).c1.qm.decay -side top

pack $latency(la).c1.qm -side top

pack $latency(la).c1 -side left






#
# Force Feedback Adaptations
#

# Phantom update rate

generic_entry $latency(la).hand_tracker_rate handTracker_update_rate \
     "Phantom update rate (Hz)" real

pack $latency(la).hand_tracker_rate -side top

# Spring in Phantom

frame $latency(la).constraint

# Variables linked to C code
set constraint_mode 0
set constraint_kspring 10.0

# Buttons to select mode
radiobutton $latency(la).constraint.none_enabled -text \
         "No constraint" -variable constraint_mode -value 0
radiobutton $latency(la).constraint.point_enabled -text \
         "Point constraint" -variable constraint_mode -value 1
radiobutton $latency(la).constraint.line_enabled -text \
         "Line constraint" -variable constraint_mode -value 2

# Slider to control spring constant
floatscale $latency(la).constraint.kspring 1.0 100.0 100 10.0 1 \
        constraint_kspring "Spring Constant"

pack $latency(la).constraint.none_enabled \
     $latency(la).constraint.point_enabled \
     $latency(la).constraint.line_enabled
pack $latency(la).constraint.kspring 

pack $latency(la).constraint -side top


#
# Diagnostics
#

frame $latency(la).timing

# When this is set, statistics are tracked;  when 0, they are *not* tracked
# (minimizing impact on performance).

set record_adaptation 0
checkbutton $latency(la).timing.record -text "Record timing statistics" \
        -variable record_adaptation

# When this is set, some statistics are printed to screen and others are
# saved to files with this filename.

set ms_timestamps_name ""
generic_entry $latency(la).timing.name ms_timestamps_name "Timing filename" ""

pack $latency(la).timing.record -side top
pack $latency(la).timing.name -side top
pack $latency(la).timing -side bottom



# This doesn't work.  I'm leaving it here-but-commented-out so it gets
# committed once.
# In general, we can't currently change between logging and non-logging
# stuff mid-run (without a lot more debugging or rework than I'm willing
# to put it).

#
# Recording/replaying Phantom
# No, this isn't latency compensation, but it's used to test latency comp
# in Tom Hudson's dissertation, so it's here.  Feel free to remove it
# in 2002.
#
# Nonorthogonal interface.  Whee.
# Recording:  checkbutton:  check to begin recording.  uncheck to stop.
# Replay:  button:  press to replay once.  checkbutton:  check to begin
#   infinite-loop replay.  uncheck to stop.

# frame $latency(la).phantom
# 
# frame $latency(la).phantom.record
# 
# generic_entry $latency(la).phantom.record.name phantom_record_name \
#   "PHANTOM recording file" ""
# checkbutton $latency(la).phantom.record.now -variable phantom_record_now \
#   -text "GO" 
# 
# pack $latency(la).phantom.record.name \
#      $latency(la).phantom.record.now -side left
# pack $latency(la).phantom.record -side bottom
# 
# frame $latency(la).phantom.replay
# 
# generic_entry $latency(la).phantom.replay.name phantom_replay_name \
#   "PHANTOM replay file" ""
# checkbutton $latency(la).phantom.replay.now -variable phantom_replay_now \
#   -text "ONCE" 
# checkbutton $latency(la).phantom.replay.many -variable phantom_replay_many \
#   -text "REPEAT" 
# 
# pack $latency(la).phantom.replay.name \
#      $latency(la).phantom.replay.now \
#      $latency(la).phantom.replay.many -side left
# pack $latency(la).phantom.replay -side bottom
# 
# pack $latency(la).phantom
# 


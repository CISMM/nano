#/*===3rdtech===
#  Copyright (c) 2000 by 3rdTech, Inc.
#  All Rights Reserved.
#
#  This file may not be distributed without the permission of 
#  3rdTech, Inc. 
#  ===3rdtech===*/
# Controls for replaying a stream file
# Includes changing replay speed,
# pausing replay, and jumping to a new time.


set streamplay(sf) [create_closing_toplevel streamfile "Replay Controls"]
# We can show the window with show.streamfile procedure.


#-------------------------------------------------------------------
label $streamplay(sf).label -text "Stream Replay Controls"
pack $streamplay(sf).label -side top -anchor nw -padx 5 -pady 5

set streamplay(buttons) [frame $streamplay(sf).buttons]
pack $streamplay(buttons) -side top -fill x -pady 5

button $streamplay(buttons).rewind_stream -text "Rewind" \
    -command {set rewind_stream 1} -padx 8

button $streamplay(buttons).play_button -text "Play" \
    -command {streamplay_play_command} -padx 8

button $streamplay(buttons).pause_button -text "Pause" \
    -command {streamplay_pause_command} -padx 8

pack $streamplay(buttons).rewind_stream \
	$streamplay(buttons).play_button \
	$streamplay(buttons).pause_button -side left 


if {[info exist stream_replay_rate]} {
    set new_replay_rate $stream_replay_rate
} else {
    set stream_replay_rate 1
    set new_replay_rate 1
}
# This widget doesn't actually set the replay rate! See the 
# trace procedure below: streamplay_replay_rate_update
generic_entry $streamplay(sf).replay_rate new_replay_rate \
	"Speed (1,30)" numeric

# Jump to a new time in the stream file with this widget.
set set_stream_time 0
set set_stream_time_now 0
generic_entry $streamplay(sf).set_stream_time set_stream_time \
    "Go to time:" numeric

# Handle an idempotent change in the stream time. If the user sets
# the stream time to the same value, we want the stream to go back to 
# that time. In a more normal widget we would ignore the change. 
proc handle_set_stream_time_change { name el op } {
global set_stream_time_now
#    puts "TCL: stream time"
    set set_stream_time_now 1
}
trace variable set_stream_time w handle_set_stream_time_change

# Make it pretty.
iwidgets::Labeledwidget::alignlabels \
	$streamplay(sf).set_stream_time \
	$streamplay(sf).replay_rate \

pack $streamplay(sf).set_stream_time \
	$streamplay(sf).replay_rate \
	-side top -anchor nw -padx 5 -pady 5


# One button must be disabled to begin with....
if {$new_replay_rate > 0 } {
    $streamplay(buttons).play_button configure -state disabled
} else {
    $streamplay(buttons).pause_button configure -state disabled
}

#--------- implement pause/resume button ------------

# If the user hits "pause":
#   disable pause button
#   enable play button
#   set the actual stream replay rate to zero
proc streamplay_pause_command {} {
    global streamplay
    global stream_replay_rate

    set stream_replay_rate 0
    $streamplay(buttons).play_button configure \
	    -state normal
    $streamplay(buttons).pause_button configure \
	    -state disabled

}

# If the user hits "play":
#   enable pause button
#   disable play button
#   set the actual stream replay rate to the value of the replay_rate widget
proc streamplay_play_command {} {
    global streamplay
    global new_replay_rate
    global stream_replay_rate

    set stream_replay_rate $new_replay_rate
    $streamplay(buttons).play_button configure \
	    -state disabled
    $streamplay(buttons).pause_button configure \
	    -state normal
}
set ignore_rate_change 0

# If the replay rate widget changes, either 
#   change real replay rate if the stream is currently playing
#   Do nothing if the stream is paused. 
proc streamplay_replay_rate_update {name element op} {
    global streamplay
    global new_replay_rate stream_replay_rate
    global ignore_rate_change

    # make sure the displayed play rate is always positive
    # user must stop playback with the "pause" button.
    if { $new_replay_rate <= 0 } { 
	set new_replay_rate 1
    }
    set pstate [$streamplay(buttons).play_button cget -state]
    if { "$pstate" == "disabled" } {
	# This variable avoids recursive callbacks. 
	set ignore_rate_change 1
	set stream_replay_rate $new_replay_rate
	set ignore_rate_change 0
    }
}

# set a trace on the replay rate widget variable. 
trace variable new_replay_rate w streamplay_replay_rate_update

# The replay_rate widget doesn't display the correct replay
# rate unless we trace changes made in the C code...
# Fix which button is disabled, as well. 
proc streamplay_update_replay_widget {name element op} {
    global streamplay
    global new_replay_rate stream_replay_rate
    global ignore_rate_change

    # avoid a recursive callback from streamplay_replay_rate_update
    if { $ignore_rate_change } return;

    if { $stream_replay_rate > 0 } {
	set new_replay_rate $stream_replay_rate
	$streamplay(buttons).play_button configure \
		-state disabled
	$streamplay(buttons).pause_button configure \
		-state normal
    } else {
	# leave the new_replay_rate the way it was.
	# Fix the buttons
	$streamplay(buttons).play_button configure \
		-state normal
	$streamplay(buttons).pause_button configure \
		-state disabled
    }	
}

trace variable stream_replay_rate w streamplay_update_replay_widget
#------- end pause/resume button ----------

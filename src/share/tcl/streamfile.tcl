set streammod(sf) .streamfile

#-------------------------------------------------------------------
toplevel $streammod(sf)
wm withdraw $streammod(sf)

button $streammod(sf).close -text "Close" -command {
    wm withdraw $streammod(sf)
}

wm protocol $streammod(sf) WM_DELETE_WINDOW {$streammod(sf).close invoke}
pack $streammod(sf).close -anchor nw

proc show_streamfile_win {} {
    global streammod 
    wm deiconify $streammod(sf)
    raise $streammod(sf)
}

button $streammod(sf).rewind_stream -text "Rewind Stream" -bg tan \
    -command {set rewind_stream 1}

pack $streammod(sf).rewind_stream -side top

#iwidgets::entryfield $streammod(sf).set_stream_time -labeltext "Go to time:" \
#    -validate numeric \
#    -command {set set_stream_time [$streammod(sf).set_stream_time get]} 

# use generic_entry, originally from keithley2400.tcl

set set_stream_time 0

generic_entry $streammod(sf).set_stream_time set_stream_time \
    "Go to time:" numeric

pack $streammod(sf).set_stream_time -side top

#--------- implement pause/resume button ------------

## I can't seem to get access to $streammod(sf).<variable> from in a command
## for the buttons, so I'll make a global variable instead
set streamreplay_old_rate "not_set"

proc $streammod(sf).pause_command {} {
    global streammod
    global replay_rate

    ## doesn't work
    #global $streammod(sf).old_rate
    #set $streammod(sf).old_rate $replay_rate
    ## use this instead
    global streamreplay_old_rate
    set streamreplay_old_rate $replay_rate

    set replay_rate 0
    $streammod(sf).pause_button configure \
            -text "Resume (at speed $streamreplay_old_rate)" \
            -command {$streammod(sf).unpause_command $streamreplay_old_rate}

    # set a trace on the slider
    trace variable replay_rate w $streammod(sf).fix_pause_button
}

proc $streammod(sf).unpause_command {rate} {
    global streammod
    global replay_rate
    trace vdelete replay_rate w $streammod(sf).fix_pause_button
    set replay_rate $rate
    $streammod(sf).pause_button configure -text "Pause Stream" \
            -command {$streammod(sf).pause_command}
    global streamreplay_old_rate
    set streamreplay_old_rate "not_set"
}

proc $streammod(sf).fix_pause_button {name element op} {
    global streammod
    global replay_rate
    $streammod(sf).pause_button configure -text "Pause Stream" \
            -command {$streammod(sf).pause_command}
    trace vdelete replay_rate w $streammod(sf).fix_pause_button
    global streamreplay_old_rate
    set streamreplay_old_rate "not_set"
}

button $streammod(sf).pause_button -text "Pause Stream" -bg tan \
    -command {$streammod(sf).pause_command}

pack $streammod(sf).pause_button -side top

#------- end pause/resume button ----------

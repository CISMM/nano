##### This file contains the interface to the Education streamfile data
##### Three streamfiles show the adenovirus and tobacco mosaic virus
##### in three different experiments: cutting, pushing, and poking the viruses
##### The interface allows the user to select one of the three experiments
##### Once an experiment is selected the user can watch different outcomes
##### For example the 'cut' streamfile shows both the adenovirus and tmv
##### being cut by the afm tip. To watch the results two buttons (one specific
##### to the type of experiment) and a 'repeat' button are used. This code
##### in this file jumps to the corresponding locations in the streamfile.
##### The repeat button lets the user see the last modification any number of
##### times, but does not let the user repeat earlier modifications.

#### This is still a work in progress -- more streamfiles and experiments on
#### viruses will be added in the future.


set z_set 0

### START Cut specific variables:
set cut 0

set number_of_cuts 7
set cuts_done 0
set virus_cut_win [create_closing_toplevel virus_cut "Cut Virus"]

set cut_filename "tmv-adeno-cut-4.nms"

set cut_start_times(0) 730 
set cut_start_times(1) 940
set cut_start_times(2) 1170 
set cut_start_times(3) 1390
set cut_start_times(4) 1490
set cut_start_times(5) 1660
set cut_start_times(6) 1830

set cut_end_times(0) 938
set cut_end_times(1) 1168
set cut_end_times(2) 1385
set cut_end_times(3) 1488
set cut_end_times(4) 1658
set cut_end_times(5) 1828
set cut_end_times(6) 2040
### END Cut specific variables:

### START Poke specific variables:
set poke 0

set number_of_pokes 10
set pokes_done 0
set virus_poke_win [create_closing_toplevel virus_poke "Poke Virus"]

set poke_filename "virus_graphite_poke1.nms"

set poke_start_times(0) 1539
set poke_start_times(1) 1965
set poke_start_times(2) 2185 
set poke_start_times(3) 2690
set poke_start_times(4) 2853
set poke_start_times(5) 3060
set poke_start_times(6) 3295
set poke_start_times(7) 3700
set poke_start_times(8) 3880
set poke_start_times(9) 4185

set poke_end_times(0) 1763
set poke_end_times(1) 2180
set poke_end_times(2) 2399
set poke_end_times(3) 2850
set poke_end_times(4) 3058
set poke_end_times(5) 3290
set poke_end_times(6) 3500
set poke_end_times(7) 3878
set poke_end_times(8) 4160
set poke_end_times(9) 4530
### END Poke specific variables:

### START Push specific variables:
set push 0

set number_of_pushes 10
set pushes_done 0
set virus_push_win [create_closing_toplevel virus_push "Push Virus"]

set push_filename "virus_graphite_push4.nms"

set push_start_times(0) 610
set push_start_times(1) 970
set push_start_times(2) 1370 
set push_start_times(3) 1640
set push_start_times(4) 1870
set push_start_times(5) 2180
set push_start_times(6) 2530
set push_start_times(7) 2700
set push_start_times(8) 2900
set push_start_times(9) 3125
set push_start_times(10) 3510
set push_start_times(11) 3685

set push_end_times(0) 968
set push_end_times(1) 1368
set push_end_times(2) 1625
set push_end_times(3) 1860
set push_end_times(4) 2170
set push_end_times(5) 2435
set push_end_times(6) 2690
set push_end_times(7) 2894
set push_end_times(8) 3120
set push_end_times(9) 3495
set push_end_times(10) 3683
set push_end_times(11) 3930
### END Push specific variables:

proc virus_play_stream { num } {
    global set_stream_time set_stream_time_now stream_replay_rate 
    global cuts_done pokes_done pushes_done
    global number_of_cuts number_of_pokes number_of_pushes
    global cut_start_times poke_start_times push_start_times
    global virus_cut_win virus_poke_win virus_push_win
    global cut poke push
    global clear_markers
    global center_pressed
    global set_stream_clip_time

    trace vdelete set_stream_clip_time w update_time

    if { $cut == 1 } {
        set cuts_done $num
        if { $num < $number_of_cuts } {
            $virus_cut_win.f4.repeat configure -state disabled
            $virus_cut_win.f4.cut configure -state normal
            
            set set_stream_time $cut_start_times($num)
            set set_stream_clip_time $cut_start_times($num)

            set set_stream_time_now 1
            set stream_replay_rate 10
            set clear_markers 1
            set center_pressed 1
            {streamplay_play_command}
        } else {
            $virus_cut_win.f4.cut configure -state disabled
        }
    } elseif { $poke == 1 } {
        set pokes_done $num
        if { $num < $number_of_pokes } {
            $virus_poke_win.f4.repeat configure -state disabled
            $virus_poke_win.f4.poke configure -state normal
            
            set set_stream_time $poke_start_times($num)
            set set_stream_clip_time $poke_start_times($num)

            set set_stream_time_now 1
            set stream_replay_rate 10
            set clear_markers 1
            set center_pressed 1
            {streamplay_play_command}
        } else {
            $virus_poke_win.f4.poke configure -state disabled
        }
    } elseif { $push == 1 } {
        set pushes_done $num
        if { $num < $number_of_pushes } {
            $virus_push_win.f4.repeat configure -state disabled
            $virus_push_win.f4.push configure -state normal
            
            set set_stream_time $push_start_times($num)
            set set_stream_clip_time $push_start_times($num)

            set set_stream_time_now 1
            set stream_replay_rate 10
            set clear_markers 1
            set center_pressed 1
            {streamplay_play_command}
        } else {
            $virus_push_win.f4.push configure -state disabled
        }
    }

    trace variable set_stream_clip_time w update_time
}

proc virus_repeat_stream { num } {
    global set_stream_time set_stream_time_now stream_replay_rate 
    global cuts_done pokes_done pushes_done
    global number_of_cuts number_of_pokes number_of_pushes
    global cut_start_times poke_start_times push_start_times
    global virus_cut_win virus_poke_win virus_push_win
    global cut poke push
    global clear_markers
    global center_pressed
    global set_stream_clip_time

    trace vdelete set_stream_clip_time w update_time
    
    if { $cut == 1 } {
        set cuts_done $num
        if { $num < $number_of_cuts } {
            $virus_cut_win.f4.repeat configure -state disabled
            $virus_cut_win.f4.cut configure -state normal
            
            set set_stream_time $cut_start_times($num)
            set set_stream_clip_time $cut_start_times($num)
            set set_stream_time_now 1
            set stream_replay_rate 10
            set clear_markers 1
            set center_pressed 1
            {streamplay_pause_command}
        } else {
            $virus_cut_win.f4.cut configure -state disabled
        }
    } elseif { $poke == 1 } {
        set pokes_done $num
        if { $num < $number_of_pokes } {
            $virus_poke_win.f4.repeat configure -state disabled
            $virus_poke_win.f4.poke configure -state normal
            
            set set_stream_time $poke_start_times($num)
            set set_stream_clip_time $poke_start_times($num)
            set set_stream_time_now 1
            set stream_replay_rate 10
            set clear_markers 1
            set center_pressed 1
            {streamplay_pause_command}
        } else {
            $virus_poke_win.f4.poke configure -state disabled
        }
    } elseif { $push == 1 } {
        set pushes_done $num
        if { $num < $number_of_pushes } {
            $virus_push_win.f4.repeat configure -state disabled
            $virus_push_win.f4.push configure -state normal
            
            set set_stream_time $push_start_times($num)
            set set_stream_clip_time $push_start_times($num)
            set set_stream_time_now 1
            set stream_replay_rate 10
            set clear_markers 1
            set center_pressed 1
            {streamplay_pause_command}
        } else {
            $virus_push_win.f4.push configure -state disabled
        }
    }

    trace variable set_stream_clip_time w update_time
}

proc cut_virus { } {
    global cut_filename virus_dir
    global open_stream_filename fileinfo
    global set_stream_time set_stream_time_now
    global center_pressed
    global clear_markers
    global virus_cut_win virus_poke_win virus_push_win
    global cut poke push
    global z_comes_from

    set cut 1
    set poke 0
    set push 0

    set open_stream_filename [file join $virus_dir $cut_filename]
    #set fileinfo(open_dir) [file dirname $cut_filename]

    {streamplay_pause_command}

    set set_stream_time 5
    set set_stream_time_now 1

    set z_comes_from "Z Piezo-Forward"

    after 100 {
      set set_stream_time 730
      set set_stream_time_now 1
      set z_comes_from "Z Piezo-Forward"
      set center_pressed 1
      set clear_markers 1
    }
    
    frame $virus_cut_win.f1
    frame $virus_cut_win.f2
    frame $virus_cut_win.f3
    frame $virus_cut_win.f4
    pack  $virus_cut_win.f1  $virus_cut_win.f2 $virus_cut_win.f3 $virus_cut_win.f4 -side top

    label $virus_cut_win.f1.label1a -font {helvetica -28 } -foreground blue -text "What can you learn by "
    label $virus_cut_win.f1.label1b -font {helvetica -28 italic } -foreground orange -text "cutting"
    label $virus_cut_win.f1.label1c -font {helvetica -28 } -foreground blue -text " a virus?"
    pack $virus_cut_win.f1.label1a $virus_cut_win.f1.label1b $virus_cut_win.f1.label1c -side left

    label $virus_cut_win.f2.label2 -font {helvetica -22 } -foreground red -text "Is the virus hard or soft?"
    pack $virus_cut_win.f2.label2 -side top

    label $virus_cut_win.f2.label3 -font {helvetica -22 } -foreground DarkGreen -text "Is the virus sticky?"
   pack $virus_cut_win.f2.label3 -side top

    label $virus_cut_win.f2.label4 -font {helvetica -22 } -foreground orange -text "Is the virus rough or smooth?"
    pack $virus_cut_win.f2.label4 -side top

    label $virus_cut_win.f3.label5 -font {helvetica -22 } -foreground purple -text "Does the virus dent like clay?"
    label $virus_cut_win.f3.label6 -font {helvetica -22 } -foreground purple -text "Or pop like a bubble?"
    pack $virus_cut_win.f3.label5 $virus_cut_win.f3.label6 -side left 

    label $virus_cut_win.f4.label7 -font {helvetica -28 } -foreground blue -text "Press here to find out: "

    button $virus_cut_win.f4.cut -text "Cut" \
            -command {virus_play_stream $cuts_done}
    button $virus_cut_win.f4.repeat -text "Repeat" \
            -command {virus_repeat_stream [expr $cuts_done - 1]}

    $virus_cut_win.f4.repeat configure -state disabled

    pack $virus_cut_win.f4.label7 $virus_cut_win.f4.cut $virus_cut_win.f4.repeat  -side left
    show.virus_cut
    hide.virus_push
    hide.virus_poke
}

proc poke_virus { } {
    global poke_filename virus_dir
    global open_stream_filename fileinfo
    global set_stream_time set_stream_time_now
    global center_pressed
    global clear_markers
    global virus_cut_win virus_poke_win virus_push_win
    global cut poke push
    global z_comes_from

    set cut 0
    set poke 1
    set push 0

    set open_stream_filename [file join $virus_dir $poke_filename]
    #set fileinfo(open_dir) [file dirname $poke_filename]

    {streamplay_pause_command}

    set set_stream_time 10
    set set_stream_time_now 1

    set z_comes_from "Z Piezo-Forward"

    after 100 {
       set set_stream_time 1530
       set set_stream_time_now 1
       set z_comes_from "Z Piezo-Forward"
       set center_pressed 1
       set clear_markers 1
    }

    frame $virus_poke_win.f1
    frame $virus_poke_win.f2
    frame $virus_poke_win.f3
    frame $virus_poke_win.f4
    pack  $virus_poke_win.f1  $virus_poke_win.f2 $virus_poke_win.f3 $virus_poke_win.f4 -side top

    label $virus_poke_win.f1.label1a -font {helvetica -28 } -foreground DarkGreen -text "What can you learn by "
    label $virus_poke_win.f1.label1b -font {helvetica -28 italic } -foreground red -text "poking"
    label $virus_poke_win.f1.label1c -font {helvetica -28 } -foreground DarkGreen -text " a virus?"
    pack $virus_poke_win.f1.label1a $virus_poke_win.f1.label1b $virus_poke_win.f1.label1c -side left

    label $virus_poke_win.f2.label2 -font {helvetica -22 } -foreground orange -text "Is the virus hard or soft?"
    pack $virus_poke_win.f2.label2 -side top

    label $virus_poke_win.f2.label3 -font {helvetica -22 } -foreground blue -text "Is the virus sticky?"
   pack $virus_poke_win.f2.label3 -side top

    label $virus_poke_win.f2.label4 -font {helvetica -22 } -foreground purple -text "Is the virus rough or smooth?"
    pack $virus_poke_win.f2.label4 -side top

    label $virus_poke_win.f3.label5 -font {helvetica -22 } -foreground red -text "Does the virus dent like clay?"
    label $virus_poke_win.f3.label6 -font {helvetica -22 } -foreground red -text "Or pop like a bubble?"
    pack $virus_poke_win.f3.label5 $virus_poke_win.f3.label6 -side left 

    label $virus_poke_win.f4.label7 -font {helvetica -28 } -foreground DarkGreen -text "Press here to find out: "

    button $virus_poke_win.f4.poke -text "poke" \
            -command {virus_play_stream $pokes_done}
    button $virus_poke_win.f4.repeat -text "Repeat" \
            -command {virus_repeat_stream [expr $pokes_done - 1]}

    $virus_poke_win.f4.repeat configure -state disabled

    pack $virus_poke_win.f4.label7 $virus_poke_win.f4.poke $virus_poke_win.f4.repeat  -side left

    show.virus_poke
    hide.virus_cut
    hide.virus_push
}

proc push_virus { } {
    global push_filename virus_dir
    global open_stream_filename fileinfo
    global set_stream_time set_stream_time_now
    global center_pressed
    global clear_markers
    global virus_cut_win virus_poke_win virus_push_win
    global cut poke push
    global z_comes_from

    set cut 0
    set poke 0
    set push 1

    set open_stream_filename [file join $virus_dir $push_filename]
    #set fileinfo(open_dir) [file dirname $push_filename]

    {streamplay_pause_command}

    set set_stream_time 10
    set set_stream_time_now 1

    after 100 {
      set set_stream_time 610
      set set_stream_time_now 1
      set z_comes_from "Z Piezo-Forward"
      set center_pressed 1
      set clear_markers 1
    }

    frame $virus_push_win.f1
    frame $virus_push_win.f2
    frame $virus_push_win.f3
    frame $virus_push_win.f4
    pack  $virus_push_win.f1  $virus_push_win.f2 $virus_push_win.f3 $virus_push_win.f4 -side top

    label $virus_push_win.f1.label1a -font {helvetica -28 } -foreground purple -text "What can you learn by "
    label $virus_push_win.f1.label1b -font {helvetica -28 italic } -foreground blue -text "pushing"
    label $virus_push_win.f1.label1c -font {helvetica -28 } -foreground purple -text " a virus?"
    pack $virus_push_win.f1.label1a $virus_push_win.f1.label1b $virus_push_win.f1.label1c -side left

    label $virus_push_win.f2.label2 -font {helvetica -22 } -foreground red -text "Is the virus hard or soft?"
    pack $virus_push_win.f2.label2 -side top

    label $virus_push_win.f2.label3 -font {helvetica -22 } -foreground orange -text "Is the virus sticky?"
   pack $virus_push_win.f2.label3 -side top

    label $virus_push_win.f2.label4 -font {helvetica -22 } -foreground DarkGreen -text "Is the virus rough or smooth?"
    pack $virus_push_win.f2.label4 -side top

    label $virus_push_win.f3.label5 -font {helvetica -22 } -foreground blue -text "Does the virus dent like clay?"
    label $virus_push_win.f3.label6 -font {helvetica -22 } -foreground blue -text "Or pop like a bubble?"
    pack $virus_push_win.f3.label5 $virus_push_win.f3.label6 -side left 

    label $virus_push_win.f4.label7 -font {helvetica -28 } -foreground purple -text "Press here to find out: "

    button $virus_push_win.f4.push -text "push" \
            -command {virus_play_stream $pushes_done}
    button $virus_push_win.f4.repeat -text "Repeat" \
            -command {virus_repeat_stream [expr $pushes_done - 1]}

    $virus_push_win.f4.repeat configure -state disabled

    pack $virus_push_win.f4.label7 $virus_push_win.f4.push $virus_push_win.f4.repeat  -side left
    show.virus_push
    hide.virus_cut
    hide.virus_poke
}

proc update_time { nm el op } {
    global set_stream_clip_time cuts_done pokes_done pushes_done
    global virus_cut_win virus_poke_win virus_push_win
    global cut_end_times push_end_times poke_end_times
    global cut poke push

    if { ($cut == 1) && ($set_stream_clip_time >= $cut_end_times($cuts_done)) } {
        set cuts_done [expr $cuts_done +1]
        $virus_cut_win.f4.repeat configure -state normal
        {streamplay_pause_command}
    } elseif { ($poke == 1) && ($set_stream_clip_time >= $poke_end_times($pokes_done)) } {
        set pokes_done [expr $pokes_done +1]
        $virus_poke_win.f4.repeat configure -state normal
        {streamplay_pause_command}
    } elseif { ($push == 1) && ($set_stream_clip_time >= $push_end_times($pushes_done)) } {
        set pushes_done [expr $pushes_done +1]
        puts "$pushes_done"
        puts "$set_stream_clip_time"
        $virus_push_win.f4.repeat configure -state normal
        {streamplay_pause_command}
    }
}


trace variable set_stream_clip_time w update_time



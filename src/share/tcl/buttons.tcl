# button box simulator

# set some characteristics for buttons in the button box. by saying
# ".*background" instead of ".background", all the widgets inside a
# frame of class Buttonbox inherit this background color, too.
option add *Buttonbox.*background tan startupFile
option add *Buttonbox.*highlightBackground tan startupFile
option add *Buttonbox.*anchor w startupFile

frame $buttons -relief raised -bd 5 -bg grey50
set butf [frame $buttons.bframe -bg grey50 -class Buttonbox]

#determines the height of the buttons.
set ipadamt 2m

set pad_x 1m

#setup buttons
label $buttons.label -text "SGI Button Box" -bg grey50
 pack $buttons.label -side top
 pack $buttons.bframe -side left -fill x 

#row0 buttons
checkbutton $butf.trig -text "Trig" -variable trigger_pressed 
button $butf.menu -text "Menu" -command not_implemented  
checkbutton $butf.mod -text "Modify" -variable modify_pressed 
button $butf.all -text "All" -command "set term_input A" 

grid x $butf.trig $butf.menu $butf.mod $butf.all \
    -ipady $ipadamt -padx $pad_x -pady 2m -sticky nsew


#row 1 buttons
radiobutton $butf.grab -text "Grab" -variable user_0_mode \
        -value 1  
radiobutton $butf.s_up -text "Up" -variable user_0_mode \
        -value 2 
radiobutton $butf.s_dn -text "Down" -variable user_0_mode \
        -value 3 
radiobutton $butf.select -text "Select" -variable user_0_mode \
        -value 4 
radiobutton $butf.light -text "Light" -variable user_0_mode \
        -value 10 
radiobutton $butf.measure -text "Meas" -variable user_0_mode \
        -value 9 

grid $butf.grab $butf.s_up $butf.s_dn $butf.select \
    $butf.light $butf.measure \
    -ipady $ipadamt -padx $pad_x -pady 2m -sticky nsew

#row 2 buttons
radiobutton $butf.normal -text "Blunt" -variable user_0_mode \
        -value 13  
radiobutton $butf.plane -text "Sharp" -variable user_0_mode \
        -value 12 
radiobutton $butf.sweep -text "Sweep"  -variable user_0_mode \
        -value 8 
radiobutton $butf.comb -text "Comb" -variable user_0_mode \
        -value 14 
checkbutton $butf.swlk -text "Sw lck" -variable sweep_lock_pressed 
radiobutton $butf.canned -text "Canned" -variable user_0_mode \
        -value 11 

grid $butf.normal $butf.plane $butf.sweep $butf.comb \
    $butf.swlk  $butf.canned \
    -ipady $ipadamt -padx $pad_x -pady 2m -sticky nsew


# row 3 buttons
button $butf.center -text "Center" -command "set term_input \"\^\" " 
button $butf.pulseclr -text "Pls Clr" -command "set term_input C"
button $butf.on -text "Pnl on" -command not_implemented 
button $butf.off -text "Pl off" -command not_implemented 
checkbutton $butf.xylk -text "Xy lck" -variable xy_lock_pressed 
button $butf.phantom_reset -text "Rst PHANT" -command "set reset_phantom 1"

grid $butf.center $butf.pulseclr $butf.on $butf.off \
    $butf.xylk  $butf.phantom_reset \
    -ipady $ipadamt -padx $pad_x -pady 2m -sticky nsew

# row 4 & 5 buttons
checkbutton $butf.commit -text "Commit" -variable commit_pressed 
button $butf.cancel -text "Cancel" -command "set cancel_commit 1" 

radiobutton $butf.scanline -text "Scanline" -variable user_0_mode \
	-value 16
button $butf.null1 -text "Unused"
button $butf.null2 -text "Unused"
button $butf.null3 -text "Unused"
button $butf.null4 -text "Unused"
button $butf.null5 -text "Unused"
button $butf.null6 -text "Unused"
button $butf.null7 -text "Unused"

grid $butf.commit $butf.cancel $butf.scanline $butf.null1  $butf.null2 \
    $butf.null3 \
    -ipady $ipadamt -padx $pad_x -pady 2m -sticky nsew

grid x  $butf.null4 $butf.null5 $butf.null6 $butf.null7 \
    -ipady $ipadamt -padx $pad_x -pady 2m -sticky nsew

# This file sets up the $view frame, and must be sourced from a
# specific location inside tools.tcl
#
# for widgets that change the surface window
#

# 5 columns of buttons, 2 in each column
frame $view -relief raised -bd 2
frame $view.c1 
frame $view.c2 
frame $view.c3
frame $view.c4 
frame $view.c5 

pack $view.c1 $view.c2 $view.c3 $view.c4 $view.c5 -side left -fill both
# Temp for collab experiment
#pack $view.c1 $view.c2 $view.c3 -side left -fill both

# View variable initialization
set commit_pressed 0
set commit_cancel 0
set center_pressed 0

set pad_x 2
#
#setup surface view buttons
#

radiobutton $view.c1.grab        -text "Grab"  -variable user_0_mode \
	-value 1 
button $view.c1.center           -text "Center" -command "set center_pressed 1"
pack $view.c1.grab $view.c1.center -side top -padx $pad_x -pady 2 -anchor w

radiobutton $view.c2.changelight -text "Position Light" -variable user_0_mode \
	-value 10 
radiobutton $view.c2.measure -text "Measure" \
    -variable user_0_mode -value 9 

pack $view.c2.changelight $view.c2.measure \
	-side top -padx $pad_x -pady 2 -anchor w

radiobutton $view.c3.scaleup     -text "Scale Up"   -variable user_0_mode \
	-value 2 
radiobutton $view.c3.scaledown   -text "Scale Down" -variable user_0_mode \
	-value 3 
pack $view.c3.scaleup $view.c3.scaledown -side top -padx $pad_x -pady 2 -anchor w

radiobutton $view.c4.live -text "Touch" \
	-variable user_0_mode -value 12 

radiobutton $view.c4.select -text "Select" \
	-variable user_0_mode -value 4 

checkbutton $view.c4.xy_lock -text "XY Lock" \
	-variable xy_lock_pressed
pack $view.c4.live $view.c4.select $view.c4.xy_lock \
        -side top -padx $pad_x -pady 2 -anchor w

# Commit to an operation, like starting to use modification force, or
# accepting the new scan region
button $view.c5.commit -text "Commit!" \
	-command "set commit_pressed 1"

# this is for cancelling an operation, instead of committing. (Line tool)
button $view.c5.cancel -text "Cancel" \
	-command "set cancel_commit 1"
pack $view.c5.commit $view.c5.cancel -side top -padx $pad_x -pady 2 -anchor w


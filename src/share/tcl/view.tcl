#/*===3rdtech===
#  Copyright (c) 2000 by 3rdTech, Inc.
#  All Rights Reserved.
#
#  This file may not be distributed without the permission of 
#  3rdTech, Inc. 
#  ===3rdtech===*/
# This file sets up the $view frame, and must be sourced from a
# specific location inside tools.tcl
#
# for widgets that change the surface window
#

# 5 columns of buttons, 2 in each column
frame $view -relief raised -bd 2

# View variable initialization
set commit_pressed 0
set commit_cancel 0
set center_pressed 0

set pad_x 2
#
#setup surface view buttons
#

radiobutton $view.grab        -text "Grab"  -variable user_0_mode \
	-value 1 
button $view.center           -text "Center" -command "set center_pressed 1"

radiobutton $view.changelight -text "Position Light" -variable user_0_mode \
	-value 10 
radiobutton $view.measure -text "Measure" \
    -variable user_0_mode -value 9 
radiobutton $view.demotouch -text "Touch Stored" \
	-variable user_0_mode -value 11 

radiobutton $view.scaleup     -text "Scale Up"   -variable user_0_mode \
	-value 2 
radiobutton $view.scaledown   -text "Scale Down" -variable user_0_mode \
	-value 3 

radiobutton $view.live -text "Touch" \
	-variable user_0_mode -value 12 

radiobutton $view.select -text "Select" \
	-variable user_0_mode -value 4 

checkbutton $view.xy_lock -text "XY Lock" \
	-variable xy_lock_pressed

# Commit to an operation, like starting to use modification force, or
# accepting the new scan region
button $view.commit -text "Commit!" \
	-command "set commit_pressed 1"

# this is for cancelling an operation, instead of committing. (Line tool)
button $view.cancel -text "Cancel" \
	-command "set cancel_commit 1"

grid $view.grab $view.changelight $view.scaleup $view.live $view.commit \
    -padx $pad_x -pady 2 -sticky nsw
grid $view.center $view.measure $view.scaledown $view.select $view.cancel \
    -padx $pad_x -pady 2 -sticky nsw
grid x  $view.demotouch x $view.xy_lock \
    -padx $pad_x -pady 2 -sticky nsw


# Some of these controls are only relevant to live devices
# They are enabled or disabled in mainwin.tcl based on this list. 
lappend device_only_controls $view.live $view.select $view.xy_lock \
        $view.commit $view.cancel 
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

# View variable initialization
set commit_pressed 0
set commit_cancel 0
set center_pressed 0

#
#setup surface view buttons
#
set bdwdth 0
set magellan_buttons { 1 2 3 4 5 6 7 8 }
# Leave out * because it doesn't do anything except to Magellan.
foreach i $magellan_buttons {
    # Read the button image from a file
    image create photo view_b$i \
	-file [file join ${tcl_script_dir} images b$i.gif] \
	-format GIF 
# The labeled frame puts a big border all the way around, too much padding
#    iwidgets::Labeledframe $view.b$i \
#            -labelimage view_b$i -labelpos w -labelmargin 0
#    set nmInfo(viewb$i) [$view.b$i childsite]

    # This is inspired by looking inside the labeledframe.itk 
    # source file from the iwidgets package - particularly 
    # the use of "place" to display the image.

    set v_image_hwidth [expr [image width view_b$i]/2.0]
    # container frame.
    frame $view.b$i 
    # spacer frame.
    frame $view.b$i.s1 -width $v_image_hwidth
    # The frame with a nice border
    frame $view.b$i.bord -relief groove -borderwidth 2
    # spacer frame
    frame $view.b$i.bord.s2 -width $v_image_hwidth
    # The content frame 
    set nmInfo(viewb$i) [frame $view.b$i.bord.cont]

    pack $view.b$i.s1 -side left -fill y
    pack $view.b$i.bord -side left -fill both -expand yes

    pack $view.b$i.bord.s2 -side left -fill y
    pack $view.b$i.bord.cont -side left -fill both -expand yes

    # The button which will display the button image. 
    # This button is hooked up to act exactly like the 
    # Magellan button when pressed. 
    button $view.b$i.pic -image view_b$i -borderwidth 0 \
            -command "set magellan_button $i"
    # Use the "place" geom manager, which lets us put the image
    # over the edge of the ridge of the frame. 
    # -rely 0.5 puts the top edge of the image at the midpoint
    # of the container frame. Then we use -y to shift up by
    # 1/2 the image height +3 pixels for the ridge frame border. 
    place $view.b$i.pic \
            -relx 0.0 \
            -rely 0.5 -y [expr -$v_image_hwidth -3] \
            -anchor nw
}

radiobutton $nmInfo(viewb1).grab -text "Grab"  -variable user_0_mode \
	-value 1 -padx 0 -pady 0
button $nmInfo(viewb5).center -text "Center" -command "set center_pressed 1"

frame $nmInfo(viewb2).r1
frame $nmInfo(viewb2).r2
radiobutton $nmInfo(viewb2).r1.changelight -text "Position Light" \
        -variable user_0_mode -value 10  -padx 0 -pady 0
radiobutton $nmInfo(viewb2).r1.measure -text "Measure" \
    -variable user_0_mode -value 9  -padx 0 -pady 0
radiobutton $nmInfo(viewb6).demotouch -text "Touch Stored" \
	-variable user_0_mode -value 11  -padx 0 -pady 0

radiobutton $nmInfo(viewb2).r2.scaleup -text "Scale Up"   -variable user_0_mode \
	-value 2  -padx 0 -pady 0
radiobutton $nmInfo(viewb2).r2.scaledown -text "Scale Down" -variable user_0_mode \
	-value 3  -padx 0 -pady 0

radiobutton $nmInfo(viewb3).live -text "Touch" \
	-variable user_0_mode -value 12  -padx 0 -pady 0

radiobutton $nmInfo(viewb3).select -text "Select" \
	-variable user_0_mode -value 4  -padx 0 -pady 0

checkbutton $nmInfo(viewb7).xy_lock -text "XY Lock" \
	-variable xy_lock_pressed -padx 0 -pady 0

# Commit to an operation, like starting to use modification force, or
# accepting the new scan region
button $nmInfo(viewb4).commit -text "Commit!" \
	-command "set commit_pressed 1"

# this is for cancelling an operation, instead of committing. (Line tool)
button $nmInfo(viewb8).cancel -text "Cancel" \
	-command "set cancel_commit 1"

grid $view.b1 $view.b2 $view.b3 $view.b4 \
    -sticky nsew -padx 1 -pady 1
grid $view.b5 $view.b6 $view.b7 $view.b8 \
    -sticky nsew -padx 1 -pady 1

pack $nmInfo(viewb1).grab -side left
pack $nmInfo(viewb2).r1 $nmInfo(viewb2).r2 -side left -fill x
pack $nmInfo(viewb2).r1.changelight $nmInfo(viewb2).r1.measure -anchor nw
pack $nmInfo(viewb2).r2.scaleup $nmInfo(viewb2).r2.scaledown -anchor nw
pack $nmInfo(viewb3).live $nmInfo(viewb3).select -anchor nw
pack $nmInfo(viewb4).commit -side left
pack $nmInfo(viewb5).center -side left
pack $nmInfo(viewb6).demotouch -side left
pack $nmInfo(viewb7).xy_lock -side left
pack $nmInfo(viewb8).cancel -side left

# Some of these controls are only relevant to live devices
# They are enabled or disabled in mainwin.tcl based on this list. 
lappend device_only_controls $nmInfo(viewb3).live $nmInfo(viewb3).select \
        $nmInfo(viewb7).xy_lock $nmInfo(viewb4).commit $nmInfo(viewb8).cancel 
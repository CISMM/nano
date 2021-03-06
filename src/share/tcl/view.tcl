#/* The nanoManipulator and its source code have been released under the
# * Boost software license when nanoManipulator, Inc. ceased operations on
# * January 1, 2014.  At this point, the message below from 3rdTech (who
# * sublicensed from nanoManipulator, Inc.) was superceded.
# * Since that time, the code can be used according to the following
# * license.  Support for this system is now through the NIH/NIBIB
# * National Research Resource at cismm.org.
#
#Boost Software License - Version 1.0 - August 17th, 2003
#
#Permission is hereby granted, free of charge, to any person or organization
#obtaining a copy of the software and accompanying documentation covered by
#this license (the "Software") to use, reproduce, display, distribute,
#execute, and transmit the Software, and to prepare derivative works of the
#Software, and to permit third-parties to whom the Software is furnished to
#do so, all subject to the following:
#
#The copyright notices in the Software and this entire statement, including
#the above license grant, this restriction and the following disclaimer,
#must be included in all copies of the Software, in whole or in part, and
#all derivative works of the Software, unless such copies or derivative
#works are solely in the form of machine-executable object code generated by
#a source language processor.
#
#THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
#SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
#FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
#ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
#DEALINGS IN THE SOFTWARE.
#*/

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
set top_view_pressed 0

#
#setup surface view buttons
#
set bdwdth 0
set magellan_buttons { 1 2 3 4 5 6 7 8 }
if { $viewer_only } {
    set magellan_buttons { 1 2 5 6 }
}
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

    set v_image_hwidth [expr [image width view_b$i]/2.0 + 0.5]
    # container frame.
    frame $view.b$i 
    # spacer frame.
    frame $view.b$i.s1 -width $v_image_hwidth
    # The frame with a nice border
    frame $view.b$i.bord -relief groove -borderwidth 2
    # spacer frame, more space on interior
    frame $view.b$i.bord.s2 -width [expr $v_image_hwidth + 2]
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

radiobutton $nmInfo(viewb1).grab -text "Grab/Scale"  -variable user_0_mode \
	-value 1 -padx 0 -pady 0
button $nmInfo(viewb5).center -text "Center" -command "set center_pressed 1" \
        -padx 5 -pady 2 -highlightthickness 2
button $nmInfo(viewb5).top_view -text "Top View" -command "set top_view_pressed 1" \
        -padx 5 -pady 2 -highlightthickness 2

frame $nmInfo(viewb2).r1
frame $nmInfo(viewb2).r2
radiobutton $nmInfo(viewb2).r1.changelight -text "Position Light" \
        -variable user_0_mode -value 10  -padx 1 -pady 0 -highlightthickness 0
radiobutton $nmInfo(viewb2).r1.measure -text "Measure" \
    -variable user_0_mode -value 9  -padx 1 -pady 0 -highlightthickness 0
radiobutton $nmInfo(viewb6).demotouch -text "Touch Stored" \
	-variable user_0_mode -value 11  -padx 0 -pady 0
radiobutton $nmInfo(viewb6).region -text "Magic Lens" -variable user_0_mode \
	-value 17  -padx 0 -pady 0

radiobutton $nmInfo(viewb2).r2.scaleup -text "Scale"   -variable user_0_mode \
	-value 2  -padx 0 -pady 0
#radiobutton $nmInfo(viewb2).r2.scaledown -text "Scale Down" -variable user_0_mode \
#	-value 3  -padx 0 -pady 0

if { !$viewer_only } {
radiobutton $nmInfo(viewb3).live -text "Touch" \
	-variable user_0_mode -value 12  -padx 1 -pady 0 -highlightthickness 0

radiobutton $nmInfo(viewb3).select -text "Scan Area" \
	-variable user_0_mode -value 4  -padx 1 -pady 0 -highlightthickness 0

checkbutton $nmInfo(viewb7).xy_lock -text "XY Lock" \
	-variable xy_lock_pressed -padx 0 -pady 0

if { !$thirdtech_ui } {    
checkbutton $nmInfo(viewb7).z_lock -text "Z Lock" \
    -variable z_lock_pressed -padx 0 -pady 0
}
# Commit to an operation, like starting to use modification force, or
# accepting the new scan region
button $nmInfo(viewb4).commit -text "Commit!" \
	-command "set commit_pressed 1" -padx 5 -pady 2 -highlightthickness 2

# this is for cancelling an operation, instead of committing. (Line tool)
button $nmInfo(viewb8).cancel -text "Cancel" \
	-command "set cancel_commit 1" -padx 5 -pady 2 -highlightthickness 2

}
grid $view.b1 $view.b2 \
    -sticky nsew -padx 1 -pady 1
grid $view.b5 $view.b6 \
    -sticky nsew -padx 1 -pady 1
if { !$viewer_only } {
grid $view.b3 -sticky nsew -padx 1 -pady 1 -row 0 -column 2
grid $view.b4 -sticky nsew -padx 1 -pady 1 -row 0 -column 3
grid $view.b7 -sticky nsew -padx 1 -pady 1 -row 1 -column 2
grid $view.b8 -sticky nsew -padx 1 -pady 1 -row 1 -column 3

}
pack $nmInfo(viewb1).grab -side left
pack $nmInfo(viewb2).r1 $nmInfo(viewb2).r2 -side left -fill x -expand yes
pack $nmInfo(viewb2).r1.changelight $nmInfo(viewb2).r1.measure -anchor nw
pack $nmInfo(viewb2).r2.scaleup -anchor nw
#$nmInfo(viewb2).r2.scaledown 
if { !$viewer_only } {
pack $nmInfo(viewb3).live $nmInfo(viewb3).select -anchor nw
pack $nmInfo(viewb4).commit -side left -fill x -expand yes
}
pack $nmInfo(viewb5).center -side left -fill x -expand yes
pack $nmInfo(viewb5).top_view -side left

pack $nmInfo(viewb6).demotouch -side left
pack $nmInfo(viewb6).region -side left 
if { !$viewer_only } {
pack $nmInfo(viewb7).xy_lock -side left
if { !$thirdtech_ui } {    
pack $nmInfo(viewb7).z_lock -side left
}
pack $nmInfo(viewb8).cancel -side left -fill x -expand yes
}
# Some of these controls are only relevant to live devices
# They are enabled or disabled in mainwin.tcl based on this list. 
if { !$viewer_only } {
lappend device_only_controls $nmInfo(viewb3).live $nmInfo(viewb3).select \
        $nmInfo(viewb7).xy_lock $nmInfo(viewb4).commit \
        $nmInfo(viewb8).cancel 
if { !$thirdtech_ui } {    
lappend device_only_controls $nmInfo(viewb7).z_lock 
}
}
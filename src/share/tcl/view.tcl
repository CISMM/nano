# This file sets up the $view frame, and must be sourced from a
# specific location inside tools.tcl
#
# for widgets that change the surface window
#

frame $view -relief raised -bg $bc -bd 2
frame $view.r1 -bg $bc
frame $view.r2 -bg $bc
frame $view.r3 -bg $bc
frame $view.r4 -bg $bc
frame $view.r5 -bg $bc

#label $view.label -text "Surface View" -bg $bc
#pack $view.label -side top -anchor nw
pack $view.r1 $view.r2 $view.r3 $view.r4 $view.r5 -side top -fill both

# View variable initialization
set view_mode 0
set commit_pressed 0
set commit_cancel 0
set center_pressed 0

#
#setup surface view buttons
#

radiobutton $view.r1.grab        -text "Grab"      -variable user_0_mode \
	-value 1 -bg $fc 
radiobutton $view.r1.measure     -text "Measure"    -variable user_0_mode \
	-value 9 -bg $fc 
radiobutton $view.r1.changelight -text "Position Light" -variable user_0_mode \
	-value 10 -bg $fc 
radiobutton $view.r1.scaleup     -text "Scale Up"   -variable user_0_mode \
	-value 2  -bg $fc 
radiobutton $view.r1.scaledown   -text "Scale Down" -variable user_0_mode \
	-value 3 -bg $fc 

pack $view.r1.grab $view.r1.measure $view.r1.changelight $view.r1.scaleup \
	$view.r1.scaledown -side left -padx 3m -pady 2

#Tom Hudson NANOX
#button $view.r2.center -text "Center" -bg $fc -command "set term_input \"\^\" "
button $view.r2.center -text "Center" -bg $fc -command "set center_pressed 1"
#Aron Helser -temporary, to test new centering commands
button $view.r2.saveview -text "Save View" -bg $fc -command "set save_xform 1 "
button $view.r2.setview -text "Recall View" -bg $fc -command "set set_xform 1 "
radiobutton $view.r2.select -text "Select" -bg $fc \
	-variable user_0_mode -value 4 
radiobutton $view.r2.canned -text "Touch - Canned" -bg $fc \
	-variable user_0_mode -value 11 

# NANOX
set tcl_lightDirX 0.0
set tcl_lightDirY 1.0
set tcl_lightDirZ 0.5
set tcl_wfr_xlate_X 0.0
set tcl_wfr_xlate_Y 0.0
set tcl_wfr_xlate_Z 0.0
set tcl_wfr_rot_0 0.0
set tcl_wfr_rot_1 0.0
set tcl_wfr_rot_2 0.0
set tcl_wfr_rot_3 1.0
set tcl_wfr_scale 1.0

pack $view.r2.center \
    $view.r2.select $view.r2.canned $view.r2.saveview $view.r2.setview \
    -side left -padx 3m -pady 2

frame $view.r3.cc -relief raised -bg $bc -bd 2
checkbutton $view.r3.cc.commit -text "Commit!" -bg $fc \
	-variable commit_pressed -relief raised -bd 2

# this is for cancelling an operation, instead of committing. (Line tool)
button $view.r3.cc.cancel -text "Cancel" -bg $fc \
	-command "set cancel_commit 1"

# same as menu command - Touch in line mode - LIVE
radiobutton $view.r3.live -text "Touch before Modify" -bg $fc \
	-variable user_0_mode -value 12 

radiobutton $view.r3.scanline -text "Line Scan" -bg $fc \
	-variable user_0_mode -value 16


button $view.r3.modmarkclr -text "Mod Mark Clear" -bg $fc  \
	-command "set term_input C"

pack $view.r3.cc $view.r3.live $view.r3.scanline  \
    $view.r3.modmarkclr\
	-side left -padx 3m -pady 2
pack $view.r3.cc.commit $view.r3.cc.cancel \
	-side left -padx 3m -pady 2

#Extra window that can be shown and hidden
button $view.r4.navigate -text "Navigate" -bg $fc \
    -command "show_nav_win"

button $view.r4.stripchart -text "Stripchart" -bg $fc \
    -command "show_stripchart_win"

button $view.r4.modfile -text "Modfile" -bg $fc \
    -command "show_mod_win"

button $view.r4.data_sets -text "Data Sets" -bg $fc \
    -command "show_data_sets_win"

button $view.r4.sliders -text "Sliders" -bg $fc \
    -command "show_sliders_win"

button $view.r4.streamfile -text "Replay Control" -bg $fc \
    -command "show_streamfile_win"
pack  $view.r4.sliders $view.r4.stripchart $view.r4.modfile \
    $view.r4.data_sets $view.r4.navigate $view.r4.streamfile \
    -side left -padx 4 -pady 2

#frame $view.r5


button $view.r5.registration -text "Data Registration" -bg $fc \
    -command "show_reg_win"

button $view.r5.sem -text "SEM" -bg $fc \
    -command "show_sem_win"

button $view.r5.vicurve -text "VI Curve" -bg $fc \
    -command "show_vicurve_win"

button $view.r5.import_objects -text "Import Objects" -bg $fc -command "show_import_objects_win"

button $view.r5.french_ohmmeter -text "Ohmmeter" -bg $fc \
    -command "show_french_ohmmeter_win"

button $view.r5.shared_ptr -text "Synchronization" -bg $fc \
	-command "show_shared_ptr_win"

pack $view.r5.registration $view.r5.sem \
    $view.r5.import_objects $view.r5.vicurve $view.r5.french_ohmmeter \
    $view.r5.shared_ptr \
    -side left -padx 4 -pady 2


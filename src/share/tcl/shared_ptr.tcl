# Amy Henderson's shared pointers (Aug 99)
# Tom Hudson's streamfile synchronization (Sept 99)

set sharedptr(sp) .sharedptr

toplevel $sharedptr(sp)
wm withdraw $sharedptr(sp)

button $sharedptr(sp).close -text "Close" -command {
	wm withdraw $sharedptr(sp)
}

wm protocol $sharedptr(sp) WM_DELETE_WINDOW {
	$sharedptr(sp).close invoke
}
pack $sharedptr(sp).close -anchor nw

proc show_shared_ptr_win {} {
	global sharedptr
	wm deiconify $sharedptr(sp)
	raise $sharedptr(sp)
}

# Choose a name for the machine to collaborate with
# Changed TCH 31 Oct 99 to a generic_entry so that if the name
# is set with command-line parameters it shows up in the tcl interface

# iwidgets::entryfield $sharedptr(sp).collab_machine_name \
# 	-labeltext "Machine to connect to:" \
# 	-command { set collab_machine_name \
# 		[$sharedptr(sp).collab_machine_name get]}
set collab_machine_name ""
generic_entry $sharedptr(sp).collab_machine_name \
              collab_machine_name \
              "Machine to connect to:" \
              ""
pack $sharedptr(sp).collab_machine_name -side top



# Streamfile synchronization
# NANOX

frame $sharedptr(sp).stream_frame
pack $sharedptr(sp).stream_frame -fill x -side top

checkbutton $sharedptr(sp).stream_frame.synchronize_stream \
    -text "Synchronize stream files"

set get_stream_sync 0

button $sharedptr(sp).stream_frame.get_stream_sync_button \
    -text "Copy" -command {set get_stream_sync 1}

pack $sharedptr(sp).stream_frame.synchronize_stream -side left
pack $sharedptr(sp).stream_frame.get_stream_sync_button -side right

# View synchronization
# NANOX

frame $sharedptr(sp).view_frame
pack $sharedptr(sp).view_frame -fill x -side top

checkbutton $sharedptr(sp).view_frame.synchronize_view \
    -text "Synchronize view"

set get_view_sync 0

button $sharedptr(sp).view_frame.get_view_sync_button \
    -text "Copy" -command {set get_view_sync 1}

# View synchronization hierarchy

frame $sharedptr(sp).view_frame.sf
pack $sharedptr(sp).view_frame.sf -side bottom

# .5 cm left padding
frame $sharedptr(sp).view_frame.sf.pad -width .5c
pack $sharedptr(sp).view_frame.sf.pad -side left

frame $sharedptr(sp).view_frame.sf.plane_frame
pack $sharedptr(sp).view_frame.sf.plane_frame -fill x -side top

checkbutton \
  $sharedptr(sp).view_frame.sf.plane_frame.synch_view_plane \
  -text "Plane"
set get_view_plane_sync 0
button \
  $sharedptr(sp).view_frame.sf.plane_frame.get_view_plane_sync \
  -text "Copy" -command {set get_view_plane_sync 1}
pack $sharedptr(sp).view_frame.sf.plane_frame.synch_view_plane \
  -side left
pack $sharedptr(sp).view_frame.sf.plane_frame.get_view_plane_sync \
  -side right

frame $sharedptr(sp).view_frame.sf.color_frame
pack $sharedptr(sp).view_frame.sf.color_frame -fill x -side top

checkbutton $sharedptr(sp).view_frame.sf.color_frame.synch_view_color \
  -text "Color"
set get_view_color_sync 0
button $sharedptr(sp).view_frame.sf.color_frame.get_view_color_sync \
  -text "Copy" -command {set get_view_color_sync 1}
pack $sharedptr(sp).view_frame.sf.color_frame.synch_view_color -side left
pack $sharedptr(sp).view_frame.sf.color_frame.get_view_color_sync -side right

frame $sharedptr(sp).view_frame.sf.measure_frame
pack $sharedptr(sp).view_frame.sf.measure_frame -fill x -side top

checkbutton $sharedptr(sp).view_frame.sf.measure_frame.synch_view_measure \
  -text "Measure Lines"
set get_view_measure_sync 0
button $sharedptr(sp).view_frame.sf.measure_frame.get_view_measure_sync \
  -text "Copy" -command {set get_view_measure_sync 1}
pack $sharedptr(sp).view_frame.sf.measure_frame.synch_view_measure -side left
pack $sharedptr(sp).view_frame.sf.measure_frame.get_view_measure_sync \
  -side right

frame $sharedptr(sp).view_frame.sf.lighting_frame
pack $sharedptr(sp).view_frame.sf.lighting_frame -fill x -side top

checkbutton $sharedptr(sp).view_frame.sf.lighting_frame.synch_view_lighting \
  -text "Lighting"
set get_view_lighting_sync 0
button $sharedptr(sp).view_frame.sf.lighting_frame.get_view_lighting_sync \
  -text "Copy" -command {set get_view_lighting_sync 1}
pack $sharedptr(sp).view_frame.sf.lighting_frame.synch_view_lighting -side left
pack $sharedptr(sp).view_frame.sf.lighting_frame.get_view_lighting_sync \
  -side right

pack $sharedptr(sp).view_frame.synchronize_view -side left
pack $sharedptr(sp).view_frame.get_view_sync_button -side right


frame $sharedptr(sp).view_frame.sf.contour_frame
pack $sharedptr(sp).view_frame.sf.contour_frame -fill x -side top

checkbutton $sharedptr(sp).view_frame.sf.contour_frame.synch_view_contour \
  -text "Contour Lines"
set get_view_contour_sync 0
button $sharedptr(sp).view_frame.sf.contour_frame.get_view_contour_sync \
  -text "Copy" -command {set get_view_contour_sync 1}
pack $sharedptr(sp).view_frame.sf.contour_frame.synch_view_contour -side left
pack $sharedptr(sp).view_frame.sf.contour_frame.get_view_contour_sync \
  -side right


frame $sharedptr(sp).view_frame.sf.grid_frame
pack $sharedptr(sp).view_frame.sf.grid_frame -fill x -side top

checkbutton $sharedptr(sp).view_frame.sf.grid_frame.synch_view_grid \
  -text "Rulergrid"
set get_view_grid_sync 0
button $sharedptr(sp).view_frame.sf.grid_frame.get_view_grid_sync \
  -text "Copy" -command {set get_view_grid_sync 1}
pack $sharedptr(sp).view_frame.sf.grid_frame.synch_view_grid -side left
pack $sharedptr(sp).view_frame.sf.grid_frame.get_view_grid_sync \
  -side right




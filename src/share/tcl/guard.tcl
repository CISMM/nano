#!/bin/sh
# ----------------------------------------------------------------------
#  Guarded scan interface
# ----------------------------------------------------------------------
#\
exec wish "$0" ${2+"$@"}

# Import Itcl and Iwidgets
package require Itcl 
catch {namespace import itcl::*}
package require Itk 
catch {namespace import itk::*}
package require Iwidgets

# GLOBAL variables
# ----------------------------------------------------------------------
global guarded_scan_start guarded_scan_stop guarded_plane_acquire guarded_plane_depth guarded_scan_max_step

set nmInfo(guard) [create_closing_toplevel_with_notify guarded_win guarded_window_open]
# ------------------------------------------------- end GLOBAL variables

# Button to acquire guard plane
button $nmInfo(guard).plane_button -text "Acquire Plane" \
	-command { set guarded_plane_acquire 1 }
pack $nmInfo(guard).plane_button -anchor n

# Input box to acquire guard depth (below acquired plane)
set guarded_plane_depth 0.0
generic_entry $nmInfo(guard).guarded_plane_depth_cnt guarded_plane_depth \
	"Guard depth below plane (height units)" real
pack $nmInfo(guard).guarded_plane_depth_cnt -anchor n

# Input box to acquire guard step
set guarded_scan_max_step 0.01
generic_entry $nmInfo(guard).guarded_scan_max_step_cnt guarded_scan_max_step \
	"Max Z step (height units)" real
pack $nmInfo(guard).guarded_scan_max_step_cnt -anchor n

# Button to start/stop the scan
set guard_start_or_stop "start"
button $nmInfo(guard).scan_button -text "Start Scan" -command {
    global guard_start_or_stop nmInfo
    if {$guard_start_or_stop == "start"} {
	set guard_start_or_stop "stop"
	$nmInfo(guard).scan_button configure -text "Stop Scan"
	set guarded_scan_start 1
	$nmInfo(guard).plane_button configure -state disabled
    } else {
	set guard_start_or_stop "start"
	$nmInfo(guard).scan_button configure -text "Start Scan"
	set guarded_scan_stop 1
	$nmInfo(guard).plane_button configure -state normal
    }
}
pack $nmInfo(guard).scan_button -anchor w


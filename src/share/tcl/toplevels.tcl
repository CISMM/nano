# Create a toplevel window for selecting which point data sets 
# are returned when modifying.
# Provide a toplevel widget to allow
# control over which data sets are enabled in scan and touch
# mode.  
# Put in one sub-area for scan and one for touch. */
# ----------------------------------------------------------------------
set nmInfo(data_sets) .data_sets

# ----------------------------------------------------------------------
toplevel $nmInfo(data_sets)
wm withdraw $nmInfo(data_sets)

button $nmInfo(data_sets).close -text "Close" -command {
    wm withdraw $nmInfo(data_sets)
}
wm protocol $nmInfo(data_sets) WM_DELETE_WINDOW {$nmInfo(data_sets).close invoke}
pack $nmInfo(data_sets).close -anchor nw

proc show_data_sets_win {} {
    global nmInfo
    wm deiconify $nmInfo(data_sets)
    raise $nmInfo(data_sets)
}

frame .data_sets.scan -relief sunken -bd 2
label .data_sets.scan.label -text "Image Scan"
frame .data_sets.touch -relief sunken -bd 2
label .data_sets.touch.label -text Touch
label .data_sets.touch.instr -text "Hit Enter to change no. of samples"
frame .data_sets.forcecurve -relief sunken -bd 2
label .data_sets.forcecurve.label -text ForceCurve
#frame .data_sets.scanline -relief sunken -bd 2
#label .data_sets.scanline.label -text "Line Scan"

pack .data_sets.scan.label -side top
pack .data_sets.scan -side left -anchor n
pack .data_sets.touch.label .data_sets.touch.instr -side top
pack .data_sets.touch -side left -anchor n
pack .data_sets.forcecurve.label -side top
pack .data_sets.forcecurve -side left -anchor n
#pack .data_sets.scanline.label -side top
#pack .data_sets.scanline -side left -anchor n

# Create a toplevel window for various sliders we don't know where
# else to put.  It is initially hidden, but can be shown by calling
# the "show" procedure.
# ----------------------------------------------------------------------
set nmInfo(sliders) .sliders

# ----------------------------------------------------------------------
toplevel $nmInfo(sliders)
wm withdraw $nmInfo(sliders)

button $nmInfo(sliders).close -text "Close" -command {
    wm withdraw $nmInfo(sliders)
}
wm protocol $nmInfo(sliders) WM_DELETE_WINDOW {$nmInfo(sliders).close invoke}
pack $nmInfo(sliders).close -anchor nw

proc show_sliders_win {} {
    global nmInfo
    wm deiconify $nmInfo(sliders)
    raise $nmInfo(sliders)
}

#make sliders initially visible
#after idle show_sliders_win 

# Create a toplevel window for the french ohmmeter controls.
# It is initially hidden, but can be shown by calling
# the "show" procedure.
# ----------------------------------------------------------------------
set french_ohmmeter [toplevel .french_ohmmeter]

wm withdraw $french_ohmmeter

button $french_ohmmeter.close -text "Close" -command {
    wm withdraw $french_ohmmeter
}
wm protocol $french_ohmmeter WM_DELETE_WINDOW {$french_ohmmeter.close invoke}
pack $french_ohmmeter.close -anchor nw

proc show_french_ohmmeter_win {} {
    global french_ohmmeter
    wm deiconify $french_ohmmeter
    raise $french_ohmmeter
}
# ----------------------------------------------------------------------


# Create a toplevel window for the Keithley VI Curve generator controls.
# It is initially hidden, but can be shown by calling
# the "show" procedure.
# ----------------------------------------------------------------------
set vi_win [toplevel .vi_win]

wm withdraw $vi_win

button $vi_win.close -text "Close" -command {
    wm withdraw $vi_win
}
wm protocol $vi_win WM_DELETE_WINDOW {$vi_win.close invoke}
pack $vi_win.close -anchor nw

proc show_vicurve_win {} {
    global vi_win
    wm deiconify $vi_win
    raise $vi_win
}

# ----------------------------------------------------------------------
# Create a toplevel window for the navigation pad, which moves the surface.
# It is initially hidden, but can be shown by calling
# the "show" procedure.
# ----------------------------------------------------------------------
set nav_win [toplevel .nav_win]

wm withdraw $nav_win

button $nav_win.close -text "Close" -command {
    wm withdraw $nav_win
}
wm protocol $nav_win WM_DELETE_WINDOW {$nav_win.close invoke}
pack $nav_win.close -anchor nw

basicjoys_create $nav_win.joy "Translate" "Rotate"

proc show_nav_win {} {
    global nav_win
    wm deiconify $nav_win
    raise $nav_win
}
# ----------------------------------------------------------------------
# Create a toplevel window for the SEM controls
# It is initially hidden, but can be shown by calling
# the "show" procedure.
# ----------------------------------------------------------------------
set sem_win [toplevel .sem_win]

wm withdraw $sem_win

button $sem_win.close -text "Close" -command {
    global sem_window_open
    wm withdraw $sem_win
    set sem_window_open 0 
}
wm protocol $sem_win WM_DELETE_WINDOW {$sem_win.close invoke}
pack $sem_win.close -anchor nw

proc show_sem_win {} {
    global sem_win
    global sem_window_open
    wm deiconify $sem_win
    raise $sem_win
    set sem_window_open 1
}

# ----------------------------------------------------------------------
# Create a toplevel window for the Registration controls
# It is initially hidden, but can be shown by calling
# the "show" procedure.
# ----------------------------------------------------------------------
set reg_win [toplevel .reg_win]

wm withdraw $reg_win

button $reg_win.close -text "Close" -command {
    global reg_window_open
    wm withdraw $reg_win
    set reg_window_open 0
}
wm protocol $reg_win WM_DELETE_WINDOW {$reg_win.close invoke}
pack $reg_win.close -anchor nw

proc show_reg_win {} {
    global reg_win
    global reg_window_open
    wm deiconify $reg_win
    raise $reg_win
    set reg_window_open 1
}


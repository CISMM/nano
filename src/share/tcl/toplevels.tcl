# Create a toplevel window for the Keithley VI Curve generator controls.
# It is initially hidden, but can be shown by calling
# the "show" procedure.
# ----------------------------------------------------------------------
set vi_win [create_closing_toplevel vi_win "VI Curves"]

# ----------------------------------------------------------------------
# Create a toplevel window for the navigation pad, which moves the surface.
# It is initially hidden, but can be shown by calling
# the "show" procedure.
# ----------------------------------------------------------------------
set nav_win [create_closing_toplevel nav_win "Navigate Tool"]

basicjoys_create $nav_win.joy "Translate" "Rotate"

set phantom_win [create_closing_toplevel phantom_win "Phantom Settings"]
button $phantom_win.phantom_reset -text "Reset Phantom" -command "set reset_phantom 1"
generic_radiobox $phantom_win.using_phantom_button \
	using_phantom_button \
	"Phantom Button" { "ButtonBox" "Phantom Trigger" }

pack $phantom_win.phantom_reset $phantom_win.using_phantom_button \
	-side top -fill x

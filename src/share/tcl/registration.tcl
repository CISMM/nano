global registration_enabled registration_needed 
global resample_resolution_x resample_resolution_y

set registrationmod(sf) [create_closing_toplevel registration]

frame $registrationmod(sf).selection3D
pack $registrationmod(sf).selection3D
frame $registrationmod(sf).selection2D
pack $registrationmod(sf).selection2D

frame $registrationmod(sf).rotate3D_enable
pack $registrationmod(sf).rotate3D_enable


button $registrationmod(sf).register -text "Register" -command {
    set registration_needed 1 }
pack $registrationmod(sf).register

# stuff for creating resampled data:
set resample_plane_name ""
#set resample_from ""
frame $registrationmod(sf).resample_plane -relief raised -bd 4
frame $registrationmod(sf).resample_plane.choice
frame $registrationmod(sf).resample_plane.name
pack $registrationmod(sf).resample_plane.choice -side left
pack $registrationmod(sf).resample_plane.name
label $registrationmod(sf).resample_plane.name.label -text "Resample plane name"
pack $registrationmod(sf).resample_plane.name.label
newlabel_dialogue resample_plane_name $registrationmod(sf).resample_plane.name
pack $registrationmod(sf).resample_plane -fill both

intscale $registrationmod(sf).resolution_x 10 10000 9991 1 1 \
		resample_resolution_x "x resolution"

intscale $registrationmod(sf).resolution_y 10 10000 9991 1 1 \
		resample_resolution_y "y resolution"

pack $registrationmod(sf).resolution_x $registrationmod(sf).resolution_y \
	-fill both



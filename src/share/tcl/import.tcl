set nmInfo(import_objects) [create_closing_toplevel import_objects "Import Objects" ]

set import_dir ""
set import_file_label "none"
set imported_objects {none }

set import_transx 0
set import_transy 0
set import_transz 0
set import_scale 1
set import_rotx 0
set import_roty 0
set import_rotz 0
set import_type ""
set import_visibility 1
set import_proj_text 1
set import_CCW 1
set import_tess 10
set import_axis_step 1
set import_clamp 0

set import_color gray
set import_r 192
set import_g 192
set import_b 192
set import_color_changed 0

iwidgets::Labeledframe $nmInfo(import_objects).basics \
	-labeltext "Import Models" \
	-labelpos nw
set nmInfo(basic_options) [$nmInfo(import_objects).basics childsite]

pack $nmInfo(import_objects).basics -side top -fill x

frame $nmInfo(basic_options).file
frame $nmInfo(basic_options).file.buttons

generic_optionmenu $nmInfo(basic_options).file.buttons.imported_files current_object\
	"Current Object" imported_objects

#label $nmInfo(basic_options).file.import_file_label -justify left -text \
#	"Current Imported file: $import_file_label"

button $nmInfo(basic_options).file.buttons.import_button -text "Import File" -command open_import_file
radiobutton $nmInfo(basic_options).file.buttons.ccw -text "CCW" -variable import_CCW -value 1
radiobutton $nmInfo(basic_options).file.buttons.cw -text "CW" -variable import_CCW -value 0
button $nmInfo(basic_options).file.buttons.close_button -text "Close File" -command close_import_file
#button $nmInfo(basic_options).file.buttons.close_button -text "Close File" import_close_file 

#generic_entry $nmInfo(basic_options).modelFile modelFile "Enter the file to import:" ""

frame $nmInfo(basic_options).f1
frame $nmInfo(basic_options).f2
frame $nmInfo(basic_options).f3
generic_entry $nmInfo(basic_options).f1.import_transx import_transx \
     "X Translation" real
generic_entry $nmInfo(basic_options).f1.import_transy import_transy \
     "Y Translation" real
generic_entry $nmInfo(basic_options).f1.import_transz import_transz \
     "Z Translation" real
generic_entry $nmInfo(basic_options).f2.import_rotx import_rotx \
     "X Rotation" real
generic_entry $nmInfo(basic_options).f2.import_roty import_roty \
     "Y Rotation" real
generic_entry $nmInfo(basic_options).f2.import_rotz import_rotz \
     "Z Rotation" real
generic_entry $nmInfo(basic_options).f3.import_scale import_scale \
     "Scale" real 
generic_entry $nmInfo(basic_options).f3.import_tess import_tess \
     "Tube Loading Tesselation" integer
generic_entry $nmInfo(basic_options).f3.import_axis_step import_axis_step \
     "Tube Loading Axis Step" integer

button $nmInfo(basic_options).f3.visibility_button -text "Hide" -command change_visibility
checkbutton $nmInfo(basic_options).f3.proj_text_button \
    -text "Show Projective Texture" -variable import_proj_text
checkbutton $nmInfo(basic_options).f3.clamp_button \
    -text "Lock Projective Texture" -variable import_clamp


button $nmInfo(basic_options).f3.set_color \
        -text "Set color" -command {
            if {[choose_color import_color "Choose color" $nmInfo(basic_options)] } {
                $nmInfo(basic_options).f3.colorsample configure -bg $import_color 
                set_import_color
            }
        }  

button $nmInfo(basic_options).f3.colorsample \
        -relief groove -bd 2 -bg $import_color \
        -command { $nmInfo(basic_options).f3.set_color invoke}


#pack $nmInfo(basic_options).modelFile -side top -anchor w -padx 1m -pady 1m -fill x
pack $nmInfo(basic_options).file -anchor w -fill x
#pack $nmInfo(basic_options).file.import_file_label -anchor w -side left
pack $nmInfo(basic_options).file.buttons -anchor w -fill x
pack $nmInfo(basic_options).file.buttons.imported_files -anchor nw -side left -padx 1m -pady 1m
pack $nmInfo(basic_options).file.buttons.import_button -anchor nw -side left -padx 1m -pady 1m
pack $nmInfo(basic_options).file.buttons.close_button -anchor nw -side left -padx 1m -pady 1m
pack $nmInfo(basic_options).file.buttons.ccw -anchor nw 
pack $nmInfo(basic_options).file.buttons.cw -anchor nw


pack $nmInfo(basic_options).f1 -side left -fill x
pack $nmInfo(basic_options).f2 -side left -fill x
pack $nmInfo(basic_options).f3 -side left -fill x
pack $nmInfo(basic_options).f1.import_transx -padx 1m -pady 1m -anchor nw
pack $nmInfo(basic_options).f1.import_transy -padx 1m -pady 1m
pack $nmInfo(basic_options).f1.import_transz -padx 1m -pady 1m
pack $nmInfo(basic_options).f2.import_rotx -padx 1m -pady 1m -anchor nw
pack $nmInfo(basic_options).f2.import_roty -padx 1m -pady 1m
pack $nmInfo(basic_options).f2.import_rotz -padx 1m -pady 1m
pack $nmInfo(basic_options).f3.import_scale -padx 1m -pady 1m -anchor nw
pack $nmInfo(basic_options).f3.import_tess -padx 1m -pady 1m -anchor nw
pack $nmInfo(basic_options).f3.import_axis_step -padx 1m -pady 1m -anchor nw
pack $nmInfo(basic_options).f3.visibility_button -anchor nw -padx 1m -pady 1m -fill x
pack $nmInfo(basic_options).f3.proj_text_button -anchor nw -padx 1m -pady 1m -fill x
pack $nmInfo(basic_options).f3.clamp_button -anchor sw -padx 1m -pady 1m -fill x
pack $nmInfo(basic_options).f3.set_color -side left -padx 1m
pack $nmInfo(basic_options).f3.colorsample -side left -padx 1m -fill x -expand yes

trace variable import_type w SetupOptions

#
################################
# Open an import file. 
proc open_import_file {} {
    global modelFile nmInfo import_dir import_file_label current_object current_object_new
    set types { {"Wave Front files" ".obj" } 
                {"MSI files" ".msi" } 
		    {"Text files" ".txt" }
                {"All files" *} }
    set filename [tk_getOpenFile -filetypes $types \
            -initialdir $import_dir \
            -title "Import a file"]
    if {$filename != ""} {
	# setting this variable triggers a callback in C code
	# which saves the file. 
        # dialog check whether file exists.
        set modelFile $filename
        set import_dir [file dirname $filename]

        regexp -indices {[^/]*$} $modelFile model_ind
        set m_start [lindex [split $model_ind] 0]
        set m_end [lindex [split $model_ind] 1]
    
        set import_file_label [string range $modelFile $m_start $m_end]
    } else {
        set modelFile ""
        set import_file_label "NONE"
    }
#    $nmInfo(basic_options).file.import_file_label configure -text \
#	"Current Imported file: $import_file_label"

 	set current_object_new $import_file_label
}

proc close_import_file {} {
    global modelFile nmInfo import_file_label

    set modelFile ""
    set import_file_label "all"

#    $nmInfo(basic_options).file.import_file_label configure -text \
#	"Current Imported file: $import_file_label"

#    destroy $nmInfo(import_objects).options
}

proc set_import_color {} {
    global import_r import_g import_b import_color_changed import_color
    global nmInfo

    # Extract three component colors of contour_color 
    # and save into import_r g b
    scan $import_color #%02x%02x%02x import_r import_g import_b
    set import_color_changed 1
}

##This procedure changes the visibility_mode from hide to show (set it up
# so that the object will be drawn)
proc change_visibility {} {
  global nmInfo
  global import_visibility
   
  if { $import_visibility == 1 } {
    ##Change the hide button to a show button
    $nmInfo(basic_options).f3.visibility_button configure -text "Show"
    set import_visibility 0
  } else { 
    ##Change the show button to a hide button  
    $nmInfo(basic_options).f3.visibility_button configure -text "Hide"
    set import_visibility 1
  }
}

proc SetupOptions {name el op} {
    global import_type
    global nmInfo

    destroy $nmInfo(import_objects).options

    if { [string compare $import_type "msi"] == 0 } {
        StandardFileOptions
        MSIFileOptions
    } 
}

proc StandardFileOptions {} {
    global nmInfo

    iwidgets::Labeledframe $nmInfo(import_objects).options \
	    -labeltext "Options" \
	    -labelpos nw
    set nmInfo(adv_options) [$nmInfo(import_objects).options childsite]

    pack $nmInfo(import_objects).options -side top -fill x
}

# MSI Specific variables

set msi_bond_mode 0
set msi_atom_radius 1

proc MSIFileOptions {} {
    global nmInfo
    global msi_color
    global msi_atom_radius
    global msi_bond_mode
    
    #Reset some variables
    #set msi_atom_radius 1
    set msi_bond_mode 0
        
    frame $nmInfo(adv_options).f
        
    button $nmInfo(adv_options).f.bond_mode -text "Atoms" -command change_bond_mode

    pack $nmInfo(adv_options).f -side top -anchor nw
    pack $nmInfo(adv_options).f.bond_mode -side left -padx 1m
}

##This procedure changes the import_mode from spheres to bonds (set it up
# so that the nanotube's bonds will be drawn, rather than its atoms)
proc change_bond_mode {} {
  global nmInfo
  global msi_bond_mode
  global msi_atom_radius

  if { $msi_bond_mode == 0 } {
    ##Change the bonds button to an atoms button
    set msi_bond_mode 1

    $nmInfo(adv_options).f.bond_mode configure -text "Bonds"
    generic_entry $nmInfo(adv_options).f.atom_radius msi_atom_radius "Atom Radius" real
    pack $nmInfo(adv_options).f.atom_radius -side left -padx 1m

  } else {
    set msi_bond_mode 0
    set msi_switch_bond_mode 0

    ##Change the atoms button to an bonds button
    $nmInfo(adv_options).f.bond_mode configure -text "Atoms"

    destroy $nmInfo(adv_options).f.atom_radius
  }
}

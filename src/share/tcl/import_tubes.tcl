#import_tubes.tcl

###########################################################################
# This file contains tcl code for importing objects into the import_objects
# manipulator, which was originally designed for importing objects from 
# tube_foundry
# The C code which interfaces with this Tcl code is in imported
# Written by Leila Plummer
# Last modified 7/2/99 by Leila Plummer
###########################################################################

###########################################################################
# Variable clarification:
# --import_mode determines whether or not to draw the bonds of nanotubes
#   (as lines) or the atoms of nanotubes (as spheres).  It is 0 when we
#   are in bond mode, and 1 when we are in sphere mode
# --visbility_mode determines whether or not to draw the object.  It is
#   0 when in hide mode (don't draw the object), and 1 when in show mode
#   (draw the object)
# --importobj(import_objects) is the frame which contains the tabnotebook
#   which contains each object's translate and rotate widgets, as well as
#   the file loading widgets
# --page_count keeps track of which page or object we are creating
###########################################################################

# Import Itcl and Iwidgets, for the tabnotebook widget and others we
# will use. This import method is a BUG - we shouldn't have to import
# the itcl and itk namespaces, but at least we can use it...
catch {package require Itcl} 
#catch {namespace import itcl::*}
catch {package require Itk} 
#catch {namespace import itk::*}
catch {package require Iwidgets}

#options for tabbed notebook
catch {option add *Tabnotebook.equalTabs no startupFile}
catch {option add *Tabnotebook.backdrop tan startupFile}
catch {option add *Tabnotebook.tabBackground tan startupFile}

##Initializations
set page_count 0
set curr_page 0
set load_button_press 0
#  set delete_object($page_count) 0;

##Set up frame which contains the tabnotebook, which will contain each 
# object's translate and rotate widgets, as well as the file loading widgets
set importobj(import_objects) [create_closing_toplevel import_objects "Import Model Objects" ]

##Set up a "subframe" to hold the tabnotebook
frame $importobj(import_objects).sources
pack $importobj(import_objects).sources -expand yes -fill both -padx 10 -pady 4
##Set up the tabnotebook
set importobj(notebook) [iwidgets::tabnotebook $importobj(import_objects).sources.tabnote -width 4i -height 6i -tabpos n]
pack $importobj(notebook) -side left -expand yes -fill both -padx 4 -pady 4

##Set up a frame for loading the filename
frame $importobj(import_objects).getfile -relief groove -borderwidth 4

##Add a button to load the file, and an entry to get the filename
button $importobj(import_objects).getfile.load_import_file_button -text "Load File" -command {
    set load_button_press [expr $load_button_press+1]
    import_proc
} 
label $importobj(import_objects).getfile.filename_label -text "File name (with extension):"
entry  $importobj(import_objects).getfile.entry -width 20 -relief sunken -bd 2 -textvariable import_filename
pack  $importobj(import_objects).getfile -side top -anchor nw -fill x
pack $importobj(import_objects).getfile.load_import_file_button $importobj(import_objects).getfile.filename_label $importobj(import_objects).getfile.entry -in $importobj(import_objects).getfile -side top -padx 1m -pady 2m

##This procedure shows the view_objects window, which contains an object's
# bond and sphere manipulation widgets (e.g. bond_width, sphere_radius)
proc show_view_objects_win {} {
    global viewobj
#    global page_count
    global curr_page

    wm deiconify $viewobj($curr_page)
    raise $viewobj($curr_page)
}

##This procedure changes the visibility_mode from hide to show (set it up
# so that the object will be drawn)
proc change_to_show_mode {} {
  global pages
  global visibility_mode
#  global page_count
    global curr_page

  ##Change the show button to a hide button
  destroy $pages($curr_page).show_button($curr_page)
  button $pages($curr_page).hide_button($curr_page) -text "Hide" -command {change_to_hide_mode}
  pack $pages($curr_page).hide_button($curr_page) -side right -padx 3m -pady 2m -in $pages($curr_page).page_buttons
  ##Change visibility_mode to reflect our mode change
  set visibility_mode($curr_page) 1
}

##This procedure changes the visibility_mode from show to hide (set it up
# so that the object will not be drawn)
proc change_to_hide_mode {} {
  global pages
  global visibility_mode
#  global page_count
    global curr_page

  ##Change the hide button to a show button
  destroy $pages($curr_page).hide_button($curr_page)
  button $pages($curr_page).show_button($curr_page) -text "Show" -command {change_to_show_mode}
  pack $pages($curr_page).show_button($curr_page) -side right -padx 3m -pady 2m -in $pages($curr_page).page_buttons
  ##Change visibility_mode to reflect our mode change
  set visibility_mode($curr_page) 0
}

##This procedure changes the import_mode from spheres to bonds (set it up
# so that the nanotube's bonds will be drawn, rather than its atoms)
proc change_to_bond_mode {} {
  global viewobj
  global import_mode
#  global page_count
    global curr_page

  ##Change the bonds button to an atoms button
  destroy $viewobj($curr_page).sphere_label
  label $viewobj($curr_page).bond_label -text "Bonds"
  pack $viewobj($curr_page).bond_label -in $viewobj($curr_page).title
  destroy $viewobj($curr_page).bonds
  button $viewobj($curr_page).spheres -text "Atoms" -command {change_to_sphere_mode}
  pack $viewobj($curr_page).spheres -side right -in $viewobj($curr_page).main_buttons
  ##Change import_mode to reflect our mode change
  set import_mode($curr_page) 0
}

##This procedure changes the import_mode from bonds to spheres (set it up
# so that the nanotube's atoms will be drawn, rather than its bonds)
proc change_to_sphere_mode {} {
  global viewobj
  global import_mode
#  global page_count
    global curr_page

  ##Change the atoms button to a bonds button
  destroy $viewobj($curr_page).bond_label
  label $viewobj($curr_page).sphere_label -text "Atoms"
  pack $viewobj($curr_page).sphere_label -in $viewobj($curr_page).title
  destroy $viewobj($curr_page).spheres
  button $viewobj($curr_page).bonds -text "Bonds" -command {change_to_bond_mode}
  pack $viewobj($curr_page).bonds -side right -in $viewobj($curr_page).main_buttons
  ##Change import_mode to reflect our mode change
  set import_mode($curr_page) 1
}

####This procedure doesn't work completely--it deletes the page frame,
# but at least the tab is not deleted
#proc delete_object_proc {} {
#  global pages
#  global page_count
#  global viewobj
#  destroy $viewobj($page_count)
#  destroy $pages($page_count)
#}

##This procedure sets up a tabnotebook page, and its object's associated view window
proc import_proc {} {  
  global load_import_file
#  global delete_object
  global importobj
  global page_count
  global pages
  global viewobj
  global import_mode
  global visibility_mode
  global tab_label
  global curr_page

  ## We have a new page, so increment the page_count
  incr page_count
  set curr_page $page_count

  ## Assume that we initially show bonds -- if you change this, you must also change some code below,
  #  as well as code in MSIFile.C, MSIFile.h, imported_obj.C, and imported_obj.h
  set import_mode($page_count) 0
  ## Assume that the object should be initially visible
  set visibility_mode($page_count) 1
  ## Create a page of the tabnotebook 
  set pages($page_count) [$importobj(import_objects).sources.tabnote add \
	  -label "$tab_label" \
	  -command {global curr_page
                    set curr_page [expr [index select]+1]}]
  ##Create frame to hold object translation and rotation data
  frame $pages($page_count).pageframe -borderwidth 2 -relief raised
  pack $pages($page_count).pageframe -fill both -pady 4

  ##Create a frame to hold View and Close buttons
  frame $pages($page_count).page_buttons -relief groove -borderwidth 4
  pack $pages($page_count).page_buttons -side right -anchor e -fill y -in $pages($page_count).pageframe

  ##Create a button "View" -- causes view window to display, which contains
  # widgets for bond_width, sphere_radius, etc.
  button $pages($page_count).page_buttons.view($page_count) -text "View" -command {show_view_objects_win}
  pack $pages($page_count).page_buttons.view($page_count) -side top -padx 3m -pady 2m -in $pages($page_count).page_buttons

#  button $pages($page_count).page_buttons.delete_button($page_count) -text "Delete" -command {delete_object_proc}
#  pack $pages($page_count).page_buttons.delete_button($page_count) -side top -padx 3m -pady 2m -in $pages($page_count).page_buttons

  ##Set up view window for corresponding page and object
  set viewobj($page_count) .view_objects($page_count)
  toplevel $viewobj($page_count)
  wm withdraw $viewobj($page_count)
  ##Make a title frame which will say either "Bonds" or "Atoms" depending
  # on the import_mode
  frame $viewobj($page_count).title -relief groove -borderwidth 4
  pack $viewobj($page_count).title -side top -anchor n -fill x

  ##Make a frame which will hold the Bonds/Atoms button and the Hide button
  frame $viewobj($page_count).main_buttons -relief groove -borderwidth 4
  pack $viewobj($curr_page).main_buttons -side top -anchor n -fill x

  ##If we are in Bond mode, then show bond widgets, etc.
  if {$import_mode($page_count)==0} {
    label $viewobj($page_count).bond_label -text "Bonds"
    pack $viewobj($page_count).bond_label -in $viewobj($page_count).title
    button $viewobj($page_count).spheres -text "Atoms" -command {change_to_sphere_mode}
    pack $viewobj($page_count).spheres -side right -in $viewobj($page_count).main_buttons

  } else {
  ##we are in Atom mode, so show sphere widgets, etc.
    label $viewobj($page_count).sphere_label -text "Atoms"
    pack $viewobj($page_count).sphere_label -in $viewobj($page_count).title
    button $viewobj($page_count).bonds -text "Bonds" -command {change_to_sphere_mode}
    pack $viewobj($page_count).bonds -side right -in $viewobj($page_count).main_buttons
    #pack $viewobj($page_count).bonds -side right -anchor ne

  }
  ##Add Close button to close bond/atom view window
  button $viewobj($page_count).close -text "Close" -command {
    wm withdraw $viewobj($curr_page)
  }
  wm protocol $viewobj($page_count) WM_DELETE_WINDOW {$viewobj($page_count).close invoke}
  pack $viewobj($page_count).close -side left -in $viewobj($page_count).main_buttons
  
##make a frame for bond/sphere widgets
  frame $viewobj($page_count).viewframe -borderwidth 2 -relief raised
  pack $viewobj($page_count).viewframe -fill both -expand yes -pady 4
  ##if we are in Show mode, then we need a Hide button to switch to hide mode
  if {$visibility_mode($page_count)==1} {
    button $pages($page_count).hide_button($page_count) -text "Hide" -command {change_to_hide_mode}
    pack $pages($page_count).hide_button($page_count) -side right -padx 3m -pady 2m -in $pages($page_count).page_buttons

  } else {
  ##we are in Hide mode, and so need a Show button to switch to show mode
    button $pages($page_count).show_button($page_count) -text "Show" -command {change_to_show_mode}
    pack $pages($page_count).show_button($page_count) -side right -padx 3m -pady 2m -in $pages($page_count).page_buttons
  }
  ##View the newly created object's page
  $importobj(import_objects).sources.tabnote view "$tab_label"
  ##The load button was pressed, so set corresponding variable
  set load_import_file [expr $load_import_file+1]
#   set load_import_file [incr load_import_file]
}

##This is a procedure to show the tabnotebook window, which contains
# the translate and rotate sliders, etc. 
proc show_import_objects_win {} {
  global importobj
  wm deiconify $importobj(import_objects)
  raise $importobj(import_objects)
}

#make importobj window initially visible
#after idle show_import_objects_win


# Make a window to show the data that comes from a modification
set nmInfo(mod) .mod

# ----------------------------------------------------------------------
toplevel $nmInfo(mod)
wm withdraw $nmInfo(mod)

# Create a quit and save button
frame $nmInfo(mod).control -bg tan

button $nmInfo(mod).control.savebutton -text "Save"  -command mod_save
button $nmInfo(mod).control.close -text "Close" -command mod_quit
wm protocol $nmInfo(mod) WM_DELETE_WINDOW {$nmInfo(mod).control.close invoke}
pack $nmInfo(mod).control.savebutton $nmInfo(mod).control.close -side left -anchor nw

proc show_mod_win {} {
    global nmInfo
    wm deiconify $nmInfo(mod)
    raise $nmInfo(mod)
}



text .mod.text -relief raised -bg tan -fg black -bd 2 \
     -yscrollcommand ".mod.scroll set"
scrollbar .mod.scroll -command ".mod.text yview" -bg tan
pack .mod.scroll -side right -fill y

pack .mod.control -side top -fill x
pack .mod.text -side top -fill both 


# TCH 3 Sept 98 - tried specifying the toplevel as "-class Dialog",
#   added dbutton, button Cancel, ...  Lots of tricks cribbed from
#   russ_widgets.tcl

proc mod_save {} {
      global filename
      global dbutton

      toplevel .dialog -class Dialog

      frame .dialog.t
      pack .dialog.t -side top
      frame .dialog.b
      pack .dialog.b -side bottom

      label .dialog.t.label -text "Name:" 
      entry .dialog.t.entry -width 20 -relief sunken -bd 2 -textvariable \
            filename  
      pack .dialog.t.label .dialog.t.entry -side left -padx 1m -pady 2m

      button .dialog.b.setbutton -text "Set" -command "set dbutton Set"
      pack .dialog.b.setbutton -side left
      button .dialog.b.cancelbutton -text "Cancel" -command "set dbutton Cancel"
      pack .dialog.b.cancelbutton -side right

      bind .dialog.t <Return> ".dialog.b.setbutton invoke"
      bind .dialog.t.entry <Return> ".dialog.b.setbutton invoke"
      bind .dialog <Escape> ".dialog.b.cancelbutton invoke"
      bind .dialog.t <Escape> ".dialog.b.cancelbutton invoke"
      bind .dialog.t.entry <Escape> ".dialog.b.cancelbutton invoke"
      bind .dialog.b <Escape> ".dialog.b.cancelbutton invoke"

      # Focus input on the dialog box and wait for input
#      tkwait window .dialog
      set oldFocus [focus]
      focus .dialog.t.entry
      tkwait variable dbutton

      focus $oldFocus
      destroy .dialog

      if {$dbutton == "Cancel"} {
        # Cancel - do nothing
      } else {
        puts $filename
        set modfile [open $filename w]
        puts -nonewline $modfile [.mod.text get 1.0 end] 
        flush $modfile
        close $modfile
        exec unix_to_dos $filename
      }
}      


proc mod_quit {} {
    global modfile_hasWindow nmInfo
#    set modfile_hasWindow 0;
    wm withdraw $nmInfo(mod)
}
    

#/usr/local/contrib/moderated/bin/wishx -f

toplevel .cross_section
set cs .cross_section

set h1 $wi
set v1 $points([expr $wi - 1])
set h2 $wi
set v2 $points([expr $wi - 1])
set h 0.0
set v 0.0

# Create the text display that goes across the top
label $cs.hdisplay -text "H value(nm):" -bg white
label $cs.hvalue -textvariable h -bg white
label $cs.vdisplay -text "V value(nm):" -bg white
label $cs.vvalue -textvariable v -bg white

# Create a quit and save button
# TCH 5 May 98:  changed quitbutton to have -command quit instead of
#   binding {destroy .} to it after Seeger found that made the error
#   messages go away in Tcl 8.0
frame $cs.encl 
button $cs.encl.quitbutton -text "Quit" -bg red -command quitxsection
button $cs.encl.savebutton -text "Save" -bg red
pack $cs.encl.savebutton $cs.encl.quitbutton -side left

# Create a graph canvas to do the drawing in
canvas $cs.graph -width $wi -height $hi

# Pack the above widgets into the window
pack $cs.graph -side bottom
pack $cs.encl -side bottom -fill x
pack $cs.hdisplay $cs.hvalue $cs.vdisplay $cs.vvalue -side left -fill x

# Create the X and Y axis markers in the graph
$cs.graph create line 0 [expr $hi / 2] $wi [expr $hi / 2] -fill blue -width 5
$cs.graph create line 0 0 0 $hi -fill blue -width 5

# Create the two lines the user will slide to select points with
set firstmarker [$cs.graph create line  $wi 0 $wi $hi -fill blue -width 2]
set secondmarker [$cs.graph create line $wi 0 $wi $hi -fill red -width 2]

# Draw the graph from the points() array
set m [expr $hi / 2]	
for {set i 0} {$i < [expr $wi - 1]} {incr i 1} {
	set j [expr $i + 1] 
	$cs.graph create line \
		$i [expr $m - $points($i)] $j [expr $m - $points($j)] \
		-fill blue -width 1
}

proc move1Marker {x y} {
	global h1 v1 h2 v2 h v firstmarker points wi hi xscale yscale
    global cs
	if { ($x >= 0) && ($x < $wi) && ($y >=0) && ($y < $hi) } { 
	$cs.graph delete $firstmarker
	set firstmarker [$cs.graph create line $x 0 $x $hi -fill blue -width 2]
	set h1 $x
	set v1 $points($x)
	set h [expr [expr $h2 - $h1] * $xscale]
	set v [expr [expr $v2 - $v1] * $yscale]
	}
}

proc move2Marker {x y} {
	global h1 v1 h2 v2 h v secondmarker points wi hi xscale yscale
    global cs
	if { ($x >= 0) && ($x < $wi) && ($y >=0) && ($y < $hi) } { 
	$cs.graph delete $secondmarker
	set secondmarker [$cs.graph create line $x 0 $x $hi -fill red -width 2]
	set h2 $x
	set v2 $points($x)
	set h [expr [expr $h2 - $h1] * $xscale]
	set v [expr [expr $v2 - $v1] * $yscale]
	}
}

bind $cs.graph <Button1-Motion> {move1Marker %x %y}
bind $cs.graph <Button2-Motion> {move2Marker %x %y}

#bind $cs.encl.quitbutton <Button-1> {destroy $cs}

proc savefile {} {
	global points wi xscale yscale yoffset name
    global cs
	toplevel .dial 
	label .dial.label -text "Name:" -bg gray
	entry .dial.entry -width 20 -relief sunken -bd 2 -textvariable name \
		-bg gray
	pack .dial.label .dial.entry -side left -padx 1m -pady 2m
	bind .dial.entry <Return> {
		destroy .dial 
	}
	tkwait window .dial
	puts $name
	set file [open $name w]
	for {set i 0} {$i < $wi} {incr i 1} {
		puts -nonewline $file [expr $i * $xscale]
		puts -nonewline $file " "
		puts $file [expr $points($i) * $yscale + $yoffset]
	}
	close $file
}

proc quitxsection {} {
    global cs
  destroy $cs
}

bind $cs.encl.savebutton <Button-1> {savefile}


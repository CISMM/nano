global knobs
 
trace variable knobs w set_knobs

#determines space between knobs
set kspace 3m
#determines the vertical size of the knobs
set ksize 10m

# knob box simulator
frame $knbs -relief raised -bd 5 -bg grey50
frame $knbs.left -bg grey50
frame $knbs.right -bg grey50

#setup knobs
label $knbs.label -text "SGI Dials" -bg grey50
pack $knbs.label -side top
pack $knbs.left $knbs.right -side left -padx 1m -fill x

#left knobs
scale .knob6 -from 0 -to 100 -orient horizontal -command "set knobs(6)" -width $ksize -bg tan
scale .knob4 -from 0 -to 100 -orient horizontal -command "set knobs(4)"  -width $ksize -bg tan
scale .knob2 -from 0 -to 100 -orient horizontal -command "set knobs(2)"  -width $ksize -bg tan
scale .knob0 -from 0 -to 100 -orient horizontal -command "set knobs(0)"  -width $ksize -bg tan
#labels
label .knob6label -text "Img. Force" -bg grey50
label .knob4label -text "" -bg grey50
label .knob2label -text "Mod. Force" -bg grey50
label .knob0label -text "" -bg grey50
frame .knob4spacer -height $kspace -bg grey50
frame .knob2spacer -height $kspace -bg grey50
frame .knob0spacer -height $kspace -bg grey50

#right knobs
scale .knob7 -from 0 -to 100 -orient horizontal -command "set knobs(7)"  -width $ksize -bg tan
scale .knob5 -from 0 -to 100 -orient horizontal -command "set knobs(5)"  -width $ksize -bg tan
scale .knob3 -from 0 -to 100 -orient horizontal -command "set knobs(3)"  -width $ksize -bg tan
scale .knob1 -from 0 -to 100 -orient horizontal -command "set knobs(1)"  -width $ksize -bg tan

#labels
label .knob7label -text "" -bg grey50
label .knob5label -text "Friction" -bg grey50
label .knob3label -text "Replay Rate" -bg grey50
label .knob1label -text "Surf. Spring" -bg grey50
frame .knob5spacer -height $kspace -bg grey50
frame .knob3spacer -height $kspace -bg grey50
frame .knob1spacer -height $kspace -bg grey50

pack .knob6 .knob6label \
	.knob4spacer .knob4 .knob4label \
	.knob2spacer .knob2 .knob2label \
	.knob0spacer .knob0 .knob0label -in $knbs.left -side top
pack .knob7 .knob7label \
	.knob5spacer .knob5 .knob5label \
	.knob3spacer .knob3 .knob3label \
	.knob1spacer .knob1 .knob1label -in $knbs.right -side top

proc set_knobs {name index op} {
    if {$index != ""} {
	upvar ${name}($index) x
	.knob$index set $x
    }
}


global	i
global	j
global	k
global	x
global	y
global	z

frame .testit

scale	.testit.i -from 0 -to 100 -orient horizontal -showvalue 0 \
	-command "set i"
label	.testit.ishow -textvariable i

scale	.testit.j -from 0 -to 100 -orient horizontal -showvalue 0 \
	-command "set j"
label	.testit.jshow -textvariable j

scale	.testit.k -from 0 -to 100 -orient horizontal -showvalue 0 \
	-command "set k"
label	.testit.kshow -textvariable k

scale	.testit.x -from 0 -to 100 -orient horizontal -showvalue 0 \
	-command "set x"

scale	.testit.y -from 0 -to 100 -orient horizontal -showvalue 0 \
	-command "set y"

floatscale .testit.z 0 100 100 0 1 z

pack .testit.i .testit.ishow .testit.j .testit.jshow .testit.k .testit.kshow
pack .testit.x .testit.y .testit.z
pack .testit


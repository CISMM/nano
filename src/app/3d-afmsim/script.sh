#!/bin/csh -f

rm -f test.out

foreach p (`ls 3d_afm/dimers*.dat`)
	    ./3d_afm/Debug/3d_afm.exe -units nm -type -s $p 1 -unca_nano -tip_radius 11.0
end


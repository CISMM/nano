#!/bin/csh -f

rm -f test.out

foreach p (`ls 3d_afm/planechange*.dat`)
	    ./3d_afm/Debug/3d_afm.exe nm -s $p 1
end


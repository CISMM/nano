#!/bin/csh -f

rm -f test.out

foreach p (`ls dimers*.dat`)
	    ./3d_afm/Debug/3d_afm.exe nm -s $p 1
end


July 23, 1999		Aron Helser

This directory contains both the VI Curve server program for WinNT,
and the stand-alone VI Curve client program for unix (SGI). 

The client program can be make by running 
gmake
on an SGI. It will link some files from the
microscape/server_microscape and microscape/server_microscape/base
directories into this directory. 

To make the server program, you must make the client program at least
once, so it has a chance to link files into this directory. 

The server program can then be made by opening the vi_curve.dsw file
with Visual C++ 5.0 (or maybe 6.0) and issuing the build command. It
assumes that the VRPN library is available either two levels up
(i.e. in you personal ~/stm/src/`whoami`/vrpn directory) or three
levels up (i.e. in ~/stm/src/vrpn). Make sure VRPN is built for winNT
in one of these places.


Run the server program, Debug/vi_server.exe on the PC connected to the 
Keithley 2400 meter. It should bring up a window and start a VRPN
connection listening on port 4545. 

To connect to it with microscape, add the argument 
-div vi@argon:4545
where argon is the name of the PC (that's where it is right now...)

To connect with the stand-alone program, run 
vi_client -d vi@argon -o streamfile
-o will save a streamfile which can be replayed later with
vi_client -i streamfile

You may have to set NM_TCL_DIR to point to a directory with
keithley2400.tcl in it - it should be in the microscape/tcl
directory. 

This directory is for the separate ohmmeter application.
files:

Makefile - builds standalone ohmmeter program
client.C - driver code for Ohmmeter user interface class from ../ohmmeter.C
		also, sets up vrpn_Ohmmeter_Remote and log file stuff
		ohmmeter.C needs to be linked with Tcl_Linkvar.C

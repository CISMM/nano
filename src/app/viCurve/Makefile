USES_LIBS := nmAux tclLinkVar nmBase vrpn quat

EXCLUDE_SRC := GPIBServer.C vrpn_GPIBDeviceServer.C

#I have no idea why this following line is needed, but somehow it includes the
#correct versions of Tcl and Tk.  If we just put Tcl and Tk, things blow up.
USES_EXTERNAL_LIBS := RhinoIO

include ../Makefile.share


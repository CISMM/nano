USES_LIBS := nmMP nmBase nmMScope nmUI tclLinkVar nmUGraphics vrpn \
             vogl quat nmAnalyze

USES_EXTERNAL_LIBS := RhinoIO glut

NAUGHTY := 1

EXCLUDE_SRC := nmg_GraphicsNull.C

ifdef VC
EXCLUDE_SRC += export_scene.C
endif

include ../Makefile.share

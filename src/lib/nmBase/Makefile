USES_LIBS := nmMP vrpn quat

USES_EXTERNAL_LIBS := magick

EXCLUDE_SRC := tnv.t.C
ifndef VC
# Only include vc_dirent.c if we are compiling on VC++
EXCLUDE_SRC += vc_dirent.c
endif

include ../Makefile.share

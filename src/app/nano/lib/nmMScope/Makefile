USES_LIBS := nmBase tclLinkVar nmAux vrpn vogl quat sdi

ifdef VC
EXCLUDE_SRC = MicroscopeIO.C NetworkedMicroscopeChannel.C stm_file.c
endif

EXCLUDE_SRC = $(EXCLUE_SRC) Microscope.C MicroscopeIO.C

NAUGHTY := 1

include ../Makefile.share

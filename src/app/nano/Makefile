USES_LIBS := nmAux nmUGraphics nmSEM nmUI nmGraphics nmMScope \
             nmRobotControl nmReg nmImageViewer \
             nmMP tclLinkVar nmBase ImgFormat \
             vrpn vogl quat

EXCLUDE_SRC = renderer.C server_talk.c test_speech.c \
  collaboration.C

ifdef VC
EXCLUDE_SRC += termio.c x_util.c
USES_LIBS += vrpn_phantom
endif

ifeq ($(HW_OS),sgi_irix)
USES_LIBS += sdi
endif

ifdef NANO_WITH_ROBOT
   EXTRA_CFLAGS += -DNANO_WITH_ROBOT
endif

include ../Makefile.share

.PHONY: install
install:
	cp $(APP_EXEC) $(NANO_ROOT)/bin
	cp -R -u -f $(TOP_LEVEL_SRC_DIR)/share/tcl $(NANO_ROOT)/share
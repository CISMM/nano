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

ifdef VIEWER
NO_MSCOPE_CONNECTION := 1
endif

# new name for a viewer-only app.
ifdef NO_MSCOPE_CONNECTION
ifdef VC
   APP_NAME := nano_view.exe
else
   APP_NAME := nano_view
endif
endif

include ../Makefile.share

.PHONY: install
install:
	cp $(APP_EXEC) $(NANO_ROOT)/bin
#	cp -R -u -f $(TOP_LEVEL_SRC_DIR)/share/tcl $(NANO_ROOT)/share
ifdef NO_MSCOPE_CONNECTION
	cp -R -u -f $(TOP_LEVEL_SRC_DIR)/share/tcl/images $(NANO_ROOT)/share/tcl_view
	$(TOP_LEVEL_SRC_DIR)/util/tcl_filters/strip_unc.tcl \
	$(TOP_LEVEL_SRC_DIR)/share/tcl $(NANO_ROOT)/share/tcl_view \
	thirdtech_ui viewer_only
else
	cp -R -u -f $(TOP_LEVEL_SRC_DIR)/share/tcl/images $(NANO_ROOT)/share/tcl
	$(TOP_LEVEL_SRC_DIR)/util/tcl_filters/strip_unc.tcl \
	$(TOP_LEVEL_SRC_DIR)/share/tcl $(NANO_ROOT)/share/tcl \
	thirdtech_ui
endif

#Which obj files differentiate the full version from the viewer-only version?
.PHONY: clean_view
clean_view:
ifdef VC
	rm $(OBJECT_DIR)/microscape.obj $(OBJECT_DIR)/tcl_tk.obj
else 
	rm $(OBJECT_DIR)/microscape.o $(OBJECT_DIR)/tcl_tk.o
endif

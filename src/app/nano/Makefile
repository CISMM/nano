USES_LIBS := nmAux nmUGraphics nmSEM nmUI nmGraphics nmMScope \
             nmRobotControl nmReg nmImageViewer nmTipConvolution \
             nmMP tclLinkVar nmBase ImgFormat nmAnalyze \
             vrpn vogl quat

EXCLUDE_SRC = renderer.C server_talk.c test_speech.c \
  collaboration.C

ifdef VC
EXCLUDE_SRC += termio.c x_util.c
# vrpn_phantom isn't really external but we specify the path explicitly
# as being in vrpn/server_src/pc_win32_MTd unlike we do for other internal libs
USES_EXTERNAL_LIBS := vrpn_phantom ghost30
else
USES_EXTERNAL_LIBS := RhinoIO
endif

ifdef NANO_WITH_ROBOT
   EXTRA_CFLAGS += -DNANO_WITH_ROBOT 
endif

# new name for a viewer-only app.
ifdef VIEWER
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
ifdef VIEWER
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

### Put this at the END of the Makefile
include Makefile.additional-targets

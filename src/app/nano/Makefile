USES_LIBS := nmAux nmUGraphics nmSEM nmUI nmGraphics nmMScope \
             nmRobotControl nmReg nmImageViewer nmTipConvolution \
             nmMP tclLinkVar nmBase ImgFormat \
             vrpn vogl quat

EXCLUDE_SRC = renderer.C server_talk.c test_speech.c \
  collaboration.C

ifdef VC
EXCLUDE_SRC += termio.c x_util.c
# vrpn_phantom isn't really external but we specify the path explicitly
# as being in vrpn/server_src/pc_win32_MTd unlike we do for other internal libs
USES_EXTERNAL_LIBS := ghost30 vrpn_phantom
else
USES_EXTERNAL_LIBS := RhinoIO
endif

ifdef NANO_WITH_ROBOT
   EXTRA_CFLAGS += -DNANO_WITH_ROBOT 
endif

include ../Makefile.share

### Put this at the END of the Makefile
include Makefile.additional-targets

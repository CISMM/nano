USES_LIBS := nmAux nmUGraphics nmSEM nmUI nmGraphics nmGenTex nmMScope \
             nmRobotControl nmReg nmImageViewer \
             nmMP tclLinkVar nmBase ImgFormat \
             vrpn vogl quat

EXCLUDE_SRC = renderer.C server_talk.c test_speech.c \
  collaboration.C

ifdef VC
EXCLUDE_SRC += termio.c x_util.c
endif

ifdef NANO_WITH_ROBOT
   EXTRA_CFLAGS += -DNANO_WITH_ROBOT
endif

include ../Makefile.share

USES_LIBS := nmAux nmUGraphics nmSEM nmUI nmGraphics nmGenTex nmMScope \
             nmRobotControl nmReg \
             nmMP tclLinkVar nmBase ImgFormat \
             vrpn vogl quat sdi

EXCLUDE_SRC = renderer.C server_talk.c test_speech.c

ifdef NANO_WITH_ROBOT
   EXTRA_CFLAGS += -DNANO_WITH_ROBOT
endif

include ../Makefile.share

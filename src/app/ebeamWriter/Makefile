USES_LIBS := ImgFormat nmReg nmSEM nmImageViewer tclLinkVar nmBase vrpn ImgFormat quat

ifdef VC
  USES_EXTERNAL_LIBS := tcl82 tk82 glut_UNC 
else
  USES_EXTERNAL_LIBS := tcl tk glut_UNC
  EXTRA_CFLAGS := -DVIRTUAL_SEM
endif 

APP_NAME=seegerizer

include ../Makefile.share

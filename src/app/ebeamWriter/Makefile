USES_LIBS := ImgFormat nmReg nmSEM nmMP nmImageViewer tclLinkVar nmBase vrpn ImgFormat quat

ifdef VC
  USES_EXTERNAL_LIBS := tcl83 tk83 glut_UNC 
else
  USES_EXTERNAL_LIBS := tcl8.3 tk8.3 glut_UNC
  EXTRA_CFLAGS := -DVIRTUAL_SEM -I/afs/cs.unc.edu/project/stm/src/seeger/nano/src/app/sem
endif 

APP_NAME=seegerizer

include ../Makefile.share

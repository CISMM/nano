USES_LIBS := nmMScope nmBase vrpn ImgFormat

ifeq ($(MAKECMDGOALS),afmcntsim)
   EXTRA_CFLAGS+=-DAFMSIM
endif

# How do we turn on debugging on PC???
#EXTRA_CFLAGS+=-g

#ifeq ($(HW_OS),sgi_irix)
LATE_INCLUDE_PATH := -I/afs/unc/proj/stm/src/external/glut/glut-3.5/include 
LATE_LOAD_PATH := -L/afs/unc/proj/stm/src/external/glut/glut-3.5/lib/glut.n32 
#endif


#ifeq ($(HW_OS),sgi_irix)
USES_EXTERNAL_LIBS := tcl
#else
USES_EXTERNAL_LIBS := tcl82
#endif




include ../Makefile.share

ifeq ($(MAKELEVEL),0)

default: surface

.PHONY: surface
surface: $(OBJECT_DIR)/surface

$(OBJECT_DIR)/surface:
	$(MAKE) APP_NAME=surface \
           MAKECMDGOALS="$(filter-out surface,$(MAKECMDGOALS))" \
           EXCLUDE_SRC="warrencntsim.C vec2d.C"

# For pc_win32, the above needs to be
#	$(MAKE) APP_NAME=surface.exe \

.PHONY: afmcntsim
afmcntsim: $(OBJECT_DIR)/afmcntsim

$(OBJECT_DIR)/afmcntsim:
	$(MAKE) APP_NAME=afmcntsim \
           MAKECMDGOALS="$(filter-out afmcntsim,$(MAKECMDGOALS))" \
           EXCLUDE_SRC=surface.C
endif


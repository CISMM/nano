USES_LIBS := nmMScope nmBase vrpn

ifeq ($(MAKECMDGOALS),afmcntsim)
   EXTRA_CFLAGS+=-DAFMSIM
endif

#ifeq ($(HW_OS),sgi_irix)
LATE_INCLUDE_PATH := -I/afs/unc/proj/stm/src/external/glut-3.5/include 
LATE_LOAD_PATH := -L/afs/unc/proj/stm/src/external/glut-3.5/lib/glut.n32 
#endif

# We're not going to get glui there, but we need to specify SOMETHING
# so it looks there for Tcl

USES_EXTERNAL_LIBS := tcl



include ../Makefile.share

ifeq ($(MAKELEVEL),0)

default: surface

.PHONY: surface
surface: $(OBJECT_DIR)/surface

$(OBJECT_DIR)/surface:
	$(MAKE) APP_NAME=surface \
           MAKECMDGOALS="$(filter-out surface,$(MAKECMDGOALS))" \
           EXCLUDE_SRC="warrencntsim.C vec2d.C"

.PHONY: afmcntsim
afmcntsim: $(OBJECT_DIR)/afmcntsim

$(OBJECT_DIR)/afmcntsim:
	$(MAKE) APP_NAME=afmcntsim \
           MAKECMDGOALS="$(filter-out afmcntsim,$(MAKECMDGOALS))" \
           EXCLUDE_SRC=surface.C
endif


USES_LIBS := nmMScope nmBase vrpn

ifeq ($(MAKECMDGOALS),afmcntsim)
   EXTRA_CFLAGS+=-DAFMSIM
endif

ifeq ($(HW_OS),sgi_irix)
LATE_INCLUDE_PATH := -I/afs/unc/proj/walk/include
LATE_LOAD_PATH := -L/afs/unc/proj/walk/lib/$(HW_OS).$(SGI_ABI)
endif
#LATE_LIBRARIES := -lglut

include ../Makefile.share

ifeq ($(MAKELEVEL),0)
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


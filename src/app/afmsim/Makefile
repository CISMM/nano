USES_LIBS := nmMScope nmBase vrpn ImgFormat nmMP

USES_EXTERNAL_LIBS := RhinoIO

ifeq ($(MAKECMDGOALS),afmcntsim)
   EXTRA_CFLAGS+=-DAFMSIM
endif

include ../Makefile.share

ifeq ($(MAKELEVEL),0)

default: surface

.PHONY: surface
surface: $(OBJECT_DIR)/surface

$(OBJECT_DIR)/surface:
	$(MAKE) APP_NAME=surface \
           MAKECMDGOALS="$(filter-out surface,$(MAKECMDGOALS))" \
           EXCLUDE_SRC="warrencntsim.C vec2d.C"

# For pc_win32
#	$(MAKE) APP_NAME=surface.exe \

.PHONY: afmcntsim
afmcntsim: $(OBJECT_DIR)/afmcntsim

$(OBJECT_DIR)/afmcntsim:
	$(MAKE) APP_NAME=afmcntsim \
           MAKECMDGOALS="$(filter-out afmcntsim,$(MAKECMDGOALS))" \
           EXCLUDE_SRC=surface.C
endif


USES_LIBS := nmMScope nmBase vrpn

ifeq ($(MAKECMDGOALS),afmcntsim)
   EXTRA_CFLAGS+=-DAFMSIM
endif

include ../Makefile.share

ifeq ($(MAKELEVEL),0)
.PHONY: surface
surface: $(OBJECT_DIR)/surface

$(OBJECT_DIR)/surface:
	$(MAKE) APP_NAME=surface \
            MAKECMDGOALS="" \
            EXCLUDE_SRC="warrencntsim.C vec2d.C"

.PHONY: afmcntsim
afmcntsim: $(OBJECT_DIR)/afmcntsim

$(OBJECT_DIR)/afmcntsim:
	$(MAKE) APP_NAME=afmcntsim \
                EXCLUDE_SRC=surface.C \
                MAKECMDGOALS=""
endif

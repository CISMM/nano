USES_LIBS := nmMScope nmBase vrpn

ifeq ($(MAKECMDGOALS),afmcntsim)
   EXTRA_CFLAGS+=-DAFMSIM
endif

include ../Makefile.share

surface:
	$(MAKE) APP_NAME=surface \
                EXCLUDE_SRC="warrencntsim.C vec2d.C" \
                MAKECMDGOALS=""

afmcntsim:
	$(MAKE) APP_NAME=afmcntsim \
                EXCLUDE_SRC=surface.C \
                MAKECMDGOALS=""

all:
	$(MAKE) surface
	$(MAKE) afmcntsim

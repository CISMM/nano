USES_LIBS := nmBase vrpn

include ../Makefile.share

deline_filter:
	$(MAKE) APP_NAME=deline_filter \
                EXCLUDE_SRC="slope_filter.c sqrt_filter.c" \
                MAKECMDGOALS=""

slope_filter:
	$(MAKE) APP_NAME=slope_filter \
                EXCLUDE_SRC="deline_filter.c sqrt_filter.c" \
                MAKECMDGOALS=""

sqrt_filter:
	$(MAKE) APP_NAME=sqrt_filter \
                EXCLUDE_SRC="deline_filter.c slope_filter.c" \
                MAKECMDGOALS=""

all:
	$(MAKE) deline_filter slope_filter sqrt_filter

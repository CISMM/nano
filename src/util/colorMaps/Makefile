include ../Makefile.share

rainbow:
	$(MAKE) APP_NAME=rainbow \
                EXCLUDE_SRC="invrainbow.C random.C test_colormap.C throughgray.C" \
                MAKECMDGOALS=""

random:
	$(MAKE) APP_NAME=random \
                EXCLUDE_SRC="rainbow.C invrainbow.C test_colormap.C throughgray.C" \
                MAKECMDGOALS=""

invrainbow:
	$(MAKE) APP_NAME=invrainbow \
                EXCLUDE_SRC="rainbow.C random.C test_colormap.C throughgray.C" \
                MAKECMDGOALS=""

throughgray:
	$(MAKE) APP_NAME=throughgray \
                EXCLUDE_SRC="rainbow.C invrainbow.C random.C test_colormap.C" \
                MAKECMDGOALS=""

all:
	$(MAKE) rainbow random invrainbow throughgray

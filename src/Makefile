### first attempt at a top-level makefile.
### cd to nano/src, and type "make"

# we want /contrib/bin/etags in cygwin-1.0
TAGS_CMD=etags
TAGS_FILENAME=TAGS
TAGS_FLAGS=-C -I -a
TAGS_CMD_LINE=xargs -0 -e ${TAGS_CMD} ${TAGS_FLAGS}

########################################################
# We need to generate in the following directories:
# this should probably be moved into app/nano/Makefile
#

GENERATE_DIRS := app/nano/lib/nmMScope/
NANO-ALL-DIRS := lib/ app/nano/lib/ app/nano/
NANO-APP-DIRS := app/nano/lib/ app/nano/
INSTALL-DIRS  := app/nano/

.PHONY: nano-all nano-app

#
# Set the default target, and define building the app only
#

nano-all: generate $(addsuffix .make,$(NANO-ALL-DIRS))

nano-app: generate $(addsuffix .make,$(NANO-APP-DIRS))

#
# Default clean, depend, and generate for app/nano
#

.PHONY: install clean clean_view generate depend

install:  $(addsuffix .install,$(INSTALL_DIRS))

clean:    $(addsuffix .clean,$(NANO-ALL-DIRS))

depend: generate $(addsuffix .depend,$(NANO-ALL-DIRS))

generate: $(addsuffix .generate,$(GENERATE_DIRS))

clean_view:
	$(MAKE) -C app/nano clean_view

#
# Implicit rules for generating and cleaning
#

.PHONY: %.generate %.make %.depend %.clean %.install

%.make: %
	$(MAKE) -wC $< 

%.generate: %
	$(MAKE) -wC $< generate

%.clean: %
	$(MAKE) -wC $< clean

%.install: %
	$(MAKE) -wC $< install

%.depend: %
	$(MAKE) -wC $< depend

### buid an emacs tags table.  The files will be in the table in the
### order they are given to etags.  We want .h files to come first.
### I'm doing this as a phony target so that the TAGS table is always
### rebuilt if one types "make TAGS".
.PHONY: TAGS
TAGS:
	${RM} ${TAGS_FILENAME}
	-find lib      -type f -name '*.h' -print0    | ${TAGS_CMD_LINE}
	-find app/nano -type f -name '*.h' -print0    | ${TAGS_CMD_LINE}
	-find lib      -type f -name '*.[Cc]' -print0 | ${TAGS_CMD_LINE}
	-find app/nano -type f -name '*.[Cc]' -print0 | ${TAGS_CMD_LINE}
	-find app/nano -type f -name '*.cc' -print0   | ${TAGS_CMD_LINE}
	-# not needed:
	-#find lib      -type f -name '*.cc' -print0   | ${TAGS_CMD_LINE}
	-#find lib      -type f -name '*.cpp' -print0  | ${TAGS_CMD_LINE}
	-#find app/nano -type f -name '*.cpp' -print0  | ${TAGS_CMD_LINE}


### first attempt at a top-level makefile.
### cd to nano/src, and type "make"

# we want /contrib/bin/etags in cygwin-1.0
TAGS_CMD=etags
TAGS_FILENAME=TAGS
TAGS_FLAGS=-C -I -a
TAGS_CMD_LINE=xargs -0 -e ${TAGS_CMD} ${TAGS_FLAGS}
.PHONY: nano-all lib app/nano/lib app/nano install clean clean_view

nano-all: lib app/nano/lib app/nano
nano-app: app/nano/lib app/nano

lib:
	$(MAKE) -C lib

app/nano: app/nano/lib
	$(MAKE) -C app/nano


### this should probably be moved into app/nano/Makefile
app/nano/lib:
	$(MAKE) -C app/nano/lib

install:
	$(MAKE) -C app/nano install

clean:
	$(MAKE) -C lib clean
	$(MAKE) -C app/nano/lib clean
	$(MAKE) -C app/nano clean

clean_view:
	$(MAKE) -C app/nano clean_view

.PHONY: depend
depend:
	$(MAKE) -C lib depend
	$(MAKE) -C app/nano/lib depend
	$(MAKE) -C app/nano depend

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


### first attempt at a top-level makefile.
### cd to nano/src, and type "make"

.PHONY: all lib app/nano/lib app/nano install clean clean_view

all: lib app/nano/lib app/nano

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
	$(MAKE) -C app/nano clean
	$(MAKE) -C app/nano/lib clean

clean_view:
	$(MAKE) -C app/nano clean_view

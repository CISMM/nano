# Find out what platform to compile on 

MAKEFILE := Makefile
MAKE := gmake -f $(MAKEFILE)
HW_OS := $(shell /usr/local/bin/hw_os)

CC = gcc
CFLAGS = -g -I/net/nano/nano3/tcltk.sgi_irix/include -I/usr/Image/6.1/include -ICIMAGE/include

ifeq ($(HW_OS),sgi_irix)
CIMAGE_LIB   = -LCIMAGE/lib/sgi-mips -lcimage

LFLAGS = -L/usr/local/lib -L/usr/Image/6.1/lib /-L/usr/lib/X11R5 -L/net/nano/nano3/tcltk.$(HW_OS)/lib

ifeq ($(HW_OS),sgi_irix)
# force tcl libraries to load statically on SGI
  TCL_LIBS = -B static -litk -litcl -ltk -ltcl -B dynamic
endif

ifeq ($(HW_OS),pc_linux)
# force tcl libraries to load statically on linux
  TCL_LIBS = -B static -litk -litcl -ltk -ltcl -B dynamic
endif

ifeq ($(HW_OS), hp700_hpux10)
# force tcl libraries to load statically on HP
  TCL_LIBS =  -l:libitk.a -l:libitcl.a -l:libtk.a -l:libtcl.a
endif

UI_LIBS = -lmaxima -limg -lio -lhdr -lbuf -ltype -lgpf -lappl
LIBS = $(TCL_LIBS) $(UI_LIBS) -lX11 -lm

#-lXext 

APPS = test_Linkvar

.c.o:
	$(CC) $(CFLAGS) -c $*.c

.C.o:
	$(CC) $(CFLAGS) -c $*.C

all:
	$(MAKE) $(APPS)

clean:
	\rm -f *.o $(APPS)

#tk_test: tk_test.o tcl_tk.o
#	\rm -f tk_test
#	$(CC) $(LFLAGS) -o tk_test tk_test.o tcl_tk.o $(LIBS)

test_Linkvar: test_Linkvar.o Tcl_Linkvar.o
	\rm -rf test_Linkvar
	$(CC) $(LFLAGS) -o test_Linkvar test_Linkvar.o Tcl_Linkvar.o $(LIBS)

Tcl_Linkvar.o: Tcl_Linkvar.C Tcl_Linkvar.h
test_Linkvar.o: test_Linkvar.C Tcl_Linkvar.h


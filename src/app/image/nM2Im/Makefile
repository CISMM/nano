# General-purpose make file for any architecture

# Include file for make - do not change.
include /usr/Image/5.0/lib/make.dat

# Everything below is subject to change by the user.

# X11 files.  These probably should not be changed.
MSDIR = ../..
MTFDIR = /usr/include/Motif1.2
MODDIR = /usr/local/contrib/moderated/include
XINCLDS = -I/usr/local/include/X11R5 \
          -I/usr/include/X11R5 \
          -I/usr/include/X11R5/X11 \
          -I/usr/include/X11
XLIBDIR = /usr/local/lib/X11R5

# My files.
MYDIR	= .

# source files
SRC	= micro2Im.c
# microscape source files
MSSRC   = BCGrid.C \
          BCPlane.C \
          readTopometrixFile.C \
          BCDebug.C \
          BCString.C
# header files
HDRS	= $(SRC:.c=.h)
MSHDRS  = $(MSSRC:.C=.h)
# object files
OBJS	= $(SRC:.c=.o)
MSOBJS  = $(MSSRC:.C=.o)

# Executable to be built.
B_FILE	= bin/micro2Im

# Miscellaneous macros
FLAGS	=	-O -Wall
INCL    =       -I$(MSDIR) -I$(INCLDIR) -I$(MTFDIR) -I$(MODDIR) $(XINCLDS) -I$(MYDIR)
.SUFFIXES:	.o .cc .C .c .y .l

# Target definitions

all:	$(B_FILE)

BCDebug.C:
	ln -s $(MSDIR)/BCDebug.C BCDebug.C

BCString.C:
	ln -s $(MSDIR)/BCString.C BCString.C

readTopometrixFile.C:
	ln -s $(MSDIR)/readTopometrixFile.C readTopometrixFile.C

BCPlane.C:
	ln -s $(MSDIR)/BCPlane.C BCPlane.C

BCGrid.C:
	ln -s $(MSDIR)/BCGrid.C BCGrid.C

.c.o:
	echo "compiling $*.c:"
	$(RM) $*.o
	$(C+) -c $(FLAGS) $(CCFLAGS) $(INCL) $*.c

.C.o:
	echo "compiling $*.C:"
	$(RM) $*.o
	$(C+) -c $(FLAGS) $(CCFLAGS) $(INCL) $*.C

$(B_FILE):	$(OBJS) $(MSOBJS)
	$(RM) $(B_FILE)
	$(C+) -o $(B_FILE) $(OBJS) $(MSOBJS) -L$(LIBDIR) -lmaxima -loldmagic -lX11 $(LIBS) $(SYSTEMLIBS)

clean:
	$(RM)	*.o

cleanall:
	$(RM)	*.o $(B_FILE)


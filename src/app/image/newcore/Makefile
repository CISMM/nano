
#=========================================================================
# - Makefile.sgi
# Makefile for core.sgi-mips
# Written by Mark Foskey 10/9/98
# Based on usr/Image/{6.0/}doc/Makefile2, which describes itself as follows:

# General-purpose make file for building a single program on any architecture

#=========================================================================

# Include file for make.  If this is ported to Solaris, change lib to glib.

include /usr/Image/6.0/lib/make.dat

#-------------------------------------------------------------------------
# Information for compilation stage.

HW_OS := $(shell /usr/local/bin/hw_os)

# Executable to be built.  Used for both compilation and linking.
M_FILE	    = 	coreview	# - M_FILE because Motif-based

# - Object files to be built.  The first six files listed have their
# sources in ../.. and are used by other nano programs.

OBJS	    =	coreview.o core_stimulate_at_point.o CoreViewImage.o \
		optimize_boundary.o

# - Include files and flags.  The three ending in "DIR" are included by
# the file make.dat above, with machine dependencies (hopefully)
# handled.

CIMAGE_INCL  =	CIMAGE/include
INCL	     =	-I$(INCLDIR) -I$(XINCLDIR) \
		-I$(MINCLDIR) -I$(CIMAGE_INCL)

# - General compiler flags.
# An earlier make file contained this list:
# -DXTFUNCPROTO -O4 -DNO_ANINT -DSYSV -DNO_VFORK -DUNC          
# However, I believe only the following flags are needed.

CCFLAGS	     =	-O4 -DSYSV 

#-------------------------------------------------------------------------
# Suffixes and target definitions for compilation

.SUFFIXES:	.o .cc .C .c .y .l

# Target definitions

all:	$(M_FILE)

# General definitions for remaining object files.

.C.o:
	echo "compiling $*.C:"
	$(RM) $*.o
	$(C+) -c $(CCFLAGS) $(INCL) $*.C

#-------------------------------------------------------------------------
# Information for linking stage.

# - Library paths.  
IMAGE_LIB    = -L/usr/image/lib -lim   # old usr-image
CIMAGE_LIB   = -LCIMAGE/lib/`mach` -lcimage
X_LIB        = -lXm -lXt -lX11 -lXext # includes motif
MISC_LIB     = -lm # Everything else

#-------------------------------------------------------------------------
# Link command.

# Any of the variables not defined above are defined in
# /usr/Image/6.0/lib/make.dat.  The usr/Image template specifies
# -L$(MLIBDIR) before linking to the Motif directories, with MLIBDIR =
# usr/local/lib/X11R5.  However, linking on this program fails with
# anything higher than R4.

$(M_FILE):	$(OBJS)
	$(C+) -o $(M_FILE) $(OBJS) -L$(LIBDIR) $(LIBS) \
	$(CIMAGE_LIB) $(IMAGE_LIB) $(X_LIB) $(MISC_LIB) 

clean:
	$(RM)	$(OBJS)

cleanall:
	$(RM)	$(OBJS) $(M_FILE)


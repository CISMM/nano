# This was originally called Makefile.sgi.  However, this program now
# compiles only on SGIs, so I have made this the primary makefile and
# removed the other one.
# Mark Foskey
# 6/99

#
# Generic makefile to compile cimage applications - needs some modifications
# bu users
# Aziz Boxwala 17th October 1994
#

# -----------------------------------------------------------------------
TOP          = /usr/Image/Delta
COMPLEX      =
#	The complex library (needed by AT&T-based C++ compilers)

# -----------------------------------------------------------------------
##########
# INCLUDE PATHS
##########
CIMAGE_INCL  = -I../newcore/CIMAGE/include
X_INCL       = -I/usr/include/X11
HP_INCS  = -I/usr/include/Motif1.2 -I/usr/include/X11R5 -I/usr/include/X11R5/X11 -I/usr/local/contrib/moderated/include

# -----------------------------------------------------------------------
##########
# LIBRARY PATHS
##########

######
# These libs must be linked to
######
COMPLEX	=	-lcomplex
IMAGE_LIB    = -L/usr/image/lib -L/usr/image/hp/lib -lim   # usr-image
CIMAGE_LIB   = -L../newcore/CIMAGE/lib/sgi-mips -lcimage
HP_LIBS      = -L/usr/lib/Motif1.2 -L/usr/lib/X11R5
######
# Application-specific linking
######
X_LIB        = -lXm -lXt -lX11 -lXext # includes motif
MISC_LIB     = -lm # Everything else

# -----------------------------------------------------------------------
CFLAGS       = -DXTFUNCPROTO -O2 -DNDEBUG -DNO_ANINT -DSYSV -DNO_VFORK -DUNC

all:
	make nanoRidgesgi

#####
# MODIFY next line for your application
####

OBJS         =	lctrace.o russell.o BCGrid.o BCPlane.o readTopometrixFile.o \
		BCDebug.o BCString.o Topo.o

BCDebug.c: ../../BCDebug.C
	-ln -s ../../BCDebug.C BCDebug.c

BCGrid.c: ../../BCGrid.C
	-ln -s ../../BCGrid.C BCGrid.c

BCPlane.c: ../../BCPlane.C
	-ln -s ../../BCPlane.C BCPlane.c

BCString.c: ../../BCString.C
	-ln -s ../../BCString.C BCString.c

readNanoscopeFile.c: ../../readNanoscopeFile.C
	-ln -s ../../readNanoscopeFile.C readNanoscopeFile.c

readTopometrixFile.c: ../../readTopometrixFile.C
	-ln -s ../../readTopometrixFile.C readTopometrixFile.c

Topo.c: ../../Topo.C
	-ln -s ../../Topo.C Topo.c

#####
# MODIFY next line for your application
####
BINARY       = nanoRidgesgi

CC           = g++
C+           = g++

# -----------------------------------------------------------------------
$(BINARY):  $(OBJS)
	rm -f $(BINARY)
	$(C+) $(CFLAGS) -o $(BINARY) $(OBJS) $(CIMAGE_LIB) $(IMAGE_LIB) \
	$(DIAL_LIB) $(HP_LIBS) $(X_LIB) $(MISC_LIB)

$(OBJS):
	$(CC) $(CFLAGS) $(CIMAGE_INCL) $(HP_INCS) $(X_INCL) -I../../ -c $*.c
clean:
	/bin/rm -f $(OBJS) $(BINARY)

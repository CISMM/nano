
# Simple make file for building a non-display program on any architecture


# Include file for make - do not change, except as follows.
# To use g++ on Solaris machines, change lib to glib.
include /usr/Image/6.0/lib/make.dat


# Everything below is subject to change by the user.
# In most cases, however, only the definitions of OBJS,
# B_FILE, and FLAGS will need to be changed.


# Object files to be built.
OBJS	=	corescribe.o draw_core_list.o 

# Executable to be built.
B_FILE	=	corescribe

# Miscellaneous macros
FLAGS	=	-O
L_FLAGS	=	# Link flags
INCL	=	-I$(INCLDIR) -I$(XINCLDIR)
.SUFFIXES:	.o .cc .C .c .y .l


# Target definitions

all:	$(B_FILE)

.C.o:
	echo "compiling $*.C:"
	$(RM) $*.o
	$(C+) -c $(FLAGS) $(CCFLAGS) $(INCL) $*.C

$(B_FILE):	$(OBJS)
	$(C+) -o $(L_FLAGS) $(B_FILE) $(OBJS) $(LDFLAGS) \
		-L$(LIBDIR) -lcores $(LIBS) $(SYSTEMLIBS)

clean:
	$(RM)	$(OBJS)

cleanall:
	$(RM)	$(OBJS) $(B_FILE)


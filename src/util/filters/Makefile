#
# common definitions
#

MAKE = make
makefile = Makefile
HW_OS = `/usr/local/bin/hw_os`

DEFAULT_CC = g++

#
# Applications that are here
#

APPS = sqrt_filter deline_filter

# subdirectories for make
OBJECT_DIR 	= $(HW_OS)

# flags
#
# {INCLUDE,LD}_FLAGS and LIBS are only defined for recursive pass
#
DEFINES		= 
LINT_FLAGS 	= $(INCLUDE_FLAGS)
CFLAGS 		= $(INCLUDE_FLAGS) $(DEFINES) -g

INCLUDE_FLAGS = -I. -I.. -I/usr/local/contrib/moderated/include
LIBS = -lm
LDFLAGS     = 

#############################################################################
#
# implicit rule for all .c files
#
.SUFFIXES:	.c .C .o .a

.c.o:
	$(CC) -c $(CFLAGS) $<
.C.o:
	$(CC) -c $(CFLAGS) $<
#
#
#############################################################################

all:
	$(MAKE) $(APPS)


#############################################################################
#
# sqrt_filter code
#
#############################################################################

#
# sqrt_filter
#
SQRT_FILES = sqrt_filter.c
SQRT_CC_FILES = ""
SQRT_OBJECTS = $(SQRT_CC_FILES:.C=.o) $(SQRT_FILES:.c=.o)
SQRT_INCLUDES = Makefile

sqrt_filter$(SUFFIX) :
	@$(MAKE) "EXE_NAME = sqrt_filter" \
		"TARGET_INCLUDES = $(SQRT_INCLUDES)" \
		"TARGET_OBJECTS = $(SQRT_OBJECTS)" \
		"PREFIX = " \
		"SUFFIX=space_holder3" \
		"CC = $(DEFAULT_CC)" \
		doTheRest

#############################################################################
#
# deline_filter code
#
#############################################################################

#
# deline_filter
#
DELINE_FILES = deline_filter.c
DELINE_CC_FILES = ""
DELINE_OBJECTS = $(DELINE_CC_FILES:.C=.o) $(DELINE_FILES:.c=.o)
DELINE_INCLUDES = Makefile

deline_filter$(SUFFIX) :
	@$(MAKE) "EXE_NAME = deline_filter" \
		"TARGET_INCLUDES = $(DELINE_INCLUDES)" \
		"TARGET_OBJECTS = $(DELINE_OBJECTS)" \
		"PREFIX = " \
		"SUFFIX=space_holder3" \
		"CC = $(DEFAULT_CC)" \
		doTheRest

#############################################################################
#
# gory details:
#
#   the following rules handle most of the work of making an 
#   application for any architecture.
#
#  Explanation:
#
#   initially, PREFIX is defined to be some non-empty string so that the
#	rule $(PREFIX)$(EXE_NAME) is not defined at the same time that
#	the rule at the top with the same name is.  For example, for
#	"user":  user is defined at the top, with a rule that spawns off
#	make "blah blah" user ("user" is the eventual target, via the 
#	EXE_NAME macro and the calls to "doTheRest" and "mainTarget").
#	If there were no PREFIX and SUFFIX, one of the invocations of "make"
#	would see two rules for "user".  By setting PREFIX initially to
#	be nonblank, the second rule for user is covered up initially, 
#	and by setting SUFFIX in the first rule, the first rule is covered
#	up thereafter.
#
#   finally, the library rule is different, since the target name "lib" is
#	different from the actual executable to be made, so we leave
#	PREFIX and SUFFIX unchanged for that one.
#	
#
#############################################################################

doTheRest :
	@( $(MAKE) \
		"OBJECT_DIR = $(OBJECT_DIR)" \
		"TARGET_INCLUDES = $(TARGET_INCLUDES)" \
		"TARGET_OBJECTS = $(TARGET_OBJECTS)" \
		"INCLUDE_FLAGS = $(INCLUDE_FLAGS)" \
		"LDFLAGS = $(LDFLAGS)" \
		"LIBS = $(LIBS)" \
		"PREFIX = $(PREFIX)" \
		"SUFFIX = $(SUFFIX)" \
		"EXE_NAME = $(EXE_NAME)" \
		mainTarget ; \
	   )


PREFIX = space_holder1
TARGET_OBJECTS = space_holder2

mainTarget :
	@echo "Making $(OBJECT_DIR) if it doesn't exist..."
	@-/bin/mkdir $(OBJECT_DIR)
	@echo "Moving objects from $(OBJECT_DIR) to current directory..."
	@-/bin/rm -f $(EXE_NAME) *.o *.a 
	@-/bin/mv -f $(OBJECT_DIR)/$(EXE_NAME) $(OBJECT_DIR)/*.o \
		$(OBJECT_DIR)/*.a . 
	@echo "Making $(EXE_NAME)..."
	@-$(MAKE) \
		"TARGET_INCLUDES = $(TARGET_INCLUDES)" \
		"TARGET_OBJECTS = $(TARGET_OBJECTS)" \
		"INCLUDE_FLAGS = $(INCLUDE_FLAGS)" \
		"LDFLAGS = $(LDFLAGS)" \
		"LIBS = $(LIBS)" \
		"PREFIX = $(PREFIX)" \
		"SUFFIX = $(SUFFIX)" \
		$(EXE_NAME)
	@echo "Moving objects back to $(OBJECT_DIR) subdir..."
	@-/bin/mv -f $(EXE_NAME) *.[oa] $(OBJECT_DIR) 
	@echo "Done.  Executable (if any) is in $(OBJECT_DIR)/$(EXE_NAME)."

$(PREFIX)$(EXE_NAME) : $(TARGET_OBJECTS)
	-/bin/rm -f $(EXE_NAME)
	$(CC) -o $(EXE_NAME) $(TARGET_OBJECTS) $(LDFLAGS) $(LIBS)

$(TARGET_OBJECTS) : $(TARGET_INCLUDES)

#############################################################################
#
# misc rules
#

clean :
	/bin/rm -f *.o *.a *~ *.j foo stack.gp gpdump* a.out user \
	$(OBJECT_DIR)/simple $(OBJECT_DIR)/user
	make -f gp_make clean

allclean :
	$(MAKE) clean
	/bin/rm -f $(OBJECT_DIR)/*


#############################################################################
#
# Dependencies that are non-obvious
#

Grid.o: ../BCGrid.C ../readNanoscopeFile.C


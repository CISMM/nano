MAKEFILE := Makefile
MAKE := $(MAKE) -f $(MAKEFILE)

# hw_os does not exist on FreeBSD at UNC
UNAME := $(shell uname -s)
ifeq ($(UNAME), FreeBSD)
  HW_OS := pc_FreeBSD
else
  # pc_cygwin doesn't have HW_OS
  ifeq ($(UNAME), CYGWIN_NT-4.0)
    HW_OS := pc_cygwin
  else
    HW_OS := $(shell hw_os)
  endif
endif

CWD := $(shell pwd)

ifeq ($(HW_OS),pc_cygwin)
  LINK = cp
else
  LINK = ln -sf
endif

ifeq ($(HW_OS),pc_linux)
 WHOAMI := $(shell /usr/bin/whoami)
else
 ifeq ($(HW_OS),sparc_solaris)
  WHOAMI := $(shell /usr/ucb/whoami)
 else
  ifeq ($(HW_OS),pc_cygwin)
   WHOAMI := nobody
  else
   WHOAMI := $(shell /bin/whoami)
  endif
 endif
endif

ifeq ($(HW_OS),sgi_irix)
  CC := CC -32 -fullwarn
else
#default case
  CC := g++ -Wall
endif

ifeq ($(HW_OS),hp700_hpux10)
  CC := CC +w
endif

OBJECT_DIR	 := $(HW_OS)

# Looks for a customized, personal version, and if that fails
# uses the group default copy.
# Looks for a customized, personal version, and if that fails
# uses the group default copy.
ifeq ($(HW_OS), pc_cygwin)
  VRPN_DIR	 := ../../vrpn
  VRPN_DEFAULT_DIR	 := ../../../vrpn
else
  VRPN_DIR	 := /afs/unc/proj/stm/src/$(WHOAMI)/vrpn
  VRPN_DEFAULT_DIR	 := /afs/unc/proj/stm/src/vrpn
endif

VRPN_INC_DIRS	= -I$(VRPN_DIR) -I$(VRPN_DEFAULT_DIR)
VRPN_LIB_DIRS	= -L$(VRPN_DIR)/$(HW_OS) -L$(VRPN_DEFAULT_DIR)/$(HW_OS)


CFLAGS = -g -I. -I.. -I../server_microscape -I../server_microscape/base \
	$(VRPN_INC_DIRS) 

  ifeq ($(HW_OS),pc_cygwin)
    ARCH_LIBS := -lwsock32 -luser32 -lgdi32 -lcomdlg32 
  else
    ARCH_LIBS :=
  endif

LFLAGS = -L$(OBJECT_DIR) -L/usr/local/lib $(VRPN_LIB_DIRS) 

LIBS = -lvrpn $(ARCH_LIBS) -lm

APPS = vrpn_headerPrint vrpn_streamTimeFix streamPrint \
	vrpn_newStreamPrint streamToNewVrpn

VHP_CC_FILES := vrpn_headerPrint.C
VSTF_CC_FILES := vrpn_streamTimeFix.C
SP_CC_FILES := nano_utils.C streamPrint.C
VNSP_CC_FILES := vrpn_newStreamPrint.C
STNV_CC_FILES := streamToNewVrpn.C nano_utils.C

VHP_OBJECTS := $(patsubst %,$(OBJECT_DIR)/%,$(VHP_CC_FILES:.C=.o))
VSTF_OBJECTS := $(patsubst %,$(OBJECT_DIR)/%,$(VSTF_CC_FILES:.C=.o))
SP_OBJECTS := $(patsubst %,$(OBJECT_DIR)/%,$(SP_CC_FILES:.C=.o))
VNSP_OBJECTS:= $(patsubst %,$(OBJECT_DIR)/%,$(VNSP_CC_FILES:.C=.o))
STNV_OBJECTS:= $(patsubst %,$(OBJECT_DIR)/%,$(STNV_CC_FILES:.C=.o))

# Build objects from .c files
$(OBJECT_DIR)/%.o: %.c 
	$(CC) $(CFLAGS) -o $@ -c $<

# Build objects from .C files
$(OBJECT_DIR)/%.o: %.C 
	$(CC) $(CFLAGS) -o $@ -c $<

.c.o:
	$(CC) $(CFLAGS) -c $*.c

.C.o:
	$(CC) $(CFLAGS) -c $*.C

.PHONY:	all
all: $(APPS)


$(OBJECT_DIR):
	mkdir $(OBJECT_DIR)


.PHONY:	vrpn_headerPrint
vrpn_headerPrint: $(OBJECT_DIR)/vrpn_headerPrint

$(OBJECT_DIR)/vrpn_headerPrint: $(OBJECT_DIR) $(VHP_OBJECTS)
#	\rm -rf $(OBJECT_DIR)/vrpn_headerPrint
	$(CC) $(LFLAGS) -o $(OBJECT_DIR)/vrpn_headerPrint $(VHP_OBJECTS) $(LIBS)


.PHONY:	vrpn_streamTimeFix
vrpn_streamTimeFix: $(OBJECT_DIR)/vrpn_streamTimeFix

$(OBJECT_DIR)/vrpn_streamTimeFix: $(OBJECT_DIR) $(VSTF_OBJECTS)
#	\rm -rf $(OBJECT_DIR)/vrpn_streamTimeFix
	$(CC) $(LFLAGS) -o $(OBJECT_DIR)/vrpn_streamTimeFix $(VSTF_OBJECTS) $(LIBS)


.PHONY:	streamPrint
streamPrint: $(OBJECT_DIR)/streamPrint

$(OBJECT_DIR)/streamPrint: $(OBJECT_DIR) $(SP_OBJECTS)
#	\rm -rf $(OBJECT_DIR)/streamPrint
	$(CC) $(LFLAGS) -o $(OBJECT_DIR)/streamPrint $(SP_OBJECTS) $(LIBS)

.PHONY: vrpn
vrpn: vrpn_newStreamPrint streamToNewVrpn

# outputs source code + header filesto stdout

.PHONYTEST: vrpntest
vrpntest: vrpn_newStreamPrint.C
	$(CC) $(CFLAGS) -E  $<


.PHONY:	vrpn_newStreamPrint
vrpn_newStreamPrint: $(OBJECT_DIR)/vrpn_newStreamPrint

$(OBJECT_DIR)/vrpn_newStreamPrint: $(OBJECT_DIR) $(VNSP_OBJECTS)
	\rm -rf $(OBJECT_DIR)/vrpn_newStreamPrint
	$(CC) $(LFLAGS) -o $(OBJECT_DIR)/vrpn_newStreamPrint $(VNSP_OBJECTS) $(LIBS)

.PHONY:	streamToNewVrpn
streamToNewVrpn: $(OBJECT_DIR)/streamToNewVrpn

$(OBJECT_DIR)/streamToNewVrpn: $(OBJECT_DIR) $(STNV_OBJECTS)
	\rm -rf $(OBJECT_DIR)/streamToNewVrpn
	$(CC) $(LFLAGS) -o $(OBJECT_DIR)/streamToNewVrpn $(STNV_OBJECTS) $(LIBS)



vrpn_newStreamPrint.C:	registry.C vrpnsp.C code.awk
	awk -f code.awk vrpnsp.C > vrpn_newStreamPrint.C

# The following 3 intermediate files work together to:
#   get the names of all the handlers in sp2.C
#   write code for a vrpn_Connection::register_message_type() and
#     vrpn_Connection::register_handler() for each

registry.C: registry code.awk
	awk -f code.awk registry > registry.C

registry: hnames registry.awk
	awk -f registry.awk hnames > registry

hnames: vrpnsp.C hnames.awk
	awk -f hnames.awk vrpnsp.C > hnames

# Can't do this in two passes if we support arbitrary names for
# the vrpn connection with which to register, although that isn't
# fully thought out yet

# registry.C: hnames registry.awk
# 	awk -f registry.awk hnames > registry.C
# 
# hnames: sp2.C hnames.awk
# 	awk -f hnames.awk vrpnsp.C > hnames
# 	

clean:
	\rm -f $(VHP_OBJECTS) $(VSTF_OBJECTS) \
	$(SP_OBJECTS) $(VNSP_OBJECTS) $(STNV_OBJECTS) \
	$(OBJECT_DIR)/vrpn_headerPrint $(OBJECT_DIR)/vrpn_streamTimeFix \
	$(OBJECT_DIR)/streamPrint $(OBJECT_DIR)/vrpn_newStreamPrint \
	$(OBJECT_DIR)/streamToNewVrpn \
	vrpn_newStreamPrint.C registry.C registry hnames 

# comment



##############################################################################
#
# Typical stuff
#

include ../share/Makefile.common

CWD := $(shell $(PWD_COMMAND))

THIS_DIR := $(subst $(TOP_LEVEL_SRC_DIR),,$(CWD))

OBJECT_DIR := $(join $(TOP_LEVEL_OBJ_DIR)/$(HW_OS),$(THIS_DIR))

CHILD_DIRS := $(filter-out CVS,$(subst /,,$(wildcard */)))

CHILD_LIBS := $(CHILD_DIRS)

###############################################################################
#
# Special targets
#

.PHONY: default
default: all

.PHONY: all
all: $(join $(addprefix $(OBJECT_DIR)/,$(CHILD_DIRS)), \
            $(addprefix /,$(CHILD_LIBS)))

.PHONY: clean
clean: 
	-rm -rf $(OBJECT_DIR) 

.PHONY: thisclean
thisclean:
	#You probably wanted to do a "make clean" here.

#############################################################################
#
# implicit rule for all subdirs
#

.PHONY: %
%:
	$(MAKE) -wC .$(subst $(OBJECT_DIR),,$(@D))

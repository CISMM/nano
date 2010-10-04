USES_LIBS := nmMP nmBase tclLinkVar nmAux nmAnalyze nmUI vrpn vogl quat nmUGraphics

USES_EXTERNAL_LIBS := RhinoIO

EXCLUDE_SRC = stm_file.c

NAUGHTY := 1

include ../Makefile.share

######################################
#
#  Find out which files must be generated
#

GEN-FILES-DEF	:= $(wildcard *.Cdef *.hdef *.vrpndef)
GEN-FILES-H	:= $(patsubst %.hdef,%.h,$(wildcard *.hdef)) \
		  $(patsubst %.vrpndef,%.h,$(wildcard *.vrpndef))
GEN-FILES-C	:= $(patsubst %.Cdef,%.C,$(wildcard *.Cdef)) \
                  $(patsubst %.vrpndef,%.C,$(wildcard *.vrpndef))
GEN-FILES	:= $(GEN-FILES-H) $(GEN-FILES-C)
# GEN-FILES-BASE only looks at .C and .vrpndef (and not header files) because they do not generate objects.
GEN-FILES-BASE	:= $(basename $(GEN-FILES-C) $(GEN-FILES-DEF))
GEN-FILES-O	:= $(addprefix $(OBJECT_DIR)/,$(addsuffix $(OBJECT_SUFFIX),$(GEN-FILES-BASE)))

######################################
#
#  Find out how to generate files
#
ifdef VC
GEN_RPC := ../../../../../../vrpn/util/gen_rpc/gen_vrpn_rpc.pl
else
GEN_RPC := /usr/bin/perl ../../../../../../vrpn/util/gen_rpc/gen_vrpn_rpc.pl
endif




.PHONY: generate_hook
generate_hook: $(GEN-FILES)

.PHONY: clean-gen clean_hook
clean_hook: clean-gen

clean-gen: 
	-rm -f $(GEN-FILES)

%.h : %.vrpndef
	${GEN_RPC} -h $<

%.C : %.vrpndef
	${GEN_RPC} -c $<

%.h : %.hdef
	$(GEN_RPC) $<

%.C : %.Cdef
	$(GEN_RPC) $<

${LIBRARY_ARCHIVE}: $(GEN-FILES-O)

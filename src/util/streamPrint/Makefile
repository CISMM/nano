USES_LIBS := nmAux nmMScope nmBase vrpn quat

ifndef VC
USES_EXTERNAL_LIBS := tcl tk
endif

include ../Makefile.share

.PHONY: tchdissn
tchdissn:  printFDPlanes printTPoints printPDPoints

.PHONY: printPDPoints
printPDoints:
	$(MAKE) APP_NAME=printPDPoints \
	GENERATED_SRC= \
	EXCLUDE_SRC="nmsPrint.C streamEdit.c streamPrint.c streamToNewVrpn.C streamToVrpn.C vrpn_streamTimeFix.C vrpn_uiHeaderPrint.C vrpnsp.C vrpn_streamPrint.C vrpn_headerPrint.C printFDPlanes.C printTPoints.C" \
	MAKECMDGOALS=""

.PHONY: printTPoints
printTPoints:
	$(MAKE) APP_NAME=printTPoints \
	GENERATED_SRC= \
	EXCLUDE_SRC="nmsPrint.C streamEdit.c streamPrint.c streamToNewVrpn.C streamToVrpn.C vrpn_streamTimeFix.C vrpn_uiHeaderPrint.C vrpnsp.C vrpn_streamPrint.C vrpn_headerPrint.C printFDPlanes.C printPDPoints.C" \
	MAKECMDGOALS=""

.PHONY: printFDPlanes
printFDPlanes:
	$(MAKE) APP_NAME=printFDPlanes \
	GENERATED_SRC= \
	EXCLUDE_SRC="nmsPrint.C streamEdit.c streamPrint.c streamToNewVrpn.C streamToVrpn.C vrpn_streamTimeFix.C vrpn_uiHeaderPrint.C vrpnsp.C vrpn_streamPrint.C vrpn_headerPrint.C printTPoints.C printPDPoints.C" \
	MAKECMDGOALS=""

.PHONY: vrpn_headerPrint
vrpn_headerPrint:
	$(MAKE) APP_NAME=vrpn_headerPrint \
                GENERATED_SRC= \
                EXCLUDE_SRC="nmsPrint.C streamEdit.c streamPrint.c streamToNewVrpn.C streamToVrpn.C vrpn_streamTimeFix.C vrpn_uiHeaderPrint.C vrpnsp.C vrpn_streamPrint.C printFDPlanes.C printTPoints.C printPDPoints.C" \
                MAKECMDGOALS=""

.PHONY: vrpn_streamPrint
vrpn_streamPrint:
	$(MAKE) APP_NAME=vrpn_streamPrint \
                GENERATED_SRC= \
                EXCLUDE_SRC="streamEdit.c streamPrint.c streamToNewVrpn.C streamToVrpn.C vrpn_streamTimeFix.C vrpn_uiHeaderPrint.C vrpnsp.C vrpn_headerPrint.C printFDPlanes.C printTPoints.C printPDPoints.C" \
                MAKECMDGOALS=""

.PHONY: vrpn_uiHeaderPrint
vrpn_uiHeaderPrint:
	$(MAKE) APP_NAME=vrpn_uiHeaderPrint \
                GENERATED_SRC= \
                EXCLUDE_SRC="nmsPrint.C streamEdit.c streamPrint.c streamToNewVrpn.C streamToVrpn.C vrpn_streamTimeFix.C vrpn_headerPrint.C vrpnsp.C printFDPlanes.C printTPoints.C printPDPoints.C" \
                MAKECMDGOALS=""

.PHONY: vrpn_streamTimeFix
vrpn_streamTimeFix:
	$(MAKE) APP_NAME=vrpn_streamTimeFix \
                GENERATED_SRC= \
                EXCLUDE_SRC="nmsPrint.C streamEdit.c streamPrint.c streamToNewVrpn.C streamToVrpn.C vrpn_headerPrint.C vrpn_uiHeaderPrint.C vrpnsp.C printFDPlanes.C printTPoints.C printPDPoints.C" \
                MAKECMDGOALS=""

.PHONY: streamPrint
streamPrint:
	$(MAKE) APP_NAME=streamPrint \
                GENERATED_SRC= \
                EXCLUDE_SRC="nmsPrint.C streamEdit.c streamToNewVrpn.C streamToVrpn.C vrpn_streamTimeFix.C vrpn_headerPrint.C vrpn_uiHeaderPrint.C vrpnsp.C printFDPlanes.C printTPoints.C printPDPoints.C" \
                MAKECMDGOALS=""

.PHONY: vrpn_newStreamPrint
vrpn_newStreamPrint:
	$(MAKE) APP_NAME=vrpn_newStreamPrint \
                GENERATED_SRC= \
                EXCLUDE_SRC="nmsPrint.C streamEdit.c streamPrint.c streamToNewVrpn.C streamToVrpn.C vrpn_streamTimeFix.C vrpn_headerPrint.C vrpn_uiHeaderPrint.C printFDPlanes.C printTPoints.C printPDPoints.C" \
               MAKECMDGOALS=""

.PHONY: streamToVrpn
streamToVrpn:
	$(MAKE) APP_NAME=streamToVrpn \
                GENERATED_SRC= \
                EXCLUDE_SRC="nmsPrint.C streamEdit.c streamPrint.c streamToNewVrpn.C vrpn_streamTimeFix.C vrpn_headerPrint.C vrpn_uiHeaderPrint.C vrpnsp.C  printFDPlanes.C printTPoints.C printPDPoints.C" \
                MAKECMDGOALS=""

.PHONY: streamToNewVrpn
streamToNewVrpn:
	$(MAKE) APP_NAME=streamToNewVrpn \
                GENERATED_SRC= \
                EXCLUDE_SRC="nmsPrint.C streamEdit.c streamPrint.c streamToVrpn.C vrpn_streamTimeFix.C vrpn_headerPrint.C vrpn_uiHeaderPrint.C vrpnsp.C printFDPlanes.C printTPoints.C printPDPoints.C" \
                MAKECMDGOALS=""

.PHONY: nmsPrint
nmsPrint: $(OBJECT_DIR)/nmsPrint$(OBJECT_SUFFIX)
	$(MAKE) APP_NAME=nmsPrint.exe \
                GENERATED_SRC= \
                EXCLUDE_SRC="streamEdit.c streamPrint.c streamToNewVrpn.C streamToVrpn.C vrpn_streamTimeFix.C vrpn_headerPrint.C vrpn_uiHeaderPrint.C vrpnsp.C printFDPlanes.C printTPoints.C printPDPoints.C" \
                MAKECMDGOALS=""

ifdef VC
GEN_RPC := ../../../../vrpn/util/gen_rpc/gen_vrpn_rpc.pl
else
GEN_RPC := /usr/local/bin/perl ../../../../vrpn/util/gen_rpc/gen_vrpn_rpc.pl
endif

nmsPrint.C: nmsPrint.Cdef
	${GEN_RPC} nmsPrint.Cdef

USES_LIBS := nmAux nmMScope nmBase vrpn quat

include ../Makefile.share

vrpn_headerPrint:
	$(MAKE) APP_NAME=vrpn_headerPrint \
                GENERATED_SRC= \
                EXCLUDE_SRC="streamEdit.c streamPrint.c streamToNewVrpn.C streamToVrpn.C vrpn_streamTimeFix.C vrpn_uiHeaderPrint.C vrpnsp.C" \
                MAKECMDGOALS=""

vrpn_uiHeaderPrint:
	$(MAKE) APP_NAME=vrpn_uiHeaderPrint \
                GENERATED_SRC= \
                EXCLUDE_SRC="streamEdit.c streamPrint.c streamToNewVrpn.C streamToVrpn.C vrpn_streamTimeFix.C vrpn_headerPrint.C vrpnsp.C" \
                MAKECMDGOALS=""

vrpn_streamTimeFix:
	$(MAKE) APP_NAME=vrpn_streamTimeFix \
                GENERATED_SRC= \
                EXCLUDE_SRC="streamEdit.c streamPrint.c streamToNewVrpn.C streamToVrpn.C vrpn_headerPrint.C vrpn_uiHeaderPrint.C vrpnsp.C" \
                MAKECMDGOALS=""

streamPrint:
	$(MAKE) APP_NAME=streamPrint \
                GENERATED_SRC= \
                EXCLUDE_SRC="streamEdit.c streamToNewVrpn.C streamToVrpn.C vrpn_streamTimeFix.C vrpn_headerPrint.C vrpn_uiHeaderPrint.C vrpnsp.C" \
                MAKECMDGOALS=""

vrpn_newStreamPrint:
	$(MAKE) APP_NAME=vrpn_newStreamPrint \
                GENERATED_SRC= \
                EXCLUDE_SRC="streamEdit.c streamPrint.c streamToNewVrpn.C streamToVrpn.C vrpn_streamTimeFix.C vrpn_headerPrint.C vrpn_uiHeaderPrint.C" \
               MAKECMDGOALS=""

streamToVrpn:
	$(MAKE) APP_NAME=streamToVrpn \
                GENERATED_SRC= \
                EXCLUDE_SRC="streamEdit.c streamPrint.c streamToNewVrpn.C vrpn_streamTimeFix.C vrpn_headerPrint.C vrpn_uiHeaderPrint.C vrpnsp.C" \
                MAKECMDGOALS=""

streamToNewVrpn:
	$(MAKE) APP_NAME=streamToNewVrpn \
                GENERATED_SRC= \
                EXCLUDE_SRC="streamEdit.c streamPrint.c streamToVrpn.C vrpn_streamTimeFix.C vrpn_headerPrint.C vrpn_uiHeaderPrint.C vrpnsp.C" \
                MAKECMDGOALS=""

USES_LIBS := nmBase tclLinkVar nmAux vrpn vogl quat

EXCLUDE_SRC = stm_file.c

NAUGHTY := 1

ifdef VC
GEN_RPC := ../../../../../../vrpn/util/gen_rpc/gen_vrpn_rpc.pl
else
GEN_RPC := /usr/local/bin/perl ../../../../../../vrpn/util/gen_rpc/gen_vrpn_rpc.pl
endif

include ../Makefile.share

nmm_SPM_Control.h: nmm_SPM_Control.vrpndef
	${GEN_RPC} -h nmm_SPM_Control.vrpndef
nmm_SPM_Control.C: nmm_SPM_Control.vrpndef nmm_SPM_Control.h
	${GEN_RPC} -c nmm_SPM_Control.vrpndef

nmm_SPM_Report.h: nmm_SPM_Report.vrpndef
	${GEN_RPC} -h nmm_SPM_Report.vrpndef
nmm_SPM_Report.C: nmm_SPM_Report.vrpndef nmm_SPM_Report.h
	${GEN_RPC} -c nmm_SPM_Report.vrpndef
nmm_AFM_Control.h: nmm_AFM_Control.vrpndef
	${GEN_RPC} -h nmm_AFM_Control.vrpndef
nmm_AFM_Control.C: nmm_AFM_Control.vrpndef nmm_AFM_Control.h
	${GEN_RPC} -c nmm_AFM_Control.vrpndef
nmm_AFM_Report.h: nmm_AFM_Report.vrpndef
	${GEN_RPC} -h nmm_AFM_Report.vrpndef
nmm_AFM_Report.C: nmm_AFM_Report.vrpndef nmm_AFM_Report.h
	${GEN_RPC} -c nmm_AFM_Report.vrpndef

nmm_MicroscopeRemote.h: nmm_SPM_Control.h nmm_SPM_Report.h nmm_AFM_Control.h \
	nmm_AFM_Report.h nmm_MicroscopeRemote.hdef
	${GEN_RPC} nmm_MicroscopeRemote.hdef
nmm_MicroscopeRemote.C: nmm_SPM_Control.h nmm_SPM_Report.h nmm_AFM_Control.h \
	nmm_AFM_Report.h nmm_MicroscopeRemote.Cdef
	${GEN_RPC} nmm_MicroscopeRemote.Cdef

${LIBRARY_ARCHIVE}: $(OBJECT_DIR)/nmm_MicroscopeRemote$(OBJECT_SUFFIX) \
	$(OBJECT_DIR)/nmm_SPM_Control$(OBJECT_SUFFIX) \
	$(OBJECT_DIR)/nmm_SPM_Report$(OBJECT_SUFFIX) \
	$(OBJECT_DIR)/nmm_AFM_Control$(OBJECT_SUFFIX) \
	$(OBJECT_DIR)/nmm_AFM_Report$(OBJECT_SUFFIX)

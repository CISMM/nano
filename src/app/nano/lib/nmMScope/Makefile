USES_LIBS := nmMP nmBase tclLinkVar nmAux nmAnalyze nmUI vrpn vogl quat

USES_EXTERNAL_LIBS := RhinoIO

EXCLUDE_SRC = stm_file.c

NAUGHTY := 1

ifdef VC
GEN_RPC := ../../../../../../vrpn/util/gen_rpc/gen_vrpn_rpc.pl
else
GEN_RPC := /usr/local/bin/perl ../../../../../../vrpn/util/gen_rpc/gen_vrpn_rpc.pl
endif

include ../Makefile.share

clean-gen: 
	-rm -f nmm_SPM_Control.h nmm_SPM_Report.h nmm_AFM_Control.h \
	nmm_AFM_Report.h nmm_MicroscopeRemote.h \
	nmm_SPM_Control.C nmm_SPM_Report.C nmm_AFM_Control.C \
	nmm_AFM_Report.C nmm_MicroscopeRemoteGen.C \
	nmm_SimulatedMicroscope_Remote.C nmm_SimulatedMicroscope_Remote.h \
	nmm_SimulatedMicroscope.C nmm_SimulatedMicroscope_Remote.C \
	nmm_AFMSIM_Report.C nmm_AFMSIM_Report.h


# All generated files need to be created before ".depend" gets created.
generate: nmm_SPM_Control.h nmm_SPM_Report.h nmm_AFM_Control.h \
	nmm_AFM_Report.h nmm_MicroscopeRemote.h \
	nmm_SPM_Control.C nmm_SPM_Report.C nmm_AFM_Control.C \
	nmm_AFM_Report.C nmm_MicroscopeRemoteGen.C \
	nmm_SimulatedMicroscope_Remote.C nmm_SimulatedMicroscope_Remote.h \
	nmm_SimulatedMicroscope.C nmm_SimulatedMicroscope_Remote.C \
	nmm_AFMSIM_Report.C nmm_AFMSIM_Report.h

nmm_SPM_Control.h: nmm_SPM_Control.vrpndef
	${GEN_RPC} -h nmm_SPM_Control.vrpndef
nmm_SPM_Control.C: nmm_SPM_Control.vrpndef
	${GEN_RPC} -c nmm_SPM_Control.vrpndef

nmm_SPM_Report.h: nmm_SPM_Report.vrpndef
	${GEN_RPC} -h nmm_SPM_Report.vrpndef
nmm_SPM_Report.C: nmm_SPM_Report.vrpndef
	${GEN_RPC} -c nmm_SPM_Report.vrpndef

nmm_AFM_Control.h: nmm_AFM_Control.vrpndef
	${GEN_RPC} -h nmm_AFM_Control.vrpndef
nmm_AFM_Control.C: nmm_AFM_Control.vrpndef
	${GEN_RPC} -c nmm_AFM_Control.vrpndef

nmm_AFM_Report.h: nmm_AFM_Report.vrpndef
	${GEN_RPC} -h nmm_AFM_Report.vrpndef
nmm_AFM_Report.C: nmm_AFM_Report.vrpndef
	${GEN_RPC} -c nmm_AFM_Report.vrpndef

nmm_AFMSIM_Report.h: nmm_AFMSIM_Report.vrpndef
	${GEN_RPC} -h nmm_AFMSIM_Report.vrpndef
nmm_AFMSIM_Report.C: nmm_AFMSIM_Report.vrpndef
	${GEN_RPC} -c nmm_AFMSIM_Report.vrpndef

nmm_MicroscopeRemote.h: nmm_SPM_Control.vrpndef nmm_SPM_Report.vrpndef \
	nmm_AFM_Control.vrpndef \
	nmm_AFM_Report.vrpndef nmm_MicroscopeRemote.hdef
	${GEN_RPC} nmm_MicroscopeRemote.hdef
nmm_MicroscopeRemoteGen.C: nmm_SPM_Control.vrpndef nmm_SPM_Report.vrpndef \
	nmm_AFM_Control.vrpndef \
	nmm_AFM_Report.vrpndef nmm_MicroscopeRemoteGen.Cdef
	${GEN_RPC} nmm_MicroscopeRemoteGen.Cdef
nmm_SimulatedMicroscope_Remote.h: nmm_AFMSIM_Report.vrpndef \
	nmm_SimulatedMicroscope_Remote.hdef \
	${GEN_RPC} nmm_SimulatedMicroscope_Remote.hdef
nmm_SimulatedMicroscope.h: nmm_AFMSIM_Report.vrpndef \
	nmm_SimulatedMicroscope.hdef \
	${GEN_RPC} nmm_SimulatedMicroscope.hdef
nmm_SimulatedMicroscope_Remote.C: nmm_AFMSIM_Report.vrpndef \
	nmm_SimulatedMicroscope_Remote.Cdef \
	${GEN_RPC} nmm_SimulatedMicroscope_Remote.Cdef
nmm_SimulatedMicroscope.C: nmm_AFMSIM_Report.vrpndef \
	nmm_SimulatedMicroscope.Cdef \
	${GEN_RPC} nmm_SimulatedMicroscope.Cdef

${LIBRARY_ARCHIVE}: $(OBJECT_DIR)/nmm_MicroscopeRemoteGen$(OBJECT_SUFFIX) \
	$(OBJECT_DIR)/nmm_SPM_Control$(OBJECT_SUFFIX) \
	$(OBJECT_DIR)/nmm_SPM_Report$(OBJECT_SUFFIX) \
	$(OBJECT_DIR)/nmm_AFM_Control$(OBJECT_SUFFIX) \
	$(OBJECT_DIR)/nmm_AFM_Report$(OBJECT_SUFFIX)

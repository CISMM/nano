#AVS stuff

setenv LM_LICENSE_FILE 1800@atlantic.ncsc.org

setenv AVS_GG_HOME /afs/unc/proj/mip/neuro/avs
setenv AVS_LOCAL_HOME /usr/avs
if (-e ${AVS_LOCAL_HOME}/avs.env) then
	source ${AVS_LOCAL_HOME}/avs.env
endif
setenv AVS_USER_DATA_TYPES ${AVS_GG_HOME}/include/gg_udd.h
setenv AVS_MODULE_HOME ${home}/avs/modules

if ($?LD_LIBRARY_PATH == 0) then
        setenv  LD_LIBRARY_PATH '.'
endif
setenv LD_LIBRARY_PATH ${LD_LIBRARY_PATH}:${AVS_LOCAL_HOME}/lib:${AVS_GG_HOME}/lib_shared:${AVS_GG_HOME}/lib

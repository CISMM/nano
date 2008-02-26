# Microsoft Developer Studio Project File - Name="nmMScopeGenerate" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Generic Project" 0x010a

CFG=nmMScopeGenerate - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "nmMScopeGenerate.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "nmMScopeGenerate.mak" CFG="nmMScopeGenerate - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "nmMScopeGenerate - Win32 Debug" (based on "Win32 (x86) Generic Project")
!MESSAGE "nmMScopeGenerate - Win32 Release" (based on "Win32 (x86) Generic Project")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
MTL=midl.exe

!IF  "$(CFG)" == "nmMScopeGenerate - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "nmMScopeGenerate___Win32_Debug"
# PROP BASE Intermediate_Dir "nmMScopeGenerate___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../../../../obj/pc_win32/debug/app/nano/lib/nmMScope"
# PROP Intermediate_Dir "../../../../../obj/pc_win32/debug/app/nano/lib/nmMScope"
# PROP Target_Dir ""

!ELSEIF  "$(CFG)" == "nmMScopeGenerate - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "nmMScopeGenerate___Win32_Release"
# PROP BASE Intermediate_Dir "nmMScopeGenerate___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../../../../obj/pc_win32/release/app/nano/lib/nmMScope"
# PROP Intermediate_Dir "../../../../../obj/pc_win32/release/app/nano/lib/nmMScope"
# PROP Target_Dir ""

!ENDIF 

# Begin Target

# Name "nmMScopeGenerate - Win32 Debug"
# Name "nmMScopeGenerate - Win32 Release"
# Begin Group "Generated"

# PROP Default_Filter ""
# Begin Group "Header"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\nmm_MicroscopeRemote.hdef

!IF  "$(CFG)" == "nmMScopeGenerate - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Performing Custom Build Step on $(InputPath)
ProjDir=.
InputPath=.\nmm_MicroscopeRemote.hdef
InputName=nmm_MicroscopeRemote

"$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(SYSTEMDRIVE)\cygwin\bin\bash -c "'$(ProjDir)/generate.sh' '$(InputPath)' "

# End Custom Build

!ELSEIF  "$(CFG)" == "nmMScopeGenerate - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Performing Custom Build Step on $(InputPath)
ProjDir=.
InputPath=.\nmm_MicroscopeRemote.hdef
InputName=nmm_MicroscopeRemote

"$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(SYSTEMDRIVE)\cygwin\bin\bash -c "'$(ProjDir)/generate.sh' '$(InputPath)' "

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\nmm_SimulatedMicroscope.hdef

!IF  "$(CFG)" == "nmMScopeGenerate - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Performing Custom Build Step on $(InputPath)
ProjDir=.
InputPath=.\nmm_SimulatedMicroscope.hdef
InputName=nmm_SimulatedMicroscope

"$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(SYSTEMDRIVE)\cygwin\bin\bash -c "'$(ProjDir)/generate.sh' '$(InputPath)' "

# End Custom Build

!ELSEIF  "$(CFG)" == "nmMScopeGenerate - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Performing Custom Build Step on $(InputPath)
ProjDir=.
InputPath=.\nmm_SimulatedMicroscope.hdef
InputName=nmm_SimulatedMicroscope

"$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(SYSTEMDRIVE)\cygwin\bin\bash -c "'$(ProjDir)/generate.sh' '$(InputPath)' "

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\nmm_SimulatedMicroscope_Remote.hdef

!IF  "$(CFG)" == "nmMScopeGenerate - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Performing Custom Build Step on $(InputPath)
ProjDir=.
InputPath=.\nmm_SimulatedMicroscope_Remote.hdef
InputName=nmm_SimulatedMicroscope_Remote

"$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(SYSTEMDRIVE)\cygwin\bin\bash -c "'$(ProjDir)/generate.sh' '$(InputPath)' "

# End Custom Build

!ELSEIF  "$(CFG)" == "nmMScopeGenerate - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Performing Custom Build Step on $(InputPath)
ProjDir=.
InputPath=.\nmm_SimulatedMicroscope_Remote.hdef
InputName=nmm_SimulatedMicroscope_Remote

"$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(SYSTEMDRIVE)\cygwin\bin\bash -c "'$(ProjDir)/generate.sh' '$(InputPath)' "

# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Group "C Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\nmm_MicroscopeRemoteGen.Cdef

!IF  "$(CFG)" == "nmMScopeGenerate - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build
ProjDir=.
InputPath=.\nmm_MicroscopeRemoteGen.Cdef
InputName=nmm_MicroscopeRemoteGen

"$(InputName).C" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(SYSTEMDRIVE)\cygwin\bin\bash -c "'$(ProjDir)/generate.sh' '$(InputPath)' "

# End Custom Build

!ELSEIF  "$(CFG)" == "nmMScopeGenerate - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build
ProjDir=.
InputPath=.\nmm_MicroscopeRemoteGen.Cdef
InputName=nmm_MicroscopeRemoteGen

"$(InputName).C" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(SYSTEMDRIVE)\cygwin\bin\bash -c "'$(ProjDir)/generate.sh' '$(InputPath)' "

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\nmm_SimulatedMicroscope.Cdef

!IF  "$(CFG)" == "nmMScopeGenerate - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build
ProjDir=.
InputPath=.\nmm_SimulatedMicroscope.Cdef
InputName=nmm_SimulatedMicroscope

"$(InputName).C" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(SYSTEMDRIVE)\cygwin\bin\bash -c "'$(ProjDir)/generate.sh' '$(InputPath)' "

# End Custom Build

!ELSEIF  "$(CFG)" == "nmMScopeGenerate - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build
ProjDir=.
InputPath=.\nmm_SimulatedMicroscope.Cdef
InputName=nmm_SimulatedMicroscope

"$(InputName).C" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(SYSTEMDRIVE)\cygwin\bin\bash -c "'$(ProjDir)/generate.sh' '$(InputPath)' "

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\nmm_SimulatedMicroscope_Remote.Cdef

!IF  "$(CFG)" == "nmMScopeGenerate - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build
ProjDir=.
InputPath=.\nmm_SimulatedMicroscope_Remote.Cdef
InputName=nmm_SimulatedMicroscope_Remote

"$(InputName).C" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(SYSTEMDRIVE)\cygwin\bin\bash -c "'$(ProjDir)/generate.sh' '$(InputPath)' "

# End Custom Build

!ELSEIF  "$(CFG)" == "nmMScopeGenerate - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build
ProjDir=.
InputPath=.\nmm_SimulatedMicroscope_Remote.Cdef
InputName=nmm_SimulatedMicroscope_Remote

"$(InputName).C" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(SYSTEMDRIVE)\cygwin\bin\bash -c "'$(ProjDir)/generate.sh' '$(InputPath)' "

# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Group "Both"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\nmm_AFM_Control.vrpndef

!IF  "$(CFG)" == "nmMScopeGenerate - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build
ProjDir=.
InputPath=.\nmm_AFM_Control.vrpndef
InputName=nmm_AFM_Control

BuildCmds= \
	$(SYSTEMDRIVE)\cygwin\bin\bash -c "'$(ProjDir)/generate.sh' -c '$(InputPath)' " \
	$(SYSTEMDRIVE)\cygwin\bin\bash -c "'$(ProjDir)/generate.sh' -h '$(InputPath)' " \
	

"$(InputName).C" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "nmMScopeGenerate - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build
ProjDir=.
InputPath=.\nmm_AFM_Control.vrpndef
InputName=nmm_AFM_Control

BuildCmds= \
	$(SYSTEMDRIVE)\cygwin\bin\bash -c "'$(ProjDir)/generate.sh' -c '$(InputPath)' " \
	$(SYSTEMDRIVE)\cygwin\bin\bash -c "'$(ProjDir)/generate.sh' -h '$(InputPath)' " \
	

"$(InputName).C" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\nmm_AFM_Report.vrpndef

!IF  "$(CFG)" == "nmMScopeGenerate - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build
ProjDir=.
InputPath=.\nmm_AFM_Report.vrpndef
InputName=nmm_AFM_Report

BuildCmds= \
	$(SYSTEMDRIVE)\cygwin\bin\bash -c "'$(ProjDir)/generate.sh' -c '$(InputPath)' " \
	$(SYSTEMDRIVE)\cygwin\bin\bash -c "'$(ProjDir)/generate.sh' -h '$(InputPath)' " \
	

"$(InputName).C" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "nmMScopeGenerate - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build
ProjDir=.
InputPath=.\nmm_AFM_Report.vrpndef
InputName=nmm_AFM_Report

BuildCmds= \
	$(SYSTEMDRIVE)\cygwin\bin\bash -c "'$(ProjDir)/generate.sh' -c '$(InputPath)' " \
	$(SYSTEMDRIVE)\cygwin\bin\bash -c "'$(ProjDir)/generate.sh' -h '$(InputPath)' " \
	

"$(InputName).C" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\nmm_AFMSIM_Report.vrpndef

!IF  "$(CFG)" == "nmMScopeGenerate - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build
ProjDir=.
InputPath=.\nmm_AFMSIM_Report.vrpndef
InputName=nmm_AFMSIM_Report

BuildCmds= \
	$(SYSTEMDRIVE)\cygwin\bin\bash -c "'$(ProjDir)/generate.sh' -c '$(InputPath)' " \
	$(SYSTEMDRIVE)\cygwin\bin\bash -c "'$(ProjDir)/generate.sh' -h '$(InputPath)' " \
	

"$(InputName).C" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "nmMScopeGenerate - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build
ProjDir=.
InputPath=.\nmm_AFMSIM_Report.vrpndef
InputName=nmm_AFMSIM_Report

BuildCmds= \
	$(SYSTEMDRIVE)\cygwin\bin\bash -c "'$(ProjDir)/generate.sh' -c '$(InputPath)' " \
	$(SYSTEMDRIVE)\cygwin\bin\bash -c "'$(ProjDir)/generate.sh' -h '$(InputPath)' " \
	

"$(InputName).C" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\nmm_AFMSIMSERVER_Report.vrpndef

!IF  "$(CFG)" == "nmMScopeGenerate - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build
ProjDir=.
InputPath=.\nmm_AFMSIMSERVER_Report.vrpndef
InputName=nmm_AFMSIMSERVER_Report

BuildCmds= \
	$(SYSTEMDRIVE)\cygwin\bin\bash -c "'$(ProjDir)/generate.sh' -c '$(InputPath)' " \
	$(SYSTEMDRIVE)\cygwin\bin\bash -c "'$(ProjDir)/generate.sh' -h '$(InputPath)' " \
	

"$(InputName).C" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "nmMScopeGenerate - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build
ProjDir=.
InputPath=.\nmm_AFMSIMSERVER_Report.vrpndef
InputName=nmm_AFMSIMSERVER_Report

BuildCmds= \
	$(SYSTEMDRIVE)\cygwin\bin\bash -c "'$(ProjDir)/generate.sh' -c '$(InputPath)' " \
	$(SYSTEMDRIVE)\cygwin\bin\bash -c "'$(ProjDir)/generate.sh' -h '$(InputPath)' " \
	

"$(InputName).C" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\nmm_Monitor.vrpndef

!IF  "$(CFG)" == "nmMScopeGenerate - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build
ProjDir=.
InputPath=.\nmm_Monitor.vrpndef
InputName=nmm_Monitor

BuildCmds= \
	$(SYSTEMDRIVE)\cygwin\bin\bash -c "'$(ProjDir)/generate.sh' -c '$(InputPath)' " \
	$(SYSTEMDRIVE)\cygwin\bin\bash -c "'$(ProjDir)/generate.sh' -h '$(InputPath)' " \
	

"$(InputName).C" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "nmMScopeGenerate - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build
ProjDir=.
InputPath=.\nmm_Monitor.vrpndef
InputName=nmm_Monitor

BuildCmds= \
	$(SYSTEMDRIVE)\cygwin\bin\bash -c "'$(ProjDir)/generate.sh' -c '$(InputPath)' " \
	$(SYSTEMDRIVE)\cygwin\bin\bash -c "'$(ProjDir)/generate.sh' -h '$(InputPath)' " \
	

"$(InputName).C" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\nmm_SPM_Control.vrpndef

!IF  "$(CFG)" == "nmMScopeGenerate - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build
ProjDir=.
InputPath=.\nmm_SPM_Control.vrpndef
InputName=nmm_SPM_Control

BuildCmds= \
	$(SYSTEMDRIVE)\cygwin\bin\bash -c "'$(ProjDir)/generate.sh' -c '$(InputPath)' " \
	$(SYSTEMDRIVE)\cygwin\bin\bash -c "'$(ProjDir)/generate.sh' -h '$(InputPath)' " \
	

"$(InputName).C" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "nmMScopeGenerate - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build
ProjDir=.
InputPath=.\nmm_SPM_Control.vrpndef
InputName=nmm_SPM_Control

BuildCmds= \
	$(SYSTEMDRIVE)\cygwin\bin\bash -c "'$(ProjDir)/generate.sh' -c '$(InputPath)' " \
	$(SYSTEMDRIVE)\cygwin\bin\bash -c "'$(ProjDir)/generate.sh' -h '$(InputPath)' " \
	

"$(InputName).C" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\nmm_SPM_Report.vrpndef

!IF  "$(CFG)" == "nmMScopeGenerate - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build
ProjDir=.
InputPath=.\nmm_SPM_Report.vrpndef
InputName=nmm_SPM_Report

BuildCmds= \
	$(SYSTEMDRIVE)\cygwin\bin\bash -c "'$(ProjDir)/generate.sh' -c '$(InputPath)' " \
	$(SYSTEMDRIVE)\cygwin\bin\bash -c "'$(ProjDir)/generate.sh' -h '$(InputPath)' " \
	

"$(InputName).C" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "nmMScopeGenerate - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build
ProjDir=.
InputPath=.\nmm_SPM_Report.vrpndef
InputName=nmm_SPM_Report

BuildCmds= \
	$(SYSTEMDRIVE)\cygwin\bin\bash -c "'$(ProjDir)/generate.sh' -c '$(InputPath)' " \
	$(SYSTEMDRIVE)\cygwin\bin\bash -c "'$(ProjDir)/generate.sh' -h '$(InputPath)' " \
	

"$(InputName).C" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# End Group
# End Group
# End Target
# End Project

# Microsoft Developer Studio Project File - Name="nmMScope" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=nmMScope - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "nmMScope.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "nmMScope.mak" CFG="nmMScope - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "nmMScope - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "nmMScope - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 1
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "nmMScope - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../../../../obj/pc_win32/debug/app/nano/lib/nmMScope"
# PROP Intermediate_Dir "../../../../../obj/pc_win32/debug/app/nano/lib/nmMScope"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /GX /Zi /Od /I "." /I "..\..\..\..\..\..\vrpn" /I "..\..\..\..\lib\tclLinkVar" /I "..\..\..\..\lib\nmBase" /I "..\..\..\..\lib\nmMP" /I "..\..\..\..\..\..\external\pc_win32\include" /I "..\..\..\..\..\..\quat" /I "..\nmUI" /I "..\.." /I "..\..\..\..\..\..\vogl" /I "..\nmAux" /I "..\nmAnalyze" /I "..\nmUGraphics" /D "NDEBUG" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "USE_VRPN_MICROSCOPE" /D "V_GLUT" /D "NO_RAW_TERM" /D "NO_XWINDOWS" /D "NO_ITCL" /D "NO_FILTERS" /D "NO_EXT_TEXTURES" /D "PROJECTIVE_TEXTURE" /FR /YX /FD /GZ /TP /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "nmMScope - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "nmMScope___Win32_Release"
# PROP BASE Intermediate_Dir "nmMScope___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../../../../obj/pc_win32/release/app/nano/lib/nmMScope"
# PROP Intermediate_Dir "../../../../../obj/pc_win32/release/app/nano/lib/nmMScope"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /GX /Zi /Od /I "." /I "..\..\..\..\..\..\vrpn" /I "..\..\..\..\lib\tclLinkVar" /I "..\..\..\..\lib\nmBase" /I "..\..\..\..\lib\nmMP" /I "..\..\..\..\..\..\external\pc_win32\include" /I "..\..\..\..\..\..\quat" /I "..\nmUI" /I "..\.." /I "..\..\..\..\..\..\vogl" /I "..\nmAux" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "USE_VRPN_MICROSCOPE" /D "V_GLUT" /D "NO_RAW_TERM" /D "NO_XWINDOWS" /D "NO_ITCL" /D "NO_FILTERS" /D "NO_EXT_TEXTURES" /D "PROJECTIVE_TEXTURE" /YX /FD /GZ /TP /c
# ADD CPP /nologo /MT /W3 /GX /O2 /Ob2 /I "." /I "..\..\..\..\..\..\vrpn" /I "..\..\..\..\lib\tclLinkVar" /I "..\..\..\..\lib\nmBase" /I "..\..\..\..\lib\nmMP" /I "..\..\..\..\..\..\external\pc_win32\include" /I "..\..\..\..\..\..\quat" /I "..\nmUI" /I "..\.." /I "..\..\..\..\..\..\vogl" /I "..\nmAux" /I "..\nmAnalyze" /D "_MBCS" /D "_LIB" /D "USE_VRPN_MICROSCOPE" /D "V_GLUT" /D "NO_RAW_TERM" /D "NO_XWINDOWS" /D "NO_ITCL" /D "NO_FILTERS" /D "NO_EXT_TEXTURES" /D "PROJECTIVE_TEXTURE" /YX /Zl /FD /TP /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "nmMScope - Win32 Debug"
# Name "nmMScope - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\active_set.C
# End Source File
# Begin Source File

SOURCE=.\AFMState.C
# End Source File
# Begin Source File

SOURCE=.\directstep.C
# End Source File
# Begin Source File

SOURCE=.\drift.c
# End Source File
# Begin Source File

SOURCE=.\nmm_AFM_Control.C
# End Source File
# Begin Source File

SOURCE=.\nmm_AFM_Report.C
# End Source File
# Begin Source File

SOURCE=.\nmm_AFMSIM_Report.C
# End Source File
# Begin Source File

SOURCE=.\nmm_AFMSIMSERVER_Report.C
# End Source File
# Begin Source File

SOURCE=.\nmm_GuardedscanClient.C
# End Source File
# Begin Source File

SOURCE=.\nmm_Microscope.C
# End Source File
# Begin Source File

SOURCE=.\nmm_MicroscopeRemote.C
# End Source File
# Begin Source File

SOURCE=.\nmm_MicroscopeRemoteGen.C
# End Source File
# Begin Source File

SOURCE=.\nmm_MicroscopeRemoteGen.Cdef

!IF  "$(CFG)" == "nmMScope - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build
InputDir=.
WkspDir=.
ProjDir=.
InputPath=.\nmm_MicroscopeRemoteGen.Cdef
InputName=nmm_MicroscopeRemoteGen

"$(ProjDir)\$(InputName).C" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	C:\cygwin\bin\bash --login -c "cd '$(WkspDir)$(InputDir)\app\nano\lib\nmMScope'; ../../../../../../vrpn/util/gen_rpc/gen_vrpn_rpc.pl $(InputName).Cdef"

# End Custom Build

!ELSEIF  "$(CFG)" == "nmMScope - Win32 Release"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build
InputDir=.
WkspDir=.
ProjDir=.
InputPath=.\nmm_MicroscopeRemoteGen.Cdef
InputName=nmm_MicroscopeRemoteGen

"$(ProjDir)\$(InputName).C" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	C:\cygwin\bin\bash --login -c "cd '$(WkspDir)$(InputDir)\app\nano\lib\nmMScope'; ../../../../../../vrpn/util/gen_rpc/gen_vrpn_rpc.pl $(InputName).Cdef"

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\nmm_MicroscopeTranslator.C
# End Source File
# Begin Source File

SOURCE=.\nmm_QueueMonitor.C
# End Source File
# Begin Source File

SOURCE=.\nmm_RelaxComp.C
# End Source File
# Begin Source File

SOURCE=.\nmm_Sample.C
# End Source File
# Begin Source File

SOURCE=.\nmm_SimulatedMicroscope.C
# End Source File
# Begin Source File

SOURCE=.\nmm_SimulatedMicroscope_Remote.C
# End Source File
# Begin Source File

SOURCE=.\nmm_SPM_Control.C
# End Source File
# Begin Source File

SOURCE=.\nmm_SPM_Report.C
# End Source File
# Begin Source File

SOURCE=.\nmm_TimestampList.C
# End Source File
# Begin Source File

SOURCE=.\optimize_now.C
# End Source File
# Begin Source File

SOURCE=.\relax.c
# End Source File
# Begin Source File

SOURCE=.\splat.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\active_set.h
# End Source File
# Begin Source File

SOURCE=.\AFMState.h
# End Source File
# Begin Source File

SOURCE=.\directstep.h
# End Source File
# Begin Source File

SOURCE=.\drift.h
# End Source File
# Begin Source File

SOURCE=.\nmm_AFM_Control.h
# End Source File
# Begin Source File

SOURCE=.\nmm_AFM_Report.h
# End Source File
# Begin Source File

SOURCE=.\nmm_AFMSIM_Report.h
# End Source File
# Begin Source File

SOURCE=.\nmm_AFMSIMSERVER_Report.h
# End Source File
# Begin Source File

SOURCE=.\nmm_Globals.h
# End Source File
# Begin Source File

SOURCE=.\nmm_GuardedscanClient.h
# End Source File
# Begin Source File

SOURCE=.\nmm_Microscope.h
# End Source File
# Begin Source File

SOURCE=.\nmm_MicroscopeRemote.h
# End Source File
# Begin Source File

SOURCE=.\nmm_MicroscopeRemote.hdef

!IF  "$(CFG)" == "nmMScope - Win32 Debug"

# Begin Custom Build
InputDir=.
WkspDir=.
InputPath=.\nmm_MicroscopeRemote.hdef
InputName=nmm_MicroscopeRemote

"$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	C:\cygwin\bin\bash --login -c "cd '$(WkspDir)$(InputDir)\app\nano\lib\nmMScope'; ../../../../../../vrpn/util/gen_rpc/gen_vrpn_rpc.pl $(InputName).Hdef"

# End Custom Build

!ELSEIF  "$(CFG)" == "nmMScope - Win32 Release"

# Begin Custom Build
InputDir=.
WkspDir=.
InputPath=.\nmm_MicroscopeRemote.hdef
InputName=nmm_MicroscopeRemote

"$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	C:\cygwin\bin\bash --login -c "cd '$(WkspDir)$(InputDir)\app\nano\lib\nmMScope'; ../../../../../../vrpn/util/gen_rpc/gen_vrpn_rpc.pl $(InputName).Hdef"

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\nmm_MicroscopeTranslator.h
# End Source File
# Begin Source File

SOURCE=.\nmm_QueueMonitor.h
# End Source File
# Begin Source File

SOURCE=.\nmm_RelaxComp.h
# End Source File
# Begin Source File

SOURCE=.\nmm_Sample.h
# End Source File
# Begin Source File

SOURCE=.\nmm_SimulatedMicroscope.h
# End Source File
# Begin Source File

SOURCE=.\nmm_SimulatedMicroscope_Remote.h
# End Source File
# Begin Source File

SOURCE=.\nmm_SPM_Control.h
# End Source File
# Begin Source File

SOURCE=.\nmm_SPM_Report.h
# End Source File
# Begin Source File

SOURCE=.\nmm_TimestampList.h
# End Source File
# Begin Source File

SOURCE=.\nmm_Types.h
# End Source File
# Begin Source File

SOURCE=.\optimize_now.h
# End Source File
# Begin Source File

SOURCE=.\relax.h
# End Source File
# Begin Source File

SOURCE=.\splat.h
# End Source File
# Begin Source File

SOURCE=.\stm_cmd.h
# End Source File
# Begin Source File

SOURCE=.\stm_file.h
# End Source File
# End Group
# End Target
# End Project

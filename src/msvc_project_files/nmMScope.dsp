# Microsoft Developer Studio Project File - Name="nmMScope" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=nmMScope - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "nmMScope.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "nmMScope.mak" CFG="nmMScope - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "nmMScope - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "nmMScope - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "nmMScope - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "nmMScope - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "." /I "../lib/ImgFormat" /I "../lib/nmBase" /I "../lib/nmImageViewer" /I "../lib/nmMP" /I "../lib/tclLinkVar" /I "../app/nano" /I "../app/nano/lib/nmAnalyze" /I "../app/nano/lib/nmAux" /I "../app/nano/lib/nmGraphics" /I "../app/nano/lib/nmMScope" /I "../app/nano/lib/nmReg" /I "../app/nano/lib/nmSEM" /I "../app/nano/lib/nmUGraphics" /I "../app/nano/lib/nmUI" /I "../../../external/pc_win32/include" /I "../../../external/pc_win32/include/ghost-stl" /I "../../../vrpn" /I "../../../quat" /I "../../../vogl" /D "_LIB" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "V_GLUT" /D "PROJECTIVE_TEXTURE" /D "NO_RAW_TERM" /D "NO_XWINDOWS" /D "NO_ITCL" /D "NO_FILTERS" /D "NO_EXT_TEXTURES" /D "USE_VRPN_MICROSCOPE" /YX /FD /GZ /TP /c
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

# Name "nmMScope - Win32 Release"
# Name "nmMScope - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\app\nano\lib\nmMScope\active_set.C
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmMScope\AFMState.C
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmMScope\directstep.C
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmMScope\drift.c
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmMScope\nmm_AFM_Control.C
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmMScope\nmm_AFM_Report.C
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmMScope\nmm_GuardedscanClient.C
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmMScope\nmm_Microscope.C
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmMScope\nmm_MicroscopeRemote.C
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmMScope\nmm_MicroscopeTranslator.C
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmMScope\nmm_QueueMonitor.C
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmMScope\nmm_RelaxComp.C
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmMScope\nmm_Sample.C
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmMScope\nmm_SPM_Control.C
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmMScope\nmm_SPM_Report.C
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmMScope\nmm_TimestampList.C
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmMScope\optimize_now.C
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmMScope\relax.c
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmMScope\splat.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\app\nano\lib\nmMScope\active_set.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmMScope\AFMState.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmMScope\directstep.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmMScope\drift.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmMScope\nmm_AFM_Control.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmMScope\nmm_AFM_Report.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmMScope\nmm_Globals.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmMScope\nmm_GuardedscanClient.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmMScope\nmm_Microscope.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmMScope\nmm_MicroscopeRemote.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmMScope\nmm_MicroscopeTranslator.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmMScope\nmm_QueueMonitor.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmMScope\nmm_RelaxComp.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmMScope\nmm_Sample.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmMScope\nmm_SPM_Control.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmMScope\nmm_SPM_Report.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmMScope\nmm_TimestampList.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmMScope\nmm_Types.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmMScope\optimize_now.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmMScope\relax.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmMScope\splat.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmMScope\stm_cmd.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmMScope\stm_file.h
# End Source File
# End Group
# End Target
# End Project

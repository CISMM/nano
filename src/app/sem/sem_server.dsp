# Microsoft Developer Studio Project File - Name="sem_server" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=sem_server - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "sem_server.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "sem_server.mak" CFG="sem_server - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "sem_server - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "sem_server - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "sem_server - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MLd /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /TP /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 vrpn.lib wsock32.lib quat.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "sem_server - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "pc_win32/Debug"
# PROP Intermediate_Dir "pc_win32/Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "." /I "../ebeamWriter" /I "../../../../vrpn" /I "../../../../external/pc_win32/include" /I "../../../../../external/pc_win32/include/stl" /I "../../../../quat" /I "../../lib/nmBase" /I "../../lib/nmMP" /I "../../lib/nmImageViewer" /I "../../lib/ImgFormat" /I "../../lib/tclLinkVar" /I "../nano/lib/nmReg" /I "../nano/lib/nmSEM" /I "../sem" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "V_GLUT" /D "VRPN_NO_STREAMS" /YX /FD /TP /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 vrpn.lib glut32_UNC.lib glu32.lib opengl32.lib wsock32.lib quat.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept /libpath:"../../../../external/pc_win32/lib" /libpath:"../../../../../external/pc_win32/lib" /libpath:"../../../../vrpn/pc_win32/Debug" /libpath:"../../../../quat/pc_win32/Debug"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "sem_server - Win32 Release"
# Name "sem_server - Win32 Debug"
# Begin Source File

SOURCE=.\clmctrl.h
# End Source File
# Begin Source File

SOURCE=.\delay.C
# End Source File
# Begin Source File

SOURCE=.\edax_server.C
# End Source File
# Begin Source File

SOURCE=..\ebeamWriter\exposurePattern.C
# End Source File
# Begin Source File

SOURCE=..\ebeamWriter\exposurePattern.h
# End Source File
# Begin Source File

SOURCE=..\ebeamWriter\exposureUtil.C
# End Source File
# Begin Source File

SOURCE=.\imgboard.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\nmBase\nmb_Device.C
# End Source File
# Begin Source File

SOURCE=..\..\lib\nmBase\nmb_Device.h
# End Source File
# Begin Source File

SOURCE=..\nano\lib\nmSEM\nmm_EDAX.C
# End Source File
# Begin Source File

SOURCE=..\nano\lib\nmSEM\nmm_EDAX.h
# End Source File
# Begin Source File

SOURCE=..\nano\lib\nmSEM\nmm_Microscope_SEM.C
# End Source File
# Begin Source File

SOURCE=..\nano\lib\nmSEM\nmm_Microscope_SEM.h
# End Source File
# Begin Source File

SOURCE=.\nmm_Microscope_SEM_EDAX.C
# End Source File
# Begin Source File

SOURCE=.\nmm_Microscope_SEM_EDAX.h
# End Source File
# Begin Source File

SOURCE=..\nano\lib\nmSEM\nmm_Microscope_SEM_Remote.C
# End Source File
# Begin Source File

SOURCE=..\nano\lib\nmSEM\nmm_Microscope_SEM_Remote.h
# End Source File
# Begin Source File

SOURCE=..\ebeamWriter\patternShape.C
# End Source File
# Begin Source File

SOURCE=.\semmsgid.h
# End Source File
# Begin Source File

SOURCE=.\srchfld.h
# End Source File
# Begin Source File

SOURCE=.\stgctrl.h
# End Source File
# Begin Source File

SOURCE=.\edaxfi32.lib
# End Source File
# Begin Source File

SOURCE=.\Pwedam32.lib
# End Source File
# Begin Source File

SOURCE=.\pwimg32.lib
# End Source File
# Begin Source File

SOURCE=.\stgctl32.lib
# End Source File
# Begin Source File

SOURCE=.\clmctl32.lib
# End Source File
# End Target
# End Project

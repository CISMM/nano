# Microsoft Developer Studio Project File - Name="vi_curve" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=vi_curve - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "vi_curve.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "vi_curve.mak" CFG="vi_curve - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "vi_curve - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "vi_curve - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "vi_curve - Win32 Release"

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
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "vi_curve - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MDd /W3 /GX /Z7 /Od /I "../../../../vrpn" /I "../../../../../vrpn" /I "../nano/lib/nmAux" /I "../../lib/nmBase" /I "../../lib/nmMP" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /FR /YX /FD /TP /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib winmm.lib vrpn.lib /nologo /subsystem:console /pdb:none /debug /machine:I386 /out:"Debug/vi_server.exe" /libpath:"..\..\..\..\vrpn\pc_win32\debug" /libpath:"..\..\..\..\..\vrpn\pc_win32\debug"

!ENDIF 

# Begin Target

# Name "vi_curve - Win32 Release"
# Name "vi_curve - Win32 Debug"
# Begin Source File

SOURCE=".\Decl-32.h"
# End Source File
# Begin Source File

SOURCE=.\GPIBServer.C
# End Source File
# Begin Source File

SOURCE=..\..\lib\nmBase\nmb_Device.C
# End Source File
# Begin Source File

SOURCE=..\..\lib\nmBase\nmb_Device.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\nmMP\thread.C
# End Source File
# Begin Source File

SOURCE=..\..\lib\nmMP\thread.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\nmMP\util.C
# End Source File
# Begin Source File

SOURCE=..\nano\lib\nmAux\vrpn_GPIBDevice.C
# End Source File
# Begin Source File

SOURCE=..\nano\lib\nmAux\vrpn_GPIBDevice.h
# End Source File
# Begin Source File

SOURCE=.\vrpn_GPIBDeviceServer.C
# End Source File
# Begin Source File

SOURCE=.\vrpn_GPIBDeviceServer.h
# End Source File
# Begin Source File

SOURCE=".\Gpib-32.lib"
# End Source File
# End Target
# End Project

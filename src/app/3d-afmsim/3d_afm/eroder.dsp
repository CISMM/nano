# Microsoft Developer Studio Project File - Name="eroder" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=eroder - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "eroder.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "eroder.mak" CFG="eroder - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "eroder - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "eroder - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "eroder - Win32 Release"

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
# ADD CPP /nologo /MT /W3 /GX /O2 /I "." /I "..\nano" /I "..\..\..\..\..\external\pc_win32\include" /I "..\..\..\..\..\vrpn" /I "..\..\nano\lib\nmMScope" /I "..\..\..\lib\nmBase" /I "..\..\..\lib\nmMP" /I "..\..\..\lib\tcllinkvar" /I "..\..\..\..\..\quat" /I "..\..\..\..\..\vogl" /I "..\..\nano\lib\nmUI" /I "..\..\nano\lib\nmGraphics" /I "..\..\..\lib\ImgFormat" /I "..\..\nano\lib\nmUGraphics" /I "..\..\..\..\..\vrpn\server_src" /I "..\..\..\..\..\external\pc_win32\include\ghost3.1" /I "..\..\..\..\..\external\pc_win32\include\ghost-stl" /I "..\..\nano\lib\nmReg" /I "..\..\..\lib\nmImageViewer" /I "..\..\nano\lib\nmAux" /I "..\..\nano\lib\nmSEM" /I "..\..\nano\lib\nmAnalyze" /I "..\..\..\..\vrpn" /I "..\..\lib\nmBase" /I "..\..\lib\tcllinkvar" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "eroder - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /Gi /GX /ZI /Od /I "." /I "..\..\..\lib\nmMP" /I "..\..\..\lib\nmBase" /I "..\nano" /I "..\..\..\..\..\external\pc_win32\include" /I "..\..\..\..\..\vrpn" /I "..\..\nano\lib\nmMScope" /I "..\..\..\lib\tcllinkvar" /I "..\..\..\..\..\quat" /I "..\..\..\..\..\vogl" /I "..\..\nano\lib\nmUGraphics" /I "..\..\..\..\..\vrpn\server_src" /I "..\..\nano\lib\nmAnalyze" /I "..\..\..\..\vrpn" /I "..\..\lib\tcllinkvar" /I "..\..\..\..\external\pc_win32\include" /I "..\nano\lib\nmUI" /I "..\nano\lib\nmUGraphics" /I "..\..\..\..\vrpn\server_src" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /FR /YX /FD /GZ /c /Tp
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib /nologo /stack:0x1000 /subsystem:console /debug /machine:I386 /nodefaultlib:"libcmtd" /pdbtype:sept
# SUBTRACT LINK32 /incremental:no /map /nodefaultlib

!ENDIF 

# Begin Target

# Name "eroder - Win32 Release"
# Name "eroder - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\ConeSphere.cpp
# End Source File
# Begin Source File

SOURCE=..\draw.cpp
# End Source File
# Begin Source File

SOURCE=..\erode.cpp
# End Source File
# Begin Source File

SOURCE=..\input.cpp
# End Source File
# Begin Source File

SOURCE=..\lightcol.cpp
# End Source File
# Begin Source File

SOURCE=..\main.cpp
# End Source File
# Begin Source File

SOURCE=..\..\nano\lib\nmMScope\nmm_SimulatedMicroscope.C
# End Source File
# Begin Source File

SOURCE=..\Tips.cpp
# End Source File
# Begin Source File

SOURCE=..\Unca.cpp
# End Source File
# Begin Source File

SOURCE=..\uncert.cpp
# End Source File
# Begin Source File

SOURCE=..\Uncertw.cpp
# End Source File
# Begin Source File

SOURCE=..\Vec3d.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\ConeSphere.h
# End Source File
# Begin Source File

SOURCE=..\defns.h
# End Source File
# Begin Source File

SOURCE=..\draw.h
# End Source File
# Begin Source File

SOURCE=..\erode.h
# End Source File
# Begin Source File

SOURCE=..\input.h
# End Source File
# Begin Source File

SOURCE=..\lightcol.h
# End Source File
# Begin Source File

SOURCE=..\main.h
# End Source File
# Begin Source File

SOURCE=..\..\nano\lib\nmMScope\nmm_SimulatedMicroscope.h
# End Source File
# Begin Source File

SOURCE=..\Tips.h
# End Source File
# Begin Source File

SOURCE=..\Unca.h
# End Source File
# Begin Source File

SOURCE=..\uncert.h
# End Source File
# Begin Source File

SOURCE=..\Uncertw.h
# End Source File
# Begin Source File

SOURCE=..\vec3d.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Group "Libraries-External"

# PROP Default_Filter ""
# Begin Group "TCL"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\..\..\external\pc_win32\lib\BLT24.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\external\pc_win32\lib\tk83.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\external\pc_win32\lib\tcl83.lib
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\..\..\..\external\pc_win32\lib\glut32_UNC.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\external\pc_win32\lib\GHOST31.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\quat\pc_win32\Debug\quat.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\vogl\pc_win32\Debug\vogl.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\vrpn\pc_win32\Debug\vrpn.lib
# End Source File
# End Group
# Begin Group "Libraries-Debug"

# PROP Default_Filter ""
# Begin Group "App/nano/lib-debug"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\..\obj\pc_win32\debug\app\nano\lib\nmAnalyze\nmAnalyze.lib

!IF  "$(CFG)" == "eroder - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "eroder - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\..\obj\pc_win32\debug\app\nano\lib\nmMScope\nmMScope.lib

!IF  "$(CFG)" == "eroder - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "eroder - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\..\obj\pc_win32\debug\app\nano\lib\nmUGraphics\nmUGraphics.lib

!IF  "$(CFG)" == "eroder - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "eroder - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\..\obj\pc_win32\debug\app\nano\lib\nmUI\nmUI.lib

!IF  "$(CFG)" == "eroder - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "eroder - Win32 Debug"

!ENDIF 

# End Source File
# End Group
# Begin Group "lib-debug"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\..\obj\pc_win32\debug\lib\nmBase\nmBase.lib

!IF  "$(CFG)" == "eroder - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "eroder - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\..\obj\pc_win32\debug\lib\nmImageViewer\nmImageViewer.lib

!IF  "$(CFG)" == "eroder - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "eroder - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\..\obj\pc_win32\debug\lib\nmMP\NMmp.lib

!IF  "$(CFG)" == "eroder - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "eroder - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\..\obj\pc_win32\debug\lib\tclLinkVar\tclLinkVar.lib

!IF  "$(CFG)" == "eroder - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "eroder - Win32 Debug"

!ENDIF 

# End Source File
# End Group
# End Group
# Begin Group "Libraries-Release"

# PROP Default_Filter ""
# Begin Group "app/nano/lib"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\..\obj\pc_win32\release\app\nano\lib\nmAnalyze\nmAnalyze.lib

!IF  "$(CFG)" == "eroder - Win32 Release"

!ELSEIF  "$(CFG)" == "eroder - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\..\obj\pc_win32\release\app\nano\lib\nmAux\nmAux.lib

!IF  "$(CFG)" == "eroder - Win32 Release"

!ELSEIF  "$(CFG)" == "eroder - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\..\obj\pc_win32\release\app\nano\lib\nmGraphics\nmGraphics.lib

!IF  "$(CFG)" == "eroder - Win32 Release"

!ELSEIF  "$(CFG)" == "eroder - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\..\obj\pc_win32\release\app\nano\lib\nmMScope\nmMScope.lib

!IF  "$(CFG)" == "eroder - Win32 Release"

!ELSEIF  "$(CFG)" == "eroder - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\..\obj\pc_win32\release\app\nano\lib\nmReg\nmReg.lib

!IF  "$(CFG)" == "eroder - Win32 Release"

!ELSEIF  "$(CFG)" == "eroder - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\..\obj\pc_win32\release\app\nano\lib\nmSEM\nmSEM.lib

!IF  "$(CFG)" == "eroder - Win32 Release"

!ELSEIF  "$(CFG)" == "eroder - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\..\obj\pc_win32\release\app\nano\lib\nmUGraphics\nmUGraphics.lib

!IF  "$(CFG)" == "eroder - Win32 Release"

!ELSEIF  "$(CFG)" == "eroder - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\..\obj\pc_win32\release\app\nano\lib\nmUI\nmUI.lib

!IF  "$(CFG)" == "eroder - Win32 Release"

!ELSEIF  "$(CFG)" == "eroder - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "lib"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\..\obj\pc_win32\release\lib\nmBase\nmBase.lib

!IF  "$(CFG)" == "eroder - Win32 Release"

!ELSEIF  "$(CFG)" == "eroder - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\..\obj\pc_win32\release\lib\nmImageViewer\nmImageViewer.lib

!IF  "$(CFG)" == "eroder - Win32 Release"

!ELSEIF  "$(CFG)" == "eroder - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\..\obj\pc_win32\release\lib\nmMP\NMmp.lib

!IF  "$(CFG)" == "eroder - Win32 Release"

!ELSEIF  "$(CFG)" == "eroder - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\..\obj\pc_win32\release\lib\tclLinkVar\tclLinkVar.lib

!IF  "$(CFG)" == "eroder - Win32 Release"

!ELSEIF  "$(CFG)" == "eroder - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# End Group
# End Target
# End Project

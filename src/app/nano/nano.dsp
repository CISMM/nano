# Microsoft Developer Studio Project File - Name="nano" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=nano - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "nano.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "nano.mak" CFG="nano - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "nano - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "nano - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "nano - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "nano - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../../obj/pc_win32/app/nano"
# PROP Intermediate_Dir "../../../obj/pc_win32/app/nano"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "." /I "..\..\..\..\vrpn" /I "..\..\lib\nmBase" /I "..\..\lib\tcllinkvar" /I "..\..\..\..\external\pc_win32\include" /I "..\..\..\..\quat" /I "..\..\..\..\vogl" /I "lib\nmUI" /I "lib\nmGraphics" /I "..\..\lib\ImgFormat" /I "lib\nmUGraphics" /I "..\..\..\..\vrpn\server_src" /I "..\..\..\..\external\pc_win32\include\ghost3.1" /I "..\..\..\..\external\pc_win32\include\ghost-stl" /I "lib\nmMScope" /I "..\..\lib\nmMP" /I "lib/nmReg" /I "..\..\lib\nmImageViewer" /I "lib\nmAux" /I "lib\nmSEM" /I "lib\nmAnalyze" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "USE_VRPN_MICROSCOPE" /D "V_GLUT" /D "NO_RAW_TERM" /D "NO_XWINDOWS" /D "NO_ITCL" /D "NO_FILTERS" /D "NO_EXT_TEXTURES" /D "PROJECTIVE_TEXTURE" /YX /FD /GZ /TP
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 oldnames.lib wsock32.lib gdi32.lib comdlg32.lib kernel32.lib kernel32.lib libcpmtd.lib libcimtd.lib libcmtd.lib nmAux.lib nmUGraphics.lib nmSEM.lib nmUI.lib nmGraphics.lib nmMScope.lib nmReg.lib nmImageViewer.lib nmMP.lib tclLinkVar.lib nmBase.lib ImgFormat.lib nmAnalyze.lib vrpn.lib vogl.lib quat.lib vrpn_phantom.lib ghost31.lib BLT24.lib tk83.lib tcl83.lib glut32_UNC.lib user32.lib glu32.lib opengl32.lib CORE_RL_lcms_.lib CORE_RL_magick_.lib CORE_RL_ttf_.lib CORE_RL_xlib_.lib CORE_RL_libxml_.lib /nologo /subsystem:console /debug /machine:I386 /nodefaultlib /pdbtype:sept /libpath:"..\..\..\..\external\pc_win32\lib" /libpath:"..\..\..\..\vrpn\pc_win32_MTd" /libpath:"..\..\..\..\quat\pc_win32_MTd" /libpath:"..\..\..\..\vogl\pc_win32_MTd\\" /libpath:"..\..\..\obj\pc_win32\app\nano\lib\nmAnalyze" /libpath:"..\..\..\obj\pc_win32\app\nano\lib\nmAux" /libpath:"..\..\..\obj\pc_win32\app\nano\lib\nmGraphics" /libpath:"..\..\..\obj\pc_win32\app\nano\lib\nmMScope" /libpath:"..\..\..\obj\pc_win32\app\nano\lib\nmReg" /libpath:"..\..\..\obj\pc_win32\app\nano\lib\nmSEM" /libpath:"..\..\..\obj\pc_win32\app\nano\lib\nmUGraphics" /libpath:"..\..\..\obj\pc_win32\app\nano\lib\nmUI" /libpath:"..\..\..\obj\pc_win32\lib\ImgFormat" /libpath:"..\..\..\obj\pc_win32\lib\nmImageViewer" /libpath:"..\..\..\obj\pc_win32\lib\nmMP" /libpath:"..\..\..\obj\pc_win32\lib\nmBase" /libpath:"..\..\..\obj\pc_win32\lib\tcllinkvar" /libpath:"../../../../vrpn/server_src/pc_win32_MTd"
# SUBTRACT LINK32 /verbose /pdb:none

!ENDIF 

# Begin Target

# Name "nano - Win32 Release"
# Name "nano - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\butt_mode.c
# End Source File
# Begin Source File

SOURCE=.\collaboration.C
# End Source File
# Begin Source File

SOURCE=.\CollaborationManager.C
# End Source File
# Begin Source File

SOURCE=.\error_display.C
# End Source File
# Begin Source File

SOURCE=.\globals.c
# End Source File
# Begin Source File

SOURCE=.\import.C
# End Source File
# Begin Source File

SOURCE=.\imported_obj.C
# End Source File
# Begin Source File

SOURCE=.\index_mode.C
# End Source File
# Begin Source File

SOURCE=.\interaction.c
# End Source File
# Begin Source File

SOURCE=.\microscape.c
# End Source File
# Begin Source File

SOURCE=.\microscopeHandlers.C
# End Source File
# Begin Source File

SOURCE=.\minit.c
# End Source File
# Begin Source File

SOURCE=.\nM_coord_change.C
# End Source File
# Begin Source File

SOURCE=.\normal.c
# End Source File
# Begin Source File

SOURCE=.\tcl_tk.c
# End Source File
# Begin Source File

SOURCE=.\updt_display.c
# End Source File
# Begin Source File

SOURCE=.\vrml.C
# End Source File
# Begin Source File

SOURCE=.\vrpn_MousePhantom.C
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\butt_mode.h
# End Source File
# Begin Source File

SOURCE=.\collaboration.h
# End Source File
# Begin Source File

SOURCE=.\CollaborationManager.h
# End Source File
# Begin Source File

SOURCE=.\error_display.h
# End Source File
# Begin Source File

SOURCE=.\globals.h
# End Source File
# Begin Source File

SOURCE=.\imported_obj.h
# End Source File
# Begin Source File

SOURCE=.\index_mode.h
# End Source File
# Begin Source File

SOURCE=.\interaction.h
# End Source File
# Begin Source File

SOURCE=.\microscape.h
# End Source File
# Begin Source File

SOURCE=.\microscopeHandlers.h
# End Source File
# Begin Source File

SOURCE=.\minit.h
# End Source File
# Begin Source File

SOURCE=.\nM_coord_change.h
# End Source File
# Begin Source File

SOURCE=.\normal.h
# End Source File
# Begin Source File

SOURCE=.\tcl_tk.h
# End Source File
# Begin Source File

SOURCE=.\termio.h
# End Source File
# Begin Source File

SOURCE=.\Timer.h
# End Source File
# Begin Source File

SOURCE=.\updt_display.h
# End Source File
# Begin Source File

SOURCE=.\vrml.h
# End Source File
# Begin Source File

SOURCE=.\vrpn_MousePhantom.h
# End Source File
# Begin Source File

SOURCE=.\x_aux.h
# End Source File
# Begin Source File

SOURCE=.\x_util.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\nano.rc
# End Source File
# Begin Source File

SOURCE=.\nano_unc.ico
# End Source File
# End Group
# End Target
# End Project

# Microsoft Developer Studio Project File - Name="nmAnalyze" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=nmAnalyze - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "nmAnalyze.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "nmAnalyze.mak" CFG="nmAnalyze - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "nmAnalyze - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "nmAnalyze - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 1
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "nmAnalyze - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../../../../obj/pc_win32/debug/app/nano/lib/nmAnalyze"
# PROP Intermediate_Dir "../../../../../obj/pc_win32/debug/app/nano/lib/nmAnalyze"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /GX /Zi /Od /I "../../../../lib/nmBase" /I "..\..\..\..\..\..\vrpn" /I "." /I "..\..\..\..\..\..\quat" /I "..\..\..\..\..\..\vogl" /I "..\..\..\..\..\..\external\pc_win32\include" /I "..\..\..\..\lib\nmBase" /I "..\..\..\..\lib\ImgFormat" /I "..\nmUGraphics" /I "..\.." /I "..\nmMScope" /I "..\..\..\..\lib\nmMP" /I "..\..\..\..\lib\tcllinkvar" /I "..\nmUI" /D "NDEBUG" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "USE_VRPN_MICROSCOPE" /D "V_GLUT" /D "NO_RAW_TERM" /D "NO_XWINDOWS" /D "NO_ITCL" /D "NO_FILTERS" /D "NO_EXT_TEXTURES" /D "PROJECTIVE_TEXTURE" /FR /YX /FD /I /GZ /TP /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "nmAnalyze - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "nmAnalyze___Win32_Release"
# PROP BASE Intermediate_Dir "nmAnalyze___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../../../../obj/pc_win32/release/app/nano/lib/nmAnalyze"
# PROP Intermediate_Dir "../../../../../obj/pc_win32/release/app/nano/lib/nmAnalyze"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /GX /Zi /Od /I "../../../../lib/nmBase" /I "..\..\..\..\..\..\vrpn" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "USE_VRPN_MICROSCOPE" /D "V_GLUT" /D "NO_RAW_TERM" /D "NO_XWINDOWS" /D "NO_ITCL" /D "NO_FILTERS" /D "NO_EXT_TEXTURES" /D "PROJECTIVE_TEXTURE" /YX /FD /I /GZ /TP /c
# ADD CPP /nologo /MT /W3 /GX /O2 /Ob2 /I "../../../../lib/nmBase" /I "..\..\..\..\..\..\vrpn" /D "_MBCS" /D "_LIB" /D "USE_VRPN_MICROSCOPE" /D "V_GLUT" /D "NO_RAW_TERM" /D "NO_XWINDOWS" /D "NO_ITCL" /D "NO_FILTERS" /D "NO_EXT_TEXTURES" /D "PROJECTIVE_TEXTURE" /YX /Zl /FD /TP /c
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

# Name "nmAnalyze - Win32 Debug"
# Name "nmAnalyze - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\cnt_filter.C
# End Source File
# Begin Source File

SOURCE=.\cnt_ia.C
# End Source File
# Begin Source File

SOURCE=.\nma_ppm.C
# End Source File
# Begin Source File

SOURCE=.\nma_ShapeAnalyze.C
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\cnt_ia.h
# End Source File
# Begin Source File

SOURCE=.\cnt_ps.h
# End Source File
# Begin Source File

SOURCE=.\nma_ShapeAnalyze.h
# End Source File
# Begin Source File

SOURCE=.\ppm.h
# End Source File
# End Group
# End Target
# End Project

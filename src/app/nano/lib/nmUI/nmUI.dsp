# Microsoft Developer Studio Project File - Name="nmUI" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=nmUI - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "nmUI.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "nmUI.mak" CFG="nmUI - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "nmUI - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "nmUI - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 1
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "nmUI - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../../../../obj/pc_win32/debug/app/nano/lib/nmUI"
# PROP Intermediate_Dir "../../../../../obj/pc_win32/debug/app/nano/lib/nmUI"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /I "." /I "\..\..\..\..\..\..\quat" /I "..\.." /I "..\..\..\..\lib\nmBase" /I "..\..\..\..\..\..\vrpn" /I "..\..\..\..\..\..\vogl" /I "..\..\..\..\lib\tclLinkVar" /I "..\..\..\..\..\..\external\pc_win32\include" /I "..\nmMScope" /I "..\..\..\..\lib\nmMP" /I "..\nmGraphics" /I "..\..\..\..\..\..\quat" /D "_DEBUG" /D "THIRDTECH" /D "_LIB" /D "_MBCS" /D "V_GLUT" /D "NO_RAW_TERM" /D "NO_XWINDOWS" /D "NO_ITCL" /D "NO_FILTERS" /D "NO_EXT_TEXTURES" /D "PROJECTIVE_TEXTURE" /D "VRPN_NO_STREAMS" /FR /YX /FD /GZ /TP /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "nmUI - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "nmUI___Win32_Release"
# PROP BASE Intermediate_Dir "nmUI___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../../../../obj/pc_win32/release/app/nano/lib/nmUI"
# PROP Intermediate_Dir "../../../../../obj/pc_win32/release/app/nano/lib/nmUI"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /GX /Zi /Od /I "." /I "\..\..\..\..\..\..\quat" /I "..\.." /I "..\..\..\..\lib\nmBase" /I "..\..\..\..\..\..\vrpn" /I "..\..\..\..\..\..\vogl" /I "..\..\..\..\lib\tclLinkVar" /I "..\..\..\..\..\..\external\pc_win32\include" /I "..\nmMScope" /I "..\..\..\..\lib\nmMP" /I "..\nmGraphics" /I "..\..\..\..\..\..\quat" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "USE_VRPN_MICROSCOPE" /D "V_GLUT" /D "NO_RAW_TERM" /D "NO_XWINDOWS" /D "NO_ITCL" /D "NO_FILTERS" /D "NO_EXT_TEXTURES" /D "PROJECTIVE_TEXTURE" /YX /FD /GZ /TP /c
# ADD CPP /nologo /MD /W3 /GX /O2 /Ob2 /I "." /I "\..\..\..\..\..\..\quat" /I "..\.." /I "..\..\..\..\lib\nmBase" /I "..\..\..\..\..\..\vrpn" /I "..\..\..\..\..\..\vogl" /I "..\..\..\..\lib\tclLinkVar" /I "..\..\..\..\..\..\external\pc_win32\include" /I "..\nmMScope" /I "..\..\..\..\lib\nmMP" /I "..\nmGraphics" /I "..\..\..\..\..\..\quat" /D "NDEBUG" /D "THIRDTECH" /D "_LIB" /D "_MBCS" /D "V_GLUT" /D "NO_RAW_TERM" /D "NO_XWINDOWS" /D "NO_ITCL" /D "NO_FILTERS" /D "NO_EXT_TEXTURES" /D "PROJECTIVE_TEXTURE" /D "VRPN_NO_STREAMS" /YX /Zl /FD /TP /c
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

# Name "nmUI - Win32 Debug"
# Name "nmUI - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\GraphMod.C
# End Source File
# Begin Source File

SOURCE=.\ModFile.C
# End Source File
# Begin Source File

SOURCE=.\nmui_ColorMap.C
# End Source File
# Begin Source File

SOURCE=.\nmui_Component.C
# End Source File
# Begin Source File

SOURCE=.\nmui_CrossSection.C
# End Source File
# Begin Source File

SOURCE=.\nmui_Haptics.C
# End Source File
# Begin Source File

SOURCE=.\nmui_HapticSurface.C
# End Source File
# Begin Source File

SOURCE=.\nmui_PlaneSync.C
# End Source File
# Begin Source File

SOURCE=.\nmui_SurfaceFeatures.C
# End Source File
# Begin Source File

SOURCE=.\nmui_Util.C
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\GraphMod.h
# End Source File
# Begin Source File

SOURCE=.\ModFile.h
# End Source File
# Begin Source File

SOURCE=.\nmui_ColorMap.h
# End Source File
# Begin Source File

SOURCE=.\nmui_Component.h
# End Source File
# Begin Source File

SOURCE=.\nmui_CrossSection.h
# End Source File
# Begin Source File

SOURCE=.\nmui_Haptics.h
# End Source File
# Begin Source File

SOURCE=.\nmui_HapticSurface.h
# End Source File
# Begin Source File

SOURCE=.\nmui_PlaneSync.h
# End Source File
# Begin Source File

SOURCE=.\nmui_SurfaceFeatures.h
# End Source File
# Begin Source File

SOURCE=.\nmui_Util.h
# End Source File
# End Group
# End Target
# End Project

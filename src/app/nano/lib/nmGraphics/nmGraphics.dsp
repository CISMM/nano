# Microsoft Developer Studio Project File - Name="nmGraphics" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=nmGraphics - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "nmGraphics.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "nmGraphics.mak" CFG="nmGraphics - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "nmGraphics - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "nmGraphics - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 1
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "nmGraphics - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../../../../../obj/pc_win32/release/app/nano/lib/nmGraphics"
# PROP Intermediate_Dir "../../../../../obj/pc_win32/release/app/nano/lib/nmGraphics"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /Ob2 /I "." /I "..\..\..\..\..\..\quat" /I "..\..\..\..\..\..\vogl" /I "C:\nsrg\external\pc_win32\include" /I "..\..\..\..\lib\nmBase" /I "..\..\..\..\..\..\vrpn" /I "..\nmUGraphics" /I "..\.." /I "..\nmMScope" /I "..\..\..\..\lib\nmMP" /I "..\..\..\..\lib\tcllinkvar" /I "..\nmUI" /D "_LIB" /D "NDEBUG" /D "_MBCS" /D "V_GLUT" /D "NO_RAW_TERM" /D "NO_XWINDOWS" /D "NO_ITCL" /D "NO_FILTERS" /D "NO_EXT_TEXTURES" /D "PROJECTIVE_TEXTURE" /YX /Zl /FD /TP /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "nmGraphics - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../../../../obj/pc_win32/debug/app/nano/lib/nmGraphics"
# PROP Intermediate_Dir "../../../../../obj/pc_win32/debug/app/nano/lib/nmGraphics"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /I "..\nmAnalyze" /I "." /I "..\..\..\..\..\..\quat" /I "..\..\..\..\..\..\vogl" /I "C:\nsrg\external\pc_win32\include" /I "..\..\..\..\lib\nmBase" /I "..\..\..\..\..\..\vrpn" /I "..\nmUGraphics" /I "..\.." /I "..\nmMScope" /I "..\..\..\..\lib\nmMP" /I "..\..\..\..\lib\tcllinkvar" /I "..\nmUI" /D "_LIB" /D "_DEBUG" /D "V_GLUT" /D "NO_RAW_TERM" /D "NO_XWINDOWS" /D "NO_ITCL" /D "NO_FILTERS" /D "NO_EXT_TEXTURES" /D "PROJECTIVE_TEXTURE" /D "_MBCS" /FR /YX /FD /GZ /TP /c
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

# Name "nmGraphics - Win32 Release"
# Name "nmGraphics - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\chartjunk.c
# End Source File
# Begin Source File

SOURCE=.\font.c
# End Source File
# Begin Source File

SOURCE=.\globjects.c
# End Source File
# Begin Source File

SOURCE=.\nmg_CloudTexturer.C
# End Source File
# Begin Source File

SOURCE=.\nmg_Funclist.C
# End Source File
# Begin Source File

SOURCE=.\nmg_Graphics.C
# End Source File
# Begin Source File

SOURCE=.\nmg_GraphicsImpl.C
# End Source File
# Begin Source File

SOURCE=.\nmg_GraphicsRemote.C
# End Source File
# Begin Source File

SOURCE=.\nmg_GraphicsTimer.C
# End Source File
# Begin Source File

SOURCE=.\nmg_haptic_graphics.c
# End Source File
# Begin Source File

SOURCE=.\nmg_ImageDisplayProjectiveTexture.C
# End Source File
# Begin Source File

SOURCE=.\nmg_RenderClient.C
# End Source File
# Begin Source File

SOURCE=.\nmg_RenderClientImpl.C
# End Source File
# Begin Source File

SOURCE=.\nmg_RenderServer.C
# End Source File
# Begin Source File

SOURCE=.\nmg_RenderServerStrategies.C
# End Source File
# Begin Source File

SOURCE=.\nmg_State.C
# End Source File
# Begin Source File

SOURCE=.\nmg_Surface.C
# End Source File
# Begin Source File

SOURCE=.\nmg_SurfaceMask.C
# End Source File
# Begin Source File

SOURCE=.\nmg_SurfaceRegion.C
# End Source File
# Begin Source File

SOURCE=.\openGL.c
# End Source File
# Begin Source File

SOURCE=.\surface_strip_create.c
# End Source File
# Begin Source File

SOURCE=.\surface_util.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\chartjunk.h
# End Source File
# Begin Source File

SOURCE=.\font.h
# End Source File
# Begin Source File

SOURCE=.\globjects.h
# End Source File
# Begin Source File

SOURCE=.\graphics.h
# End Source File
# Begin Source File

SOURCE=.\graphics_globals.h
# End Source File
# Begin Source File

SOURCE=.\nmg_CloudTexturer.h
# End Source File
# Begin Source File

SOURCE=.\nmg_Funclist.h
# End Source File
# Begin Source File

SOURCE=.\nmg_Globals.h
# End Source File
# Begin Source File

SOURCE=.\nmg_Graphics.h
# End Source File
# Begin Source File

SOURCE=.\nmg_GraphicsImpl.h
# End Source File
# Begin Source File

SOURCE=.\nmg_GraphicsNull.h
# End Source File
# Begin Source File

SOURCE=.\nmg_GraphicsRemote.h
# End Source File
# Begin Source File

SOURCE=.\nmg_GraphicsTimer.h
# End Source File
# Begin Source File

SOURCE=.\nmg_haptic_graphics.h
# End Source File
# Begin Source File

SOURCE=.\nmg_ImageDisplayProjectiveTexture.h
# End Source File
# Begin Source File

SOURCE=.\nmg_RenderClient.h
# End Source File
# Begin Source File

SOURCE=.\nmg_RenderClientImpl.h
# End Source File
# Begin Source File

SOURCE=.\nmg_RenderServer.h
# End Source File
# Begin Source File

SOURCE=.\nmg_RenderServerStrategies.h
# End Source File
# Begin Source File

SOURCE=.\nmg_State.h
# End Source File
# Begin Source File

SOURCE=.\nmg_Surface.h
# End Source File
# Begin Source File

SOURCE=.\nmg_SurfaceMask.h
# End Source File
# Begin Source File

SOURCE=.\nmg_SurfaceRegion.h
# End Source File
# Begin Source File

SOURCE=.\nmg_Types.h
# End Source File
# Begin Source File

SOURCE=.\openGL.h
# End Source File
# Begin Source File

SOURCE=.\surface_strip_create.h
# End Source File
# Begin Source File

SOURCE=.\surface_util.h
# End Source File
# End Group
# End Target
# End Project

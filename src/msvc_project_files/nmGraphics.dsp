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
# PROP AllowPerConfigDependencies 0
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

!ELSEIF  "$(CFG)" == "nmGraphics - Win32 Debug"

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

# Name "nmGraphics - Win32 Release"
# Name "nmGraphics - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\app\nano\lib\nmGraphics\chartjunk.c
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmGraphics\font.c
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmGraphics\globjects.c
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmGraphics\graphics.c
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmGraphics\graphics_globals.c
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmGraphics\nmg_CloudTexturer.C
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmGraphics\nmg_Funclist.C
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmGraphics\nmg_Graphics.C
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmGraphics\nmg_GraphicsImpl.C
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmGraphics\nmg_GraphicsRemote.C
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmGraphics\nmg_GraphicsTimer.C
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmGraphics\nmg_RenderClient.C
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmGraphics\nmg_RenderClientImpl.C
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmGraphics\nmg_RenderServer.C
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmGraphics\nmg_RenderServerStrategies.C
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmGraphics\nmg_Surface.C
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmGraphics\nmg_SurfaceMask.C
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmGraphics\nmg_SurfaceRegion.C
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmGraphics\openGL.c
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmGraphics\surface_strip_create.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\app\nano\lib\nmGraphics\chartjunk.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmGraphics\font.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmGraphics\globjects.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmGraphics\graphics.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmGraphics\graphics_globals.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmGraphics\nmg_CloudTexturer.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmGraphics\nmg_Funclist.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmGraphics\nmg_Globals.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmGraphics\nmg_Graphics.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmGraphics\nmg_GraphicsImpl.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmGraphics\nmg_GraphicsNull.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmGraphics\nmg_GraphicsRemote.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmGraphics\nmg_GraphicsTimer.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmGraphics\nmg_RenderClient.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmGraphics\nmg_RenderClientImpl.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmGraphics\nmg_RenderServer.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmGraphics\nmg_RenderServerStrategies.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmGraphics\nmg_Surface.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmGraphics\nmg_SurfaceMask.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmGraphics\nmg_SurfaceRegion.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmGraphics\nmg_Types.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmGraphics\openGL.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmGraphics\surface_strip_create.h
# End Source File
# End Group
# End Target
# End Project

# Microsoft Developer Studio Project File - Name="nmUGraphics" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=nmUGraphics - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "nmUGraphics.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "nmUGraphics.mak" CFG="nmUGraphics - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "nmUGraphics - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "nmUGraphics - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "nmUGraphics - Win32 Release"

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

!ELSEIF  "$(CFG)" == "nmUGraphics - Win32 Debug"

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

# Name "nmUGraphics - Win32 Release"
# Name "nmUGraphics - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\app\nano\lib\nmUGraphics\FileGenerator.C
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmUGraphics\GeomGenerator.C
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmUGraphics\MSIFileGenerator.C
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmUGraphics\TubeGenerator.C
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmUGraphics\URAxis.C
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmUGraphics\URender.C
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmUGraphics\URPolygon.C
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmUGraphics\URTexture.C
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmUGraphics\UTree.C
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmUGraphics\WaveFrontFileGenerator.C
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmUGraphics\Xform.C
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmUGraphics\Xform4x4.C
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\app\nano\lib\nmUGraphics\FileGenerator.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmUGraphics\GeomGenerator.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmUGraphics\MSIFileGenerator.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmUGraphics\TubeGenerator.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmUGraphics\URAxis.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmUGraphics\URender.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmUGraphics\URenderAux.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmUGraphics\URPolygon.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmUGraphics\URTexture.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmUGraphics\UTree.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmUGraphics\WaveFrontFileGenerator.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmUGraphics\Xform.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmUGraphics\Xform4x4.h
# End Source File
# End Group
# End Target
# End Project

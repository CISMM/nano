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
# PROP Output_Dir "../../../../../obj/pc_win32/app/nano/lib/nmUGraphics"
# PROP Intermediate_Dir "../../../../../obj/pc_win32/app/nano/lib/nmUGraphics"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /GX /Zi /Od /I "..\..\..\..\..\..\quat" /I "..\.." /I "..\..\..\..\lib\nmBase" /I "..\..\..\..\..\..\vrpn" /I "..\..\..\..\..\..\vogl" /I "..\..\..\..\lib\tclLinkVar" /I "..\..\..\..\..\..\external\pc_win32\include" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "USE_VRPN_MICROSCOPE" /D "V_GLUT" /D "NO_RAW_TERM" /D "NO_XWINDOWS" /D "NO_ITCL" /D "NO_FILTERS" /D "NO_EXT_TEXTURES" /D "PROJECTIVE_TEXTURE" /YX /FD /GZ /TP /c
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

SOURCE=.\FileGenerator.C
# End Source File
# Begin Source File

SOURCE=.\GeomGenerator.C
# End Source File
# Begin Source File

SOURCE=.\MSIFileGenerator.C
# End Source File
# Begin Source File

SOURCE=.\TubeGenerator.C
# End Source File
# Begin Source File

SOURCE=.\URAxis.C
# End Source File
# Begin Source File

SOURCE=.\URender.C
# End Source File
# Begin Source File

SOURCE=.\URPolygon.C
# End Source File
# Begin Source File

SOURCE=.\URTexture.C
# End Source File
# Begin Source File

SOURCE=.\UTree.C

!IF  "$(CFG)" == "nmUGraphics - Win32 Release"

!ELSEIF  "$(CFG)" == "nmUGraphics - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\WaveFrontFileGenerator.C
# End Source File
# Begin Source File

SOURCE=.\Xform.C
# End Source File
# Begin Source File

SOURCE=.\Xform4x4.C
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\FileGenerator.h
# End Source File
# Begin Source File

SOURCE=.\GeomGenerator.h
# End Source File
# Begin Source File

SOURCE=.\MSIFileGenerator.h
# End Source File
# Begin Source File

SOURCE=.\TubeGenerator.h
# End Source File
# Begin Source File

SOURCE=.\URAxis.h
# End Source File
# Begin Source File

SOURCE=.\URender.h
# End Source File
# Begin Source File

SOURCE=.\URenderAux.h
# End Source File
# Begin Source File

SOURCE=.\URPolygon.h
# End Source File
# Begin Source File

SOURCE=.\URTexture.h
# End Source File
# Begin Source File

SOURCE=.\UTree.h
# End Source File
# Begin Source File

SOURCE=.\WaveFrontFileGenerator.h
# End Source File
# Begin Source File

SOURCE=.\Xform.h
# End Source File
# Begin Source File

SOURCE=.\Xform4x4.h
# End Source File
# End Group
# End Target
# End Project

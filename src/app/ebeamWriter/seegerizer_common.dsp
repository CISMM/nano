# Microsoft Developer Studio Project File - Name="seegerizer_common" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=seegerizer_common - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "seegerizer_common.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "seegerizer_common.mak" CFG="seegerizer_common - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "seegerizer_common - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "seegerizer_common - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "seegerizer_common - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GX /O2 /I "../../../../external/pc_win32/include" /I "." /I "../../../../vrpn" /I "../../../../quat" /I "../../lib/nmBase" /I "../../lib/nmImageViewer" /I "../../lib/ImgFormat" /I "../../lib/tclLinkVar" /I "../../lib/nmMP" /I "../nano/lib/nmReg" /I "../nano/lib/nmSEM" /I "../nano/lib/nmUI" /I "../nano" /I "../sem" /D "VRPN_NO_STREAMS" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "V_GLUT" /D "NO_ITCL" /YX /FD /TP /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "seegerizer_common - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../../../external/pc_win32/include" /I "." /I "../../../../vrpn" /I "../../../../quat" /I "../../lib/nmBase" /I "../../lib/nmImageViewer" /I "../../lib/ImgFormat" /I "../../lib/tclLinkVar" /I "../../lib/nmMP" /I "../nano/lib/nmReg" /I "../nano/lib/nmSEM" /I "../nano/lib/nmUI" /I "../nano" /I "../sem" /D "VRPN_NO_STREAMS" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "V_GLUT" /D "NO_ITCL" /YX /FD /GZ /TP /c
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

# Name "seegerizer_common - Win32 Release"
# Name "seegerizer_common - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\controlPanels.C
# End Source File
# Begin Source File

SOURCE=..\sem\delay.C
# End Source File
# Begin Source File

SOURCE=.\exposurePattern.C
# End Source File
# Begin Source File

SOURCE=.\exposureUtil.C
# End Source File
# Begin Source File

SOURCE=.\main.C
# End Source File
# Begin Source File

SOURCE=.\patternEditor.C
# End Source File
# Begin Source File

SOURCE=.\patternFile.C
# End Source File
# Begin Source File

SOURCE=.\patternShape.C
# End Source File
# Begin Source File

SOURCE=.\transformFile.C
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\controlPanels.h
# End Source File
# Begin Source File

SOURCE=..\sem\delay.h
# End Source File
# Begin Source File

SOURCE=.\edgeTable.h
# End Source File
# Begin Source File

SOURCE=.\exposurePattern.h
# End Source File
# Begin Source File

SOURCE=.\exposureUtil.h
# End Source File
# Begin Source File

SOURCE=.\patternEditor.h
# End Source File
# Begin Source File

SOURCE=.\patternFile.h
# End Source File
# Begin Source File

SOURCE=.\patternShape.h
# End Source File
# Begin Source File

SOURCE=.\transformFile.h
# End Source File
# End Group
# End Target
# End Project

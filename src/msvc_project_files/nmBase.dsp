# Microsoft Developer Studio Project File - Name="nmBase" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=nmBase - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "nmBase.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "nmBase.mak" CFG="nmBase - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "nmBase - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "nmBase - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "nmBase - Win32 Release"

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

!ELSEIF  "$(CFG)" == "nmBase - Win32 Debug"

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

# Name "nmBase - Win32 Release"
# Name "nmBase - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\lib\nmBase\BCGrid.C
# End Source File
# Begin Source File

SOURCE=..\lib\nmBase\BCPlane.C
# End Source File
# Begin Source File

SOURCE=..\lib\nmBase\BCRenderGrid.C
# End Source File
# Begin Source File

SOURCE=..\lib\nmBase\BCString.C
# End Source File
# Begin Source File

SOURCE=..\lib\nmBase\filter.C
# End Source File
# Begin Source File

SOURCE=..\lib\nmBase\nmb_CalculatedPlane.C
# End Source File
# Begin Source File

SOURCE=..\lib\nmBase\nmb_ColorMap.C
# End Source File
# Begin Source File

SOURCE=..\lib\nmBase\nmb_Dataset.C
# End Source File
# Begin Source File

SOURCE=..\lib\nmBase\nmb_Debug.C
# End Source File
# Begin Source File

SOURCE=..\lib\nmBase\nmb_Decoration.C
# End Source File
# Begin Source File

SOURCE=..\lib\nmBase\nmb_Device.C
# End Source File
# Begin Source File

SOURCE=..\lib\nmBase\nmb_DeviceSequencer.C
# End Source File
# Begin Source File

SOURCE=..\lib\nmBase\nmb_FlattenedPlane.C
# End Source File
# Begin Source File

SOURCE=..\lib\nmBase\nmb_Image.C
# End Source File
# Begin Source File

SOURCE=..\lib\nmBase\nmb_ImgMagick.C
# End Source File
# Begin Source File

SOURCE=..\lib\nmBase\nmb_Interval.C
# End Source File
# Begin Source File

SOURCE=..\lib\nmBase\nmb_LBLFlattenedPlane.C
# End Source File
# Begin Source File

SOURCE=..\lib\nmBase\nmb_Line.C
# End Source File
# Begin Source File

SOURCE=..\lib\nmBase\nmb_PlaneSelection.C
# End Source File
# Begin Source File

SOURCE=..\lib\nmBase\nmb_SharedDevice.C
# End Source File
# Begin Source File

SOURCE=..\lib\nmBase\nmb_String.C
# End Source File
# Begin Source File

SOURCE=..\lib\nmBase\nmb_Subgrid.C
# End Source File
# Begin Source File

SOURCE=..\lib\nmBase\nmb_SummedPlane.C
# End Source File
# Begin Source File

SOURCE=..\lib\nmBase\nmb_Time.C
# End Source File
# Begin Source File

SOURCE=..\lib\nmBase\nmb_TimerList.C
# End Source File
# Begin Source File

SOURCE=..\lib\nmBase\nmb_Transform_TScShR.C
# End Source File
# Begin Source File

SOURCE=..\lib\nmBase\nmb_TransformMatrix44.C
# End Source File
# Begin Source File

SOURCE=..\lib\nmBase\Point.C
# End Source File
# Begin Source File

SOURCE=..\lib\nmBase\Position.C
# End Source File
# Begin Source File

SOURCE=..\lib\nmBase\PPM.C
# End Source File
# Begin Source File

SOURCE=..\lib\nmBase\Scanline.C
# End Source File
# Begin Source File

SOURCE=..\lib\nmBase\Topo.C
# End Source File
# Begin Source File

SOURCE=..\lib\nmBase\vc_dirent.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\lib\nmBase\afmstm.h
# End Source File
# Begin Source File

SOURCE=..\lib\nmBase\BCGrid.h
# End Source File
# Begin Source File

SOURCE=..\lib\nmBase\BCPlane.h
# End Source File
# Begin Source File

SOURCE=..\lib\nmBase\BCRenderGrid.h
# End Source File
# Begin Source File

SOURCE=..\lib\nmBase\BCString.h
# End Source File
# Begin Source File

SOURCE=..\lib\nmBase\filter.h
# End Source File
# Begin Source File

SOURCE=..\lib\nmBase\nmb_CalculatedPlane.h
# End Source File
# Begin Source File

SOURCE=..\lib\nmBase\nmb_ColorMap.h
# End Source File
# Begin Source File

SOURCE=..\lib\nmBase\nmb_Dataset.h
# End Source File
# Begin Source File

SOURCE=..\lib\nmBase\nmb_Debug.h
# End Source File
# Begin Source File

SOURCE=..\lib\nmBase\nmb_Decoration.h
# End Source File
# Begin Source File

SOURCE=..\lib\nmBase\nmb_Device.h
# End Source File
# Begin Source File

SOURCE=..\lib\nmBase\nmb_DeviceSequencer.h
# End Source File
# Begin Source File

SOURCE=..\lib\nmBase\nmb_FlattenedPlane.h
# End Source File
# Begin Source File

SOURCE=..\lib\nmBase\nmb_Globals.h
# End Source File
# Begin Source File

SOURCE=..\lib\nmBase\nmb_Image.h
# End Source File
# Begin Source File

SOURCE=..\lib\nmBase\nmb_ImgMagick.h
# End Source File
# Begin Source File

SOURCE=..\lib\nmBase\nmb_Interval.h
# End Source File
# Begin Source File

SOURCE=..\lib\nmBase\nmb_LBLFlattenedPlane.h
# End Source File
# Begin Source File

SOURCE=..\lib\nmBase\nmb_Line.h
# End Source File
# Begin Source File

SOURCE=..\lib\nmBase\nmb_PlaneSelection.h
# End Source File
# Begin Source File

SOURCE=..\lib\nmBase\nmb_SharedDevice.h
# End Source File
# Begin Source File

SOURCE=..\lib\nmBase\nmb_String.h
# End Source File
# Begin Source File

SOURCE=..\lib\nmBase\nmb_Subgrid.h
# End Source File
# Begin Source File

SOURCE=..\lib\nmBase\nmb_SummedPlane.h
# End Source File
# Begin Source File

SOURCE=..\lib\nmBase\nmb_Time.h
# End Source File
# Begin Source File

SOURCE=..\lib\nmBase\nmb_TimerList.h
# End Source File
# Begin Source File

SOURCE=..\lib\nmBase\nmb_Transform_TScShR.h
# End Source File
# Begin Source File

SOURCE=..\lib\nmBase\nmb_TransformMatrix44.h
# End Source File
# Begin Source File

SOURCE=..\lib\nmBase\nmb_Types.h
# End Source File
# Begin Source File

SOURCE=..\lib\nmBase\Point.h
# End Source File
# Begin Source File

SOURCE=..\lib\nmBase\Position.h
# End Source File
# Begin Source File

SOURCE=..\lib\nmBase\PPM.h
# End Source File
# Begin Source File

SOURCE=..\lib\nmBase\Scanline.h
# End Source File
# Begin Source File

SOURCE=..\lib\nmBase\Topo.h
# End Source File
# Begin Source File

SOURCE=..\lib\nmBase\vc_dirent.h
# End Source File
# End Group
# End Target
# End Project

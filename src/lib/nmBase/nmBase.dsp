# Microsoft Developer Studio Project File - Name="nmBase" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=nmBase - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "nmBase.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "nmBase.mak" CFG="nmBase - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "nmBase - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "nmBase - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 1
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "nmBase - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../../obj/pc_win32/debug/lib/nmBase"
# PROP Intermediate_Dir "../../../obj/pc_win32/debug/lib/nmBase"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /Zi /Od /I "." /I "../../app/image/protvol" /I "../nmMP" /I "..\..\..\..\vrpn" /I "..\..\..\..\quat" /I "..\..\..\..\external\pc_win32\include" /D "_DEBUG" /D "_LIB" /D "_MBCS" /D "V_GLUT" /D "NO_RAW_TERM" /D "NO_XWINDOWS" /D "NO_ITCL" /D "NO_FILTERS" /D "NO_EXT_TEXTURES" /D "PROJECTIVE_TEXTURE" /FR /YX /FD /GZ /TP /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "nmBase - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "nmBase___Win32_Release"
# PROP BASE Intermediate_Dir "nmBase___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../../obj/pc_win32/release/lib/nmBase"
# PROP Intermediate_Dir "../../../obj/pc_win32/release/lib/nmBase"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /GX /Zi /Od /I "." /I "../../app/image/protvol" /I "../nmMP" /I "..\..\..\..\vrpn" /I "..\..\..\..\quat" /I "..\..\..\..\external\pc_win32\include" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "USE_VRPN_MICROSCOPE" /D "V_GLUT" /D "NO_RAW_TERM" /D "NO_XWINDOWS" /D "NO_ITCL" /D "NO_FILTERS" /D "NO_EXT_TEXTURES" /D "PROJECTIVE_TEXTURE" /YX /FD /GZ /TP /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /Ob2 /I "." /I "../../app/image/protvol" /I "../nmMP" /I "..\..\..\..\vrpn" /I "..\..\..\..\quat" /I "..\..\..\..\external\pc_win32\include" /D "_LIB" /D "NDEBUG" /D "_MBCS" /D "V_GLUT" /D "NO_RAW_TERM" /D "NO_XWINDOWS" /D "NO_ITCL" /D "NO_FILTERS" /D "NO_EXT_TEXTURES" /D "PROJECTIVE_TEXTURE" /YX /Zl /FD /TP /c
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

# Name "nmBase - Win32 Debug"
# Name "nmBase - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\BCGrid.C
# End Source File
# Begin Source File

SOURCE=.\BCPlane.C
# End Source File
# Begin Source File

SOURCE=.\BCRenderGrid.C
# End Source File
# Begin Source File

SOURCE=.\filter.C
# End Source File
# Begin Source File

SOURCE=.\nmb_CalculatedPlane.C
# End Source File
# Begin Source File

SOURCE=.\nmb_ColorMap.C
# End Source File
# Begin Source File

SOURCE=.\nmb_Dataset.C
# End Source File
# Begin Source File

SOURCE=.\nmb_Debug.C
# End Source File
# Begin Source File

SOURCE=.\nmb_Decoration.C
# End Source File
# Begin Source File

SOURCE=.\nmb_Device.C
# End Source File
# Begin Source File

SOURCE=.\nmb_DeviceSequencer.C
# End Source File
# Begin Source File

SOURCE=.\nmb_FlattenedPlane.C
# End Source File
# Begin Source File

SOURCE=.\nmb_Image.C
# End Source File
# Begin Source File

SOURCE=.\nmb_ImageManager.C
# End Source File
# Begin Source File

SOURCE=.\nmb_ImgMagick.C
# End Source File
# Begin Source File

SOURCE=.\nmb_Interval.C
# End Source File
# Begin Source File

SOURCE=.\nmb_LBLFlattenedPlane.C
# End Source File
# Begin Source File

SOURCE=.\nmb_Line.C
# End Source File
# Begin Source File

SOURCE=.\nmb_MorphologyPlane.C
# End Source File
# Begin Source File

SOURCE=.\nmb_PlaneSelection.C
# End Source File
# Begin Source File

SOURCE=.\nmb_SharedDevice.C
# End Source File
# Begin Source File

SOURCE=.\nmb_String.C
# End Source File
# Begin Source File

SOURCE=.\nmb_Subgrid.C
# End Source File
# Begin Source File

SOURCE=.\nmb_SummedPlane.C
# End Source File
# Begin Source File

SOURCE=.\nmb_Time.C
# End Source File
# Begin Source File

SOURCE=.\nmb_TimerList.C
# End Source File
# Begin Source File

SOURCE=.\nmb_Transform_TScShR.C
# End Source File
# Begin Source File

SOURCE=.\nmb_TransformMatrix44.C
# End Source File
# Begin Source File

SOURCE=.\Point.C
# End Source File
# Begin Source File

SOURCE=.\Position.C
# End Source File
# Begin Source File

SOURCE=.\PPM.C
# End Source File
# Begin Source File

SOURCE=.\readNanoscopeFile.C
# End Source File
# Begin Source File

SOURCE=.\readNanotecFile.C
# End Source File
# Begin Source File

SOURCE=.\Scanline.C
# End Source File
# Begin Source File

SOURCE=.\Topo.C
# End Source File
# Begin Source File

SOURCE=.\vc_dirent.c
# End Source File
# Begin Source File

SOURCE=.\wsxmHeader.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\afmstm.h
# End Source File
# Begin Source File

SOURCE=.\BCGrid.h
# End Source File
# Begin Source File

SOURCE=.\BCPlane.h
# End Source File
# Begin Source File

SOURCE=.\BCRenderGrid.h
# End Source File
# Begin Source File

SOURCE=.\filter.h
# End Source File
# Begin Source File

SOURCE=.\nmb_CalculatedPlane.h
# End Source File
# Begin Source File

SOURCE=.\nmb_ColorMap.h
# End Source File
# Begin Source File

SOURCE=.\nmb_Dataset.h
# End Source File
# Begin Source File

SOURCE=.\nmb_Debug.h
# End Source File
# Begin Source File

SOURCE=.\nmb_Decoration.h
# End Source File
# Begin Source File

SOURCE=.\nmb_Device.h
# End Source File
# Begin Source File

SOURCE=.\nmb_DeviceSequencer.h
# End Source File
# Begin Source File

SOURCE=.\nmb_FlattenedPlane.h
# End Source File
# Begin Source File

SOURCE=.\nmb_Globals.h
# End Source File
# Begin Source File

SOURCE=.\nmb_Image.h
# End Source File
# Begin Source File

SOURCE=.\nmb_ImageDisplay.h
# End Source File
# Begin Source File

SOURCE=.\nmb_ImageManager.h
# End Source File
# Begin Source File

SOURCE=.\nmb_ImgMagick.h
# End Source File
# Begin Source File

SOURCE=.\nmb_Interval.h
# End Source File
# Begin Source File

SOURCE=.\nmb_LBLFlattenedPlane.h
# End Source File
# Begin Source File

SOURCE=.\nmb_Line.h
# End Source File
# Begin Source File

SOURCE=.\nmb_MorphologyPlane.h
# End Source File
# Begin Source File

SOURCE=.\nmb_PlaneSelection.h
# End Source File
# Begin Source File

SOURCE=.\nmb_SharedDevice.h
# End Source File
# Begin Source File

SOURCE=.\nmb_String.h
# End Source File
# Begin Source File

SOURCE=.\nmb_Subgrid.h
# End Source File
# Begin Source File

SOURCE=.\nmb_SummedPlane.h
# End Source File
# Begin Source File

SOURCE=.\nmb_Time.h
# End Source File
# Begin Source File

SOURCE=.\nmb_TimerList.h
# End Source File
# Begin Source File

SOURCE=.\nmb_Transform_TScShR.h
# End Source File
# Begin Source File

SOURCE=.\nmb_TransformMatrix44.h
# End Source File
# Begin Source File

SOURCE=.\nmb_Types.h
# End Source File
# Begin Source File

SOURCE=.\Point.h
# End Source File
# Begin Source File

SOURCE=.\Position.h
# End Source File
# Begin Source File

SOURCE=.\PPM.h
# End Source File
# Begin Source File

SOURCE=.\readNanoscopeFile.h
# End Source File
# Begin Source File

SOURCE=.\readNanotecFile.h
# End Source File
# Begin Source File

SOURCE=.\Scanline.h
# End Source File
# Begin Source File

SOURCE=.\Topo.h
# End Source File
# Begin Source File

SOURCE=.\vc_dirent.h
# End Source File
# Begin Source File

SOURCE=.\wsxmHeader.h
# End Source File
# End Group
# End Target
# End Project

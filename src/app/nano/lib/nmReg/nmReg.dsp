# Microsoft Developer Studio Project File - Name="nmReg" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=nmReg - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "nmReg.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "nmReg.mak" CFG="nmReg - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "nmReg - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "nmReg - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 1
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "nmReg - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../../../../obj/pc_win32/debug/app/nano/lib/nmReg"
# PROP Intermediate_Dir "../../../../../obj/pc_win32/debug/app/nano/lib/nmReg"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /GX /Zi /Od /I "." /I "..\..\..\..\..\..\vrpn" /I "..\..\..\..\lib\nmImageViewer" /I "..\..\..\..\lib\nmBase" /I "..\..\..\..\..\..\quat" /I "..\..\..\..\lib\nmMP" /I "..\nmGraphics" /I "..\..\..\..\..\..\external\pc_win32\include" /I "..\..\..\..\lib\ImgFormat" /I "..\..\..\..\..\..\vogl" /I "..\..\..\..\lib\tcllinkvar" /I "..\.." /I "..\nmUI" /D "NDEBUG" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "USE_VRPN_MICROSCOPE" /D "V_GLUT" /D "NO_RAW_TERM" /D "NO_XWINDOWS" /D "NO_ITCL" /D "NO_FILTERS" /D "NO_EXT_TEXTURES" /D "PROJECTIVE_TEXTURE" /YX /FD /GZ /TP /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "nmReg - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "nmReg___Win32_Release"
# PROP BASE Intermediate_Dir "nmReg___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../../../../obj/pc_win32/release/app/nano/lib/nmReg"
# PROP Intermediate_Dir "../../../../../obj/pc_win32/release/app/nano/lib/nmReg"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /GX /Zi /Od /I "." /I "..\..\..\..\..\..\vrpn" /I "..\..\..\..\lib\nmImageViewer" /I "..\..\..\..\lib\nmBase" /I "..\..\..\..\..\..\quat" /I "..\..\..\..\lib\nmMP" /I "..\nmGraphics" /I "..\..\..\..\..\..\external\pc_win32\include" /I "..\..\..\..\lib\ImgFormat" /I "..\..\..\..\..\..\vogl" /I "..\..\..\..\lib\tcllinkvar" /I "..\.." /I "..\nmUI" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "USE_VRPN_MICROSCOPE" /D "V_GLUT" /D "NO_RAW_TERM" /D "NO_XWINDOWS" /D "NO_ITCL" /D "NO_FILTERS" /D "NO_EXT_TEXTURES" /D "PROJECTIVE_TEXTURE" /YX /FD /GZ /TP /c
# ADD CPP /nologo /MT /W3 /GX /O2 /Ob2 /I "." /I "..\..\..\..\..\..\vrpn" /I "..\..\..\..\lib\nmImageViewer" /I "..\..\..\..\lib\nmBase" /I "..\..\..\..\..\..\quat" /I "..\..\..\..\lib\nmMP" /I "..\nmGraphics" /I "..\..\..\..\..\..\external\pc_win32\include" /I "..\..\..\..\lib\ImgFormat" /I "..\..\..\..\..\..\vogl" /I "..\..\..\..\lib\tcllinkvar" /I "..\.." /I "..\nmUI" /D "_MBCS" /D "_LIB" /D "USE_VRPN_MICROSCOPE" /D "V_GLUT" /D "NO_RAW_TERM" /D "NO_XWINDOWS" /D "NO_ITCL" /D "NO_FILTERS" /D "NO_EXT_TEXTURES" /D "PROJECTIVE_TEXTURE" /YX /Zl /FD /TP /c
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

# Name "nmReg - Win32 Debug"
# Name "nmReg - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\blas_extract.c
# End Source File
# Begin Source File

SOURCE=.\correspondence.C
# End Source File
# Begin Source File

SOURCE=.\correspondenceEditor.C
# End Source File
# Begin Source File

SOURCE=.\dgels.c
# End Source File
# Begin Source File

SOURCE=.\dgglse.c
# End Source File
# Begin Source File

SOURCE=.\f2c_extract.c
# End Source File
# Begin Source File

SOURCE=.\linLeastSqr.C
# End Source File
# Begin Source File

SOURCE=.\nmr_AlignerMI.C
# End Source File
# Begin Source File

SOURCE=.\nmr_Gaussian.C
# End Source File
# Begin Source File

SOURCE=.\nmr_MultiResObjectiveMI.C
# End Source File
# Begin Source File

SOURCE=.\nmr_ObjectiveMI.C
# End Source File
# Begin Source File

SOURCE=.\nmr_Registration_Client.C
# End Source File
# Begin Source File

SOURCE=.\nmr_Registration_Impl.C
# End Source File
# Begin Source File

SOURCE=.\nmr_Registration_ImplUI.C
# End Source File
# Begin Source File

SOURCE=.\nmr_Registration_Interface.C
# End Source File
# Begin Source File

SOURCE=.\nmr_Registration_Proxy.C
# End Source File
# Begin Source File

SOURCE=.\nmr_Registration_Server.C
# End Source File
# Begin Source File

SOURCE=.\nmr_RegistrationUI.C
# End Source File
# Begin Source File

SOURCE=.\nmr_Util.C
# End Source File
# Begin Source File

SOURCE=.\transformSolve.C
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\correspondence.h
# End Source File
# Begin Source File

SOURCE=.\correspondenceEditor.h
# End Source File
# Begin Source File

SOURCE=.\f2c.h
# End Source File
# Begin Source File

SOURCE=.\linLeastSqr.h
# End Source File
# Begin Source File

SOURCE=.\nmr_AlignerMI.h
# End Source File
# Begin Source File

SOURCE=.\nmr_Gaussian.h
# End Source File
# Begin Source File

SOURCE=.\nmr_MultiResObjectiveMI.h
# End Source File
# Begin Source File

SOURCE=.\nmr_ObjectiveMI.h
# End Source File
# Begin Source File

SOURCE=.\nmr_Registration_Client.h
# End Source File
# Begin Source File

SOURCE=.\nmr_Registration_Impl.h
# End Source File
# Begin Source File

SOURCE=.\nmr_Registration_ImplUI.h
# End Source File
# Begin Source File

SOURCE=.\nmr_Registration_Interface.h
# End Source File
# Begin Source File

SOURCE=.\nmr_Registration_Proxy.h
# End Source File
# Begin Source File

SOURCE=.\nmr_Registration_Server.h
# End Source File
# Begin Source File

SOURCE=.\nmr_RegistrationUI.h
# End Source File
# Begin Source File

SOURCE=.\nmr_Util.h
# End Source File
# Begin Source File

SOURCE=.\transformSolve.h
# End Source File
# End Group
# End Target
# End Project

# Microsoft Developer Studio Project File - Name="nmReg" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=nmReg - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "nmReg.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "nmReg.mak" CFG="nmReg - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "nmReg - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "nmReg - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "nmReg - Win32 Release"

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

!ELSEIF  "$(CFG)" == "nmReg - Win32 Debug"

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

# Name "nmReg - Win32 Release"
# Name "nmReg - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\app\nano\lib\nmReg\blas_extract.c
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmReg\correspondence.C
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmReg\correspondenceEditor.C
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmReg\dgels.c
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmReg\dgglse.c
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmReg\f2c_extract.c
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmReg\linLeastSqr.C
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmReg\nmr_AlignerMI.C
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmReg\nmr_CoarseToFineSearch.C
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmReg\nmr_Gaussian.C
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmReg\nmr_Registration_Client.C
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmReg\nmr_Registration_Impl.C
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmReg\nmr_Registration_ImplUI.C
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmReg\nmr_Registration_Interface.C
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmReg\nmr_Registration_Proxy.C
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmReg\nmr_Registration_Server.C
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmReg\nmr_RegistrationUI.C
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmReg\nmr_Util.C
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmReg\transformSolve.C
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\app\nano\lib\nmReg\correspondence.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmReg\correspondenceEditor.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmReg\f2c.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmReg\linLeastSqr.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmReg\nmr_AlignerMI.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmReg\nmr_CoarseToFineSearch.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmReg\nmr_Gaussian.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmReg\nmr_Registration_Client.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmReg\nmr_Registration_Impl.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmReg\nmr_Registration_ImplUI.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmReg\nmr_Registration_Interface.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmReg\nmr_Registration_Proxy.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmReg\nmr_Registration_Server.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmReg\nmr_RegistrationUI.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmReg\nmr_Util.h
# End Source File
# Begin Source File

SOURCE=..\app\nano\lib\nmReg\transformSolve.h
# End Source File
# End Group
# End Target
# End Project

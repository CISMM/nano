# Microsoft Developer Studio Project File - Name="seegerizer" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=seegerizer - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "seegerizer.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "seegerizer.mak" CFG="seegerizer - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "seegerizer - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "seegerizer - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "seegerizer - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "seegerizer - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "." /I "../../../../external/pc_win32/include" /I "../../../../../external/pc_win32/include" /I "../../../../external/pc_win32/include/stl" /I "../../../../../external/pc_win32/include/stl" /I "../../../../vrpn" /I "../../../../quat" /I "../../lib/nmBase" /I "../../lib/nmImageViewer" /I "../../lib/ImgFormat" /I "../../lib/tclLinkVar" /I "../nano/lib/nmReg" /I "../nano/lib/nmSEM" /I "../sem" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "V_GLUT" /YX /FD /GZ /TP /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 vrpn.lib wsock32.lib glut32_UNC.lib glu32.lib opengl32.lib BLT24.lib tcl82.lib tk82.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept /libpath:"../../../../external/pc_win32/lib" /libpath:"../../../../../external/pc_win32/lib" /libpath:"../../../../vrpn/pc_win32_MTd"

!ENDIF 

# Begin Target

# Name "seegerizer - Win32 Release"
# Name "seegerizer - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\lib\ImgFormat\AbstractImage.C
# End Source File
# Begin Source File

SOURCE=..\..\lib\nmBase\BCGrid.C
# End Source File
# Begin Source File

SOURCE=..\..\lib\nmBase\BCPlane.C
# End Source File
# Begin Source File

SOURCE=..\..\lib\nmBase\BCString.C
# End Source File
# Begin Source File

SOURCE=..\nano\lib\nmReg\blas_extract.c
# End Source File
# Begin Source File

SOURCE=.\controlPanels.C
# End Source File
# Begin Source File

SOURCE=..\nano\lib\nmReg\correspondence.C
# End Source File
# Begin Source File

SOURCE=..\nano\lib\nmReg\correspondenceEditor.C
# End Source File
# Begin Source File

SOURCE=..\nano\lib\nmReg\dgels.c
# End Source File
# Begin Source File

SOURCE=..\nano\lib\nmReg\dgglse.c
# End Source File
# Begin Source File

SOURCE=.\exposureManager.C
# End Source File
# Begin Source File

SOURCE=.\exposurePattern.C
# End Source File
# Begin Source File

SOURCE=..\nano\lib\nmReg\f2c_extract.c
# End Source File
# Begin Source File

SOURCE=..\..\lib\ImgFormat\ImageMaker.C
# End Source File
# Begin Source File

SOURCE=..\..\lib\nmImageViewer\imageViewer.C
# End Source File
# Begin Source File

SOURCE=..\nano\lib\nmReg\linLeastSqr.C
# End Source File
# Begin Source File

SOURCE=.\main.C
# End Source File
# Begin Source File

SOURCE=..\..\lib\nmBase\nmb_Debug.C
# End Source File
# Begin Source File

SOURCE=..\..\lib\nmBase\nmb_Device.C
# End Source File
# Begin Source File

SOURCE=..\..\lib\nmBase\nmb_Image.C
# End Source File
# Begin Source File

SOURCE=..\..\lib\nmBase\nmb_String.C
# End Source File
# Begin Source File

SOURCE=..\..\lib\nmBase\nmb_TimerList.C
# End Source File
# Begin Source File

SOURCE=..\..\lib\nmBase\nmb_Transform_TScShR.C
# End Source File
# Begin Source File

SOURCE=..\..\lib\nmBase\nmb_TransformMatrix44.C
# End Source File
# Begin Source File

SOURCE=..\nano\lib\nmSEM\nmm_EDAX.C
# End Source File
# Begin Source File

SOURCE=..\nano\lib\nmSEM\nmm_Microscope_SEM.C
# End Source File
# Begin Source File

SOURCE=..\nano\lib\nmSEM\nmm_Microscope_SEM_Remote.C
# End Source File
# Begin Source File

SOURCE=..\nano\lib\nmReg\nmr_AlignerMI.C
# End Source File
# Begin Source File

SOURCE=..\nano\lib\nmReg\nmr_CoarseToFineSearch.C
# End Source File
# Begin Source File

SOURCE=..\nano\lib\nmReg\nmr_Gaussian.C
# End Source File
# Begin Source File

SOURCE=..\nano\lib\nmReg\nmr_Registration_Client.C
# End Source File
# Begin Source File

SOURCE=..\nano\lib\nmReg\nmr_Registration_Impl.C
# End Source File
# Begin Source File

SOURCE=..\nano\lib\nmReg\nmr_Registration_ImplUI.C
# End Source File
# Begin Source File

SOURCE=..\nano\lib\nmReg\nmr_Registration_Interface.C
# End Source File
# Begin Source File

SOURCE=..\nano\lib\nmReg\nmr_Registration_Proxy.C
# End Source File
# Begin Source File

SOURCE=..\nano\lib\nmReg\nmr_Registration_Server.C
# End Source File
# Begin Source File

SOURCE=..\nano\lib\nmReg\nmr_Util.C
# End Source File
# Begin Source File

SOURCE=.\patternEditor.C
# End Source File
# Begin Source File

SOURCE=..\..\lib\ImgFormat\PNMImage.C
# End Source File
# Begin Source File

SOURCE=..\..\lib\nmBase\PPM.C
# End Source File
# Begin Source File

SOURCE=..\..\lib\tclLinkVar\Tcl_Linkvar.C
# End Source File
# Begin Source File

SOURCE=..\..\lib\tclLinkVar\Tcl_Netvar.C
# End Source File
# Begin Source File

SOURCE=..\..\lib\ImgFormat\TIFFImage.C
# End Source File
# Begin Source File

SOURCE=..\..\lib\nmBase\Topo.C
# End Source File
# Begin Source File

SOURCE=.\transformFile.C
# End Source File
# Begin Source File

SOURCE=..\nano\lib\nmReg\transformSolve.C
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\lib\ImgFormat\AbstractImage.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\nmBase\BCGrid.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\nmBase\BCPlane.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\nmBase\BCString.h
# End Source File
# Begin Source File

SOURCE=.\controlPanels.h
# End Source File
# Begin Source File

SOURCE=..\nano\lib\nmReg\correspondence.h
# End Source File
# Begin Source File

SOURCE=..\nano\lib\nmReg\correspondenceEditor.h
# End Source File
# Begin Source File

SOURCE=.\exposureManager.h
# End Source File
# Begin Source File

SOURCE=.\exposurePattern.h
# End Source File
# Begin Source File

SOURCE=..\nano\lib\nmReg\f2c.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\ImgFormat\ImageMaker.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\nmImageViewer\imageViewer.h
# End Source File
# Begin Source File

SOURCE=..\nano\lib\nmReg\linLeastSqr.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\nmBase\nmb_Debug.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\nmBase\nmb_Device.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\nmBase\nmb_Image.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\nmBase\nmb_String.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\nmBase\nmb_TimerList.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\nmBase\nmb_Transform_TScShR.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\nmBase\nmb_TransformMatrix44.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\nmBase\nmb_Types.h
# End Source File
# Begin Source File

SOURCE=..\nano\lib\nmSEM\nmm_EDAX.h
# End Source File
# Begin Source File

SOURCE=..\nano\lib\nmSEM\nmm_Microscope_SEM.h
# End Source File
# Begin Source File

SOURCE=..\nano\lib\nmSEM\nmm_Microscope_SEM_Remote.h
# End Source File
# Begin Source File

SOURCE=..\nano\lib\nmReg\nmr_Gaussian.h
# End Source File
# Begin Source File

SOURCE=..\nano\lib\nmReg\nmr_Registration_Client.h
# End Source File
# Begin Source File

SOURCE=..\nano\lib\nmReg\nmr_Registration_Impl.h
# End Source File
# Begin Source File

SOURCE=..\nano\lib\nmReg\nmr_Registration_ImplUI.h
# End Source File
# Begin Source File

SOURCE=..\nano\lib\nmReg\nmr_Registration_Interface.h
# End Source File
# Begin Source File

SOURCE=..\nano\lib\nmReg\nmr_Registration_Proxy.h
# End Source File
# Begin Source File

SOURCE=..\nano\lib\nmReg\nmr_Registration_Server.h
# End Source File
# Begin Source File

SOURCE=..\nano\lib\nmReg\nmr_RegistrationUI.h
# End Source File
# Begin Source File

SOURCE=..\nano\lib\nmReg\nmr_Util.h
# End Source File
# Begin Source File

SOURCE=.\patternEditor.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\ImgFormat\PixelBuffer.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\ImgFormat\PNMImage.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\nmBase\PPM.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\tclLinkVar\Tcl_Linkvar.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\tclLinkVar\Tcl_Netvar.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\ImgFormat\TIFFImage.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\nmBase\Topo.h
# End Source File
# Begin Source File

SOURCE=.\transformFile.h
# End Source File
# Begin Source File

SOURCE=..\nano\lib\nmReg\transformSolve.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Group "EDAX"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\sem\clmctrl.h
# End Source File
# Begin Source File

SOURCE=..\sem\imgboard.h
# End Source File
# Begin Source File

SOURCE=..\sem\nmm_Microscope_SEM_EDAX.C

!IF  "$(CFG)" == "seegerizer - Win32 Release"

!ELSEIF  "$(CFG)" == "seegerizer - Win32 Debug"

# ADD CPP /D "VIRTUAL_SEM"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\sem\nmm_Microscope_SEM_EDAX.h
# End Source File
# Begin Source File

SOURCE=..\sem\semmsgid.h
# End Source File
# Begin Source File

SOURCE=..\sem\srchfld.h
# End Source File
# Begin Source File

SOURCE=..\sem\stgctrl.h
# End Source File
# Begin Source File

SOURCE=..\sem\Pwedam32.lib
# End Source File
# Begin Source File

SOURCE=..\sem\pwimg32.lib
# End Source File
# Begin Source File

SOURCE=..\sem\stgctl32.lib
# End Source File
# Begin Source File

SOURCE=..\sem\edaxfi32.lib
# End Source File
# End Group
# End Target
# End Project

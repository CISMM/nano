# Microsoft Developer Studio Project File - Name="regserver" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=regserver - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "regserver.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "regserver.mak" CFG="regserver - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "regserver - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "regserver - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "regserver - Win32 Release"

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
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "regserver - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../../../vrpn" /I "../../../../quat" /I "../../../../../external/pc_win32/include" /I "../nano/lib/nmReg" /I "../../lib/nmImageViewer" /I "../../lib/nmBase" /I "../../lib/ImgFormat" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "V_GLUT" /YX /FD /TP /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 glut32.lib opengl32.lib vrpn.lib wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept /libpath:"../../../../../external/pc_win32/lib" /libpath:"../../../../vrpn/pc_win32/debug"

!ENDIF 

# Begin Target

# Name "regserver - Win32 Release"
# Name "regserver - Win32 Debug"
# Begin Source File

SOURCE=..\..\lib\nmBase\BCGrid.C
# End Source File
# Begin Source File

SOURCE=..\..\lib\nmBase\BCGrid.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\nmBase\BCPlane.C
# End Source File
# Begin Source File

SOURCE=..\..\lib\nmBase\BCPlane.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\nmBase\BCString.C
# End Source File
# Begin Source File

SOURCE=..\..\lib\nmBase\BCString.h
# End Source File
# Begin Source File

SOURCE=..\nano\lib\nmReg\blas_extract.c
# End Source File
# Begin Source File

SOURCE=..\nano\lib\nmReg\correspondence.C
# End Source File
# Begin Source File

SOURCE=..\nano\lib\nmReg\correspondence.h
# End Source File
# Begin Source File

SOURCE=..\nano\lib\nmReg\correspondenceEditor.C
# End Source File
# Begin Source File

SOURCE=..\nano\lib\nmReg\correspondenceEditor.h
# End Source File
# Begin Source File

SOURCE=..\nano\lib\nmReg\dgels.c
# End Source File
# Begin Source File

SOURCE=..\nano\lib\nmReg\dgglse.c
# End Source File
# Begin Source File

SOURCE=..\nano\lib\nmReg\f2c.h
# End Source File
# Begin Source File

SOURCE=..\nano\lib\nmReg\f2c_extract.c
# End Source File
# Begin Source File

SOURCE=..\..\lib\nmImageViewer\imageViewer.C
# End Source File
# Begin Source File

SOURCE=..\..\lib\nmImageViewer\imageViewer.h
# End Source File
# Begin Source File

SOURCE=..\nano\lib\nmReg\linLeastSqr.C
# End Source File
# Begin Source File

SOURCE=..\nano\lib\nmReg\linLeastSqr.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\nmBase\nmb_Device.C
# End Source File
# Begin Source File

SOURCE=..\..\lib\nmBase\nmb_Device.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\nmBase\nmb_Image.C
# End Source File
# Begin Source File

SOURCE=..\..\lib\nmBase\nmb_Image.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\nmBase\nmb_String.C
# End Source File
# Begin Source File

SOURCE=..\..\lib\nmBase\nmb_String.h
# End Source File
# Begin Source File

SOURCE=..\nano\lib\nmReg\nmr_Registration_Impl.C
# End Source File
# Begin Source File

SOURCE=..\nano\lib\nmReg\nmr_Registration_Impl.h
# End Source File
# Begin Source File

SOURCE=..\nano\lib\nmReg\nmr_Registration_ImplUI.C
# End Source File
# Begin Source File

SOURCE=..\nano\lib\nmReg\nmr_Registration_ImplUI.h
# End Source File
# Begin Source File

SOURCE=..\nano\lib\nmReg\nmr_Registration_Interface.C
# End Source File
# Begin Source File

SOURCE=..\nano\lib\nmReg\nmr_Registration_Interface.h
# End Source File
# Begin Source File

SOURCE=..\nano\lib\nmReg\nmr_Registration_Server.C
# End Source File
# Begin Source File

SOURCE=..\nano\lib\nmReg\nmr_Registration_Server.h
# End Source File
# Begin Source File

SOURCE=..\..\lib\nmBase\PPM.C
# End Source File
# Begin Source File

SOURCE=..\..\lib\nmBase\PPM.h
# End Source File
# Begin Source File

SOURCE=.\server.C
# End Source File
# Begin Source File

SOURCE=..\..\lib\nmBase\Topo.C
# End Source File
# Begin Source File

SOURCE=..\..\lib\nmBase\Topo.h
# End Source File
# Begin Source File

SOURCE=..\nano\lib\nmReg\transformSolve.C
# End Source File
# Begin Source File

SOURCE=..\nano\lib\nmReg\transformSolve.h
# End Source File
# End Target
# End Project

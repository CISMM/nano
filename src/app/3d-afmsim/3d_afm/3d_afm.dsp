# Microsoft Developer Studio Project File - Name="3d_afm" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=3d_afm - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "3d_afm.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "3d_afm.mak" CFG="3d_afm - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "3d_afm - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "3d_afm - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "3d_afm - Win32 Release"

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

!ELSEIF  "$(CFG)" == "3d_afm - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\..\..\..\..\external\pc_win32\include" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c /Tp
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib glut32_UNC.lib glu32.lib opengl32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept /libpath:"..\..\..\..\..\external\pc_win32\lib"

!ENDIF 

# Begin Target

# Name "3d_afm - Win32 Release"
# Name "3d_afm - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\3Dobject.cpp
# End Source File
# Begin Source File

SOURCE=..\ConeSphere.cpp
# End Source File
# Begin Source File

SOURCE=..\dna.cpp
# End Source File
# Begin Source File

SOURCE=..\draw.cpp
# End Source File
# Begin Source File

SOURCE=..\input.cpp
# End Source File
# Begin Source File

SOURCE=..\lightcol.cpp
# End Source File
# Begin Source File

SOURCE=..\scan.cpp
# End Source File
# Begin Source File

SOURCE=..\sim.cpp
# End Source File
# Begin Source File

SOURCE=..\Tips.cpp
# End Source File
# Begin Source File

SOURCE=..\Unca.cpp
# End Source File
# Begin Source File

SOURCE=..\uncert.cpp
# End Source File
# Begin Source File

SOURCE=..\Uncertw.cpp
# End Source File
# Begin Source File

SOURCE=..\Vec3d.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\3Dobject.h
# End Source File
# Begin Source File

SOURCE=..\ConeSphere.h
# End Source File
# Begin Source File

SOURCE=..\defns.h
# End Source File
# Begin Source File

SOURCE=..\draw.h
# End Source File
# Begin Source File

SOURCE=..\input.h
# End Source File
# Begin Source File

SOURCE=..\lightcol.h
# End Source File
# Begin Source File

SOURCE=..\scan.h
# End Source File
# Begin Source File

SOURCE=..\sim.h
# End Source File
# Begin Source File

SOURCE=..\Tips.h
# End Source File
# Begin Source File

SOURCE=..\Unca.h
# End Source File
# Begin Source File

SOURCE=..\uncert.h
# End Source File
# Begin Source File

SOURCE=..\Uncertw.h
# End Source File
# Begin Source File

SOURCE=..\vec3d.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Source File

SOURCE=.\11nm_dimerWithDNA_try1.dat
# End Source File
# Begin Source File

SOURCE=.\11nm_dimerWithDNA_try2.dat
# End Source File
# Begin Source File

SOURCE=.\11nmtip_dimerNewest.dat
# End Source File
# Begin Source File

SOURCE=.\11nmtip_protein1.dat
# End Source File
# Begin Source File

SOURCE=.\11nmtip_protein1_x4.dat
# End Source File
# Begin Source File

SOURCE=.\11nmtip_protein2.dat
# End Source File
# Begin Source File

SOURCE=.\11nmtip_protein2_try2.dat
# End Source File
# Begin Source File

SOURCE=.\4.4.dat
# End Source File
# Begin Source File

SOURCE=.\5nmtip_protein2.dat
# End Source File
# Begin Source File

SOURCE=.\actualMonomer.dat
# End Source File
# Begin Source File

SOURCE=.\dimerOneSphere.dat
# End Source File
# Begin Source File

SOURCE=.\dimers1.1.dat
# End Source File
# Begin Source File

SOURCE=.\dimers1.2.dat
# End Source File
# Begin Source File

SOURCE=.\dimers1.3.dat
# End Source File
# Begin Source File

SOURCE=.\dimers1.4.dat
# End Source File
# Begin Source File

SOURCE=.\dimers1.5.dat
# End Source File
# Begin Source File

SOURCE=.\dimers1.dat
# End Source File
# Begin Source File

SOURCE=.\dimers2.dat
# End Source File
# Begin Source File

SOURCE=.\dimers3.dat
# End Source File
# Begin Source File

SOURCE=.\dimers4.dat
# End Source File
# Begin Source File

SOURCE=.\dimers5.dat
# End Source File
# Begin Source File

SOURCE=.\dna2.dat
# End Source File
# Begin Source File

SOURCE=.\Hong.dat
# End Source File
# Begin Source File

SOURCE=.\Hong2.dat
# End Source File
# Begin Source File

SOURCE=.\Hong3.dat
# End Source File
# Begin Source File

SOURCE="U:\.cs.unc.edu\proj\stm\src\hilchey\nano\src\app\3d-afmsim\lac-small.dat"
# End Source File
# Begin Source File

SOURCE="U:\.cs.unc.edu\proj\stm\src\hilchey\nano\src\app\3d-afmsim\lac-smaller.dat"
# End Source File
# Begin Source File

SOURCE=.\linear1.txt
# End Source File
# Begin Source File

SOURCE=.\linear2x.dat
# End Source File
# Begin Source File

SOURCE=.\linear3x.dat
# End Source File
# Begin Source File

SOURCE=.\linear4x.dat
# End Source File
# Begin Source File

SOURCE=.\linear5x.dat
# End Source File
# Begin Source File

SOURCE=.\linear6x.dat
# End Source File
# Begin Source File

SOURCE=.\linear88.5.dat
# End Source File
# Begin Source File

SOURCE=.\monomer4nm.txt
# End Source File
# Begin Source File

SOURCE="U:\.cs.unc.edu\proj\stm\src\hilchey\nano\src\app\3d-afmsim\script.sh"
# End Source File
# Begin Source File

SOURCE=.\sphere1.dat
# End Source File
# Begin Source File

SOURCE=.\sphere2.dat
# End Source File
# Begin Source File

SOURCE=.\sphere3.dat
# End Source File
# Begin Source File

SOURCE=.\sphere4.dat
# End Source File
# Begin Source File

SOURCE=.\sphere5.dat
# End Source File
# Begin Source File

SOURCE=.\sphere6.dat
# End Source File
# Begin Source File

SOURCE=.\sphere_output.txt
# End Source File
# Begin Source File

SOURCE=.\testfile.dat
# End Source File
# End Target
# End Project

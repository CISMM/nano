rem @echo off

set NANO_ROOT=PATH_GOES_HERE
set NM_COLORMAP_DIR=%NANO_ROOT%\share\colormaps
set NM_TCL_DIR=%NANO_ROOT%\share\tcl
set TRACKER=null Phantom
set BDBOX=Magellan
REM set BDBOX=null
set PATH=c:\nsrg\external\pc_win32\bin;%NANO_ROOT%/bin/;%PATH%
nano.exe
if not errorlevel 0 pause

REM set V_DISPLAY=workbench
REM set TRACKER=head@localhost null
REM head@localhost null
REM set NM_COLORMAP_DIR=C:\progra~1\nanoManipulator\share\colormapsREM 
REM set NM_TCL_DIR=E:\mason\nano\obj\pc_win32\release\share\tcl
REM obj\pc_win32\release\app\nano\nanorelease.exe -dsem sem@holmium-cs


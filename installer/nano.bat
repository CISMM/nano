rem @echo off

set NANO_ROOT=PATH_GOES_HERE
set NM_COLORMAP_DIR=%NANO_ROOT%\share\colormaps
set NM_TCL_DIR=%NANO_ROOT%\share\tcl
set TRACKER=null Phantom
set BDBOX=Magellan
set PATH=%NANO_ROOT%/bin/;%NANO_ROOT%/bin/sensable;%PATH%
nano.exe %1 %2 %3 %4 %5 %6 %7 %8
if not errorlevel 0 pause

REM set V_DISPLAY=workbench
REM set TRACKER=head@localhost null
REM -dsem sem@holmium-cs


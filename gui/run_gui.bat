@echo off
rem ===========================================================================
rem  LWFDSPC host-side diagnostic GUI - launcher
rem
rem  Double-click this file, or run it with arguments:
rem      run_gui.bat                 auto-scan the COM port and connect
rem      run_gui.bat --demo          no hardware, layout check only
rem      run_gui.bat --port COM12    connect to a specific port
rem      run_gui.bat --read-only     disable all writes
rem
rem  THIS FILE IS DELIBERATELY ASCII-ONLY.
rem  cmd.exe parses a batch file's bytes using the console's *current* code
rem  page, so UTF-8 Chinese literals in a .bat come out as mojibake - and a
rem  "chcp 65001" inside the same file does not help, because the string
rem  literals were already parsed before it ran.  All Chinese messages
rem  therefore live in launcher.py, where Python controls the encoding.
rem ===========================================================================

setlocal
cd /d "%~dp0"

rem Prefer the py launcher and pick a version pyx2cscope supports (3.10-3.14).
rem Newest first: that is the version this tool was verified on.
set "PYEXE="
call :try py -3.14
call :try py -3.13
call :try py -3.12
call :try py -3.11
call :try py -3.10
call :try py -3
call :try python

if not defined PYEXE goto :no_python

rem Tell launcher.py what the user actually typed, so argparse usage/errors
rem say "run_gui.bat" instead of "launcher.py".
set "LWFDSPC_INVOKED_AS=run_gui.bat"

%PYEXE% "launcher.py" %*
set "RC=%ERRORLEVEL%"
rem Pause only on failure, so a normal GUI exit does not leave a stray window.
if not "%RC%"=="0" pause
exit /b %RC%


:try
rem Probe one interpreter.  First one that answers wins; later calls no-op.
if defined PYEXE exit /b 0
%* -c "import sys" >nul 2>&1
if not errorlevel 1 set "PYEXE=%*"
exit /b 0


:no_python
echo.
echo   [Cannot start] No usable Python found.
echo.
echo   Install Python 3.13 or 3.14 (64-bit) from:
echo     https://www.python.org/downloads/windows/
echo.
echo   During setup, make sure BOTH of these are ticked:
echo     [x] tcl/tk and IDLE          (the GUI needs tkinter)
echo     [x] Add python.exe to PATH
echo.
echo   Python 3.10 - 3.14 are supported.  That range is required by
echo   pyx2cscope (Requires-Python: ^>=3.10,^<3.15), not by this tool.
echo.
pause
exit /b 1

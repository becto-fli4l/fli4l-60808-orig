@echo off
REM -------------------------------------------------------------------------
REM creates a fli4l-Floppy or Image                              __FLI4LVER__
REM
REM using AutoIT Scripts
REM
REM Creation: 2005-04-03 lanspezi
REM $Id$
REM -------------------------------------------------------------------------

set _cmd_line=
set wait=

:para_next
if "%1" == "--imonc" set wait=/wait
if "%1" == "" goto para_end
set _cmd_line=%_cmd_line% %1
shift
goto para_next
:para_end

REM check for current dir on exec the script
if exist opt\base.txt goto dir_base_ok
    echo You are now in the following directory, which is not known as a fli4l-basedir:
    cd
    echo.
    echo Please start mkfli4l.bat from your fli4l-basedir!
    echo.
    goto error

:dir_base_ok
if exist flibuildrestart.flg del flibuildrestart.flg
set wait=/wait

start %wait% windows\autoit\AutoIt3.exe windows\scripts\mkfli4l.au3 %_cmd_line%
if exist flibuildrestart.flg goto dir_base_ok
goto end

:error
pause

:end
set _cmd_line=

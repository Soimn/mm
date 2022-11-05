@echo off
setlocal

if "%Platform%" neq "x64" (
    echo ERROR: Platform is not "x64" - please run this from the MSVC x64 native tools command prompt.
    exit /b 1
)

set "root_dir=%cd%"
set "build_dir=%root_dir%\build"
set "src_dir=%root_dir%\src"

set "temp_link_libs=Shlwapi.lib"

set "common_comp_options=/GS- /Oi /W4 /wd4201"
set "common_link_options=/INCREMENTAL:no /opt:icf /opt:ref /fixed /SUBSYSTEM:console libvcruntime.lib kernel32.lib %temp_link_libs%"

if "%1"=="debug" (
	set "mm_comp_options=%common_comp_options% /DMM_DEBUG=1 /Od /Zo /Z7 /MTd /RTC1 /wd4100"
	set "mm_link_options=%common_link_options% libucrtd.lib libvcruntimed.lib"
) else if "%1"=="release" (
	set "mm_comp_options=%common_comp_options% /O2"
	set "mm_link_options=%common_link_options%"
) else (
	echo Illegal first argument^, must be one of ^<debug^|release^>
	goto end
)

if "%2" neq "" (
	echo Illegal number of arguments^, expected^: build ^<debug^|release^>
	goto end
)

if not exist %build_dir% mkdir %build_dir%
cd %build_dir%

REM Compile mm - commandline tool for running the compiler
cl /nologo /diagnostics:caret %mm_comp_options% %src_dir%\mm.c /link %mm_link_options%

:end
endlocal
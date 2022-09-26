@echo off
setlocal

if "%Platform%" neq "x64" (
    echo ERROR: Platform is not "x64" - please run this from the MSVC x64 native tools command prompt.
    exit /b 1
)

set "root_dir=%cd%"
set "build_dir=%root_dir%\build"
set "src_dir=%root_dir%\src"

set "common_comp_options=-W4 -Wno-unused-parameter -Wno-microsoft-anon-tag"
set "common_link_options=-INCREMENTAL:no -opt:icf -opt:ref"

set "common_mmc_link_options= -SUBSYSTEM:console"

if "%1"=="debug" (
	set "mmc_comp_options=%common_comp_options% -DMM_DEBUG=1 -Od -Zo -Z7 -RTC1 -Wno-unused-variable"
	set "mmc_link_options=%common_link_options% %common_mmc_link_options%"
) else if "%1"=="release" (
	set "mmc_comp_options=%common_comp_options%"
	set "mmc_link_options=%common_link_options% %common_mmc_link_options%"
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

REM Compile mmc - commandline tool for running the compiler
clang-cl %mmc_comp_options% %src_dir%\mmc.c -link %mmc_link_options%

:end
endlocal
@echo off

if "%Platform%" neq "x64" (
    echo ERROR: Platform is not "x64" - please run this from the MSVC x64 native tools command prompt.
    exit /b 1
)

set "diag_options= /nologo /FC /diagnostics:column /W4 /wd4201 /wd4100"
set "nocrt_options= /Zl /GR- /GS- /Gs9999999"
set "nocrt_link= /STACK:0x100000,0x100000 /NODEFAULTLIB /SUBSYSTEM:console"
set "fast_fp= /fp:fast /fp:except-"

IF NOT EXIST .\build mkdir build
pushd .\build

IF "%1"=="debug" (
set "compile_options= /DMM_DEBUG %nocrt_options% %diag_options% /Od /Zo /Zf /Z7"
) ELSE (
set "compile_options= %nocrt_options% %diag_options% /O2 /Zo /Zf /Z7"
)

set "link_options= /INCREMENTAL:NO /opt:ref %nocrt_link%"

cl %compile_options% ..\src\mm.c /link %link_options% /PDB:mmc.pdb /ENTRY:WinMainCRTStartup Kernel32.lib /out:mmc.exe

del mm.obj

popd

@echo off

if "%Platform%" neq "x64" (
    echo ERROR: Platform is not "x64" - please run this from the MSVC x64 native tools command prompt.
    exit /b 1
)

set "diag_options= /nologo /FC /diagnostics:column /W4"
set "nocrt_options= /Zl /GR- /GS- /Gs9999999"
set "link_options= /INCREMENTAL:NO /opt:ref /STACK:0x100000,0x100000 /NODEFAULTLIB /SUBSYSTEM:console"

IF NOT EXIST .\build mkdir build
pushd .\build

IF "%1"=="debug" (
set "compile_options= /DMM_DEBUG=1 %nocrt_options% %diag_options% /Od /Zo /Zf /Z7"
) ELSE (
set "compile_options= %nocrt_options% %diag_options% /O2 /Zo /Zf /Z7"
)

cl %compile_options% ..\src\mm_win32.c /link %link_options% /PDB:mm.pdb /ENTRY:WinMainCRTStartup Kernel32.lib /out:mm.exe

del mm_win32.obj

popd

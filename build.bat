@echo off

if "%Platform%" neq "x64" (
    echo ERROR: Platform is not "x64" - please run this from the MSVC x64 native tools command prompt.
    exit /b 1
)

set "diag_options= /W4 /WX /wd4204 /wd4100 /wd4201 /wd4200 /wd4223 -Wno-microsoft-anon-tag -Wno-logical-op-parentheses  -Wno-unused-function"
set "compile_options= /nologo /FC /diagnostics:column %diag_options% /GS-"
set "link_options= /INCREMENTAL:NO /opt:icf /opt:ref /fixed /SUBSYSTEM:console libvcruntime.lib kernel32.lib"

IF NOT EXIST .\build mkdir build
pushd .\build

IF "%1"=="debug" (
set "compile_options=%compile_options% /DMM_DEBUG=1 /Od /Zo /Z7 /RTC1"
set "link_options=%link_options% libucrtd.lib libvcruntimed.lib"
) ELSE (
set "compile_options=%compile_options% /O2 /Zo /Z7"
)

clang-cl %compile_options% ..\src\mm.c /link %link_options% /PDB:mm.pdb /out:mm.exe

popd
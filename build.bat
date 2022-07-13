@echo off

if "%Platform%" neq "x64" (
    echo ERROR: Platform is not "x64" - please run this from the MSVC x64 native tools command prompt.
    exit /b 1
)

set "diag_options= /nologo /FC /diagnostics:column /W4 /wd4204 /wd4100 /wd4201 /wd4200 /wd4223 -Wno-microsoft-anon-tag -Wno-logical-op-parentheses  -Wno-unused-function"
set "nocrt_options= /Zl /GR- /GS- /Gs9999999"
set "link_options= /INCREMENTAL:NO /opt:ref /STACK:0x100000,0x100000 /NODEFAULTLIB /SUBSYSTEM:console"

IF NOT EXIST .\build mkdir build
pushd .\build

IF "%1"=="debug" (
set "compile_options= /DMM_DEBUG=1 %nocrt_options% %diag_options% /Od /Zo /Z7"
) ELSE (
set "compile_options= %nocrt_options% %diag_options% /O2 /Zo /Z7"
)

clang-cl /LD %compile_options% ..\src\mm.c       /link %link_options% /PDB:libmm.pdb /out:libmm.dll

popd

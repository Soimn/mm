@echo off

cd %~dp0

if "%Platform%" neq "x64" (
    echo ERROR: Platform is not "x64" - please run this from the MSVC x64 native tools command prompt.
    exit /b 1
)

IF NOT EXIST .\build mkdir build
pushd .\build

IF "%1"=="init" (
	clang-cl .\..\build_src\main.c /link /out:build.exe
) ELSE IF "%1"=="debug" (
	.\build.exe "debug"
) ELSE IF "%1"=="release" (
	.\build.exe "release"
) ELSE (
	echo|set /p="Invalid Arguments. Expected: build <init|debug|release>"
)

popd
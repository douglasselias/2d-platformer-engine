@echo off
SETLOCAL ENABLEDELAYEDEXPANSION
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64
cls

rmdir /S /Q .\build
mkdir build
pushd .\build
echo Entered the build folder

set libs=opengl32.lib kernel32.lib user32.lib shell32.lib gdi32.lib winmm.lib msvcrt.lib

cl /Zi /MD -nologo ..\main.cpp /I"../vendor" "../vendor/raylib.lib" %libs%

echo Build Finished

main.exe

popd

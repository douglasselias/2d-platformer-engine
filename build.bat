@echo off

cl

if %ERRORLEVEL% neq 0 (
  call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
)

cls

rmdir /S /Q .\build
mkdir build
pushd .\build

set better_output=/nologo /diagnostics:caret /FC
set basic_optimizations=/Oi /fp:fast /fp:except- /jumptablerdata /kernel /GS- /Gs9999999
set warnings=/RTCc /WX /W4
set base_flags=%better_output% %basic_optimizations% %warnings% /cgthreads8 /MD

set debug_flags=/Z7 /Zo
set release_flags=/GL /O2

set vendor_libs="../vendor/raylib.lib"
set system_libs=user32.lib shell32.lib gdi32.lib winmm.lib opengl32.lib

cl %base_flags% %debug_flags% /c ..\main.cpp /I"../vendor"
@REM /NODEFAULTLIB

rc /nologo /r ..\icons\resource.rc
move ..\icons\resource.res ..\build

link /nologo %vendor_libs% %system_libs% main.obj resource.res -out:game.exe

game.exe

popd

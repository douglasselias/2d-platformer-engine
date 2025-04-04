@echo off

chcp 65001

cl

if %ERRORLEVEL% neq 0 (
  call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
)

cls

rmdir /S /Q .\build
mkdir build
pushd .\build

set better_output=/nologo /diagnostics:caret /FC
set basic_optimizations=/Oi /fp:fast /fp:except- /jumptablerdata /kernel /GS-
set warnings=/WX /W4
@REM maybe remove this flag later C4996 /wd4996 - /wd4244
set disable_stupid_errors=/wd4189 /wd4700 /wd4100 /wd4996 /wd4244 /wd4514
set compiler_base_flags=%better_output% %basic_optimizations% %warnings% %disable_stupid_errors% /cgthreads8 /MD /utf-8

set compiler_debug_flags=/Z7 /Zo /RTCc
set compiler_release_flags=/Gw /GL /O2

set vendor_libs="../vendor/raylib.lib"
set system_libs=user32.lib shell32.lib gdi32.lib winmm.lib opengl32.lib

cl %compiler_base_flags% %compiler_debug_flags% /c ..\main.cpp /I"../vendor"

if %ERRORLEVEL% neq 0 (
  echo [ERROR] Compilation failed.
  popd
  exit /b %ERRORLEVEL%
)

rc /nologo /r ..\icons\resource.rc
move ..\icons\resource.res ..\build >nul

set linker_debug_flags=/debug /incremental:no /cgthreads:8 /WX /STACK:0x100000,0x100000 kernel32.lib
@REM set linker_release_flags=

link /nologo %linker_debug_flags% %vendor_libs% %system_libs% main.obj resource.res /out:game.exe

if %ERRORLEVEL% neq 0 (
  echo [ERROR] Linking failed.
  popd
  exit /b %ERRORLEVEL%
)

game.exe

popd

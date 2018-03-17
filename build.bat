@echo off

if not exist "build" mkdir build
pushd build

set INCLUDE=C:\Users\Chris\dev\SDL2-2.0.8\include;%INCLUDE%;
set INCLUDE=C:\Users\Chris\dev\SDL2_image-2.0.3\include;%INCLUDE%;
set LIBPATH=C:\Users\Chris\dev\SDL2-2.0.8\lib\x64;%LIBPATH%;
set LIBPATH=C:\Users\Chris\dev\SDL2_image-2.0.3\lib\x64;%LIBPATH%;
set LIB=%LIBPATH%;%LIB%;
set COMPILERFLAGS="/W4 /Zi /Od"
cl "%COMPILERFLAGS%" ..\main.cpp /link SDL2.lib SDL2main.lib SDL2_image.lib /SUBSYSTEM:CONSOLE

popd
if %errorlevel% neq 0 exit /b %errorlevel%

pushd data
..\build\main.exe
popd
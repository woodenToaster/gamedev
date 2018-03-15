@echo off

mkdir build
pushd build
set INCLUDE=C:\Users\Chris\dev\SDL2-2.0.8\include;%INCLUDE%;
set INCLUDE=C:\Users\Chris\dev\SDL2_image-2.0.3\include;%INCLUDE%;
cl /W4 /Zi /Od  ..\main.cpp /link /LIBPATH:C:\Users\Chris\dev\SDL2-2.0.8\lib\x64 /LIBPATH:C:\Users\Chris\dev\SDL2_image-2.0.3\lib\x64 SDL2.lib SDL2main.lib SDL2_image.lib /SUBSYSTEM:CONSOLE
popd

if %errorlevel% neq 0 exit /b %errorlevel%

pushd data
..\build\main.exe
popd
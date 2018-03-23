@echo off

if not exist "build" mkdir build
pushd build

set DEV_PREFIX=C:\Users\Chris\dev\
set INCLUDE=%DEV_PREFIX%\SDL2-2.0.8\include;%INCLUDE%;
set INCLUDE=%DEV_PREFIX%\SDL2_image-2.0.3\include;%INCLUDE%;
set INCLUDE=%DEV_PREFIX%\SDL2_mixer-2.0.2\include;%INCLUDE%;
set LIBPATH=%DEV_PREFIX%\SDL2-2.0.8\lib\x64;%LIBPATH%;
set LIBPATH=%DEV_PREFIX%\SDL2_image-2.0.3\lib\x64;%LIBPATH%;
set LIBPATH=%DEV_PREFIX%\SDL2_mixer-2.0.2\lib\x64;%LIBPATH%;
set LIB=%LIBPATH%;%LIB%;
set COMPILERFLAGS="/W4 /Zi /Od"
set LINK_LIBS=SDL2.lib SDL2main.lib SDL2_image.lib SDL2_mixer.lib

cl "%COMPILERFLAGS%" ..\main.cpp /link %LINK_LIBS% /SUBSYSTEM:CONSOLE

popd
if %errorlevel% neq 0 exit /b %errorlevel%

pushd data
echo %cd%
..\build\main.exe
popd
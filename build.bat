@echo off

if not exist "build" mkdir build
pushd build

set DEV_PREFIX=C:\Users\Chris\dev\
set INCLUDE=%DEV_PREFIX%\SDL2-2.0.8\include;%INCLUDE%;
set INCLUDE=%DEV_PREFIX%\SDL2_mixer-2.0.2\include;%INCLUDE%;
set INCLUDE=%DEV_PREFIX%/gamedev/stb;%INCLUDE%;
set LIBPATH=%DEV_PREFIX%\SDL2-2.0.8\lib\x64;%LIBPATH%;
set LIBPATH=%DEV_PREFIX%\SDL2_mixer-2.0.2\lib\x64;%LIBPATH%;
set LIB=%LIBPATH%;%LIB%;
set COMPILERFLAGS="/DDEBUG /W4 /Gm- /Zi /GR- /nologo /EHa- /MT /Oi /Od /F 536870912"
set LINK_LIBS=SDL2.lib SDL2main.lib SDL2_mixer.lib

cl "%COMPILERFLAGS%" ..\main.cpp /link %LINK_LIBS% /SUBSYSTEM:CONSOLE

popd
if %errorlevel% neq 0 exit /b %errorlevel%

rem pushd data
rem ..\build\main.exe
rem popd
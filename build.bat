@echo off

if not exist "build" mkdir build
pushd build

set DEV_PREFIX=D:\dev
set SDL_PATH=SDL2-2.0.8
set SDL_MIXER_PATH=SDL2_mixer-2.0.2
set INCLUDE=%DEV_PREFIX%\%SDL_PATH%\include;%INCLUDE%;
set INCLUDE=%DEV_PREFIX%\%SDL_MIXER_PATH%\include;%INCLUDE%;
set INCLUDE=%DEV_PREFIX%\gamedev\stb;%INCLUDE%;
set LIBPATH=%DEV_PREFIX%\%SDL_PATH%\lib\x64;%LIBPATH%;
set LIBPATH=%DEV_PREFIX%\%SDL_MIXER_PATH%\lib\x64;%LIBPATH%;
set LIB=%LIBPATH%;%LIB%;
set COMPILERFLAGS="/DDEBUG /W4 /Gm- /Zi /GR- /nologo /EHa- /MTd /Oi /Od"
set LINK_LIBS=SDL2.lib SDL2main.lib SDL2_mixer.lib

del gamedev_*.pdb > NUL 2> NUL

REM Not sure why cl doesn't have permission to overwrite this file.
REM We have to rename it manually for hot reloading to work.
move gamedev.dll gamedev.dll.old

echo waiting for pdb > lock.tmp
cl "%COMPILERFLAGS%" ..\src\gamedev.cpp /LD ^
  /link /incremental:no /opt:ref /pdb:gamedev_%random%.pdb /export:gameUpdateAndRender
del lock.tmp
cl "%COMPILERFLAGS%" ..\src\sdl2_gamedev.cpp /Fmsdl2_gamedev.map ^
  /link /incremental:no /opt:ref %LINK_LIBS% /SUBSYSTEM:WINDOWS

popd
if %errorlevel% neq 0 exit /b %errorlevel%

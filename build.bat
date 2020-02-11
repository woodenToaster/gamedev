@echo off

if not exist "build" mkdir build
pushd build

set DEV_PREFIX=D:\dev

rem Debug SDL Build
rem set DEBUG_SDL_PATH=%DEV_PREFIX%\SDL2-2.0.10-source\VisualC\x64\Debug
rem set DEBUG_SDL_INCLUDE_PATH=%DEV_PREFIX%\SDL2-2.0.10-source\include
rem set INCLUDE=%DEBUG_SDL_INCLUDE_PATH%;%INCLUDE%;
rem set LIBPATH=%DEBUG_SDL_PATH%;%LIBPATH%;

rem Optimized SDL build
set SDL_PATH=SDL2-2.0.10
set INCLUDE=%DEV_PREFIX%\%SDL_PATH%\include;%INCLUDE%;
set LIBPATH=%DEV_PREFIX%\%SDL_PATH%\lib\x64;%LIBPATH%;

set SDL_MIXER_PATH=SDL2_mixer-2.0.2
set INCLUDE=%DEV_PREFIX%\%SDL_MIXER_PATH%\include;%INCLUDE%;
set LIBPATH=%DEV_PREFIX%\%SDL_MIXER_PATH%\lib\x64;%LIBPATH%;

set INCLUDE=%DEV_PREFIX%\gamedev\stb;%INCLUDE%;
set LIB=%LIBPATH%;%LIB%;
set PREPROCESSOR_DEFINES=/DDEVELOPER
set COMPILERFLAGS=/DDEBUG /W4 /Gm- /Zi /GR- /nologo /EHa- /MTd /Oi /Od
set LINK_LIBS=SDL2.lib SDL2main.lib SDL2_mixer.lib

del gamedev_*.pdb > NUL 2> NUL

REM Not sure why cl doesn't have permission to overwrite this file.
REM We have to rename it manually for hot reloading to work.
rem move gamedev.dll gamedev.dll.old

echo waiting for pdb > lock.tmp
cl %PREPROCESSOR_DEFINES% %COMPILERFLAGS% ..\src\gamedev.cpp /LD ^
  /link /incremental:no /opt:ref /pdb:gamedev_%random%.pdb /export:gameUpdateAndRender
del lock.tmp

REM SDL Platform Layer
rem cl %PREPROCESSOR_DEFINES% %COMPILERFLAGS% ..\src\sdl2_gamedev.cpp /Fmsdl2_gamedev.map ^
rem   /link /incremental:no /opt:ref %LINK_LIBS% /SUBSYSTEM:WINDOWS

REM Win32 Platform Layer
cl %PREPROCESSOR_DEFINES% %COMPILERFLAGS% ..\src\win32_gamedev.cpp /Fmwin32_gamedev.map ^
  /link /incremental:no /opt:ref user32.lib gdi32.lib winmm.lib

popd
if %errorlevel% neq 0 exit /b %errorlevel%

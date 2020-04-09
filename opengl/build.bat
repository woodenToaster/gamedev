@echo off

if not exist "build" mkdir build
pushd build

set COMPILERFLAGS=/DDEBUG /W4 /FC /Gm- /Zi /GR- /nologo /EHa- /MTd /Oi /Od
set INCLUDE=..\..\src;C:\Program Files (x86)\Windows Kits\10\Include\10.0.17134.0\ucrt;%INCLUDE%;
set LIB=C:\Program Files (x86)\Windows Kits\10\Lib\10.0.17134.0\ucrt\x64;%LIB%;
set LINK_LIBS=User32.lib Opengl32.lib Gdi32.lib winmm.lib

cl %COMPILERFLAGS% ..\main.cpp /link %LINK_LIBS% /SUBSYSTEM:WINDOWS

popd
if %errorlevel% neq 0 exit /b %errorlevel%

@echo off

if not exist "build" mkdir build
pushd build

set COMPILERFLAGS=/DDEBUG /W4 /Gm- /Zi /GR- /nologo /EHa- /MTd /Oi /Od
set LINK_LIBS=User32.lib Opengl32.lib Gdi32.lib 

cl %COMPILERFLAGS% ..\main.c /link %LINK_LIBS% /SUBSYSTEM:WINDOWS

popd
if %errorlevel% neq 0 exit /b %errorlevel%

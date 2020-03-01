@echo off

if not exist "build" mkdir build
pushd build

set COMPILERFLAGS=/DDEBUG /W4 /wd4201 /Gm- /Zi /GR- /nologo /EHa- /MTd /Oi /O2
set LINK_LIBS=user32.lib gdi32.lib

cl /EHsc %COMPILERFLAGS% ..\main.cpp %LINK_LIBS%

popd
if %errorlevel% neq 0 exit /b %errorlevel
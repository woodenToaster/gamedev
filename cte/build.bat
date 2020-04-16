@echo off

if not exist "build" mkdir build
pushd build

set COMPILERFLAGS=/W4 /FC /Gm- /Zi /GR- /nologo /EHa- /MTd /Oi /Od
set INCLUDE=../../src;%INCLUDE%;
set PREPROCESSOR_DEFINES=
set LINK_LIBS=user32.lib gdi32.lib winmm.lib opengl32.lib

cl %PREPROCESSOR_DEFINES% %COMPILERFLAGS% ..\main.cpp /link /incremental:no 

popd
if %errorlevel% neq 0 exit /b %errorlevel%

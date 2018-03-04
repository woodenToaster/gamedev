@echo off
set "VSCMD_START_DIR=w:\"
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
set PATH=C:\Users\Chris\dev\SDL2-2.0.8\lib\x64;%PATH%
@REM @echo off
set "VSCMD_START_DIR=w:\"
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
set PATH=C:\Users\Chris\dev\SDL2-2.0.8\lib\x64;%PATH%
set PATH=C:\Users\Chris\dev\SDL2_image-2.0.3\lib\x64;%PATH%
set PATH=C:\Users\Chris\dev\SDL2_mixer-2.0.2\lib\x64;%PATH%
set PATH=C:\Users\Chris\programs\emacs-w64-25.3-O2-with-modules\emacs\bin;%PATH%
set PATH=C:\Users\Chris\programs\ag-2018-01-25_2;%PATH%
cd /d w:

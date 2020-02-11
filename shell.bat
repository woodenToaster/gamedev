@REM @echo off
set "VSCMD_START_DIR=w:\"
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

rem SDL debug build
rem set PATH=D:\dev\SDL2-2.0.10-source\VisualC\x64\Debug;%PATH%;

rem SDL optimized build
set PATH=D:\dev\SDL2-2.0.10\lib\x64;%PATH%

set PATH=D:\dev\SDL2_mixer-2.0.2\lib\x64;%PATH%
set PATH=C:\Users\Chris\programs\emacs-w64-25.3-O2-with-modules\emacs\bin;%PATH%
set PATH=D:\dev\gnuGlobal\bin;%PATH%
set PATH=C:\Users\Chris\programs\ag-2018-01-25_2;%PATH%
set PATH=D:\dev\cloc;%PATH%
set PATH=D:\dev\cmder\vendor\git-for-windows\usr\bin;%PATH%
set PATH=D:\programs\4coder\cjh_4coder_stable;%PATH%
cd /d d:
cd d:\dev\gamedev


@echo off

mkdir build
pushd build
cl /Zi /Od  ..\main.cpp /I C:\Users\Chris\dev\SDL2-2.0.8\include /link /LIBPATH:C:\Users\Chris\dev\SDL2-2.0.8\lib\x64 SDL2.lib SDL2main.lib /SUBSYSTEM:CONSOLE
popd

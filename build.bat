@echo off

mkdir build
pushd build
cl /Zi ..\main.cpp
popd
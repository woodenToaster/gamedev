@echo off

set COMPILERFLAGS=/W4 /FC /Gm- /Zi /GR- /nologo /EHa- /MTd /Oi /Od

cl %COMPILERFLAGS% generated_cte_test.cpp /link /incremental:no 

popd
if %errorlevel% neq 0 exit /b %errorlevel
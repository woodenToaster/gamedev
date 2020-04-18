rem @echo off

if not exist "generated" mkdir generated
pushd generated

set generated_filename=%~nx1

rem Only supports .cpp (or other 4 character) extension
set generated_filename_base=%generated_filename:~0,-4%

set COMPILERFLAGS=/W4 /FC /Gm- /Zi /GR- /nologo /EHa- /MTd /Oi /Od

cl %COMPILERFLAGS% %generated_filename% /link /incremental:no
%generated_filename_base%.exe > %generated_filename%.out

popd


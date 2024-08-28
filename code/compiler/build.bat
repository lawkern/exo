@ECHO off

SET CODEPATH=%~dp0
CD /D %CODEPATH%

IF NOT EXIST ..\..\build mkdir ..\..\build
PUSHD ..\..\build

SET WARNING_FLAGS=/WX /W4 /wd4201 /wd4204 /wd4100 /wd4101 /wd4189 /wd4505 /wd4702
SET COMPILER_FLAGS=/nologo /Z7 /Oi /FC /MT /diagnostics:column /D_CRT_SECURE_NO_WARNINGS /I ..\code\shared %WARNING_FLAGS%
SET LINKER_FLAGS=/opt:ref /incremental:no

cl ..\code\compiler\platform_win32.c ..\code\compiler\compiler_main.c %COMPILER_FLAGS% /Od /DDEVELOPMENT_BUILD=1 /Fecompiler_x64_debug   /link %LINKER_FLAGS%
cl ..\code\compiler\platform_win32.c ..\code\compiler\compiler_main.c %COMPILER_FLAGS% /O2 /DDEVELOPMENT_BUILD=0 /Fecompiler_x64_release /link %LINKER_FLAGS%

POPD

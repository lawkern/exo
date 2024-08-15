@ECHO off

SET CODEPATH=%~dp0
CD /D %CODEPATH%

IF NOT EXIST ..\..\build mkdir ..\..\build
PUSHD ..\..\build

REM   NOTE(law): Specify the path to the SDL2 include and lib directories
REM   below. Additionally, a copy of the file SDL2.dll must be present in the
REM   build directory in order to run the compiled executable.

SET SDLPATH=D:\SDL2
SET INCLUDE=%CODEPATH%\..\shared;%INCLUDE%
SET INCLUDE=%SDLPATH%\include;%INCLUDE%
SET LIB=%SDLPATH%\lib\x64;%LIB%

SET WARNING_FLAGS=/WX /W4 /wd4201 /wd4204 /wd4100 /wd4101 /wd4189 /wd4505 /wd4702
SET COMPILER_FLAGS=/nologo /Z7 /Oi /FC /MT /diagnostics:column %WARNING_FLAGS%
SET COMPILER_FLAGS=%COMPILER_FLAGS% /DSDL_MAIN_HANDLED=1 /D_CRT_SECURE_NO_WARNINGS

SET LINKER_FLAGS=/opt:ref /incremental:no sdl2.lib sdl2main.lib

cl ..\code\desktop\platform_sdl_main.cpp %COMPILER_FLAGS% /Od /DDEVELOPMENT_BUILD=1 /Fedesktop_debug   /link %LINKER_FLAGS%
cl ..\code\desktop\platform_sdl_main.cpp %COMPILER_FLAGS% /O2 /DDEVELOPMENT_BUILD=0 /Fedesktop_release /link %LINKER_FLAGS%

POPD

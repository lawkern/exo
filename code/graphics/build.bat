@ECHO off

SET DEVELOPMENT_BUILD=1

SET WARNING_FLAGS=-WX -W4 -wd4201 -wd4204
IF %DEVELOPMENT_BUILD%==1 (
   SET WARNING_FLAGS=%WARNING_FLAGS% -wd4100 -wd4101 -wd4189 -wd4505 -wd4702
)

REM   NOTE(law): Specify the path to the SDL2 include and lib directories
REM   below. Additionally, copies of the file SDL2.dll and SDL2_ttf.ddl must be
REM   present in the build directory in order to run the compiled executable.

SET INCLUDE=D:\SDL2\include;D:\SDL2_ttf\include;%INCLUDE%
SET LIB=D:\SDL2\lib\x64;D:\SDL2_ttf\lib\x64;%LIB%

SET COMPILER_FLAGS=-nologo -Z7 -Oi -Od -FC -MT -diagnostics:column %WARNING_FLAGS%
SET COMPILER_FLAGS=%COMPILER_FLAGS% -DDEVELOPMENT_BUILD=%DEVELOPMENT_BUILD% -DSDL_MAIN_HANDLED=1 -D_CRT_SECURE_NO_WARNINGS

SET LINKER_FLAGS=-opt:ref -incremental:no sdl2.lib sdl2main.lib sdl2_ttf.lib

IF NOT EXIST ..\..\build mkdir ..\..\build
PUSHD ..\..\build

cl ..\code\graphics\platform_sdl.cpp %COMPILER_FLAGS% -Fegfx /link %LINKER_FLAGS%

POPD

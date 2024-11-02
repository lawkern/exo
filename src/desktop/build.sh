#!/bin/bash

CFLAGS="-g -fdiagnostics-absolute-paths -fno-threadsafe-statics -I ../src/shared/"
CFLAGS="${CFLAGS} $(sdl2-config --cflags)"
CFLAGS="${CFLAGS} -Werror -Wall"
CFLAGS="${CFLAGS} -Wno-missing-braces"
CFLAGS="${CFLAGS} -Wno-writable-strings"
CFLAGS="${CFLAGS} -Wno-char-subscripts"
CFLAGS="${CFLAGS} -Wno-unused-variable"
CFLAGS="${CFLAGS} -Wno-unused-function"

LDFLAGS="$(sdl2-config --libs)"

mkdir -p ../../build
pushd ../../build > /dev/null

clang $CFLAGS ../src/desktop/desktop_renderer.cpp -c -Og -DDEVELOPMENT_BUILD=1 -o desktop_renderer_x64_debug.o
clang $CFLAGS ../src/desktop/desktop_renderer.cpp -c -O2 -DDEVELOPMENT_BUILD=0 -o desktop_renderer_x64_release.o

clang $CFLAGS ../src/desktop/platform_sdl_main.c -DDEVELOPMENT_BUILD=1 -Og -o desktop_x64_debug   $LDFLAGS desktop_renderer_x64_debug.o
clang $CFLAGS ../src/desktop/platform_sdl_main.c -DDEVELOPMENT_BUILD=0 -O2 -o desktop_x64_release $LDFLAGS desktop_renderer_x64_release.o

popd

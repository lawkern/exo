#!/bin/bash

CFLAGS="-g -std=c++11 -mavx2 -fdiagnostics-absolute-paths -fno-threadsafe-statics -I ../code/shared/"
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

clang $CFLAGS ../code/desktop/platform_sdl_main.cpp -DDEVELOPMENT_BUILD=1 -Og -o desktop_debug   $LDFLAGS
clang $CFLAGS ../code/desktop/platform_sdl_main.cpp -DDEVELOPMENT_BUILD=0 -O2 -o desktop_release $LDFLAGS

popd

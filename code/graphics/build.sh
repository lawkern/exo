#!/bin/bash

DEVELOPMENT_BUILD=1

CFLAGS="-O2 -g -fdiagnostics-absolute-paths -fno-threadsafe-statics"
CLFAGS="${CFLAGS} -DDEVELOPMENT_BUILD=${DEVELOPMENT_BUILD}"
CFLAGS="${CFLAGS} $(sdl2-config --cflags)"
CFLAGS="${CFLAGS} -Werror"
CFLAGS="${CFLAGS} -Wall"
CFLAGS="${CFLAGS} -Wno-missing-braces"
CFLAGS="${CFLAGS} -Wno-writable-strings"

if [[ "$DEVELOPMENT_BUILD" == 1 ]]
then
    CFLAGS="${CFLAGS} -Wno-unused-variable"
    CFLAGS="${CFLAGS} -Wno-unused-function"
fi

LDFLAGS="$(sdl2-config --libs) $(pkg-config --libs SDL2_ttf)"

mkdir -p ../../build
pushd ../../build > /dev/null

clang $CFLAGS ../code/graphics/platform_sdl.cpp -o gfx $LDFLAGS

popd

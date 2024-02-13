#!/bin/sh

DEVELOPMENT_BUILD=1

CFLAGS="-O2 -g $(sdl2-config --cflags) -fdiagnostics-absolute-paths -DDEVELOPMENT_BUILD=${DEVELOPMENT_BUILD}"
CFLAGS="${CFLAGS} -Werror"
CFLAGS="${CFLAGS} -Wall"
CFLAGS="${CFLAGS} -Wno-missing-braces"

if [[ "$DEVELOPMENT_BUILD" == 1 ]]
then
    CFLAGS="${CFLAGS} -Wno-unused-variable"
    CFLAGS="${CFLAGS} -Wno-unused-function"
fi

LDFLAGS="$(sdl2-config --libs)"

mkdir -p ../../build
pushd ../../build > /dev/null

clang $CFLAGS ../code/graphics/platform_sdl.c -o gfx $LDFLAGS

popd

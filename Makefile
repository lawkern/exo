CFLAGS = -g -I ./src/shared/
CFLAGS += -Werror -Wall
CFLAGS += -Wno-missing-braces
CFLAGS += -Wno-writable-strings
CFLAGS += -Wno-char-subscripts
CFLAGS += -Wno-unused-variable
CFLAGS += -Wno-unused-function

DEBUG   = -DDEVELOPMENT_BUILD=1 -Og
RELEASE = -DDEVELOPMENT_BUILD=0 -O2

KFLAGS = -g -ffreestanding -Wall -Wextra -Wno-unused-function -I ./src/shared/
SDLFLAGS = `pkg-config --cflags --libs sdl3`

kernel:
	@mkdir -p build
	nasm ./src/kernel/boot.asm -felf32 -o ./build/boot.o

	i686-elf-gcc -o ./build/kernel_debug.o   -c $(DEBUG)   $(KFLAGS) ./src/kernel/kernel.c
	i686-elf-gcc -o ./build/kernel_release.o -c $(RELEASE) $(KFLAGS) ./src/kernel/kernel.c

	i686-elf-gcc -o ./build/exo_i386_debug.bin   $(KFLAGS) -T./src/kernel/linker.ld -nostdlib -lgcc ./build/boot.o ./build/kernel_debug.o
	i686-elf-gcc -o ./build/exo_i386_release.bin $(KFLAGS) -T./src/kernel/linker.ld -nostdlib -lgcc ./build/boot.o ./build/kernel_release.o

compiler:
	@mkdir -p build
	$(CC) -o ./build/compiler_debug   $(CFLAGS) $(DEBUG)   ./src/shared/platform_unix.c ./src/compiler/main.c $(LDFLAGS)
	$(CC) -o ./build/compiler_release $(CFLAGS) $(RELEASE) ./src/shared/platform_unix.c ./src/compiler/main.c $(LDFLAGS)

desktop:
	@mkdir -p build
	$(CC) -o ./build/renderer_debug.o   -c $(CFLAGS) $(DEBUG)   ./src/desktop/renderer.cpp
	$(CC) -o ./build/renderer_release.o -c $(CFLAGS) $(RELEASE) ./src/desktop/renderer.cpp

	$(CC) -o ./build/desktop_debug.o    -c $(CFLAGS) $(DEBUG)   ./src/desktop/desktop.c
	$(CC) -o ./build/desktop_release.o  -c $(CFLAGS) $(RELEASE) ./src/desktop/desktop.c

	$(CC) -o ./build/desktop_debug         $(CFLAGS) $(SDLFLAGS) $(DEBUG)   ./src/desktop/sdl_main.c ./build/desktop_debug.o   ./build/renderer_debug.o
	$(CC) -o ./build/desktop_release       $(CFLAGS) $(SDLFLAGS) $(RELEASE) ./src/desktop/sdl_main.c ./build/desktop_release.o ./build/renderer_release.o

run:
	qemu-system-i386 -kernel ./build/exo_i386_debug.bin

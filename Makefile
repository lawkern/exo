CFLAGS = -g -I ./src/shared/
CFLAGS += -Werror -Wall
CFLAGS += -Wno-missing-braces
CFLAGS += -Wno-writable-strings
CFLAGS += -Wno-char-subscripts
CFLAGS += -Wno-unused-variable
CFLAGS += -Wno-unused-function

KFLAGS = -g -ffreestanding -Wall -Wextra -Wno-unused-function -I ./src/shared/

kernel:
	@mkdir -p build
	nasm ./src/kernel/boot.asm -felf32 -o ./build/boot.o

	i686-elf-gcc ./src/kernel/kernel.c -c -Og -DDEVELOPMENT_BUILD=1 $(KFLAGS) -o ./build/kernel_debug.o
	i686-elf-gcc ./src/kernel/kernel.c -c -O2 -DDEVELOPMENT_BUILD=0 $(KFLAGS) -o ./build/kernel_release.o

	i686-elf-gcc -T./src/kernel/linker.ld -o ./build/exo_i386_debug.bin   $(KFLAGS) -nostdlib -lgcc ./build/boot.o ./build/kernel_debug.o
	i686-elf-gcc -T./src/kernel/linker.ld -o ./build/exo_i386_release.bin $(KFLAGS) -nostdlib -lgcc ./build/boot.o ./build/kernel_release.o

compiler:
	@mkdir -p build
	$(CC) $(CFLAGS) ./src/shared/platform_unix.c ./src/compiler/main.c -DDEVELOPMENT_BUILD=1 -Og -o ./build/compiler_x64_debug   $(LDFLAGS)
	$(CC) $(CFLAGS) ./src/shared/platform_unix.c ./src/compiler/main.c -DDEVELOPMENT_BUILD=0 -O2 -o ./build/compiler_x64_release $(LDFLAGS)

desktop:
	@mkdir -p build
	$(CC) $(CFLAGS) ./src/desktop/renderer.cpp -c -Og -DDEVELOPMENT_BUILD=1 -o ./build/renderer_debug.o
	$(CC) $(CFLAGS) ./src/desktop/renderer.cpp -c -O2 -DDEVELOPMENT_BUILD=0 -o ./build/renderer_release.o

	$(CC) $(CFLAGS) ./src/desktop/desktop.c -c -Og -DDEVELOPMENT_BUILD=1 -o ./build/desktop_debug.o
	$(CC) $(CFLAGS) ./src/desktop/desktop.c -c -O2 -DDEVELOPMENT_BUILD=0 -o ./build/desktop_release.o

	$(CC) $(CFLAGS) ./src/desktop/sdl_main.c -DDEVELOPMENT_BUILD=1 -Og -o ./build/desktop_x64_debug   ./build/desktop_debug.o   ./build/renderer_debug.o   `pkg-config --cflags --libs sdl3`
	$(CC) $(CFLAGS) ./src/desktop/sdl_main.c -DDEVELOPMENT_BUILD=0 -O2 -o ./build/desktop_x64_release ./build/desktop_release.o ./build/renderer_release.o `pkg-config --cflags --libs sdl3`

run:
	qemu-system-i386 -kernel ./build/exo_i386_debug.bin

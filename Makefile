# Automatically generate lists of sources using wildcards.
C_SOURCES = $(wildcard kernel/*.c lib/*/*.c)
HEADERS = $(wildcard kernel/*.h)

BOOTDIR = boot
DESTDIR = build

CFLAGS := -nostdlib -ffreestanding -02 -g -Wall -Wextra

OBJS = ${C_SOURCES:.c=.o}

# Default build target
all: build

# Run the operating system on qemu
run: all
	qemu-system-i386 -fda $(DESTDIR)/kernel.bin

build: boot.o linker.ld
	i386-elf-gcc -I lib/includes -I kernel/includes $(C_SOURCES) $(DESTDIR)/boot.o -o $(DESTDIR)/kernel.bin -nostdlib -ffreestanding -T linker.ld

boot.o:
	nasm -f elf32 $(BOOTDIR)/boot.asm -o $(DESTDIR)/boot.o

clean:
	rm -rf $(DESTDIR)/*

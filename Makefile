# Automatically generate lists of sources using wildcards.
C_SOURCES = $(wildcard $(KERNELDIR)/*.c lib/*/*.c)
HEADERS = $(wildcard $(KERNELDIR)/*.h)

BUILDDIR = build
KERNELDIR = kernel

CFLAGS := -nostdlib -ffreestanding -02 -g -Wall -Wextra

OBJS = ${C_SOURCES:.c=.o}

# Default build target
all: build

# Run the operating system on qemu
run: all
	qemu-system-i386 -serial file:$(BUILDDIR)/serial.log -kernel $(BUILDDIR)/kernel.bin

build: boot.o linker.ld
	i386-elf-gcc -I lib/includes -I kernel/includes $(C_SOURCES) $(BUILDDIR)/boot.o -o $(BUILDDIR)/kernel.bin -nostdlib -ffreestanding -T linker.ld

boot.o:
	nasm -f elf32 $(KERNELDIR)/boot.asm -o $(BUILDDIR)/boot.o

clean:
	rm -rf $(BUILDDIR)/*

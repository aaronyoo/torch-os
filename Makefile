# Automatically generate lists of sources using wildcards.
C_SOURCES = $(wildcard $(KERNELDIR)/*.c lib/*/*.c)
ASM_SOURCES = $(KERNELDIR)/utility.asm
HEADERS = $(wildcard $(KERNELDIR)/*.h)

BUILDDIR = build
KERNELDIR = kernel

CFLAGS := -nostdlib -ffreestanding -02 -g -Wall -Wextra

OBJS = ${C_SOURCES:.c=.o}

# Default build target
all: build

# Run the operating system on qemu
run: all
	qemu-system-i386 -m 1G -serial stdio -kernel $(BUILDDIR)/kernel.bin

monitor: all
	qemu-system-i386 -m 1G -monitor stdio -kernel $(BUILDDIR)/kernel.bin

debug: all
	qemu-system-i386 -m 1G -s -S -monitor stdio -kernel $(BUILDDIR)/kernel.bin

build: asm_objects linker.ld
	i386-elf-gcc -g -I lib/includes -I kernel/includes $(C_SOURCES) $(BUILDDIR)/*.o -o $(BUILDDIR)/kernel.bin -nostdlib -ffreestanding -T linker.ld

asm_objects:
	nasm -f elf32 $(KERNELDIR)/boot.asm -o $(BUILDDIR)/boot.o
	nasm -f elf32 $(KERNELDIR)/utility.asm -o $(BUILDDIR)/utility.o
	nasm -f elf32 $(KERNELDIR)/interrupts.asm -o $(BUILDDIR)/interrupts.o 
	nasm -f elf32 $(KERNELDIR)/logger.asm -o $(BUILDDIR)/logger.o

clean:
	rm -rf $(BUILDDIR)/*

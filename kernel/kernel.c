#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <tty.h>
#include <idt.h>
#include <pic.h>
#include <io.h>
#include <logger.h>
#include <gdt.h>

void kmain(void) {
   init_logger();
   logf("Logger initialized\n");

	init_terminal();
   logf("Terminal initialized\n");

   init_gdt();
   logf("GDT initialized\n");

	init_pic();
   logf("PIC initialized\n");

   init_idt();
   logf("IDT initialized\n");

   __asm__ volatile("int $0x3");
	__asm__ volatile("int $0x4");
}

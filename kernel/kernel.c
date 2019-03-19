#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <tty.h>
#include <idt.h>
#include <pic.h>
#include <io.h>
#include <logger.h>

void kmain(void) {
   init_logger();
	terminal_initialize();
	terminal_print("About to cause an interrupt..\n");
	init_pic();
	init_idt();
	__asm__("int3");
}

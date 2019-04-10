#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <tty.h>
#include <idt.h>
#include <pic.h>
#include <io.h>
#include <logger.h>
#include <gdt.h>
#include <timer.h>
#include <keyboard.h>

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

   init_timer(50);
   logf("Timer initialized\n");

   logf("%u\n", 32); // TODO: this is a big bug
   // I cannot believe that I can't handle this correctly

   init_keyboard();
   logf("Keyboard intialized\n");

   __asm__ volatile("sti");

   while(1) {
      
   }
}

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
#include <memory.h>
#include <panic.h>
#include <multiboot.h>
#include <tasking.h>
#include <filesystem.h>

void check_multiboot_info(multiboot_info_t* mbd, uint32_t magic) {
   // Check if the bootloader magic is correct
   if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
      logf("Magic is incorrect...: %x\n", magic);
      panic("Kernel Panic due to Incorrect Magic");
   }

   logf("mbd->flags: %x\n", mbd->flags);
   if (mbd->flags & MULTIBOOT_INFO_MEMORY) {
      logf("mem_lower = %x\n", mbd->mem_lower);
      logf("mem_upper = %x\n", mbd->mem_upper);
   }

   // Log a map of the regions of memory
   if (mbd->flags & MULTIBOOT_INFO_MEM_MAP) {
      logf("mmap_addr = %x, mmap_length = %x\n", mbd->mmap_addr, mbd->mmap_length);
      multiboot_memory_map_t* map;

      for (map = (multiboot_memory_map_t*) mbd->mmap_addr;
            (uint32_t) map < (mbd->mmap_addr + mbd->mmap_length);
            map = (multiboot_memory_map_t*)((uint32_t) map + map->size + sizeof(map->size))) 
      {
         logf("base_addr= %x, length= %x, type = %x\n",
               map->addr_lower,
               map->len_lower,
               map->type);
      }
   }

   // Check for the ramdisk module
   if (mbd->mods_count <= 0) {
      panic("Module counter shows no ramdisk present");
   }
}

void kmain(multiboot_info_t* mbd, uint32_t magic) {
   init_logger();
   logf("Logger initialized\n");

   check_multiboot_info(mbd, magic);

   // Locate the ramdisk
   uint32_t initrd_start = *((uint32_t*) mbd->mods_addr);
   uint32_t initrd_end = *(uint32_t*)(mbd->mods_addr+4);
   logf("initrd_start: %x\n", initrd_start);
   logf("initrd_end: %x\n", initrd_end);

   // Make sure the ramdisk does not bleed past the kernel end
   if (initrd_end > KERNEL_END) {
      panic("Ramdisk is bleeding past the kernel area!");
   }

   init_filesystem(initrd_start, initrd_end);
   logf("Filesystem Initialized\n");

   panic("stop");

   init_gdt();
   logf("GDT initialized\n");

	init_pic();
   logf("PIC initialized\n");

   init_idt();
   logf("IDT initialized\n");

   init_terminal();
   logf("Terminal initialized\n");

   init_keyboard();
   logf("Keyboard intialized\n");

   init_paging();
   logf("Paging initialized\n");
   
   init_timer(TIMER_FREQUENCY);
   logf("Timer initialized\n");

   // Need to enable interrupts before tasking so that
   // we can receive timer interrupts
   __asm__ volatile("sti");

   init_tasking();
   logf("Tasking initialized\n");

   while(1) {
      
   }
}

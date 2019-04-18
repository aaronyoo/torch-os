#include <memory.h>
#include <stdint.h>
#include "logger.h"
#include <stddef.h>
#include <isr.h>
#include <panic.h>

extern void load_page_directory(uint32_t);
extern void enable_paging();

#define KERNEL_START_PMA 0x00100000
#define KERNEL_START_VMA 0xC0100000
#define AL

void boot_map_page_ia32(uint32_t*, uint32_t, uint32_t);
void page_fault_handler(context_t*);

void init_paging(void) {
    /* Register page fault handler */
    register_interrupt_handler(14, page_fault_handler);

    /* Clear the page directory by marking not present */
    for (unsigned int i = 0; i < 1024; i++) {
        page_directory[i] = 0x00000002;
    }
 
    /* Put initial page table in page directory */
    page_directory[0] = ((uint32_t) init_page_table) | 3;  // supervisor, r/w, present

    /* Identity map ap ~2 MB after the starting point of the kernel */
    /* This maps the kernel so we don't immediately crash */
    for (unsigned int i = 0; i < 512; i++) {
        boot_map_page_ia32(page_directory, (i * 4096) + KERNEL_START_PMA, (i * 4096) + KERNEL_START_PMA);
    }

    /* Identity map the VGA buffer so that we can still write to
       the VGA terminal without immediately page faulting */
    boot_map_page_ia32(page_directory, 0xb8000, 0xb8000);

    /* Map a pool of memory for the memory allocator */
    // TODO:

    /* Load the page directory and initalize paging */
    load_page_directory((uint32_t) page_directory);
    enable_paging();
}

void boot_map_page_ia32(uint32_t* kernel_page_directory, uint32_t virtual_address, uint32_t physical_address) {
    uint32_t page_directory_index = virtual_address >> 22;
    uint32_t page_directory_entry = *(kernel_page_directory + page_directory_index);
    if (!(page_directory_entry & 0x1)) {
        logf("[PANIC] Trying to access page index: %x", page_directory_index);
        logf("Address %x", virtual_address);
        panic("Page Table Not Present");
    }

    uint32_t* page_table = (uint32_t *) (page_directory_entry & 0xFFFFF000);
    uint32_t page_table_index = virtual_address >> 12 & 0x03FF;

    page_table[page_table_index] = physical_address | 3; // supervisor, r/w, present
}

void page_fault_handler(context_t* context) {
    logf("[PANIC] Page fault, hanging\n");
    while(1);
}
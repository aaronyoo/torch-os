#include <memory.h>
#include <stdint.h>

extern void load_page_directory(uint32_t);
extern void enable_paging();

/* Initializes and jumps to the higher half kernel */
void init_higher_half(void) {
    /* Clear the page directory by marking not present */
    for (unsigned int i = 0; i < 1024; i++) {
        page_directory[i] = 0x00000002;
    }

    /* Identity map the first 4 MiB */
    /* Might need to map more if the kernel grows too much */
    for (unsigned int i = 0; i < 1024; i++) {
        init_page_table[i] = (i * 4096) | 3; // supervisor, r/w, present
    }

    /* Put initial page table in page directory */
    page_directory[0] = ((uint32_t) init_page_table) | 3;  // supervisor, r/w, present

    /* Load the page directory and initalize paging */
    load_page_directory((uint32_t) page_directory);
    enable_paging();
}
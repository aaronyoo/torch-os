#include <memory.h>
#include <stdint.h>
#include <logger.h>
#include <stddef.h>
#include <isr.h>
#include <panic.h>
#include <string.h>

extern void load_page_directory(uint32_t);
extern void enable_paging();

#define PAGE_SIZE 4096

#define KERNEL_START 0x00100000
#define KERNEL_END   0x00300000
#define PAGE_TABLE_AREA_START 0x00300000
#define PAGE_TABLE_AREA_END 0x00400000
#define ALLOCATOR_AREA_START 0x00400000
#define ALLOCATOR_AREA_END 0x00C00000

#define BIT_SIZE_OF_UINT32 32

void init_page_frame_allocator();
void init_page_table_allocator();
uint32_t* allocate_page_frame(uint32_t);
void identity_map_range(uint32_t*, uint32_t, uint32_t);
void map_page(uint32_t*, uint32_t, uint32_t);
void page_fault_handler(context_t*);

uint32_t page_directory[1024] __attribute__((aligned(PAGE_SIZE)));
uint32_t first_page_table[1024] __attribute__((aligned(PAGE_SIZE)));
uint32_t second_page_table[1024] __attribute__((aligned(PAGE_SIZE)));

#define BITMAP_SIZE (ALLOCATOR_AREA_END - ALLOCATOR_AREA_START) / 4096 / 32  // calculate the correct sized bitmap
uint32_t bitmap[BITMAP_SIZE];

uint32_t next_page_table;


void init_paging(void) {
    /* Register page fault handler */
    register_interrupt_handler(14, page_fault_handler);

    /* Clear the page directory by marking not present */
    for (unsigned int i = 0; i < 1024; i++) {
        page_directory[i] = 0x00000002;
    }
 
    /* Put initial page tables in page directory */
    page_directory[0] = ((uint32_t) first_page_table) | 3;  // supervisor, r/w, present
    page_directory[1] = ((uint32_t) second_page_table) | 3;

    /* Identity map the kernel and page table reserved areas
       We will need to be able to access them later */
    identity_map_range(page_directory, KERNEL_START, KERNEL_END);
    identity_map_range(page_directory, PAGE_TABLE_AREA_START, PAGE_TABLE_AREA_END);

    init_page_frame_allocator();
    init_page_table_allocator();

    /* Identity map the VGA buffer so that we can still write to
       the VGA terminal without immediately page faulting */
    map_page(page_directory, 0xb8000, 0xb8000);

    /* Load the page directory and initalize paging */
    load_page_directory((uint32_t) page_directory);
    enable_paging();


    // Some test code to test out paging
    // Can be deleted later
    uint32_t* random_page = allocate_page_frame(0x801000);
    uint32_t* another_one = allocate_page_frame(0xF000000);
    logf("%x\n", random_page);
    logf("First bitmap entry: %x\n", bitmap[0]);
    logf("Random value from page: %x\n", *random_page);
    memset(random_page, 0, PAGE_SIZE);
    logf("Should be zero: %x\n", *random_page);
}

void init_page_frame_allocator() {
    /* Clear the bitmap (all pages are unmapped) */
    memset(bitmap, 0, sizeof(bitmap));

    /* Make sure the allocator starts and ends at a page aligned address */
    if (ALLOCATOR_AREA_START % PAGE_SIZE != 0) {
        panic("Page Frame Allocator does not start at a page aligned address!");
    }
    if (ALLOCATOR_AREA_END % PAGE_SIZE != 0) {
        panic("Page Frame Allocator does not end at a page aligned address!");
    }
}

/* Initialize the allocator that takes pages from the
   reserved page table page region */
void init_page_table_allocator() {
    /* This is a watermark allocator which works for now */
    // TODO: change this from a watermark allocator later
    
    /* Make sure that the allocator starts and ends at a page aligned address */
    if (PAGE_TABLE_AREA_START % PAGE_SIZE != 0) {
        panic("Page Table Allocator does not start at a page aligned address!");
    }
    if (PAGE_TABLE_AREA_END % PAGE_SIZE != 0) {
        panic("Page Table Allocator does not end at a page aligned address!");
    }
    next_page_table = PAGE_TABLE_AREA_START;
}

uint32_t* allocate_page_frame(uint32_t virtual_address) {
    /* Make sure the address is page aligned */
    if (virtual_address % PAGE_SIZE != 0) {
        panic("Trying to allocate page frame at unaligned address!");
    }

    /* Find a free page using the bitmap */
    int page_index = -1;
    for (int i = 0; i < BITMAP_SIZE; i++) {
        logf("Looking at bitmap %u: %x\n", i, bitmap[i]);
        if (bitmap[i] != 0xFFFFFFFF) {
            /* There is an available page, scan the bits */
            for (int j = 0; j < BIT_SIZE_OF_UINT32; j++) {
                if (!(bitmap[i] & (1 << j))) {
                    page_index = i * BIT_SIZE_OF_UINT32 + j;
                    bitmap[i] |= (1 << j);
                    break;
                }
            }
            break;
        }
    }

    if (page_index == -1) {
        /* There are no free pages left */
        panic("Allocator has run out of space!");
    }

    const uint32_t physical_address = ALLOCATOR_AREA_START + (PAGE_SIZE * page_index);
    map_page(page_directory, virtual_address, physical_address);

    return (uint32_t*) virtual_address;
}

uint32_t* allocate_page_table() {
    /* Check to make sure we still have more preallocated page tables */
    if (next_page_table >= PAGE_TABLE_AREA_END) {
        panic("Page table allocator has run out of space!");
    }

    uint32_t* page_table = (uint32_t*) next_page_table;
    memset(page_table, 0, PAGE_SIZE);
    next_page_table += PAGE_SIZE;
    return page_table;
}

/* Identity maps the range defined by [start_address, end_address) */
void identity_map_range(uint32_t* kernel_page_directory, uint32_t start_address, uint32_t end_address) {
    for (uint32_t addr = start_address; addr < end_address; addr += PAGE_SIZE) {
        map_page(kernel_page_directory, addr, addr);
    }
}

void map_page(uint32_t* kernel_page_directory, uint32_t virtual_address, uint32_t physical_address) {
    uint32_t page_directory_index = virtual_address >> 22;
    uint32_t page_directory_entry = *(kernel_page_directory + page_directory_index);

    if (!(page_directory_entry & 0x1)) {
        /* Page table is not present so we need to grab a new page table
           and put it into the correct spot */
        logf("Page table not present for virtual address: %x\n", virtual_address);
        logf("Adding a new page table at page_directory_index: %x\n", page_directory_index);

        uint32_t* new_page_table = allocate_page_table();
        logf("New page table allocated at v_addr: %x\n", new_page_table);
        *(kernel_page_directory + page_directory_index) = ((uint32_t) new_page_table) | 3; 

        /* Need to set the page_direcotyr_entry to the new page table and continue */
        page_directory_entry = *(kernel_page_directory + page_directory_index) ;
    }

    uint32_t* page_table = (uint32_t *) (page_directory_entry & 0xFFFFF000);
    uint32_t page_table_index = virtual_address >> 12 & 0x03FF;

    page_table[page_table_index] = physical_address | 3; // supervisor, r/w, present
}

void page_fault_handler(context_t* context) {
    logf("[PANIC] Page fault, hanging\n");
    while(1);
}
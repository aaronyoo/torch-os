#include <memory.h>
#include <stdint.h>
#include <logger.h>
#include <stddef.h>
#include <isr.h>
#include <panic.h>
#include <string.h>

#define PAGE_SIZE 4096

// Logical markers for memory regions
#define KERNEL_START 0x00100000
#define KERNEL_END   0x00300000
#define PAGE_TABLE_AREA_START 0x00500000
#define PAGE_TABLE_AREA_END 0x00900000
#define PAGE_FRAME_ALLOCATOR_AREA_START 0x00900000
#define PAGE_FRAME_ALLOCATOR_AREA_END 0x0F000000
#define HEAP_ALLOCATOR_AREA_START 0x0F000000
#define HEAP_ALLOCATOR_AREA_END 0x11000000

#define BIT_SIZE_OF_UINT32 32
#define INTERRUPT_NUMBER_PAGE_FAULT 14

extern void load_page_directory(uint32_t);
extern void enable_paging();

void init_page_frame_allocator();
void init_page_table_allocator();
void* allocate_page_frame(uint32_t);
void identity_map_range(uint32_t*, uint32_t, uint32_t);
void map_page(uint32_t*, uint32_t, uint32_t);
void page_fault_handler(context_t*);
void init_heap_allocator();

// Calculate the correct sized bitmap for page from allocator
#define BITMAP_SIZE (PAGE_FRAME_ALLOCATOR_AREA_END - PAGE_FRAME_ALLOCATOR_AREA_START) / 4096 / 32  
uint32_t bitmap[BITMAP_SIZE];

// Global pointer for watermark page table allocator
// Points to the next free page_table page
uint32_t next_page_table;

// Pointers for the heap allocator
uint32_t heap_current;  // The current top of the heap
uint32_t heap_mapped;   // Points to the furthest heap address mapped

void init_paging(void) {
    logf("%x\n",BITMAP_SIZE);

    // Register page fault handler
    register_interrupt_handler(INTERRUPT_NUMBER_PAGE_FAULT, page_fault_handler);

    // Clear the page directory by marking not present
    for (unsigned int i = 0; i < 1024; i++) {
        page_directory[i] = 0x00000002;
    }
 
    // Put initial page table sin the page directory
    page_directory[0] = ((uint32_t) first_page_table) | 3;  // supervisor, r/w, present
    page_directory[1] = ((uint32_t) second_page_table) | 3;

    // Identity map the kernel and page table reserved areas
    // We will need to be able to access them on demand later
    identity_map_range(page_directory, KERNEL_START, KERNEL_END);
    identity_map_range(page_directory, PAGE_TABLE_AREA_START, PAGE_TABLE_AREA_END);

    // Initialize allocators
    init_page_frame_allocator();
    init_page_table_allocator();
    init_heap_allocator();

    // Identity map the VGA buffer so that we can still write to
    // the VGA terminal without immediately page faulting
    map_page(page_directory, 0xb8000, 0xb8000);

    // Load the page directory and initalize paging
    load_page_directory((uint32_t) page_directory);
    enable_paging();

    /* TODO: gotta fix this bug, I don't know why it page faults */
    // uint32_t* random_page = allocate_page_frame(0x00900000);
}

void init_page_frame_allocator() {
    // Clear the bitmap (all pages are unmapped)
    memset(bitmap, 0, sizeof(bitmap));

    // Make sure the allocator starts and ends at a page aligned address
    if (PAGE_FRAME_ALLOCATOR_AREA_START % PAGE_SIZE != 0) {
        panic("Page Frame Allocator does not start at a page aligned address!");
    }
    if (PAGE_FRAME_ALLOCATOR_AREA_END % PAGE_SIZE != 0) {
        panic("Page Frame Allocator does not end at a page aligned address!");
    }
}

void init_page_table_allocator() {
    // The page table allocator take pre-mapped pages from the
    // page table reserved area in order to resolve the chicken
    // and egg problem that can sometimes come up when needing to
    // allocate a new page table.
    // TODO: change this from a watermark allocator later
    
    // Make sure that the allocator starts and ends at a page aligned address
    if (PAGE_TABLE_AREA_START % PAGE_SIZE != 0) {
        panic("Page Table Allocator does not start at a page aligned address!");
    }
    if (PAGE_TABLE_AREA_END % PAGE_SIZE != 0) {
        panic("Page Table Allocator does not end at a page aligned address!");
    }
    next_page_table = PAGE_TABLE_AREA_START;
}

void* allocate_page_frame(uint32_t virtual_address) {
    // Make sure the address is page aligned
    if (virtual_address % PAGE_SIZE != 0) {
        panic("Trying to allocate page frame at unaligned address!");
    }

    // Find a free page using the bitmap
    int page_index = -1;
    for (int i = 0; i < BITMAP_SIZE; i++) {
        logf("Looking at bitmap %u: %x\n", i, bitmap[i]);
        if (bitmap[i] != 0xFFFFFFFF) {
            // There is an available page, scan the bits
            for (int j = 0; j < BIT_SIZE_OF_UINT32; j++) {
                if (!(bitmap[i] & (1 << j))) {
                    // The current bit is set to 0 so we can allocate
                    page_index = i * BIT_SIZE_OF_UINT32 + j;
                    bitmap[i] |= (1 << j);
                    break;
                }
            }
            break;
        }
    }

    if (page_index == -1) {
        // There are no free pages left
        panic("Page frame allocator has run out of space!");
    }

    // Calculate the physical address and map it in memory
    const uint32_t physical_address = PAGE_FRAME_ALLOCATOR_AREA_START + (PAGE_SIZE * page_index);
    map_page(page_directory, virtual_address, physical_address);

    return (uint32_t*) virtual_address;
}

void* allocate_page_table() {
    // Check to make sure we still have more preallocated page tables
    if (next_page_table >= PAGE_TABLE_AREA_END) {
        panic("Page table allocator has run out of space!");
    }

    // Allocate the next page table and increment watermark
    uint32_t* page_table = (uint32_t*) next_page_table;
    memset(page_table, 0, PAGE_SIZE);
    next_page_table += PAGE_SIZE;
    return page_table;
}

void identity_map_range(uint32_t* kernel_page_directory, uint32_t start_address, uint32_t end_address) {
    // Identity map the range defined by [start_address, end_address)
    for (uint32_t addr = start_address; addr < end_address; addr += PAGE_SIZE) {
        map_page(kernel_page_directory, addr, addr);
    }
}

void map_page(uint32_t* kernel_page_directory, uint32_t virtual_address, uint32_t physical_address) {
    uint32_t page_directory_index = virtual_address >> 22;
    uint32_t page_directory_entry = *(kernel_page_directory + page_directory_index);

    if (!(page_directory_entry & 0x1)) {
        // Page table is not present so we need to grab a new page table
        // and put it into the correct spot
        logf("Page table not present for virtual address: %x\n", virtual_address);
        logf("Adding a new page table at page_directory_index: %x\n", page_directory_index);

        uint32_t* new_page_table = allocate_page_table();
        logf("New page table allocated at v_addr: %x\n", new_page_table);
        *(kernel_page_directory + page_directory_index) = ((uint32_t) new_page_table) | 3; 

        // Put the new page table in the page directory
        page_directory_entry = *(kernel_page_directory + page_directory_index) ;
    }

    uint32_t* page_table = (uint32_t *) (page_directory_entry & 0xFFFFF000);
    uint32_t page_table_index = virtual_address >> 12 & 0x03FF;

    page_table[page_table_index] = physical_address | 3; // supervisor, r/w, present
}

void page_fault_handler(context_t* context) {
    // TODO: change this eventually to actually handle page faulting
    logf("[PANIC] Page fault, hanging\n");
    while(1);
}

/****************************************************
 * Heap Allocator:
 * - A watermark heap alllocator for the time being
 * TODO: change this from a watermark allocator later
 ****************************************************/

void init_heap_allocator() {
    // Make sure the heap allocator starts and ends at a page aligned address
    if (HEAP_ALLOCATOR_AREA_START % PAGE_SIZE != 0) {
        panic("Heap allocator does not start at a page aligned address!");
    }
    if (HEAP_ALLOCATOR_AREA_END % PAGE_SIZE != 0) {
        panic("Heap allocator does not end at a page aligned address!");
    }

    // Both the current location of the heap and the mapped pointer
    // should start at the same location
    heap_current = HEAP_ALLOCATOR_AREA_START;
    heap_mapped = HEAP_ALLOCATOR_AREA_START;
}

void* heap_allocate(size_t size) {
    logf("Heap allocation of size %x bytes at address %x\n", size, heap_current);
    // Check that tht heap does not overflow
    if (heap_current + size > HEAP_ALLOCATOR_AREA_END) {
        panic("Heap is overflowing! Cannot allocate!");
    }

    // Allocate new pages for the heap on demand
    // When the current pointer passes the furthest mapped address
    while (heap_current + size > heap_mapped) {
        allocate_page_frame(heap_mapped);
        heap_mapped += PAGE_SIZE;
    }

    // Clear the memory before allocating
    memset((void*) heap_current, 0, size);

    uint32_t ret = heap_current;
    heap_current += size;

    return (void*) ret;
}


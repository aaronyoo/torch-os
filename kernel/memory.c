#include <memory.h>
#include <stdint.h>
#include <logger.h>
#include <stddef.h>
#include <isr.h>
#include <panic.h>
#include <string.h>
#include <stdbool.h>

#define PAGE_SIZE 4096

#define BIT_SIZE_OF_UINT32 32
#define INTERRUPT_NUMBER_PAGE_FAULT 14

extern void load_page_directory(uint32_t);
extern void enable_paging();

void init_page_frame_allocator();
void init_page_table_allocator();
void* page_frame_allocate(uint32_t);
void identity_map_range(uint32_t*, uint32_t, uint32_t);
void map_page(uint32_t*, uint32_t, uint32_t);
void page_fault_handler(context_t*);
void init_heap_allocator();

// TODO:
// - Fix this bug uint32_t* random_page = page_frame_allocate(0x00900000);
//   I don't know why it page faults
// - Change page table allocator from a watermark allocator later
// - Change the page fault handler to actually do something instead of panic


//-----------------------------------------------------------------------------
// Global Variables and State
//-----------------------------------------------------------------------------

// Calculate the correct sized page_frame_bitmap for page frame allocator
// The 32 divisor is because we are using uint32_t for our bitmap
#define PAGE_FRAME_BITMAP_SIZE (PAGE_FRAME_ALLOCATOR_AREA_END - PAGE_FRAME_ALLOCATOR_AREA_START) / PAGE_SIZE / 32
uint32_t page_frame_bitmap[PAGE_FRAME_BITMAP_SIZE];

// Calculate the correct sized page_table_bitmap for the page table allocator
// The 32 divisor is because we are using uint32_t for our bitmap
#define PAGE_TABLE_BITMAP_SIZE (PAGE_TABLE_AREA_END - PAGE_TABLE_AREA_START) / PAGE_SIZE / 32
uint32_t page_table_bitmap[PAGE_TABLE_BITMAP_SIZE];

// Global pointer for watermark page table allocator
// Points to the next free page_table page
uint32_t next_page_table;

// Pointers for the heap allocator
uint32_t heap_current;  // The current top of the heap
uint32_t heap_mapped;   // Points to the furthest heap address mapped

//-----------------------------------------------------------------------------
// Page Frame Allocator
//-----------------------------------------------------------------------------

void init_page_frame_allocator() {
    // Clear the page_frame_bitmap (all pages are unmapped)
    memset(page_frame_bitmap, 0, sizeof(page_frame_bitmap));

    // Make sure the allocator starts and ends at a page aligned address
    if (PAGE_FRAME_ALLOCATOR_AREA_START % PAGE_SIZE != 0) {
        panic("Page Frame Allocator does not start at a page aligned address!");
    }
    if (PAGE_FRAME_ALLOCATOR_AREA_END % PAGE_SIZE != 0) {
        panic("Page Frame Allocator does not end at a page aligned address!");
    }
}

void* page_frame_allocate(uint32_t virtual_address) {
    // Make sure the address is page aligned
    if (virtual_address % PAGE_SIZE != 0) {
        panic("Trying to allocate page frame at unaligned address!");
    }

    // Find a free page using the page_frame_bitmap
    int page_index = -1;
    for (int i = 0; i < PAGE_FRAME_BITMAP_SIZE; i++) {
        logf("Looking at page_frame_bitmap %u: %x\n", i, page_frame_bitmap[i]);
        if (page_frame_bitmap[i] != 0xFFFFFFFF) {
            // There is an available page, scan the bits
            for (int j = 0; j < BIT_SIZE_OF_UINT32; j++) {
                if (!(page_frame_bitmap[i] & (1 << j))) {
                    // The current bit is set to 0 so we can allocate
                    page_index = i * BIT_SIZE_OF_UINT32 + j;
                    page_frame_bitmap[i] |= (1 << j);
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

    memset((void*) virtual_address, 0, PAGE_SIZE); // Initialize Page to Zeroes

    return (uint32_t*) virtual_address;
}

//-----------------------------------------------------------------------------
// Page Table Allocator
//-----------------------------------------------------------------------------

void init_page_table_allocator() {
    // Identity map the page table area
    identity_map_range(page_directory, PAGE_TABLE_AREA_START, PAGE_TABLE_AREA_END);

    // Clear the page table bitmap (all page tables are unmapped)
    memset(page_table_bitmap, 0, sizeof(page_frame_bitmap));

    // Make sure that the allocator starts and ends at a page aligned address
    if (PAGE_TABLE_AREA_START % PAGE_SIZE != 0) {
        panic("Page Table Allocator does not start at a page aligned address!");
    }
    if (PAGE_TABLE_AREA_END % PAGE_SIZE != 0) {
        panic("Page Table Allocator does not end at a page aligned address!");
    }
}

void* page_table_allocate(void) {
    // Find a free page using the page table bitmap
    int index = -1;
    for (int i = 0; i < PAGE_TABLE_BITMAP_SIZE; i++) {
        if (page_table_bitmap[i] != 0xFFFFFFFF) {
            // There is a free page table, scan the bits
            for (int j = 0; j < 32; j++) {
                if (!(page_table_bitmap[i] & (1 << j))) {
                    // The current bit is set to 0 so we can allocate
                    index = i * BIT_SIZE_OF_UINT32 + j;
                    page_table_bitmap[i] |= (1 << j);
                    break;
                }
            }
            break;
        }
    }

    if (index == -1) {
        // There are no free page tables left
        panic("Trying to allocate page tables but no page tables left to allocate!");
    }

    // Since page tables are identity mapped, the virtual address and physical
    // address are the same so we can return the physical address.
    const uint32_t physical_address = PAGE_TABLE_AREA_START + (PAGE_SIZE * index);
    return (uint32_t*) physical_address;
}

//-----------------------------------------------------------------------------
// Heap Allocator
//-----------------------------------------------------------------------------

void init_heap_allocator() {
    // Make sure the heap allocator starts and ends at a page aligned address
    if (HEAP_ALLOCATOR_AREA_START % PAGE_SIZE != 0) {
        panic("Heap allocator does not start at a page aligned address!");
    }
    if (HEAP_ALLOCATOR_AREA_END % PAGE_SIZE != 0) {
        panic("Heap allocator does not end at a page aligned address!");
    }

    // Identity map the entire heap
    // [WARNING], this may not be a good idea. But we need to be able to access the heap if
    // we want to do something like a free list so idk how that would work to be honest.
    // identity_map_range(page_directory, HEAP_ALLOCATOR_AREA_START, HEAP_ALLOCATOR_AREA_END);

    // Both the current location of the heap and the mapped pointer
    // should start at the same location
    heap_current = HEAP_ALLOCATOR_AREA_START;
    heap_mapped = HEAP_ALLOCATOR_AREA_START;
}

void* heap_allocate(size_t size) {
    logf("Heap allocation of size %x bytes at address %x\n", size, heap_current);
    // Check that the heap does not overflow
    if (heap_current + size > HEAP_ALLOCATOR_AREA_END) {
        panic("Heap is overflowing! Cannot allocate!");
    }

    // Allocate new pages for the heap on demand
    // When the current pointer passes the furthest mapped address
    while (heap_current + size > heap_mapped) {
        page_frame_allocate(heap_mapped);
        heap_mapped += PAGE_SIZE;
    }

    // Clear the memory before allocating
    memset((void*) heap_current, 0, size);

    uint32_t ret = heap_current;
    heap_current += size;

    return (void*) ret;
}

//-----------------------------------------------------------------------------
// Page Directory and Page Table Functions
//-----------------------------------------------------------------------------
page_directory_entry new_page_directory_entry() {
    // Return a template for a new page_directory_entry with
    // some sane and controllable defaults
    return 0x00000000; // default to supervisor
} 
uint32_t* get_page_table_address(page_directory_entry entry) { return (uint32_t*) (entry & 0xFFFFF000); }
uint32_t get_page_table_index(uint32_t virtual_address) { return virtual_address >> 12 & 0x03FF; }
bool is_present(page_directory_entry entry) { return entry & (1 << 0); }
void set_present(uint32_t* entry) { (*entry) |= (1 << 0); }
void unset_present(uint32_t* entry) { (*entry) &= ~(1 << 0); }
void set_read_write(uint32_t* entry) { (*entry) |= (1 << 1); }
void set_user_mode(uint32_t* entry) { (*entry) |= (1 << 2); }


void identity_map_range(uint32_t* kernel_page_directory, uint32_t start_address, uint32_t end_address) {
    // Identity map the range defined by [start_address, end_address)
    for (uint32_t addr = start_address; addr < end_address; addr += PAGE_SIZE) {
        map_page(kernel_page_directory, addr, addr);
    }
}

void map_page(uint32_t* kernel_page_directory, uint32_t virtual_address, uint32_t physical_address) {
    // Check that both the virtual address and the physical address are page aligned
    if (virtual_address % PAGE_SIZE != 0) {
        panic("Trying to map a virtual address that is not page aligned!");
    }
    if (physical_address % PAGE_SIZE != 0) {
        panic("Trying to map a physical address that is not page aligned!");
    }

    int page_directory_index = virtual_address >> 22;
    page_directory_entry pde = *(kernel_page_directory + page_directory_index);

    if (!is_present(pde)) {
        // Page table is not present so we need to grab a new page table
        // and put it into the correct spot
        logf("Page table not present for virtual address: %x\n", virtual_address);
        logf("Adding a new page table at page_directory_index: %x\n", page_directory_index);

        uint32_t* new_page_table = page_table_allocate();
        logf("New page table allocated at v_addr: %x\n", new_page_table);
        *(kernel_page_directory + page_directory_index) = ((uint32_t) new_page_table) | 3; 

        // Put the new page table in the page directory
        pde = *(kernel_page_directory + page_directory_index) ;
    }

    uint32_t* page_table = get_page_table_address(pde);
    uint32_t page_table_index = virtual_address >> 12 & 0x03FF;

    // Set present and read write for the physical address
    set_present(&physical_address);
    set_read_write(&physical_address);
    page_table[page_table_index] = physical_address;
}

void unmap_page(uint32_t* kernel_page_directory, uint32_t virtual_address) {
    // Check that the virtual address is page aligned
    if (virtual_address % PAGE_SIZE != 0) {
        panic("Trying to unmap a virtual address that is not page aligned!");
    }

    int page_directory_index = virtual_address >> 22;
    page_directory_entry pde = *(kernel_page_directory + page_directory_index);

    // Make sure that the page directory entry is present
    if (!(is_present(pde))) {
        panic("Trying to unmap a virtual address that does not have a page directory entry!");
    }

    uint32_t* page_table = get_page_table_address(pde);
    uint32_t page_table_index = get_page_table_index(virtual_address);

    // Unset the present bit
    uint32_t physical_address = page_table[page_table_index];

    if (!is_present(physical_address)) {
        panic("Trying to unmap a virtual address that has not been mapped yet!");
    }

    unset_present(&physical_address);
    page_table[page_table_index] = physical_address;

    // TODO: maybe I can free the page table by scanning all the other
    // entries to see if they are present. Otherwise I may have to free
    // page tables only when the page directory is freed or in a GC style
}

void page_fault_handler(context_t* context) {
    logf("[PANIC] Page fault, hanging\n");
    while(1);
}

void init_paging(void) {
    logf("%x\n",PAGE_FRAME_BITMAP_SIZE);

    // Register page fault handler
    register_interrupt_handler(INTERRUPT_NUMBER_PAGE_FAULT, page_fault_handler);

    // Clear the page directory by marking not present
    for (unsigned int i = 0; i < 1024; i++) {
        page_directory[i] = 0x00000002;
    }
 
    // Put initial page table sin the page directory
    page_directory[0] = ((uint32_t) first_page_table) | 3;  // supervisor, r/w, present
    page_directory[1] = ((uint32_t) second_page_table) | 3;

    // Identity map the kernel
    identity_map_range(page_directory, KERNEL_START, KERNEL_END);

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
}
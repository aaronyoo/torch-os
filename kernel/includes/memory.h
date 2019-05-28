#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <stdint.h>
#include <stddef.h>

#define PAGE_SIZE 4096
#define halloc(a) heap_allocate(a)
#define kalloc(a) heap_allocate(a)

// Logical markers for memory regions
#define KERNEL_START 0x00100000
#define KERNEL_END   0x00300000
#define PAGE_TABLE_AREA_START 0x00500000
#define PAGE_TABLE_AREA_END 0x00900000
#define PAGE_FRAME_ALLOCATOR_AREA_START 0x00900000
#define PAGE_FRAME_ALLOCATOR_AREA_END 0x0F000000
#define HEAP_ALLOCATOR_AREA_START 0x0F000000
#define HEAP_ALLOCATOR_AREA_END 0x11000000

typedef uint32_t page_directory_entry;
typedef uint32_t page_table_entry;

page_directory_entry page_directory[1024] __attribute__((aligned(PAGE_SIZE)));
page_table_entry first_page_table[1024] __attribute__((aligned(PAGE_SIZE)));
page_table_entry second_page_table[1024] __attribute__((aligned(PAGE_SIZE)));

void init_paging(void);
void* heap_allocate(size_t size);

#endif
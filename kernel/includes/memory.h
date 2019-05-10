#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <stdint.h>
#include <stddef.h>

#define PAGE_SIZE 4096
#define halloc(a) heap_allocate(a)
#define kalloc(a) heap_allocate(a)

uint32_t page_directory[1024] __attribute__((aligned(PAGE_SIZE)));
uint32_t first_page_table[1024] __attribute__((aligned(PAGE_SIZE)));
uint32_t second_page_table[1024] __attribute__((aligned(PAGE_SIZE)));

void init_paging(void);
void* heap_allocate(size_t size);

#endif
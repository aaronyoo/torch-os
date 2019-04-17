#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <stdint.h>

#define KERNEL_START_PMA 0x00100000

uint32_t page_directory[1024] __attribute__((aligned(4096)));
uint32_t init_page_table[1024] __attribute__((aligned(4096)));

void init_paging(void);

#endif
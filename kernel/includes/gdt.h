#ifndef __GDT_H__
#define __GDT_H__

#include <stdint.h>

struct _gdt_entry {    // also known as a segment descriptor
    uint16_t limit_low;     // limit[0-15]
    uint16_t base_low;      // base[0-15]
    uint8_t base_middle;    // base[16-23]
    uint8_t access;         // access flags
    uint8_t granularity;    // misc bits including limit[16-19]
    uint8_t base_high;      // base[24-31]
} __attribute__((packed));
typedef struct _gdt_entry gdt_entry_t;

struct _gdt_ptr {       // also known as a pseudo-descriptor
    uint16_t size;
    uint32_t address;
} __attribute__((packed));  
typedef struct _gdt_ptr gdt_ptr_t;

void init_gdt();

#define GDT_NUM_ENTRIES 5
gdt_ptr_t gdt_ptr;
gdt_entry_t gdt_entries[GDT_NUM_ENTRIES];

#endif
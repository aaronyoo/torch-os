#ifndef __IDT_H__
#define __IDT_H__

#include <stdint.h>

struct _idt_entry_t {
	uint16_t base_lo;
	uint16_t sel;
	uint8_t res;
	uint8_t flags;
	uint16_t base_hi;
} __attribute__((packed));
typedef struct _idt_entry_t idt_entry_t;

struct _idt_ptr_t {
	uint16_t limit;
	uint32_t base;
} __attribute__((packed));
typedef struct _idt_ptr_t idt_ptr_t;

#define IDT_ENTRIES 256
idt_entry_t idt[IDT_ENTRIES];
idt_ptr_t idt_ptr;

int init_idt();

#endif
#ifndef __IDT_H__
#define __IDT_H__

#include <stdint.h>

struct _idt_entry {
	uint16_t baseLo;
	uint16_t sel;
	uint8_t res;
	uint8_t flags;
	uint16_t baseHi;
} __attribute__((packed));
typedef struct _idt_entry idt_entry;

struct _idt_ptr {
	uint16_t limit;
	uint32_t base;
} __attribute__((packed));
typedef struct _idt_ptr idt_ptr;

#define IDT_ENTRIES 256
static idt_entry idt[IDT_ENTRIES];
static idt_ptr idtr;

int init_idt();

#endif
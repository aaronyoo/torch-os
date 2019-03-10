#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <tty.h>

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

static void idt_install() {
	__asm__(
		"lidt (idtr)\n"
	);
}

void i86_default_handler() {
	terminal_print("Unhandled Exception\n");
	terminal_print("Hanging...\n");
	for (;;);
}

void install_handler(uint8_t n, uint32_t base, uint16_t sel, uint8_t flags) {
	idt[n].baseLo = base & 0xFFFF;
	idt[n].baseHi = (base >> 16) & 0xFFFF;
	idt[n].sel = sel;
	idt[n].res = 0;

	// need to | 0x60 later when I create a userspace
	idt[n].flags = flags;
}

int init_idt() {
	idtr.limit = sizeof(idt_entry) * IDT_ENTRIES;
	idtr.base = (uint32_t)idt; 

	memset(idt, 0, sizeof(idt_entry) * IDT_ENTRIES);

	for (int i = 0; i < IDT_ENTRIES; i++) {
		install_handler(i, (uint32_t) i86_default_handler, 0x08, 0x8E);
	}

	idt_install();
}

void kernel_main() {
	terminal_initialize();
	terminal_print("About to cause an interrupt..\n");
	init_idt();
	__asm__("int3");
}

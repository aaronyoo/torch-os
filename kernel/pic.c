#define PIC1		0x20		/* IO base address for master PIC */
#define PIC2		0xA0		/* IO base address for slave PIC */
#define PIC1_COMMAND	PIC1
#define PIC1_DATA	(PIC1+1)
#define PIC2_COMMAND	PIC2
#define PIC2_DATA	(PIC2+1)
#define PIC1_OFFSET 0x20
#define PIC2_OFFSET 0x28

#define PIC_EOI		0x20		/* End-of-interrupt command code */

#define ICW1_ICW4	0x01		/* ICW4 (not) needed */
#define ICW1_INIT	0x10		/* Initialization - required! */
#define ICW4_8086 	0x01		/* 8086 mode */
 
#include <pic.h>
#include <io.h>

void pic_send_eoi(unsigned char irq)
{
	if(irq >= 8)
		outb(PIC2_COMMAND,PIC_EOI);
 
	outb(PIC1_COMMAND,PIC_EOI);
}

void init_pic() {

	// Start initialization in cascade mode.
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);  
	io_wait();
	outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);  
	io_wait();

	// Remap to offsets.
	outb(PIC1_DATA, PIC1_OFFSET); // Master PIC offset
	io_wait();
	outb(PIC2_DATA, PIC2_OFFSET); // Slave PIC offset
	io_wait();

	// Cascade identity of slave with master.
	outb(PIC1_DATA, 4);
	io_wait();
	outb(PIC2_DATA, 2);
	io_wait();

	// 8086 mode on both the master and slave.
	outb(PIC1_DATA, ICW4_8086);
	io_wait();
	outb(PIC2_DATA, ICW4_8086);
	io_wait();

	outb(PIC1_DATA, 0x0);
	io_wait();
	outb(PIC2_DATA, 0x0);
	io_wait();
}
#include <isr.h>
#include <tty.h>
#include <logger.h>
#include <pic.h>

isr_t interrupt_handlers[256];

void isr_handler(registers_t regs) {
    logf("Received interrupt: ");
    logf("%x\n", regs.int_no);
}

void irq_handler(registers_t regs) {
    // Send an EOI to the PIC. 
    // Subtract 32 from the interrupt number to get the IRQ number.
    pic_send_eoi(regs.int_no - 32);

    if (interrupt_handlers[regs.int_no] != 0) {
        isr_t handler = interrupt_handlers[regs.int_no];
        handler(regs);
    }
}

void register_interrupt_handler(uint8_t n, isr_t handler) {
    interrupt_handlers[n] = handler;
}
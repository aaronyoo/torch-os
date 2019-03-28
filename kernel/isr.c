#include <isr.h>
#include <tty.h>
#include <logger.h>
#include <pic.h>

isr_t interrupt_handlers[256];

void isr_handler(context_t* context) {
    logf("Received interrupt: %x\n", context->int_no);

    if (interrupt_handlers[context->int_no] != 0) {
        isr_t handler = interrupt_handlers[context->int_no];
        handler(context);
    }
}

void irq_handler(context_t* context) {
    logf("Received IRQ: %x\n", context->int_no - 32);
    // Send an EOI to the PIC-> 
    // Subtract 32 from the interrupt number to get the IRQ number->
    pic_send_eoi(context->int_no - 32);

    if (interrupt_handlers[context->int_no] == 0) {
        logf("[PANIC] Unhandled IRQ: %x\n", context->int_no - 32);
        while(1); // TODO: change this to panic
    }

    isr_t handler = interrupt_handlers[context->int_no];
    handler(context);
}

void register_interrupt_handler(uint8_t n, isr_t handler) {
    logf("Registering interrupt handler for interrupt %x\n", n);
    interrupt_handlers[n] = handler;
}
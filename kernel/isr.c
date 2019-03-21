#include <isr.h>
#include <tty.h>
#include <logger.h>

void isr_handler(registers_t regs) {
    logf("Received interrupt: ");
    logf("%c\n", regs.int_no + '0'); // TODO: change this, probably the hackiest code I've ever written
}
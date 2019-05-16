#include <timer.h>
#include <isr.h>
#include <io.h>
#include <logger.h>
#include <tty.h>

uint32_t tick = 0;

static void timer_callback(context_t* context)
{
   tick++;
   if (tick % TIMER_FREQUENCY == 0) {
      terminal_print("One second has passed...\n");
      tick = 0;
   }
}

void init_timer(uint32_t frequency)
{
   // Register timer callback
   register_interrupt_handler(32, &timer_callback);

   // Calculate the correct divisor to get our frequency
   uint32_t divisor = 1193180 / frequency;

   // Send the command byte
   outb(0x43, 0x36);

   // Divisor has to be sent byte-wise, so split here into upper/lower bytes
   uint8_t l = (uint8_t)(divisor & 0xFF);
   uint8_t h = (uint8_t)( (divisor>>8) & 0xFF );

   // Send the frequency divisor
   outb(0x40, l);
   outb(0x40, h);
}
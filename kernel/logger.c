/*
 * The logger pushes output through the COM1 serials port.
 * Must be used with the qemu -serial file:[filename] option.
 * COM1 initialization code from: https://wiki.osdev.org/Serial_Ports
 * 
 * TODO: this code may be later refactored to create a serial driver.
 */

#include <io.h>
#include <string.h>
#include <stddef.h>
#include <stdarg.h>
#include <logger.h>
#include <tty.h>

#define COM1_BASE 0x3f8   /* COM1 */
 
static void init_serial_com1();
static int is_transmit_empty();
static void put_serial(const char a);
static void print_serial(const char *s);

void init_logger() {
    init_serial_com1();
}

void logf(const char* format, ...) {
    va_list parameters;
    va_start(parameters, format);

    while (*format != '\0') {
        if (format[0] != '%') {
            put_serial(format[0]);
            format++;
            continue;
        }

        format++;  // skip past '%'

        if (*format == 's') {
            format++;
            const char* str = va_arg(parameters, const char*);
            size_t len = strlen(str);
            print_serial(str);
        } else if (*format == 'c') {
            format++;
            char c = (char) va_arg(parameters, int); // char is promoted to int
            put_serial(c);
        }
    }

    va_end(parameters);
}

static void init_serial_com1() {
   outb(COM1_BASE + 1, 0x00);    // Disable all interrupts
   outb(COM1_BASE + 3, 0x80);    // Enable DLAB (set baud rate divisor)
   outb(COM1_BASE + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
   outb(COM1_BASE + 1, 0x00);    //                  (hi byte)
   outb(COM1_BASE + 3, 0x03);    // 8 bits, no parity, one stop bit
   outb(COM1_BASE + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
   outb(COM1_BASE + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

static int is_transmit_empty() {
   return inb(COM1_BASE + 5) & 0x20;
}
 
static void put_serial(char a) {
   while (is_transmit_empty() == 0);   // Stall while transit is filled
 
   outb(COM1_BASE,a);
}

static void print_serial(const char *s) {
    for (size_t i = 0; i < strlen(s); i++) {
        put_serial(s[i]);
    }
}
#include <logger.h>

void panic(const char* message) {
    logf("[PANIC] - %s\n", message);
    __asm__("cli");
    while (1); // hang
}
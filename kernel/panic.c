#include <logger.h>

void panic(const char* message) {
    logf("[PANIC] - %s\n", message);
    while (1); // hang
}
#include <string.h>

void* memcpy(void* destination, const void* source, size_t num) {
    char* d = destination;
    const char* s = source;
    while (num--) {
        *d++ = *s++;
    }
    return destination;
}
#include <string.h>

// Taken from the C standarad library
// https://clc-wiki.net/wiki/strncmp
int strncmp(const char* s1, const char* s2, size_t n) {
    while(n--)
        if(*s1++!=*s2++)
            return *(unsigned char*)(s1 - 1) - *(unsigned char*)(s2 - 1);
    return 0;
}
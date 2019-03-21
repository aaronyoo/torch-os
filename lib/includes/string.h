#ifndef __STRING_H__
#define __STRING_H__

#include <stddef.h>

void* memset(void*, int, size_t);
size_t strlen(const char* str);
void strrev(char* str);

#endif
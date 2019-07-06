#ifndef __STRING_H__
#define __STRING_H__

#include <stddef.h>

void* memset(void*, int, size_t);
size_t strlen(const char* str);
void strrev(char* str);
char* strcpy(char*, const char*);
void* memcpy(void*, const void*, size_t);
int strncmp(const char* s1, const char* s2, size_t n);

#endif
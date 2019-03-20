#ifndef __TTY_H__
#define __TTY_H__

#include <stddef.h>

void init_terminal(void);
void terminal_putchar(char c);
void terminal_print(const char* data);

#endif
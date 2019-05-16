#ifndef __TIMER_H__
#define __TIMER_H__

#include <stdint.h>

#define TIMER_FREQUENCY 1000

void init_timer(uint32_t frequency);
uint32_t read_timer(void);
uint32_t elapsed_time(uint32_t start, uint32_t end);

#endif
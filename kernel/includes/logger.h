#ifndef __LOGGER_H__
#define __LOGGER_H__

void init_logger();
void put_serial(const char a); // TODO: I only need logger.c to see this, what is the best way to do that?
void logf(const char* format, ...);

#endif
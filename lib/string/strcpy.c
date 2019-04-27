#include <string.h>

char* strcpy(char* destination, const char* source) {
    char *save = destination;
    while (*destination++ = *source++);
    return save;
}
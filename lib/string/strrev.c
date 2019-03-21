#include <string.h>

void strrev(char * str) {
    size_t len = strlen(str);
    for (int i = 0; i <= len/2; i++) {
        char temp = str[i];
        // swap characters
        str[i] = str[len - 1 - i];
        str[len - 1 - i] = temp;
    }
}
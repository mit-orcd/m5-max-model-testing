#include <ctype.h>

void rot13(char *s) {
    while (*s != '\0') {
        if (isalpha(*s)) {
            *s = ((*s & 31) + 13) % 26 + 'A';
        }
        s++;
    }
}